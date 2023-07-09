#pragma once

/*
 * Mouse event based walking simulator.
 * Lets guybrush walk through different locations in woodtick
 * in order to record the adaptive music pieces produced by iMuse.
 */

namespace WoodtickSimulator {

/**
 * The form of the waypoints is { num_of_args, delay_time, x,y, ... }
 */
int walk_to_town_center_from[16][5*3+1] {
    {}, //0
    {}, //1
    {}, //2
    {}, //3
    {}, //4
    {}, //5
    {}, //6
    //7 outside (starting point)
    { 5*3, 
        0, 1,68,
        200, 50,71,
        220, 79,68, // reach center
        200, 57,77, // wait at center
        200, 160,77, // wait at center
    },
    //8 wally
    { 3*3,
        0, 12,91,
        100, 72,78,
        300, 161,76 },
    //9 bar
    { 3*3,
        0, 230,24,
        500, 16,77,
        400, 161,76 },
    {}, //10
    {}, //11
    //12 hotel
    { 3*3,
        0, 244,39,
        400, 161,78,
        300, 161,76 },
    {}, //13
    //14 laundry
    { 3*3,
        0, 182,141,
        150, 144,78,
        300, 161,76 },
    //15 woodshop
    { 4*3,
        0, 81,86,
        100, 23,68,
        300, 57,78,
        400, 161,76 },
};

int walk_inside[16][3*3+1] {
    {}, //0
    {}, //1
    {}, //2
    {}, //3
    {}, //4
    {}, //5
    {}, //6
    {}, //7 outside
    //8 wally
    { 3, 0, 255,56 },
    //9 bar
    { 3, 0, 307,114 },
    {}, //10
    {}, //11
    //12 hotel
    { 3, 0, 66,64 },
    {}, //13
    //14 laundry
    { 3, 0, 185,40 },
    //15 woodshop
    { 2*3,
        0, 315,72,
        200, 268,48 },
};

/**
 * Enum holds all available rooms in woodtick
 * 
 * USAGE: startScene(Woodtick::bar, nullptr, 0);
 * -> Unfortunately, this does not affect the iMuse system
 */
enum Woodtick {
	outside = 7,
    wally = 8,
    bar = 9,
    hotel = 12,
    laundry = 14,
    woodshop = 15,
};
Woodtick WoodtickAsArray[] = {outside, wally, bar, hotel, laundry, woodshop};

/**
 * Get targets in sequential order (TODO make this random)
 */
int target_pos_in_enum=0;
Woodtick next_target() {
    target_pos_in_enum++;
    if (target_pos_in_enum > 5)
        target_pos_in_enum = 1; // skip Woodtick::outside as target
    return WoodtickAsArray[target_pos_in_enum];
}

int current_room = Woodtick::outside;

#define MAX_QUEUE_LENGTH 10
int delayQueue[MAX_QUEUE_LENGTH];
Common::Point pointQueue[MAX_QUEUE_LENGTH];
int queue_pos = -1;

// set false and the SIM will not play
bool active = true;

void quit_simulation() {
    Common::Event event;
    event.type = Common::EVENT_QUIT;
    g_system->getEventManager()->pushEvent(event);
}

/* time till next walk starts */
int idle_timer = 50;

// TODO add randomized time here
void reset_idle_timer() {
    idle_timer = 350; // 350 is verified minimum value
}

void click_at(Common::Point point) {

    if (point.x < 0 || point.x > 319
        || point.y < 0 || point.y > 143) {
        debug("ERROR : mouse position (%d,%d) out of frame", point.x, point.y);
        quit_simulation();
    }

    debug("SIM : click! (x=%d,y=%d)", point.x, point.y);
    Common::Event event;
    event.type = Common::EVENT_LBUTTONDOWN;
    event.mouse = point;
    g_system->getEventManager()->pushEvent(event);
}

/*
 * waypoints must be added in inverse order!
 */
void add_waypoint(int delay, Common::Point point) {
    debug("SIM : add waypoint (delay=%d, x=%d, y=%d)", delay, point.x, point.y);

    queue_pos++;
    if (queue_pos >= MAX_QUEUE_LENGTH) {
        debug("ERROR : queue_pos >= MAX_QUEUE_LENGTH.");
        quit_simulation();
    }

    delayQueue[queue_pos] = delay;
    pointQueue[queue_pos] = point;
}

void add_waypoint(int delay, int x, int y) {
    add_waypoint(delay, Common::Point(x,y));
}

/*
 * this ensures correct ordering of waypoints
 */
void add_multiple_waypoints(int waypoints[]) {
    //// HOWTO:
    // second waypoint: go to 5,5 after waiting for 200 
    //add_waypoint(300, Common::Point(319,143));
    // first waypoint: go to 20,20 without waiting 
    //add_waypoint(0, Common::Point(0,0));

    int num_of_args = waypoints[0];
    for (int i = num_of_args; i>0; i-=3) {
        add_waypoint(waypoints[i-2], waypoints[i-1], waypoints[i]);
    }
}

bool initialized = false; // DO NOT CHANGE!
void simulator_step(int delta) {

    // debug print mouse position to determine click locations
    Common::Point mp = g_system->getEventManager()->getMousePos();
    debug("SIM : Mouse pos=(%d,%d)", mp.x, mp.y);

    if (!initialized) {
        
        add_multiple_waypoints(walk_to_town_center_from[Woodtick::outside]);
        
        initialized = true;
    }

    if (!active) {
        debug("SIM : not active");
        return;
    }

    if (idle_timer > 0) {
		idle_timer -= delta;
        debug("SIM : delta=%d (idle_timer=%d), room=%d.",
            delta, idle_timer, current_room);
        return;
	}
	
    if (queue_pos >= 0) {
        debug("SIM : queue pos=%d, queue_timer=%d", queue_pos, delayQueue[queue_pos]);
        if (delayQueue[queue_pos] > 0) {
            delayQueue[queue_pos] -= delta;
        }
        else {
            click_at(pointQueue[queue_pos]);
            queue_pos--;
        }
    }

    if (idle_timer <= 0
        && queue_pos < 0
        && current_room == Woodtick::outside) {
            
            // TODO: make target a random choice!
            int target_room = next_target();

            debug("SIM : New target is room %d", target_room);
            add_multiple_waypoints(walk_inside[target_room]);
            reset_idle_timer();
        }
}

/**
 * Called by room.cpp for simulation sanity check
 */
void set_current_room(int roomNumber) {
    debug("SIM : got call from room.cpp, entered room %d", roomNumber);

    current_room = roomNumber;

    switch (roomNumber) {
        case woodshop:
        case wally:
        case laundry:
        case hotel:
        case bar:
            add_multiple_waypoints(walk_to_town_center_from[current_room]);
            reset_idle_timer();
        case outside:
            // do not add waypoint for going outside
            // TODO maybe add decision of new destination here?
            break;
        default:
            debug("ERROR : We have left woodtick!");
            quit_simulation();
    }
}

} // End of namespace Scumm

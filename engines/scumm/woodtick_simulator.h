#pragma once

/*
 * Mouse event based walking simulator.
 * Lets guybrush walk through different locations in woodtick
 * in order to record the adaptive music pieces produced by iMuse.
 * 
 * Header-only without any class definition (should be changed in case it is to become more complex).
 */

namespace WoodtickSimulator {

/*
 * Main config:
 */
const uint32 SIM_TIME_LIMIT_IN_SECS = 10*60; // time limit until simulation stops, -1 for endless mode
int MAX_IDLE_TIME = 12000; // max time in one room (is randomly decreased during initialization for more variation between runs)
const bool DEBUG_PRINTS = false; // enable debug prints to stdout
const bool DEBUG_TARGETS = false; // enable forced room route
const bool ACTIVE = true; // if the simulation is active at all

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
    { 4*3,
        0, 182,141,
        150, 75,78,
        500, 161,76,
        100, 161,76 }, // be careful here!
        // -> Screen is not pushed all to the left,
        //    but seems to work for now from all other waypoints..
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
    { 3, 0, 240,56 },
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
 * USAGE in room.cpp: startScene(Woodtick::bar, nullptr, 0);
 * -> Unfortunately, this does not affect the iMuse system
 */
const int WOODTICK_LENGTH = 6;
enum Woodtick {
	outside = 7,
    wally = 8,
    bar = 9,
    hotel = 12,
    laundry = 14,
    woodshop = 15,
};
Woodtick WoodtickAsArray[] = {outside, wally, bar, hotel, laundry, woodshop};

// use this for debugging waypoints
int debug_target_idx = 0;
Woodtick debugTargets[] = {laundry,woodshop,wally,bar,hotel};
/**
 * Return target rooms in random order
 * (or debug order if debug bool is set)
 */
Woodtick next_target() {
		if (DEBUG_TARGETS) {
			return debugTargets[debug_target_idx++];
		}

    int target_pos_in_array = std::rand() % (WOODTICK_LENGTH-1) + 1;
    //if (DEBUG_PRINTS) debug("SIM : next target from array pos=%d", target_pos_in_array);
    return WoodtickAsArray[target_pos_in_array];
}

int current_room = Woodtick::outside;

#define MAX_QUEUE_LENGTH 10
int delayQueue[MAX_QUEUE_LENGTH];
Common::Point pointQueue[MAX_QUEUE_LENGTH];
int queue_pos = -1;

// set false and the SIM will stop playing
bool active = ACTIVE;

void quit_simulation() {
    Common::Event event;
    event.type = Common::EVENT_QUIT;
    g_system->getEventManager()->pushEvent(event);
}

/* time till next walk starts */
int idle_timer = 50;

// TODO add randomized time here
void reset_idle_timer() {
    int time = std::rand() % MAX_IDLE_TIME;
    if (time < 400)
        time = 400; // minimum 400 is tested with 2 hours without crash of waypoints
    idle_timer = time;
}

void click_at(Common::Point point) {

    if (point.x < 0 || point.x > 319
        || point.y < 0 || point.y > 143) {
        debug("SIM ERROR : mouse position (%d,%d) out of frame", point.x, point.y);
        quit_simulation();
    }

    //if (DEBUG_PRINTS) debug("SIM : click! (x=%d,y=%d)", point.x, point.y);
    Common::Event event;
    event.type = Common::EVENT_WOODTICK_SIMULATOR_CLICK;
    event.mouse = point;
    g_system->getEventManager()->pushEvent(event);
}

/*
 * waypoints must be added in inverse order!
 */
void add_waypoint(int delay, Common::Point point) {

    queue_pos++;
    if (DEBUG_PRINTS) debug("SIM : --- add waypoint (delay=%d, x=%d, y=%d) at queue_pos=%d",
                                      delay, point.x, point.y, queue_pos);

    if (queue_pos >= MAX_QUEUE_LENGTH) {
        debug("SIM ERROR : queue_pos >= MAX_QUEUE_LENGTH.");
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
    //Common::Point mp = g_system->getEventManager()->getMousePos();
    //if (DEBUG_PRINTS) debug("SIM : Mouse pos=(%d,%d)", mp.x, mp.y);

    if (!active) {
        if (DEBUG_PRINTS) debug("SIM : not active");
        return;
    }

    uint32 sys_seconds = g_system->getMillis()/1000;
    if (SIM_TIME_LIMIT_IN_SECS > 0
        && sys_seconds >= SIM_TIME_LIMIT_IN_SECS) {
            if (DEBUG_PRINTS) debug("SIM : Simulation reached time limit of %ds (system_time=%ds).",
                                                                SIM_TIME_LIMIT_IN_SECS, sys_seconds);
            active=false;
            quit_simulation();
        }

    if (!initialized) {

		if (DEBUG_PRINTS) debug("SIM : Initializing Woodtick Simulator..");

        // first thing to do: go to start point
        add_multiple_waypoints(walk_to_town_center_from[Woodtick::outside]);

        // initialize seed for random number generation
        TimeDate time;
        g_system->getTimeAndDate(time);
        uint32 seed = time.tm_sec + time.tm_min * 60U + time.tm_hour * 3600U;
        seed += time.tm_mday * 86400U + time.tm_mon * 86400U * 31U;
        seed += time.tm_year * 86400U * 366U;
        uint32 millis = g_system->getMillis();
	    seed = seed * 1000U + millis;
        if (DEBUG_PRINTS) debug("SIM : Initialize random number generator with seed=%u, millis=%d", seed, millis);
        std::srand(seed);

        if (DEBUG_PRINTS) debugN("SIM : Test random numbers: ");
        for (int j=0; j<10; j++) {
            if (DEBUG_PRINTS) debugN("%d, ", std::rand() % 5 + 1);
        }
        if (DEBUG_PRINTS) debug("...");

        MAX_IDLE_TIME = std::rand() % MAX_IDLE_TIME;
        debug("Set MAX_IDLE_TIME=%d", MAX_IDLE_TIME);

        initialized = true;
    }

    if (idle_timer > 0) {
		idle_timer -= delta;
        if (DEBUG_PRINTS) debug("SIM : delta=%d (idle_timer=%d), room=%d.",
            delta, idle_timer, current_room);
        return;
	}
	
    if (queue_pos >= 0) {
        if (DEBUG_PRINTS) debug("SIM : queue pos=%d, queue_timer=%d", queue_pos, delayQueue[queue_pos]);
        if (delayQueue[queue_pos] > 0) {
            delayQueue[queue_pos] -= delta;
        }
        else {
            click_at(pointQueue[queue_pos]);
            queue_pos--;
        }
        return;
    }

    if (current_room == Woodtick::outside) {     
            int target_room = next_target();

            if (DEBUG_PRINTS) debug("SIM : New target is room %d", target_room);
            add_multiple_waypoints(walk_inside[target_room]);
            reset_idle_timer();
        }
}

/**
 * Called by room.cpp for simulation sanity check
 */
void set_current_room(int roomNumber) {
    if (DEBUG_PRINTS) debug("SIM : got call from room.cpp, entered room %d", roomNumber);

    current_room = roomNumber;

    switch (roomNumber) {
        case woodshop:
        case wally:
        case laundry:
        case hotel:
        case bar:
            add_multiple_waypoints(walk_to_town_center_from[current_room]);
            reset_idle_timer();
            break;
        case outside:
            // do not add waypoint for going outside
            // TODO maybe add decision of new destination here?
            break;
        default:
            debug("SIM ERROR : We have left woodtick!");
            quit_simulation();
    }
}

} // End of namespace Scumm

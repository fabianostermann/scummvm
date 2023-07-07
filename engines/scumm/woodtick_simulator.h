#pragma once

/*
 * Mouse event based walking simulator.
 * Lets guybrush walk through different locations in woodtick
 * in order to record the adaptive music pieces produced by iMuse.
 */

namespace WoodtickSimulator {

/**
 * Enum holds all available rooms in woodtick
 * 
 * USAGE: startScene(Woodtick::bar, nullptr, 0);
 * -> Unfortunately, this does not affect the iMuse system
 */
enum Woodtick {
	outside = 7,
    woodshop = 15,
    wally = 8,
    laundry = 14,
    hotel = 12,
    bar = 9
};

int current_room = Woodtick::outside;
int last_room = Woodtick::outside;
int target_room = Woodtick::woodshop;

#define QUEUE_LENGTH 3
int timerQueue[QUEUE_LENGTH];
Common::Point pointQueue[QUEUE_LENGTH];
int queue_pos = -1;

/*
 * So now, we will navigate the actor through woodtick by mouse click events
 */
bool active = true;

void quit_simulation() {
    Common::Event event;
    event.type = Common::EVENT_QUIT;
    g_system->getEventManager()->pushEvent(event);
}

// TODO add randomized time here
int reset_time() {
    return 200;
}

/* time till next walk starts */
int timer = reset_time();

void click_at(Common::Point point) {
    debug("SIM : click!");
    Common::Event event;
    event.type = Common::EVENT_LBUTTONDOWN;
    event.mouse = point;
    g_system->getEventManager()->pushEvent(event);
}

void simulator_step(int delta) {

    if (!active) {
        return;
    }

	timer -= delta;
    debug("SIM : delta=%d (timer=%d), last=%d, now=%d, target=%d",
            delta, timer, last_room, current_room, target_room);

    if (queue_pos > -1) {
        debug("SIM : queue pos=%d, queue_timer=%d", queue_pos, timerQueue[queue_pos]);
        if (timerQueue[queue_pos] > 0) {
            timerQueue[queue_pos] -= delta;
        }
        else {
            click_at(pointQueue[queue_pos]);
            queue_pos--;
        }
        if (queue_pos < 0) // TODO must become a location test in the main loop below
            active = false;
    }
    else if (timer <= 0) {
		
        queue_pos = 1;
        timerQueue[1] = 0;
        pointQueue[1] = Common::Point(20,20);

        timerQueue[0] = 200;
        pointQueue[0] = Common::Point(5,5);

		//timer = reset_time();
	}
}

void set_current_room(int roomNumber) {
    switch (roomNumber) {
        case outside:
        case woodshop:
        case wally:
        case laundry:
        case hotel:
        case bar:
            break;
        default:
            debug("ERROR : We have left woodtick!");
            quit_simulation();
    }
    current_room = roomNumber;
}

} // End of namespace Scumm

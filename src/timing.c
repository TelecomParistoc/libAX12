#include "i2c-cache.h"
#include "i2c-functions.h"
#include "timing.h"
#include <time.h>
#include <stdio.h>

#define SCHEDULED_MAX 40

static void (*scheduledCallbacks[SCHEDULED_MAX])(void) = {NULL};
static long long int scheduledTimes[SCHEDULED_MAX] = {[0 ... SCHEDULED_MAX-1]=-1};
static int scheduledUID[SCHEDULED_MAX] = {[0 ... SCHEDULED_MAX-1]=-1};

long long int getCurrentTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long int currentTime = ts.tv_nsec/1000000 + ts.tv_sec*1000;
    return currentTime;
}

//called each period using I2C cache timer
void timingManager() {
    long long int currentTime = getCurrentTime();
    // call scheduled callbacks
    for(int i=0; i<SCHEDULED_MAX; i++) {
        if(scheduledTimes[i] > 0 && (scheduledTimes[i] - currentTime) <= 0) {
            scheduledTimes[i] = -1;
            if(scheduledCallbacks[i] != NULL)
                scheduledCallbacks[i]();
        }
    }
}

void waitFor(int milliseconds) {
	delayMilli(milliseconds);
}
void waitForMicro(int microseconds) {
	struct timespec wait_time = {
		.tv_sec = 0,
		.tv_nsec = 1000*microseconds
	};
	nanosleep(&wait_time, NULL);
}

int scheduleIn(int milliseconds, void (*callback)(void)) {
    static int uidCounter = 0;
	int i=0;
	while(i<SCHEDULED_MAX && scheduledTimes[i]>0)
		i++;
	if(i==SCHEDULED_MAX) {
        printf("ERROR : scheduled functions buffer full, cannot schedule\n");
        return -1;
    }

	scheduledCallbacks[i] = callback;
	scheduledTimes[i] = getCurrentTime() + milliseconds;
    scheduledUID[i] = uidCounter;

    //make sure loop is started
    startLoop();
	return uidCounter++;
}

int cancelScheduled(int uid) {
    int i=0;
    while(i<SCHEDULED_MAX && scheduledUID[i]!=uid)
		i++;
    if(i==SCHEDULED_MAX) {
        printf("ERROR : scheduled function not found\n");
        return -1;
    }
    scheduledTimes[i] = -1;
    scheduledUID[i] = -1;
    return 0;
}

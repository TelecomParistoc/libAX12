#include "timing.h"
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#define SCHEDULED_MAX 40

static void (*scheduledCallbacks[SCHEDULED_MAX])(void) = {NULL};
static long long int scheduledTimes[SCHEDULED_MAX] = {[0 ... SCHEDULED_MAX-1]=-1};
static int scheduledUID[SCHEDULED_MAX] = {[0 ... SCHEDULED_MAX-1]=-1};

static pthread_t timingThread;
static int timingStarted = 0;

//called each period using I2C cache timer
static void* timingManager(void* arg) {
	long long int currentTime;

	(void) arg;

	while(1) {
		currentTime = getCurrentTime();
		// call scheduled callbacks
		for(int i=0; i<SCHEDULED_MAX; i++) {
			if(scheduledTimes[i] > 0 && (scheduledTimes[i] - currentTime) <= 0) {
				scheduledTimes[i] = -1;
				if(scheduledCallbacks[i] != NULL)
				scheduledCallbacks[i]();
			}
		}
		waitForMicro(2000);
	}
	return NULL;
}

long long int getCurrentTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long int currentTime = ts.tv_nsec/1000000 + ts.tv_sec*1000;
    return currentTime;
}

void waitForMicro(int microseconds) {
	struct timespec wait_time = {
		.tv_sec = 0,
		.tv_nsec = 1000*microseconds
	};
	nanosleep(&wait_time, NULL);
}

void waitFor(int milliseconds) {
    while(milliseconds >= 1000) {
        waitForMicro(500000);
        milliseconds -= 500;
    }
    waitForMicro(1000*milliseconds);
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

	// start loop if it is not started yet
	if(!timingStarted) {
		if(pthread_create(&timingThread, NULL, timingManager, NULL))
			printf("ERROR : cannot create timing update thread\n");
		else
			timingStarted = 1;
	}
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

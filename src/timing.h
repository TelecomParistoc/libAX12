/* This file provides useful timing functions */

#ifndef TIMING_H
#define TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

/* return current time, in ms */
long long int getCurrentTime();

/* wait for the given amount of time (in milliseconds). Please note that actual
 * wait time may vary of +/- 10% the target value */
void waitFor(int milliseconds);
/* wait for the given amount of time (in microseconds). Please note that actual
 * wait time may vary from 0 to the given value */
void waitForMicro(int microseconds);

/* call a function after the given time (+/-10ms precision), starting when
 * scheduleIn is called.
 * milliseconds : the time to wait before calling the function in milliseconds
 * callback : the function to call. It should be like void myFunction() {...}
 *
 * returns : the scheduled function UID (positive), or -1 if an error occured
 */
int scheduleIn(int milliseconds, void (*callback)(void));

/* unscheduled a previously scheduled function (stop it from being called)
 * uid : the unique ID of the task, returned by scheduleIn
 *
 * returns : 0 if task was successfully unscheduled, -1 if it was not found in
 * the pending tasks */
int cancelScheduled(int uid);

#ifdef __cplusplus
}
#endif

#endif

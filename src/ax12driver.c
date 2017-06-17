#include "ax12driver.h"
#include "ax-comm.h"
#include "ax-constants.h"
#include "timing.h"
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define AX_MAX_MOVING 40
#define AX_UPDATE_TIME_LIMIT 3

static void (*axMovingCallbacks[AX_MAX_MOVING])(void);
static int axMovingIDs[AX_MAX_MOVING] = {[0 ... AX_MAX_MOVING-1]=-1};
static double axMovingGoals[AX_MAX_MOVING];

static pthread_t updater;

static int axModes[256] = {[0 ... 255]=-1};

double AX12getPosition(uint8_t id) {
	uint16_t value;
	double position;
	axRead16(id, AX_POS, &value, NULL);

	position = (value - 0x01FF)*0.293255 - 0.146695;
	return roundf(position * 100) / 100;
}
double AX12getSpeed(uint8_t id) {
	uint16_t value;
	double speed;
	axRead16(id, AX_SPEED, &value, NULL);
	speed = roundf((value&0x03FF) * 10000 / 1023.0) / 100;
	if(value & 0x0400)
		speed *= -1;
	return speed;
}
double AX12getLoad(uint8_t id) {
	uint16_t value;
	double load;
	axRead16(id, AX_LOAD, &value, NULL);
	load = roundf((value&0x03FF) * 10000 / 1023.0) / 100;
	if(value & 0x0400)
		load *= -1;
	return load;
}
int AX12getStatus(uint8_t id) {
	uint8_t status;
	axPing(id, &status);
	return status;
}
double AX12getVoltage(uint8_t id) {
	uint8_t volt;
	axRead8(id, AX_VOLT, &volt, NULL);
	return volt/10.0;
}
int AX12getTemperature(uint8_t id) {
	uint8_t temp;
	axRead8(id, AX_TEMP, &temp, NULL);
	return temp;
}
int AX12isMoving(uint8_t id) {
	uint8_t value;
	enableErrorPrint(0);
	axRead8(id, AX_MOVING, &value, NULL);
	enableErrorPrint(1);
	return value == 1;
}

int AX12setMode(uint8_t id, int mode) {
	if(id == 0xFE)
		for(int i=0; i<0xFE; i++)
			axModes[i] = mode;
	else
		axModes[id] = mode;
	return axWrite16(id, AX_CCW_LIMIT, mode ? 0: 0x3FF, NULL);
}
int AX12setSpeed(uint8_t id, double speed) {
	uint16_t value = (fabs(speed)*1023.0)/100;

	if(fabs(speed) > 100)
		value = 0x03FF;
	if(speed < 0)
		value |= 0x0400;
	return axWrite16(id, AX_GOAL_SPEED, value, NULL);
}
int AX12setTorque(uint8_t id, double torque) {
	int code;
	uint16_t value = (fabs(torque)*1023.0)/100;
	if(fabs(torque) > 100)
		value = 0x03FF;

	code = axWrite8(id, AX_TORQUE_ENABLE, roundf(torque*100) == 0 ? 0 : 1, NULL);
	if(roundf(torque*100) != 0)
		code = axWrite16(id, AX_MAX_TORQUE, value, NULL);

	return code;
}
int AX12setLED(uint8_t id, int state) {
	return axWrite8(id, AX_LED, state ? 1 : 0, NULL);
}
int AX12move(uint8_t id, double position, void (*callback)(void)) {
	uint16_t value;
	int i=0, code;

	if(position < -150)
		position = -150;
	if(position > 150)
		position = 150;
	value = (uint16_t) (position + 150.0)*3.41;

	if(axModes[id] != DEFAULT_MODE)
		AX12setMode(id, DEFAULT_MODE);
	code = axWrite16(id, AX_GOAL_POS, value, NULL);

	// make sure there is no remaining reference to this AX12 in the moving AX12 list
	AX12cancelCallback(id);
	AX12cancelCallback(id);

	if(callback != NULL) {
		// find the smallest available index
		while(i<AX_MAX_MOVING && axMovingIDs[i] > -1)
			i++;
		if(i == AX_MAX_MOVING) {
			printf("ERROR : AX12 callback buffer full, callback won't be called\n");
			return -5;
		}

		axMovingGoals[i] = position;
		axMovingIDs[i] = id;
		axMovingCallbacks[i] = callback;
	}
	return code;
}
void AX12cancelCallback(uint8_t id) {
	int i=0;
	while(i<AX_MAX_MOVING && axMovingIDs[i] != id)
		i++;
	if(i < AX_MAX_MOVING)
		axMovingIDs[i] = -1;
}
int AX12turn(uint8_t id, double speed) {
	uint16_t value = (uint16_t) fabs(speed)*1023.0/100;
	if(fabs(speed) > 100)
		value = 0x03FF;
	if(speed < 0)
		value |= 0x0400;
	if(axModes[id] != WHEEL_MODE)
		AX12setMode(id, WHEEL_MODE);
	return axWrite16(id, AX_GOAL_SPEED, value, NULL);
}
void AX12resetAll() {
	axWrite8(0xFE, AX_RETURN, 2, NULL); // AX12 respond to all instructions
	axWrite16(0xFE, AX_DELAY, 3, NULL); // return delay = 6us
	axWrite8(0xFE, AX_ALARM_SHUTDOWN, 0x25, NULL); // torque OFF on overheating, overload, voltage error
	axWrite8(0xFE, AX_ALARM_LED, 0x25, NULL); // LED blinks on overheating, overload, voltage error

	AX12setTorque(0xFE, 100); // enable torque
	AX12setSpeed(0xFE, 50); // by defaut, speed set to half the max
}

static void axUpdateMoving(int i) {
	if(AX12isMoving(axMovingIDs[i]))
		return;

	// check goal position has been reached
	if(fabs(AX12getPosition(axMovingIDs[i])- axMovingGoals[i]) > 1.5) {
		waitFor(20);
		// false alarm, AX12 moving again
		if(AX12isMoving(axMovingIDs[i]))
			return;

		//printf("AX12 error : AX12 %d can't reach its goal\n", axMovingIDs[i]);
	}

	axMovingIDs[i] = -1;
	if(axMovingCallbacks[i] != NULL)
		axMovingCallbacks[i]();
}

static void* axMovingUpdater(void* arg) {
	long long int loopStartTime;
	int i = 0;

	if(arg) {}

	while(1) {
		loopStartTime = getCurrentTime();
		if(i >= AX_MAX_MOVING)
			i = 0;
		for(; i<AX_MAX_MOVING; i++)
			if(axMovingIDs[i] != -1) {
				if(getCurrentTime() - loopStartTime > AX_UPDATE_TIME_LIMIT)
					break;
				axUpdateMoving(i);
			}
		waitFor(100 + loopStartTime - getCurrentTime()); // 1 cycle / 100ms
	}
	return NULL;
}

// init AX12
int initAX12(int baudrate) {
	int code = initAXcomm(baudrate);
	if(code) {
		printf("ERROR : cannot initialize AX12 communication, error code: %d \n", code);
		return code;
	}

	AX12resetAll();

	if(pthread_create(&updater, NULL, axMovingUpdater, NULL)) {
		printf("ERROR : cannot create AX12 update thread\n");
		return -3;
	}
	return 0;
}

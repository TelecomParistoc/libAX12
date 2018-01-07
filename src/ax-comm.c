#include "ax-comm.h"
#include "ax-constants.h"

#ifdef CHIBIOS
#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"

int getDataAvail(input_queue_t *iqp) {
	int ret; /* Number of bytes available for reading */
	chSysLock();
	ret = chIQGetFullI(iqp);
	chSysUnlock();
	return ret;
}

#define SEND_CHAR(driver, value) sdPut(driver, value)
#define GET_CHAR(driver) sdGetTimeout(driver, TIME_IMMEDIATE)
#define DATA_AVAILABLE(driver) getDataAvail(&driver->iqueue)
#define GET_TIME_IN_MS() ST2MS(chVTGetSystemTime())
#define LOCK_SEM(sem) chBSemWait(sem)
#define UNLOCK_SEM(sem) chBSemSignal(sem)
#define INIT_SEM(sem) 1
#define INIT_SERIAL_DRIVER(driver, config) sdStart(driver, config)
#define IS_SD_VALID(driver) (driver != NULL)
#define SERIAL_FLUSH(driver)
#define waitForMicro(delay)
static virtual_timer_t vt;
typedef void(*SchedulerCb)(void);
void vt_cb(void *cb) {
	((SchedulerCb)cb)();
}
#define SCHEDULE_IN(time, cb) chVTSet(&vt, MS2ST(time), vt_cb, cb);
#else /* RASPBERRY PI */
#include <robotutils.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#define SEND_CHAR(driver, value) serialPutchar(driver, value)
#define GET_CHAR(driver) serialGetchar(driver)
#define DATA_AVAILABLE(driver) serialDataAvail(driver)
#define GET_TIME_IN_MS() getCurrentTime()
#define LOCK_SEM(sem) pthread_mutex_lock(sem)
#define UNLOCK_SEM(sem) pthread_mutex_unlock(sem)
#define INIT_SEM(sem) pthread_mutex_init(sem, NULL)
#define INIT_SERIAL_DRIVER(driver, config) driver = serialOpen("/dev/serial0", config->speed)
#define IS_SD_VALID(driver) (driver >= 0)
#define SERIAL_FLUSH(driver) serialFlush(driver)
#define SCHEDULE_IN(time, cb) scheduleIn(time, cb)
#endif /* CHIBIOS */

// make sure there's at most one transaction ongoing
#ifdef CHIBIOS
static SerialDriver *serial = &SD1;
BSEMAPHORE_DECL(serialLock, FALSE);
#else
pthread_mutex_t serialLock;
static int serial = -1;
#endif /* CHIBIOS */

static long long int startTime = 0;

static int errorLog = 1;

int initAXcomm(SerialConfig_t *config) {
	if(INIT_SEM(&serialLock)) {
		printf("ERROR : cannot create mutex\n");
		return -2;
	}

	INIT_SERIAL_DRIVER(serial, config);
	if(!IS_SD_VALID(serial)) {
		printf("ERROR : cannot open AX12 serial port\n");
		return -1;
	}
	return 0;
}

static int axSendPacket(uint8_t id, uint8_t instruction, uint8_t command, uint8_t arg1, uint8_t arg2, int argCount) {
	uint8_t checksum = ~(id + instruction + command + arg1 + arg2 + 2 + argCount);

    if (!IS_SD_VALID(serial)) {
		printf("ERROR : serial port not initialized\n");
		return -1;
	}

	SEND_CHAR(serial, 0xFF); SEND_CHAR(serial, 0xFF);
	SEND_CHAR(serial, id);
	SEND_CHAR(serial, 2+argCount);
	SEND_CHAR(serial, instruction);
	if(argCount>0)
		SEND_CHAR(serial, command);
	if(argCount>1)
		SEND_CHAR(serial, arg1);
	if(argCount==3)
		SEND_CHAR(serial, arg2);
	SEND_CHAR(serial, checksum);

	return 0;
}

static int checkTimeout() {
	return GET_TIME_IN_MS() - startTime > AX_MAX_ANSWER_WAIT;
}
static int axReceiveAnswer(uint8_t expectedId, uint16_t* result, uint8_t* statusError) {
	startTime = GET_TIME_IN_MS();
	while(!checkTimeout()) {
		if(DATA_AVAILABLE(serial) >= 6 && GET_CHAR(serial) == 0xFF && GET_CHAR(serial) == 0xFF) {
			uint8_t id, length, error, checksum, arg1, arg2;
			id = GET_CHAR(serial);
			length = GET_CHAR(serial);
			error = GET_CHAR(serial);
			if(length > 2) {
				arg1 = GET_CHAR(serial);
				// wait for one more byte
				while(!checkTimeout() && DATA_AVAILABLE(serial) < 1)
					waitForMicro(100);
			} else {
				arg1 = 0;
			}
			if(length > 3 && !checkTimeout()) {
				arg2 = GET_CHAR(serial);
				// wait for one more byte
				while(!checkTimeout() && DATA_AVAILABLE(serial) < 1)
					waitForMicro(100);
			} else {
				arg2 = 0;
			}

			// make sure packet came back complete and without error
			if(checkTimeout())
				return -4;
			checksum = GET_CHAR(serial);
			if(((uint8_t)~(id+length+error+arg1+arg2)) != checksum)
				return -2;
			if(id != expectedId)
				return -3;
			if(statusError != NULL)
				*statusError = error;
			if(result != NULL)
				*result = arg1 + (arg2 << 8);
			return 0;
		}
		waitForMicro(200);
	}
	return -4;
}
static void printCommError(int id, int code) {
	printf("AX12 communication ERROR (with id=%d) : ", id);
	switch(code) {
	case -1:
		printf("serial port not initialized\n");
		break;
	case -2:
		printf("wrong checksum\n");
		break;
	case -3:
		printf("ID doesnt match\n");
		break;
	case -4:
		printf("timeout\n");
		break;
	}
}

static void releaseSerialLock() {
	UNLOCK_SEM(&serialLock);
}

static int axTransaction(uint8_t id, uint8_t instruction, uint8_t command, uint16_t arg,
	int argCount, uint16_t* result, uint8_t* error)
{
	int code = 0;
	int index;

	LOCK_SEM(&serialLock); // only one transaction at a time
	for(index = 0; index < AX_SEND_RETRY + 1; index++) {
		SERIAL_FLUSH(serial); // make sure there is no byte left in RX buffer
		if(axSendPacket(id, instruction, command, arg&0xFF, (arg >> 8)&0xFF, argCount)) {
			code = -1;
			break;
		}
		if(id == 0xFE) break; // no answer when broadcasting
		code = axReceiveAnswer(id, result, error);
		if(!code) break; // if everything went well, return, otherwise retry
	}

	// wait before releasing the lock to make sure AX12 has time to recover
	SCHEDULE_IN(15, releaseSerialLock);

	if(errorLog && code != 0)
		printCommError(id, code);
	return code;
}

int axWrite8(uint8_t id, uint8_t command, uint8_t arg, uint8_t* statusError) {
	return axTransaction(id, AX_WRITE_DATA, command, arg, 2, (uint16_t*)NULL, statusError);
}

int axWrite16(uint8_t id, uint8_t command, uint16_t arg, uint8_t* statusError) {
	return axTransaction(id, AX_WRITE_DATA, command, arg, 3, (uint16_t*)NULL, statusError);
}

int axRead8(uint8_t id, uint8_t command, uint8_t* result, uint8_t* statusError) {
	int code;
	uint16_t result16;
	code = axTransaction(id, AX_READ_DATA, command, 1, 2, &result16, statusError);
	*result = (uint8_t) result16;
	return code;
}

int axRead16(uint8_t id, uint8_t command, uint16_t* result, uint8_t* statusError) {
	return axTransaction(id, AX_READ_DATA, command, 2, 2, result, statusError);
}

int axPing(uint8_t id, uint8_t* statusError) {
	return axTransaction(id, AX_PING, 0, 0, 0, (uint16_t*)NULL, statusError);
}

int axFactoryReset(uint8_t id, uint8_t* statusError) {
	return axTransaction(id, AX_RESET, 0, 0, 0, (uint16_t*)NULL, statusError);
}

void enableErrorPrint(int enable) {
	errorLog = enable;
}

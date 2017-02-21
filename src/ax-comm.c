#include "ax-comm.h"
#include "ax-constants.h"
#include <robotutils.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// make sure there's at most one transaction ongoing
pthread_mutex_t serialLock;

static int serial = -1;
static long long int startTime = 0;

static int errorLog = 1;

int initAXcomm(int baudrate) {
	if(pthread_mutex_init(&serialLock, NULL)) {
		printf("ERROR : cannot create mutex\n");
		return -2;
	}
	serial = serialOpen("/dev/serial0", baudrate);
	if(serial == -1) {
		printf("ERROR : cannot open AX12 serial port\n");
		return -1;
	}
	return 0;
}

static int axSendPacket(uint8_t id, uint8_t instruction, uint8_t command, uint8_t arg1, uint8_t arg2, int argCount) {
	uint8_t checksum = ~(id + instruction + command + arg1 + arg2 + 2 + argCount);
	if(serial < 0) {
		printf("ERROR : serial port not initialized\n");
		return -1;
	}
	serialPutchar(serial, 0xFF); serialPutchar(serial, 0xFF);
	serialPutchar(serial, id);
	serialPutchar(serial, 2+argCount);
	serialPutchar(serial, instruction);
	if(argCount>0)
		serialPutchar(serial, command);
	if(argCount>1)
		serialPutchar(serial, arg1);
	if(argCount==3)
		serialPutchar(serial, arg2);
	serialPutchar(serial, checksum);

	return 0;
}

static int checkTimeout() {
	return getCurrentTime() - startTime > AX_MAX_ANSWER_WAIT;
}
static int axReceiveAnswer(uint8_t expectedId, uint16_t* result, uint8_t* statusError) {
	startTime = getCurrentTime();
	while(!checkTimeout()) {
		if(serialDataAvail(serial) >= 6 && serialGetchar(serial) == 0xFF && serialGetchar(serial) == 0xFF) {
			uint8_t id, length, error, checksum, arg1, arg2;
			id = serialGetchar(serial);
			length = serialGetchar(serial);
			error = serialGetchar(serial);
			if(length > 2) {
				arg1 = serialGetchar(serial);
				// wait for one more byte
				while(!checkTimeout() && serialDataAvail(serial) < 1)
					waitForMicro(100);
			} else {
				arg1 = 0;
			}
			if(length > 3 && !checkTimeout()) {
				arg2 = serialGetchar(serial);
				// wait for one more byte
				while(!checkTimeout() && serialDataAvail(serial) < 1)
					waitForMicro(100);
			} else {
				arg2 = 0;
			}

			// make sure packet came back complete and without error
			if(checkTimeout())
				return -4;
			checksum = serialGetchar(serial);
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
	pthread_mutex_unlock(&serialLock);
}
static int axTransaction(uint8_t id, uint8_t instruction, uint8_t command, uint16_t arg,
	int argCount, uint16_t* result, uint8_t* error)
{
	int code = 0;

	pthread_mutex_lock(&serialLock); // only one transaction at a time
	for(int i=0; i<AX_SEND_RETRY+1; i++) {
		serialFlush(serial); // make sure there is no byte left in RX buffer
		if(axSendPacket(id, instruction, command, arg&0xFF, (arg >> 8)&0xFF, argCount)) {
			code = -1;
			break;
		}
		if(id == 0xFE) break; // no answer when broadcasting
		code = axReceiveAnswer(id, result, error);
		if(!code) break; // if everything went well, return, otherwise retry
	}

	// wait before releasing the lock to make sure AX12 has time to recover
	scheduleIn(15, releaseSerialLock);

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

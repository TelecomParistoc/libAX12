#include <stdint.h>

#ifndef AXCOMM_H
#define AXCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

// how many times we try again if an error is returned
#ifndef AX_SEND_RETRY
#define AX_SEND_RETRY 3
#endif

// how long we wait for an answer from AX12 (in ms)
#ifndef AX_MAX_ANSWER_WAIT
#define AX_MAX_ANSWER_WAIT 20
#endif


/* initialize serial port. Calling this is MANDATORY before any other operation */
int initAXcomm(int baudrate);

/* perform a transaction with an AX12 (send a packet and wait for the answer)
 * common arguments :
 *      id : the ID of the target AX12 (from 0 to 254), 254 is broadcast
 *      statusError : the byte containing the error flags
 * return codes :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms) */

/* write some data in AX12 memory, 1 byte for axWrite8, 2 byte for axWrite16
 * arguments :
 *      address : the start address to write to
 *      value : the value to write in the memory */
int axWrite8(uint8_t id, uint8_t address, uint8_t value, uint8_t* statusError);
int axWrite16(uint8_t id, uint8_t address, uint16_t value, uint8_t* statusError);
/* read some data in AX12 memory, 1 byte for axRead8, 2 byte for axRead16
 * arguments :
 *      address : the start address to read from
 *      value : the value we read */
int axRead8(uint8_t id, uint8_t address, uint8_t* value, uint8_t* statusError);
int axRead16(uint8_t id, uint8_t address, uint16_t* value, uint8_t* statusError);
/* ask for a status packet (returned in statusError) */
int axPing(uint8_t id, uint8_t* statusError);
/* reset AX12 memory to factory default. The communication may be broken as it
 * will set the baudrate to 1Mbps and ID to 1 */
int axFactoryReset(uint8_t id, uint8_t* statusError);

/* enable debug messafe print on communication error */
void enableErrorPrint(int enable);

#ifdef __cplusplus
}
#endif

#endif

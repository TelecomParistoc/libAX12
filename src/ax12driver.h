/*
 * this file provides control over the AX12 servos.
 * WARNING : it is required to call initAX12 before any operations
 */

#include <stdint.h>
#ifndef AX12_H
#define AX12_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initialize the AX12 driver.
 *
 *  Calling this is **MANDATORY** for proper use.
 *  @param baudrate
 *  @return Error code:
 *      - 0: Success.
 *      - -1: Cannot open  AX12 serial port.
 *      - -2: Cannot create mutex.
 */
int initAX12(int baudrate);

/** @brief Get AX12 position.
 *
 * Get AX12 position in degree from -150 to 150, increasing
 * clockwise (when looking at the front side of the AX12).
 * @param id The id of the AX-12 to talk to.
 * @return The position (between -150 and 150).
 */
double AX12getPosition(uint8_t id);

/** @brief Get the speed of the servomotor.
 *
 * Get current output speed in %, from -100 to 100, positive speed is clockwise.
 * Current speed may be inaccurate and should not be relied on.
 * @param id The id of the AX-12 to talk to.
 * @return The speed value (between -100 and 100).
 */
double AX12getSpeed(uint8_t id);

/** @brief Get the load of the servomotor.
 *
 * Get current output load in %, from -100 to 100, positive load means rotating
 * clockwise. Current load may be inaccurate and should not be relied on.
 * @param id The id of the AX-12 to talk to.
 * @return The load (between -100 and 100).
 */
double AX12getLoad(uint8_t id);

/** @brief Get current AX12 status (error flags).
 *
 * See datasheet for more info.
 * @param id The id of the AX-12 to talk to.
 * @return AX-12 status code.
 */
int AX12getStatus(uint8_t id);

/** @brief Get AX12 power voltage (in volts)
 *
 * @param id The id of the AX-12 to talk to.
 * @return The measured voltage.
 */
double AX12getVoltage(uint8_t id);

/** @brief Get AX12 temperature (in degrees celsius)
 *
 * @param id The id of the AX-12 to talk to.
 * @return The measured temperature.
 */
int AX12getTemperature(uint8_t id);

/** @brief Allow you to know if the AX-12 is moving.
 *
 * @return
 *  - 1 if AX12 is moving by its own power.
 *  - 0 otherwise.
 */
int AX12isMoving(uint8_t id);

// AX-12 MODES
/** @brief In this mode, you can set the target position.
 *
 * You can't do a full turn.
 */
#define DEFAULT_MODE 0

/** @brief In this mode, you can set the target speed.
 */
#define WHEEL_MODE 1

/** @brief Set AX12 control mode.
 *
 * Default mode allows position control, wheel mode allows endless turn.
 * @param id The id of the AX-12 to talk to.
 * @param mode The mode to set (DEFAULT_MODE or WHEEL_MODE).
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 */
int AX12setMode(uint8_t id, int mode);

/** @brief Set AX target speed.
 *
 * Set AX12 goal speed from -100 to 100, positive speed is clockwise (when
 * looking at the front side of the AX12).
 * @param id The id of the AX-12 to talk to.
 * @param speed The target speed wanted.
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 */
int AX12setSpeed(uint8_t id, double speed);

/** @brief Set AX12 max torque.
 *
 * Set AX12 max torque from 0 to 100. Setting torque to zero will disable torque
 * (preventing AX12 from moving), setting to any other value will enable torque.
 * @param id The id of the AX-12 to talk to.
 * @param torque The max torque wanted.
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 */
int AX12setTorque(uint8_t id, double torque);

/** @brief Set rear LED state
 *
 * @param id The id of the AX-12 to talk to.
 * @param state The desired state of the LED:
 * - 1 for ON
 * - 0 for OFF
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 */
int AX12setLED(uint8_t id, int state);

/** @Ask an AX12 to move to the specified position.
 *
 * Move to a given position in degree from -150 to 150, increasing clockwise
 * (when looking at the front side of the AX12). It will change mode to default
 * if necessary. A callback can be called when the AX12 stops moving (when it
 * reached the goal position or if an error occured).
 * @param id The id of the AX-12 to talk to.
 * @param position The position the AX-12 as to move to.
 * @param callback The function that will be called when the movement is finished.
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 *      -5 : callback buffer is full
 */
int AX12move(uint8_t id, double position, void (*callback)(void));

/** @brief Cancel an end move callback for a given AX12.
 *
 * Only the callback is cancelled, not the movement.
 * @param id The id of the AX-12 to talk to.
 */
void AX12cancelCallback(uint8_t id);

/** @brief Switch to wheel mode and set speed.
 *
 * @param id The id of the AX-12 to talk to.
 * @param speed The desired speed (from -100 to 100, positive is clockwise).
 * @return error code :
 *       0 : no communication error
 *      -1 : serial port not initialized
 *      -2 : wrong checksum
 *      -3 : target and answer ID mismatch
 *      -4 : timeout (no answer received after AX_MAX_ANSWER_WAIT ms)
 */
int AX12turn(uint8_t id, double speed);

/** @brief Reset all AX12 to a default config.
 *
 * Default config includes enable and set torque to maximum,speed at 50%, default mode).
 */
void AX12resetAll();

#ifdef __cplusplus
}
#endif

#endif

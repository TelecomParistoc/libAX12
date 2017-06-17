#ifndef AXCONSTANTS_H
#define AXCONSTANTS_H

// INSTRUCTIONS
#define AX_PING 0x01
#define AX_READ_DATA 0x02
#define AX_WRITE_DATA 0x03
#define AX_RESET 0x06

// commands (AX12 registers) : not exhaustive but sufficient in most cases
// EEPROM
#define AX_ID 0x03
#define AX_BAUDRATE 0x04
#define AX_DELAY 0x05
#define AX_CW_LIMIT 0x06
#define AX_CCW_LIMIT 0x08
#define AX_MAX_TEMP 0x0B
#define AX_MIN_VOLT 0x0C
#define AX_MAX_VOLT 0x0D
#define AX_EE_MAX_TORQUE 0x0E
#define AX_RETURN 0x10
#define AX_ALARM_LED 0x11
#define AX_ALARM_SHUTDOWN 0x12

//RAM
#define AX_TORQUE_ENABLE 0x18
#define AX_LED 0x19
#define AX_GOAL_POS 0x1E
#define AX_GOAL_SPEED 0x20
#define AX_MAX_TORQUE 0x22
#define AX_POS 0x24
#define AX_SPEED 0x26
#define AX_LOAD 0x28
#define AX_VOLT 0x2A
#define AX_TEMP 0x2B
#define AX_MOVING 0x2E

#endif

#ifndef SYSEX_H
#define SYSEX_H

#define SYSTEM_EXCLUSIVE        0xf0
#define END_OF_EXCLUSIVE        0xf7

#define MANUFACTURER_ID         0x60
#define DEVICE_ID               0x01

#define COMMAND_ALIVE           0x41
#define COMMAND_RESET           0x42
#define COMMAND_READ_VERSION    0x51
#define COMMAND_READ_SETTINGS   0x52
#define COMMAND_READ_PRESET     0x53
#define COMMAND_WRITE_SETTINGS  0x61
#define COMMAND_WRITE_PRESET    0x62

#define RESPONSE_SUCCESS        0x71
#define RESPONSE_ERROR          0x72

#endif

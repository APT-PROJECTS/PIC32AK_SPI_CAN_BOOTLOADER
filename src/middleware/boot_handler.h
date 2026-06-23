#ifndef _BOOT_HANDLER_NAME_H 
#define _BOOT_HANDLER_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================
 * CAN IDs REQUEST (SCU → MCU)
 * ===================================================== */
typedef enum {
  CANID_REQ_PICID_1 =                         0xB0,     // Request PICID UDID0 UDID1
  CANID_REQ_PICID_2 =                         0xB2,     // Request PICID UDID2 UDID3
  CANID_REQ_SOFTWARE_ID =                     0xB4,     // Request software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
  CANID_SCU_REQ =                             0xB6,     // SCU request id
  CANID_REQ_END =                             0xB8      // Update software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
} CANID_REQ_t;

/* =====================================================
 * CAN IDs REQUEST (MCU → SCU)
 * ===================================================== */
typedef enum {
  CANID_RESP_PICID_1 =                        0xB1,   // Response PICID UDID0 UDID1
  CANID_RESP_PICID_2 =                        0xB3,   // Response PICID UDID2 UDID3S
  CANID_RESP_SOFTWARE_ID =                    0xB5,   // Response software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
  CANID_MCU_RESP =                            0xB7,   // MCU response id
  CANID_RESP_END =                            0xB9    // Software id updated successfully
} CANID_RESP_t;

/* =====================================================
 * COMMANDS REQUEST (SCU → MCU)
 * ===================================================== */
typedef enum {
  CMD_REQ_STATUS =                            0x54,       // Request MCU status
  CMD_REQ_BOOTLOADER =                        0x51,       // Jump to bootloader
  CMD_REQ_START =                             0x53,       // Start OTA
  CMD_REQ_DATA =                              0x52,       // Firmware chunk
  CMD_REQ_APP_JUMP =                          0x56        // Jump to application
} CMD_REQ_t;

/* =====================================================
 * COMMAND RESPONSE (MCU → SCU)
 * ===================================================== */
typedef enum {
  CMD_RESP_BOOTLOADER_MODE =                  0x58,     // MCU in bootloader mode
  CMD_RESP_APPLICATION_MODE =                 0x57,     // MCU in application mode
  CMD_RESP_UNDEFINED_MODE =                   0x59,     // MCU in undefined mode
  CMD_RESP_JUMP_OK =                          0x1E,     // MCU jumped in bootloader mode
  CMD_RESP_FLASH_READY =                      0x2E      // Flash memory erased and ready to receive data
} CMD_RESP_t;



#ifdef __cplusplus
}
#endif

#endif 
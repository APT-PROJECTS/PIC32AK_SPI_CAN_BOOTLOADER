#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <stdint.h>
#include <SPIFFS.h>

#define CAN_RX_TIMEOUT 3000
#define OTA_FLASH_CHUNK_SIZE 4096U
#define OTA_FRAME_DATA       7U  // 1 byte CMD + 7 byte payload

// ---------------- API BASE ----------------
#define OTA_VERSION_URL "https://ota.aproposdrive.com/api/firmware/latest-version"
#define OTA_FIRMWARE_URL "https://ota.aproposdrive.com/api/firmware/latest-firmware"
#define OTA_REPORT_URL "https://ota.aproposdrive.com/api/ota-updates/report"

/* =====================================================
 * CAN_ID
 * ===================================================== */
typedef enum {
  OTA_CAN_ID_HOST = 0xB0U,
  OTA_CAN_ID_CLIENT = 0xB1U
} CAN_ID_t;

/* =====================================================
 * CAN_CMD
 * ===================================================== */
typedef enum {
  OTA_CAN_CMD_2_READ = 0x01,
  OTA_CAN_CMD_2_WRITE = 0x02,
  OTA_CAN_CMD_2_COMPARE = 0x03,

  OTA_CAN_CMD_3_SUCCESS = 0x01,
  OTA_CAN_CMD_3_FAIL = 0x02,

  OTA_CAN_CMD_0_PIC_ID = 0x01U,
  OTA_CAN_CMD_1_PIC_ID_1 = 0x01U,
  OTA_CAN_CMD_1_PIC_ID_2 = 0x02U,
  OTA_CAN_CMD_1_PIC_ID_3 = 0x03U,
  OTA_CAN_CMD_1_PIC_ID_4 = 0x04U,

  OTA_CAN_CMD_0_SOFTWARE_ID = 0x02U,
  OTA_CAN_CMD_1_PROJECT_ID = 0x01U,
  OTA_CAN_CMD_1_FIRMWARE_ID = 0x02U,

  OTA_CAN_CMD_0_STATUS = 0x03U,
  OTA_CAN_CMD_1_STATUS_BOOT = 0x01U,
  OTA_CAN_CMD_1_STATUS_APP = 0x02U,
  OTA_CAN_CMD_1_STATUS_UNKNOWN = 0x03U,

  OTA_CAN_CMD_0_APP_SIZE = 0x04U,

  OTA_CAN_CMD_0_APP_CHUNK_TEST = 0x05U,
  OTA_CAN_CMD_1_APP_CHUNK_1_TEST = 0x01U,
  OTA_CAN_CMD_1_APP_CHUNK_2_TEST = 0x02U,
  OTA_CAN_CMD_1_APP_CHUNK_3_TEST = 0x03U,
  OTA_CAN_CMD_1_APP_CHUNK_4_TEST = 0x04U,
  OTA_CAN_CMD_1_APP_CHUNK_5_TEST = 0x05U,
  OTA_CAN_CMD_1_APP_CHUNK_6_TEST = 0x06U,
  OTA_CAN_CMD_1_APP_CHUNK_7_TEST = 0x07U,
  OTA_CAN_CMD_1_APP_CHUNK_8_TEST = 0x08U,

  OTA_CAN_CMD_0_JUMP = 0x06U,
  OTA_CAN_CMD_1_BOOT_JUMP = 0x01U,
  OTA_CAN_CMD_1_APP_JUMP = 0x02U,

  OTA_CAN_CMD_0_APP_ERASE = 0x07U,

  OTA_CAN_CMD_0_APP_CHUNK = 0x08U,

  OTA_CAN_CMD_0_APP_CRC = 0x09U,
} CAN_CMD_t;


/* =====================================================
 * CAN IDs REQUEST (SCU → MCU)
 * ===================================================== */
typedef enum {
  CANID_REQ_PICID_1 =                         0xB0,     // Request PICID DEVSN0 DEVSN1
  CANID_REQ_PICID_2 =                         0xB2,     // Request PICID DEVSN2 DEVSN3
  CANID_REQ_SOFTWARE_ID =                     0xB4,     // Request software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
  CANID_SCU_REQ =                             0xB6,     // SCU request id
  CANID_REQ_END =                             0xB8      // Update software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
} CANID_REQ_t;

/* =====================================================
 * CAN IDs REQUEST (MCU → SCU)
 * ===================================================== */
typedef enum {
  CANID_RESP_PICID_1 =                        0xB1,   // Response PICID DEVSN0 DEVSN1
  CANID_RESP_PICID_2 =                        0xB3,   // Response PICID DEVSN2 DEVSN3
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


typedef enum {
  MCU_MODE_UNDEFINED = 0,
  MCU_MODE_APPLICATION,
  MCU_MODE_BOOTLOADER
} MCU_MODE_t;

typedef enum {
  ALREADY_UPDATED =               0,
  TWO_STEP_SUCCESS =              1,
  THREE_STEP_SUCCESS =            2,
  APPLICATION_ID_MISMATCH =       -1,
  OEM_ID_MISMATCH =               -2,
  FAILED_TO_GET_PICID =           -3,
  FAILED_TO_GET_SOFTWARE_ID =     -4,
  APP_SIZE_EXCEED =               -11,
  BOOT_JUMP_FAILED =              -5,
  FAILED_TO_GET_STATUS =          -6,
  MCU_MEMORY_ERASE_FAILED =       -7,
  FIRMWARE_UPLOAD_FAILED =        -8,
  SOFTWARE_ID_UPDATE_FAILED =     -9,
  APPLICATION_JUMP_FAILED =       -10
} OTA_REPORT_t;

/* =====================================================
 * OTA CORE0 HANDLER
 * ===================================================== */
typedef enum {
  OTA_CORE0_IDLE = 0,
  OTA_CORE0_CHECK_SERVER_VERSION,
  OTA_CORE0_DOWNLOAD_BIN_FILE,
  OTA_CORE0_REPORT,
  OTA_CORE0_DONE,
  OTA_CORE0_ERROR
} OTA_CORE0_STATE_t;

/* =====================================================
 * OTA CORE1 HANDLER
 * ===================================================== */
typedef enum {
  OTA_CORE1_IDLE = 0,
  OTA_CORE1_READ_PIC_ID_1,
  OTA_CORE1_READ_PIC_ID_2,
  OTA_CORE1_READ_PIC_ID_3,
  OTA_CORE1_READ_PIC_ID_4,
  OTA_CORE1_READ_PROJECT_ID,
  OTA_CORE1_READ_FIRMWARE_ID,
  OTA_CORE1_COMPARE_APP_SIZE,
  OTA_CORE1_SEND_BOOT_JMP_REQ,
  OTA_CORE1_SEND_STATUS_REQ, 
  OTA_CORE1_SEND_START_REQ,    
  OTA_CORE1_SEND_DATA,  
  OTA_CORE1_SEND_END_REQ,
  OTA_CORE1_APP_JMP_REQ,
  OTA_CORE1_CAN_RX_HANDLE,
  OTA_CORE1_DONE,
  OTA_CORE1_ERROR
} OTA_CORE1_State_t;


void OTA_Handler_CORE0(const uint8_t *mac);
bool OTA_Handler_CORE1(void);
void CORE0_OTA_Task(void *p);

#endif  // API_HANDLER_H
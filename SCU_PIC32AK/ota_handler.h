#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <stdint.h>
#include <SPIFFS.h>

#define CAN_RX_TIMEOUT 3000
#define OTA_APP_CHUNK_SIZE  1024U
#define OTA_FRAME_DATA       4U  // CAN bytes 4..7; bytes 0..3 are protocol

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


typedef enum {
  CANID_REQ_END =                             0xB8      // Update software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
} CANID_REQ_t;

typedef enum {
  CANID_RESP_END =                            0xB9    // Software id updated successfully
} CANID_RESP_t;

typedef enum {
  ALREADY_UPDATED =               0,
  TWO_STEP_SUCCESS =              1,
  THREE_STEP_SUCCESS =            2,
  APPLICATION_ID_MISMATCH =       -1,
  OEM_ID_MISMATCH =               -2,
  FAILED_TO_GET_PICID =           -3,
  FAILED_TO_GET_SOFTWARE_ID =     -4,
  APP_SIZE_EXCEED =               -11,
  BOOT_JUMP_FAIL =                -5,
  APP_ERASE_FAIL =                -7,
  FIRMWARE_UPLOAD_FAILED =        -8,
  SOFTWARE_ID_UPDATE_FAILED =     -9,
  APP_JUMP_FAIL =                 -10
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
  OTA_CORE1_BOOT_JUMP,
  OTA_CORE1_APP_ERASE,    
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

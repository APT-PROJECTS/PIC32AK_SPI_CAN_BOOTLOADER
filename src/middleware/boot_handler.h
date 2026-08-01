#ifndef _BOOT_HANDLER_NAME_H 
#define _BOOT_HANDLER_NAME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif    
    
/* =====================================================
 * FLASH MEMORY MAP
 * =====================================================*/
#define BOOTLOADER_START_ADDRESS        (0x800000UL)
#define BOOTLOADER_END_ADDRESS          (0x807FFFUL)
    
#define APPLICATION_START_ADDRESS       (0x808000UL)
#define APPLICATION_END_ADDRESS         (0x80EFFFUL)

#define EEPROM_START_ADDRESS            (0x80F000UL)
#define EEPROM_END_ADDRESS              (0x80FFFFUL)

#define APPLICATION_IVT_ADDRESS         (APPLICATION_START_ADDRESS + 0x40U)
    
/* =====================================================
 * CAN_ID
 * ===================================================== */
typedef enum {
  BOOT_CAN_ID_HOST = 0xB0U,
  BOOT_CAN_ID_CLIENT = 0xB1U
} CAN_ID_t;

/* =====================================================
 * CAN_CMD
 * ===================================================== */
typedef enum {
  BOOT_CAN_CMD_2_READ = 0x01,
  BOOT_CAN_CMD_2_WRITE = 0x02,
  BOOT_CAN_CMD_2_COMPARE = 0x03,

  BOOT_CAN_CMD_3_SUCCESS = 0x01,
  BOOT_CAN_CMD_3_FAIL = 0x02,

  BOOT_CAN_CMD_0_PIC_ID = 0x01U,
  BOOT_CAN_CMD_1_PIC_ID_1 = 0x01U,
  BOOT_CAN_CMD_1_PIC_ID_2 = 0x02U,
  BOOT_CAN_CMD_1_PIC_ID_3 = 0x03U,
  BOOT_CAN_CMD_1_PIC_ID_4 = 0x04U,

  BOOT_CAN_CMD_0_SOFTWARE_ID = 0x02U,
  BOOT_CAN_CMD_1_PROJECT_ID = 0x01U,
  BOOT_CAN_CMD_1_FIRMWARE_ID = 0x02U,

  BOOT_CAN_CMD_0_STATUS = 0x03U,
  BOOT_CAN_CMD_1_STATUS_BOOT = 0x01U,
  BOOT_CAN_CMD_1_STATUS_APP = 0x02U,
  BOOT_CAN_CMD_1_STATUS_UNKNOWN = 0x03U,

  BOOT_CAN_CMD_0_APP_SIZE = 0x04U,

  BOOT_CAN_CMD_0_APP_CHUNK_TEST = 0x05U,
  BOOT_CAN_CMD_1_APP_CHUNK_1_TEST = 0x01U,
  BOOT_CAN_CMD_1_APP_CHUNK_2_TEST = 0x02U,
  BOOT_CAN_CMD_1_APP_CHUNK_3_TEST = 0x03U,
  BOOT_CAN_CMD_1_APP_CHUNK_4_TEST = 0x04U,
  BOOT_CAN_CMD_1_APP_CHUNK_5_TEST = 0x05U,
  BOOT_CAN_CMD_1_APP_CHUNK_6_TEST = 0x06U,
  BOOT_CAN_CMD_1_APP_CHUNK_7_TEST = 0x07U,
  BOOT_CAN_CMD_1_APP_CHUNK_8_TEST = 0x08U,

  BOOT_CAN_CMD_0_JUMP = 0x06U,
  BOOT_CAN_CMD_1_BOOT_JUMP = 0x01U,
  BOOT_CAN_CMD_1_APP_JUMP = 0x02U,

  BOOT_CAN_CMD_0_APP_ERASE = 0x07U,

  BOOT_CAN_CMD_0_APP_CHUNK = 0x08U,

  BOOT_CAN_CMD_0_APP_CRC = 0x09U,
} CAN_CMD_t;


/* =====================================================
 * CAN IDs REQUEST (SCU ? MCU)
 * ===================================================== */
typedef enum {
  CANID_REQ_PICID_1 =                         0xB0,     // Request PICID UDID0 UDID1
  CANID_REQ_PICID_2 =                         0xB2,     // Request PICID UDID2 UDID3
  CANID_REQ_SOFTWARE_ID =                     0xB4,     // Request software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
  CANID_SCU_REQ =                             0xB6,     // SCU request id
  CANID_REQ_END =                             0xB8      // Update software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
} CANID_REQ_t;

/* =====================================================
 * CAN IDs RESPONSE (MCU ? SCU)
 * ===================================================== */
typedef enum {
  CANID_RESP_PICID_1 =                        0xB1,   // Response PICID UDID0 UDID1
  CANID_RESP_PICID_2 =                        0xB3,   // Response PICID UDID2 UDID3S
  CANID_RESP_SOFTWARE_ID =                    0xB5,   // Response software id(8) = {project id(4) = oem id(2) + app id(2)} + {firmware id(4) = major(2) + minor(2)}
  CANID_MCU_RESP =                            0xB7,   // MCU response id
  CANID_RESP_END =                            0xB9    // Software id updated successfully
} CANID_RESP_t;

/* =====================================================
 * COMMANDS REQUEST (SCU ? MCU)
 * ===================================================== */
typedef enum {
  CMD_REQ_STATUS =                            0x54,       // Request MCU status
  CMD_REQ_BOOTLOADER =                        0x51,       // Jump to bootloader
  CMD_REQ_START =                             0x53,       // Start OTA
  CMD_REQ_DATA =                              0x52,       // Firmware chunk
  CMD_REQ_APP_JUMP =                          0x56        // Jump to application
} CMD_REQ_t;

/* =====================================================
 * COMMAND RESPONSE (MCU ? SCU)
 * ===================================================== */
typedef enum {
  CMD_RESP_BOOTLOADER_MODE =                  0x58,     // MCU in bootloader mode
  CMD_RESP_APPLICATION_MODE =                 0x57,     // MCU in application mode
  CMD_RESP_UNDEFINED_MODE =                   0x59,     // MCU in undefined mode
  CMD_RESP_JUMP_OK =                          0x1E,     // MCU jumped in bootloader mode
  CMD_RESP_FLASH_READY =                      0x2E      // Flash memory erased and ready to receive data
} CMD_RESP_t;

/* =====================================================
 * API
 * ===================================================== */
void Boot_PIC_ID_Handler(void);
void Boot_Software_ID_Handler(void);
void Boot_Status_Handler(void);
void Boot_App_Size_Handler(void);
bool Boot_Is_App_Present(void);
void Boot_App_Jump(void);
void Boot_Jump_Handler(void);
bool App_Erase(void);
bool App_Erase_Verify(void);
void App_Erase_Success(void);
static bool App_Firmware_Write_Flash(void);
void App_Firmware_Write_Buffer(uint8_t *rxBuffer);
void Run_Boot_Handler(uint32_t ID, uint8_t *rxBuffer);


#ifdef __cplusplus
}
#endif

#endif 
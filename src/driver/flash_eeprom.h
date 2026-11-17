#ifndef FLASH_EEPROM_H
#define FLASH_EEPROM_H

#include "definitions.h"

#define FLASH_PAGE_SIZE       4096UL
#define EEPROM_HEADER_SIZE    32UL
#define EEPROM_DATA_SIZE      (FLASH_PAGE_SIZE - EEPROM_HEADER_SIZE)
#define EEPROM_NUM_PAGES      1

// For PIC32AK6416GC41064, reserve the last 4 KB flash page.
//#define EEPROM_PAGE1_ADDR     0x80F000      // 64KB
#define EEPROM_PAGE1_ADDR     0x81F000        // 128KB

#define EEPROM_VALID_MARKER   0xFFFFFFFE

typedef enum
{
    FLASH_EEPROM_OK = 0,
    FLASH_EEPROM_ERROR,
    FLASH_EEPROM_CRC_ERROR,
    FLASH_EEPROM_NO_VALID_PAGE
} FLASH_EEPROM_STATUS;

extern uint8_t eepromBuffer[EEPROM_DATA_SIZE];


FLASH_EEPROM_STATUS Flash_EEPROM_Init(void);
FLASH_EEPROM_STATUS Flash_EEPROM_Write_Page(const uint8_t *data, uint32_t length);
FLASH_EEPROM_STATUS Flash_EEPROM_Read_Page(uint8_t *data, uint32_t length);
FLASH_EEPROM_STATUS Flash_EEPROM_Write_Word(uint32_t address, uint32_t data,
                                            uint8_t *buffer);
FLASH_EEPROM_STATUS Flash_EEPROM_Read_Word(uint32_t address, uint32_t *data);
FLASH_EEPROM_STATUS Flash_EEPROM_Erase_All(void);

#endif

#ifndef _DEVICE_STARTUP_H    /* Guard against multiple inclusion */
#define _DEVICE_STARTUP_H


#ifdef __cplusplus
extern "C" {
#endif
    
#include "driver.h"
    
//--------------------------------------------------
// MAC ID PIC32AK
//--------------------------------------------------
#define UDID0_ADDR  ((volatile uint32_t *)0x7F2BE0)
#define UDID1_ADDR  ((volatile uint32_t *)0x7F2BE4)
#define UDID2_ADDR  ((volatile uint32_t *)0x7F2BE8)
#define UDID3_ADDR  ((volatile uint32_t *)0x7F2BEC)
    
//--------------------------------------------------
// DEVICE ID PIC32AK
//--------------------------------------------------
#define DEVID_ADDR  ((volatile uint32_t *)0x7C2000UL)
    
//--------------------------------------------------
// MATRIX SIZE
//--------------------------------------------------
#define ROWS    16
#define COLS    5
    
//==================================================
// ROW 0
//==================================================
#define R_PROJECT_ID                 readMatrix[0][0]
#define R_FIRMWARE_ID                readMatrix[0][1]
#define R_BOOT_FLAG                  readMatrix[0][2]
#define R_BOOT_VERSION               readMatrix[0][3]
#define R_APP_SIZE                   readMatrix[0][4]

#define W_PROJECT_ID                 writeMatrix[0][0]
#define W_FIRMWARE_ID                writeMatrix[0][1]
#define W_BOOT_FLAG                  writeMatrix[0][2]
#define W_BOOT_VERSION               writeMatrix[0][3]
#define W_APP_SIZE                   writeMatrix[0][4]

    
extern uint32_t udid[4];
extern uint32_t devid;

//--------------------------------------------------
// EEPROM BUFFER
//--------------------------------------------------
extern uint8_t eepromBuffer[EEPROM_DATA_SIZE];

//--------------------------------------------------
// EXTERNAL MATRIX
//--------------------------------------------------
extern float readMatrix[ROWS][COLS];
extern float writeMatrix[ROWS][COLS];

    
void Get_PIC_ID(void);
void NVM_EEPROM_Initialize(void);
void Load_Matrix(void);

#ifdef __cplusplus
}
#endif

#endif /* _DEVICE_STARTUP_H */


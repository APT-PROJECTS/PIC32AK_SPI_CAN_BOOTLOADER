#ifndef _METADATA_H    
#define _METADATA_H 


#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"
    
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
// METADATA FIELD LENGTH
//--------------------------------------------------
#define BRAND_NAME                      3U
#define PHASES                          1U
#define SWITCHES_IN_PARALLEL            1U
#define SWITCH_TYPE                     1U
#define PCB_TYPE                        1U

#define PART_NUMBER_LENGTH              (BRAND_NAME + PHASES + \
                                        SWITCHES_IN_PARALLEL + \
                                        SWITCH_TYPE + PCB_TYPE)

#define SENSOR                          1U
#define NTC_RESISTOR                    3U
#define NTC_VOLT                        1U
#define NTC_VALUE_B                     2U
#define CONNECTOR                       2U
#define COMS                            2U

#define MODEL_NUMBER_LENGTH             (SENSOR + NTC_RESISTOR + \
                                        NTC_VOLT + NTC_VALUE_B + \
                                        CONNECTOR + COMS)

#define EMS                             2U
#define YEAR_MONTH                      4U
#define BATCH_NUMBER                    4U
#define SEQUENCE_NUMBER                 6U

#define SERIAL_NUMBER_LENGTH            (EMS + YEAR_MONTH + \
                                        BATCH_NUMBER + SEQUENCE_NUMBER)

#define OEM_ID                          2U
#define PROJECT_ID                      2U
#define APP_VERSION_L                   1U
#define APP_VERSION_M                   2U
#define APP_VERSION_R                   2U
#define APP_SIZE                        4U
#define BOOT_VERSION_L                  1U
#define BOOT_VERSION_M                  2U
#define BOOT_VERSION_R                  2U
#define STAY_IN_BOOT_FLAG               1U

#define SOFTWARE_ID_LENGTH              (OEM_ID + PROJECT_ID + \
                                        APP_VERSION_L + APP_VERSION_M + \
                                        APP_VERSION_R + APP_SIZE + \
                                        BOOT_VERSION_L + BOOT_VERSION_M + \
                                        BOOT_VERSION_R + STAY_IN_BOOT_FLAG)

#define MCU_ID_LENGTH                   (PART_NUMBER_LENGTH + \
                                        MODEL_NUMBER_LENGTH + \
                                        SERIAL_NUMBER_LENGTH)

#define FIRMWARE_ID_LENGTH              (PART_NUMBER_LENGTH + \
                                        MODEL_NUMBER_LENGTH + \
                                        SOFTWARE_ID_LENGTH)

#define METADATA_LENGTH                 (MCU_ID_LENGTH + FIRMWARE_ID_LENGTH)


//--------------------------------------------------
// METADATA FIELD OFFSET
//--------------------------------------------------
#define MCU_ID_OFFSET                   0U
#define FIRMWARE_ID_OFFSET              (MCU_ID_LENGTH)

#define APP_SIZE_OFFSET                 (FIRMWARE_ID_OFFSET + \
                                         PART_NUMBER_LENGTH + \
                                         MODEL_NUMBER_LENGTH + \
                                         OEM_ID + PROJECT_ID + \
                                         APP_VERSION_L + APP_VERSION_M + \
                                         APP_VERSION_R)

#define BOOT_VERSION_OFFSET             (APP_SIZE_OFFSET + APP_SIZE)
    
    
//--------------------------------------------------
// EEPROM FIELD OFFSET
//-------------------------------------------------- 
#define EEPROM_METADATA_OFFSET          0UL
#define EEPROM_METADATA_SIZE            (EEPROM_DATA_SIZE / 2UL)


//--------------------------------------------------
// EXTERN VARIABLES
//--------------------------------------------------
extern uint32_t udid[4];
extern uint32_t devid;

extern char appSize[APP_SIZE + 1U];
extern char stayInBootFlag[STAY_IN_BOOT_FLAG + 1U];

extern char metadata[METADATA_LENGTH + 1U];  


//--------------------------------------------------
// API
//--------------------------------------------------
void Get_PIC_ID(void);
void Get_Part_Number(void);
void Get_Model_Number(void);
void Get_Software_ID(void);
void Get_Firmware_ID(void);
void Build_Metadata(void);
void Metadata_Init(void);


#ifdef __cplusplus
}
#endif

#endif /* _METADATA_H */

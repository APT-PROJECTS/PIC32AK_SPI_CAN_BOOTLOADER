#include "definitions.h"
#include "driver.h"
#include "middleware.h"

uint32_t udid[4];
uint32_t devid;

/* Part Number */
char brandName[BRAND_NAME + 1U] = "***";
char phases[PHASES + 1U] = "*";
char switchesInParallel[SWITCHES_IN_PARALLEL + 1U] = "*";
char switchType[SWITCH_TYPE + 1U] = "*";
char pcbType[PCB_TYPE + 1U] = "*";

char partNumber[PART_NUMBER_LENGTH + 1U];

/* Model Number */
char sensor[SENSOR + 1U] = "*";
char ntcResistor[NTC_RESISTOR + 1U] = "***";
char ntcVolt[NTC_VOLT + 1U] = "*";
char ntcValueB[NTC_VALUE_B + 1U] = "**";
char connector[CONNECTOR + 1U] = "**";
char coms[COMS + 1U] = "**";

char modelNumber[MODEL_NUMBER_LENGTH + 1U];

/* Software ID */
char oemId[OEM_ID + 1U] = "**";
char projectId[PROJECT_ID + 1U] = "**";
char appVersionL[APP_VERSION_L + 1U] = "*";
char appVersionM[APP_VERSION_M + 1U] = "**";
char appVersionR[APP_VERSION_R + 1U] = "**";
char appSize[APP_SIZE + 1U] = "****";
char bootVersionL[BOOT_VERSION_L + 1U] = "1";
char bootVersionM[BOOT_VERSION_M + 1U] = "00";
char bootVersionR[BOOT_VERSION_R + 1U] = "01";
char stayInBootFlag[STAY_IN_BOOT_FLAG + 1U] = "0";

char softwareId[SOFTWARE_ID_LENGTH + 1U];

char mcuId[MCU_ID_LENGTH + 1U];
char firmwareId[FIRMWARE_ID_LENGTH + 1U];

char metadata[METADATA_LENGTH + 1U];

void Get_PIC_ID(void) {
    udid[0] = *UDID0_ADDR;
    udid[1] = *UDID1_ADDR;
    udid[2] = *UDID2_ADDR;
    udid[3] = *UDID3_ADDR;

    devid = (*DEVID_ADDR) & 0x0FFFFFFFUL;
}

void Get_Part_Number(void) {
    __builtin_memset(partNumber, 0, sizeof (partNumber));

    __builtin_memcpy(&partNumber[0],
            brandName,
            BRAND_NAME);

    __builtin_memcpy(&partNumber[BRAND_NAME],
            phases,
            PHASES);

    __builtin_memcpy(&partNumber[BRAND_NAME + PHASES],
            switchesInParallel,
            SWITCHES_IN_PARALLEL);

    __builtin_memcpy(&partNumber[BRAND_NAME + PHASES + SWITCHES_IN_PARALLEL],
            switchType,
            SWITCH_TYPE);

    __builtin_memcpy(&partNumber[BRAND_NAME + PHASES +
            SWITCHES_IN_PARALLEL + SWITCH_TYPE],
            pcbType,
            PCB_TYPE);

    partNumber[PART_NUMBER_LENGTH] = '\0';
}

void Get_Model_Number(void) {
    __builtin_memset(modelNumber, 0, sizeof (modelNumber));

    __builtin_memcpy(&modelNumber[0], sensor, SENSOR);

    __builtin_memcpy(&modelNumber[SENSOR],
            ntcResistor, NTC_RESISTOR);

    __builtin_memcpy(&modelNumber[SENSOR + NTC_RESISTOR],
            ntcVolt, NTC_VOLT);

    __builtin_memcpy(&modelNumber[SENSOR + NTC_RESISTOR + NTC_VOLT],
            ntcValueB, NTC_VALUE_B);

    __builtin_memcpy(&modelNumber[SENSOR + NTC_RESISTOR + NTC_VOLT +
            NTC_VALUE_B],
            connector, CONNECTOR);

    __builtin_memcpy(&modelNumber[SENSOR + NTC_RESISTOR + NTC_VOLT +
            NTC_VALUE_B + CONNECTOR],
            coms, COMS);

    modelNumber[MODEL_NUMBER_LENGTH] = '\0';
}

void Get_Software_ID(void) {
    __builtin_memset(softwareId, 0, sizeof (softwareId));

    __builtin_memcpy(&softwareId[0],
            oemId, OEM_ID);

    __builtin_memcpy(&softwareId[OEM_ID],
            projectId, PROJECT_ID);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID],
            appVersionL, APP_VERSION_L);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L],
            appVersionM, APP_VERSION_M);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L +
            APP_VERSION_M],
            appVersionR, APP_VERSION_R);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L +
            APP_VERSION_M + APP_VERSION_R],
            appSize, APP_SIZE);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L +
            APP_VERSION_M + APP_VERSION_R + APP_SIZE],
            bootVersionL, BOOT_VERSION_L);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L +
            APP_VERSION_M + APP_VERSION_R + APP_SIZE +
            BOOT_VERSION_L],
            bootVersionM, BOOT_VERSION_M);

    __builtin_memcpy(&softwareId[OEM_ID + PROJECT_ID + APP_VERSION_L +
            APP_VERSION_M + APP_VERSION_R + APP_SIZE +
            BOOT_VERSION_L + BOOT_VERSION_M],
            bootVersionR, BOOT_VERSION_R);

    __builtin_memcpy(&softwareId[SOFTWARE_ID_LENGTH - STAY_IN_BOOT_FLAG],
            stayInBootFlag, STAY_IN_BOOT_FLAG);

    softwareId[SOFTWARE_ID_LENGTH] = '\0';
}

void Get_Firmware_ID(void) {
    __builtin_memset(firmwareId, 0, sizeof (firmwareId));

    __builtin_memcpy(&firmwareId[0],
            partNumber, PART_NUMBER_LENGTH);

    __builtin_memcpy(&firmwareId[PART_NUMBER_LENGTH],
            modelNumber, MODEL_NUMBER_LENGTH);

    __builtin_memcpy(&firmwareId[PART_NUMBER_LENGTH + MODEL_NUMBER_LENGTH],
            softwareId, SOFTWARE_ID_LENGTH);

    firmwareId[FIRMWARE_ID_LENGTH] = '\0';
}

void Build_Metadata(void) {
    __builtin_memset(metadata, 0, sizeof (metadata));

    __builtin_memcpy(&metadata[0],
            mcuId, MCU_ID_LENGTH);

    __builtin_memcpy(&metadata[MCU_ID_LENGTH],
            firmwareId, FIRMWARE_ID_LENGTH);

    metadata[METADATA_LENGTH] = '\0';
}

void Metadata_Init(void) {
    bool mcuIdValid = true;
    bool firmwareIdValid = true;
    bool metadataChanged = false;
    bool allZero;
    bool allFF;
    bool allStars = true;
    size_t i;
    uint16_t firmwareOffset;
    char storedFirmwareId[FIRMWARE_ID_LENGTH];

    /* Step 1: Read PIC ID. */
    Get_PIC_ID();

    /* Step 2: Read stored metadata from the EEPROM buffer. */
    __builtin_memcpy(metadata,
            &eepromBuffer[EEPROM_METADATA_OFFSET],
            METADATA_LENGTH);

    metadata[METADATA_LENGTH] = '\0';
    __builtin_memcpy(storedFirmwareId,
            &metadata[FIRMWARE_ID_OFFSET],
            FIRMWARE_ID_LENGTH);

    /* Step 3: Validate stored MCU ID: all 0x00, 0xFF, or '*' is invalid. */
    allZero = true;
    allFF = true;

    for (i = 0U; i < MCU_ID_LENGTH; i++) {
        uint8_t value = (uint8_t) metadata[MCU_ID_OFFSET + i];

        if (value != 0x00U) {
            allZero = false;
        }

        if (value != 0xFFU) {
            allFF = false;
        }

        if (value != (uint8_t) '*') {
            allStars = false;
        }
    }

    if (allZero || allFF || allStars) {
        mcuIdValid = false;
    }

    if (mcuIdValid) {
        /* Preserve the complete stored MCU ID. */
        __builtin_memcpy(mcuId,
                &metadata[MCU_ID_OFFSET],
                MCU_ID_LENGTH);

        mcuId[MCU_ID_LENGTH] = '\0';
    } else {
        __builtin_memset(mcuId, '*', MCU_ID_LENGTH);
        mcuId[MCU_ID_LENGTH] = '\0';
    }

    /* Step 4: Validate stored Firmware ID: all 0x00 or all 0xFF is invalid. */
    allZero = true;
    allFF = true;

    for (i = 0U; i < FIRMWARE_ID_LENGTH; i++) {
        uint8_t value = (uint8_t) storedFirmwareId[i];

        if (value != 0x00U) {
            allZero = false;
        }

        if (value != 0xFFU) {
            allFF = false;
        }
    }

    if (allZero || allFF) {
        firmwareIdValid = false;
    }

    if (!firmwareIdValid) {
        Get_Part_Number();
        Get_Model_Number();
        Get_Software_ID();
        Get_Firmware_ID();
        Build_Metadata();
        metadataChanged = true;
    } else {
        /* Restore stored fields. Do not restore bootloader version fields. */
        firmwareOffset = 0U;

        __builtin_memcpy(brandName, &storedFirmwareId[firmwareOffset], BRAND_NAME);
        firmwareOffset += BRAND_NAME;

        __builtin_memcpy(phases, &storedFirmwareId[firmwareOffset], PHASES);
        firmwareOffset += PHASES;

        __builtin_memcpy(switchesInParallel, &storedFirmwareId[firmwareOffset],
                SWITCHES_IN_PARALLEL);
        firmwareOffset += SWITCHES_IN_PARALLEL;

        __builtin_memcpy(switchType, &storedFirmwareId[firmwareOffset], SWITCH_TYPE);
        firmwareOffset += SWITCH_TYPE;

        __builtin_memcpy(pcbType, &storedFirmwareId[firmwareOffset], PCB_TYPE);
        firmwareOffset += PCB_TYPE;

        __builtin_memcpy(sensor, &storedFirmwareId[firmwareOffset], SENSOR);
        firmwareOffset += SENSOR;

        __builtin_memcpy(ntcResistor, &storedFirmwareId[firmwareOffset], NTC_RESISTOR);
        firmwareOffset += NTC_RESISTOR;

        __builtin_memcpy(ntcVolt, &storedFirmwareId[firmwareOffset], NTC_VOLT);
        firmwareOffset += NTC_VOLT;

        __builtin_memcpy(ntcValueB, &storedFirmwareId[firmwareOffset], NTC_VALUE_B);
        firmwareOffset += NTC_VALUE_B;

        __builtin_memcpy(connector, &storedFirmwareId[firmwareOffset], CONNECTOR);
        firmwareOffset += CONNECTOR;

        __builtin_memcpy(coms, &storedFirmwareId[firmwareOffset], COMS);
        firmwareOffset += COMS;

        __builtin_memcpy(oemId, &storedFirmwareId[firmwareOffset], OEM_ID);
        firmwareOffset += OEM_ID;

        __builtin_memcpy(projectId, &storedFirmwareId[firmwareOffset], PROJECT_ID);
        firmwareOffset += PROJECT_ID;

        __builtin_memcpy(appVersionL, &storedFirmwareId[firmwareOffset], APP_VERSION_L);
        firmwareOffset += APP_VERSION_L;

        __builtin_memcpy(appVersionM, &storedFirmwareId[firmwareOffset], APP_VERSION_M);
        firmwareOffset += APP_VERSION_M;

        __builtin_memcpy(appVersionR, &storedFirmwareId[firmwareOffset], APP_VERSION_R);
        firmwareOffset += APP_VERSION_R;

        __builtin_memcpy(appSize, &storedFirmwareId[firmwareOffset], APP_SIZE);
        firmwareOffset += APP_SIZE;
        appSize[APP_SIZE] = '\0';

        /* Keep bootVersionL, bootVersionM, and bootVersionR from bootloader. */
        firmwareOffset += BOOT_VERSION_L + BOOT_VERSION_M + BOOT_VERSION_R;

        __builtin_memcpy(stayInBootFlag, &storedFirmwareId[firmwareOffset],
                STAY_IN_BOOT_FLAG);
        stayInBootFlag[STAY_IN_BOOT_FLAG] = '\0';

        Get_Part_Number();
        Get_Model_Number();
        Get_Software_ID();
        Get_Firmware_ID();

        if (__builtin_memcmp(firmwareId, storedFirmwareId,
                FIRMWARE_ID_LENGTH) != 0) {
            Build_Metadata();
            metadataChanged = true;
        }
    }

    if (metadataChanged) {
        __builtin_memset(&eepromBuffer[EEPROM_METADATA_OFFSET], 0xFF,
                EEPROM_METADATA_SIZE);

        __builtin_memcpy(&eepromBuffer[EEPROM_METADATA_OFFSET],
                metadata,
                METADATA_LENGTH);

        Flash_EEPROM_Write_Page(eepromBuffer,
                EEPROM_DATA_SIZE);
    }
}

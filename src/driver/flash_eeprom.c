#include "definitions.h"
#include "driver.h"

typedef struct {
    uint32_t marker;
    uint32_t sequence;
    uint32_t crc;
    uint32_t length;
    uint32_t reserved;
    uint32_t padding[3];
} EEPROM_HEADER;

typedef char EEPROM_HEADER_SIZE_MUST_BE_32[
        (sizeof (EEPROM_HEADER) == EEPROM_HEADER_SIZE) ? 1 : -1];

static const uint32_t pages[EEPROM_NUM_PAGES] = {
    EEPROM_PAGE1_ADDR
};

static uint32_t activePage = 0;

uint8_t eepromBuffer[EEPROM_DATA_SIZE];

static uint32_t Calculate_CRC(const uint8_t *data, uint32_t len) {
    uint32_t crc = 0;

    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc;
}

static bool Flash_Wait_Complete(void) {
    while (NVM_IsBusy());

    if (NVM_ErrorGet())
        return false;

    return true;
}

static bool Erase_Page(uint32_t addr) {
    NVM_PageErase(addr);

    return Flash_Wait_Complete();
}

static bool Write_Quad_Word(uint32_t addr, const uint8_t *data, uint32_t length) {
    uint32_t quadword[4] = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };

    if (length > sizeof (quadword)) {
        return false;
    }

    memcpy(quadword, data, length);
    NVM_QuadWordWrite(quadword, addr);

    return Flash_Wait_Complete();
}

static bool Page_Is_Valid(uint32_t addr, EEPROM_HEADER *header) {
    memcpy(
            header,
            (void*) addr,
            sizeof (EEPROM_HEADER)
            );

    if (header->marker != EEPROM_VALID_MARKER)
        return false;

    if (header->sequence == 0xFFFFFFFF)
        return false;

    if (header->length > EEPROM_DATA_SIZE)
        return false;

    if (header->reserved != 0xAAAAAAAA)
        return false;

    uint32_t crc = Calculate_CRC(
            (const uint8_t *) (addr + sizeof (EEPROM_HEADER)),
            header->length
            );

    if (crc != header->crc)
        return false;

    return true;
}

static uint32_t Find_Latest_Page(void) {
    EEPROM_HEADER hdr;
    uint32_t maxSeq = 0;
    uint32_t latest = 0xFFFFFFFF;

    for (int i = 0; i < EEPROM_NUM_PAGES; i++) {
        if (Page_Is_Valid(pages[i], &hdr)) {
            if (hdr.sequence > maxSeq) {
                maxSeq = hdr.sequence;
                latest = pages[i];
            }
        }
    }

    return latest;
}

FLASH_EEPROM_STATUS Flash_EEPROM_Init(void) {
    activePage = Find_Latest_Page();

    if (activePage == 0xFFFFFFFF) {
        for (int i = 0; i < EEPROM_NUM_PAGES; i++) {
            if (!Erase_Page(pages[i])) {
                return FLASH_EEPROM_ERROR;
            }
        }

        activePage = pages[0]; // FIX
    }
    
    __builtin_memset(eepromBuffer, 0, sizeof (eepromBuffer));
    Flash_EEPROM_Read_Page(eepromBuffer, sizeof (eepromBuffer));

    return FLASH_EEPROM_OK;
}

FLASH_EEPROM_STATUS Flash_EEPROM_Write_Page(const uint8_t *data, uint32_t length) {
    if ((data == NULL) || (length > EEPROM_DATA_SIZE))
        return FLASH_EEPROM_ERROR;

    EEPROM_HEADER oldHdr;
    bool hadValidPage;
    uint32_t nextPage = 0;
    int currentIndex = -1;

    for (int i = 0; i < EEPROM_NUM_PAGES; i++) {
        if (pages[i] == activePage) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1) {
        nextPage = pages[0];
    } else {
        nextPage = pages[(currentIndex + 1) % EEPROM_NUM_PAGES];
    }

    if (nextPage == 0)
        return FLASH_EEPROM_ERROR;

    hadValidPage = (activePage != 0xFFFFFFFF) && Page_Is_Valid(activePage, &oldHdr);

    if (!Erase_Page(nextPage))
        return FLASH_EEPROM_ERROR;

    EEPROM_HEADER newHdr;

    if (hadValidPage) {
        newHdr.sequence = oldHdr.sequence + 1;
    } else {
        newHdr.sequence = 1;
    }

    newHdr.marker = EEPROM_VALID_MARKER;
    newHdr.length = length;
    newHdr.crc = Calculate_CRC(data, length);
    newHdr.reserved = 0xAAAAAAAA;

    uint32_t writeAddr = nextPage + sizeof (EEPROM_HEADER);

    /* Write DATA first */
    for (uint32_t i = 0; i < length; i += 16) {
        uint32_t chunkLength = ((length - i) >= 16U) ? 16U : (length - i);

        if (!Write_Quad_Word(writeAddr + i, &data[i], chunkLength)) {
            return FLASH_EEPROM_ERROR;
        }
    }

    /* Write HEADER last -> commit page */
    newHdr.padding[0] = 0xFFFFFFFF;
    newHdr.padding[1] = 0xFFFFFFFF;
    newHdr.padding[2] = 0xFFFFFFFF;

    /* Write sequence, crc, length, reserved first */
    if (!Write_Quad_Word(nextPage + 16U, ((uint8_t *) & newHdr) + 16U, 16U)) {
        return FLASH_EEPROM_ERROR;
    }

    /* Write marker LAST -> final commit */
    if (!Write_Quad_Word(nextPage, (uint8_t *) & newHdr, 16U)) {
        return FLASH_EEPROM_ERROR;
    }

    activePage = nextPage;
    return FLASH_EEPROM_OK;
}

FLASH_EEPROM_STATUS Flash_EEPROM_Read_Page(uint8_t *data, uint32_t length) {
    EEPROM_HEADER hdr;

    if (!Page_Is_Valid(activePage, &hdr))
        return FLASH_EEPROM_NO_VALID_PAGE;

    if (length > hdr.length)
        length = hdr.length;

    if (!NVM_Read(
            (uint32_t*) data,
            length,
            activePage + sizeof (EEPROM_HEADER)
            )) {
        return FLASH_EEPROM_ERROR;
    }

    uint32_t crc = Calculate_CRC(
            (const uint8_t *) (activePage + sizeof (EEPROM_HEADER)),
            hdr.length);

    if (crc != hdr.crc)
        return FLASH_EEPROM_CRC_ERROR;

    return FLASH_EEPROM_OK;
}

FLASH_EEPROM_STATUS Flash_EEPROM_Write_Word(uint32_t address, uint32_t data,
                                            uint8_t *buffer) {
    if (address >= (EEPROM_DATA_SIZE / 4)) {
        return FLASH_EEPROM_ERROR;
    }

    if (buffer == NULL) {
        return FLASH_EEPROM_ERROR;
    }

    memset(buffer, 0xFF, EEPROM_DATA_SIZE);

    FLASH_EEPROM_STATUS status;

    status = Flash_EEPROM_Read_Page(buffer, EEPROM_DATA_SIZE);

    if ((status != FLASH_EEPROM_OK) &&
            (status != FLASH_EEPROM_NO_VALID_PAGE)) {
        return status;
    }

    uint32_t *wordBuffer = (uint32_t*) buffer;

    wordBuffer[address] = data;

    return Flash_EEPROM_Write_Page(buffer, EEPROM_DATA_SIZE);
}

FLASH_EEPROM_STATUS Flash_EEPROM_Read_Word(uint32_t address, uint32_t *data) {
    if (address >= (EEPROM_DATA_SIZE / 4)) {
        return FLASH_EEPROM_ERROR;
    }

    EEPROM_HEADER hdr;

    if ((data == NULL) || !Page_Is_Valid(activePage, &hdr)) {
        return FLASH_EEPROM_NO_VALID_PAGE;
    }

    if (((address + 1U) * sizeof (uint32_t)) > hdr.length) {
        return FLASH_EEPROM_ERROR;
    }

    memcpy(data, (const void *) (activePage + sizeof (EEPROM_HEADER) +
            (address * sizeof (uint32_t))), sizeof (*data));

    return FLASH_EEPROM_OK;
}

FLASH_EEPROM_STATUS Flash_EEPROM_Erase_All(void) {
    for (int i = 0; i < EEPROM_NUM_PAGES; i++) {
        if (!Erase_Page(pages[i]))
            return FLASH_EEPROM_ERROR;
    }

    activePage = pages[0];

    return FLASH_EEPROM_OK;
}

#include "ota_handler.h"
#include "can_handler.h"
#include "wifi_mqtt_handler.h"
#include "mac_handler.h"
#include "mcp23017_handler.h"

// ----------------SOFTWARE ID----------------------
volatile uint16_t ota_server_major = 0;
volatile uint16_t ota_server_minor = 0;

uint16_t mcu_major_previous = 0;
uint16_t mcu_minor_previous = 0;

uint16_t mcu_major_latest = 0;
uint16_t mcu_minor_latest = 0;

volatile uint16_t OEM_ID = 0;
volatile uint16_t APPLICATION_ID = 0;

// -----------------PIC ID----------------------
uint8_t PIC_ID_RAW[16];
uint8_t PIC_ID[16];
static uint8_t picid_rx_count = 0;

// -----------------FLAGS-----------------------
static bool otaServerVersionUpdated = false;
static bool downloadBinFile = false;
static bool isBinFileDownloaded = false;
static bool mcuUpdateStarted = false;
static bool waitingChunkAck = false;
static bool otaValidRx = false;
static bool sendReport = false;

// ---------------- OTA DATA TRANSFER ----------------
char fileURL[128];
const char *filePath = "/fw.bin";

static File fwFile;
static size_t fwSize = 0;
static size_t totalSent = 0;

static size_t chunkIndex = 0;
static size_t bytesSentInChunk = 0;
static size_t currentChunkSize = 0;
/* Bytes already held by the PIC after its most recent 4 KiB ACK. */
static size_t picBufferedBytes = 0;

static uint32_t otaRxStartTime = 0;

size_t totalFWSize = 0;
bool fileOpened = false;

// -----------------INITIAL STATES----------------------
static MCU_MODE_t mcuMode = MCU_MODE_UNDEFINED;
static OTA_CORE0_STATE_t otaCore0State = OTA_CORE0_IDLE;
static OTA_CORE1_State_t otaCore1State = OTA_CORE1_IDLE;
static OTA_CORE1_State_t otaPrevTxState = OTA_CORE1_IDLE;
static OTA_REPORT_t otaReportStatus = ALREADY_UPDATED;


static void macToString(const uint8_t *mac, char *out) {
  sprintf(out,
          "%02X%02X%02X%02X%02X%02X",
          mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
}

void OTA_Handler_CORE0(const uint8_t *mac) {
  switch (otaCore0State) {

    // =================================================
    case OTA_CORE0_IDLE:
      if (!mcuUpdateStarted) {
        otaCore0State = OTA_CORE0_CHECK_SERVER_VERSION;
      }
      break;

    // =================================================
    case OTA_CORE0_CHECK_SERVER_VERSION:
      {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("OTA CORE0: WiFi not connected");
          break;
        }

        char macStr[13];
        char url[128];

        macToString(mac, macStr);

        snprintf(url, sizeof(url),
                 "%s/%s",
                 OTA_VERSION_URL, macStr);

        HTTPClient http;
        http.begin(url);

        int code = http.GET();
        if (code != HTTP_CODE_OK) {
          Serial.printf("OTA CORE0: Version HTTP error %d\n", code);
          http.end();
          break;
        }

        DynamicJsonDocument doc(128);
        if (deserializeJson(doc, http.getString())) {
          Serial.println("OTA CORE0: Version JSON parse failed");
          http.end();
          break;
        }

        const char *ver = doc["version"];
        if (!ver) {
          Serial.println("OTA CORE0: Version field missing");
          http.end();
          break;
        }

        // -------- Parse version string --------
        uint16_t oem = 0;
        uint16_t app = 0;
        uint16_t major = 0;
        uint16_t minor = 0;

        sscanf(ver, "%hu.%hu.%hu.%hu", &oem, &app, &major, &minor);

        // -----------SOFTWARE ID---------------
        // -----------PROJECT ID----------------
        OEM_ID = oem;
        APPLICATION_ID = app;

        // -----------FIRMWARE ID---------------
        ota_server_major = major;
        ota_server_minor = minor;

        Serial.printf(
          "OTA CORE0: Server OEM:%u APP:%u VER:%u.%u\n",
          OEM_ID,
          APPLICATION_ID,
          ota_server_major,
          ota_server_minor);

        http.end();

        if (
          (ota_server_major > mcu_major_previous) || (ota_server_major == mcu_major_previous && ota_server_minor > mcu_minor_previous)) {
          otaServerVersionUpdated = true;

          Serial.println("OTA CORE0: OTA REQUIRED → downloading firmware");
          otaCore0State = OTA_CORE0_DOWNLOAD_BIN_FILE;
        } else {
          otaServerVersionUpdated = false;

          Serial.println("OTA CORE0: OTA NOT required");
          otaCore0State = OTA_CORE0_DONE;
        }

        break;
      }

    // =================================================
    case OTA_CORE0_DOWNLOAD_BIN_FILE:
      {
        if (!downloadBinFile) {
          // Serial.println("OTA CORE0: Download flag not set, skipping");
          // otaCore0State = OTA_CORE0_DONE;
          break;
        }

        SPIFFS.begin(true);

        char macStr[13];
        macToString(mac, macStr);

        snprintf(fileURL, sizeof(fileURL),
                 "%s/%s",
                 OTA_FIRMWARE_URL, macStr);

        Serial.printf("OTA CORE0: Downloading firmware:\n%s\n", fileURL);

        WiFiClientSecure client;
        client.setInsecure();  // Accept HTTPS certs

        HTTPClient http;
        if (!http.begin(client, fileURL)) {
          Serial.println("OTA CORE0: HTTPS begin failed");
          // otaCore0State = OTA_CORE0_ERROR;
          break;
        }

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
          Serial.printf("OTA CORE0: HTTPS GET failed (%d)\n", httpCode);
          http.end();
          // otaCore0State = OTA_CORE0_ERROR;
          break;
        }

        int len = http.getSize();
        WiFiClient *stream = http.getStreamPtr();

        Serial.printf("OTA CORE0: Firmware size = %d bytes\n", len);

        SPIFFS.remove(filePath);
        File fw = SPIFFS.open(filePath, FILE_WRITE);
        if (!fw) {
          Serial.println("OTA CORE0: Failed to open file");
          http.end();
          // otaCore0State = OTA_CORE0_ERROR;
          break;
        }

        uint8_t buffer[256];

        while (http.connected() && (len > 0 || len == -1)) {
          size_t available = stream->available();
          if (available) {
            int toRead = min((int)available, (int)sizeof(buffer));
            int readBytes = stream->readBytes(buffer, toRead);
            fw.write(buffer, readBytes);
            if (len > 0) len -= readBytes;
          }
          delay(1);  // yield to RTOS
        }

        fw.close();
        http.end();

        isBinFileDownloaded = true;

        Serial.println("OTA CORE0: Firmware saved to SPIFFS");
        otaCore0State = OTA_CORE0_REPORT;
        break;
      }

    case OTA_CORE0_REPORT:
      {
        if (!sendReport) {
          break;
        }

        Serial.println("OTA CORE0: Sending OTA report");

        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("OTA CORE0: WiFi not connected");
          break;
        }

        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;

        if (!http.begin(client, OTA_REPORT_URL)) {
          Serial.println("OTA CORE0: HTTP begin failed");
          break;
        }

        http.addHeader("Content-Type", "application/json");

        DynamicJsonDocument doc(256);

        // ---------------- PIC ID (NO REVERSE) ----------------
        char picidString[33] = { 0 };
        for (int i = 0; i < 16; i++)
          sprintf(&picidString[i * 2], "%02X", PIC_ID[i]);

        doc["pic_id"] = picidString;

        // ---------------- DEVICE ID (NO REVERSE) ----------------
        char macStr[13] = { 0 };
        for (int i = 0; i < 6; i++)
          sprintf(&macStr[i * 2], "%02X", mac[i]);

        doc["deviceId"] = macStr;

        // ---------------- STATUS ----------------
        char statusStr[4];
        sprintf(statusStr, "%d", otaReportStatus);
        doc["status"] = statusStr;

        // ---------------- VERSION ----------------
        char prevVer[8], newVer[8];
        sprintf(prevVer, "%d.%d", mcu_major_previous, mcu_minor_previous);
        sprintf(newVer, "%d.%d", mcu_major_latest, mcu_minor_latest);

        doc["previousVersion"] = prevVer;
        doc["updatedVersion"] = newVer;

        // ---------------- REPROGRAMMING FLAG ----------------
        uint8_t reprogrammingFlag =
          (otaReportStatus == TWO_STEP_SUCCESS) ? 1 : 0;

        doc["reprogramming"] = reprogrammingFlag;

        // ---------------- SEND ----------------
        String body;
        serializeJson(doc, body);

        Serial.println("OTA REPORT JSON:");
        Serial.println(body);

        int httpCode = http.POST(body);
        Serial.printf("OTA REPORT HTTP Code: %d\n", httpCode);

        http.end();

        otaCore0State = OTA_CORE0_DONE;
        break;
      }

    // =================================================
    case OTA_CORE0_DONE:
      // otaCore0State = OTA_CORE0_IDLE;
      break;

    // =================================================
    case OTA_CORE0_ERROR:
      // Serial.println("OTA CORE0: ERROR state");
      break;
  }
}

bool OTA_Handler_CORE1(void) {
  static CanFrame rx;

  switch (otaCore1State) {

    // ==================================================
    case OTA_CORE1_IDLE:
      if (otaServerVersionUpdated) {
        LED_Green_On();
        otaCore1State = OTA_CORE1_SEND_PICID_1_REQ;
      }
      break;

    // ==================================================
    case OTA_CORE1_SEND_PICID_1_REQ:
      {
        uint8_t req[8] = { 0 };

        memset(PIC_ID_RAW, 0, sizeof(PIC_ID_RAW));

        if (!CAN_WriteFrame(CANID_REQ_PICID_1, req, 8)) {
          Serial.println("OTA CORE1: CAN send failed (PICID_1)");
          break;
        }

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

    // ==================================================
    case OTA_CORE1_SEND_PICID_2_REQ:
      {
        uint8_t req[8] = { 0 };

        if (!CAN_WriteFrame(CANID_REQ_PICID_2, req, 8)) {
          Serial.println("OTA CORE1: CAN send failed (PICID_2)");
          break;
        }

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

    // ==================================================
    case OTA_CORE1_SEND_SOFTWARE_ID_REQ:
      {
        uint8_t req[8] = { 0 };

        if (!CAN_WriteFrame(CANID_REQ_SOFTWARE_ID, req, 8)) {
          Serial.println("OTA CORE1: CAN send failed (BOARD_ID)");
          // otaCore1State = OTA_CORE1_ERROR;
          break;
        }

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

      // ==================================================
    case OTA_CORE1_SEND_BOOT_JMP_REQ:
      {
        if (!isBinFileDownloaded) {
          // Firmware not ready yet → wait here
          break;
        }

        uint8_t tx[8] = { 0 };

        mcuUpdateStarted = true;
        tx[0] = CMD_REQ_BOOTLOADER;

        if (!CAN_WriteFrame(CANID_SCU_REQ, tx, 8)) {
          Serial.println("OTA CORE1: Failed to send BOOTLOADER JUMP cmd");
          // otaCore1State = OTA_CORE1_ERROR;
          break;
        }

        Serial.println("OTA CORE1: BOOTLOADER JUMP command sent");

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

      // ==================================================
    case OTA_CORE1_SEND_STATUS_REQ:
      {
        uint8_t tx[8] = { 0 };
        tx[0] = CMD_REQ_STATUS;

        if (!CAN_WriteFrame(CANID_SCU_REQ, tx, 8)) {
          Serial.println("OTA CORE1: STATUS request send failed");
          // otaCore1State = OTA_CORE1_ERROR;
          break;
        }

        Serial.println("OTA CORE1: STATUS request sent");

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

      // ==================================================
    case OTA_CORE1_SEND_START_REQ:
      {
        uint8_t tx[8] = { 0 };
        tx[0] = CMD_REQ_START;  // 0x53

        if (!CAN_WriteFrame(CANID_SCU_REQ, tx, 8)) {
          Serial.println("OTA CORE1: START request send failed");
          // otaCore1State = OTA_CORE1_ERROR;
          break;
        }

        Serial.println("OTA CORE1: START request sent (flash erase)");

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

      // ==================================================
    case OTA_CORE1_SEND_DATA:
      {
        static uint8_t rawData[OTA_FRAME_DATA];
        static uint8_t txBuf[8];

        // ---------------- OPEN FILE ON FIRST ENTRY ----------------
        if (!fileOpened) {
          fwFile = SPIFFS.open(filePath, FILE_READ);
          if (!fwFile || fwFile.size() == 0) {
            Serial.println("OTA CORE1: Firmware file open failed");
            // otaCore1State = OTA_CORE1_ERROR;
            break;
          }

          totalFWSize = fwFile.size();
          totalSent = 0;
          chunkIndex = 0;
          bytesSentInChunk = 0;
          picBufferedBytes = 0;
          waitingChunkAck = false;
          fileOpened = true;

          Serial.printf("OTA CORE1: Firmware size = %u bytes\n", totalFWSize);
        }

        // A wire chunk is only a flow-control window. It must never add
        // padding between real firmware bytes.
        if (!waitingChunkAck && bytesSentInChunk == 0) {
          if (totalSent >= totalFWSize) {
            fwFile.close();
            Serial.println("OTA CORE1: All chunks sent");
            otaCore1State = OTA_CORE1_SEND_END_REQ;
            break;
          }

          Serial.printf(
            "\n[WINDOW %u] firmware offset %u\n",
            chunkIndex, totalSent);
        }

        // Send one continuous file stream. 0xFF fills only the final CAN
        // frame; it is never inserted at an ACK/window boundary.
        if (!waitingChunkAck) {
          size_t fileBytesRemaining = totalFWSize - totalSent;
          size_t toRead = min((size_t)OTA_FRAME_DATA, fileBytesRemaining);

          if (toRead > 0) {
            int n = fwFile.read(rawData, toRead);
            if (n != (int)toRead) {
              Serial.println("OTA CORE1: SPIFFS read failed");
              fwFile.close();
              sendReport = true;
              otaReportStatus = FIRMWARE_UPLOAD_FAILED;
              otaCore1State = OTA_CORE1_ERROR;
              otaCore0State = OTA_CORE0_REPORT;
              break;
            }
          }

          if (toRead < OTA_FRAME_DATA) {
            memset(rawData + toRead, 0xFF, OTA_FRAME_DATA - toRead);
          }

          txBuf[0] = CMD_REQ_DATA;
          memcpy(&txBuf[1], rawData, OTA_FRAME_DATA);

          if (!CAN_WriteFrame(CANID_SCU_REQ, txBuf, 8)) {
            Serial.println("OTA CORE1: CAN send failed (DATA)");
            // otaCore1State = OTA_CORE1_ERROR;
            break;
          }

          bytesSentInChunk += OTA_FRAME_DATA;
          totalSent += toRead;

          delay(1);

          /* The final padded frame is followed immediately by END. */
          if (totalSent >= totalFWSize) {
            fwFile.close();
            Serial.println("OTA CORE1: Final firmware frame sent");
            otaCore1State = OTA_CORE1_SEND_END_REQ;
          } else {
            /*
             * Stop immediately after the frame which fills the PIC's 4096
             * byte buffer. The frame can carry 0..6 bytes for the next
             * buffer; they are already in the PIC before it starts NVM
             * programming. Sending another frame here can overflow the
             * MCP2515 receive buffers during the flash write.
             */
            size_t bytesUntilPicAck = OTA_FLASH_CHUNK_SIZE - picBufferedBytes;
            size_t wireBytesUntilPicAck =
              ((bytesUntilPicAck + OTA_FRAME_DATA - 1U) / OTA_FRAME_DATA) * OTA_FRAME_DATA;

            if (bytesSentInChunk >= wireBytesUntilPicAck) {
              waitingChunkAck = true;
              otaPrevTxState = otaCore1State;
              otaRxStartTime = millis();
              otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
            }
          }
        }

        break;
      }

      // ==================================================
    case OTA_CORE1_SEND_END_REQ:
      {
        uint8_t tx[8];

        // ---- OEM ID ----
        tx[0] = (OEM_ID >> 8) & 0xFF;
        tx[1] = OEM_ID & 0xFF;

        // ---- APPLICATION ID ----
        tx[2] = (APPLICATION_ID >> 8) & 0xFF;
        tx[3] = APPLICATION_ID & 0xFF;

        // ---- OTA VERSION MAJOR ----
        tx[4] = (ota_server_major >> 8) & 0xFF;
        tx[5] = ota_server_major & 0xFF;

        // ---- OTA VERSION MINOR ----
        tx[6] = (ota_server_minor >> 8) & 0xFF;
        tx[7] = ota_server_minor & 0xFF;

        if (!CAN_WriteFrame(CANID_REQ_END, tx, 8)) {
          Serial.println("OTA CORE1: END request send failed");
          break;
        }

        Serial.printf(
          "OTA CORE1: END sent (OEM:%u APP:%u VER:%u.%u)\n",
          OEM_ID,
          APPLICATION_ID,
          ota_server_major,
          ota_server_minor);

        otaPrevTxState = otaCore1State;
        otaRxStartTime = millis();
        otaCore1State = OTA_CORE1_CAN_RX_HANDLE;
        break;
      }

      // ==================================================
    case OTA_CORE1_APP_JMP_REQ:
      {
        uint8_t tx[8] = { 0 };
        tx[0] = CMD_REQ_APP_JUMP;  // 0x56

        if (!CAN_WriteFrame(CANID_SCU_REQ, tx, 8)) {
          Serial.println("OTA CORE1: APP JUMP request send failed");
          // otaCore1State = OTA_CORE1_ERROR;
          break;
        }

        Serial.println("OTA CORE1: APP JUMP command sent");

        sendReport = true;

        // No ACK expected → OTA complete
        otaCore1State = OTA_CORE1_DONE;
        otaCore0State = OTA_CORE0_REPORT;    // Trigger report
        otaReportStatus = TWO_STEP_SUCCESS;  // ✅ SET SUCCESS
        break;
      }

      // ==================================================
    case OTA_CORE1_CAN_RX_HANDLE:
      {
        static uint8_t rxRetryCount = 0;

        if (CAN_ReadFrame(&rx, CAN_RX_TIMEOUT)) {

          otaValidRx = false;

          if (rx.rtr) {
            break;
          }

          switch (rx.identifier) {

              /* ---------- PIC ID PART 1 ---------- */
            case CANID_RESP_PICID_1:
              {
                otaValidRx = true;

                memcpy(&PIC_ID_RAW[0], rx.data, 8);

                Serial.println("OTA CORE1: PICID_1 received");

                otaCore1State = OTA_CORE1_SEND_PICID_2_REQ;
                break;
              }

            /* ---------- PIC ID PART 2 ---------- */
            case CANID_RESP_PICID_2:
              {
                otaValidRx = true;

                memcpy(&PIC_ID_RAW[8], rx.data, 8);

                Serial.println("OTA CORE1: PICID_2 received");

                // Now RAW contains:
                // [0..3]  = DEVSN0
                // [4..7]  = DEVSN1
                // [8..11] = DEVSN2
                // [12..15]= DEVSN3

                // If PIC sends MSB first per word (your PIC code does),
                // then NO word reversal is needed.

                memcpy(PIC_ID, PIC_ID_RAW, 16);

                Serial.print("OTA CORE1: PIC ID = ");
                for (int i = 0; i < 16; i++)
                  Serial.printf("%02X", PIC_ID[i]);
                Serial.println();

                otaCore1State = OTA_CORE1_SEND_SOFTWARE_ID_REQ;
                break;
              }

              /* ---------- SOFTWARE ID ---------- */
            case CANID_RESP_SOFTWARE_ID:
              {
                otaValidRx = true;

                uint16_t mcu_oem_id =
                  ((uint16_t)rx.data[0] << 8) | (uint16_t)rx.data[1];

                uint16_t mcu_application_id =
                  ((uint16_t)rx.data[2] << 8) | (uint16_t)rx.data[3];

                /* -------- NEW MCU CHECK -------- */
                bool newMCU = (mcu_oem_id == 0x0000 && mcu_application_id == 0x0000);

                /* -------- OEM ID CHECK -------- */
                if (!newMCU && mcu_oem_id != OEM_ID) {
                  Serial.println("OTA CORE1: OEM ID mismatch");

                  sendReport = true;
                  otaReportStatus = OEM_ID_MISMATCH;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                  break;
                }

                /* -------- APPLICATION ID CHECK -------- */
                if (!newMCU && mcu_application_id != APPLICATION_ID) {
                  Serial.println("OTA CORE1: APPLICATION ID mismatch");

                  sendReport = true;
                  otaReportStatus = APPLICATION_ID_MISMATCH;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                  break;
                }

                /* -------- VERSION (16-bit) -------- */
                mcu_major_previous =
                  ((uint16_t)rx.data[4] << 8) | (uint16_t)rx.data[5];

                mcu_minor_previous =
                  ((uint16_t)rx.data[6] << 8) | (uint16_t)rx.data[7];

                Serial.printf(
                  "OTA CORE1: MCU %u.%u | Server %u.%u\n",
                  mcu_major_previous, mcu_minor_previous,
                  ota_server_major, ota_server_minor);

                /* -------- VERSION CHECK -------- */
                if ((mcu_major_previous > ota_server_major) || (mcu_major_previous == ota_server_major && mcu_minor_previous >= ota_server_minor)) {

                  Serial.println("OTA CORE1: OTA NOT required");

                  sendReport = true;

                  mcu_major_latest = ota_server_major;
                  mcu_minor_latest = ota_server_minor;

                  otaReportStatus = ALREADY_UPDATED;

                  otaCore1State = OTA_CORE1_DONE;
                  otaCore0State = OTA_CORE0_REPORT;

                  return false;
                }

                downloadBinFile = true;
                Serial.println("OTA CORE1: OTA REQUIRED");

                otaCore1State = OTA_CORE1_SEND_BOOT_JMP_REQ;
                break;
              }

            /* ---------- MCU STATUS / ACK ---------- */
            case CANID_MCU_RESP:
              {
                otaValidRx = true;

                static const uint8_t CHUNK_ACK_DATA[8] = { 0x88, 0x77, 0x66, 0xFF, 0x11, 0x22, 0x44, 0x33 };

                if (waitingChunkAck && rx.data_length_code == 8 && memcmp(rx.data, CHUNK_ACK_DATA, 8) == 0) {

                  Serial.printf("[CHUNK %u] ACK OK\n", chunkIndex);

                  picBufferedBytes += bytesSentInChunk;
                  if ((picBufferedBytes < OTA_FLASH_CHUNK_SIZE) || (picBufferedBytes >= (OTA_FLASH_CHUNK_SIZE + OTA_FRAME_DATA))) {
                    Serial.println("OTA CORE1: Invalid PIC buffer accounting");
                    sendReport = true;
                    otaReportStatus = FIRMWARE_UPLOAD_FAILED;
                    otaCore1State = OTA_CORE1_ERROR;
                    otaCore0State = OTA_CORE0_REPORT;
                    break;
                  }

                  picBufferedBytes -= OTA_FLASH_CHUNK_SIZE;
                  bytesSentInChunk = 0;
                  waitingChunkAck = false;
                  chunkIndex++;

                  otaCore1State = OTA_CORE1_SEND_DATA;
                  break;
                }

                uint8_t resp = rx.data[0];

                switch (resp) {

                  case CMD_RESP_JUMP_OK:
                  case CMD_RESP_BOOTLOADER_MODE:
                    mcuMode = MCU_MODE_BOOTLOADER;
                    Serial.println("OTA CORE1: MCU in BOOTLOADER mode");
                    otaCore1State = OTA_CORE1_SEND_START_REQ;
                    break;

                  case CMD_RESP_FLASH_READY:
                    Serial.println("OTA CORE1: MCU FLASH READY");
                    otaCore1State = OTA_CORE1_SEND_DATA;
                    break;

                  case CMD_RESP_APPLICATION_MODE:
                    Serial.println("OTA CORE1: MCU in APPLICATION mode");
                    otaCore1State = OTA_CORE1_SEND_BOOT_JMP_REQ;
                    break;

                  default:
                    break;
                }
                break;
              }

            case CANID_RESP_END:
              {
                mcu_major_latest = ota_server_major;
                mcu_minor_latest = ota_server_minor;
                Serial.println("OTA CORE1: UPDATE SUCCESS received");
                otaCore1State = OTA_CORE1_APP_JMP_REQ;
                break;
              }

            default:
              break;
          }

          /* ---------- Reset timeout ONLY on valid OTA RX ---------- */
          if (otaValidRx) {
            otaRxStartTime = millis();
            rxRetryCount = 0;  // RESET retry counter on success
          }
        }

        if ((millis() - otaRxStartTime) >= CAN_RX_TIMEOUT) {
          rxRetryCount++;

          Serial.printf(
            "OTA CORE1: RX timeout → retry %u / 5 (state %d)\n",
            rxRetryCount,
            otaPrevTxState);

          if (rxRetryCount >= 5) {

            Serial.println("OTA CORE1: Max retries reached → ABORT OTA");

            rxRetryCount = 0;

            switch (otaPrevTxState) {
              /* ---------- PIC ID FAILURE ---------- */
              case OTA_CORE1_SEND_PICID_1_REQ:
              case OTA_CORE1_SEND_PICID_2_REQ:
                {
                  Serial.println("OTA CORE1: FAILED TO GET PIC ID");

                  sendReport = true;
                  otaReportStatus = FAILED_TO_GET_PICID;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                  break;
                }

              case OTA_CORE1_SEND_SOFTWARE_ID_REQ:
                {
                  Serial.println("OTA CORE1: FAILED TO GET SOFTWARE ID");

                  sendReport = true;
                  otaReportStatus = FAILED_TO_GET_SOFTWARE_ID;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              case OTA_CORE1_SEND_BOOT_JMP_REQ:
                {
                  Serial.println("OTA CORE1: BOOTLOADER JUMP FAILED");

                  sendReport = true;
                  otaReportStatus = BOOT_JUMP_FAILED;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              case OTA_CORE1_SEND_STATUS_REQ:
                {
                  Serial.println("OTA CORE1: FAILED TO GET STATUS");

                  sendReport = true;
                  otaReportStatus = FAILED_TO_GET_STATUS;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              case OTA_CORE1_SEND_START_REQ:
                {
                  Serial.println("OTA CORE1: MCU MEMORY ERASE FAILED");

                  sendReport = true;
                  otaReportStatus = MCU_MEMORY_ERASE_FAILED;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              case OTA_CORE1_SEND_DATA:
                {
                  Serial.println("OTA CORE1: FIRMWARE UPLOAD FAILED");

                  sendReport = true;
                  otaReportStatus = FIRMWARE_UPLOAD_FAILED;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              case OTA_CORE1_SEND_END_REQ:
                {
                  Serial.println("OTA CORE1: SOFTWARE ID UPDATE FAILED");

                  sendReport = true;
                  otaReportStatus = SOFTWARE_ID_UPDATE_FAILED;

                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_REPORT;
                }

              default:
                {
                  otaCore1State = OTA_CORE1_ERROR;
                  otaCore0State = OTA_CORE0_ERROR;
                  break;
                }
            }
          } else {
            otaCore1State = otaPrevTxState;  // retry previous TX state
          }
        }

        break;
      }

    // ==================================================
    case OTA_CORE1_DONE:
      LED_Green_Off();

      otaServerVersionUpdated = false;
      downloadBinFile = false;
      isBinFileDownloaded = false;
      mcuUpdateStarted = false;

      // -------- RESET OTA DATA STATE --------
      totalFWSize = 0;
      totalSent = 0;
      chunkIndex = 0;
      bytesSentInChunk = 0;
      picBufferedBytes = 0;
      waitingChunkAck = false;
      fileOpened = false;

      if (fwFile) fwFile.close();

      otaCore1State = OTA_CORE1_IDLE;
      break;

    // ==================================================
    case OTA_CORE1_ERROR:
      LED_Green_Off();
      // Serial.println("OTA CORE1: ERROR state");
      break;
  }

  return false;
}

void CORE0_OTA_Task(void *p) {
  while (1) {

    if (WiFi.status() == WL_CONNECTED) {
      OTA_Handler_CORE0(mac);
    }

    // OTA FSM runs fast but non-blocking
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}
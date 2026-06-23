#ifndef DRIVER_MCP2515_H
#define DRIVER_MCP2515_H

#include <stdint.h>
#include <stdbool.h>

#define MCP2515_FIFO_SIZE       128U
#define MCP2515_CAN_MAX_DLC     8U

bool MCP2515_Init(void);

void MCP2515_Task(void);

bool MCP2515_IsBusy(void);

bool MCP2515_SendMessage(uint32_t id, bool extended, uint8_t dlc, const uint8_t *data);

bool MCP2515_ReceiveMessage(uint32_t *id, bool *extended, uint8_t *dlc, uint8_t *data);

uint8_t MCP2515_TxFifoFreeGet(void);

uint8_t MCP2515_RxFifoCountGet(void);

bool MCP2515_SendMessageDone(void);

#endif

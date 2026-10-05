#include "definitions.h"
#include "driver.h"

/* MCP2515 Commands */
#define MCP2515_RESET_CMD        0xC0
#define MCP2515_READ_CMD         0x03
#define MCP2515_WRITE_CMD        0x02

/* MCP2515 Registers */
#define CANSTAT                  0x0E
#define CANCTRL                  0x0F

/* CAN Timing */
#define CNF1       0x2A
#define CNF2       0x29
#define CNF3       0x28

/* TX Registers*/
#define TXB0CTRL       0x30

#define TXB0SIDH       0x31
#define TXB0SIDL       0x32

#define TXB0DLC        0x35

#define TXB0D0         0x36

/* RX Registers */
#define RXB0CTRL       0x60

#define RXB0SIDH       0x61
#define RXB0SIDL       0x62

#define RXB0DLC        0x65

#define RXB0D0         0x66

#define CANINTF        0x2C

/* Bit Modify CMD*/
#define MCP2515_BIT_MODIFY_CMD      0x05

/* RTS CMD */
#define MCP2515_RTS_TX0_CMD      0x81

#define MCP2515_INIT_ATTEMPTS          5U
#define MCP2515_POWER_ON_DELAY_MS      100U
#define MCP2515_RESET_DELAY_MS         10U
#define MCP2515_MODE_CHANGE_DELAY_MS   2U

static uint8_t spiTx[16];
static uint8_t spiRx[16];

#define MCP2515_FRAME_FLAG_EXTENDED    0x01U
#define MCP2515_FRAME_HEADER_SIZE      6U
#define MCP2515_FRAME_MAX_SIZE         (MCP2515_FRAME_HEADER_SIZE + MCP2515_CAN_MAX_DLC)

typedef enum
{
    MCP2515_ASYNC_IDLE = 0,
    MCP2515_ASYNC_TX_LOAD,
    MCP2515_ASYNC_TX_RTS,
    MCP2515_ASYNC_TX_POLL,
    MCP2515_ASYNC_RX_STATUS,
    MCP2515_ASYNC_RX_FRAME,
    MCP2515_ASYNC_RX_CLEAR,
    MCP2515_ASYNC_ERROR
} MCP2515_ASYNC_STATE;

typedef enum
{
    MCP2515_SPI_OP_NONE = 0,
    MCP2515_SPI_OP_WRITE,
    MCP2515_SPI_OP_READ,
    MCP2515_SPI_OP_BIT_MODIFY,
    MCP2515_SPI_OP_RTS
} MCP2515_SPI_OP;

static volatile bool spiDone = true;
static volatile bool spiActive = false;
static MCP2515_ASYNC_STATE asyncState = MCP2515_ASYNC_IDLE;
static MCP2515_SPI_OP activeSpiOp = MCP2515_SPI_OP_NONE;
static uint32_t activeTxId = 0U;
static bool activeTxExtended = false;
static uint8_t activeTxDlc = 0U;
static uint8_t activeTxData[MCP2515_CAN_MAX_DLC];
static uint32_t activeRxId = 0U;
static bool activeRxExtended = false;
static uint8_t activeRxDlc = 0U;
static uint8_t activeRxData[MCP2515_CAN_MAX_DLC];
static bool asyncTxDone = false;
static bool asyncCallbacksRegistered = false;

static uint8_t txFifo[MCP2515_FIFO_SIZE];
static uint8_t txFifoHead = 0U;
static uint8_t txFifoTail = 0U;
static uint8_t txFifoCount = 0U;

static uint8_t rxFifo[MCP2515_FIFO_SIZE];
static uint8_t rxFifoHead = 0U;
static uint8_t rxFifoTail = 0U;
static uint8_t rxFifoCount = 0U;

static void MCP2515_AsyncRegisterCallback(void);
static bool MCP2515_StartWriteTransaction(uint8_t length);
static bool MCP2515_StartReadTransaction(uint8_t length);
static bool MCP2515_AsyncReadRegister(uint8_t address);
static bool MCP2515_AsyncBitModify(uint8_t address, uint8_t mask, uint8_t data);
static bool MCP2515_AsyncRtsTx0(void);
static bool MCP2515_FrameEnqueue(uint8_t *fifo, uint8_t *head, uint8_t *count, uint32_t id, bool extended, uint8_t dlc, const uint8_t *data);
static bool MCP2515_FrameDequeue(uint8_t *fifo, uint8_t *tail, uint8_t *count, uint32_t *id, bool *extended, uint8_t *dlc, uint8_t *data);

static void MCP2515_Select(void) {
    MCP2515_CS_Clear();
}

static void MCP2515_Deselect(void) {
    MCP2515_CS_Set();
}

static void MCP2515_Delay(void) {
    volatile uint32_t i;

    for (i = 0; i < 100000; i++);
}

static void MCP2515_DelayMs(uint32_t delayMs)
{
    while(delayMs > 0U)
    {
        MCP2515_Delay();
        delayMs--;
    }
}

static void MCP2515_FifoWriteByte(uint8_t *fifo, uint8_t *head, uint8_t *count, uint8_t data)
{
    fifo[*head] = data;
    *head = (uint8_t)((*head + 1U) % MCP2515_FIFO_SIZE);
    (*count)++;
}

static uint8_t MCP2515_FifoReadByte(uint8_t *fifo, uint8_t *tail, uint8_t *count)
{
    uint8_t data = fifo[*tail];

    *tail = (uint8_t)((*tail + 1U) % MCP2515_FIFO_SIZE);
    (*count)--;

    return data;
}

static bool MCP2515_FrameEnqueue(uint8_t *fifo, uint8_t *head, uint8_t *count, uint32_t id, bool extended, uint8_t dlc, const uint8_t *data)
{
    uint8_t i;
    uint8_t frameSize;
    uint8_t flags = 0U;

    if((dlc > MCP2515_CAN_MAX_DLC) || ((dlc > 0U) && (data == NULL)))
    {
        return false;
    }

    if((extended == false) && (id > 0x7FFU))
    {
        return false;
    }

    if((extended == true) && (id > 0x1FFFFFFFUL))
    {
        return false;
    }

    frameSize = (uint8_t)(MCP2515_FRAME_HEADER_SIZE + dlc);

    if((MCP2515_FIFO_SIZE - *count) < frameSize)
    {
        return false;
    }

    if(extended == true)
    {
        flags |= MCP2515_FRAME_FLAG_EXTENDED;
    }

    MCP2515_FifoWriteByte(fifo, head, count, flags);
    MCP2515_FifoWriteByte(fifo, head, count, (uint8_t)(id & 0xFFU));
    MCP2515_FifoWriteByte(fifo, head, count, (uint8_t)((id >> 8) & 0xFFU));
    MCP2515_FifoWriteByte(fifo, head, count, (uint8_t)((id >> 16) & 0xFFU));
    MCP2515_FifoWriteByte(fifo, head, count, (uint8_t)((id >> 24) & 0xFFU));
    MCP2515_FifoWriteByte(fifo, head, count, dlc);

    for(i = 0U; i < dlc; i++)
    {
        MCP2515_FifoWriteByte(fifo, head, count, data[i]);
    }

    return true;
}

static bool MCP2515_FrameDequeue(uint8_t *fifo, uint8_t *tail, uint8_t *count, uint32_t *id, bool *extended, uint8_t *dlc, uint8_t *data)
{
    uint8_t i;
    uint8_t flags;
    uint32_t frameId;
    uint8_t frameDlc;

    if((id == NULL) || (extended == NULL) || (dlc == NULL) || (data == NULL))
    {
        return false;
    }

    if(*count < MCP2515_FRAME_HEADER_SIZE)
    {
        return false;
    }

    flags = MCP2515_FifoReadByte(fifo, tail, count);
    frameId = (uint32_t)MCP2515_FifoReadByte(fifo, tail, count);
    frameId |= ((uint32_t)MCP2515_FifoReadByte(fifo, tail, count) << 8);
    frameId |= ((uint32_t)MCP2515_FifoReadByte(fifo, tail, count) << 16);
    frameId |= ((uint32_t)MCP2515_FifoReadByte(fifo, tail, count) << 24);
    frameDlc = MCP2515_FifoReadByte(fifo, tail, count);

    if((*count < frameDlc) || (frameDlc > MCP2515_CAN_MAX_DLC))
    {
        *count = 0U;
        return false;
    }

    for(i = 0U; i < frameDlc; i++)
    {
        data[i] = MCP2515_FifoReadByte(fifo, tail, count);
    }

    *id = frameId;
    *extended = ((flags & MCP2515_FRAME_FLAG_EXTENDED) != 0U);
    *dlc = frameDlc;

    return true;
}

static void MCP2515_AsyncResetState(void)
{
    spiDone = true;
    spiActive = false;
    asyncState = MCP2515_ASYNC_IDLE;
    activeSpiOp = MCP2515_SPI_OP_NONE;
    asyncTxDone = false;
}

static void MCP2515_Reset(void) {
    uint8_t cmd;

    cmd = MCP2515_RESET_CMD;

    MCP2515_Select();

    SPI1_Write(&cmd, 1);

    while (SPI1_IsBusy());

    MCP2515_Deselect();

    MCP2515_DelayMs(MCP2515_RESET_DELAY_MS);
}

static uint8_t MCP2515_ReadRegister(uint8_t address) {
    spiTx[0] = MCP2515_READ_CMD;
    spiTx[1] = address;
    spiTx[2] = 0xFF;

    MCP2515_Select();

    SPI1_WriteRead(spiTx, 3, spiRx, 3);

    while (SPI1_IsBusy());

    MCP2515_Deselect();

    return spiRx[2];
}

static void MCP2515_WriteRegister(uint8_t address, uint8_t data) {
    spiTx[0] = MCP2515_WRITE_CMD;
    spiTx[1] = address;
    spiTx[2] = data;

    MCP2515_Select();

    SPI1_Write(spiTx, 3);

    while (SPI1_IsBusy());

    MCP2515_Deselect();
}

static void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
    spiTx[0] = MCP2515_BIT_MODIFY_CMD;

    spiTx[1] = address;

    spiTx[2] = mask;

    spiTx[3] = data;

    MCP2515_Select();

    SPI1_Write(spiTx, 4);

    while(SPI1_IsBusy());

    MCP2515_Deselect();
}

bool MCP2515_Init(void)
{
    uint8_t status;
    uint8_t attempt;

    MCP2515_AsyncRegisterCallback();

    /* Configure CS Pin */
    MCP2515_CS_OutputEnable();

    MCP2515_Deselect();

    MCP2515_DelayMs(MCP2515_POWER_ON_DELAY_MS);

    for(attempt = 0U; attempt < MCP2515_INIT_ATTEMPTS; attempt++)
    {
        MCP2515_AsyncResetState();

        /* Reset MCP2515 */
        MCP2515_Reset();

        /* Enter Configuration Mode */
        MCP2515_WriteRegister(CANCTRL, 0x80);

        MCP2515_DelayMs(MCP2515_MODE_CHANGE_DELAY_MS);

        /* Verify Configuration Mode */
        status = MCP2515_ReadRegister(CANSTAT);

        if((status & 0xE0) != 0x80)
        {
            MCP2515_DelayMs(MCP2515_RESET_DELAY_MS);
            continue;
        }

        /*
            CAN Baudrate = 500kbps
            MCP2515 Crystal = 8MHz
        */

        MCP2515_WriteRegister(CNF1, 0x00);

        MCP2515_WriteRegister(CNF2, 0x98);

        MCP2515_WriteRegister(CNF3, 0x01);

        /*
            Receive Any Standard Message
        */

        MCP2515_WriteRegister(RXB0CTRL, 0x60);

        /*
            Clear stale interrupt flags after brown-out/cold start
        */

        MCP2515_BitModify(CANINTF, 0xFF, 0x00);

        /* Enter Normal Mode */

        MCP2515_WriteRegister(CANCTRL, 0x00);

        MCP2515_DelayMs(MCP2515_MODE_CHANGE_DELAY_MS);

        /* Verify Normal Mode */

        status = MCP2515_ReadRegister(CANSTAT);

        if((status & 0xE0) == 0x00)
        {
            return true;
        }

        MCP2515_DelayMs(MCP2515_RESET_DELAY_MS);
    }

    return false;
}

bool MCP2515_SendMessage(uint32_t id, bool extended, uint8_t dlc, const uint8_t *data)
{
    return MCP2515_FrameEnqueue(txFifo, &txFifoHead, &txFifoCount, id, extended, dlc, data);
}

bool MCP2515_ReceiveMessage(uint32_t *id, bool *extended, uint8_t *dlc, uint8_t *data)
{
    return MCP2515_FrameDequeue(rxFifo, &rxFifoTail, &rxFifoCount, id, extended, dlc, data);
}

uint8_t MCP2515_TxFifoFreeGet(void)
{
    return (uint8_t)(MCP2515_FIFO_SIZE - txFifoCount);
}

uint8_t MCP2515_RxFifoCountGet(void)
{
    return rxFifoCount;
}

static void MCP2515_SPI_Callback(uintptr_t context)
{
    (void)context;

    MCP2515_Deselect();
    spiActive = false;
    spiDone = true;
}

static void MCP2515_AsyncRegisterCallback(void)
{
    if(asyncCallbacksRegistered == false)
    {
        SPI1_CallbackRegister(MCP2515_SPI_Callback, 0);
        asyncCallbacksRegistered = true;
    }
}

static bool MCP2515_StartWriteTransaction(uint8_t length)
{
    if((spiActive == true) || (SPI1_IsBusy() == true))
    {
        return false;
    }

    spiDone = false;
    spiActive = true;
    if(activeSpiOp == MCP2515_SPI_OP_NONE)
    {
        activeSpiOp = MCP2515_SPI_OP_WRITE;
    }

    MCP2515_Select();

    if(SPI1_Write(spiTx, length) == false)
    {
        MCP2515_Deselect();
        spiActive = false;
        spiDone = true;
        activeSpiOp = MCP2515_SPI_OP_NONE;
        return false;
    }

    return true;
}

bool MCP2515_IsBusy(void)
{
    return (asyncState != MCP2515_ASYNC_IDLE) || (spiActive == true) || (SPI1_IsBusy() == true);
}

bool MCP2515_SendMessageDone(void)
{
    bool done = asyncTxDone;

    asyncTxDone = false;

    return done;
}

void MCP2515_Task(void)
{
    uint8_t i;
    uint8_t length;

    if((spiActive == true) || (spiDone == false))
    {
        return;
    }

    switch(asyncState)
    {
        case MCP2515_ASYNC_IDLE:
        {
            activeSpiOp = MCP2515_SPI_OP_NONE;

            if(txFifoCount >= MCP2515_FRAME_HEADER_SIZE)
            {
                if(MCP2515_FrameDequeue(txFifo, &txFifoTail, &txFifoCount, &activeTxId, &activeTxExtended, &activeTxDlc, activeTxData) == true)
                {
                    asyncTxDone = false;
                    asyncState = MCP2515_ASYNC_TX_LOAD;
                }
            }
            else
            {
                asyncState = MCP2515_ASYNC_RX_STATUS;
            }
            break;
        }

        case MCP2515_ASYNC_TX_LOAD:
        {
            spiTx[0] = MCP2515_WRITE_CMD;
            spiTx[1] = TXB0SIDH;

            if(activeTxExtended == true)
            {
                spiTx[2] = (uint8_t)((activeTxId >> 21) & 0xFFU);
                spiTx[3] = (uint8_t)(((activeTxId >> 13) & 0xE0U) | 0x08U | ((activeTxId >> 16) & 0x03U));
                spiTx[4] = (uint8_t)((activeTxId >> 8) & 0xFFU);
                spiTx[5] = (uint8_t)(activeTxId & 0xFFU);
            }
            else
            {
                spiTx[2] = (uint8_t)((activeTxId >> 3) & 0xFFU);
                spiTx[3] = (uint8_t)((activeTxId & 0x07U) << 5);
                spiTx[4] = 0x00;
                spiTx[5] = 0x00;
            }

            spiTx[6] = activeTxDlc;

            for(i = 0; i < activeTxDlc; i++)
            {
                spiTx[7U + i] = activeTxData[i];
            }

            activeSpiOp = MCP2515_SPI_OP_NONE;
            length = (uint8_t)(7U + activeTxDlc);

            if(MCP2515_StartWriteTransaction(length) == true)
            {
                asyncState = MCP2515_ASYNC_TX_RTS;
            }
            else
            {
                asyncState = MCP2515_ASYNC_ERROR;
            }
            break;
        }

        case MCP2515_ASYNC_TX_RTS:
        {
            activeSpiOp = MCP2515_SPI_OP_NONE;

            if(MCP2515_AsyncRtsTx0() == true)
            {
                asyncState = MCP2515_ASYNC_TX_POLL;
            }
            else
            {
                asyncState = MCP2515_ASYNC_ERROR;
            }
            break;
        }

        case MCP2515_ASYNC_TX_POLL:
        {
            if(activeSpiOp == MCP2515_SPI_OP_READ)
            {
                activeSpiOp = MCP2515_SPI_OP_NONE;

                if((spiRx[2] & 0x08U) == 0U)
                {
                    asyncTxDone = true;
                    asyncState = MCP2515_ASYNC_IDLE;
                    break;
                }
            }

            if(MCP2515_AsyncReadRegister(TXB0CTRL) == false)
            {
                asyncState = MCP2515_ASYNC_ERROR;
            }
            break;
        }

        case MCP2515_ASYNC_RX_STATUS:
        {
            if(activeSpiOp == MCP2515_SPI_OP_READ)
            {
                activeSpiOp = MCP2515_SPI_OP_NONE;

                if((spiRx[2] & 0x01U) == 0U)
                {
                    asyncState = MCP2515_ASYNC_IDLE;
                    break;
                }

                asyncState = MCP2515_ASYNC_RX_FRAME;
            }
            else if(MCP2515_AsyncReadRegister(CANINTF) == false)
            {
                asyncState = MCP2515_ASYNC_ERROR;
            }
            break;
        }

        case MCP2515_ASYNC_RX_FRAME:
        {
            if(activeSpiOp == MCP2515_SPI_OP_READ)
            {
                activeSpiOp = MCP2515_SPI_OP_NONE;

                activeRxExtended = ((spiRx[3] & 0x08U) != 0U);

                if(activeRxExtended == true)
                {
                    activeRxId = ((uint32_t)spiRx[2] << 21);
                    activeRxId |= ((uint32_t)(spiRx[3] & 0xE0U) << 13);
                    activeRxId |= ((uint32_t)(spiRx[3] & 0x03U) << 16);
                    activeRxId |= ((uint32_t)spiRx[4] << 8);
                    activeRxId |= (uint32_t)spiRx[5];
                }
                else
                {
                    activeRxId = ((uint32_t)spiRx[2] << 3);
                    activeRxId |= (uint32_t)(spiRx[3] >> 5);
                }

                activeRxDlc = (spiRx[6] & 0x0FU);

                if(activeRxDlc > MCP2515_CAN_MAX_DLC)
                {
                    activeRxDlc = MCP2515_CAN_MAX_DLC;
                }

                for(i = 0; i < activeRxDlc; i++)
                {
                    activeRxData[i] = spiRx[7U + i];
                }

                asyncState = MCP2515_ASYNC_RX_CLEAR;
            }
            else
            {
                spiTx[0] = MCP2515_READ_CMD;
                spiTx[1] = RXB0SIDH;

                for(i = 2; i < 15U; i++)
                {
                    spiTx[i] = 0xFF;
                }

                if(MCP2515_StartReadTransaction(15) == false)
                {
                    asyncState = MCP2515_ASYNC_ERROR;
                }
            }
            break;
        }

        case MCP2515_ASYNC_RX_CLEAR:
        {
            if(activeSpiOp == MCP2515_SPI_OP_BIT_MODIFY)
            {
                activeSpiOp = MCP2515_SPI_OP_NONE;
                (void)MCP2515_FrameEnqueue(rxFifo, &rxFifoHead, &rxFifoCount, activeRxId, activeRxExtended, activeRxDlc, activeRxData);
                asyncState = MCP2515_ASYNC_IDLE;
            }
            else if(MCP2515_AsyncBitModify(CANINTF, 0x01, 0x00) == false)
            {
                asyncState = MCP2515_ASYNC_ERROR;
            }
            break;
        }

        case MCP2515_ASYNC_ERROR:
        default:
        {
            MCP2515_Deselect();
            spiActive = false;
            spiDone = true;
            activeSpiOp = MCP2515_SPI_OP_NONE;
            asyncState = MCP2515_ASYNC_IDLE;
            break;
        }
    }
}

static bool MCP2515_StartReadTransaction(uint8_t length)
{
    if((spiActive == true) || (SPI1_IsBusy() == true))
    {
        return false;
    }

    spiDone = false;
    spiActive = true;
    activeSpiOp = MCP2515_SPI_OP_READ;

    MCP2515_Select();

    if(SPI1_WriteRead(spiTx, length, spiRx, length) == false)
    {
        MCP2515_Deselect();
        spiActive = false;
        spiDone = true;
        activeSpiOp = MCP2515_SPI_OP_NONE;
        return false;
    }

    return true;
}

static bool MCP2515_AsyncReadRegister(uint8_t address)
{
    spiTx[0] = MCP2515_READ_CMD;
    spiTx[1] = address;
    spiTx[2] = 0xFF;

    return MCP2515_StartReadTransaction(3);
}

static bool MCP2515_AsyncBitModify(uint8_t address, uint8_t mask, uint8_t data)
{
    spiTx[0] = MCP2515_BIT_MODIFY_CMD;
    spiTx[1] = address;
    spiTx[2] = mask;
    spiTx[3] = data;

    activeSpiOp = MCP2515_SPI_OP_BIT_MODIFY;

    return MCP2515_StartWriteTransaction(4);
}

static bool MCP2515_AsyncRtsTx0(void)
{
    spiTx[0] = MCP2515_RTS_TX0_CMD;
    activeSpiOp = MCP2515_SPI_OP_RTS;

    return MCP2515_StartWriteTransaction(1);
}
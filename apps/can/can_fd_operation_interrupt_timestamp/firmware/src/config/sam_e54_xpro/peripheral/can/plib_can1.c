/*******************************************************************************
  Controller Area Network (CAN) Peripheral Library Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_can1.c

  Summary:
    CAN peripheral library interface.

  Description:
    This file defines the interface to the CAN peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Header Includes
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "interrupts.h"
#include "plib_can1.h"

// *****************************************************************************
// *****************************************************************************
// Global Data
// *****************************************************************************
// *****************************************************************************
#define CAN_STD_ID_Msk        0x7FFU
#define CAN_CALLBACK_TX_INDEX 3U
#define NUM_RX_FIFOS 3U
#define NUM_RX_BUFFER_ELEMENTS 1U
static CAN_RX_MSG can1RxMsg[NUM_RX_FIFOS][NUM_RX_BUFFER_ELEMENTS];
static CAN_CALLBACK_OBJ can1CallbackObj[4];
static CAN_OBJ can1Obj;

static const can_sidfe_registers_t can1StdFilter[] =
{
    {
        .CAN_SIDFE_0 = CAN_SIDFE_0_SFT(0UL) |
                  CAN_SIDFE_0_SFID1(0x469UL) |
                  CAN_SIDFE_0_SFID2(0x469UL) |
                  CAN_SIDFE_0_SFEC(1UL)
    },
    {
        .CAN_SIDFE_0 = CAN_SIDFE_0_SFT(0UL) |
                  CAN_SIDFE_0_SFID1(0x45aUL) |
                  CAN_SIDFE_0_SFID2(0x0UL) |
                  CAN_SIDFE_0_SFEC(7UL)
    },
};

static const can_xidfe_registers_t can1ExtFilter[] =
{
    {
        .CAN_XIDFE_0 = CAN_XIDFE_0_EFID1(0x100000a5UL) | CAN_XIDFE_0_EFEC(7UL),
        .CAN_XIDFE_1 = CAN_XIDFE_1_EFID2(0x0UL) | CAN_XIDFE_1_EFT(0UL),
    },
    {
        .CAN_XIDFE_0 = CAN_XIDFE_0_EFID1(0x10000096UL) | CAN_XIDFE_0_EFEC(2UL),
        .CAN_XIDFE_1 = CAN_XIDFE_1_EFID2(0x10000096UL) | CAN_XIDFE_1_EFT(0UL),
    },
};

/******************************************************************************
Local Functions
******************************************************************************/

static void CANLengthToDlcGet(uint8_t length, uint8_t *dlc)
{
    if (length <= 8U)
    {
        *dlc = length;
    }
    else if (length <= 12U)
    {
        *dlc = 0x9U;
    }
    else if (length <= 16U)
    {
        *dlc = 0xAU;
    }
    else if (length <= 20U)
    {
        *dlc = 0xBU;
    }
    else if (length <= 24U)
    {
        *dlc = 0xCU;
    }
    else if (length <= 32U)
    {
        *dlc = 0xDU;
    }
    else if (length <= 48U)
    {
        *dlc = 0xEU;
    }
    else
    {
        *dlc = 0xFU;
    }
}
static uint8_t CANDlcToLengthGet(uint8_t dlc)
{
    uint8_t msgLength[] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
    return msgLength[dlc];
}

// *****************************************************************************
// *****************************************************************************
// CAN1 PLib Interface Routines
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
    void CAN1_Initialize(void)

   Summary:
    Initializes given instance of the CAN peripheral.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None
*/
void CAN1_Initialize(void)
{
    /* Start CAN initialization */
    CAN1_REGS->CAN_CCCR = CAN_CCCR_INIT_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) != CAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }

    /* Set CCE to unlock the configuration registers */
    CAN1_REGS->CAN_CCCR |= CAN_CCCR_CCE_Msk;

    /* Set Data Bit Timing and Prescaler Register */
    CAN1_REGS->CAN_DBTP = CAN_DBTP_DTSEG2(7UL) | CAN_DBTP_DTSEG1(20UL) | CAN_DBTP_DBRP(0UL) | CAN_DBTP_DSJW(6UL);

    /* Set Nominal Bit timing and Prescaler Register */
    CAN1_REGS->CAN_NBTP  = CAN_NBTP_NTSEG2(4UL) | CAN_NBTP_NTSEG1(13UL) | CAN_NBTP_NBRP(5UL) | CAN_NBTP_NSJW(3UL);

    /* Receive Buffer / FIFO Element Size Configuration Register */
    CAN1_REGS->CAN_RXESC = 0UL  | CAN_RXESC_F0DS(7UL) | CAN_RXESC_F1DS(7UL) | CAN_RXESC_RBDS(7UL);
    /*lint -e{9048} PC lint incorrectly reports a missing 'U' Suffix */
    CAN1_REGS->CAN_NDAT1 = CAN_NDAT1_Msk;
    /*lint -e{9048} PC lint incorrectly reports a missing 'U' Suffix */
    CAN1_REGS->CAN_NDAT2 = CAN_NDAT2_Msk;

    /* Transmit Buffer/FIFO Element Size Configuration Register */
    CAN1_REGS->CAN_TXESC = CAN_TXESC_TBDS(7UL);

    /* Global Filter Configuration Register */
    CAN1_REGS->CAN_GFC = CAN_GFC_ANFS_REJECT | CAN_GFC_ANFE_REJECT;

    /* Extended ID AND Mask Register */
    CAN1_REGS->CAN_XIDAM = CAN_XIDAM_Msk;

    /* Timestamp Counter Configuration Register */
    CAN1_REGS->CAN_TSCC = CAN_TSCC_TCP(0UL) | CAN_TSCC_TSS_INC;

    /* Set the operation mode */
    CAN1_REGS->CAN_CCCR = (CAN1_REGS->CAN_CCCR & ~CAN_CCCR_INIT_Msk) | CAN_CCCR_FDOE_Msk | CAN_CCCR_BRSE_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) == CAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }

    /* Select interrupt line */
    CAN1_REGS->CAN_ILS = 0x0U;

    /* Enable interrupt line */
    CAN1_REGS->CAN_ILE = CAN_ILE_EINT0_Msk;

    /* Enable CAN interrupts */
    CAN1_REGS->CAN_IE = CAN_IE_BOE_Msk;

    // Initialize the CAN PLib Object
    can1Obj.txBufferIndex = 0U;
    can1Obj.rxBufferIndex1 = 0U;
    can1Obj.rxBufferIndex2 = 0U;
    memset(can1RxMsg, 0x00, sizeof(can1RxMsg));
    memset(&can1Obj.msgRAMConfig, 0x00, sizeof(CAN_MSG_RAM_CONFIG));
}

// *****************************************************************************
/* Function:
    bool CAN1_MessageTransmit(uint32_t id, uint8_t length, uint8_t* data, CAN_MODE mode, CAN_MSG_TX_ATTRIBUTE msgAttr)

   Summary:
    Transmits a message into CAN bus.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id      - 11-bit / 29-bit identifier (ID).
    length  - length of data buffer in number of bytes.
    data    - pointer to source data buffer
    mode    - CAN mode Classic CAN or CAN FD without BRS or CAN FD with BRS
    msgAttr - Data Frame or Remote frame using Tx FIFO or Tx Buffer

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_MessageTransmit(uint32_t id, uint8_t length, uint8_t* data, CAN_MODE mode, CAN_MSG_TX_ATTRIBUTE msgAttr)
{
    uint8_t dlc = 0U;
    uint8_t tfqpi = 0U;
    can_txbe_registers_t *fifo = NULL;
    static uint8_t messageMarker = 0U;
    bool op_success = false;

    switch (msgAttr)
    {
        case CAN_MSG_ATTR_TX_FIFO_DATA_FRAME:
        case CAN_MSG_ATTR_TX_FIFO_RTR_FRAME:
            /* The FIFO is not full */
            if (0U == (CAN1_REGS->CAN_TXFQS & CAN_TXFQS_TFQF_Msk))
            {
                tfqpi = (uint8_t)((CAN1_REGS->CAN_TXFQS & CAN_TXFQS_TFQPI_Msk) >> CAN_TXFQS_TFQPI_Pos);
                fifo = (can_txbe_registers_t *) ((uint8_t*)can1Obj.msgRAMConfig.txBuffersAddress + ((uint32_t)tfqpi * CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE));
                op_success = true;
            }
            break;
        default:
            /* Invalid Message Attribute */
            break;
    }
    if (op_success)
    {
        /* If the id is longer than 11 bits, it is considered as extended identifier */
        if (id > CAN_STD_ID_Msk)
        {
            /* An extended identifier is stored into ID */
            fifo->CAN_TXBE_0 = (id & CAN_TXBE_0_ID_Msk) | CAN_TXBE_0_XTD_Msk;
        }
        else
        {
            /* A standard identifier is stored into ID[28:18] */
            fifo->CAN_TXBE_0 = id << 18U;
        }
        if (length > 64U)
        {
            length = 64U;
        }

        CANLengthToDlcGet(length, &dlc);

        fifo->CAN_TXBE_1 = CAN_TXBE_1_DLC((uint32_t)dlc);

        if(mode == CAN_MODE_FD_WITH_BRS)
        {
            fifo->CAN_TXBE_1 |= CAN_TXBE_1_FDF_Msk | CAN_TXBE_1_BRS_Msk;
        }
        else if (mode == CAN_MODE_FD_WITHOUT_BRS)
        {
            fifo->CAN_TXBE_1 |= CAN_TXBE_1_FDF_Msk;
        }
        else
        {
            /* Do nothing */
        }
        if ((msgAttr == CAN_MSG_ATTR_TX_BUFFER_DATA_FRAME) || (msgAttr == CAN_MSG_ATTR_TX_FIFO_DATA_FRAME))
        {
            /* copy the data into the payload */
            memcpy((uint8_t *)&fifo->CAN_TXBE_DATA, data, length);
        }
        else if (msgAttr == CAN_MSG_ATTR_TX_BUFFER_RTR_FRAME || msgAttr == CAN_MSG_ATTR_TX_FIFO_RTR_FRAME)
        {
            fifo->CAN_TXBE_0 |= CAN_TXBE_0_RTR_Msk;
        }
        else
        {
            /* Do nothing */
        }

        messageMarker++;
        fifo->CAN_TXBE_1 |= (((uint32_t)(messageMarker) << CAN_TXBE_1_MM_Pos) & CAN_TXBE_1_MM_Msk) | CAN_TXBE_1_EFC_Msk;

        CAN1_REGS->CAN_TXBTIE = 1UL << tfqpi;

        /* request the transmit */
        CAN1_REGS->CAN_TXBAR = 1UL << tfqpi;

        CAN1_REGS->CAN_IE |= CAN_IE_TCE_Msk;
    }
    return op_success;
}

// *****************************************************************************
/* Function:
    bool CAN1_MessageReceive(uint32_t *id, uint8_t *length, uint8_t *data, uint16_t *timestamp,
                                             CAN_MSG_RX_ATTRIBUTE msgAttr, CAN_MSG_RX_FRAME_ATTRIBUTE *msgFrameAttr)

   Summary:
    Receives a message from CAN bus.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id           - Pointer to 11-bit / 29-bit identifier (ID) to be received.
    length       - Pointer to data length in number of bytes to be received.
    data         - pointer to destination data buffer
    timestamp    - Pointer to Rx message timestamp, timestamp value is 0 if timestamp is disabled
    msgAttr      - Message to be read from Rx FIFO0 or Rx FIFO1 or Rx Buffer
    msgFrameAttr - Data frame or Remote frame to be received

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_MessageReceive(uint32_t *id, uint8_t *length, uint8_t *data, uint16_t *timestamp,
                                         CAN_MSG_RX_ATTRIBUTE msgAttr, CAN_MSG_RX_FRAME_ATTRIBUTE *msgFrameAttr)
{
    uint8_t bufferIndex = 0U;
    bool status = false;

    switch (msgAttr)
    {
        case CAN_MSG_ATTR_RX_BUFFER:
            for (bufferIndex = 0U; bufferIndex < 1U; bufferIndex++)
            {
                if (bufferIndex < 32U)
                {
                    if ((can1Obj.rxBufferIndex1 & (1UL << (bufferIndex & 0x1FU))) == 0U)
                    {
                        can1Obj.rxBufferIndex1 |= (1UL << (bufferIndex & 0x1FU));
                        break;
                    }
                }
                else if ((can1Obj.rxBufferIndex2 & (1UL << ((bufferIndex - 32U) & 0x1FU))) == 0U)
                {
                    can1Obj.rxBufferIndex2 |= (1UL << ((bufferIndex - 32U) & 0x1FU));
                    break;
                }
                else
                {
                    /* Do nothing */
                }
            }
            if(bufferIndex == 1U)
            {
                /* The Rx buffers are full */
                return false;
            }
            can1RxMsg[msgAttr][bufferIndex].rxId = id;
            can1RxMsg[msgAttr][bufferIndex].rxBuffer = data;
            can1RxMsg[msgAttr][bufferIndex].rxsize = length;
            can1RxMsg[msgAttr][bufferIndex].timestamp = timestamp;
            can1RxMsg[msgAttr][bufferIndex].msgFrameAttr = msgFrameAttr;
            CAN1_REGS->CAN_IE |= CAN_IE_DRXE_Msk;
            status = true;
            break;
        case CAN_MSG_ATTR_RX_FIFO0:
            bufferIndex = (uint8_t)((CAN1_REGS->CAN_RXF0S & CAN_RXF0S_F0GI_Msk) >> CAN_RXF0S_F0GI_Pos);
            can1RxMsg[msgAttr][bufferIndex].rxId = id;
            can1RxMsg[msgAttr][bufferIndex].rxBuffer = data;
            can1RxMsg[msgAttr][bufferIndex].rxsize = length;
            can1RxMsg[msgAttr][bufferIndex].timestamp = timestamp;
            can1RxMsg[msgAttr][bufferIndex].msgFrameAttr = msgFrameAttr;
            CAN1_REGS->CAN_IE |= CAN_IE_RF0NE_Msk;
            status = true;
            break;
        case CAN_MSG_ATTR_RX_FIFO1:
            bufferIndex = (uint8_t)((CAN1_REGS->CAN_RXF1S & CAN_RXF1S_F1GI_Msk) >> CAN_RXF1S_F1GI_Pos);
            can1RxMsg[msgAttr][bufferIndex].rxId = id;
            can1RxMsg[msgAttr][bufferIndex].rxBuffer = data;
            can1RxMsg[msgAttr][bufferIndex].rxsize = length;
            can1RxMsg[msgAttr][bufferIndex].timestamp = timestamp;
            can1RxMsg[msgAttr][bufferIndex].msgFrameAttr = msgFrameAttr;
            CAN1_REGS->CAN_IE |= CAN_IE_RF1NE_Msk;
            status = true;
            break;
        default:
            /* Do nothing */
            break;
    }
    return status;
}

// *****************************************************************************
/* Function:
    bool CAN1_TransmitEventFIFOElementGet(uint32_t *id, uint8_t *messageMarker, uint16_t *timestamp)

   Summary:
    Get the Transmit Event FIFO Element for the transmitted message.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id            - Pointer to 11-bit / 29-bit identifier (ID) to be received.
    messageMarker - Pointer to Tx message message marker number to be received
    timestamp     - Pointer to Tx message timestamp to be received, timestamp value is 0 if Timestamp is disabled

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_TransmitEventFIFOElementGet(uint32_t *id, uint8_t *messageMarker, uint16_t *timestamp)
{
    can_txefe_registers_t *txEventFIFOElement = NULL;
    uint8_t txefgi = 0U;
    bool status = false;

    /* Check if Tx Event FIFO Element available */
    if ((CAN1_REGS->CAN_TXEFS & CAN_TXEFS_EFFL_Msk) != 0U)
    {
        /* Get a pointer to Tx Event FIFO Element */
        txefgi = (uint8_t)((CAN1_REGS->CAN_TXEFS & CAN_TXEFS_EFGI_Msk) >> CAN_TXEFS_EFGI_Pos);
        txEventFIFOElement = (can_txefe_registers_t *) ((uint8_t *)can1Obj.msgRAMConfig.txEventFIFOAddress + ((uint32_t)txefgi * sizeof(can_txefe_registers_t)));

        /* Check if it's a extended message type */
        if (0U != (txEventFIFOElement->CAN_TXEFE_0 & CAN_TXEFE_0_XTD_Msk))
        {
            *id = txEventFIFOElement->CAN_TXEFE_0 & CAN_TXEFE_0_ID_Msk;
        }
        else
        {
            *id = (txEventFIFOElement->CAN_TXEFE_0 >> 18U) & CAN_STD_ID_Msk;
        }

        *messageMarker = (uint8_t)((txEventFIFOElement->CAN_TXEFE_1 & CAN_TXEFE_1_MM_Msk) >> CAN_TXEFE_1_MM_Pos);

        /* Get timestamp from transmitted message */
        if (timestamp != NULL)
        {
            *timestamp = (uint16_t)(txEventFIFOElement->CAN_TXEFE_1 & CAN_TXEFE_1_TXTS_Msk);
        }

        /* Ack the Tx Event FIFO position */
        CAN1_REGS->CAN_TXEFA = CAN_TXEFA_EFAI((uint32_t)txefgi);

        /* Tx Event FIFO Element read successfully, so return true */
        status = true;
    }
    return status;
}

// *****************************************************************************
/* Function:
    CAN_ERROR CAN1_ErrorGet(void)

   Summary:
    Returns the error during transfer.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    None.

   Returns:
    Error during transfer.
*/
CAN_ERROR CAN1_ErrorGet(void)
{
    CAN_ERROR error;
    uint32_t   errorStatus = CAN1_REGS->CAN_PSR;

    error = (CAN_ERROR) ((errorStatus & CAN_PSR_LEC_Msk) | (errorStatus & CAN_PSR_EP_Msk) | (errorStatus & CAN_PSR_EW_Msk)
            | (errorStatus & CAN_PSR_BO_Msk) | (errorStatus & CAN_PSR_DLEC_Msk) | (errorStatus & CAN_PSR_PXE_Msk));

    if ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) == CAN_CCCR_INIT_Msk)
    {
        CAN1_REGS->CAN_CCCR |= CAN_CCCR_CCE_Msk;
        CAN1_REGS->CAN_CCCR = (CAN1_REGS->CAN_CCCR & ~CAN_CCCR_INIT_Msk) | CAN_CCCR_FDOE_Msk | CAN_CCCR_BRSE_Msk;
        while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) == CAN_CCCR_INIT_Msk)
        {
            /* Wait for initialization complete */
        }
    }

    return error;
}

// *****************************************************************************
/* Function:
    void CAN1_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)

   Summary:
    Returns the transmit and receive error count during transfer.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    txErrorCount - Transmit Error Count to be received
    rxErrorCount - Receive Error Count to be received

   Returns:
    None.
*/
void CAN1_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)
{
    *txErrorCount = (uint8_t)(CAN1_REGS->CAN_ECR & CAN_ECR_TEC_Msk);
    *rxErrorCount = (uint8_t)((CAN1_REGS->CAN_ECR & CAN_ECR_REC_Msk) >> CAN_ECR_REC_Pos);
}

// *****************************************************************************
/* Function:
    bool CAN1_InterruptGet(CAN_INTERRUPT_MASK interruptMask)

   Summary:
    Returns the Interrupt status.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    interruptMask - Interrupt source number

   Returns:
    true - Requested interrupt is occurred.
    false - Requested interrupt is not occurred.
*/
bool CAN1_InterruptGet(CAN_INTERRUPT_MASK interruptMask)
{
    return ((CAN1_REGS->CAN_IR & (uint32_t)interruptMask) != 0x0U);
}

// *****************************************************************************
/* Function:
    void CAN1_InterruptClear(CAN_INTERRUPT_MASK interruptMask)

   Summary:
    Clears Interrupt status.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    interruptMask - Interrupt to be cleared

   Returns:
    None
*/
void CAN1_InterruptClear(CAN_INTERRUPT_MASK interruptMask)
{
    CAN1_REGS->CAN_IR = (uint32_t)interruptMask;
}

// *****************************************************************************
/* Function:
    bool CAN1_TxFIFOIsFull(void)

   Summary:
    Returns true if Tx FIFO is full otherwise false.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    None

   Returns:
    true  - Tx FIFO is full.
    false - Tx FIFO is not full.
*/
bool CAN1_TxFIFOIsFull(void)
{
    return ((CAN1_REGS->CAN_TXFQS & CAN_TXFQS_TFQF_Msk) == CAN_TXFQS_TFQF_Msk);
}

// *****************************************************************************
/* Function:
    void CAN1_MessageRAMConfigSet(uint8_t *msgRAMConfigBaseAddress)

   Summary:
    Set the Message RAM Configuration.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    msgRAMConfigBaseAddress - Pointer to application allocated buffer base address.
                              Application must allocate buffer from non-cached
                              contiguous memory and buffer size must be
                              CAN1_MESSAGE_RAM_CONFIG_SIZE

   Returns:
    None
*/
void CAN1_MessageRAMConfigSet(uint8_t *msgRAMConfigBaseAddress)
{
    uint32_t offset = 0U;

    memset(msgRAMConfigBaseAddress, 0x00, CAN1_MESSAGE_RAM_CONFIG_SIZE);

    /* Set CAN CCCR Init for Message RAM Configuration */
    CAN1_REGS->CAN_CCCR = CAN_CCCR_INIT_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) != CAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }

    /* Set CCE to unlock the configuration registers */
    CAN1_REGS->CAN_CCCR |= CAN_CCCR_CCE_Msk;

    can1Obj.msgRAMConfig.rxFIFO0Address = (can_rxf0e_registers_t *)msgRAMConfigBaseAddress;
    offset = CAN1_RX_FIFO0_SIZE;
    /* Receive FIFO 0 Configuration Register */
    CAN1_REGS->CAN_RXF0C = CAN_RXF0C_F0S(1UL) | CAN_RXF0C_F0WM(0UL) | CAN_RXF0C_F0OM_Msk |
            CAN_RXF0C_F0SA((uint32_t)can1Obj.msgRAMConfig.rxFIFO0Address);

    can1Obj.msgRAMConfig.rxFIFO1Address = (can_rxf1e_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += CAN1_RX_FIFO1_SIZE;
    /* Receive FIFO 1 Configuration Register */
    CAN1_REGS->CAN_RXF1C = CAN_RXF1C_F1S(1UL) | CAN_RXF1C_F1WM(0UL) | CAN_RXF1C_F1OM_Msk |
            CAN_RXF1C_F1SA((uint32_t)can1Obj.msgRAMConfig.rxFIFO1Address);

    can1Obj.msgRAMConfig.rxBuffersAddress = (can_rxbe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += CAN1_RX_BUFFER_SIZE;
    CAN1_REGS->CAN_RXBC = CAN_RXBC_RBSA((uint32_t)can1Obj.msgRAMConfig.rxBuffersAddress);

    can1Obj.msgRAMConfig.txBuffersAddress = (can_txbe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += CAN1_TX_FIFO_BUFFER_SIZE;
    /* Transmit Buffer/FIFO Configuration Register */
    CAN1_REGS->CAN_TXBC = CAN_TXBC_TFQS(1UL) |
            CAN_TXBC_TBSA((uint32_t)can1Obj.msgRAMConfig.txBuffersAddress);

    can1Obj.msgRAMConfig.txEventFIFOAddress =  (can_txefe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += CAN1_TX_EVENT_FIFO_SIZE;
    /* Transmit Event FIFO Configuration Register */
    CAN1_REGS->CAN_TXEFC = CAN_TXEFC_EFWM(0UL) | CAN_TXEFC_EFS(1UL) |
            CAN_TXEFC_EFSA((uint32_t)can1Obj.msgRAMConfig.txEventFIFOAddress);

    can1Obj.msgRAMConfig.stdMsgIDFilterAddress = (can_sidfe_registers_t *)(msgRAMConfigBaseAddress + offset);
    memcpy(can1Obj.msgRAMConfig.stdMsgIDFilterAddress,
           (const void *)can1StdFilter,
           CAN1_STD_MSG_ID_FILTER_SIZE);
    offset += CAN1_STD_MSG_ID_FILTER_SIZE;
    /* Standard ID Filter Configuration Register */
    CAN1_REGS->CAN_SIDFC = CAN_SIDFC_LSS(2UL) |
            CAN_SIDFC_FLSSA((uint32_t)can1Obj.msgRAMConfig.stdMsgIDFilterAddress);

    can1Obj.msgRAMConfig.extMsgIDFilterAddress = (can_xidfe_registers_t *)(msgRAMConfigBaseAddress + offset);
    memcpy(can1Obj.msgRAMConfig.extMsgIDFilterAddress,
           (const void *)can1ExtFilter,
           CAN1_EXT_MSG_ID_FILTER_SIZE);
    /* Extended ID Filter Configuration Register */
    CAN1_REGS->CAN_XIDFC = CAN_XIDFC_LSE(2UL) |
            CAN_XIDFC_FLESA((uint32_t)can1Obj.msgRAMConfig.extMsgIDFilterAddress);

    /* Reference offset variable once to remove warning about the variable not being used after increment */
    (void)offset;

    /* Complete Message RAM Configuration by clearing CAN CCCR Init */
    CAN1_REGS->CAN_CCCR = (CAN1_REGS->CAN_CCCR & ~CAN_CCCR_INIT_Msk) | CAN_CCCR_FDOE_Msk | CAN_CCCR_BRSE_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) == CAN_CCCR_INIT_Msk)
    {
        /* Wait for configuration complete */
    }
}

// *****************************************************************************
/* Function:
    bool CAN1_StandardFilterElementSet(uint8_t filterNumber, can_sidfe_registers_t *stdMsgIDFilterElement)

   Summary:
    Set a standard filter element configuration.

   Precondition:
    CAN1_Initialize and CAN1_MessageRAMConfigSet must have been called
    for the associated CAN instance.

   Parameters:
    filterNumber          - Standard Filter number to be configured.
    stdMsgIDFilterElement - Pointer to Standard Filter Element configuration to be set on specific filterNumber.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_StandardFilterElementSet(uint8_t filterNumber, can_sidfe_registers_t *stdMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (stdMsgIDFilterElement == NULL))
    {
        return false;
    }
    can1Obj.msgRAMConfig.stdMsgIDFilterAddress[filterNumber - 1U].CAN_SIDFE_0 = stdMsgIDFilterElement->CAN_SIDFE_0;

    return true;
}

// *****************************************************************************
/* Function:
    bool CAN1_StandardFilterElementGet(uint8_t filterNumber, can_sidfe_registers_t *stdMsgIDFilterElement)

   Summary:
    Get a standard filter element configuration.

   Precondition:
    CAN1_Initialize and CAN1_MessageRAMConfigSet must have been called
    for the associated CAN instance.

   Parameters:
    filterNumber          - Standard Filter number to get filter configuration.
    stdMsgIDFilterElement - Pointer to Standard Filter Element configuration for storing filter configuration.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_StandardFilterElementGet(uint8_t filterNumber, can_sidfe_registers_t *stdMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (stdMsgIDFilterElement == NULL))
    {
        return false;
    }
    stdMsgIDFilterElement->CAN_SIDFE_0 = can1Obj.msgRAMConfig.stdMsgIDFilterAddress[filterNumber - 1U].CAN_SIDFE_0;

    return true;
}

// *****************************************************************************
/* Function:
    bool CAN1_ExtendedFilterElementSet(uint8_t filterNumber, can_xidfe_registers_t *extMsgIDFilterElement)

   Summary:
    Set a Extended filter element configuration.

   Precondition:
    CAN1_Initialize and CAN1_MessageRAMConfigSet must have been called
    for the associated CAN instance.

   Parameters:
    filterNumber          - Extended Filter number to be configured.
    extMsgIDFilterElement - Pointer to Extended Filter Element configuration to be set on specific filterNumber.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_ExtendedFilterElementSet(uint8_t filterNumber, can_xidfe_registers_t *extMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (extMsgIDFilterElement == NULL))
    {
        return false;
    }
    can1Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].CAN_XIDFE_0 = extMsgIDFilterElement->CAN_XIDFE_0;
    can1Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].CAN_XIDFE_1 = extMsgIDFilterElement->CAN_XIDFE_1;

    return true;
}

// *****************************************************************************
/* Function:
    bool CAN1_ExtendedFilterElementGet(uint8_t filterNumber, can_xidfe_registers_t *extMsgIDFilterElement)

   Summary:
    Get a Extended filter element configuration.

   Precondition:
    CAN1_Initialize and CAN1_MessageRAMConfigSet must have been called
    for the associated CAN instance.

   Parameters:
    filterNumber          - Extended Filter number to get filter configuration.
    extMsgIDFilterElement - Pointer to Extended Filter Element configuration for storing filter configuration.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_ExtendedFilterElementGet(uint8_t filterNumber, can_xidfe_registers_t *extMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (extMsgIDFilterElement == NULL))
    {
        return false;
    }
    extMsgIDFilterElement->CAN_XIDFE_0 = can1Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].CAN_XIDFE_0;
    extMsgIDFilterElement->CAN_XIDFE_1 = can1Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].CAN_XIDFE_1;

    return true;
}

void CAN1_SleepModeEnter(void)
{
    CAN1_REGS->CAN_CCCR |=  CAN_CCCR_CSR_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_CSA_Msk) != CAN_CCCR_CSA_Msk)
    {
        /* Wait for clock stop request to complete */
    }
}

void CAN1_SleepModeExit(void)
{
    CAN1_REGS->CAN_CCCR &=  ~CAN_CCCR_CSR_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_CSA_Msk) == CAN_CCCR_CSA_Msk)
    {
        /* Wait for no clock stop */
    }
    CAN1_REGS->CAN_CCCR &= ~CAN_CCCR_INIT_Msk;
    while ((CAN1_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) == CAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }
}

// *****************************************************************************
/* Function:
    void CAN1_TxCallbackRegister(CAN_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given CAN's transfer events occur.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the CAN_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void CAN1_TxCallbackRegister(CAN_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    can1CallbackObj[CAN_CALLBACK_TX_INDEX].callback = callback;
    can1CallbackObj[CAN_CALLBACK_TX_INDEX].context = contextHandle;
}

// *****************************************************************************
/* Function:
    void CAN1_RxCallbackRegister(CAN_CALLBACK callback, uintptr_t contextHandle, CAN_MSG_RX_ATTRIBUTE msgAttr)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given CAN's transfer events occur.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the CAN_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

    msgAttr   - Message to be read from Rx FIFO0 or Rx FIFO1 or Rx Buffer

   Returns:
    None.
*/
void CAN1_RxCallbackRegister(CAN_CALLBACK callback, uintptr_t contextHandle, CAN_MSG_RX_ATTRIBUTE msgAttr)
{
    if (callback == NULL)
    {
        return;
    }

    can1CallbackObj[msgAttr].callback = callback;
    can1CallbackObj[msgAttr].context = contextHandle;
}

// *****************************************************************************
/* Function:
    void CAN1_InterruptHandler(void)

   Summary:
    CAN1 Peripheral Interrupt Handler.

   Description:
    This function is CAN1 Peripheral Interrupt Handler and will
    called on every CAN1 interrupt.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None.

   Remarks:
    The function is called as peripheral instance's interrupt handler if the
    instance interrupt is enabled. If peripheral instance's interrupt is not
    enabled user need to call it from the main while loop of the application.
*/
void CAN1_InterruptHandler(void)
{
    uint8_t length = 0U;
    uint8_t rxgi = 0U;
    uint8_t bufferIndex = 0U;
    bool testCondition = false;
    can_rxbe_registers_t *rxbeFifo = NULL;
    can_rxf0e_registers_t *rxf0eFifo = NULL;
    can_rxf1e_registers_t *rxf1eFifo = NULL;
    uint32_t ir = CAN1_REGS->CAN_IR;

    /* Check if error occurred */
    if ((ir & CAN_IR_BO_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_BO_Msk;
    }
    /* New Message in Rx FIFO 0 */
    if ((ir & CAN_IR_RF0N_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_RF0N_Msk;
        CAN1_REGS->CAN_IE &= (~CAN_IE_RF0NE_Msk);

        if ((CAN1_REGS->CAN_RXF0S & CAN_RXF0S_F0FL_Msk) != 0U)
        {
            /* Read data from the Rx FIFO0 */
            rxgi = (uint8_t)((CAN1_REGS->CAN_RXF0S & CAN_RXF0S_F0GI_Msk) >> CAN_RXF0S_F0GI_Pos);
            rxf0eFifo = (can_rxf0e_registers_t *) ((uint8_t *)can1Obj.msgRAMConfig.rxFIFO0Address + ((uint32_t)rxgi * CAN1_RX_FIFO0_ELEMENT_SIZE));

            /* Get received identifier */
            if ((rxf0eFifo->CAN_RXF0E_0 & CAN_RXF0E_0_XTD_Msk) != 0U)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].rxId = rxf0eFifo->CAN_RXF0E_0 & CAN_RXF0E_0_ID_Msk;
            }
            else
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].rxId = (rxf0eFifo->CAN_RXF0E_0 >> 18) & CAN_STD_ID_Msk;
            }

            /* Check RTR and FDF bits for Remote/Data Frame */
            testCondition = ((rxf0eFifo->CAN_RXF0E_0 & CAN_RXF0E_0_RTR_Msk) != 0U);
            testCondition = ((rxf0eFifo->CAN_RXF0E_1 & CAN_RXF0E_1_FDF_Msk) == 0U) && testCondition;
            if (testCondition)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].msgFrameAttr = CAN_MSG_RX_REMOTE_FRAME;
            }
            else
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].msgFrameAttr = CAN_MSG_RX_DATA_FRAME;
            }

            /* Get received data length */
            length = CANDlcToLengthGet((uint8_t)((rxf0eFifo->CAN_RXF0E_1 & CAN_RXF0E_1_DLC_Msk) >> CAN_RXF0E_1_DLC_Pos));

            /* Copy data to user buffer */
            memcpy(can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].rxBuffer, (uint8_t *)&rxf0eFifo->CAN_RXF0E_DATA, length);
            *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].rxsize = length;

            /* Get timestamp from received message */
            if (can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].timestamp != NULL)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO0][rxgi].timestamp = (uint16_t)(rxf0eFifo->CAN_RXF0E_1 & CAN_RXF0E_1_RXTS_Msk);
            }

            /* Ack the fifo position */
            CAN1_REGS->CAN_RXF0A = CAN_RXF0A_F0AI((uint32_t)rxgi);

            if (can1CallbackObj[CAN_MSG_ATTR_RX_FIFO0].callback != NULL)
            {
                can1CallbackObj[CAN_MSG_ATTR_RX_FIFO0].callback(can1CallbackObj[CAN_MSG_ATTR_RX_FIFO0].context);
            }
        }
    }
    /* New Message in Rx FIFO 1 */
    if ((ir & CAN_IR_RF1N_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_RF1N_Msk;
        CAN1_REGS->CAN_IE &= (~CAN_IE_RF1NE_Msk);

        if ((CAN1_REGS->CAN_RXF1S & CAN_RXF1S_F1FL_Msk) != 0U)
        {
            /* Read data from the Rx FIFO1 */
            rxgi = (uint8_t)((CAN1_REGS->CAN_RXF1S & CAN_RXF1S_F1GI_Msk) >> CAN_RXF1S_F1GI_Pos);
            rxf1eFifo = (can_rxf1e_registers_t *) ((uint8_t *)can1Obj.msgRAMConfig.rxFIFO1Address + ((uint32_t)rxgi * CAN1_RX_FIFO1_ELEMENT_SIZE));

            /* Get received identifier */
            if ((rxf1eFifo->CAN_RXF1E_0 & CAN_RXF1E_0_XTD_Msk) != 0U)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].rxId = rxf1eFifo->CAN_RXF1E_0 & CAN_RXF1E_0_ID_Msk;
            }
            else
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].rxId = (rxf1eFifo->CAN_RXF1E_0 >> 18) & CAN_STD_ID_Msk;
            }

            /* Check RTR and FDF bits for Remote/Data Frame */
            testCondition = ((rxf1eFifo->CAN_RXF1E_0 & CAN_RXF1E_0_RTR_Msk) != 0U);
            testCondition = ((rxf1eFifo->CAN_RXF1E_1 & CAN_RXF1E_1_FDF_Msk) == 0U) && testCondition;
            if (testCondition)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].msgFrameAttr = CAN_MSG_RX_REMOTE_FRAME;
            }
            else
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].msgFrameAttr = CAN_MSG_RX_DATA_FRAME;
            }

            /* Get received data length */
            length = CANDlcToLengthGet((uint8_t)((rxf1eFifo->CAN_RXF1E_1 & CAN_RXF1E_1_DLC_Msk) >> CAN_RXF1E_1_DLC_Pos));

            /* Copy data to user buffer */
            memcpy(can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].rxBuffer, (uint8_t *)&rxf1eFifo->CAN_RXF1E_DATA, length);
            *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].rxsize = length;

            /* Get timestamp from received message */
            if (can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].timestamp != NULL)
            {
                *can1RxMsg[CAN_MSG_ATTR_RX_FIFO1][rxgi].timestamp = (uint16_t)(rxf1eFifo->CAN_RXF1E_1 & CAN_RXF1E_1_RXTS_Msk);
            }

            /* Ack the fifo position */
            CAN1_REGS->CAN_RXF1A = CAN_RXF1A_F1AI((uint32_t)rxgi);

            if (can1CallbackObj[CAN_MSG_ATTR_RX_FIFO1].callback != NULL)
            {
                can1CallbackObj[CAN_MSG_ATTR_RX_FIFO1].callback(can1CallbackObj[CAN_MSG_ATTR_RX_FIFO1].context);
            }
        }
    }
    /* New Message in Dedicated Rx Buffer */
    if ((ir & CAN_IR_DRX_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_DRX_Msk;
        CAN1_REGS->CAN_IE &= (~CAN_IE_DRXE_Msk);

        /* Read data from the Rx Buffer */
        if (CAN1_REGS->CAN_NDAT1 != 0U)
        {
            for (rxgi = 0U; rxgi <= 0x1FU; rxgi++)
            {
                if ((CAN1_REGS->CAN_NDAT1 & (1UL << rxgi)) == (1UL << rxgi))
                {
                    break;
                }
            }
        }
        else
        {
            for (rxgi = 0U; rxgi <= 0x1FU; rxgi++)
            {
                if ((CAN1_REGS->CAN_NDAT2 & (1UL << rxgi)) == (1UL << rxgi))
                {
                    rxgi = rxgi + 32U;
                    break;
                }
            }
        }
        rxbeFifo = (can_rxbe_registers_t *) ((uint8_t *)can1Obj.msgRAMConfig.rxBuffersAddress + ((uint32_t)rxgi * CAN1_RX_BUFFER_ELEMENT_SIZE));

        for (bufferIndex = 0U; bufferIndex < 1U; bufferIndex++)
        {
            if (bufferIndex < 32U)
            {
                if ((can1Obj.rxBufferIndex1 & (1UL << (bufferIndex & 0x1FU))) == (1UL << (bufferIndex & 0x1FU)))
                {
                    can1Obj.rxBufferIndex1 &= ~(1UL << (bufferIndex & 0x1FU));
                    break;
                }
            }
            else if ((can1Obj.rxBufferIndex2 & (1UL << ((bufferIndex - 32U) & 0x1FU))) == (1UL << ((bufferIndex - 32U) & 0x1FU)))
            {
                can1Obj.rxBufferIndex2 &= ~(1UL << ((bufferIndex - 32U) & 0x1FU));
                break;
            }
            else
            {
                /* Do nothing */
            }
        }

        if(bufferIndex >= NUM_RX_BUFFER_ELEMENTS)
        {
            bufferIndex = (uint8_t)(NUM_RX_BUFFER_ELEMENTS - 1U);
        }

        /* Get received identifier */
        if ((rxbeFifo->CAN_RXBE_0 & CAN_RXBE_0_XTD_Msk) != 0U)
        {
            *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].rxId = rxbeFifo->CAN_RXBE_0 & CAN_RXBE_0_ID_Msk;
        }
        else
        {
            *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].rxId = (rxbeFifo->CAN_RXBE_0 >> 18U) & CAN_STD_ID_Msk;
        }

        /* Check RTR and FDF bits for Remote/Data Frame */
        testCondition = ((rxbeFifo->CAN_RXBE_0 & CAN_RXBE_0_RTR_Msk) != 0U);
        testCondition  = ((rxbeFifo->CAN_RXBE_1 & CAN_RXBE_1_FDF_Msk) == 0U) && testCondition;
        if (testCondition)
        {
            *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].msgFrameAttr = CAN_MSG_RX_REMOTE_FRAME;
        }
        else
        {
            *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].msgFrameAttr = CAN_MSG_RX_DATA_FRAME;
        }

        /* Get received data length */
        length = CANDlcToLengthGet((uint8_t)((rxbeFifo->CAN_RXBE_1 & CAN_RXBE_1_DLC_Msk) >> CAN_RXBE_1_DLC_Pos));

        /* Copy data to user buffer */
        memcpy(can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].rxBuffer, (uint8_t *)&rxbeFifo->CAN_RXBE_DATA, length);
        *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].rxsize = length;

        /* Get timestamp from received message */
        if (can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].timestamp != NULL)
        {
            *can1RxMsg[CAN_MSG_ATTR_RX_BUFFER][bufferIndex].timestamp = (uint16_t)(rxbeFifo->CAN_RXBE_1 & CAN_RXBE_1_RXTS_Msk);
        }

        /* Clear new data flag */
        if (rxgi < 32U)
        {
            CAN1_REGS->CAN_NDAT1 = (1UL << rxgi);
        }
        else
        {
            CAN1_REGS->CAN_NDAT2 = (1UL << (rxgi - 32U));
        }
        if (can1CallbackObj[CAN_MSG_ATTR_RX_BUFFER].callback != NULL)
        {
            can1CallbackObj[CAN_MSG_ATTR_RX_BUFFER].callback(can1CallbackObj[CAN_MSG_ATTR_RX_BUFFER].context);
        }
    }

    /* TX Completed */
    if ((ir & CAN_IR_TC_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_TC_Msk;
        CAN1_REGS->CAN_IE &= (~CAN_IE_TCE_Msk);
        for (bufferIndex = 0U; bufferIndex < (CAN1_TX_FIFO_BUFFER_SIZE/CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE); bufferIndex++)
        {
            uint32_t txbufferMask = (1UL << ((uint32_t)bufferIndex & 0x1FU));
            testCondition = ((CAN1_REGS->CAN_TXBTO & txbufferMask) != 0U);
            testCondition = ((CAN1_REGS->CAN_TXBTIE & txbufferMask) != 0U) && testCondition;
            if (testCondition)
            {
                CAN1_REGS->CAN_TXBTIE &= ~txbufferMask;
            }
        }
        if (can1CallbackObj[CAN_CALLBACK_TX_INDEX].callback != NULL)
        {
            can1CallbackObj[CAN_CALLBACK_TX_INDEX].callback(can1CallbackObj[CAN_CALLBACK_TX_INDEX].context);
        }
    }

    /* TX FIFO is empty */
    if ((ir & CAN_IR_TFE_Msk) != 0U)
    {
        CAN1_REGS->CAN_IR = CAN_IR_TFE_Msk;
        uint8_t getIndex = (uint8_t)((CAN1_REGS->CAN_TXFQS & CAN_TXFQS_TFGI_Msk) >> CAN_TXFQS_TFGI_Pos);
        uint8_t putIndex = (uint8_t)((CAN1_REGS->CAN_TXFQS & CAN_TXFQS_TFQPI_Msk) >> CAN_TXFQS_TFQPI_Pos);
        for (uint8_t fifoIndex = getIndex; ; fifoIndex++)
        {
            if (fifoIndex >= putIndex)
            {
                break;
            }
        }
    }
}


/*******************************************************************************
 End of File
*/

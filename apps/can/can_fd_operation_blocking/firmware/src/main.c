/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

uint8_t Can1MessageRAM[CAN1_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32)));

/* Standard identifier id[28:18]*/
#define WRITE_ID(id) (id << 18)
#define READ_ID(id)  (id >> 18)

static uint32_t status = 0;
static uint8_t loop_count = 0;
static uint8_t user_input = 0;

static uint8_t txFiFo[CAN1_TX_FIFO_BUFFER_SIZE];
static uint8_t rxFiFo0[CAN1_RX_FIFO0_SIZE];
static uint8_t rxFiFo1[CAN1_RX_FIFO1_SIZE];
static uint8_t rxBuffer[CAN1_RX_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: Local functions
// *****************************************************************************
// *****************************************************************************

/* Message Length to Data length code */
static uint8_t CANLengthToDlcGet(uint8_t length)
{
    uint8_t dlc = 0;

    if (length <= 8U)
    {
        dlc = length;
    }
    else if (length <= 12U)
    {
        dlc = 0x9U;
    }
    else if (length <= 16U)
    {
        dlc = 0xAU;
    }
    else if (length <= 20U)
    {
        dlc = 0xBU;
    }
    else if (length <= 24U)
    {
        dlc = 0xCU;
    }
    else if (length <= 32U)
    {
        dlc = 0xDU;
    }
    else if (length <= 48U)
    {
        dlc = 0xEU;
    }
    else
    {
        dlc = 0xFU;
    }
    return dlc;
}

/* Data length code to Message Length */
static uint8_t CANDlcToLengthGet(uint8_t dlc)
{
    uint8_t msgLength[] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
    return msgLength[dlc];
}

/* Menu */
static void display_menu(void)
{
	printf("Menu :\r\n"
	       "  -- Select the action:\r\n"
	       "  0: Send FD standard message with ID: 0x45A and 64 byte data 0 to 63. \r\n"
	       "  1: Send FD standard message with ID: 0x469 and 64 byte data 128 to 191. \r\n"
	       "  2: Send FD extended message with ID: 0x100000A5 and 64 byte data 0 to 63. \r\n"
	       "  3: Send FD extended message with ID: 0x10000096 and 64 byte data 128 to 191. \r\n"
	       "  4: Send normal standard message with ID: 0x469 and 8 byte data 0 to 7. \r\n"
	       "  m: Display menu \r\n\r\n");
}

/* Print Rx Message */
static void print_message(uint8_t numberOfMessage, CAN_RX_BUFFER *rxBuf, uint8_t rxBufLen, uint8_t rxFifoBuf)
{
    uint8_t length = 0;
    uint8_t msgLength = 0;
    uint32_t id = 0;

    if (rxFifoBuf == 0)
        printf(" Rx FIFO0 :");
    else if (rxFifoBuf == 1)
        printf(" Rx FIFO1 :");
    else if (rxFifoBuf == 2)
        printf(" Rx Buffer :");

    for (uint8_t count = 0; count < numberOfMessage; count++)
    {
        /* Print message to Console */
        printf(" New Message Received\r\n");
        id = rxBuf->xtd ? rxBuf->id : READ_ID(rxBuf->id);
        msgLength = CANDlcToLengthGet(rxBuf->dlc);
        length = msgLength;
        printf(" Message - ID : 0x%x Length : 0x%x ", (unsigned int)id, (unsigned int)msgLength);
        printf("Message : ");
        while(length)
        {
            printf("0x%x ", rxBuf->data[msgLength - length--]);
        }
        printf("\r\n");
        rxBuf += rxBufLen;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    CAN_TX_BUFFER *txBuffer = NULL;
    uint8_t        bufferNumber = 0;
    uint8_t        numberOfMessage = 0;

    /* Initialize all modules */
    SYS_Initialize ( NULL );

    printf(" ------------------------------ \r\n");
    printf("            CAN FD Demo          \r\n");
    printf(" ------------------------------ \r\n");
    
    /* Set Message RAM Configuration */
    CAN1_MessageRAMConfigSet(Can1MessageRAM);

    display_menu();
     
    while ( true )
    {
        /* Rx Buffers */
        if (CAN1_InterruptGet(CAN_INTERRUPT_DRX_MASK))
        {    
            CAN1_InterruptClear(CAN_INTERRUPT_DRX_MASK);

            /* Check CAN Status */
            status = CAN1_ErrorGet();

            if (((status & CAN_PSR_LEC_Msk) == CAN_ERROR_NONE) || ((status & CAN_PSR_LEC_Msk) == CAN_ERROR_LEC_NC))
            {
                if (CAN1_RxBufferNumberGet(&bufferNumber))
                {
                    memset(rxBuffer, 0x00, CAN1_RX_BUFFER_ELEMENT_SIZE);
                    if (CAN1_MessageReceive(bufferNumber, (CAN_RX_BUFFER *)rxBuffer) == true)
                    {
                        print_message(1, (CAN_RX_BUFFER *)rxBuffer, CAN1_RX_BUFFER_ELEMENT_SIZE, 2);
                    }
                    else
                    {
                        printf(" Error in received message\r\n");
                    }
                }
            }
            else
            {
                printf(" Error in received message\r\n");
            }
        }

        /* Rx FIFO0 */
        if (CAN1_InterruptGet(CAN_INTERRUPT_RF0N_MASK))
        {    
            CAN1_InterruptClear(CAN_INTERRUPT_RF0N_MASK);

            /* Check CAN Status */
            status = CAN1_ErrorGet();

            if (((status & CAN_PSR_LEC_Msk) == CAN_ERROR_NONE) || ((status & CAN_PSR_LEC_Msk) == CAN_ERROR_LEC_NC))
            {
                numberOfMessage = CAN1_RxFifoFillLevelGet(CAN_RX_FIFO_0);
                if (numberOfMessage != 0)
                {
                    memset(rxFiFo0, 0x00, (numberOfMessage * CAN1_RX_FIFO0_ELEMENT_SIZE));
                    if (CAN1_MessageReceiveFifo(CAN_RX_FIFO_0, numberOfMessage, (CAN_RX_BUFFER *)rxFiFo0) == true)
                    {
                        print_message(numberOfMessage, (CAN_RX_BUFFER *)rxFiFo0, CAN1_RX_FIFO0_ELEMENT_SIZE, 0);
                    }
                    else
                    {
                        printf(" Error in received message\r\n");
                    }
                }
            }
            else
            {
                printf(" Error in received message\r\n");
            }
        }

        /* Rx FIFO1 */
        if (CAN1_InterruptGet(CAN_INTERRUPT_RF1N_MASK))
        {    
            CAN1_InterruptClear(CAN_INTERRUPT_RF1N_MASK);

            /* Check CAN Status */
            status = CAN1_ErrorGet();

            if (((status & CAN_PSR_LEC_Msk) == CAN_ERROR_NONE) || ((status & CAN_PSR_LEC_Msk) == CAN_ERROR_LEC_NC))
            {
                numberOfMessage = CAN1_RxFifoFillLevelGet(CAN_RX_FIFO_1);
                if (numberOfMessage != 0)
                {
                    memset(rxFiFo1, 0x00, (numberOfMessage * CAN1_RX_FIFO1_ELEMENT_SIZE));
                    if (CAN1_MessageReceiveFifo(CAN_RX_FIFO_1, numberOfMessage, (CAN_RX_BUFFER *)rxFiFo1) == true)
                    {
                        print_message(numberOfMessage, (CAN_RX_BUFFER *)rxFiFo1, CAN1_RX_FIFO1_ELEMENT_SIZE, 1);
                    }
                    else
                    {
                        printf(" Error in received message\r\n");
                    }
                }
            }
            else
            {
                printf(" Error in received message\r\n");
            }
        }

        /* User input */
        if (SERCOM2_USART_ReceiverIsReady() == false)
        {
            continue;
        }
        user_input = (uint8_t)SERCOM2_USART_ReadByte();

        switch (user_input)
        {
            case '0':
                memset(txFiFo, 0x00, CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
                txBuffer = (CAN_TX_BUFFER *)txFiFo;
                txBuffer->id = WRITE_ID(0x45A);
                txBuffer->dlc = CANLengthToDlcGet(64);
                txBuffer->fdf = 1;
                txBuffer->brs = 1;
                for (loop_count = 0; loop_count < 64; loop_count++){
                    txBuffer->data[loop_count] = loop_count;
                }                
                printf("  0: Send FD standard message with ID: 0x45A and 64 byte data 0 to 63.\r\n");
                if (CAN1_MessageTransmitFifo(1, txBuffer) == true)
                {    
                    printf(" Success \r\n");
                }
                else
                {
                    printf(" Failed \r\n");
                }             
                break;  
            case '1':
                memset(txFiFo, 0x00, CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
                txBuffer = (CAN_TX_BUFFER *)txFiFo;
                txBuffer->id = WRITE_ID(0x469);
                txBuffer->dlc = CANLengthToDlcGet(64);
                txBuffer->fdf = 1;
                txBuffer->brs = 1;
                for (loop_count = 128; loop_count < 192; loop_count++){
                    txBuffer->data[loop_count - 128] = loop_count;
                }                
                printf("  1: Send FD standard message with ID: 0x469 and 64 byte data 128 to 191.\r\n");
                if (CAN1_MessageTransmitFifo(1, txBuffer) == true)
                {    
                    printf(" Success \r\n");
                }
                else
                {
                    printf(" Failed \r\n");
                }    
                break;
            case '2': 
                memset(txFiFo, 0x00, CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
                txBuffer = (CAN_TX_BUFFER *)txFiFo;
                txBuffer->id = 0x100000A5;
                txBuffer->dlc = CANLengthToDlcGet(64);
                txBuffer->xtd = 1;
                txBuffer->fdf = 1;
                txBuffer->brs = 1;
                for (loop_count = 0; loop_count < 64; loop_count++){
                    txBuffer->data[loop_count] = loop_count;
                }
                printf("  2: Send FD extended message with ID: 0x100000A5 and 64 byte data 0 to 63.\r\n");
                if (CAN1_MessageTransmitFifo(1, txBuffer) == true)
                {    
                    printf(" Success \r\n");
                }
                else
                {
                    printf(" Failed \r\n");
                }             
                break;
            case '3':
                memset(txFiFo, 0x00, CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
                txBuffer = (CAN_TX_BUFFER *)txFiFo;
                txBuffer->id = 0x10000096;
                txBuffer->dlc = CANLengthToDlcGet(64);
                txBuffer->xtd = 1;
                txBuffer->fdf = 1;
                txBuffer->brs = 1;
                for (loop_count = 128; loop_count < 192; loop_count++){
                    txBuffer->data[loop_count - 128] = loop_count;
                }
                printf("  3: Send FD extended message with ID: 0x10000096 and 64 byte data 128 to 191.\r\n");
                if (CAN1_MessageTransmitFifo(1, txBuffer) == true)
                {    
                    printf(" Success \r\n");
                }
                else
                {
                    printf(" Failed \r\n");
                }             
                break;
            
            case '4':
                memset(txFiFo, 0x00, CAN1_TX_FIFO_BUFFER_ELEMENT_SIZE);
                txBuffer = (CAN_TX_BUFFER *)txFiFo;
                txBuffer->id = WRITE_ID(0x469);
                txBuffer->dlc = 8;
                for (loop_count = 0; loop_count < 8; loop_count++){
                    txBuffer->data[loop_count] = loop_count;
                }                
                printf("  4: Send normal standard message with ID: 0x469 and 8 byte data 0 to 7.\r\n");
                if (CAN1_MessageTransmitFifo(1, txBuffer) == true)
                {    
                    printf(" Success \r\n");
                }
                else
                {
                    printf(" Failed \r\n");
                }             
                break;                 

            case 'm':
            case 'M':
                display_menu();
                break;
                
            default:
                printf(" Invalid Input \r\n");
                break;
        }  
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

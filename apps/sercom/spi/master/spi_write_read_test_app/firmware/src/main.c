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
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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
#include <string.h>

/*
 * Protocol:
 * Master:     [WR_CMD ADDR1 ADDR0 DATA0 DATA1 .. DATAN] ... ... ... ... ... ... [RD_CMD ADDR1 ADDR0] [DUMMY DUMMY .. DUMMY]
 * Slave:      [DUMMY  DUMMY DUMMY DUMMY DUMMY .. DUMMY] ... ... ... ... ... ... [DUMMY  DUMMY DUMMY] [DATA0 DATA1 .. DATAN] 
 * BUSY PIN:    ----------------------------------------------------------------
 * ____________|                                                                |____________________________________________
 * 
 * WR_CMD = 0x02
 * RD_CMD = 0x03
 * BUSY   = 0x01
 * READY  = 0x00
 */  

typedef enum
{
	APP_STATE_INITIALIZE,
    APP_STATE_TRANSFER_COMPLETE_WAIT,    
    APP_STATE_WRITE_DATA,        
    APP_STATE_SEND_READ_DATA_CMD,
    APP_STATE_READ_DATA,
    APP_STATE_VERIFY,
    APP_STATE_ERROR,

} APP_STATES;

/* Commands */
#define APP_CMD_WRITE                       0x02
#define APP_CMD_READ                        0x03

#define APP_RX_BUFFER_SIZE                  256
#define APP_TX_BUFFER_SIZE                  256
#define APP_MEM_ADDR                        0x0010
#define APP_TEST_DATA                       "Test Data from SPI Master"
#define APP_TEST_DATA_SIZE                  strlen(APP_TEST_DATA)

#define LED_On()                            LED_Clear()
#define LED_Off()                           LED_Set()

uint8_t rxData[APP_RX_BUFFER_SIZE];
uint8_t txData[APP_TX_BUFFER_SIZE];
static APP_STATES state = APP_STATE_INITIALIZE;
APP_STATES nextState;
volatile bool isTransferDone = false;

void SPIEventHandler(uintptr_t context )
{
    isTransferDone = true;
    
    /* De-assert the CS line */
    APP_SLAVE_CS_Set();
}

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    APP_SLAVE_CS_Set();       
    
    LED_Off();
    
    /* Wait for user to press the switch */
    while (SWITCH_Get() == 1);

    while(1)
    {
        switch (state)
        {
            case APP_STATE_INITIALIZE:
                SERCOM6_SPI_CallbackRegister(SPIEventHandler, (uintptr_t) 0); 
                state = APP_STATE_WRITE_DATA;
                break;          
            
            case APP_STATE_TRANSFER_COMPLETE_WAIT:
                if (isTransferDone == true)
                {
                    isTransferDone = false;      
                    /* Wait for the SPI slave to become ready before sending next commands */         
                    while (APP_SLAVE_BUSY_PIN_Get() == 1);
                    state = nextState;
                }
                break;

            case APP_STATE_WRITE_DATA:
                                                          
                txData[0] = APP_CMD_WRITE;
                txData[1] = (APP_MEM_ADDR >> 8);
                txData[2] = APP_MEM_ADDR;
                memcpy(&txData[3], APP_TEST_DATA, APP_TEST_DATA_SIZE);
                APP_SLAVE_CS_Clear();  
                SERCOM6_SPI_WriteRead(txData, (3 + APP_TEST_DATA_SIZE), NULL, 0);
                state = APP_STATE_TRANSFER_COMPLETE_WAIT;
                nextState = APP_STATE_SEND_READ_DATA_CMD;                
                break;
                                 
            case APP_STATE_SEND_READ_DATA_CMD:
                txData[0] = APP_CMD_READ;
                txData[1] = (APP_MEM_ADDR >> 8);
                txData[2] = APP_MEM_ADDR;
                txData[3] = APP_TEST_DATA_SIZE;                
                APP_SLAVE_CS_Clear(); 
                SERCOM6_SPI_WriteRead(txData, 4, NULL, 0);   
                state = APP_STATE_TRANSFER_COMPLETE_WAIT;
                nextState = APP_STATE_READ_DATA;
                break;
                
            case APP_STATE_READ_DATA:
                APP_SLAVE_CS_Clear();
                SERCOM6_SPI_WriteRead(NULL, 0, rxData, APP_TEST_DATA_SIZE);
                state = APP_STATE_TRANSFER_COMPLETE_WAIT;
                nextState = APP_STATE_VERIFY;
                break;
                
            case APP_STATE_VERIFY: 
                if (memcmp(rxData, APP_TEST_DATA, APP_TEST_DATA_SIZE) == 0)
                {
                    LED_On();    
                    /* Repeat the test */
                    state = APP_STATE_WRITE_DATA;
                }
                else
                {
                    LED_Off();
                    state = APP_STATE_ERROR;
                }
                break;
                
            case APP_STATE_ERROR:
                break;
                
            default:
                break;
        }
    }
}

/*******************************************************************************
 End of File
*/
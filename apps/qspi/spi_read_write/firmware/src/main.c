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

/*******************************************************************************
 * QSPI Flash Device Selection (SPI Mode)
 *
 * This implementation supports both N25Q and SST26 flash memory devices.
 * Set ONE of the following defines to 1 to select the flash device:
 *
 * - APP_USE_N25Q_FLASH   : For N25Q flash devices (currently selected)
 * - APP_USE_SST26_FLASH  : For SST26 flash devices
 *
 * Only one device should be enabled at a time.
 ******************************************************************************/

#define APP_USE_N25Q_FLASH     1
#define APP_USE_SST26_FLASH    0

#if (APP_USE_N25Q_FLASH == 1) && (APP_USE_SST26_FLASH == 1)
#error "Only one flash device type should be enabled"
#endif

#if (APP_USE_N25Q_FLASH == 0) && (APP_USE_SST26_FLASH == 0)
#error "At least one flash device type must be enabled"
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>                     // Defines memcmp
#include "definitions.h"                // SYS function prototypes

// *****************************************************************************
// *****************************************************************************
// Section: Flash Command Definitions
// *****************************************************************************
// *****************************************************************************

#define APP_CMD_ENABLE_RESET                      0x66
#define APP_CMD_MEMORY_RESET                      0x99
#define APP_CMD_STATUS_REG_READ                   0x05
#define APP_CMD_CONFIG_REG_READ                   0x35
#define APP_CMD_MEMORY_READ                       0x03
#define APP_CMD_MEMORY_HIGH_SPEED_READ            0x0B
#define APP_CMD_ENABLE_WRITE                      0x06
#define APP_CMD_DISABLE_WRITE                     0x04
#define APP_CMD_4KB_SECTOR_ERASE                  0x20
#define APP_CMD_BLOCK_ERASE                       0xD8
#define APP_CMD_CHIP_ERASE                        0xC7
#define APP_CMD_PAGE_PROGRAM                      0x02
#define APP_CMD_JEDEC_ID_READ                     0x9F
#define APP_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK    0x98

#define APP_STATUS_BIT_WEL                        (0x01 << 1)
#if APP_USE_N25Q_FLASH
#define APP_STATUS_BIT_BUSY                       (0x01 << 0)
#elif APP_USE_SST26_FLASH
#define APP_STATUS_BIT_BUSY                       (0x01 << 7)
#endif

#define APP_PAGE_PROGRAM_SIZE_BYTES               256
#define APP_CS_ENABLE()                           CHIP_SELECT_Clear()
#define APP_CS_DISABLE()                          CHIP_SELECT_Set()

#define APP_MEM_ADDR                              0x10000
#define LED_On()                                  LED_Clear()
#define LED_Off()                                 LED_Set()

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    APP_STATE_INITIALIZE,
    APP_STATE_WAIT_MIN_POWER_UP_TIME,
    APP_STATE_RESET,
    APP_STATE_GLOBAL_BLK_PROTECTION_UNLOCK,
    APP_STATE_JEDEC_ID_READ,
    APP_STATE_SECTOR_ERASE,
    APP_STATE_READ_STATUS,
    APP_STATE_PAGE_PROGRAM,
    APP_STATE_MEMORY_READ,
    APP_STATE_VERIFY,
    APP_STATE_XFER_SUCCESSFUL,
    APP_STATE_XFER_ERROR,
    APP_STATE_IDLE,
} APP_STATES;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    APP_STATES          state;
    APP_STATES          nextState;
    uint8_t             transmitBuffer[APP_PAGE_PROGRAM_SIZE_BYTES + 5];
    uint8_t             manufacturerID;
    uint16_t            deviceID;
    uint8_t             isCSDeAssert;
    volatile bool       isTransferDone;
} APP_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

APP_DATA          appData;
uint8_t           writeDataBuffer[APP_PAGE_PROGRAM_SIZE_BYTES];
uint8_t           readDataBuffer[APP_PAGE_PROGRAM_SIZE_BYTES];

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void APP_Reset(void)
{
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_ENABLE_RESET;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);
    while (appData.isTransferDone == false);

    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_MEMORY_RESET;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);

    while (appData.isTransferDone == false);
}

void APP_WriteEnable(void)
{
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_ENABLE_WRITE;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);

    while (appData.isTransferDone == false);
}

void APP_WriteDisable(void)
{
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_DISABLE_WRITE;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);

    while (appData.isTransferDone == false);
}

void APP_SectorErase(uint32_t address)
{
    APP_WriteEnable();

    appData.isTransferDone = false;

    /* The address bits from A11:A0 are don't care and must be Vih or Vil */
    address = address & 0xFFFFF000;

    appData.transmitBuffer[0] = APP_CMD_4KB_SECTOR_ERASE;
    appData.transmitBuffer[1] = (address >> 16);
    appData.transmitBuffer[2] = (address >> 8);
    appData.transmitBuffer[3] = address;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 4);

    while (appData.isTransferDone == false);
}

void APP_ChipErase(void)
{
    APP_WriteEnable();

    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_CHIP_ERASE;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);

    while (appData.isTransferDone == false);
}

void APP_PageProgram(uint32_t address, uint8_t* pPageData)
{
    uint32_t i;

    APP_WriteEnable();

    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_PAGE_PROGRAM;
    appData.transmitBuffer[1] = (address >> 16);
    appData.transmitBuffer[2] = (address >> 8);
    appData.transmitBuffer[3] = address;

    for (i = 0; i < APP_PAGE_PROGRAM_SIZE_BYTES; i++)
    {
        appData.transmitBuffer[4 + i] = pPageData[i];
    }

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, (4 + APP_PAGE_PROGRAM_SIZE_BYTES));

    while (appData.isTransferDone == false);
}

void APP_MemoryRead(uint32_t address, uint8_t* pReadBuffer, uint32_t nBytes, bool isHighSpeedRead)
{
    uint8_t nTxBytes;

    appData.isTransferDone = false;

    appData.transmitBuffer[1] = (address >> 16);
    appData.transmitBuffer[2] = (address >> 8);
    appData.transmitBuffer[3] = address;

    if (isHighSpeedRead == true)
    {
        appData.transmitBuffer[0] = APP_CMD_MEMORY_HIGH_SPEED_READ;
        /* For high speed read, perform a dummy write */
        appData.transmitBuffer[4] = 0xFF;
        nTxBytes = 5;
    }
    else
    {
        appData.transmitBuffer[0] = APP_CMD_MEMORY_READ;
        nTxBytes = 4;
    }

    APP_CS_ENABLE();
    appData.isCSDeAssert = false;
    QSPI_Write(appData.transmitBuffer, nTxBytes);

    while (appData.isTransferDone == false);

    appData.isTransferDone = false;
    appData.isCSDeAssert = true;
    QSPI_Read(pReadBuffer, nBytes);

    while (appData.isTransferDone == false);
}

uint8_t APP_StatusRead(void)
{
    uint8_t status;
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_STATUS_REG_READ;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_WriteRead(appData.transmitBuffer, 1, appData.transmitBuffer, (1+1));

    while (appData.isTransferDone == false);

    status = appData.transmitBuffer[1];

    return status;
}

uint8_t APP_ConfigRegisterRead(void)
{
    uint8_t config_reg;
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_CONFIG_REG_READ;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_WriteRead(appData.transmitBuffer, 1, appData.transmitBuffer, (1+1));

    while (appData.isTransferDone == false);

    config_reg = appData.transmitBuffer[1];

    return config_reg;
}

void APP_JEDEC_ID_Read(uint8_t* manufacturerID, uint16_t* deviceID)
{
    appData.isTransferDone = false;

    appData.transmitBuffer[0] = APP_CMD_JEDEC_ID_READ;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_WriteRead(appData.transmitBuffer, 1, appData.transmitBuffer, (1+3));

    while (appData.isTransferDone == false);

    *manufacturerID = appData.transmitBuffer[1];
    *deviceID = (appData.transmitBuffer[2] << 8UL) | appData.transmitBuffer[3];
}

void APP_GlobalWriteProtectionUnlock(void)
{
    APP_WriteEnable();

    appData.isTransferDone = false;
    appData.transmitBuffer[0] = APP_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK;

    APP_CS_ENABLE();
    appData.isCSDeAssert = true;
    QSPI_Write(appData.transmitBuffer, 1);

    while (appData.isTransferDone == false);
}

void APP_MinPowerOnDelay(void)
{
    uint32_t i;

    /* Cheap delay.
     * Based on the CPU frequency, ensure the delay is at-least 100 microseconds.
     */
#if APP_USE_N25Q_FLASH
    for (i = 0; i < 100000; i++)
#elif APP_USE_SST26_FLASH
    for (i = 0; i < 300000; i++)
#endif
    {
        asm("NOP");
    }
}

/* This function will be called by SPI PLIB when transfer is completed */
void APP_SPIEventHandler(uintptr_t context )
{
    uint8_t* isCSDeAssert = (uint8_t*)context;

    if (*isCSDeAssert == true)
    {
        /* De-assert the chip select */
        APP_CS_DISABLE();
    }

    appData.isTransferDone = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     Application initialization routine.

  Description:
    This function initializes the application.  It places the
    application in its initial state and prepares it to run so that its
    APP_Tasks function can be called.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    This routine must be called from the main function.
*/

void APP_Initialize (void)
{
    uint32_t i;

    LED_Off();

    appData.state = APP_STATE_INITIALIZE;

    /* Fill up the test data */
    for (i = 0; i < APP_PAGE_PROGRAM_SIZE_BYTES; i++)
    {
        writeDataBuffer[i] = i;
    }
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    Application tasks function

  Description:
    This routine is the Application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks ( void )
{
    uint8_t status_reg = 0;

    /* Check the application's current state. */
    switch (appData.state)
    {
        case APP_STATE_INITIALIZE:
            APP_CS_DISABLE();

            /* Register a callback with the SPI PLIB and pass a pointer to the isCSDeAssert variable as the context */
            QSPI_CallbackRegister(APP_SPIEventHandler, (uintptr_t) &appData.isCSDeAssert);
            appData.state = APP_STATE_WAIT_MIN_POWER_UP_TIME;
            break;

        case APP_STATE_WAIT_MIN_POWER_UP_TIME:
            APP_MinPowerOnDelay();
            appData.state = APP_STATE_RESET;
            break;

        case APP_STATE_RESET:
            APP_Reset();
            appData.state = APP_STATE_GLOBAL_BLK_PROTECTION_UNLOCK;
            break;

        case APP_STATE_GLOBAL_BLK_PROTECTION_UNLOCK:
            APP_GlobalWriteProtectionUnlock();
            appData.state = APP_STATE_JEDEC_ID_READ;
            break;

        case APP_STATE_JEDEC_ID_READ:
            APP_JEDEC_ID_Read(&appData.manufacturerID, &appData.deviceID);
            appData.state = APP_STATE_SECTOR_ERASE;
            break;

        case APP_STATE_SECTOR_ERASE:
            APP_SectorErase(APP_MEM_ADDR);
            appData.state = APP_STATE_READ_STATUS;
            appData.nextState = APP_STATE_PAGE_PROGRAM;
            break;

        case APP_STATE_READ_STATUS:
            status_reg = APP_StatusRead();
            if ((status_reg & APP_STATUS_BIT_BUSY) == 0)
            {
                appData.state = appData.nextState;
            }
            break;

        case APP_STATE_PAGE_PROGRAM:
            APP_PageProgram(APP_MEM_ADDR, &writeDataBuffer[0]);
            appData.state = APP_STATE_READ_STATUS;
            appData.nextState = APP_STATE_MEMORY_READ;
            break;

        case APP_STATE_MEMORY_READ:
            APP_MemoryRead(APP_MEM_ADDR, readDataBuffer, APP_PAGE_PROGRAM_SIZE_BYTES, false);
            appData.state = APP_STATE_VERIFY;
            break;

        case APP_STATE_VERIFY:
            if (memcmp(writeDataBuffer, readDataBuffer, APP_PAGE_PROGRAM_SIZE_BYTES) == 0)
            {
                appData.state = APP_STATE_XFER_SUCCESSFUL;
            }
            else
            {
                appData.state = APP_STATE_XFER_ERROR;
            }
            break;

        case APP_STATE_XFER_SUCCESSFUL:
            LED_On();
            appData.state = APP_STATE_IDLE;
            break;

        case APP_STATE_XFER_ERROR:
            LED_Off();
            appData.state = APP_STATE_IDLE;
            break;

        case APP_STATE_IDLE:
            break;

        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    APP_Initialize ();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
         APP_Tasks();
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

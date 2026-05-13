/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
 * QSPI Flash Device Selection
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
#include <string.h>                     // Defines memset, memcmp
#include "definitions.h"                // SYS function prototypes

// *****************************************************************************
// *****************************************************************************
// Section: Flash Device Definitions
// *****************************************************************************
// *****************************************************************************

#define PAGE_SIZE                  (256U)
#define SECTOR_SIZE                (4096U)

/* Erase, Write and Read 80KBytes of memory */
#define SECTORS_TO_EWR             (20U)

#define BUFFER_SIZE                (SECTOR_SIZE * SECTORS_TO_EWR)

#define MEM_START_ADDRESS          (0x0U)

#if APP_USE_N25Q_FLASH
#define N25Q256_JEDEC_ID           (0x19BA20UL)
#elif APP_USE_SST26_FLASH
#define SST26VF064B_JEDEC_ID       (0x004326BFUL)
#endif

#define LED_ON                     LED_Clear
#define LED_OFF                    LED_Set
#define LED_TOGGLE                 LED_Toggle

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
    /* The app mounts the disk */
    APP_STATE_INIT = 0,

    /* Reset Flash*/
    APP_STATE_RESET_FLASH,

#if APP_USE_N25Q_FLASH
    /* Enable Quad IO Mode*/
    APP_STATE_ENTER_QUAD_IO,
#elif APP_USE_SST26_FLASH
    /* Enable Quad IO Mode*/
    APP_STATE_ENABLE_QUAD_IO,

    /* Unlock Flash*/
    APP_STATE_UNLOCK_FLASH,
#endif

    /* Read JEDEC ID*/
    APP_STATE_READ_JEDEC_ID,

    /* Erase Flash */
    APP_STATE_ERASE_FLASH,

    /* Erase Wait */
    APP_STATE_ERASE_WAIT,

    /* Write to Memory */
    APP_STATE_WRITE_MEMORY,

    /* Erase Wait */
    APP_STATE_WRITE_WAIT,

    /* Read From Memory */
    APP_STATE_READ_MEMORY,

    /* Verify Data Read */
    APP_STATE_VERIFY_DATA,

    /* The app idles */
    APP_STATE_SUCCESS,

    /* An app error has occurred */
    APP_STATE_ERROR

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
    /* Application's current state */
    APP_STATES state;

    /* Application transfer status */
    volatile bool xfer_done;

    /* Jedec-ID*/
    uint32_t jedec_id;

    /* Read Buffer */
    uint8_t readBuffer[BUFFER_SIZE];

    /* Write Buffer*/
    uint8_t writeBuffer[BUFFER_SIZE];
} APP_DATA;

#if APP_USE_N25Q_FLASH
/* N25Q Command set

  Summary:
    Enumeration listing the N25QVF commands.

  Description:
    This enumeration defines the commands used to interact with the N25QVF
    series of devices.

  Remarks:
    None
*/

typedef enum
{
    /* Reset enable command. */
    N25Q_CMD_FLASH_RESET_ENABLE = 0x66,

    /* Command to reset the flash. */
    N25Q_CMD_FLASH_RESET        = 0x99,

    /* Command to read JEDEC-ID of the flash device. */
    N25Q_CMD_JEDEC_ID_READ      = 0x9F,

    /*QUAD Command to read JEDEC-ID of the flash device. */
    N25Q_CMD_MULTIPLE_IO_READ_ID = 0xAF,

    /* Command to perfrom High Speed Read */
    N25Q_CMD_FAST_READ          = 0x0B,

    /* Write enable command. */
    N25Q_CMD_WRITE_ENABLE       = 0x06,

    /* Page Program command. */
    N25Q_CMD_PAGE_PROGRAM       = 0x02,

    /* Command to read the Flash status register. */
    N25Q_CMD_READ_STATUS_REG    = 0x05,

    /* Command to perform sector erase */
    N25Q_CMD_SUBSECTOR_ERASE      = 0x20,

    /* Command to perform Bulk erase */
    N25Q_CMD_SECTOR_ERASE_64K     = 0xD8,

    /* Command to perform Chip erase */
    N25Q_CMD_BULK_ERASE         = 0xC7,

    /* Command to enter quad mode */
    N25Q_CMD_ENTER_QUAD   = 0x35,

    /* Command to exit quad mode */
    N25Q_CMD_EXIT_QUAD   = 0xF5,

    /*Command to Write enhanced volatile config register */
    N25Q_CMD_WRITE_ENHANCED_VOLATILE_CONFIG_REGISTER   = 0x61
} N25Q_CMD;

#elif APP_USE_SST26_FLASH
/* SST26 Command set

  Summary:
    Enumeration listing the SST26VF commands.

  Description:
    This enumeration defines the commands used to interact with the SST26VF
    series of devices.

  Remarks:
    None
*/

typedef enum
{
    /* Reset enable command. */
    SST26_CMD_FLASH_RESET_ENABLE = 0x66,

    /* Command to reset the flash. */
    SST26_CMD_FLASH_RESET        = 0x99,

    /* Command to Enable QUAD IO */
    SST26_CMD_ENABLE_QUAD_IO     = 0x38,

    /* Command to Reset QUAD IO */
    SST26_CMD_RESET_QUAD_IO      = 0xFF,

    /* Command to read JEDEC-ID of the flash device. */
    SST26_CMD_JEDEC_ID_READ      = 0x9F,

    /* QUAD Command to read JEDEC-ID of the flash device. */
    SST26_CMD_QUAD_JEDEC_ID_READ = 0xAF,

    /* Command to perfrom High Speed Read */
    SST26_CMD_HIGH_SPEED_READ    = 0x0B,

    /* Write enable command. */
    SST26_CMD_WRITE_ENABLE       = 0x06,

    /* Page Program command. */
    SST26_CMD_PAGE_PROGRAM       = 0x02,

    /* Command to read the Flash status register. */
    SST26_CMD_READ_STATUS_REG    = 0x05,

    /* Command to perform sector erase */
    SST26_CMD_SECTOR_ERASE       = 0x20,

    /* Command to perform Bulk erase */
    SST26_CMD_BULK_ERASE_64K     = 0xD8,

    /* Command to perform Chip erase */
    SST26_CMD_CHIP_ERASE         = 0xC7,

    /* Command to unlock the flash device. */
    SST26_CMD_UNPROTECT_GLOBAL   = 0x98

} SST26_CMD;
#endif

typedef enum
{
    /* Buffer is being processed */
    APP_TRANSFER_BUSY,
    /* APP Buffer transfer is successfully completed*/
    APP_TRANSFER_COMPLETED,
    /* APP Buffer transfer had error*/
    APP_TRANSFER_ERROR_UNKNOWN,
    /* APP Transfer has not been asked yet */
    APP_TRANSFER_IDLE
} APP_TRANSFER_STATUS;

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

APP_DATA appData;

static uint32_t write_index = 0;
static uint32_t sector_index = 0;

static qspi_command_xfer_t qspi_command_xfer = { 0 };
static qspi_register_xfer_t qspi_register_xfer = { 0 };
static qspi_memory_xfer_t qspi_memory_xfer = { 0 };

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* This function resets the flash by sending down the reset enable command
 * followed by the reset command. */

static APP_TRANSFER_STATUS APP_ResetFlash(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_command_xfer.instruction = N25Q_CMD_FLASH_RESET_ENABLE;
#elif APP_USE_SST26_FLASH
    qspi_command_xfer.instruction = SST26_CMD_FLASH_RESET_ENABLE;
#endif
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

#if APP_USE_N25Q_FLASH
    qspi_command_xfer.instruction = N25Q_CMD_FLASH_RESET;
#elif APP_USE_SST26_FLASH
    qspi_command_xfer.instruction = SST26_CMD_FLASH_RESET;
#endif
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;
}

#if APP_USE_N25Q_FLASH
static APP_TRANSFER_STATUS APP_EnableQuadIO(void)
{
    uint32_t config_reg = 0x1F;
	memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = N25Q_CMD_WRITE_ENABLE;
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    memset((void *)&qspi_register_xfer, 0, sizeof(qspi_register_xfer_t));

    qspi_register_xfer.instruction = N25Q_CMD_WRITE_ENHANCED_VOLATILE_CONFIG_REGISTER;
    qspi_register_xfer.width = SINGLE_BIT_SPI;
    qspi_register_xfer.dummy_cycles = 0;

    if (QSPI_RegisterWrite(&qspi_register_xfer,&config_reg, 1) == false)
    {
          return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;

}
#elif APP_USE_SST26_FLASH
/* Enables the QUAD IO on the flash */
static APP_TRANSFER_STATUS APP_EnableQuadIO(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_ENABLE_QUAD_IO;
    qspi_command_xfer.width = SINGLE_BIT_SPI;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}
#endif

/* Sends Write Enable command to flash */
static APP_TRANSFER_STATUS APP_WriteEnable(void)
{
    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_command_xfer.instruction = N25Q_CMD_WRITE_ENABLE;
#elif APP_USE_SST26_FLASH
    qspi_command_xfer.instruction = SST26_CMD_WRITE_ENABLE;
#endif
    qspi_command_xfer.width = QUAD_CMD;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

#if APP_USE_SST26_FLASH
/* This function sends down command to perform a global unprotect of the flash. */
static APP_TRANSFER_STATUS APP_UnlockFlash(void)
{
    if (APP_TRANSFER_COMPLETED != APP_WriteEnable())
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    memset((void *)&qspi_command_xfer, 0, sizeof(qspi_command_xfer_t));

    qspi_command_xfer.instruction = SST26_CMD_UNPROTECT_GLOBAL;
    qspi_command_xfer.width = QUAD_CMD;

    if (QSPI_CommandWrite(&qspi_command_xfer, 0) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}
#endif

/* This function reads and stores the flash id. */
static APP_TRANSFER_STATUS APP_ReadJedecId(uint32_t *jedec_id)
{
    memset((void *)&qspi_register_xfer, 0, sizeof(qspi_register_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_register_xfer.instruction = N25Q_CMD_MULTIPLE_IO_READ_ID;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 0;
#elif APP_USE_SST26_FLASH
    qspi_register_xfer.instruction = SST26_CMD_QUAD_JEDEC_ID_READ;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 2;
#endif

    if (QSPI_RegisterRead(&qspi_register_xfer, jedec_id, 3) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Function to read the status register of the flash. */
static APP_TRANSFER_STATUS APP_ReadStatus( uint32_t *rx_data, uint32_t rx_data_length )
{
    memset((void *)&qspi_register_xfer, 0, sizeof(qspi_register_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_register_xfer.instruction = N25Q_CMD_READ_STATUS_REG;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 0;
#elif APP_USE_SST26_FLASH
    qspi_register_xfer.instruction = SST26_CMD_READ_STATUS_REG;
    qspi_register_xfer.width = QUAD_CMD;
    qspi_register_xfer.dummy_cycles = 2;
#endif

    if (QSPI_RegisterRead(&qspi_register_xfer, rx_data, rx_data_length) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;
}

/* Checks for any pending transfers Erase/Write */
static APP_TRANSFER_STATUS APP_TransferStatusCheck(void)
{
    uint8_t reg_status = 0;

    if (APP_ReadStatus((uint32_t *)&reg_status, 1) != APP_TRANSFER_COMPLETED)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    if((reg_status & (1<<0)))
        return APP_TRANSFER_BUSY;
    else
        return APP_TRANSFER_COMPLETED;
}

/* Reads n Bytes of data from the flash memory */
static APP_TRANSFER_STATUS APP_MemoryRead( uint32_t *rx_data, uint32_t rx_data_length, uint32_t address )
{
    memset((void *)&qspi_memory_xfer, 0, sizeof(qspi_memory_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_memory_xfer.instruction = N25Q_CMD_FAST_READ;
    qspi_memory_xfer.width = QUAD_CMD;
    qspi_memory_xfer.dummy_cycles = 10;
#elif APP_USE_SST26_FLASH
    qspi_memory_xfer.instruction = SST26_CMD_HIGH_SPEED_READ;
    qspi_memory_xfer.width = QUAD_CMD;
    qspi_memory_xfer.dummy_cycles = 6;
#endif

    if (QSPI_MemoryRead(&qspi_memory_xfer, rx_data, rx_data_length, address) == false) {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }
    return APP_TRANSFER_COMPLETED;
}

/* Writes n Bytes of data to the flash memory */
static APP_TRANSFER_STATUS APP_MemoryWrite( uint32_t *tx_data, uint32_t tx_data_length, uint32_t address )
{
    if (APP_TRANSFER_COMPLETED != APP_WriteEnable())
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    memset((void *)&qspi_memory_xfer, 0, sizeof(qspi_memory_xfer_t));

#if APP_USE_N25Q_FLASH
    qspi_memory_xfer.instruction = N25Q_CMD_PAGE_PROGRAM;
#elif APP_USE_SST26_FLASH
    qspi_memory_xfer.instruction = SST26_CMD_PAGE_PROGRAM;
#endif
    qspi_memory_xfer.width = QUAD_CMD;

    if (QSPI_MemoryWrite(&qspi_memory_xfer, tx_data, tx_data_length, address) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

/* Sends Erase command to flash */
static APP_TRANSFER_STATUS APP_Erase(uint8_t instruction, uint32_t address)
{
    if (APP_WriteEnable() != APP_TRANSFER_COMPLETED)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    qspi_command_xfer.instruction = instruction;
    qspi_command_xfer.width = QUAD_CMD;
    qspi_command_xfer.addr_en = 1;

    if (QSPI_CommandWrite(&qspi_command_xfer, address) == false)
    {
        return APP_TRANSFER_ERROR_UNKNOWN;
    }

    return APP_TRANSFER_COMPLETED;
}

static APP_TRANSFER_STATUS APP_SectorErase(uint32_t address)
{
#if APP_USE_N25Q_FLASH
    return (APP_Erase(N25Q_CMD_SUBSECTOR_ERASE, address));
#elif APP_USE_SST26_FLASH
    return (APP_Erase(SST26_CMD_SECTOR_ERASE, address));
#endif
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

void APP_Initialize ( void )
{
    uint32_t i = 0;

    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    for (i = 0; i < BUFFER_SIZE; i++)
        appData.writeBuffer[i] = i;

    SYSTICK_TimerStart();
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
    /* Check the application's current state. */
    switch ( appData.state )
    {
        case APP_STATE_INIT:
        {
            appData.state = APP_STATE_RESET_FLASH;
        }

        case APP_STATE_RESET_FLASH:
        {
            if (APP_ResetFlash() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
#if APP_USE_N25Q_FLASH
                appData.state = APP_STATE_ENTER_QUAD_IO;
#elif APP_USE_SST26_FLASH
                appData.state = APP_STATE_ENABLE_QUAD_IO;
#endif
            }
            break;
        }

#if APP_USE_N25Q_FLASH
        case APP_STATE_ENTER_QUAD_IO:
        {
            if (APP_EnableQuadIO() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {

                appData.state = APP_STATE_READ_JEDEC_ID;
            }
            break;
        }
#elif APP_USE_SST26_FLASH
        case APP_STATE_ENABLE_QUAD_IO:
        {
            if (APP_EnableQuadIO() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
                appData.state = APP_STATE_UNLOCK_FLASH;
            }
            break;
        }

        case APP_STATE_UNLOCK_FLASH:
        {
            if (APP_UnlockFlash() != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
            }
            else
            {
                appData.state = APP_STATE_READ_JEDEC_ID;
            }
            break;
        }
#endif

        case APP_STATE_READ_JEDEC_ID:
        {
            if (APP_ReadJedecId(&appData.jedec_id) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

#if APP_USE_N25Q_FLASH
            if (appData.jedec_id != N25Q256_JEDEC_ID)
#elif APP_USE_SST26_FLASH
            if (appData.jedec_id != SST26VF064B_JEDEC_ID)
#endif
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_ERASE_FLASH;

            break;
        }

        case APP_STATE_ERASE_FLASH:
        {
            if (APP_SectorErase((MEM_START_ADDRESS + sector_index)) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_ERASE_WAIT;

            break;
        }

        case APP_STATE_ERASE_WAIT:
        {
            if (APP_TransferStatusCheck() == APP_TRANSFER_COMPLETED)
            {
                sector_index += SECTOR_SIZE;

                if (sector_index < BUFFER_SIZE)
                {
                    appData.state = APP_STATE_ERASE_FLASH;
                }
                else
                {
                    appData.state = APP_STATE_WRITE_MEMORY;
                }
            }
            break;
        }

        case APP_STATE_WRITE_MEMORY:
        {
            if (APP_MemoryWrite((uint32_t *)&appData.writeBuffer[write_index], PAGE_SIZE, (MEM_START_ADDRESS + write_index)) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_WRITE_WAIT;

            break;
        }

        case APP_STATE_WRITE_WAIT:
        {
            if (APP_TransferStatusCheck() == APP_TRANSFER_COMPLETED)
            {
                write_index += PAGE_SIZE;
                if (write_index < BUFFER_SIZE)
                {
                    appData.state = APP_STATE_WRITE_MEMORY;
                }
                else
                {
                    appData.state = APP_STATE_READ_MEMORY;
                }
            }
            break;
        }

        case APP_STATE_READ_MEMORY:
        {
            if (APP_MemoryRead((uint32_t *)&appData.readBuffer[0], BUFFER_SIZE, MEM_START_ADDRESS) != APP_TRANSFER_COMPLETED)
            {
                appData.state = APP_STATE_ERROR;
                break;
            }

            appData.state = APP_STATE_VERIFY_DATA;

            break;
        }

        case APP_STATE_VERIFY_DATA:
        {
            if (!memcmp(appData.writeBuffer, appData.readBuffer, BUFFER_SIZE))
            {
                appData.state = APP_STATE_SUCCESS;
            }
            else
            {
                appData.state = APP_STATE_ERROR;
            }

            break;
        }

        case APP_STATE_SUCCESS:
        {
            SYSTICK_DelayMs(1000);

            LED_TOGGLE();

            break;
        }

        case APP_STATE_ERROR:
        default:
        {
            LED_ON();
            break;
        }
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

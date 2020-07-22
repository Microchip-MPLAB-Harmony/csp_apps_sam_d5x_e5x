/*******************************************************************************
  Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sst26.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef APP_SST26_H
#define APP_SST26_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdlib.h>
#include <string.h>
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
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
#define APP_STATUS_BIT_BUSY                       (0x01 << 7)

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
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

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

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the main function.
*/

void APP_Initialize ( void );


/*******************************************************************************
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

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks( void );


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* APP_SST26_H */

/*******************************************************************************
 End of File
 */


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

#ifndef TOOLCHAIN_SPECIFICS_H
#define TOOLCHAIN_SPECIFICS_H

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif

#include "cmsis_compiler.h"

#ifndef _SSIZE_T_DECLARED
#ifdef __SIZE_TYPE__
/* If __SIZE_TYPE__ is defined (armclang) we define ssize_t based on size_t.
We simply change "unsigned" to "signed" for this single definition
to make sure ssize_t and size_t only differ by their signedness. */
#define unsigned signed
typedef __SIZE_TYPE__ _ssize_t;
#undef unsigned
#else
#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int _ssize_t;
#else
typedef long _ssize_t;
#endif
#endif
typedef _ssize_t ssize_t;
#define    _SSIZE_T_DECLARED
#endif

#define NO_INIT        __attribute__((section(".no_init")))
#define SECTION(a)     __attribute__((__section__(a)))

#define CACHE_LINE_SIZE    (16u)
#define CACHE_ALIGN        __ALIGNED(CACHE_LINE_SIZE)

#define CACHE_ALIGNED_SIZE_GET(size)     (size + ((size % CACHE_LINE_SIZE)? (CACHE_LINE_SIZE - (size % CACHE_LINE_SIZE)) : 0))

#ifndef FORMAT_ATTRIBUTE
   #define FORMAT_ATTRIBUTE(archetype, string_index, first_to_check)  __attribute__ ((format (archetype, string_index, first_to_check)))
#endif

#ifdef __cplusplus
}
#endif

#endif // end of header


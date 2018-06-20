/**************************************************************************//**
 *
 * @file
 * @brief x86_64_accton_as7312_54xs Porting Macros.
 *
 * @addtogroup x86_64_accton_as7312_54xs-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_ACCTON_AS7312_54XS_PORTING_H__
#define __X86_64_ACCTON_AS7312_54XS_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_accton_as7312_54xs_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_accton_as7312_54xs_MALLOC GLOBAL_MALLOC
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_MALLOC malloc
    #else
        #error The macro x86_64_accton_as7312_54xs_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_accton_as7312_54xs_FREE GLOBAL_FREE
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_FREE free
    #else
        #error The macro x86_64_accton_as7312_54xs_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_accton_as7312_54xs_MEMSET GLOBAL_MEMSET
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_MEMSET memset
    #else
        #error The macro x86_64_accton_as7312_54xs_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_accton_as7312_54xs_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_MEMCPY memcpy
    #else
        #error The macro x86_64_accton_as7312_54xs_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_accton_as7312_54xs_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_STRNCPY strncpy
    #else
        #error The macro x86_64_accton_as7312_54xs_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_accton_as7312_54xs_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_accton_as7312_54xs_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_accton_as7312_54xs_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_SNPRINTF snprintf
    #else
        #error The macro x86_64_accton_as7312_54xs_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_accton_as7312_54xs_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_accton_as7312_54xs_STRLEN GLOBAL_STRLEN
    #elif X86_64_ACCTON_AS7312_54XS_CONFIG_PORTING_STDLIB == 1
        #define x86_64_accton_as7312_54xs_STRLEN strlen
    #else
        #error The macro x86_64_accton_as7312_54xs_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_ACCTON_AS7312_54XS_PORTING_H__ */
/* @} */

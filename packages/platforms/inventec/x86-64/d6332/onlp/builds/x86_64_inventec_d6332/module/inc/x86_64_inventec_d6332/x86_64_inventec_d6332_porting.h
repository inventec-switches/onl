/**************************************************************************//**
 *
 * @file
 * @brief x86_64_inventec_d6332 Porting Macros.
 *
 * @addtogroup x86_64_inventec_d6332-porting
 * @{
 *
 *****************************************************************************/
#ifndef __x86_64_inventec_d6332_PORTING_H__
#define __x86_64_inventec_d6332_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if x86_64_inventec_d6332_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef x86_64_inventec_d6332_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define x86_64_inventec_d6332_MALLOC GLOBAL_MALLOC
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_MALLOC malloc
    #else
        #error The macro x86_64_inventec_d6332_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_FREE
    #if defined(GLOBAL_FREE)
        #define x86_64_inventec_d6332_FREE GLOBAL_FREE
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_FREE free
    #else
        #error The macro x86_64_inventec_d6332_FREE is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define x86_64_inventec_d6332_MEMSET GLOBAL_MEMSET
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_MEMSET memset
    #else
        #error The macro x86_64_inventec_d6332_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define x86_64_inventec_d6332_MEMCPY GLOBAL_MEMCPY
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_MEMCPY memcpy
    #else
        #error The macro x86_64_inventec_d6332_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define x86_64_inventec_d6332_STRNCPY GLOBAL_STRNCPY
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_STRNCPY strncpy
    #else
        #error The macro x86_64_inventec_d6332_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define x86_64_inventec_d6332_VSNPRINTF GLOBAL_VSNPRINTF
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_VSNPRINTF vsnprintf
    #else
        #error The macro x86_64_inventec_d6332_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define x86_64_inventec_d6332_SNPRINTF GLOBAL_SNPRINTF
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_SNPRINTF snprintf
    #else
        #error The macro x86_64_inventec_d6332_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef x86_64_inventec_d6332_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define x86_64_inventec_d6332_STRLEN GLOBAL_STRLEN
    #elif x86_64_inventec_d6332_CONFIG_PORTING_STDLIB == 1
        #define x86_64_inventec_d6332_STRLEN strlen
    #else
        #error The macro x86_64_inventec_d6332_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __x86_64_inventec_d6332_PORTING_H__ */
/* @} */

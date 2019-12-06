/**************************************************************************//**
 *
 * @file
 * @brief x86_64_alphanetworks_snx60a0_486f Porting Macros.
 *
 * @addtogroup x86_64_alphanetworks_snx60a0_486f-porting
 * @{
 *
 *****************************************************************************/
#ifndef __X86_64_ALPHANETWORKS_SNX60A0_486F_PORTING_H__
#define __X86_64_ALPHANETWORKS_SNX60A0_486F_PORTING_H__


/* <auto.start.portingmacro(ALL).define> */
#if X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory.h>
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_MALLOC
    #if defined(GLOBAL_MALLOC)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MALLOC GLOBAL_MALLOC
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MALLOC malloc
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_MALLOC is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_FREE
    #if defined(GLOBAL_FREE)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_FREE GLOBAL_FREE
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_FREE free
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_FREE is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_MEMSET
    #if defined(GLOBAL_MEMSET)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MEMSET GLOBAL_MEMSET
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MEMSET memset
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_MEMSET is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_MEMCPY
    #if defined(GLOBAL_MEMCPY)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MEMCPY GLOBAL_MEMCPY
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_MEMCPY memcpy
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_MEMCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_STRNCPY
    #if defined(GLOBAL_STRNCPY)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_STRNCPY GLOBAL_STRNCPY
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_STRNCPY strncpy
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_STRNCPY is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_VSNPRINTF
    #if defined(GLOBAL_VSNPRINTF)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_VSNPRINTF GLOBAL_VSNPRINTF
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_VSNPRINTF vsnprintf
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_VSNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_SNPRINTF
    #if defined(GLOBAL_SNPRINTF)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_SNPRINTF GLOBAL_SNPRINTF
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_SNPRINTF snprintf
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_SNPRINTF is required but cannot be defined.
    #endif
#endif

#ifndef X86_64_ALPHANETWORKS_SNX60A0_486F_STRLEN
    #if defined(GLOBAL_STRLEN)
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_STRLEN GLOBAL_STRLEN
    #elif X86_64_ALPHANETWORKS_SNX60A0_486F_CONFIG_PORTING_STDLIB == 1
        #define X86_64_ALPHANETWORKS_SNX60A0_486F_STRLEN strlen
    #else
        #error The macro X86_64_ALPHANETWORKS_SNX60A0_486F_STRLEN is required but cannot be defined.
    #endif
#endif

/* <auto.end.portingmacro(ALL).define> */


#endif /* __X86_64_ALPHANETWORKS_SNX60A0_486F_PORTING_H__ */
/* @} */

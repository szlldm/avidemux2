/** *************************************************************************
             
    \fn ADM_default.h
    \brief include that file to get most of the basic types & functions
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// The following directives must be kept safe from the header guard
// to make sure we can always override definitions from Qt headers.
#undef QT_TR_NOOP
#undef QT_TRANSLATE_NOOP
#define QT_TR_NOOP(x) ADM_translate("adm",x)
#define QT_TRANSLATE_NOOP(a,x) ADM_translate(a,x)

#ifndef ADM_DEFAULT_H
#define ADM_DEFAULT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32 // needed for unlink
#include <unistd.h>
#endif

#include "ADM_core6_export.h"
#include "ADM_coreConfig.h"
#include "ADM_inttype.h"
#include "ADM_assert.h"
#define ADM_NO_PTS 0xffffffffffffffffLL
#ifdef __cplusplus
#include "ADM_cpuCap.h"
#include "ADM_clock.h"
#include "ADM_misc.h"
#include "ADM_files.h"
#endif

#ifdef __cplusplus
extern "C" 
{
#endif
ADM_CORE6_EXPORT void ADM_warning2(const char *f,const char *st, ...) ;
ADM_CORE6_EXPORT void ADM_info2(const char *f,const char *st, ...) ;
ADM_CORE6_EXPORT void ADM_error2(const char *f,const char *st, ...) ;

#ifdef __GNUC__
 #define ADM_warning(a,...)  ADM_warning2(__PRETTY_FUNCTION__,a, ##__VA_ARGS__)
 #define ADM_info(a,...)     ADM_info2(__PRETTY_FUNCTION__,a,    ##__VA_ARGS__)
 #define ADM_error(a,...)    ADM_error2(__PRETTY_FUNCTION__,a,   ##__VA_ARGS__)
#else
 #define ADM_warning(a,...)  ADM_warning2(__FUNCTION__,a, ##__VA_ARGS__)
 #define ADM_info(a,...)     ADM_info2(__FUNCTION__,a,    ##__VA_ARGS__)
 #define ADM_error(a,...)    ADM_error2(__FUNCTION__,a,   ##__VA_ARGS__)
#endif

ADM_CORE6_EXPORT const char *ADM_translate(const char *domain, const char *stringToTranslate);

#ifdef __cplusplus
}
#endif

#include "ADM_mangle.h"

//
#ifdef _MSC_VER
        #define admAlloca _alloca
        #define ADM_PLUGIN_EXPORT __declspec(dllexport)

#else
        #define admAlloca alloca
        #define ADM_PLUGIN_EXPORT 
#endif

#ifdef __cplusplus

class notStackAllocator
{
public:    
    notStackAllocator(int nb)
    {
        data=new uint8_t[nb];
    }
    ~notStackAllocator()
    {
        delete [] data;
        data=NULL;
    }
    uint8_t *data;
};
#endif
#endif /* ADM_DEFAULT_H */

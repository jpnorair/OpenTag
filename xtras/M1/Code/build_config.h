
#ifndef __BUILD_CONFIG_H
#define __BUILD_CONFIG_H

#include "OT_support.h"

#define __DEBUG__
#ifdef _STM32L152VBT6_ // STM32H152:
    #define PLATFORM_TYPE       STM32H152
#else // DK7A433:
    #define PLATFORM_TYPE       SX1212USB
#endif
//#define APP_TYPE            AG_Blinker    /*****AGAIDI*****/

#endif 

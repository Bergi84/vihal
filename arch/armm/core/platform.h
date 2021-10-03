/* -----------------------------------------------------------------------------
 * This file is a part of the VIHAL project: https://github.com/nvitya/vihal
 * Copyright (c) 2021 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     platform.h (armm)
 *  brief:    Main ARM Cortex-M platform include
 *  version:  1.00
 *  date:     2021-09-28
 *  authors:  nvitya
*/

#ifndef __PLATFORM_H
#define __PLATFORM_H

// board.h must be provided by the application. It should minimum define the cpu.
#include "board.h"

#ifndef MCU_EXTERNAL_INPUT_FREQ
  // Use the internal oscillator then
  #define MCU_EXTERNAL_INPUT_FREQ  0
#endif

#define DONT_USE_CMSIS_INIT

// some CMSIS definition define it
#ifdef LITTLE_ENDIAN
	#undef LITTLE_ENDIAN
#endif

#if defined(MCU_DEFINED)

  // in the board.h the MCU also can be defined

#else

  #include "mcu_builtin.h"

#endif

#if defined(DEBUG)
  #define __DEBUG_BKPT()  asm volatile ("bkpt 0")
#else
  #define __DEBUG_BKPT()
#endif

#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN
#endif

//------------------------------------------------------------------

// the mcu_defs.h comes from the MCUF directory

#include "mcu_defs.h"

// handle some required definitions

#if !defined(MAX_IRQ_HANDLER_COUNT)
	#if defined(NUMBER_OF_INT_VECTORS)
		#define MAX_IRQ_HANDLER_COUNT  (NUMBER_OF_INT_VECTORS - 16)
	#else
		#define MAX_IRQ_HANDLER_COUNT  72
	#endif
#endif

#if !defined(MAX_CLOCK_SPEED)
  #error "MCU Maximal clock speed is not defined!"
#endif

#ifndef MCU_CLOCK_SPEED
  #define MCU_CLOCK_SPEED  MAX_CLOCK_SPEED
#endif

#ifndef HW_DMA_MAX_COUNT
  // usually this is the maximum amount that the DMA can transfer
  #define HW_DMA_MAX_COUNT  32768
#endif

#ifndef IRQVECTAB_OFFS_24_VALUE
  #define IRQVECTAB_OFFS_24_VALUE  0
#endif

#ifndef IRQVECTAB_OFFS_28_VALUE
  #define IRQVECTAB_OFFS_28_VALUE  0
#endif

//------------------------------------------------------------------

#include "mcu_generic.h"

#endif

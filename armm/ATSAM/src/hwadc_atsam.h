/* -----------------------------------------------------------------------------
 * This file is a part of the NVCM project: https://github.com/nvitya/nvcm
 * Copyright (c) 2018 Viktor Nagy, nvitya
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
 *  file:     hwadc_atsam.cpp
 *  brief:    ATSAM Simple Internal ADC
 *  version:  1.00
 *  date:     2018-09-24
 *  authors:  nvitya
*/

#ifndef HWADC_ATSAM_H_
#define HWADC_ATSAM_H_

#define HWADC_PRE_ONLY
#include "hwadc.h"

#if defined(ADC) || defined(ADC1)
  #define HW_ADC_REGS  Adc
#elif defined(AFEC0)
  #define HW_ADC_REGS  Afec
#endif

#define HWADC_MAX_CHANNELS  16
#define HWADC_DATA_LSHIFT    4

class THwAdc_atsam : public THwAdc_pre
{
public:
	uint32_t        channel_map = 0;  // by default convert only ch 0

	THwDmaChannel   dmach;
	THwDmaTransfer  dmaxfer;

	int             dmachannel = 12;

	uint8_t         dmarqid = 0;

	HW_ADC_REGS *   regs = nullptr;

	bool            Init(int adevnum, uint32_t achannel_map);
#if defined(ADC)
	inline uint16_t ChValue(uint8_t ach) { return (regs->ADC_CDR[ach] << HWADC_DATA_LSHIFT); } // report left aligned
#else
	uint16_t ChValue(uint8_t ach);
#endif

	void StartFreeRun(uint32_t achsel);
	void StopFreeRun();
	void StartRecord(uint32_t achsel, uint32_t acount, uint16_t * adstptr);
	bool RecordFinished() { return !dmach.Active(); }
};

#define HWADC_IMPL THwAdc_atsam

#endif /* HWADC_ATSAM_H_ */

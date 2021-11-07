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
 *  file:     hwintflash_atsam.h
 *  brief:    Internal Flash Handling for ATSAM
 *  version:  1.00
 *  date:     2019-04-07
 *  authors:  nvitya
*/

#ifndef HWINTFLASH_ATSAM_H_
#define HWINTFLASH_ATSAM_H_

#define HWINTFLASH_PRE_ONLY
#include "hwintflash.h"

class THwIntFlash_atsam : public THwIntFlash_pre
{
public:
	bool           HwInit();

public:
	Efc *          regs = nullptr;
	uint8_t        ctrl_count = 1;
	uint32_t       ctrl_bytesize = 0;
	uint32_t       ctrl2_addr = 0xF0000000;

	bool           StartFlashCmd(uint8_t acmd);

	void           CmdEraseBlock(); // at address
	void           CmdWritePage();
	void           CmdClearPageBuffer();

	inline bool    CmdFinished() { return (regs->EEFC_FSR & 1); }

  uint32_t       EraseSize(uint32_t aaddress);

#if defined(MCUSF_3X)
public:
  void           SelectController(uint32_t aaddress);
#endif

};

#define HWINTFLASH_IMPL THwIntFlash_atsam

#endif // def HWINTFLASH_ATSAM_H_

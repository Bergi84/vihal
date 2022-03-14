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
 *  file:     hwuart_stm32.h
 *  brief:    STM32 UART
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#ifndef HWUART_STM32_H_
#define HWUART_STM32_H_

#define HWUART_PRE_ONLY
#include "hwuart.h"

class THwUart_stm32 : public THwUart_pre
{
public:
	bool Init(int adevnum);  // 0x101 = LPUART1

	bool SetPeriphClock(unsigned aclk);
	bool SetBaudRate(int abaudrate = 0);

	bool TrySendChar(char ach);
	bool TryRecvChar(char * ach);

	bool SendFinished();

	void DmaAssign(bool istx, THwDmaChannel * admach);

	bool DmaStartSend(THwDmaTransfer * axfer);
	bool DmaStartRecv(THwDmaTransfer * axfer);

public:
	USART_TypeDef *      regs = nullptr;
};

#define HWUART_IMPL THwUart_stm32

#endif // def HWUART_STM32_H_

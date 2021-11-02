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
 *  file:     hwi2c_stm32.cpp
 *  brief:    STM32 I2C / TWI
 *  version:  1.00
 *  date:     2018-02-10
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwi2c.h"

#include "stm32_utils.h"

#include "traces.h"

#if I2C_HW_VER == 1

bool THwI2c_stm32::Init(int adevnum)
{
	unsigned tmp;
	uint8_t busid = STM32_BUSID_APB1;

	initialized = false;

	devnum = adevnum;

	regs = nullptr;

	if (false)
	{
	}
#ifdef I2C1
	else if (1 == devnum)
	{
		regs = I2C1;
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
#endif
#ifdef I2C2
	else if (2 == devnum)
	{
		regs = I2C2;
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
#endif
	if (!regs)
	{
		return false;
	}

	// disable for setup
	regs->CR1 &= ~I2C_CR1_PE;

	regs->CR1 |= I2C_CR1_SWRST;
	regs->CR1 &= ~I2C_CR1_SWRST;

	unsigned cr1 = 0;  // use the default settings, keep disabled

	regs->CR1 = cr1;

	unsigned periphclock = stm32_bus_speed(busid);

	// CR2

	tmp = 0
		| (0 << 12)  // LAST: 1 = Last transfer on DMA EOT
		| (0 << 11)  // DMAEN: 1 = enable DMA
		| ((periphclock / 1000000) << 0)  // set clock speed in MHz
	;
	regs->CR2 = tmp;

	if (speed > 100000)
	{
		regs->TRISE = ((periphclock * 3) / (1000000 * 10)) + 1;
	}
	else
	{
		regs->TRISE = (periphclock / 1000000) + 1;
	}

	unsigned halfclockdiv = (periphclock / (speed * 2));

	regs->CCR = 0
		| (0 << 15)  // F/S: 0 = Sm mode
		| (0 << 14)  // DUTY: Fm mode duty cycle ratio
		| ((halfclockdiv) << 0) // CCR(12):
	;

	regs->CR1 = cr1;

	cr1 |= I2C_CR1_PE;  // Enable

	regs->CR1 = cr1;

	initialized = true;

	return true;
}

void THwI2c_stm32::DmaAssign(bool istx, THwDmaChannel * admach)
{
  if (istx)
  {
    txdma = admach;
    if (admach)
    {
      admach->Prepare(istx, (void *)&regs->DR, 0);
    }
  }
  else
  {
    rxdma = admach;
    if (admach)
    {
      admach->Prepare(istx, (void *)&regs->DR, 0);
    }
  }
}

int THwI2c_stm32::StartReadData(uint8_t adaddr, unsigned aextra, void * dstptr, unsigned len)
{
	if (busy)
	{
		return HWERR_BUSY;
	}

	istx = false;
	devaddr = adaddr;
	dataptr = (uint8_t *)dstptr;
	datalen = len;
	remainingbytes = datalen;
	dmaused = false;
	error = 0;

	extracnt = ((aextra >> 24) & 3);
	if (extracnt)
	{
		// reverse byte order
		uint32_t edr = __REV(aextra);
		if (1 == extracnt)
		{
			extradata[0] = (aextra & 0xFF);
		}
		else if (2 == extracnt)
		{
			extradata[0] = ((aextra >> 8) & 0xFF);
			extradata[1] = ((aextra >> 0) & 0xFF);
		}
		else if (3 == extracnt)
		{
			extradata[0] = ((aextra >> 16) & 0xFF);
			extradata[1] = ((aextra >>  8) & 0xFF);
			extradata[2] = ((aextra >>  0) & 0xFF);
		}
	}
	extraremaining = extracnt;
	runstate = 0;
	busy = true;  // start the state machine

	if (rxdma && rxdma->initialized)
	{
		regs->CR2 |= I2C_CR2_DMAEN;
	}
	else
	{
		regs->CR2 &= ~I2C_CR2_DMAEN;
	}

	regs->SR1 = 0x00FF; // clear all sticky errors

	Run();

	return HWERR_OK;
}

int THwI2c_stm32::StartWriteData(uint8_t adaddr, unsigned aextra, void * srcptr, unsigned len)
{
	if (busy)
	{
		return HWERR_BUSY;
	}

	istx = true;
	devaddr = adaddr;
	dataptr = (uint8_t *)srcptr;
	datalen = len;
	remainingbytes = datalen;
	dmaused = (txdma && txdma->initialized);
	error = 0;

	extracnt = ((aextra >> 24) & 3);
	if (extracnt)
	{
		// reverse byte order
		uint32_t edr = __REV(aextra);
		if (1 == extracnt)
		{
			extradata[0] = (aextra & 0xFF);
		}
		else if (2 == extracnt)
		{
			extradata[0] = ((aextra >> 8) & 0xFF);
			extradata[1] = ((aextra >> 0) & 0xFF);
		}
		else if (3 == extracnt)
		{
			extradata[0] = ((aextra >> 16) & 0xFF);
			extradata[1] = ((aextra >>  8) & 0xFF);
			extradata[2] = ((aextra >>  0) & 0xFF);
		}
	}
	extraremaining = extracnt;
	runstate = 0;
	busy = true;  // start the state machine

	if (txdma && txdma->initialized)
	{
		regs->CR2 |= I2C_CR2_DMAEN;
	}
	else
	{
		regs->CR2 &= ~I2C_CR2_DMAEN;
	}

	regs->SR1 = 0x00FF; // clear all sticky errors

	Run();

	return HWERR_OK;
}


void THwI2c_stm32::Run()
{
	if (!busy)
	{
		return;
	}

	uint8_t  firstbyte;
	unsigned cr1;
	unsigned sr1 = regs->SR1;
	unsigned sr2 = regs->SR2;

	// check error flags
	if (!error)
	{
		if (sr1 & I2C_SR1_AF)
		{
			error = ERR_I2C_ACK;
		}
		else if (sr1 & I2C_SR1_ARLO)
		{
			error = ERR_I2C_ARBLOST;
		}
		else if (sr1 & I2C_SR1_BERR)
		{
			error = ERR_I2C_BUS;
		}
		else if (sr1 & I2C_SR1_OVR)
		{
			error = ERR_I2C_OVERRUN;
		}

		if (error)
		{
			// jump to error handling
			runstate = 90;
		}
	}

	switch (runstate)
	{
	case 0:  // wait until busy to start
		// wait until busy
		if (sr2 & I2C_SR2_BUSY)
		{
			return;
		}

		// Start bit
		cr1 =	(regs->CR1 & 0x00FF);
		if (!istx)	cr1 |= I2C_CR1_ACK;
		cr1 |= I2C_CR1_START;  // send start condition
		regs->CR1 = cr1;

		runstate = 1;
		break;

	case 1:  // wait for the start condition
		if ((sr1 & I2C_SR1_SB) == 0)
		{
			return;
		}

		regs->CR1 &= ~I2C_CR1_START; // clear start

		firstbyte = (devaddr << 1);  // LSB = 0: Master Transmitter, LSB = 1: Master Receiver
		if ((extraremaining == 0) && !istx)
		{
			firstbyte |= 1; // master receiver
		}
		else
		{
			// master transmitter
		}
		regs->DR = firstbyte;	// send the first byte

		runstate = 2;
		break;

	case 2: // wait until the address sent
		if (sr1 & I2C_SR1_ADDR)
		{
			if (extraremaining > 0)
			{
				runstate = 5;  Run();  return;  // jump to phase 5
			}
			else
			{
				if (istx)
				{
					runstate = 10;  Run();	return;  // jump to phase 10
				}
				else
				{
					runstate = 20;  Run();  return;  // jump to phase 20
				}
			}
		}
		else if (sr1 & I2C_SR1_ARLO)
		{
			// todo: timeout for receiving addr
		}
		break;

	case 5: // send extra bytes
		if (sr1 & I2C_SR1_TXE)
		{
			regs->DR = extradata[extracnt - extraremaining];
			--extraremaining;

			if (extraremaining == 0)
			{
				if (istx)
				{
					// continue with sending data
					runstate = 10;
					return;
				}
				else
				{
					runstate = 6;
				}
			}
		}
		break;

	case 6:  // wait for transfer finished to send re-start
		if (sr1 & I2C_SR1_BTF)
		{
			// send re-start
			cr1 =	(regs->CR1 & 0x00FF);
			cr1 |= I2C_CR1_ACK;
			cr1 |= I2C_CR1_START;  // send start condition
			regs->CR1 = cr1;
			runstate = 7;
		}
		break;

	case 7:  // wait for the re-start condition
		if ((sr1 & I2C_SR1_SB) == 0)
		{
			return;
		}

		regs->CR1 &= ~I2C_CR1_START; // clear start

		firstbyte = (devaddr << 1) | 1;  // LSB = 0: Master Transmitter, LSB = 1: Master Receiver
		regs->DR = firstbyte;	// send the first byte

		runstate = 20; // receive data bytes
		break;

	case 10: // start sending data bytes
		runstate = 11; // continue at sending data
		dmaused = (txdma && (remainingbytes > 0));
		if (dmaused)
		{
			xfer.srcaddr = dataptr;
			xfer.bytewidth = 1;
			xfer.count = remainingbytes;
			xfer.flags = 0;

			dataptr += xfer.count;
			remainingbytes = 0;

			txdma->StartTransfer(&xfer);
		}
		else
		{
			Run();  return; // jump to body now
		}
		break;

	case 11: // continue sending data bytes
		if (dmaused && txdma->Active())
		{
			return;
		}

		if (remainingbytes > 0)
		{
			if ((sr1 & I2C_SR1_TXE) == 0)  // TX Ready?
			{
				return;
			}

			regs->DR = *dataptr++;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				return;
			}
		}

		runstate = 29; // finish
		break;

	case 20: //	start receive
		runstate = 21; // continue at receive data
		dmaused = (rxdma && (remainingbytes > 2));
		if (dmaused)
		{
			xfer.dstaddr = dataptr;
			xfer.bytewidth = 1;
			xfer.count = remainingbytes - 2;
			xfer.flags = 0;

			dataptr += xfer.count;
			remainingbytes = 2;

			rxdma->StartTransfer(&xfer);
		}
		else
		{
			Run();  return; // jump to receive data now
		}
		break;

	case 21: // receive data bytes
		if (dmaused && rxdma->Active())
		{
			return;
		}

		if (remainingbytes > 0)
		{
			if ((sr1 & I2C_SR1_RXNE) == 0)  // RX Ready?
			{
				return;
			}

			if (remainingbytes <= 2)
			{
				regs->CR1 &= ~I2C_CR1_ACK;  // do not ack the following bytes
			}

			*dataptr++ = regs->DR;
			--remainingbytes;

			if (remainingbytes > 0)
			{
				return;
			}

			runstate = 29; // terminate
		}
		break;

	case 29: // wait last transfer to finish and send stop
		if ((sr1 & I2C_SR1_BTF) == 0)
		{
			return;
		}

		regs->CR1 |= I2C_CR1_STOP;  // send stop condition
		runstate = 30; // closing
		break;

	case 30: // closing
		if (sr2 & I2C_SR2_BUSY)   // wait until end of the transfer
		{
			return;
		}

		busy = false; // finished.
		runstate = 50;
	  break;

	case 50: // finished
		break;

	case 90: // handling errors
		regs->CR1 |= I2C_CR1_STOP;  // send stop condition
		runstate = 91;
		break;

	case 91:
		// todo: reset on timeout
		if (sr2 & I2C_SR2_BUSY)   // wait until end of the transfer
		{
			return;
		}
		busy = false; // finished.
		runstate = 50;
		break;

	} // case
}

#endif

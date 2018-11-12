/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include "common/mbox.h"

#define MBOX_FULL		0x80000000
#define	MBOX_EMPTY		0x40000000
#define BASE_ADJUST_V1 0

static uint32_t mbox_base = MBOX_BASE;

uintptr_t base_adjust = BASE_ADJUST_V1;

inline void mmio_write(uintptr_t reg, uint32_t data)
{
	*(volatile uint32_t *)(reg + base_adjust) = data;
}

inline uint32_t mmio_read(uintptr_t reg)
{
	return *(volatile uint32_t *)(reg + base_adjust);
}

void mbox_set_base(uint32_t base)
{
	mbox_base = base;
}

uint32_t mbox_read(uint8_t channel)
{
	while(1)
	{
		while(mmio_read(mbox_base + MBOX_STATUS) & MBOX_EMPTY);

		uint32_t data = mmio_read(mbox_base + MBOX_READ);
		uint8_t read_channel = (uint8_t)(data & 0xf);
		if(read_channel == channel)
			return (data & 0xfffffff0);
	}
}

void mbox_write(uint8_t channel, uint32_t data)
{
	while(mmio_read(mbox_base + MBOX_STATUS) & MBOX_FULL);
	mmio_write(mbox_base + MBOX_WRITE, (data & 0xfffffff0) | (uint32_t)(channel & 0xf));
}

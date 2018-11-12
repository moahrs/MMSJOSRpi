/**
 * @file bcm2835_emmc.h
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BCM2835_EMMC_H_
#define BCM2835_EMMC_H_

#include <stdint.h>

// BLKSIZECNT, offset 0x04
#define BCM2835_EMMC_BLKSIZECNT_BLKSIZE_MASK	((uint32_t)0x000003ff)	///< SDHCI_BLOCK_SIZE    0x04
#define BCM2835_EMMC_BLKSIZECNT_BLKCNT_MASK		((uint32_t)0xffff0000)	///< SDHCI_BLOCK_COUNT   0x06

// STATUS, offset 0x24
#define BCM2835_EMMC_STATUS_CMD_INHIBIT			((uint32_t)0x00000001)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_CMD_INHIBIT 0x00000001
#define BCM2835_EMMC_STATUS_DATA_INHIBIT		((uint32_t)0x00000002)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_DATA_INHIBIT 0x00000002
#define BCM2835_EMMC_STATUS_CARD_PRESENT   		((uint32_t)0x00010000)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_CARD_PRESENT 0x00010000

// CONTROL0, offset 0x28
#define BCM2835_EMMC_CONTROL0_USE_4BITBUS		((uint32_t)(1 << 1))	///< SDHCI_HOST_CONTROL  0x28, SDHCI_CTRL_4BITBUS 0x02
#define BCM2835_EMMC_CONTROL0_POWER_ON			((uint32_t)(1 << 8))	///< SDHCI_POWER_CONTROL 0x29, SDHCI_POWER_ON 0x01

// CONTROL1, offset 0x2C
#define BCM2835_EMMC_CONTROL1_CLK_INTLEN		((uint32_t)(1 << 0))	///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_EN 0x0001
#define BCM2835_EMMC_CONTROL1_CLK_STABLE		((uint32_t)(1 << 1))	///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_STABLE 0x0002
#define BCM2835_EMMC_CONTROL1_CLK_EN			((uint32_t)(1 << 2))	///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_CARD_EN	0x0004
// SDHCI_TIMEOUT_CONTROL 0x2E
#define BCM2835_EMMC_CONTROL1_SRST_HC 			((uint32_t)(1 << 24))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_ALL 0x01
#define BCM2835_EMMC_CONTROL1_RESET_CMD 		((uint32_t)(1 << 25))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_CMD 0x02
#define BCM2835_EMMC_CONTROL1_RESET_DATA		((uint32_t)(1 << 26))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_DATA 0x04

#define BCM2835_EMMC_CONTROL1_DATA_TOUNIT_MAX	((uint32_t)(0x000e0000))

// INTERRUPT, offset 0x30 --> SDHCI_INT_STATUS    0x30
// IRPT_MASK, offset 0x34 --> SDHCI_INT_ENABLE    0x34
// IRPT_EN,   offset 0x38 --> SDHCI_SIGNAL_ENABLE 0x38

// SLOTISR_VER, offset 0xFC --> SDHCI_SLOT_INT_STATUS 0xFC
//                          --> SDHCI_HOST_VERSION    0xFE
#define BCM2835_EMMC_SLOTISR_VER_SLOT_STATUS_MASK	((uint32_t)(0x000000FF))///< SDHCI_SLOT_INT_STATUS 0xFC
#define BCM2835_EMMC_SLOTISR_VER_VENDOR_MASK		((uint32_t)(0xFF000000))///< SDHCI_HOST_VERSION 0xFE, SDHCI_VENDOR_VER_MASK 0xFF00
#define BCM2835_EMMC_SLOTISR_VER_VENDOR_SHIFT		24						///< 	SDHCI_VENDOR_VER_SHIFT 8
#define BCM2835_EMMC_SLOTISR_VER_SDVERSION_MASK		((uint32_t)(0x00FF0000))///< SDHCI_HOST_VERSION 0xFE, SDHCI_SPEC_VER_MASK 0x00FF
#define BCM2835_EMMC_SLOTISR_VER_SDVERSION_SHIFT	16						///< 	SDHCI_SPEC_VER_SHIFT   0

typedef struct {
	 uint32_t ARG2;			///< 0x00
	 uint32_t BLKSIZECNT;	///< 0x04
	 uint32_t ARG1;			///< 0x08
	 uint32_t CMDTM;		///< 0x0C
	 uint32_t RESP0;		///< 0x10
	 uint32_t RESP1;		///< 0x14
	 uint32_t RESP2;		///< 0x18
	 uint32_t RESP3;		///< 0x1C
	 uint32_t DATA;				///< 0x20
	 uint32_t STATUS;			///< 0x24
	 uint32_t CONTROL0;			///< 0x28
	 uint32_t CONTROL1;			///< 0x2C
	 uint32_t INTERRUPT;		///< 0x30
	 uint32_t IRPT_MASK;		///< 0x34
	 uint32_t IRPT_EN;			///< 0x38
	 uint32_t CONTROL2;			///< 0x3C
	 uint32_t CAPABILITIES_0;	///< 0x40
	 uint32_t CAPABILITIES_1;	///< 0x44
	 uint32_t NOTINUSE1[2];
	 uint32_t FORCE_IRPT;		///< 0x50
	 uint32_t NOTINUSE2[7];
	 uint32_t BOOT_TIMEOUT;		///< 0x70
	 uint32_t DBG_SEL;			///< 0x74
	 uint32_t NOTINUSE3[2];
	 uint32_t EXRDFIFO_CFG;		///< 0x80
	 uint32_t EXRDFIFO_EN;		///< 0x84
	 uint32_t TUNE_STEP;		///< 0x88
	 uint32_t TUNE_STEPS_STD;	///< 0x8C
	 uint32_t TUNE_STEPS_DDR;	///< 0x90
	 uint32_t NOTINUSE4[23];
	 uint32_t SPI_INT_SPT;		///< 0xF0
	 uint32_t NOTINUSE5[2];
	 uint32_t SLOTISR_VER;		///< 0xFC
} BCM2835_EMMC_TypeDef;

#define BCM2835_EMMC_BASE		(BCM2835_PERI_BASE + 0x300000)
#define BCM2835_EMMC			((BCM2835_EMMC_TypeDef *) BCM2835_EMMC_BASE)


#endif /* BCM2835_EMMC_H_ */

/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Driver for the MMA8451Q device, this is a 14-bit accelerometer
// Karim Ladha 21-12-2012

// Includes
#include "HardwareProfile.h"
#include <stdio.h>
#include <TimeDelay.h>
#include "Peripherals/Accel.h"
#include "myI2C.h"

#ifndef ACCEL_I2C_MODE
	#error "This accelerometer is I2C only."
#endif

// The device id
#define ACCEL_DEVICE_ID		0x1a /*Staic response*/

// MMAQ8452Q registers
#define 	ACCEL_ADDR_STATUS			0x00
#define 	ACCEL_ADDR_F_STATUS			0x00
#define 	ACCEL_ADDR_OUT_X_MSB		0x01
#define 	ACCEL_ADDR_OUT_X_LSB		0x02
#define 	ACCEL_ADDR_OUT_Y_MSB		0x03
#define 	ACCEL_ADDR_OUT_Y_LSB		0x04
#define 	ACCEL_ADDR_OUT_Z_MSB		0x05
#define 	ACCEL_ADDR_OUT_Z_LSB		0x06
#define 	ACCEL_ADDR_F_SETUP			0x09
#define 	ACCEL_ADDR_TRIG_CFG			0x0A
#define 	ACCEL_ADDR_SYSMOD			0x0B
#define 	ACCEL_ADDR_INT_SOURCE		0x0C
#define 	ACCEL_ADDR_WHO_AM_I			0x0D
#define 	ACCEL_ADDR_XYZ_DATA_CFG		0x0E
#define 	ACCEL_ADDR_HP_FILTER_CUTOFF	0x0F
#define 	ACCEL_ADDR_PL_STATUS		0x10
#define 	ACCEL_ADDR_PL_CFG			0x11
#define 	ACCEL_ADDR_PL_COUNT			0x12
#define 	ACCEL_ADDR_PL_BF_ZCOMP		0x13
#define 	ACCEL_ADDR_P_L_THS_REG		0x14
#define 	ACCEL_ADDR_FF_MT_CFG		0x15
#define 	ACCEL_ADDR_FF_MT_SRC		0x16
#define 	ACCEL_ADDR_FF_MT_THS		0x17
#define 	ACCEL_ADDR_FF_MT_COUNT		0x18
#define 	ACCEL_ADDR_TRANSIENT_CFG	0x1D
#define 	ACCEL_ADDR_TRANSIENT_SRC	0x1E
#define 	ACCEL_ADDR_TRANSIENT_THS	0x1F
#define 	ACCEL_ADDR_TRANSIENT_COUNT	0x20
#define 	ACCEL_ADDR_PULSE_CFG		0x21
#define 	ACCEL_ADDR_PULSE_SRC		0x22
#define 	ACCEL_ADDR_PULSE_THSX		0x23
#define 	ACCEL_ADDR_PULSE_THSY		0x24
#define 	ACCEL_ADDR_PULSE_THSZ		0x25
#define 	ACCEL_ADDR_PULSE_TMLT		0x26
#define 	ACCEL_ADDR_PULSE_LTCY		0x27
#define 	ACCEL_ADDR_PULSE_WIND		0x28
#define 	ACCEL_ADDR_ASLP_COUNT		0x29
#define 	ACCEL_ADDR_CTRL_REG1		0x2A
#define 	ACCEL_ADDR_CTRL_REG2		0x2B
#define 	ACCEL_ADDR_CTRL_REG3		0x2C
#define 	ACCEL_ADDR_CTRL_REG4		0x2D
#define 	ACCEL_ADDR_CTRL_REG5		0x2E
#define 	ACCEL_ADDR_OFF_X			0x2F
#define 	ACCEL_ADDR_OFF_Y			0x30
#define 	ACCEL_ADDR_OFF_Z			0x31

#define 	ACCEL_INT_ASLP 			0x80 
#define 	ACCEL_INT_FIFO			0x40 
#define 	ACCEL_INT_TRANS 		0x20
#define 	ACCEL_INT_LNDPRT 		0x10
#define 	ACCEL_INT_PULSE 		0x08
#define 	ACCEL_INT_FF_MT 		0x04 
#define 	ACCEL_INT_DRDY			0x01

#define		RATE_800HZ	0
#define		RATE_400HZ	1
#define		RATE_200HZ	2
#define		RATE_100HZ	3
#define		RATE_50HZ	4
#define		RATE_12HZ	5
#define		RATE_6HZ	6
#define		RATE_1HZ	7

// I2C routines
#define USE_AND_OR
#include "myI2C.h"
#define LOCAL_I2C_RATE		((OSCCONbits.COSC==1)? 7 : 1)		/*2MHz for this device, controls baud*/
#define ACCEL_ADDRESS		0x38 	/*I2C address*/
#define ACCEL_MASK_READ  	0x01 	/*I2C_READ_MASK*/

// I2C functions for using the ACCEL.c code
#define ACCELIdleI2C        myI2CIdle
#define ACCELStartI2C       myI2CStart
#define ACCELWriteI2C       myI2Cputc
#define ACCELStopI2C        myI2CStop
#define ACCELAckI2C         myI2CAck
#define ACCELNackI2C 	    myI2CNack
#define ACCELReadI2C        myI2Cgetc
#define ACCELRestartI2C     myI2CRestart
#define ACCELOpenI2C()	    myI2COpen()
#define ACCELWaitStartI2C() WaitStartmyI2C()
#define ACCELWaitStopI2C()  WaitStopmyI2C()
#define ACCELWaitRestartI2C() WaitRestartmyI2C()
#define ACCELCloseI2C()		myI2CClose()


// I2C - (OR register in ACCELAddressX with ACCEL_MASK_BURST)
#define ACCELOpen()              ACCELOpenI2C();ACCELStartI2C(); ACCELWaitStartI2C();
#define ACCELAddressRead(_r)     ACCELWriteI2C(ACCEL_ADDRESS); ACCELWriteI2C((_r)); ACCELRestartI2C(); ACCELWaitStartI2C(); ACCELWriteI2C(ACCEL_ADDRESS | ACCEL_MASK_READ);
#define ACCELAddressWrite(_r)    ACCELWriteI2C(ACCEL_ADDRESS); ACCELWriteI2C((_r)); 
#define ACCELReadContinue()      ACCELReadI2C(); ACCELAckI2C()
#define ACCELReadLast()          ACCELReadI2C(); ACCELNackI2C()
#define ACCELWrite(_v)           ACCELWriteI2C((_v));
#define ACCELClose()             ACCELStopI2C(); ACCELWaitStopI2C();ACCELCloseI2C();
#define ACCELReopen()            ACCELRestartI2C(); ACCELWaitRestartI2C();


static unsigned char 	accelRate = 0;
static unsigned char 	accelRange = 0;
static unsigned short 	accelFrequency = 0;
				char 	accelPresent = 0;
	   unsigned char 	accelOrientation = 0;
	   unsigned char 	accelTapStaus = 0;

unsigned char AccelRate(void) 		{ return accelRate; }
unsigned short AccelFrequency(void) { return accelFrequency; }

// Read device ID
unsigned char AccelVerifyDeviceId(void)
{
	unsigned char id;	
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_WHO_AM_I);
	id = ACCELReadLast();
	ACCELClose();
    accelPresent = (id == ACCEL_DEVICE_ID) ? 1 : 0;
	return accelPresent;
}

// AccelStartup
void AccelStartup(unsigned char setting)
{
	unsigned int ctrlreg1;
	ACCELOpen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1); 
	ACCELWrite(0); // power off device so you can access the other ctrl regs	
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_XYZ_DATA_CFG);
	accelRange = setting>>4;
	switch (accelRange){
		case (ACCEL_RANGE_8G) : {ACCELWrite(0b00000010);break;}
		case (ACCEL_RANGE_4G) : {ACCELWrite(0b00000001);break;}
		case (ACCEL_RANGE_2G) : {ACCELWrite(0b00000000);break;}
		default : 				{ACCELWrite(0b00000001);break;}
	}/*	XYZ_DATA_CFG
			b4		: 	HPF on/off 	- 0: High pass filter on data out off
			b1-0	:	Data res	- 10: +/- 4g range*/
	ACCELWrite(0b00110000);	/*	HP_FILTER_CUTOFF
			b5		: 	Pulse_HPF_BYP 	- 0: HPF enabled for Pulse Processing
			b4		:	Pulse_LPF_EN	- 0: LPF disabled for Pulse Processing
			b1-0	:	SEL[1:0]		- 00:See data sheet HPF Cut-off frequency selection*/
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_PL_CFG);	
	ACCELWrite(0b11000000);	/*	PL_CFG
			b7		: 	DBCNTM			- 1: Clears counter whenever condition of interest is no longer valid.
			b6		:	PL_EN			- 0: Portrait/Landscape Detection is Disabled.*/
	ACCELWrite(25);			/*	PL_COUNT
			b7-0	:	Portrait/Landscape Debounce Counter*/
	ACCELWrite(0b01000100);	/*	PL_BF_ZCOMP
			b7-6	:	BKFR[7:6] Back Front Trip Angle Fixed Threshold = 01 which is = �75�.
			b2-0	:	ZLOCK[2:0] Z-Lock Angle Fixed Threshold = 100 which is 29�.*/
	ACCELWrite(0b10000100);	/*	P_L_THS_REG
			b7-3	:	P_L_THS[7:3] Portrait/Landscape Fixed Threshold angle = 1_0000 (45�).
			b2-0	:	HYS[2:0] This is a fixed angle added to the threshold angle fixed at �14�, which is 100.*/
	ACCELWrite(0b00000000);	/*	FF_MT_CFG
			b7		:	ELE		0: Event flag latch disabled; 1: event flag latch enabled
			b6		:	OAE		0: Freefall Flag  1: Motion Flag 
			b5		:	ZEFE	Event flag enable on Z Default value: 0.
			b4		:	YEFE	Event flag enable on Y event. Default value: 0.
			b3		:	XEFE	Event flag enable on X event. Default value: 0.*/
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_FF_MT_THS);
	ACCELWrite(0b00000000);	/*	FF_MT_THS
			b7		:	DBCNTM	0: increments or decrements debounce, 1: increments or clears counter.
			b6-0	:	THS[6:0] Freefall /Motion Threshold: Default value: 000_0000.*/
	ACCELWrite(0b00000000);	/*	FF_MT_COUNT
			b7-0	:	D[7:0] Count value. Default value: 0000_0000*/
	ACCELWrite(0b00010000);	/*	TRANSIENT_CFG
			b4		:	ELE		0: Event flag latch disabled; 1: Event flag latch enabled
			b3		:	ZTEFE	0: Event detection disabled; 1: Raise event flag on measured acceleration delta value greater than transient threshold.
			b2		:	YTEFE	0: Event detection disabled; 1: Raise event flag on measured acceleration delta value greater than transient threshold.
			b1		:	XTEFE	0: Event detection disabled; 1: Raise event flag on measured acceleration delta value greater than transient threshold.
			b0		:	HPF_BYP 0: Data to transient acceleration detection block is through HPF 1: Data to transient acceleration detection block is NOT through*/
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_TRANSIENT_THS);
	ACCELWrite(0b00000000);	/*	TRANSIENT_THS
			b7		:	0: increments or decrements debounce; 1: increments or clears counter.
			b6-0	:	THS[6:0] Transient Threshold: Default value: 000_0000.*/
	ACCELWrite(0b00000000);	/*	TRANSIENT_COUNT
			b7-0	:	D[7:0] Count value. Default value: 0000_0000.*/
	ACCELWrite(0b01000000);	/*	PULSE_CFG
			b7		:	DPA	0: 	Double Pulse detection is not aborted if the start of a pulse is detected during the time period specified by the PULSE_LTCY register.
							1: 	Setting the DPA bit momentarily suspends the double tap detection if the start of a pulse is detected during the time period specified
								by the PULSE_LTCY register and the pulse ends before the end of the time period specified by the PULSE_LTCY register.
			b6		:	ELE		0: Event flag latch disabled; 1: Event flag latch enabled
			b5		:	ZDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b4		:	ZSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse
			b3		:	YDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b2		:	YSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse
			b1		:	XDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b0		:	XSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse*/
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_PULSE_THSX);
	ACCELWrite(0b00000000);	/*	PULSE_THSX Pulse threshold x*/
	ACCELWrite(0b00000000);	/*	PULSE_THSY Pulse threshold y*/
	ACCELWrite(0b00000000);	/*	PULSE_THSZ Pulse threshold z*/
	ACCELWrite(0b00000000);	/*	PULSE_TMLT TMLT[7:0] Pulse Time Limit. Default value: 0000_0000.*/
	ACCELWrite(0b00000000);	/*	PULSE_LTCY LTCY[7:0] Latency Time Limit. Default value: 0000_0000 */
	ACCELWrite(0b00000000);	/*	PULSE_WIND WIND[7:0] Second Pulse Time Window. Default value: 0000_0000.*/
	ACCELWrite(0b00000000);	/*	ASLP_COUNT D[7:0] Duration value. Default value: 0000_0000 */

	accelRate = setting&0xf;
	ctrlreg1 = 0; // Standby mode
	switch (accelRate) {
	case (ACCEL_RATE_800) :	{ctrlreg1|=0b00000000;break;}
	case (ACCEL_RATE_400) :	{ctrlreg1|=0b00001000;break;}
	case (ACCEL_RATE_200) :	{ctrlreg1|=0b00010000;break;}
	case (ACCEL_RATE_100) :	{ctrlreg1|=0b00011000;break;}
	case (ACCEL_RATE_50) : 	{ctrlreg1|=0b00100000;break;}
	case (ACCEL_RATE_12_5) :{ctrlreg1|=0b00101000;break;}
	case (ACCEL_RATE_6_25) :{ctrlreg1|=0b00110000;break;}
	case (ACCEL_RATE_1_56) :{ctrlreg1|=0b00111000;break;}
	default	:	{accelRate=ACCEL_RATE_100;ctrlreg1|=0b00011000;break;}}/*100Hz default*/
	accelFrequency = ACCEL_FREQUENCY_FOR_RATE(setting);
	ACCELWrite(ctrlreg1);
	/*ACCELWrite(0b00011001);*/	/*	CTRL_REG1
			b7-6	:	ASLP_RATE[1:0] Configures the Auto-WAKE sample frequency when the device is in SLEEP Mode. Default value: 00.
			b5-3	:	DR[2:0] Data rate selection. 011 -> 100Hz
			b2		:	LNOISE (0: Normal mode; 1: Reduced Noise mode)
			b1		:	F_READ (0: Normal mode 1: Fast Read Mode)
			b0		:	ACTIVE (0: STANDBY mode; 1: ACTIVE mode)*/	
	ACCELWrite(0b00000000);	/*	CTRL_REG2
			b7		:	ST 0: Self-Test disabled; 1: Self-Test enabled
			b6		:	RST 0: Device reset disabled; 1: Device reset enabled.
			b4-3	:	SMODS[1:0] SLEEP mode power scheme selection. Default value: 00.
			b2		:	SLPE Auto-SLEEP enable. Default value: 0.
			b1-0	:	MODS[1:0] ACTIVE mode power scheme selection. Default value: 00.*/
	ACCELWrite(0b00000010);	/*	CTRL_REG3
			b7		:	FIFO_GATE	0: FIFO gate is bypassed. FIFO is flushed upon sleep 1: FIFO preserved on sleep
			b6		:	WAKE_TRANS 	0: Transient function is bypassed in SLEEP mode. Default value: 0.
			b5		:	WAKE_LNDPRT 0: Orientation function is bypassed in SLEEP mode. Default value: 0.
			b4		:	WAKE_PULSE 	0: Pulse function is bypassed in SLEEP mode. Default value: 0.
			b3		:	WAKE_FF_MT 	0: Freefall/Motion function is bypassed in SLEEP mode. Default value: 0.
			b1		:	IPOL 		1: ACTIVE high Interrupt polarity ACTIVE high 1, or ACTIVE low 0. Default value: 0.
			b0		:	PP_OD		0:Push-Pull/1:Open Drain selection on interrupt pad. Default value: 0.*/
	ACCELWrite(0b00000000);	/*	CTRL_REG4
			b7		:	INT_EN_ASLP		0: Auto-SLEEP/WAKE interrupt disabled; 1: Auto-SLEEP/WAKE interrupt enabled.
			B6		: 	INT_EN_FIFO		0: FIFO interrupt off
			b5		:	INT_EN_TRANS	0: Transient interrupt disabled; 1: Transient interrupt enabled.
			b4		:	INT_EN_LNDPRT	0: Orientation (Landscape/Portrait) interrupt disabled.
			b3		:	INT_EN_PULSE	0: Pulse Detection interrupt disabled; 1: Pulse Detection interrupt enabled
			b2		:	INT_EN_FF_MT	0: Freefall/Motion interrupt disabled; 1: Freefall/Motion interrupt enabled
			b0		:	INT_EN_DRDY		0: Data Ready interrupt disabled; 1: Data Ready interrupt enabled*/
	ACCELWrite(0b00000000);	/*	CTRL_REG5
			b7		:	INT_CFG_ASLP	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b6		:	INT_CFG_FIFO	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b5		:	INT_CFG_TRANS	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b4		:	INT_CFG_LNDPRT	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b3		:	INT_CFG_PULSE	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b2		:	INT_CFG_FF_MT	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
			b0		:	INT_CFG_DRDY	0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin*/
	ACCELWrite(0b00000000);	/*	OFF_X Offset Correction X Register */
	ACCELWrite(0b00000000);	/*	OFF_Y Offset Correction Y Register */
	ACCELWrite(0b00000000);	/*	OFF_Z Offset Correction Z Register */
	ACCELReopen();
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1);
	ACCELWrite(ctrlreg1|1); // Turn on device
	ACCELClose();
	return;
}

// Shutdown the accelerometer to standby mode (standby mode, interrupts disabled)
void AccelStandby(void)
{
	ACCELOpen();
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1);
	ACCELWrite(0b00011000);	/*	CTRL_REG1
			b7-6	:	ASLP_RATE[1:0] Configures the Auto-WAKE sample frequency when the device is in SLEEP Mode. Default value: 00.
			b5-3	:	DR[2:0] Data rate selection. 011 -> 100Hz
			b2		:	LNOISE (0: Normal mode; 1: Reduced Noise mode)
			b1		:	F_READ (0: Normal mode 1: Fast Read Mode)
			b0		:	ACTIVE (0: STANDBY mode; 1: ACTIVE mode)*/	
	ACCELClose();
}

/*Note: The samples are Left justified signed and 12bit*/
void AccelSingleSample(accel_t *value)
{
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_OUT_X_MSB);
	value->xh = ACCELReadContinue();
	value->xl = ACCELReadContinue();
	value->yh = ACCELReadContinue();
	value->yl = ACCELReadContinue();
	value->zh = ACCELReadContinue();
	value->zl = ACCELReadLast();
	ACCELClose();
	return;
}

// Enable interrupts - FIFO enabled and interrupting
void AccelEnableInterrupts(unsigned char flags, unsigned char pinMask)
{
	unsigned char ctrl_reg1;
	/*POWER OFF DEVICE*/
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_CTRL_REG1);
	ctrl_reg1 = ACCELReadLast();
	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1); 
	ACCELWrite(0); // power off device so you can access the other ctrl regs	
	ACCELReopen();	
	/*SETUP FIFO*/
	ACCELAddressWrite(ACCEL_ADDR_F_SETUP); 
	ACCELWrite(0b10011001);	/*F_SETUP
	b7-6	00: FIFO is disabled.
			01: FIFO contains the most recent samples when overflowed (circular buffer). Oldest sample is discarded to be replaced by new sample.
			->10: FIFO stops accepting new samples when overflowed.
			11: Trigger mode. The FIFO will be in a circular mode up to the number of samples in the watermark.
	b5-0	Watermark: 25 = 0b011001 */
	ACCELWrite(0);	/*TRIG_CFG - enables trigger sources used to start the FIFO capture
	b5		Trig_TRANS 
	b4		Trig_LNDPRT 
	b3		Trig_PULSE 
	b2		Trig_FF_MT */
	/*SETUP PULSE DETECTION*/
	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_PULSE_CFG); 
	ACCELWrite(0b01111111); /*PULSE_CFG
			b7		:	DPA	0: 	Double Pulse detection is not aborted if the start of a pulse is detected during the time period specified by the PULSE_LTCY register.
							1: 	Setting the DPA bit momentarily suspends the double tap detection if the start of a pulse is detected during the time period specified
								by the PULSE_LTCY register and the pulse ends before the end of the time period specified by the PULSE_LTCY register.
			b6		:	ELE		0: Event flag latch disabled; 1: Event flag latch enabled
			b5		:	ZDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b4		:	ZSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse
			b3		:	YDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b2		:	YSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse
			b1		:	XDPEFE	0: Event detection disabled; 1: Event detection enabled double pulse
			b0		:	XSPEFE	0: Event detection disabled; 1: Event detection enabled single pulse*/
	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_PULSE_THSX); /*7 bit values, 0.063g/LSB fixed, 32 = 2g*/ 
	ACCELWrite(32); /*X pulse threshold*/
	ACCELWrite(32); /*Y pulse threshold*/
	ACCELWrite(32); /*Z pulse threshold*/
	/*Note: The startup code turns on LPF and HPF for pulse detection.*/
	ACCELWrite(2);	/*<20ms> PULSE_TMLT - multiple of sample timestep/4 (x by 4 if LPF is on), defines pulse width limit*/
	ACCELWrite(2); 	/*<40ms> PULSE_LTCY - time after pulse detection that device ignores other pulses in Tsamp/2 (x4 for LPF)*/
	ACCELWrite(25); /*<250ms>PULSE_WIND - window in which second pulse can arrive to create double tap, same step as PULSE_TMLT*/
	ACCELReopen();	
	// ENABLE SELECTED INTERRUPTS
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG4); 
	ACCELWrite(flags);		/*	CTRL_REG4
			b7		:	INT_EN_ASLP		0: Auto-SLEEP/WAKE interrupt disabled; 1: Auto-SLEEP/WAKE interrupt enabled.
			B6		: 	INT_EN_FIFO		0: FIFO interrupt off
			b5		:	INT_EN_TRANS	0: Transient interrupt disabled; 1: Transient interrupt enabled.
			b4		:	INT_EN_LNDPRT	0: Orientation (Landscape/Portrait) interrupt disabled.
			b3		:	INT_EN_PULSE	0: Pulse Detection interrupt disabled; 1: Pulse Detection interrupt enabled
			b2		:	INT_EN_FF_MT	0: Freefall/Motion interrupt disabled; 1: Freefall/Motion interrupt enabled
			b0		:	INT_EN_DRDY		0: Data Ready interrupt disabled; 1: Data Ready interrupt enabled*/
	ACCELWrite(pinMask);	/*	CTRL_REG5 - which ints are mapped to int1 pin */
	// TURN ON DEVICE
	ACCELReopen();	
	ACCELAddressWrite(ACCEL_ADDR_CTRL_REG1); 
	ACCELWrite(ctrl_reg1); /*Re-enable device*/
	ACCELClose();

	// Empty the fifo, read tap and orientation regs to clear flags
	AccelReadFIFO(NULL,32);
	AccelReadTapStatus();
	AccelReadOrientaion();

	// Now enable the interrupts
	ACCEL_INT1_IP = ACCEL_INT_PRIORITY;
	ACCEL_INT1_IF = 0;
	ACCEL_INT1_IE = 1;
	ACCEL_INT2_IP = ACCEL_INT_PRIORITY;
	ACCEL_INT2_IF = 0;
	ACCEL_INT2_IE = 1;

	/*	To clear the interrupts you need to read specific registers:
		FIFO cleared on reading F_STATUS
		LNDPRT cleared on reading PL_STATUS
		PULSE cleared on reading PULSE_SRC */
}

// Read number of bytes in fifo - if the fifo has not been initialised this will return unknown
unsigned char AccelReadFifoLength(void)
{
	unsigned char number_in_fifo;
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_F_STATUS); // Bottom 6 bits are number of samples
	number_in_fifo = ACCELReadLast();
	number_in_fifo &= 0x3f;
	ACCELClose();
	return number_in_fifo;
}

// Read at most 'maxEntries' 3-axis samples (3 words = 6 bytes) from the accelerometer FIFO into the specified RAM buffer
unsigned char AccelReadFIFO(accel_t *accelBuffer, unsigned char maxEntries)
{
	unsigned char number_in_fifo, number_read=0;
	unsigned char * ptr = (unsigned char*)accelBuffer; // Pointer cast to bytes
	if (maxEntries == 0) return 0;

	ACCELOpen();	
	ACCELWriteI2C(ACCEL_ADDRESS | ACCEL_MASK_READ); // The first reg to read will be F_STATUS
	number_in_fifo = ACCELReadContinue();
	number_in_fifo &= 0x3f;

	while ((number_in_fifo>1)&&(maxEntries>1))
	{
		if (accelBuffer == NULL)
			{ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();
			ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();}
		else
		{
			*(ptr+1) = ACCELReadContinue(); // XH
			*(ptr+0) = ACCELReadContinue(); // XL
			*(ptr+3) = ACCELReadContinue(); // YH
			*(ptr+2) = ACCELReadContinue(); // YL
			*(ptr+5) = ACCELReadContinue(); // ZH
			*(ptr+4) = ACCELReadContinue(); // ZL
			ptr+=6;
		}
		number_in_fifo--;
		maxEntries--;
		number_read++;		
	}
	if (accelBuffer == NULL)
		{ACCELReadContinue();ACCELReadContinue();ACCELReadContinue();
		ACCELReadContinue();ACCELReadContinue();ACCELReadLast();}
	else
	{
		*(ptr+1) = ACCELReadContinue(); // XH
		*(ptr+0) = ACCELReadContinue(); // XL
		*(ptr+3) = ACCELReadContinue(); // YH
		*(ptr+2) = ACCELReadContinue(); // YL
		*(ptr+5) = ACCELReadContinue(); // ZH
		*(ptr+4) = ACCELReadLast(); 	// ZL
	}
	ACCELClose();
	number_read++;

	return number_read;
}


// Read tap status
unsigned char AccelReadTapStatus(void)
{
	unsigned char tap_status_reg;
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_PULSE_SRC);
	tap_status_reg = ACCELReadLast();
	ACCELClose();
	/*
	b7	- EA , pulse event has occured
	b6	- AxZ, pulse on Z axis
	b5  - AxY,
	b4 	- AxX,
	b3	- DPE, double pulse on last event
	b2	- PolZ, Pulse was positive
	b1	- PolY,
	b0	- PolX,
	*/
	return tap_status_reg;
}

// Read tap status
unsigned char AccelReadOrientaion(void)
{
	unsigned char pl_status_reg;
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_PL_STATUS);
	pl_status_reg = ACCELReadLast();
	ACCELClose();
	/*
	b7	- NEWLP, 	New orientaion detected
	b6	- LO,		Lockout detected		
	b2-1	- LAPO[1:0]	00 Portrait up, 01 Portrait down, 10 Landscape right, 11 Landscape left
	b0	- BAFRO		0 Back facing, 1 Front facing
	*/
	return pl_status_reg;
}

// Read interrupt source
unsigned char AccelReadIntSource(void)
{
	unsigned char reason;	
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_INT_SOURCE);
	reason = ACCELReadLast();
	ACCELClose();
	return reason;
}


// Debug dump registers
void AccelDebugDumpRegisters(void)
{
	static unsigned char i,regs[100];
	ACCELOpen();	
	ACCELAddressRead(ACCEL_ADDR_CTRL_REG1); 
	for (i=0;i<100;i++)
	{
		regs[i] = ACCELReadContinue();	
	}
	ACCELReadLast();
	ACCELClose();
	Nop();
	Nop();
	Nop();
	Nop();
 	return;
}

// 
void AccelPackData(short *input, unsigned char *output)
{
	// This function could be implemented if <14 bits were needed
	// For 14 bits, the best packing is 42 bits from 48 bits  - a small saving of only 6 bits 
}

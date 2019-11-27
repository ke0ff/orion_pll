/****************************************************************************************
 ****************** COPYRIGHT (c) 2019 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: channels.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the ADF4351 PLL setup application
 *
 *  File scope revision history:
 *    10-20-19 jmh:  Created custom file for K7AYP
 *    04-29-17 jmh:  Rev 1.2:
 *					 Added #if build option to support NUM_CHAN such that only the required number of channels
 *						are supported.
 *    06-12-16 jmh:  Rev 0.1:
 *                   Added notes for linker settings.
 *					 Updated PLL registers for Orion-I PLL
 *    04-29-16 jmh:  Rev 0.0:
 *                   Initial file creation
 *					 This source file holds the channel data arrays in the top sector of FLASH
 *
 *	To get the pll_ch[] array to target a specific FLASH address, configure the linker (for Keil,
 *	this is in the options dialog, under BL51 Locate, enter "?CO?CHANNELS(0x1280)" into the Code: field)
 *	to set the target address to 0x1280 to allow space for 100 channels (plus some spares).
 *
 ***************************************************************************************/
#include "typedef.h"
#include "init.h"

//#define	CHDEBUG		// enables debug data pattern

	// declarations for PLL channel data
	//	A number, N, channels are allocated (N = 100, typ.), with the first channel in the list being 00,
	//	and the last being N-1.
	//	Each Channel is organized as 6, 32 bit registers (for the ADF-4351) starting with register 0 at
	//	the lowest address, and register 5 at the highest address.
	//	Unused channels should have R5 set to 0xFFFFFFFF (the SW detects this and disables the ADF4351 output).
	//	0xFFFFFFFF is the implicit value of a register location assuming that the FLASH bytes are in the erased state.
	//
	// These register data are for an Orion-I with 10 MHz ref osc, and set -4dBm output level:
	U32 code pll_ch_array[] = {

	//	        R0          R1          R2          R3          R4          R5		// ADF reg#s
		// Ch 00
		0x00B40108,	0x08008141, 0x00004E62, 0x000004B3, 0x00A50434, 0x00580005,		// 10 MHz Reference Clock and 902.0625 MHz, PLL disabled, +5dBm (RX)
		// Ch 01
		0x00730070, 0x080080C9, 0x00004E42, 0x000004B3, 0x00C50024, 0x00580005,		// 10 MHz Reference Clock and 144.1 MHz (RX) -4dBm
		// Ch 02 - 1152.0
		0x00730010, 0x00008029, 0x00004E42, 0x000004B3, 0x0095042C, 0x00580005,		// 10 MHz Reference Clock and 1152 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
		// Ch 03 - 1872.0
		0x00BB0010, 0x00008029, 0x00004E42, 0x000004B3, 0x0095042C, 0x00580005,		// 10 MHz Reference Clock and 1872 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
#if NUM_CHAN > 4
		// Ch 04 - 2160.0
		0x00D80000, 0x08008011, 0x00004E42, 0x000004B3, 0x0095042C, 0x00580005,		// 10 MHz Reference Clock and 2160 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 5
		// Ch 05 - 3024.0
		0x00970010, 0x00008029, 0x00004E42, 0x000004B3, 0x0085042C, 0x00580005,		// 10 MHz Reference Clock and 3024 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 6
		// Ch 06 - 3312.0
		0x00A58008, 0x00008029, 0x00004E42, 0x000004B3, 0x0085042C, 0x00580005,		// 10 MHz Reference Clock and 3312 MHz (RX) -1dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 7
		// Ch 07 - 1776.0 (x3 = 5328.0)
		0x00B18008, 0x00008029, 0x00004E42, 0x000004B3, 0x0095043C, 0x00580005,		// 10 MHz Reference Clock and 1776 MHz (RX) +5dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 8
		// Ch 08 - 1872.0 (x3 = 5616.0)
		0x00BB0010, 0x00008029, 0x00004E42, 0x000004B3, 0x0095043C, 0x00580005,		// 10 MHz Reference Clock and 1872 MHz (RX) +5dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 9
		// Ch 09 - 3312.0 (x3 = 9936.0)
		0x00A58008, 0x00008029, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 3312 MHz (RX) +5dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 10
		// Ch 10 - 3408.0 (x3 = 10224.0)
		0x00AA0020, 0x00008029, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 3408 MHz (RX) +5dBm, 100KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 11
		// Ch 11 - 3456.1
		0x00AC8018, 0x00008029, 0x00004E42, 0x000004B3, 0x0085043c, 0x00580005,		// 10 MHz Reference Clock and 3456 MHz (RX) +5dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 12
		// Ch 12
		0x00BC0080, 0x000080C9, 0x00004E42, 0x000004B3, 0x00B50034, 0x00580005,		// 10 MHz Reference Clock and 470.8 MHz (RX) +2dBm, 10KHz channel, MTLD, PS 4/5
#endif
#if NUM_CHAN > 13
		// Ch 13
		0x007C8018, 0x00008029, 0x00004E42, 0x000004B3, 0x00D5043C, 0x00580005,		// 10 MHz Reference Clock and 78 MHz (RX) +5dBm, 100KHz chan, MTLD, PS 4/5
#endif
#if NUM_CHAN > 14
		// Ch 14
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 15
		// Ch 15
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 16
		// Ch 16
		0x006E0000, 0x08008011, 0x00004E42, 0x000004B3, 0x00E50024, 0x00580005,		// 10 MHz Reference Clock and 34.375 MHz (RX) -4dBm, PS 4/5
#endif
#if NUM_CHAN > 17
		// Ch 17 - 432.1
		0x00AC8088, 0x000080C9, 0x00004E42, 0x000004B3, 0x00B50424, 0x00580005,		// 10 MHz Reference Clock and 432.1 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 18
		// Ch 18 - 902.1
		0x00B400A8, 0x000080C9, 0x00004E42, 0x000004B3, 0x00A50424, 0x00580005,		// 10 MHz Reference Clock and 902.1 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 19
		// Ch 19 - 1296.1
		0x00818058, 0x00008191, 0x00004E42, 0x000004B3, 0x00950424, 0x00580005,		// 10 MHz Reference Clock and 1296.1 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 20
		// Ch 20 - 2304.1
		0x00730148, 0x00008321, 0x00004E42, 0x000004B3, 0x00850424, 0x00580005,		// 10 MHz Reference Clock and 2304.1 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 21
		// Ch 21 - 3456.1
		0x00AC81E8, 0x00008321, 0x00004E42, 0x000004B3, 0x00850424, 0x00580005,		// 10 MHz Reference Clock and 3456.1 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 22
		// Ch 22 - 1920.03333 (x3 = 5760.096)
		0x00C00020, 0x08009389, 0x00004E42, 0x000004B3, 0x0095003C, 0x00580005,		// 10 MHz Reference Clock and 1920.032 MHz (RX) +5dBm, MTLD, Rcnt = 1, , PS 8/9
#endif
#if NUM_CHAN > 23
		// Ch 23 - 3456.032 (x3 = 10368.096 MHz)
		0x00AC8BC8, 0x08009389, 0x00004E42, 0x000004B3, 0x0085003C, 0x00580005,		// 10 MHz Reference Clock and 3456.03333 MHz (RX) +5dBm, MTLD, Rcnt = 1, , PS 8/9
#endif
#if NUM_CHAN > 24
		// Ch 24 - 3000
		0x00960000, 0x00008011, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 3000 MHz (RX) +5dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 25
		// Ch 25 - 3000
		0x00960000, 0x08008011, 0x00004E42, 0x000004B3, 0x0085043C, 0x00580005,		// 10 MHz Reference Clock and 3000 MHz (RX) +5dBm, MTLD, PS 8/9
#endif
#if NUM_CHAN > 26
		// Ch 26
		0x00960000, 0x00008011, 0x00004E42, 0x000004B3, 0x00850424, 0x00580005,		// 10 MHz Reference Clock and 3000 MHz (RX) -4dBm, MTLD, PS 4/5
#endif
#if NUM_CHAN > 27
		// Ch 27
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 28
		// Ch 28
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 29
		// Ch 29
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 30
		// Ch 30
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 31
		// Ch 31
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 32
		// Ch 32
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 33
		// Ch 33
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 34
		// Ch 34
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 35
		// Ch 35
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 36
		// Ch 36
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 37
		// Ch 37
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 38
		// Ch 38
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 39
		// Ch 39
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 40
		// Ch 40
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 41
		// Ch 41
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 42
		// Ch 42
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 43
		// Ch 43
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 44
		// Ch 44
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 45
		// Ch 45
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 46
		// Ch 46
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 47
		// Ch 47
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 48
		// Ch 48
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 49
		// Ch 49
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 50
		// Ch 50
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 51
		// Ch 51
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 52
		// Ch 52
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 53
		// Ch 53
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 54
		// Ch 54
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 55
		// Ch 55
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 56
		// Ch 56
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 57
		// Ch 57
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 58
		// Ch 58
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 59
		// Ch 59
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 60
		// Ch 60
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 61
		// Ch 61
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 62
		// Ch 62
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 63
		// Ch 63
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 64
		// Ch 64
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 65
		// Ch 65
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 66
		// Ch 66
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 67
		// Ch 67
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 68
		// Ch 68
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 69
		// Ch 69
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 70
		// Ch 70
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 71
		// Ch 71
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 72
		// Ch 72
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 73
		// Ch 73
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 74
		// Ch 74
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 75
		// Ch 75
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 76
		// Ch 76
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 77
		// Ch 77
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 78
		// Ch 78
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 79
		// Ch 79
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 80
		// Ch 80
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 81
		// Ch 81
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 82
		// Ch 82
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 83
		// Ch 83
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 84
		// Ch 84
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 85
		// Ch 85
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 86
		// Ch 86
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 87
		// Ch 87
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 88
		// Ch 88
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 89
		// Ch 89
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 90
		// Ch 90
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 91
		// Ch 91
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 92
		// Ch 92
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 93
		// Ch 93
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 94
		// Ch 94
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 95
		// Ch 95
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 96
		// Ch 96
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 97
		// Ch 97
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 98
		// Ch 98
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
#if NUM_CHAN > 99
		// Ch 99
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,		// null channel
#endif
	};

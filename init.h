/*************************************************************************
 *********** COPYRIGHT (c) 2017 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: init.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for main.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    05-10-13 jmh:  creation date
 *    07-13-13 jmh:  removed typecast from timer defines & updates XTAL freq 
 *
 *******************************************************************/

#include "typedef.h"

#ifndef INIT_H
#define INIT_H
#endif

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define	NUM_CHAN	100		// define number of PLL channels for this build

// timer definitions.  Uses EXTXTAL #def to select between ext crystal and int osc
//  for normal mode.
// SYSCLK value in Hz
#define SYSCLKL 10000L
#ifdef EXTXTAL
#define SYSCLKF 20000000L
#else
#define SYSCLKF (12000000L) // / 8)
#endif
// timer2 register value
#define TMR2RLL_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) & 0xff)
#define TMR2RLH_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) >> 8)
#define	TMRSECHACK	2
#define TMRIPL		1
#define TMRRUN		0

#define MS_PER_TIC  1
// General timer constants
#define MS50        	(50/MS_PER_TIC)
#define MS100       	(100/MS_PER_TIC)
#define MS250       	(250/MS_PER_TIC)
#define MS400       	(400/MS_PER_TIC)
#define MS450       	(450/MS_PER_TIC)
#define MS500       	(500/MS_PER_TIC)
#define MS750       	(750/MS_PER_TIC)
#define MS1000      	(1000/MS_PER_TIC)
#define MS2000      	(2000/MS_PER_TIC)
#define MS5000      	(5000/MS_PER_TIC)
#define MS10000     	(10000/MS_PER_TIC)
#define MS20000     	(20000/MS_PER_TIC)
#define MINPERHOUR		60
#define SECPERMIN		60
#define MINPER6HOUR		(6 * MINPERHOUR)
#define MINPER12HOUR	(12 * MINPERHOUR)
#define MINPER18HOUR	(18 * MINPERHOUR)
#define MINPER24HOUR	(24 * MINPERHOUR)


//PBSW mode defines
#define PB_POR      0xFF
#define PB_NORM     0
#define UCPC        1

// CLI commands
#define	CLI_NULL	0x00
#define	CLI_LOGHDR	0x01
#define	CLI_SURV	0x02

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#ifndef IS_MAINC
extern U8 spi_tmr;
#endif

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void Init_Device(void);
void wait(U16 waitms);

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------

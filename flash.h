//-----------------------------------------------------------------------------
// flash.h
//-----------------------------------------------------------------------------
// Copyright (C) 2017 KE0FF
//
// Description:
// 	This file contains function prototypes for flash functions
//------------------------------------------------------------------------------


#include "typedef.h"

// extern defines
#ifndef FLASH_INCL

#endif

//------------------------------------------------------------------------------

// Function Prototypes

//------------------------------------------------------------------------------

void init_flash(void);

U8 erase_flash(U8 xdata * addr);
void wr_flash(char byte, U8 xdata * addr);

//------------------------------------------------------------------------------

// global defines

//------------------------------------------------------------------------------

#define FLRT 0x30	// for 75MHz < sysclk < =100MHz
//FLSCL:
#define FLWE 0x01
//PSCTL:
#define SFLE 0x04
#define PSEE 0x02
#define PSWE 0x01
//RSTSRC
#define PORSF 0x02

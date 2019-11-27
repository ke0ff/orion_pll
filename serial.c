/*************************************************************************
 *********** COPYRIGHT (c) 2017 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: serial.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the serial I/O module for the F360 MCU
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    05-12-13 jmh:  creation date
 *
 *******************************************************************/

#include "c8051F520.h"
#include "typedef.h"
#include "init.h"
//#include "stdio.h"
#include "serial.h"

//------------------------------------------------------------------------------
// local defines
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variable Declarations
//-----------------------------------------------------------------------------

#define RXD_ERR 0x01
//#define RXD_CR 0x02					// CR rcvd flag
#define RXD_BS 0x04					// BS rcvd flag
#define RXD_ESC 0x40				// ESC rcvd flag
#define RXD_CHAR 0x80				// CHAR rcvd flag (not used)
#define RXD_BUFF_END 64
idata S8	rxd_buff[RXD_BUFF_END];		// rx data buffer
U8	rxd_hptr;					// rx buf head ptr = next available buffer input
U8	rxd_tptr;					// rx buf tail ptr = next available buffer output
U8	rxd_stat;					// rx buff status
U8	rxd_crcnt;					// CR counter
bit	qTI0B;						// UART TI0 reflection (set by interrupt)
//------------------------------------------------------------------------------
// local fn declarations
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// init_serial() initializes serial port vars
//-----------------------------------------------------------------------------
//
void init_serial(void){
	qTI0B = 1;					// UART TI0 reflection (set by interrupt)
	rxd_hptr = 0;				// rx buf head ptr
	rxd_tptr = 0;				// rx buf tail ptr
	rxd_stat = 0;				// rx buff status
	rxd_crcnt = 0;				// init cr counter
}
//
//-----------------------------------------------------------------------------
// putch, UART0
//-----------------------------------------------------------------------------
//
// SFR Paged version of putch, no CRLF translation
//
char putch (char c)  {

	// output character
	while(!qTI0B){			// wait for tx buffer to clear
		continue;
	}
	qTI0B = 0;
	SBUF0 = c;
	return (c);
}

//
//-----------------------------------------------------------------------------
// getch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// SFR Paged, waits for a chr and returns
// Processor spends most of its idle time waiting in this fn
//
char getch00(void)
{
	char c = '\0';		// default to null return
	bit	EA_save;

	if(rxd_tptr != rxd_hptr){
		c = rxd_buff[rxd_tptr++];
		if(rxd_tptr == RXD_BUFF_END){
			rxd_tptr = 0;
		}
	}
	if((c == '\r') && rxd_crcnt){
		EA_save = EA;					// prohibit intrpts
		EA = 0;
		rxd_crcnt--;					// clear flag
		EA = EA_save;					// re-set intrpt enable
	}
	return c;
}

//
//-----------------------------------------------------------------------------
// cleanline() cleans buffer up to and including 1st CR.  
//	Pull CHRS from buffer and discard until first CR is encountered.  Then, exit.
//-----------------------------------------------------------------------------
//
void cleanline(void)
{
	char c;	// temp char

	if(rxd_tptr != rxd_hptr){							// skip if buffer empty
		do{
			c = rxd_buff[rxd_tptr++];					// pull chr and update pointer
			if(rxd_tptr == RXD_BUFF_END){
				rxd_tptr = 0;
			}
		}while((c != '\r') && (rxd_tptr != rxd_hptr));	// repeat until CR or buffer empty
	}
	return;
}

//
//-----------------------------------------------------------------------------
// gotch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
//
char gotch00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\r')){
		c = 1;						// set buffer has data
	}
	if(rxd_stat & RXD_BS){					// process backspace
		rxd_stat &= ~RXD_BS;				// clear flag
		putss("\b \b");					// echo clearing BS to terminal
	}
	return c;
}

//
//-----------------------------------------------------------------------------
// anych00 checks for any input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
//
char anych00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\0')){
		c = 1;						// set buffer has data
	}
	return c;
}

//
//-----------------------------------------------------------------------------
// gotcr checks for '\r' @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no cr rcvd
//
char gotcr(void)
{
	char c = 0;
	bit	EA_save;

	if(rxd_crcnt){
		EA_save = EA;					// prohibit intrpts
		EA = 0;
		rxd_crcnt--;					// clear flag
		EA = EA_save;					// re-set intrpt enable
		c = 1;							// set buffer has data
	}
/*	if(rxd_stat & RXD_BS){				// process backspace
		rxd_stat &= ~RXD_BS;			// clear flag
		putss("\b \b");					// echo clearing BS to terminal
	}*/
	return c;
}

//-----------------------------------------------------------------------------
// putss() does puts w/o newline
//-----------------------------------------------------------------------------

void putss (char* string)
{
	while(*string){
		if(*string == '\n') putch('\r');
		putch(*string++);
	}
	return;
}
//
//-----------------------------------------------------------------------------
// rxd_intr
//-----------------------------------------------------------------------------
//
// UART intr.  Captures RX data and places into circular buffer
//	For TX, if mode = buffered, the intr will process the TX buffer until empty.
//	To send data, fill the tx buffer, then kick-start the transmit process as follows:
//
//		c = txd_buff[txd_tptr++];		// get 1st chr of buffered data
//		if(txd_tptr >= TXD_BUFF_END){
//			txd_tptr = 0;				// roll-over headptr
//		}
//		SBUF0 = c;						// prime the pump (kick-start the TX output)
//		TI0B = 0;
//
//	If not in buffered mode, use reflection register to pass TXD empty status back to
//	putchar.
//
//
//-----------------------------------------------------------------------------
// uart1_intr
//-----------------------------------------------------------------------------
//
// UART1 rx intr.  Captures RX data and places into circular buffer
//

void rxd_intr(void) interrupt 4
{
/* buffer registers repeated here for reference ... !! DO NOT UNCOMMENT !!
S8   rxd_buff[10];					// rx data buffer
U8   rxd_hptr = 0;					// rx buf head ptr
U8   rxd_tptr = 0;					// rx buf tail ptr
U8   rxd_stat = 0;					// rx buff status*/

	char	c;

	if(TI0){
		qTI0B = 1;								// set TX reflection flag
		TI0 = 0;
	}
	if(RI0){
		c = SBUF0;
		if((c == '\n') || (c == ESC)){			// don't capture linefeeds or ESC
			if(c == ESC){
				rxd_hptr = 0;					// if ESC, re-init serial buffer
				rxd_tptr = 0;
				rxd_crcnt = 0;
				rxd_stat = RXD_ESC;
			}
		}else{
			rxd_buff[rxd_hptr] = c;
			if(rxd_buff[rxd_hptr] == '\r'){
				rxd_crcnt++;					// set CR rcvd flag
			}
			if(rxd_buff[rxd_hptr] == '\b'){
				if(rxd_hptr != rxd_tptr){		// only process BS if buffer is not empty
					rxd_stat |= RXD_BS;			// set BS rcvd flag
					if(rxd_hptr == 0){			// decrement headptr and rollunder if needed
						rxd_hptr = RXD_BUFF_END;
					}
					rxd_hptr--;
				}
			}else{
	//			rxd_stat |= RXD_CHAR;
				if(++rxd_hptr == RXD_BUFF_END){	// increment headptr and rollover if needed
					rxd_hptr = 0;
				}
			}
			if(rxd_hptr == rxd_tptr){			// check for overflow, flag error if true
				rxd_stat |= RXD_ERR;
			}
		}
		RI0 = 0;								// clear intr flag
	}
	return;
}

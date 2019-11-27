/****************************************************************************************
 ****************** COPYRIGHT (c) 2018 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: main.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the Orion ADF4351 PLL setup application
 *	License and other legal stuff:
 *			   This software, comprised of all files contained in the original distribution archive,
 *				are protected by US Copyright laws.  The files may be used and modified by the person
 *				receiving them under the following terms and conditions:
 *				1) This software, or any protion thereof, is protected under the GNU V3 license
 *
 *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Keil V5 note:
 *	channel array pointer initialization does not work the same for V5.  The V4 format produces a warning-free
 *	compile, but the target SW does not work.  The V5 format produces the following warning:
 *						main.c(351): warning C182: pointer to different objects
 *	but the target SW works correctly.
 *
 *	pll_ch = pll_ch_array[0];					// {{{Keil V4 format}}} set array to point to fixed location
 *	pll_ch = pll_ch_array;						// {{{Keil V5 format}}} set array to point to fixed location
 *
 *!!!!!!!!!!!!!!!!!!!!!!
 *	MEMORY MAP NOTE:
 *		Because of how the memory is allocated and initialized by the cource code, the Program size
 *		as reported by the Keil compiler (code) is limited to 7518 bytes.  If this number is equalled or exceeded,
 *		the code space will have encroached on the channel sectors.  In this case, either the number of channels
 *		must be decreased, or the code must be optimized to reduce its size.  There will likely not be a
 *		compiler warning if this happens.  The channel FLASH space is included in the compiler report.
 *		Thus, the available FLASH space for program code is calculated by the eqn:
 *			  Remaining code FLASH = 0x1dfe - Program_size_code - 160
 *				^						^			^				^
 *				|						|			|				|__ space wasted in channel sectors
 *				|						|			|___ size reported by compiler
 *				|						|___ top of useable FLASH
 *				|___ space available for new code
 *
 *		The largest value reported from the compiler (given the current waste value of 160 bytes) would be
 *		Program_size_code(max) = (0x1dfe - 160) = 7518
 *		If the channel space is modified, the wasted space value may change and this must be taken into account
 *		if this action is taken.
 *		Since FLASH erase operates on sectors (512 bytes for the F530), The channels must be placed within sectors
 *		that contain no other data.  The current schema aligns the channels with an offset of 128 to provide
 *		scratchpad space and align the channels so that CH16 starts on the next sector.  The number of sectors
 *		required is calculated by dividing the channel space by 512 and rounding up.  Each channel occupies 24
 *		bytes (6, 4-byte registers), so 100 channels occupies 2400 bytes.
 *!!!!!!!!!!!!!!!!!!!!!!
 *
 *  Project scope revision history:
 *    08-11-18 jmh:  Rev 1.6, HWrevC (released)
 *						Changed delay_halfbit to use HW timer0 instead of cheesy for-loop
 *						Converged delay_halfbit into a single Fn for BB/HWSPI.  Now, base timer value for delay half-bit
 *							changes based on compile directive.
 *						Added delay_halfbit call to end of send_spi32 (HWSPI version)
 *						Updated rev messages.
 *    07-11-18 jmh:  Rev 1.6, HWrevC (proto)
 *						Created "max valid channel" mode.  If a non-BCD code is applied to the FSEL[3:0] inputs,
 *							The system will look for the highest valid channel ("valid" means that R5 != 0xFFFFFFFF)
 *							and select this channel if found (or CH#0 if all are erased).  It does this by starting
 *							at the highest channel (NUM_CHAN), looking at R5 in the channel memory, and stopping if
 *							it is not erased.  If it is erased, the next lowest channel is checked.  This repeats until
 *							CH#00 is reached.
 *							This allows the default active channel to be changed without erasing any other channels
 *							(this scheme provides 99 chances to update the "default" channel).
 *    07-20-17 jmh:  Rev 1.5, HWrevC (released)
 *						Created PGM TEMP CH to CH##.  If temp channel valid, writes the temp ch to the
 *							FLASH ch# specified.  Allows temp ch to be verified before committing to FLASH.
 *    07-13-17 jmh:  Rev 1.4, HWrevC (released)
 *						Tested all user fns, PTT, and channel select
 *						Fixed FLASH	erase bug
 *						changed all null channels to erased channels
 *						added "E16" to erase ch 16-99)
 *						changed "E" to "EA" to erase all
 *						changed default channel map to feature standard uw LO frequencies in lower channels,
 *							a string of blank channels in the low bank, and a string of simplex frequencies
 *							for RX testing in the upper channels.
 *						beefed up support for compile-time changes to NUM_CHAN
 *						Added haradware SPI and #if construct to select bit-bang or hdwr
 *    06-24-17 jmh:  Rev 1.3, HWrevC (released)
 *						Fixed MISO initialization bug (PLL lock would not work)
 *    04-29-17 jmh:  Rev 1.1, HWrevC (release candidate)
 *						Modified to support revC hardware:
 *							inverted LE and LOCK
 *							moved CH01 to 144.1 MHz
 *						CRC check modified to check only the # channels specified in NUM_CHAN
 *    08-12-16 jmh:  Rev 1.0 (release candidate)
 *						PTT and serial functions added to basic ADF4351 upload framework
 *						Updated comments to capture command line features
 *						Updated C and BL configs to get mem allocation to work within the 256 byte limit.
 *							mem model: small
 *							code model: large
 *							specify idata for large RAM buffers and arrays
 *						Finalized FLASH workarounds to address loss of upper 1.5K of FLASH
 *    04-09-16 jmh:  Rev 0000:
 *                   Initial project code copied from GPSDO and cleaned
 *
 ***************************************************************************************/

#define	REVC_HW 	1		// 1 = build for rev C hardware, else set to 0
#undef	BB_SPI				// if defined, use bit-bang SPI, else use hdwr SPI

//--------------------------------------------------------------------------------------
// main.c
//      Uses a C8051F531-C-IT processor to program the ADF4351 PLL chip registers.
//			20 pin TSSOP package.
// *****
//		Some housekeeping:
//		Linker directive "?CO?CHANNELS(0x1280)" must be placed in the CODE SEGMENT command field
//			of the Keil compiler (configuring for other compiler suites are left as an excercise).
//			This places the channel data at the fixed FLASH location of 0x1280 which allows for
//			the containing FLASH sectors to be erased and re-written.  Sectors are 512 bytes, so
//			there is also 128 bytes of uncommitted FLASH between 0x1200 and 0x127F that are available
//			for cal or other NV memory features.
//
//			When using Keil, "pll_ch = pll_ch_array;" must be placed in main().  Configuring
//			this arrangement for other compiler suites is left as an excercise.
//			This allows the SW to access the array at the fixed CODE location of 0x1280.
//
//			If a clean data build is needed (i.e., no channel data in the object file), "channels.c"
//			can be removed from the project ("channels.h" header file must still be available to main.c)
//			The "pll_ch_array" statement in main() illustrated above must be replaced by "pll_ch = (U32 *) CHAN_ADDR;"
//			This will produce a SW obect that should load over into a part with an existing channel array
//			without disturbing the array.
//
//			SW Updates should be distributed as a SW object and a default channel object to allow
//			users the option to perform a clean load (load both objects) or just update the SW
//			(load the SW object only).
// *****
//      Up to 100 PLL channels may be stored and recalled using a 2-digit BCD input (2 nybbles in 8-bits),
//			GND true.  Channels are stored in top page of FLASH, starting at 0x1280.  Unprogrammed,
//			or blank, chennels should contain 0xFFFFFFFF for all register entries.  If selected, these
//			channels will revert to CH00 (if CH00 is blank, a default register set that disables the VCO
//			will be sent to the PLL chip).
//
//			The hardware is configured to use a 16-pin dual-row header
//			  to input the channel select and PTT lines, C2D lines, and power.
//			  PTT is used to select between CH00 (PTT = hi) or the channel at the FSEL inputs
//			  (PTT = low).  If PTT funtion is not desired, it may be tied low.  This allows CH00
//			  to be used as any other channel.  If PTT is to be used to key the output, CH00 must
//			  contain a register set that disables the VCO (this is in the default compile load).
//
//		UART access supports external programming of channels.  Format is TBD, but
//			ASCII XFR of S-records or Intel HEX records is the initial target.
//			The programming would load the entire channel memory space.  A PC app
//			to manage the register input from the user and output to HEX format file
//			would be helpful.
//
//      The following resources of the F531 are used:
//      24.000 MHz internal osc
//
//      UART: 9600 baud, Simple I/O protocol
//
//      Timer0: n/u
//      Timer1: UART baud rate (9600 baud)
//      Timer2: Application timer (1ms/tic)
//
//      ADC: n/u
//
//      PCA: n/u
//
//      SYSTEM NOTES:
//		16 pin header for I/O (REV A/C):						Header for Rev B:
//		pin  1: +Vin (5-12V)	pin  2: GND						pin  1: +Vin (5-12V)	pin  2: GND
//		pin  3: RS-232 RX		pin  4: GND						pin  3: /PTT			pin  4: GND
//		pin  5: RS-232 TX		pin  6: GND						pin  5: RS-232 RX		pin  6: GND
//		pin  7: /PTT			pin  8: FSEL 0					pin  7: RS-232 TX		pin  8: FSEL 0
//		pin  9: FSEL1			pin 10: FSEL 2					pin  9: FSEL1			pin 10: FSEL 2
//		pin 11: FSEL3			pin 12: FSEL 4					pin 11: FSEL3			pin 12: FSEL 4
//		pin 13: FSEL5			pin 14: FSEL 6					pin 13: FSEL5			pin 14: FSEL 6
//		pin 15: FSEL7			pin 16: GND						pin 15: FSEL7			pin 16: GND
//
//		Notes: !! Rev A hardware is 5V only !!
//			   a 10 pin header may be used for a 4 frequency system (2-T/R channels, with 1of2 select),
//				or ground FSEL[7:4] for an 8 frequency system (4-T/R channels with 2-bit binary select)
//
//		Channels consist of 100, 32 bit register values that are sent to the PLL when the channel is selected.
//		Channel data is stored in top FLASH sectors.  System only supports upload of the entire channel sector mem.
//
//			Note: this limits available code space to 4K (4096 bytes)
//
//		Serial protocol:
//		EA/E16
//			ErAse all channels, or Erase ch16+
//			prompts "Erase all, press Y to accept" and waits 5 sec for input.  Any character
//			other than upper-case "Y", or a delay of more than 5 sec will cause this command to abort.
//
//		Mxxaaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
//			Programs channel "xx" (xx is BCD ASCII '00' thru '99') with R0 (A), thru R5 (F) values
//			Data is represented as ASCII hex
//			spaces may be present between channel data characters, but the buffer limit is 62 characters,
//			including the command and channel# characters, and will be ignored.
//			If any invalid data is received (non-numeric, non-hex), the command is aborted (with error message)
//			Serial buffer is limited to 64 characters (including <CR>).
//
//		txxaaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
//			Programs temp channel (xx is BCD ASCII '00' thru '99', but is otherwise ignored) with
//			R0 (A), thru R5 (F) values.  It has the same requirements as the "M" command.
//			The temp channel allows a register set to be input to the Orion without setting it
//			in FLASH.  Once set, the temp command overrides the BCD inputs.  PTT behaves normally
//			and will not change the temp channel configuration.
//			Selecting a new BCD channel, or programming a channel with the "M" command will cancel
//			the temp channel.  The temp channel command must then be be re-entered if needed.
//
//		rxx
//			Read channel "xx" (xx is BCD ASCII '00' thru '99')
//		rr
//			Read temp channel
//		r-
//			Read all channels
//			Channel data is output in the "M" entry format described above with spaces inserted
//			between register fields.
//
//		i
//			re-send last resister set to the ADF4351.  Re-sends current register selection (BCD or temp) based on
//			PTT status (if PTT = high, CH00 is resent).
//
//		e
//			echo command line.  This is a debug command that will echo the characters on the command line.
//
//		All commands are terminated with <CR> ('\r').
//		Serial port does not echo characters.
//
//		Serial port sends power on message when reset.
//		Serial port outputs current channel selection, CH#, when setting is changed or PTT is cycled.
//
//--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// compile defines
//#define F360DK ON

//#include <stdio.h>
//#include <string.h>
//#include <math.h>
#define IS_MAINC
#include "init.h"
#include "typedef.h"
#include "c8051F520.h"
#include "serial.h"
//#include "version.h"
#include "channels.h"
#include "flash.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//  see init.h for #defines

#define	PBMAX	100				// max channel #s (2-digit BCD input)
#define	PBMVAL	254				// indicates max-valid channel mode is active
#define	MAX_REG	24				// max bytes in an ADF4351 reg set

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

U8  spi_tmr = 0;

//-----------------------------------------------------------------------------
// Main Variables
//-----------------------------------------------------------------------------

// port assignments

sbit PB0		= P1^0;				// (i) button input 0
sbit PB1		= P1^1;				// (i) button input 1
sbit PB2		= P1^2;				// (i) button input 2
sbit SCK        = P0^0;				// (o) SPI SCLK
sbit MISO       = P0^1;				// (i) SPI MISO/LDET
sbit MOSI       = P0^2;				// (o) SPI MOSI
sbit nPTT		= P0^3;				// (i) /PTT input
sbit nPLL_LE	= P0^7;				// (o) SPI LE

#if (REVC_HW == 1)
#define	LE_ON	1
#define	LE_OFF	0
#else
#define	LE_ON	0
#define	LE_OFF	1
#endif

#if (REVC_HW == 1)
#define	PLL_LOCK	0
#else
#define	PLL_LOCK	1
#endif

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
//U16 temptimer; // = 0;
U16 waittimer; // = 0;              // wait() function timer
U8	dbounce_tmr;
U8	iplTMR; // = TMRIPL;            // timer IPL init flag
U32* pll_ch;						// pointer to base of channel array (initialized in main())

//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

void send_spi32(U32 plldata);
void delay_halfbit(void);
U16 calcrc(U8 c, U16 oldcrc);
void wait(U16 waitms);
//void pb_state(U8 imode);
U32 *get_chan(U8 chanum);
U8 conv_to_chnum(U8 portbits);
void put_hex(U8 dhex);
void put_dec(U8 dhex);
U8 convnyb(U8 c);
U8 getbyte(U8* dataptr);
U8 whitespc(char c);

//******************************************************************************
// main()
//  The main function inits I/O and process inputs.
//	sends PLL data to ADF4351 when channel select inputs change
//
//******************************************************************************
void main(void) //using 0
{
	char c;				// temp term chr
	bit	ipl;			// initial loop flag
	U8	i;				// loop counter
	U8	j;				// loop counter
	U8	k;				// loop counter
	U8	maxtemp;		// maxval temp port reg
	bit	flag;			// temp flag
	bit	goteol;			// temp flag
	U8	PBreg;			// PB memory
	U8	PBtemp;			// PB temp holding
	U8	PTTtemp;		// PTT temp holding reg
	U8	PTTreg;			// PTT memory
	U8	CHtemp;			// channel temp
	bit	temp_active;	// temp reg active flag
	bit loaderr;		// channel pgm error flag
	U8	pgm_chnum;		// prog chan temp
	U8	tempbyte;		// prog byte temp
	U8	tempbyte2;		// prog byte temp
	U8	temp_chan[24];	// temp channel register set (bytes)
	bit	z_temp;			// "z" cmd flag
	U16	temp_crc;		// crc temp
	U16 ii;				// crc temp
	U32	temp32;			// 32 bit temp
	U32* tptr;			// reg pointer
	U8 xdata * fptr;	// flash pointer
	U8 code * rptr;		// flash pointer
	
	// start of main
	PCA0MD = 0x00;							// disable watchdog
	// init MCU system
	Init_Device();							// init MCU
#ifndef	BB_SPI
    XBR0      = 0x03;						// enable hdwr SPI on xbar
    SPI0CN    = 0x01;						// enable hdwr SPI
#endif
	SCK = 0;								// init SPI pins
	MISO = 1;
	nPLL_LE = LE_OFF;
	init_flash();							// init FLASH
	P1 = 0xFF;								// enable port for input
	PBreg = P1;								// init PB memory
	PTTreg = ~nPTT;							// force PTT edge det for POR
	pll_ch = pll_ch_array;					// set array to point to fixed location
	init_serial();							// init serial module
	// init module vars
	iplTMR = TMRIPL;                        // timer IPL init flag
	EA = 1;
	wait(50);                               // 50 ms delay
	
#if (REVC_HW == 1)
	putss("\nADF4351 PLL Driver Ver 1.6, de ke0ff\n");	// send sw version msg to serial port
#else
	putss("\nADF4351 PLL Driver Ver A1.6, de ke0ff\n");	// send sw version msg to serial port
#endif
#if NUM_CHAN > 100
	putss("Err");							// compile-time err
#endif
#if NUM_CHAN > 99
	putss("100");							// include # channels supported
#else
	put_dec(NUM_CHAN);						// include # channels supported
#endif

	putss(" CH, gnd-true BCD, PTT hi = CH00\n"); // send help screen
	putss("Serial cmd enabled\n");			// send help screen
	if(RSTSRC & 0x40){
		putss("FLERR\n");
		RSTSRC = 0x42;
	}
	temp_active = 0;						// de-activate temp reg
	loaderr = 0;							// init chan error status
//	RSTSRC = PORSF;
	ipl = 1;								// set initial loop
	
	// main loop
	// PB0 is a toggle switch that selects one of two channels.  Flip the switch to send the opposite channel
	while(1){
		PBtemp = (~P1);								// convert port to POS logic
		PTTtemp = nPTT;
		if((PBtemp != PBreg) || (PTTtemp != PTTreg)){ // look for a change in port state
			// this only runs if there is a change in state
			maxtemp = 0xff;							// invalid maxtem
//			if((PTTtemp != PTTreg) || ((PTTtemp == PTTreg) && (PTTreg == 0))){ // if(pttedge OR (!pttedge && ptt==gnd))...
			if((PTTtemp != PTTreg) || (PTTreg == 0)){ // if(pttedge OR (!pttedge && ptt==gnd))...
				if((PBtemp & 0x0f) > 0x09){			// look for "max valid search" semaphore (any non-BCD in 1's digit)
					maxtemp = PBtemp;				// save port reg so we can preserve the change detect logic..
					CHtemp = (NUM_CHAN + 1);		// ..because we are going to use PBreg to squeeze in the max chan selection
					do{
						CHtemp -= 1;
						tptr = get_chan(CHtemp);	// calc tptr to R5 of correct channel array
					}while((*tptr == 0xFFFFFFFF) && (CHtemp != 0));
					i = CHtemp / 10;				// set PBtemp = BCD code for max valid channel#
					PBtemp = CHtemp - (i * 10);		// LSnyb = 1's (remainder)
					PBtemp |= i << 4;				// MSnyb = 10's
				}
			}
			CHtemp = 0;								// set CH0 as default
			if(PTTtemp != PTTreg){
				PTTreg = PTTtemp;					// update edge detect
				PBreg = PBtemp;						// copy the new port state to memory just in case it chaged at same time as PTT
				if(PTTreg == 0){					// if PTT active (grounded):
					CHtemp = conv_to_chnum(PBreg);	// convert port state to channel#
				}
			}else{
				PBreg = PBtemp;						// copy the new port state to memory
				temp_active = 0;					// abandon temp regs
				if(PTTreg == 0){					// only update PLL if PTT = gnd
					CHtemp = conv_to_chnum(PBreg);	// convert port state to channel#
				}
			}
			if(CHtemp <= PBMAX){					// if valid channel#:
				if((!temp_active) || (CHtemp == 0)){
					putss("CH ");					// send status msg (at 9600 baud, this gives us > 10ms of debounce)
					put_dec(CHtemp);				// print ch#
					tptr = get_chan(CHtemp);		// calc tptr to R5 of correct channel array
					if(*tptr == 0xffffffff) tptr = get_chan(0);	// default to ch#00 if R5 is 0xffffffff (i.e., ch is empty)
					for(i=6;i!=0;i--){
						send_spi32(*tptr--);		// transfer channel data to PLL
					}
				}else{
					putss("tmp");					// do temp channel
					for(i=6; i!=0; i--){
						k = (i-1) * 4;
						temp32 = (U32)temp_chan[k++] << 24;
						temp32 |= (U32)temp_chan[k++] << 16;
						temp32 |= (U32)temp_chan[k++] << 8;
						temp32 |= (U32)temp_chan[k];
						send_spi32(temp32);			// transfer channel data to PLL
					}
				}
			}
			if(maxtemp != 0xff){
				PBreg = maxtemp;					// update port reg to hold setting
			}
			wait(50);								// debounce
			putss("\npll>");						// post prompt
		}
		// process serial input
		if(gotcr()){									// wait for a cr ('\r') to be entered
			z_temp = 0;									// pre-clear "z" flag
			do{
				c = getch00();							// skip over leading control chrs
			}while((c <= ESC) && (c != '\0'));
			putch(c);
			switch(c){
				default:								// invalid command chr
					do{
						c = getch00();					// skip to EOL or end of input
					}while((c != '\r') && (c != '\0'));
				case '\r':								// empty line
					break;
				
				case 'i':
					putss("\nresend");					// post prompt
					PTTreg = ~PTTreg;					// force re-send (simulate a change in the BCD settings)
					break;
				
				case 'E':
					// "EA" erase all 5 sectors of channel FLASH where the channel data lives	
					// "E16" erases the last four sectors (CH 16-99).
					//		tempbyte is TRUE if "E16"
					tempbyte = FALSE;					// preclear qualifiers
					tempbyte2 = FALSE;
					c = getch00();
					if((c == '1')){
						c = getch00();
						if((c == '6')){
							tempbyte = TRUE;			// erase 16+ enabled
						}
					}else{
						if(c == 'A'){
							tempbyte2 = TRUE;			// erase all enabled
						}
					}
					if(tempbyte || tempbyte2){				// if valid, execute
						while(getch00());					// clean out serial buffer
						if(tempbyte){
							putss("\nErase CH16+");			// Are you sure? prompt (ch16-end)
						}else{
							putss("\nErase All CH");		// Are you sure? prompt (all)
						}
						putss(", Press \"Y\" to cont...");	// Are you sure? prompt
						waittimer = 5000;					// set 5 sec timer
						while((!anych00()) && (waittimer != 0)); // wait for user input
						if(getch00() == 'Y'){				// if timeout, getch00 will return '\0' which will abort
							fptr = (U8 xdata *)SECT00_ADDR;	// set pointer to 1st sector
							// !!!!! These params are dependent on the size of the allocated channel array !!!!!
							for(i=0; i<5; i++){				// 100 ch = 5 sectors
								if((tempbyte && (i > 0)) || (!tempbyte)){
									erase_flash(fptr);		// erase sector (skip if first sector and tempbyte = true)
									putch('.');				// display progress
								}
								fptr += SECTOR_SIZE;		// set next sector
							}
							// !!!!!
							putss("Erased!\n");				// announce completion
						}else{
							putss("Aborted.\n");			// abort msg
						}
					}
					break;
				
				case 'e':
					// echo terminal buffer
					do{
						c = getch00();				 		// get byte
						putch(c);							// echo
					}while(c != '\r');						// until cr
					putch('\n');
					break;
				
				case 'Q':
					// error querry/clear
					if(loaderr){
						putss("\nLoad errs\n");
					}else{
						putss("\nNO errs\n");
					}
					if(gotch00()){
						if(getch00() == 'C'){
							putss("Err status cleared\n");
							loaderr = 0;					// clear error status
						}
					}
					putch('\n');
					break;

				case 'z':
					z_temp = 1;										// set "z" cmd flag
				case 'c':
					// calc CRC16 on channels
					temp_crc = 0;
					rptr = (U8 code *) CHAN_ADDR;
					for(ii=0; ii<(24 * NUM_CHAN); ii++){
						temp_crc = calcrc(*rptr++,temp_crc);
					}
					if(z_temp){										// do CRC compare if true
						putss(" CMP CRC16...");
						j = 1;										// preset PASS
						if(getbyte(&tempbyte)) j = 0;				// 1st CRC byte -- compare data fail
						if((U8)(temp_crc >> 8) != tempbyte) j = 0;	// crc fail
						if(getbyte(&tempbyte)) j = 0;				// 2nd CRC byte -- compare data fail
						if((U8)(temp_crc & 0xff) != tempbyte) j = 0;	// crc fail
						wait(1000);									// wait 1 sec
						if(j){
							putss("\nPASS\n");
						}else{
							loaderr = 1;							// set global fail
							putss("\nFAIL\n");
						}
					}else{
						putss("\nCRC16 = 0x");						// display calculated CRC
						put_hex((U8)(temp_crc >> 8));
						put_hex((U8)(temp_crc & 0xff));
						putss("\n");
					}
					break;
				
				case 't':
				case 'M':
					// program reg
					// syntax: Mxxaaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff
					// First, validate buffered data (xfr to temp array)
					k = c;								// save smd chr
					flag = TRUE;						// default to data good
					temp_active = 0;					// default to temp = inactive
					c = getch00();
					if((c < '0') || (c > '9')){
						flag = FALSE;
					}
					i = (c & 0x0f) << 4;				// ms nyb
					c = getch00();
					if((c < '0') || (c > '9')){
						flag = FALSE;
					}
					i |= (c & 0x0f);					// ls nyb
					pgm_chnum = conv_to_chnum(i);		// convert BCD to hex
					if(pgm_chnum >= NUM_CHAN){			// max error
						flag = FALSE;
					}
					goteol = 0;
					i = 0;								// init reg counter
					while(flag && !goteol){
						goteol = getbyte(&tempbyte); 	// get byte
						if(!goteol){
							temp_chan[i++] = tempbyte;
						}
						if(i == MAX_REG){
							goteol = 1;					// force end if 24 bytes
						}else{
							if(goteol) flag = FALSE;	// end of data too soon, error
						}
					}
					// Program data to FLASH
					if(flag && (k == 'M')){
						fptr = (U8 xdata *) CHAN_ADDR;	// set pointer to 1st ch
						fptr += 24 * pgm_chnum;			// jump to ch#
						for(i=0; i<24; i++){
							wr_flash(temp_chan[i], fptr++);
						}
						putss("CH ");
						put_dec(pgm_chnum);				// print ch#
						putss(" pgmd!\n");				// announce completion
					}else{
						if(flag){
							temp_active = 1;			// temp channel active
							PTTreg = ~PTTreg;
							putss("Temp reg pgmd\n");	// announce temp reg programmed
						}else{
							putss("ERROR!\n");			// announce err
							loaderr = 1;				// set error
						}
					}
					// Complete
					break;

				case 'P':
					if(temp_active){
						flag = TRUE;					// default to data good if temp active
					}else{
						flag = FALSE;					// else the whole "P" thing is invalid
					}
					c = getch00();						// get & test msd
					if((c < '0') || (c > '9')){
						flag = FALSE;
					}
					i = (c & 0x0f) << 4;				// convert ms nyb
					c = getch00();						// get & test lsd
					if((c < '0') || (c > '9')){
						flag = FALSE;
					}
					i |= (c & 0x0f);					// convert ls nyb
					pgm_chnum = conv_to_chnum(i);		// convert BCD to hex
					if(pgm_chnum >= NUM_CHAN){			// check if CH valid
						flag = FALSE;
					}
					// Program temp data to FLASH
					if(flag){							// only write to FLASH if valid CH and valid temp
						fptr = (U8 xdata *) CHAN_ADDR;	// set pointer to 1st ch
						fptr += 24 * pgm_chnum;			// calc index to ch#
						for(i=0; i<24; i++){			// copy data to FLASH
							wr_flash(temp_chan[i], fptr++);
						}
						putss("CH ");
						put_dec(pgm_chnum);				// print ch#
						putss(" pgmd!\n");				// announce completion
					}else{
						putss("ERROR!\n");				// announce err
					}
					break;

				case 'r':
					// read reg
					// syntax: rxx
					flag = TRUE;
					goteol = TRUE;
					putss("\n");
					c = getch00();
					if(c == '-'){
						i = NUM_CHAN;							// send all chnnels
						j = 0;
						pgm_chnum = conv_to_chnum(j);			// convert BCD to hex
					}else{
						if(c == 'r'){
							i = 1;								// send 1 chnnels
							j = 0;
							goteol = FALSE;						// select temp channel
						}else{
							if((c < '0') || (c > '9')){
								flag = FALSE;
							}
							i = (c & 0x0f) << 4;				// ms nyb
							c = getch00();
							if((c < '0') || (c > '9')){
								flag = FALSE;
							}
							i |= (c & 0x0f);					// ls nyb
							pgm_chnum = conv_to_chnum(i);		// convert BCD to hex
							if(pgm_chnum >= NUM_CHAN){			// check for max error
								flag = FALSE;
							}
							i = 1;								// just send 1 chan
							j = pgm_chnum;
						}
					}
					// read data from FLASH
					if(flag){
						if(goteol){
							rptr = (U8 code *) CHAN_ADDR;		// set pointer to 1st ch
							rptr += 24 * pgm_chnum;				// jump to ch#
						}
						do{
							if(goteol){
								putch('M');						// pre-amble
								put_dec(j++);					// print ch#
								putch(' ');
							}else{
								putss("t00 ");					// temp chan preamble
							}
							tempbyte = 0;						// use tempbyte to format register fields
							for(k=0; k<24; k++){
								if(goteol){
									put_hex(*rptr++);			// display FLASH data
								}else{
									put_hex(temp_chan[k]);		// display temp reg data
								}
								if(++tempbyte == 4){
									putch(' ');					// insert some formatting between 32bit words
									tempbyte = 0;
								}
							}
							putss("\n");
						}while(--i != 0);
					}else{
						putss("CHerr\n");
					}
					break;
			
				case 'l':
				case 'L':
					// read PLL lock bit
					// syntax: l, return "1" or "0"
					if(MISO == PLL_LOCK){
						putss("1\n");
					}else{
						putss("0\n");
					}
					break;

				case '?':
					// Help screen
					putss("\nOrion Help V1.6\n");
					putss("Mnna..f: PGM CH nn\t\tt00a..f: temp CH\n");
					putss("Pnn: PGM temp to CH nn\n");
					putss("EA: erase all CH\t\tE16: erase CH16-99\n");
					putss("c: disp CRC16 (0x1021 poly)\tz hhhh: cmp CRC16\n");
					putss("rnn: read CH nn\t\t\tr-: read all CH\n");
					putss("rr: read temp CH\t\ti: re-send CH\n");
					putss("Q: querry errs\t\t\tQC: Clr errs\n");
					putss("L: read PLL lock stat\t\te: echo cmdln\n");
					putss("\nMaxValid ch if bcdin = 0xXF\n");			// send help screen
					break;
			}
			cleanline();										// clean up rest of current line
			putss("\npll>");									// post prompt
		}
	}
}  // end main()

// *********************************************
//  *************** SUBROUTINES ***************
// *********************************************

//-----------------------------------------------------------------------------
// send_spi32
//-----------------------------------------------------------------------------
//
// sends 32 bit word to ADS4351 via port4 bit-bang SPI
//
void send_spi32(U32 plldata){
#ifdef BB_SPI
	U32	mask;

	nPLL_LE = LE_ON;								// latch enab = low to clock in data
	for(mask = 0x80000000; mask != 0; mask >>= 1){	// start shifting 32 bits starting at MSb
		if(mask & plldata) MOSI = 1;				// set MOSI
		else MOSI = 0;
		delay_halfbit();							// delay half clock
		SCK = 1;									// clock = high
		delay_halfbit();							// delay remaining half
		SCK = 0;									// clock low
	}
	delay_halfbit();								// delay for LE
	nPLL_LE = LE_OFF;								// latch enab = high to latch data
	delay_halfbit();								// pad intra-word xfers by a half bit
	return;	
	
#else
	U8	i;	// loop temps
	U8	d;
	union Data32 {	// temp union to parse out 32b word to 8b pieces
	   U32 l;
	   U8 b[4];
	} pllu;  

	pllu.l = plldata;
	nPLL_LE = LE_ON;								// latch enab = low to clock in data
	delay_halfbit();								// pad intra-word xfers by a half bit
	for(i=0; i < 4; i++){							// shifting 32 bits 8 bits at a time
		d = (U8)(pllu.b[i]);
		while(SPI0CFG & 0x80);						// wait for buffer to clear
		SPI0DAT = d;
	}
	while(SPI0CFG & 0x80);							// wait for buffer to clear
	delay_halfbit();								// pad intra-word xfers by a half bit
	nPLL_LE = LE_OFF;								// latch enab = low to clock in data
	delay_halfbit();								// delay for RC pullup on revC CS line
	return;
#endif
}

//-----------------------------------------------------------------------------
// delay_halfbit
//-----------------------------------------------------------------------------
//
// sets delay for spi SCK and for CS setup/hold
//
	// BitBang version uses for-loop to establish ~~200 us delay
/*void delay_halfbit(void){
	U16	i;
	
	for(i = 0; i < 500;){							// cheezy for-next-loop to set delay for bit-bang-spi
													// 500 loops is approx 200us
		i += 1;
	}
	return;	
}*/

#ifdef BB_SPI
	// BitBangSPI version uses HW timer0 to establish the bit-delay (200us, nominal)
	// T0 has about 0.5us of delay per timer tic when configured for clock source = SYSCLK/12
	// define 200us timer delay @24.5MHz/12 timer clock = (65536 - (400*0.5us))
#define	T0_VALUE	65136
#else
	// HWSPI version uses HW timer0 to establish quick delay (8us, nominal)
	// T0 has about 0.5us of delay per timer tic when configured for clock source = SYSCLK/12
	// define 8us timer delay @24.5MHz/12 timer clock = (65536 - (16*0.5us))
#define	T0_VALUE	0xFFF0
#endif

void delay_halfbit(void){							// for HWSPI, this fnd is just used to set reg-reg xfr delay

	TH0 = (T0_VALUE >> 8);							// prep timer registers for delay
	TL0 = (T0_VALUE & 0xFF);
	TF0 = 0;
	TR0 = 1;										// start timer
	while(TF0 == 0);								// loop
	TR0 = 0;										// stop timer
	return;	
}

//-----------------------------------------------------------------------------
// calcrc() calculates incremental crcsum using defined poly
//	(xmodem poly = 0x1021)
//-----------------------------------------------------------------------------
U16 calcrc(U8 c, U16 oldcrc){
#define	POLY 0x1021	// xmodem polynomial

	U16 crc;
	U8	i;
	
	crc = oldcrc ^ ((U16)c << 8);
	for (i = 0; i < 8; ++i){
		if (crc & 0x8000) crc = (crc << 1) ^ POLY; //0x1021;
		else crc = crc << 1;
	 }
	 return crc;
}

//-----------------------------------------------------------------------------
// wait() uses ms timer to establish a defined delay
//-----------------------------------------------------------------------------

void wait(U16 waitms)
{

    waittimer = waitms/MS_PER_TIC;
	if(waittimer == 0) waittimer = 1;
    while(waittimer != 0);
}

//-----------------------------------------------------------------------------
// put_hex
//-----------------------------------------------------------------------------
//
// sends 8b hex to serial port as ASCII
//
void put_hex(U8 dhex){
	char	c;
	
	c = (dhex >> 4) + '0';
	if(c > '9') c += 'A' - '9' - 1;
	putch(c);
	c = (dhex & 0x0f) + '0';
	if(c > '9') c += 'A' - '9' - 1;
	putch(c);
	return;
}

//-----------------------------------------------------------------------------
// put_dec
//-----------------------------------------------------------------------------
//
// sends 8b hex to serial port as decimal ASCII
//
void put_dec(U8 dhex){
	U8		d;		// bcd temp
	U8 		i;		// temps
	U8 		j;

	if(dhex > 99){
		putss(">>");
	}else{
		j = dhex / 10;
		i = dhex - (j * 10);
		d = (j << 4) | i;
		put_hex(d);
	}
	return;
}

//--------------------------------------------------------------------------------------
// getbyte() returns 1 if no EOL is encountered: processes ASCII byte into pointer location.
//	skips spaces.  Other chars are data error.
//	returns 1 if EOL or data error
//--------------------------------------------------------------------------------------
U8 getbyte(U8* dataptr){
	U8	c;		// temps
	U8	cc;
	U8	temp;
	U8	rtn = 0;	// default to !goteol rtn

	do{									// 1st nyb, skip spaces & trap EOL
		c = (U8)getch00();
	}while(whitespc(c));
	do{									// 2nd nyb, skip spaces & trap EOL
		cc = (U8)getch00();
	}while(whitespc(cc));
	if((c <= ESC) || (cc <= ESC)){
		rtn = 1;						// any cntl chr is interpreted as EOL
	}else{
		temp = convnyb(c);
		if(temp > 0x0f) rtn = 2;		// error data
		c = convnyb(cc);
		if(c > 0x0f) rtn = 2;			// error data
	}
	if(!rtn){
		*dataptr = (temp << 4) | c;
	}
	return rtn;
}

//--------------------------------------------------------------------------------------
// whitespc() returns 1 if chr = space, comma, or tab, else returns 0
//--------------------------------------------------------------------------------------
U8 whitespc(char c){
	U8 rtn = 0;	// temp rtn

	switch(c){
		case ' ':
		case ',':
		case '\t':
			rtn = 1;
			break;
	}
	return rtn;
}

//--------------------------------------------------------------------------------------
// convnyb() converts ASCII to bin nybble.  returns 0xff if non-hex ascii
//--------------------------------------------------------------------------------------
U8 convnyb(U8 c){
	U8	rtn = 0xff;

	if((c >= 'a') && (c <= 'z')) c -= 'a' - 'A';	// upcase
	if((c >= '0') && (c <= 'F')){			// 1st validation step
		rtn = (c - '0');			// passed, convert to BIN
		if(rtn > 9){
			rtn -= 'A' - '9' - 1;		// convert letters
			if(rtn < 0x0a) rtn = 0xff;	// 2nd validation step (if true, fail)
		}
	}
	return rtn;					// return result
}

//--------------------------------------------------------------------------------------
// get_chan() returns pointer to R5 of specified channel data
//--------------------------------------------------------------------------------------
U32 *get_chan(U8 chanum){
	U32 *ptemp;
	
	ptemp = pll_ch + (6 * chanum) + 5;		// channel pointer is base + #regs * ch# + 5
	return ptemp;
}

//--------------------------------------------------------------------------------------
// conv_to_chnum() converts dual-BCD port bits to channel# (0 = first channel)
//	if invalid BCD, returns ch#0
//--------------------------------------------------------------------------------------

U8 conv_to_chnum(U8 portbits){
	U8	i;				// temp return value
	U8	r;				// temp regs
	U8	s;
	
	r = portbits & 0x0f;		// get low BCD nyb
	s = (portbits >> 4) & 0x0f;
	if((r > 9) || (s > 9)){
		i = 0;					// bcd error, default to ch#0
	}else{
		i = r + (s * 10);		// get integer version of 2 digit BCD
	}
	return i;
}
//-----------------------------------------------------------------------------
// pca_intr
//-----------------------------------------------------------------------------
//
// PCA int, processes RDY pulse and rs232 PBSW button
//      PB uses two PCA inputs to trap button up and button down edges and applies
//          40ms of debounce to each edge.  button down increments button count & sets
//          pbd_flag.  Button up sets pbu_flag.
//

void pca_intr(void) interrupt 9 using 2
{
    // process RDY falling edge
    if(CCF0 == 1){
//        RDY_trap = 1;                   // set RDY trap to signal CPLD ready to read
        CCF0 = 0;                       // clr intr flag
    }
    // process push-button
    if(CCF1 == 1){
//        pb_state(PB_NORM);              // process switch state machine
        CCF1 = 0;                       // clr intr flag
    }
    return;
}
//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 2 overflows (NORM mode):
//      updates app timers @ 10ms rate
//		rate = (sysclk/12) / (65536 - TH:L)
//
//-----------------------------------------------------------------------------

void Timer2_ISR(void) interrupt 5 using 2
{

    TF2H = 0;                           // Clear Timer2 interrupt flag
    if(waittimer != 0){                 // g.p. delay timer
        waittimer--;
    }
    if(dbounce_tmr != 0){               // pbsw debounce timer
        dbounce_tmr--;
        if(dbounce_tmr == 0){
//            pb_state(PB_NORM);          // process switch state machine
        }
    }
//    if(temptimer != 0){                 // temperature delay timer
//        temptimer--;
//    }
}

#undef IS_MAINC
//**************
// End Of File
//**************

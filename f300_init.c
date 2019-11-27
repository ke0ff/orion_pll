/////////////////////////////////////
//  Generated Initialization File  //
/////////////////////////////////////

#include "compiler_defs.h"
#include "c8051F520.h"

// Peripheral specific initialization functions,
// Called from the Init_Device() function
void Reset_Sources_Init()
{
    int i = 0;
    VDDMON    = 0xA0;
    for (i = 0; i < 20; i++);  // Wait 5us for initialization
    RSTSRC    = 0x02;
}

void PCA_Init()
{
//    PCA0CN    = 0x40;
    PCA0CPM0  = 0x31;
}

void Timer_Init()
{
/*    TCON      = 0x40;
    TMOD      = 0x20;
    TH1       = 0x96;
    TMR2CN    = 0x04;
    TMR2RLL   = 0x06;
    TMR2RLH   = 0xF8;
    TMR2L     = 0x06;
    TMR2H     = 0xF8;*/

    TCON      = 0x40;
    TMOD      = 0x21;
    TL0       = 0xF0;
    TH0       = 0xFF;
    TH1       = 0x96;
    TMR2CN    = 0x04;
    TMR2RLL   = 0x06;
    TMR2RLH   = 0xF8;
    TMR2L     = 0x06;
    TMR2H     = 0xF8;
}

void UART_Init()
{
    SCON0     = 0x10;
}

void SPI_Init()
{
    SPI0CFG   = 0x40;
    SPI0CN    = 0x00;
    SPI0CKR   = 0x79;
}

void Port_IO_Init()
{
    // P0.0  -  Unassigned,  Push-Pull,  Digital
    // P0.1  -  Unassigned,  Open-Drain, Digital
    // P0.2  -  Unassigned,  Push-Pull,  Digital
    // P0.3  -  Unassigned,  Open-Drain, Digital
    // P0.4  -  TX   (UART), Push-Pull,  Digital
    // P0.5  -  RX   (UART), Open-Drain, Digital
    // P0.6  -  Unassigned,  Open-Drain, Digital
    // P0.7  -  Unassigned,  Push-Pull,  Digital

    // P1.0  -  Unassigned,  Open-Drain, Digital
    // P1.1  -  Unassigned,  Open-Drain, Digital
    // P1.2  -  Unassigned,  Open-Drain, Digital
    // P1.3  -  Unassigned,  Open-Drain, Digital
    // P1.4  -  Unassigned,  Open-Drain, Digital
    // P1.5  -  Unassigned,  Open-Drain, Digital
    // P1.6  -  Unassigned,  Open-Drain, Digital
    // P1.7  -  Unassigned,  Open-Drain, Digital

    P0MDOUT   = 0x95;
    P0SKIP    |= 0x30;
    XBR0      = 0x01;
    XBR1      = 0x40;
}

void Oscillator_Init()
{
    OSCICN    = 0xC7;
}

void Interrupts_Init()
{
    EIE1      = 0x04;
    IE        = 0x30;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    Reset_Sources_Init();
    PCA_Init();
    Timer_Init();
    UART_Init();
    SPI_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
}

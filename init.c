/////////////////////////////////////
//  Generated Initialization File  //
/////////////////////////////////////

#include "c8051F120.h"

// Peripheral specific initialization functions,
// Called from the Init_Device() function
void Reset_Sources_Init()
{
    WDTCN     = 0xFF;
    WDTCN     = 0x07;
}

void Timer_Init()
{
    SFRPAGE   = TMR2_PAGE;
    TMR2CN    = 0x04;
    TMR2CF    = 0x08;
    RCAP2L    = 0xC1;
    RCAP2H    = 0xFE;
    SFRPAGE   = TMR3_PAGE;
    TMR3CN    = 0x04;
    RCAP3L    = 0x7F;
    RCAP3H    = 0x60;
    TMR3L     = 0x7F;
    TMR3H     = 0x60;
    SFRPAGE   = TMR4_PAGE;
    TMR4CN    = 0x04;
    TMR4CF    = 0x08;
    RCAP4L    = 0x98;
    RCAP4H    = 0x49;
    TMR4L     = 0x98;
    TMR4H     = 0x49;
}

void PCA_Init()
{
    SFRPAGE   = PCA0_PAGE;
    PCA0MD    = 0x01;
}

void UART_Init()
{
    SFRPAGE   = UART0_PAGE;
    SCON0     = 0x70;
    SSTA0     = 0x15;
}

void SMBus_Init()
{
    SFRPAGE   = SMB0_PAGE;
    SMB0CN    = 0x40;
    SMB0CR    = 0xC7;
}

void SPI_Init()
{
    SFRPAGE   = SPI0_PAGE;
    SPI0CFG   = 0x40;
    SPI0CN    = 0x0D;
    SPI0CKR   = 0xC7;
}

void EMI_Init()
{
    SFRPAGE   = EMI0_PAGE;
    EMI0CF    = 0x37;
}

void Port_IO_Init()
{
    // P0.0  -  TX0 (UART0), Open-Drain, Digital
    // P0.1  -  RX0 (UART0), Open-Drain, Digital
    // P0.2  -  SCK  (SPI0), Open-Drain, Digital
    // P0.3  -  MISO (SPI0), Open-Drain, Digital
    // P0.4  -  MOSI (SPI0), Open-Drain, Digital
    // P0.5  -  NSS  (SPI0), Open-Drain, Digital
    // P0.6  -  SDA (SMBus), Open-Drain, Digital
    // P0.7  -  SCL (SMBus), Open-Drain, Digital

    // P1.0  -  Unassigned,  Open-Drain, Digital
    // P1.1  -  Unassigned,  Open-Drain, Digital
    // P1.2  -  Unassigned,  Open-Drain, Digital
    // P1.3  -  Unassigned,  Open-Drain, Digital
    // P1.4  -  Unassigned,  Open-Drain, Digital
    // P1.5  -  Unassigned,  Open-Drain, Digital
    // P1.6  -  Unassigned,  Open-Drain, Digital
    // P1.7  -  Unassigned,  Open-Drain, Digital

    // P2.0  -  Unassigned,  Open-Drain, Digital
    // P2.1  -  Unassigned,  Open-Drain, Digital
    // P2.2  -  Unassigned,  Open-Drain, Digital
    // P2.3  -  Unassigned,  Open-Drain, Digital
    // P2.4  -  Unassigned,  Open-Drain, Digital
    // P2.5  -  Unassigned,  Open-Drain, Digital
    // P2.6  -  Unassigned,  Open-Drain, Digital
    // P2.7  -  Unassigned,  Open-Drain, Digital

    // P3.0  -  Unassigned,  Open-Drain, Digital
    // P3.1  -  Unassigned,  Open-Drain, Digital
    // P3.2  -  Unassigned,  Open-Drain, Digital
    // P3.3  -  Unassigned,  Open-Drain, Digital
    // P3.4  -  Unassigned,  Open-Drain, Digital
    // P3.5  -  Unassigned,  Open-Drain, Digital
    // P3.6  -  Unassigned,  Open-Drain, Digital
    // P3.7  -  Unassigned,  Open-Drain, Digital

    SFRPAGE   = CONFIG_PAGE;
    XBR0      = 0x07;
    XBR2      = 0x40;
}

void Oscillator_Init()
{
    int i = 0;
    SFRPAGE   = CONFIG_PAGE;
    CCH0CN    &= ~0x20;
    SFRPAGE   = LEGACY_PAGE;
    FLSCL     = 0x90;
    SFRPAGE   = CONFIG_PAGE;
    CCH0CN    |= 0x20;
    PLL0CN    |= 0x01;
    PLL0DIV   = 0x01;
    PLL0FLT   = 0x21;
    PLL0MUL   = 0x02;
    for (i = 0; i < 15; i++);  // Wait 5us for initialization
    PLL0CN    |= 0x02;
    while ((PLL0CN & 0x10) == 0);
    CLKSEL    = 0x02;
    OSCICN    = 0x83;
}

void Interrupts_Init()
{
    IE        = 0x90;
    EIE1      = 0x0B;
    EIE2      = 0x05;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    Reset_Sources_Init();
    Timer_Init();
    PCA_Init();
    UART_Init();
    SMBus_Init();
    SPI_Init();
    EMI_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
}

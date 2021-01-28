#include <c8051F120.h>
#include "definition.h"
#include "macro.h"

xdata unsigned char ch0_value;

sbit sound = P4^0;

#define osc_freq 49E+6/2/12

#define DO      261.63 
#define DO_     277.18 
#define RE      293.67 
#define RE_     311.13 
#define MI      329.63 
#define FA      349.22 
#define FA_     369.99 
#define SOL     391.99 
#define SOL_    415.30 
#define LA      440.00 
#define LA_     466.16 
#define SI      493.88 
 


code unsigned int note[]={
0-osc_freq/DO,0-osc_freq/DO_,0-osc_freq/RE,0-osc_freq/RE_,0-osc_freq/MI,0-osc_freq/FA,0-osc_freq/FA_,0-osc_freq/SOL,0-osc_freq/SOL_,0-osc_freq/LA,0-osc_freq/LA_,0-osc_freq/SI,
0-osc_freq/DO/2,0-osc_freq/DO_/2,0-osc_freq/RE/2,0-osc_freq/RE_/2,0-osc_freq/MI/2,0-osc_freq/FA/2,0-osc_freq/FA_/2,0-osc_freq/SOL/2,0-osc_freq/SOL_/2,0-osc_freq/LA/2,0-osc_freq/LA_/2,0-osc_freq/SI/2,
0-osc_freq/DO/4,0-osc_freq/DO_/4,0-osc_freq/RE/4,0-osc_freq/RE_/4,0-osc_freq/MI/4,0-osc_freq/FA/4,0-osc_freq/FA_/4,0-osc_freq/SOL/4,0-osc_freq/SOL_/4,0-osc_freq/LA/4,0-osc_freq/LA_/4,0-osc_freq/SI/4,
0-osc_freq/DO/8,0-osc_freq/DO_/8,0-osc_freq/RE/8,0-osc_freq/RE_/8,0-osc_freq/MI/8,0-osc_freq/FA/8,0-osc_freq/FA_/8,0-osc_freq/SOL/8,0-osc_freq/SOL_/8,0-osc_freq/LA/8,0-osc_freq/LA_/8,0-osc_freq/SI/8,
0-osc_freq/DO/16,0-osc_freq/DO_/16,0-osc_freq/RE/16,0-osc_freq/RE_/16,0-osc_freq/MI/16,0-osc_freq/FA/16,0-osc_freq/FA_/16,0-osc_freq/SOL/16,0-osc_freq/SOL_/16,0-osc_freq/LA/16,0-osc_freq/LA_/16,0-osc_freq/SI/16,
0-osc_freq/DO/32,0-osc_freq/DO_/32,0-osc_freq/RE/32,0-osc_freq/RE_/32,0-osc_freq/MI/32,0-osc_freq/FA/32,0-osc_freq/FA_/32,0-osc_freq/SOL/32,0-osc_freq/SOL_/32,0-osc_freq/LA/32,0-osc_freq/LA_/32,0-osc_freq/SI/32,
//0-osc_freq/DO/64,0-osc_freq/DO_/64,0-osc_freq/RE/64,0-osc_freq/RE_/64,0-osc_freq/MI/64,0-osc_freq/FA/64,0-osc_freq/FA_/64,0-osc_freq/SOL/64,0-osc_freq/SOL_/64,0-osc_freq/LA/64,0-osc_freq/LA_/64,0-osc_freq/SI/64,
};

sfr16 PCA0 = 0xf9;
void pca0_isr() interrupt 9 using 3
{
    unsigned char back_sfr;

    back_sfr = SFRPAGE;

    clr_bit(PCA0CN,CF);
    clr_bit(PCA0CN,CR);
    PCA0 = note[ch0_value];
    set_bit(PCA0CN,CR);
    
    SFRPAGE = CONFIG_PAGE;
    sound = ~sound;
    SFRPAGE = back_sfr; 

}
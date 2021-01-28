
#ifndef __DEFINITION_H
#define __DEFINITION_H

//PSCTL
#define SFLE    2
#define PSEE    1
#define PSWE    0

//FLSCL
#define FLRT_1  5
#define FLRT_0  4
#define FLWE    0

//FLSTAT SFR Page:F
#define FLBUSY  0 

//CCH0CN SFR Page:F
#define CHWREN  7 
#define CHRDEN  6
#define CHPFEN  5
#define CHFLSH  4
#define CHRETI  3
#define CHISR   2
#define CHMOVC  1
#define CHBLKW  0

//SPI0CFG SFR Page:0 SPI0 Configuration Register
#define SPIBSY  7
#define MSTEN   6
#define CKPHA   5
#define CKPOL   4
#define SLVSEL  3
#define NSSIN   2
#define SRMT    1
#define RXBMT   1


//MAC0CF SFR Page:3 MAC0 Configuration Register
#define MAC0SC  5
#define MAC0SD  4
#define MAC0CA  3
#define MAC0SAT 2
#define MAC0FM  1
#define MAC0MS  0


//PCA0CN: SFR Page:0 PCA Control Register
#define CF      7
#define CR      6
#define CCF5    5
#define CCF4    4
#define CCF3    3
#define CCF2    2
#define CCF1    1
#define CCF0    0

//EIE2: SFR Page: All Pages Extended Interrupt Enable 

#define ES1     6
 
#define EADC2   4
#define EWADC2  3
#define ET4     2
#define EADC0   1
#define ET3     0



#endif //__DEFINITION_H
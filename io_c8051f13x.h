#include "io_macros.h"

#ifndef __c8051f13X_H
#define __c8051f13X_H

/*
typedef struct {
  __REG8 PSWE           :1;
  __REG8 PSEE           :1;
  __REG8 SFLE           :1;
  __REG8 none               :5;
} __psctl_bits;


__IO_REG8_BIT(PSCTL,0x8F,__psctl_bits);

*/

typedef struct {
  __REG8 PSWE           :1;
  __REG8 PSEE           :1;
  __REG8 SFLE           :1;
  __REG8                :5;
} __psctl_bits;
    
volatile union 
{
    unsigned char PSCTL;
    __psctl_bits psctl_bit;
}psctl _at_ 0x8f;

#endif    /* __c8051f13X_H */


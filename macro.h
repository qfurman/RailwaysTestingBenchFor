

#ifndef __MACROS_H
#define __MACROS_H

#define set_bit(z,x) z|=1<<x
#define clr_bit(z,x) z&=~(1<<x)

#define bcd_to_bin(bcd) ((0x0f&(bcd))+((bcd)>>4)*10)
#define bin_to_bcd(bin) ((bin)%10)|(((bin)/10)<<4)

#endif	//__MACROS_H


#ifndef __IO_MACROS_H
#define __IO_MACROS_H

#define __REG8 unsigned char


typedef struct
{ 
    unsigned char no0:1;
    unsigned char no1:1;
    unsigned char no2:1;
    unsigned char no3:1;
    unsigned char no4:1;
    unsigned char no5:1;
    unsigned char no6:1;
    unsigned char no7:1;
} __BITS8;


/***********************************************
 * Define NAME as an I/O reg
 * Access of 8/16/32 bit reg:  NAME
 ***********************************************/
#define __IO_REG8(NAME, ADDRESS, ATTRIBUTE)              \
                   volatile __no_init ATTRIBUTE unsigned char NAME _at_ ADDRESS;
        
        
/***********************************************
 * Define NAME as an I/O reg
 * Access of 8/16/32 bit reg:  NAME
 * Access of bit(s):           NAME_bit.noX  (X=1-31)
 ***********************************************/
#define __IO_REG8_BIT(NAME, ADDRESS, ATTRIBUTE, BIT_STRUCT)\
                       volatile __no_init ATTRIBUTE union \
                        {                                 \
                          unsigned char NAME;             \
                          BIT_STRUCT NAME ## _bit;      \
                        } @ ADDRESS;
        
        
#endif /* __IO_MACROS_H */
#include <c8051F120.h>
#include "macro.h"
#include "definition.h"



//стерти всю скретчпад область - 256 bytes
unsigned long count = 0;

/******************************************************************************/
/*void scratchpad_erase()
{
    WDTCN = 0xA5;//RELOAD WATCHDOG
    EA = 0;
        set_bit(PSCTL,SFLE);//FLASH access from user software directed to the two 128 byte Scratchpad sectors.
        set_bit(FLSCL,FLWE);//writes/erases enabled.
        set_bit(PSCTL,PSEE);//FLASH program memory erasure enabled
        set_bit(PSCTL,PSWE);//Write to FLASH program memory enabled. MOVX write operations target FLASH memory.
        *(char xdata*)0x400 = 0xff;//erase all scratchpad memory
        clr_bit(PSCTL,PSEE);//FLASH program memory erasure disabled.
        clr_bit(PSCTL,PSWE);//Write to FLASH program memory disabled. MOVX write operations target External RAM.
        clr_bit(FLSCL,FLWE);//writes/erases disabled.
        clr_bit(PSCTL,SFLE);//FLASH access from user software directed to the 128k byte Program/Data FLASH sector.
    EA = 1;    
}

*/
/******************************************************************************/
//src - звідки(xdata), dest - куди(scratchpad), quantity - скільки
/*void scratchpad_write(char xdata*src, char xdata*dest, unsigned char data quantity)
{
    data unsigned char byte;
    char xdata*data src_;
    char xdata*data dest_;

    src_ = src;
    dest_ = dest;
        
    do{
        WDTCN = 0xA5;//RELOAD WATCHDOG
        byte = *src_;
        EA = 0;
        set_bit(PSCTL,SFLE);//FLASH access from user software directed to the two 128 byte Scratchpad sectors.
        set_bit(FLSCL,FLWE);//writes/erases enabled.
        set_bit(PSCTL,PSWE);//Write to FLASH program memory enabled. MOVX write operations target FLASH memory.
        *dest_ = byte;
        clr_bit(PSCTL,PSWE);//Write to FLASH program memory disabled. MOVX write operations target External RAM.
        clr_bit(FLSCL,FLWE);//writes/erases disabled.
        clr_bit(PSCTL,SFLE);//FLASH access from user software directed to the 128k byte Program/Data FLASH sector.
        EA = 1;
        dest_++;
        src_++;
    }
    while(--quantity);
                
}
*/
/******************************************************************************/
//src - звідки(scratchpad), dest - куди(xdata), quantity - скільки
/*void scratchpad_read(char code*src, char xdata*dest, unsigned char data quantity)
{
    data unsigned char byte;
    char code*data src_;
    char xdata*data dest_;

    src_ = src;
    dest_ = dest;

    do{
        WDTCN = 0xA5;//RELOAD WATCHDOG
        EA = 0;
        set_bit(PSCTL,SFLE);//FLASH access from user software directed to the two 128 byte Scratchpad sectors.
        byte = *src_;
        clr_bit(PSCTL,SFLE);//FLASH access from user software directed to the 128k byte Program/Data FLASH sector.
        EA = 1;
        *dest_ = byte;
        dest_++;
        src_++;
    }
    while(--quantity);        
}
*/
/******************************************************************************/
void flash_erase()
{
   char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
   char PSBANK_SAVE = PSBANK;          // preserve PSBANK

    WDTCN = 0xA5;//RELOAD WATCHDOG
    EA = 0;

        SFRPAGE = LEGACY_PAGE;
        PSBANK &= ~0x30;
        PSBANK |=  0x20;//COBANK = 2
        
        set_bit(FLSCL,FLWE);//writes/erases enabled.
        set_bit(PSCTL,PSEE);//FLASH program memory erasure enabled
        set_bit(PSCTL,PSWE);//Write to FLASH program memory enabled. MOVX write operations target FLASH memory.
        *(char xdata*)0x8000 = 0x00;//erase 1024 byte memory
        clr_bit(PSCTL,PSEE);//FLASH program memory erasure disabled.
        clr_bit(PSCTL,PSWE);//Write to FLASH program memory disabled. MOVX write operations target External RAM.
        clr_bit(FLSCL,FLWE);//writes/erases disabled.
        
        PSBANK = PSBANK_SAVE;               // restore PSBANK
        SFRPAGE = SFRPAGE_SAVE;             // restore SFRPAGE
    EA = 1;
}


/******************************************************************************/
void flash_write(char xdata*src, char xdata*dest, unsigned int data quantity)
{
    data unsigned char byte;
    char xdata*data src_;
    char xdata*data dest_;
    char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
    char PSBANK_SAVE = PSBANK;          // preserve PSBANK

    src_ = src;
    dest_ = dest;
        
    do{
        WDTCN = 0xA5;//RELOAD WATCHDOG
        byte = *src_;
        EA = 0;
    
            SFRPAGE = LEGACY_PAGE;
            PSBANK &= ~0x30;
            PSBANK |=  0x20;//COBANK = 1
            
            set_bit(FLSCL,FLWE);//writes/erases enabled.
            set_bit(PSCTL,PSWE);//Write to FLASH program memory enabled. MOVX write operations target FLASH memory.
            *dest_ = byte;
            clr_bit(PSCTL,PSWE);//Write to FLASH program memory disabled. MOVX write operations target External RAM.
            clr_bit(FLSCL,FLWE);//writes/erases disabled.
            
            PSBANK = PSBANK_SAVE;// restore PSBANK
            SFRPAGE = SFRPAGE_SAVE;// restore SFRPAGE
    
        EA = 1;
        dest_++;
        src_++;
    }
    while(--quantity);
                
}
/******************************************************************************/
void flash_read(char code*src, char xdata*dest, unsigned int data quantity)
{
    data unsigned char byte;
    char code*data src_;
    char xdata*data dest_;
    char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
    char PSBANK_SAVE = PSBANK;          // preserve PSBANK
 

    src_ = src;
    dest_ = dest;

    do{
        WDTCN = 0xA5;//RELOAD WATCHDOG
        EA = 0;
            
            SFRPAGE = LEGACY_PAGE;
            PSBANK &= ~0x30;                 // COBANK = 0x2
            PSBANK |=  0x20;
      
            byte = *src_;
        
            PSBANK = PSBANK_SAVE;
            SFRPAGE = SFRPAGE_SAVE;             // restore SFRPAGE
        
        EA = 1;
        *dest_ = byte;
        dest_++;
        src_++;
    }
    while(--quantity);
            
}
/******************************************************************************/
void flash_lock()
{
    char SFRPAGE_SAVE = SFRPAGE;        // preserve SFRPAGE
    char PSBANK_SAVE = PSBANK;          // preserve PSBANK

        WDTCN = 0xA5;//RELOAD WATCHDOG
        EA = 0;
/*    
            SFRPAGE = LEGACY_PAGE;
            PSBANK &= ~0x30;
            PSBANK |=  0x30;//COBANK = 1
            
            set_bit(FLSCL,FLWE);//writes/erases enabled.
            set_bit(PSCTL,PSWE);//Write to FLASH program memory enabled. MOVX write operations target FLASH memory.
            *(char xdata*)0xfbff = 0x73;
            clr_bit(PSCTL,PSWE);//Write to FLASH program memory disabled. MOVX write operations target External RAM.
            clr_bit(FLSCL,FLWE);//writes/erases disabled.
            
            PSBANK = PSBANK_SAVE;// restore PSBANK
            SFRPAGE = SFRPAGE_SAVE;// restore SFRPAGE
*/    
        EA = 1;
             
}
/******************************************************************************/







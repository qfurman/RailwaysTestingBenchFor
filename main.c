#include <c8051F120.h>
#include <lcd.h>
#include <stdio.h>

#include "init.h"
#include "macro.h"
#include "scratch.h"
#include "i2c.h"
#include "timer_4.h"
#include "spi.h"
#include "pca0.h"
#include "menu.h"
#include "uart_0.h"
#include "timer_3.h"
#include "spec_func.h"



void main()
{
    /*unsigned char next_one;
    unsigned long temp;
    unsigned int out;
    unsigned long for_relay = 1;
    */



    Init_Device();
    SFRPAGE   = LEGACY_PAGE;
    WDTCN = 0xA5;//RELOAD WATCHDOG
    flash_lock();
    lcd_init();
    timer_4_init();
    i2c_init();
    spi0_init();
    menu_init();
    timer_3_init();
    //scratchpad_erase();
    //scratchpad_write(str,0,41);

  
    
    ch0_value = 0x0000;

    for(;;)
    {
        WDTCN = 0xA5;//RELOAD WATCHDOG
        i2c_process();
        processing();
        cts_process();//clear to send Ó˜≥ÍÛ‚‡ÌÌˇ „ÓÚÓ‚ÌÓÒÚ≥ RS232—
        cool_control();//¬ Àﬁ◊≈ÕÕﬂ ¬≈Õ“»Àﬂ“Œ–≤¬ Ã“—
        leds_and_alarm();

/*        sprintf(lcd_str,"test string   %#06x",(int)io);
        lcd_put_str(0,0,lcd_str,0);
        //scratchpad_read(0,lcd_str,41);
        //gettimestring((char*)&datetime);


        sprintf(lcd_str,"%#02d:%#02d:%#02d",
        (int)bcd_to_bin(datetime.hours),
        (int)bcd_to_bin(datetime.minutes),
        (int)bcd_to_bin(datetime.seconds)
        );
        lcd_put_str(1,0,lcd_str,0);
        io = ~io;

        ml140_160.cool = ~io;
        ml140_150.reg = 0x22;
        next_one = keys_pop();
        if(next_one == 6)ml140_130.dac_ad7243--;
        if(next_one == 5)ml140_130.dac_ad7243++;
        
        if(next_one == 6)ml140_140.dac_ad5312--;
        if(next_one == 5)ml140_140.dac_ad5312++;
        
        if(next_one == 8)ch0_value++;
        if(next_one == 7)ch0_value--;
        temp = (ml140_130.dac_ad7243<<4)? (ml140_130.dac_ad7243<<4) : 0;
        temp *= (unsigned long)5000;
        *(unsigned char*)&out =  *(unsigned char*)&temp;
        *((unsigned char*)&out+1) =  *((unsigned char*)&temp+1);
        if(*((unsigned char*)&temp+2) & 0x80)out++;
        //temp = 
        sprintf(lcd_str,"%#06d %#06x %#04x",out,ml140_150.adc_ads7808,(int)ch0_value);
        lcd_put_str(2,0,lcd_str,0);
        if(next_one == 8)
            if(for_relay != 0x00800000)for_relay = for_relay << 1;
        if(next_one == 7)
            if(for_relay != 1)for_relay = for_relay >> 1;

        ml140_140.relay16_23 = *((unsigned char*)&for_relay+1);
        ml140_140.relay8_15 = *((unsigned char*)&for_relay+2);
        ml140_140.relay0_7 = *((unsigned char*)&for_relay+3);

        sprintf(lcd_str,"%#06x %#010L",ml140_140.dac_ad5312,(unsigned long)for_relay);
        lcd_put_str(3,0,lcd_str,0);
*/
    }

}



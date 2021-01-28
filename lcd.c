#include <ctype.h>

xdata unsigned char lcd_command   _at_ 0x4000;
xdata unsigned char lcd_data      _at_ 0x4001;
xdata unsigned char lcd_busy      _at_ 0x4002;
xdata unsigned char lcd_data_read _at_ 0x4003;

xdata char lcd_str[256];
xdata char lcd_input_str[24];
xdata char lcd_crsr_position;

code unsigned char lcd_koi[64]=
{0x41,0xA0,0x42,0xA1,0xE0,0x45,0xA3,0xA4,0xA5,0xA6,0x4B,0xA7,0x4D,0x48,0x4F,0xA8,
 0x50,0x43,0x54,0xA9,0xAA,0x58,0xE1,0xAB,0xAC,0xE2,0xAD,0xAE,0x62,0xAF,0xB0,0xB1,
 0x61,0xB2,0xB3,0xB4,0xE3,0x65,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0x6F,0xBE,
 0x70,0x63,0xBF,0x79,0xE4,0x78,0xE5,0xC0,0xC1,0xE6,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7};

/******************************************************************************/
/******************************************************************************/
void lcd_delay (unsigned char arg)
{
    unsigned long arg_del;
    arg_del = arg*1000;//значення 2000 при 98MGhz - ~1mS
    while (arg_del) arg_del--; //затримка arg * 1mS
}
/******************************************************************************/
void lcd_wait_for_ready(void)
{
    while (lcd_busy&(1<<(7)));
    while (lcd_busy&(1<<(7)));
    while (lcd_busy&(1<<(7)));
    while (lcd_busy&(1<<(7)));
}
/******************************************************************************/
void lcd_cls(void)
{
    lcd_command = 0x01;
}
/******************************************************************************/
void lcd_init (void)
{
    lcd_delay(30);
    lcd_command = 0x30;

    lcd_delay(4);
    lcd_command = 0x30;

    lcd_delay(1);
    lcd_command = 0x30;

    lcd_wait_for_ready();
    lcd_command = 0x38;
    
    lcd_wait_for_ready();
    lcd_command = 0x08;
    
    lcd_wait_for_ready();
    lcd_command = 0x01;
    
    lcd_wait_for_ready();
    lcd_command = 0x06;
    
    lcd_wait_for_ready();
    lcd_command = 0x0c;
    
    lcd_wait_for_ready();

    lcd_cls();
}
/******************************************************************************/
void lcd_put_str(unsigned char row,column,unsigned char *string, bit charact)
{
    unsigned char ch;

    switch (row)
    {
         case 0: lcd_command = 0x80|(0x00+column);break;
         case 1: lcd_command = 0x80|(0x40+column);break;
         case 2: lcd_command = 0x80|(0x14+column);break;
         case 3: lcd_command = 0x80|(0x54+column);break;
    }
    lcd_wait_for_ready();

    for(;;)
    {
        ch = *string++;
        if(!ch)break;
        if((ch>=192)&(!charact)) ch = lcd_koi[ch-192];
        lcd_data = ch;

        while (lcd_busy&(1<<(7)));
    }

}
/******************************************************************************/
void lcd_read_str(char row)
{
    char i;

    while(lcd_busy&(1<<(7)));
    while(lcd_busy&(1<<(7)));
    while(lcd_busy&(1<<(7)));
    while(lcd_busy&(1<<(7)));
    switch (row)
    {
         case 0: lcd_command = 0x80|0x00;break;
         case 1: lcd_command = 0x80|0x40;break;
         case 2: lcd_command = 0x80|0x14;break;
         case 3: lcd_command = 0x80|0x54;break;
    }

    for(i=0;i<20;i++)
    {
        while(lcd_busy&(1<<(7)));
        while(lcd_busy&(1<<(7)));
        while(lcd_busy&(1<<(7)));
        while(lcd_busy&(1<<(7)));
        lcd_input_str[i] = lcd_data_read;
    }
}
/******************************************************************************/
void lcd_crsr_place(char row)
{
    while(lcd_busy&(1<<(7)));
    switch (row)
    {
         case 0: lcd_command = 0x80|0x00 + lcd_crsr_position; break;
         case 1: lcd_command = 0x80|0x40 + lcd_crsr_position; break;
         case 2: lcd_command = 0x80|0x14 + lcd_crsr_position; break;
         case 3: lcd_command = 0x80|0x54 + lcd_crsr_position; break;
    }
}
/******************************************************************************/
bit lcd_init_crsr_position()
{
    lcd_crsr_position = 19;

    do{
        if(isdigit(lcd_input_str[lcd_crsr_position]))return 1;
    }while(lcd_crsr_position--);

    return 0;
}    //|| ispunct(lcd_input_str[lcd_crsr_position])
/******************************************************************************/













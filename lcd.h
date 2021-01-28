/*--------------------------------------------------------------------------
LCD.H

Standard functions.
Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
All rights reserved.
--------------------------------------------------------------------------*/

#ifndef __LCD_H__
#define __LCD_H__

extern xdata volatile unsigned char lcd_command;
extern xdata volatile unsigned char lcd_data;
extern xdata volatile unsigned char lcd_busy;
extern xdata volatile unsigned char lcd_data_read;

extern xdata char lcd_str[];
extern xdata char lcd_input_str[];
extern xdata char lcd_crsr_position;

extern void lcd_wait_for_ready(void);
extern void lcd_init (void);
extern void lcd_cls(void);
extern void lcd_put_str(unsigned char row,column,unsigned char *string, bit charact);
//extern void lcd_read_str(char row);
//extern void lcd_crsr_place(char row);
extern bit lcd_init_crsr_position();//(char row);


#endif
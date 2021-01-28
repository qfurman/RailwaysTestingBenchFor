#include <c8051F120.h>
#include "timer_4.h"

sbit  sda = P0^6;
sbit  scl = P0^7;

//*****************************************************************
//*****************************************************************
//*****************************************************************
xdata struct
{
    unsigned char task_number;//номер задачі
    unsigned char kill_after_execution;//зняти після виконання
}i2c_management;
//*****************************************************************
xdata struct Tdatetime
{
	unsigned char	seconds;
	unsigned char	minutes;
	unsigned char	hours;
	unsigned char	day;
	unsigned char	date;
	unsigned char	mounth;
	unsigned char	year;
	unsigned char	calibrate;
} datetime,datetime_w;

xdata struct{
    unsigned char addr;
    unsigned char count;
    unsigned char xdata* ptr;
} i2cdata,i2cdata_w;

//*****************************************************************

unsigned char I2Ccmd = 0;
unsigned char I2Ccount = 0;
unsigned char I2Caddr = 0;
unsigned char I2Cp = 0;
unsigned char xdata*I2Cptr;
unsigned char I2Cok = 0;

//*****************************************************************
void i2c_init()
{
    i2c_management.task_number = 0;
    i2c_management.kill_after_execution = 0;
}
//*****************************************************************
//ручною маніпуляцією SCL виводить і2с із коми
void i2c_reset()
{
    unsigned char i,k, back_sfr;
    
    i = 24;    
    back_sfr = SFRPAGE;
    //зробити глибокий ресет завнішній мікросхемі
    SFRPAGE   = CONFIG_PAGE;
    XBR0      = 0x06;
    do{
        scl = 0;
        k = 64;
        while(--k);
        scl = 1;
        k = 64;
        while(--k);
    }while(--i);
    XBR0      = 0x07;
    //зробити ресет і2с контролеру
    SFRPAGE   = SMB0_PAGE;
    EA = 0;
    SMB0CN    = 0x00;
    SMB0CN    = 0x40;
    EA = 1;

    SFRPAGE   = back_sfr;
}
//*****************************************************************

void i2c_int(void) interrupt 7 using 2
{

	switch(I2Ccmd)
    { 
    	case 1://Get data
    		switch(SMB0STA)
            {
                case 0x08:
        			SMB0DAT = 0xd0;
        			STA = 0;
        			I2Cp = 0;
        		    break;
        
        		case 0x18:
        			SMB0DAT = I2Caddr;	
        			STA = 0;
        			I2Cp = 0;
        		    break;        
        
        		case 0x28:
                	STA =1;
        		    break;
        		
        		case 0x10:
        			SMB0DAT = 0xd1;
        			STA = 0;
        			I2Cp = 0;
        		    break;
        
        		case 0x50:
        			*(I2Cptr+I2Cp) = SMB0DAT;	
        			I2Cp++;
        			if (I2Cp==I2Ccount) AA = 0; else AA = 1;
        			//break;
        
        		default: if(I2Cp<=I2Ccount)break;
                case 0x48:
                case 0x58:
                case 0x00:
                    STO = 1;
        			I2Cok = 1;        		    
    	    }
            break;
    
    	case 2://Set data
    		switch(SMB0STA){            
                case 0x18:
        		    SMB0DAT = I2Caddr;	
        			STA = 0;
        			I2Cp = 0;
        		    break;    
        
        		case 0x08:
        		case 0x10:
        			SMB0DAT = 0xd0;
        			STA = 0;
        			I2Cp = 0;
                    break;
        
        		case 0x28:
        			SMB0DAT = *(I2Cptr+I2Cp);	
        			I2Cp++;
        		    //break;
        
        		default: if(I2Cp<=I2Ccount)break;
        			STO = 1;
        			I2Cok = 1;
        	}                
	        AA = 0;
            break;
    }
	SI = 0;
}
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
/*void I2Csetdata()
{
    if(i2cdata_w.addr > 55)i2cdata_w.addr = 0;
    if(i2cdata_w.count > 56)i2cdata_w.count = 1;
    if(i2cdata_w.addr+i2cdata_w.count>56)i2cdata_w.count = 56-i2cdata_w.addr;
    I2Cok = 0;
	I2Ccount = i2cdata_w.count;
	I2Ccmd = 2;
	I2Caddr = 8 + i2cdata_w.addr;
	I2Cptr = i2cdata_w.ptr;
	AA = 0;
	STA = 1;
    i2c_management.kill_after_execution = 1;
}
*/
//*****************************************************************
/*void I2Cgetdata()
{
	I2Cok = 0;
	I2Ccount = i2cdata.count;
	I2Ccmd = 1;
	I2Caddr = 8 + i2cdata.addr;
	I2Cptr = i2cdata.ptr;
	AA = 1;
	STA = 1;
    i2c_management.kill_after_execution = 1;
}
*/
//*****************************************************************
void settime()
{
    I2Cok = 0;
	I2Ccount = 8;
	I2Ccmd = 2;
	I2Caddr = 0;
	I2Cptr = (char*)&datetime_w;
	AA = 0;
	STA = 1;
    i2c_management.kill_after_execution = 1;
}
//*****************************************************************
void gettime()
{
	I2Cok = 0;
	I2Ccount = 8;
	I2Ccmd = 1;
	I2Caddr = 0;
	I2Cptr = (char*)&datetime;
	AA = 1;
	STA = 1;
}
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//головна фішка сезону - масив функцій які використовують і2с
//мають перевіряти і2с на готовність, якщо зайнятий то з вийти функції
void(*i2c_func[5])()= {gettime,0,0,0,0};
//*****************************************************************
//*****************************************************************

void i2c_process()
{

	if(BUSY && *timer_arr[0].tick != 0)return;//якщо таймер запущений то вийти
    timer_arr[0].start_funct();//почати передачу
    
    if(i2c_management.kill_after_execution){
        i2c_management.kill_after_execution = 0;
        i2c_func[i2c_management.task_number] = 0;
    }

    if(i2c_management.task_number<(sizeof(i2c_func)/sizeof(void*))-1)i2c_management.task_number++;
    else i2c_management.task_number = 0;

    if(i2c_func[i2c_management.task_number]){
        i2c_func[i2c_management.task_number]();
        return;
    }
}
//*****************************************************************
//*****************************************************************
//*****************************************************************

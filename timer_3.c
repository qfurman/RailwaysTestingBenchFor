#include <c8051F120.h>
#include "macro.h"
#include "definition.h"
#include "timer_4.h"
#include "spi.h"
#include "spec_func.h"
#include "uart_0.h."
#include "menu.h"

typedef struct virtual_timer_p
{
    unsigned int xdata*tick;//відліки до нуля
    void(*start_funct)(unsigned char i);   //фунція запуску
    void(*end_funct)(unsigned char i);     //дія після досягнення нуля
};

typedef enum DAC_status{idle,runing,stoping};

typedef struct DAC{
    unsigned long value;
    long delta;
    unsigned int limit;
    enum DAC_status status;
};

xdata struct DAC MTS[4];

void mts_start(unsigned char i);
void mts_end(unsigned char i);

void gpt_start(unsigned char i);
void gpt_stop();

void printer_stop();

void regulator_();
void regulator_50();
                           
xdata unsigned int xtimer_arr3[11];

code struct virtual_timer_p timer_arr3[]={
{&xtimer_arr3[0],mts_start,mts_end},
{&xtimer_arr3[1],mts_start,mts_end},
{&xtimer_arr3[2],mts_start,mts_end},
{&xtimer_arr3[3],mts_start,mts_end},
{&xtimer_arr3[4],gpt_start,gpt_stop},//таймер загального використання для організації тестування блоків
{&xtimer_arr3[5],0,0},//таймер для аварії КЗ МТС
{&xtimer_arr3[6],0,printer_stop},//таймер для зняття задачі якщо принтер не готовий
{&xtimer_arr3[7],0,0},//таймер для вимірювання часових ітнервалів
{&xtimer_arr3[8],0,regulator_},//таймер для регулятора напруги 
{&xtimer_arr3[9],0,regulator_50},//таймер для регулятора напруги 
{&xtimer_arr3[10],0,0},//таймер загального викориcтання 
};

void timer3_ISR (void) interrupt 14 using 3//10mS таймер
{
    unsigned char i;
    TF3 = 0;

    for(i=0;i<sizeof(timer_arr3)/sizeof(struct virtual_timer);i++){
        if(*timer_arr3[i].tick != 0){
            *timer_arr3[i].tick -= 1;
            if(*timer_arr3[i].tick == 0 && timer_arr3[i].end_funct)
                timer_arr3[i].end_funct(i);
        } 
    }
}

void timer_3_init()
{
    unsigned char i;

    for(i=0;i<(sizeof(timer_arr3)/sizeof(struct virtual_timer_p));i++)
        *timer_arr3[i].tick = 0;

}

void mts_start(unsigned char i)
{
    *timer_arr3[i].tick = 1;
    MTS[i].status = 1;
}

void mts_end(unsigned char i)
{
    unsigned int temp;

    if(MTS[i].status == 2)return;//якщо зупинений то не перезапускати

    *((char*)&temp) =  *((char*)&MTS[i].value);
    *((char*)&temp+1) =  *((char*)&MTS[i].value+1);

    if(((MTS[i].delta>=0)&&(temp >= MTS[i].limit))||((MTS[i].delta<0)&&(temp <= MTS[i].limit))){
        *((char*)&MTS[i].value) = *((char*)&MTS[i].limit);
        *((char*)&MTS[i].value+1) = *((char*)&MTS[i].limit+1);
        MTS[i].status = 2;

    }
    else{
        MTS[i].value += MTS[i].delta;
        *timer_arr3[i].tick = 1;//перезапустити
    }
    if(i != 3){
        ml140_130[i].dac_ad7243 = *((int*)&MTS[i].value);
    }
    else{
        ml140_140.dac_ad5312 = *((int*)&MTS[i].value);
    }
}

void gpt_start(unsigned char i)//0 - 225 секунд
{
    *timer_arr3[4].tick = (unsigned int)i*100;
}               

void gpt_stop()
{
    tst_tact++;
}

void printer_stop()
{
    printer_not_ready = 0;
}

void regulator_()
{
    *timer_arr3[8].tick = Tdiscrete180;
    PI_regulator();
}

void regulator_50()
{
    *timer_arr3[9].tick = Tdiscrete50;
    PI_regulator50();
}

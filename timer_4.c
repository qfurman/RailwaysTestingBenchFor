#include <c8051F120.h>
#include "macro.h"
#include "definition.h"
#include "i2c.h"
#include "pca0.h"


xdata volatile unsigned char io _at_ 0x8000;
/******************************************************************************/
//віртуальний таймер
typedef struct virtual_timer
{
    unsigned int xdata*tick;//відліки до нуля
    void(*start_funct)();   //фунція запуску
    void(*end_funct)();     //дія після досягнення нуля
};
/******************************************************************************/
#define keys_buf_size 16

xdata struct{
    unsigned char buf[keys_buf_size];
    unsigned char index;
}keys;

typedef struct repeat{
    unsigned int first;
    unsigned int all_next;
};

xdata struct repeat key_temp={0,0};
code struct repeat key_set={50,2};//зразкові інтервали перша *20мс всі наступні *20мс 

/******************************************************************************/

typedef struct melody{
    unsigned char nota;     //12нот х 6 октав = 0..71; 128 - пауза 255 - кінець
    unsigned int duration; //1000-1с; 500-1/2с; 125-1/4с; 8-1/8с.
};
//          
#define CC(octave)  0+12*(octave-1)
#define CC_(octave) 1+12*(octave-1)
#define DD(octave)  2+12*(octave-1)
#define DD_(octave) 3+12*(octave-1)
#define EE(octave)  4+12*(octave-1)
#define FF(octave)  5+12*(octave-1)
#define FF_(octave) 6+12*(octave-1)
#define GG(octave)  7+12*(octave-1)
#define GG_(octave) 8+12*(octave-1)
#define AA(octave)  9+12*(octave-1)
#define AA_(octave) 10+12*(octave-1)
#define BB(octave)  11+12*(octave-1)
 
#define dur(t,w) 1000.0/(float)t/(float)w

code struct melody death[]={
//#define death_speed 0.75
#define death_speed 5
{BB(2),dur(10,death_speed)},//c.2,
{AA(2),dur(10,death_speed)},//c.2,
{GG(2),dur(10,death_speed)},//c.2,
{FF(2),dur(10,death_speed)},//c.2,
{EE(2),dur(10,death_speed)},//c.2,
{DD(2),dur(10,death_speed)},//c.2,
{CC(2),dur(10,death_speed)},//c.2,

{BB(2),dur(10,death_speed)},//c.2,
{CC(2),dur(10,death_speed)},//c.2,
{DD(2),dur(10,death_speed)},//c.2,
{EE(2),dur(10,death_speed)},//c.2,
{FF(2),dur(10,death_speed)},//c.2,
{GG(2),dur(10,death_speed)},//c.2,
{AA(2),dur(10,death_speed)},//c.2,

{BB(2),dur(10,death_speed)},//c.2,
{AA(2),dur(10,death_speed)},//c.2,
{GG(2),dur(10,death_speed)},//c.2,
{FF(2),dur(10,death_speed)},//c.2,
{EE(2),dur(10,death_speed)},//c.2,
{DD(2),dur(10,death_speed)},//c.2,
{CC(2),dur(10,death_speed)},//c.2,

{BB(2),dur(10,death_speed)},//c.2,
{CC(2),dur(10,death_speed)},//c.2,
{DD(2),dur(10,death_speed)},//c.2,
{EE(2),dur(10,death_speed)},//c.2,
{FF(2),dur(10,death_speed)},//c.2,
{GG(2),dur(10,death_speed)},//c.2,
{AA(2),dur(10,death_speed)},//c.2,

{BB(2),dur(10,death_speed)},//c.2,
{AA(2),dur(10,death_speed)},//c.2,
{GG(2),dur(10,death_speed)},//c.2,
{FF(2),dur(10,death_speed)},//c.2,
{EE(2),dur(10,death_speed)},//c.2,
{DD(2),dur(10,death_speed)},//c.2,
{CC(2),dur(10,death_speed)},//c.2,

/*
{CC(2),dur(3,death_speed)},//c.2,       
{CC(2),dur(3,death_speed)},//c2,
{CC(2),dur(5,death_speed)},//8c2,
{CC(2),dur(3,death_speed)},//c.2,
{DD_(2),dur(3,death_speed)},//d#2,
{DD(2),dur(5,death_speed)},//8d2,
{DD(2),dur(3,death_speed)},//d2,
{CC(2),dur(5,death_speed)},//8c2,
{CC(2),dur(3,death_speed)},//c2,
{BB(1),dur(5,death_speed)},//8b1,
{CC(2),dur(3,death_speed)},//c2
*/
{255,0}
};

code struct melody yesterday[]={
#define yesterday_speed 1.25
 
{DD(3),dur(3,yesterday_speed)},//d2
{CC(3),dur(3,yesterday_speed)},//c2 
{CC(3),dur(2,yesterday_speed)},//2c2 
{128,dur(3,yesterday_speed)},//4p 
{EE(3),dur(3,yesterday_speed)},//e2 
{FF_(3),dur(3,yesterday_speed)},//f#2 
{GG_(3),dur(3,yesterday_speed)},//g#2 
{AA(3),dur(3,yesterday_speed)},//a2 
{BB(3),dur(3,yesterday_speed)},//b2 
{CC(4),dur(3,yesterday_speed)},//c3 
{BB(3),dur(2,yesterday_speed)},//4b2 
{AA(3),dur(3,yesterday_speed)},//a2 
{AA(3),dur(2,yesterday_speed)},//2a2 
{128,dur(4,yesterday_speed)}, //4p
{AA(3),dur(3,yesterday_speed)},//a2 
{AA(3),dur(3,yesterday_speed)},//a2 
{GG(3),dur(3,yesterday_speed)},//g2 
{FF(3),dur(3,yesterday_speed)},//f2 
{EE(3),dur(3,yesterday_speed)},//e2 
{DD(3),dur(3,yesterday_speed)},//d2 
{FF(3),dur(2,yesterday_speed)},//4f2 
{EE(3),dur(3,yesterday_speed)},//e2 
{EE(3),dur(2,yesterday_speed)},//2e2 
{DD(3),dur(3,yesterday_speed)},//d2
{CC(3),dur(2,yesterday_speed)},//4c2 
{EE(3),dur(3,yesterday_speed)},//e2 
{DD(3),dur(2,yesterday_speed)},//4d2 
{128,dur(3,yesterday_speed)}, //p
{AA(2),dur(2,yesterday_speed)},//4a1 
{CC(3),dur(2,yesterday_speed)},//4c2 
{EE(3),dur(3,yesterday_speed)},//e2 
{EE(3),dur(2,yesterday_speed)},//2e2
{255,0}
};

code struct melody ole[]={
#define ole_speed 1.25
//4e2, 4g2, p, e2, g2, e2, g2, e2, 2c2, 4p, 4e2, 16d2, 16p, 2d2, p, 4e2, 2c2
{EE(2),dur(4,ole_speed)},
{GG(2),dur(4,ole_speed)}, 
{128,dur(3,ole_speed)}, 
{EE(2),dur(3,ole_speed)}, 
{GG(2),dur(3,ole_speed)}, 
{EE(2),dur(3,ole_speed)}, 
{GG(2),dur(3,ole_speed)}, 
{EE(2),dur(3,ole_speed)}, 
{CC(2),dur(3,ole_speed)}, 
{128,dur(4,ole_speed)}, 
{EE(2),dur(4,ole_speed)}, 
{DD(2),dur(5,ole_speed)}, 
{128,dur(5,ole_speed)}, 
{DD(2),dur(3,ole_speed)}, 
{128,dur(3,ole_speed)}, 
{EE(2),dur(4,ole_speed)}, 
{CC(2),dur(3,ole_speed)},
{255,0}
};

code struct melody beep[]={
#define beep_speed 2
/*{EE(2),dur(10,beep_speed)},
{CC(2),dur(10,beep_speed)},
{CC_(2),dur(10,beep_speed)},
{DD(2),dur(10,beep_speed)},
{DD_(2),dur(10,beep_speed)},
{EE(2),dur(10,beep_speed)},
{FF(2),dur(10,beep_speed)},
{FF_(2),dur(10,beep_speed)},
{GG(2),dur(10,beep_speed)},
{GG_(2),dur(10,beep_speed)},*/
{AA(2),dur(10,beep_speed)},
{AA_(2),dur(10,beep_speed)},
{BB(2),dur(10,beep_speed)},
{255,0}
};

code struct melody uuuup[]={
#define uuuup_speed 2
{BB(0),dur(10,uuuup_speed)},
{AA_(0),dur(10,uuuup_speed)},
{AA(0),dur(10,uuuup_speed)},
{255,0}
};

code struct melody final[]={
#define final_speed 1
{BB(2),dur(1,final_speed)},
{128,dur(1,final_speed)},
{BB(2),dur(1,final_speed)},
{128,dur(1,final_speed)},
{BB(2),dur(1,final_speed)},
{255,0}
};

xdata struct melody *melody_ptr;    //сюди треба записати початок мелодії і запустити мелодійний таймер

/******************************************************************************/
void i2c_start_timeout();
void i2c_end_timeout();
void keys_start_timeout();
void keys_end_timeout();
void sound_start_timeout();
void lcd_blink_start_timeout();
/******************************************************************************/
xdata unsigned int xtimer_arr[5];

code struct virtual_timer timer_arr[]={
{&xtimer_arr[0],i2c_start_timeout,i2c_end_timeout},
{&xtimer_arr[1],keys_start_timeout,keys_end_timeout},
{&xtimer_arr[2],sound_start_timeout,sound_start_timeout},
{&xtimer_arr[3],lcd_blink_start_timeout,lcd_blink_start_timeout},
};
/******************************************************************************/
void timer4_ISR (void) interrupt 16 using 3//1mS таймер
{
    unsigned char i;
    TF4 = 0;

    for(i=0;i<sizeof(timer_arr)/sizeof(struct virtual_timer);i++){
        if(*timer_arr[i].tick != 0){
            *timer_arr[i].tick -= 1;
            if(*timer_arr[i].tick == 0 && timer_arr[i].end_funct)
                timer_arr[i].end_funct();
        } 
    }
}

/******************************************************************************/
void timer_4_init()
{
    unsigned char i;
    unsigned char xdata *ptr;

    for(i=0;i<(sizeof(timer_arr)/sizeof(struct virtual_timer));i++)
        *timer_arr[i].tick = 0;
    
    i = sizeof(keys);
    ptr = (char*)&keys;
    do{*ptr = 0;ptr++;}while(--i);

    keys_start_timeout();
    
    //ЦЕ ТЕСТОВА ЧАСТИНА
    melody_ptr = &yesterday;
    //timer_arr[2].start_funct();

    timer_arr[3].start_funct();//для поморгування на дисплеї
}
/******************************************************************************/
//i2c порятунок від зависання
void i2c_start_timeout(){*timer_arr[0].tick = 100;}
void i2c_end_timeout()
{
    i2c_reset();
}
/******************************************************************************/

void keys_push(unsigned char key_)
{
    if(keys.index == keys_buf_size)return;
    keys.buf[keys.index++] = key_;
}
unsigned char keys_pop()
{
    unsigned char key_;

    if(keys.index == 0)return 0;
    key_ = keys.buf[--keys.index];
    keys.buf[keys.index] = 0;
    return key_; 
}

 
void keys_start_timeout(){*timer_arr[1].tick = 20;}
void keys_end_timeout()
{
    unsigned char i,new_key;
    static unsigned int last_key;

    new_key = ~io;
    if(new_key !=0x00){
        
        i = 0;
        while(!(new_key & (1<<i))){//пошук першої одиниці
            i++;                
        }
        new_key = i + 1;//код клавіші від 1 до 8;
         
        if(new_key != last_key){
            keys_push(new_key);
            last_key = new_key; 
        }
        else{
            if(key_temp.first < key_set.first){
                key_temp.first++;
            }
            else{
                if(++key_temp.all_next == key_set.all_next){
                    key_temp.all_next = 0;
                    keys_push(new_key);
                }
            }                
        }
    }
    else{
        last_key = 0;
        key_temp.first = 0;
        key_temp.all_next = 0;
    }

    keys_start_timeout();
}
/******************************************************************************/
xdata unsigned char melody_tact = 0;

void sound_start_timeout()
{
    unsigned char back_sfr;
    unsigned char nota_;
    
    back_sfr = SFRPAGE;

    switch(melody_tact){
        case 0:
            //пауза між кожною нотою
            SFRPAGE = CONFIG_PAGE;
            sound = 1;
            SFRPAGE = PCA0_PAGE;
            clr_bit(PCA0CN,CR);//зупинити звуковий таймер
            SFRPAGE = back_sfr;
            *timer_arr[2].tick = 20;
            melody_tact = 1;
            break;        
        default:
            nota_ = melody_ptr->nota;
            switch(nota_){
                    case 255: 
                        SFRPAGE = CONFIG_PAGE;
                        sound = 1;         //вимкнути звук
                        SFRPAGE = PCA0_PAGE;
                        clr_bit(PCA0CN,CR);//зупинити звуковий таймер
                        SFRPAGE = back_sfr;
                        melody_ptr++;
                        return;
                    case 128:     
                        //pause
                        SFRPAGE = CONFIG_PAGE;
                        sound = 1;
                        SFRPAGE = PCA0_PAGE;
                        clr_bit(PCA0CN,CR);//зупинити звуковий таймер
                        SFRPAGE = back_sfr;
                        break;
                    default:
                        ch0_value = nota_;
                        SFRPAGE = PCA0_PAGE;
                        set_bit(PCA0CN,CR);//зупустити звуковий таймер
                        SFRPAGE = back_sfr;
            }    
            *timer_arr[2].tick = melody_ptr->duration;
            melody_ptr++;
            melody_tact = 0;
    }    
}
/******************************************************************************/
xdata unsigned char lcd_blink_flag;
void lcd_blink_start_timeout()
{
    if(lcd_blink_flag){
        *timer_arr[3].tick = 100;
        lcd_blink_flag = 0;
    }
    else{
        *timer_arr[3].tick = 400;
        lcd_blink_flag = 1;
    }    
}
/******************************************************************************/



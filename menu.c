#include <c8051f120.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <intrins.h>
#include <ctype.h>
#include <lcd.h>

#include "definition.h"
#include "macro.h"
#include "timer_4.h"
#include "timer_3.h"
#include "i2c.h"
#include "spi.h"
#include "uart_0.h"
#include "spec_func.h"
#include "arrays.h"
#include "scratch.h"

#define left        1
#define up          2
#define down        3
#define right       4
#define menu_key    5
#define enter       6
#define SS          7
#define PP          8


bit time();
bit time_back();
bit daten();
bit daten_back();
bit calibr();
bit calibr_back();
bit week_day();
bit rs232_bit_rate();
bit rs232_bit_rate_back();
//bit printer();
bit printer1();
bit start_mts0();
bit start_mts1();
bit start_mts2();
bit start_regulator180();
bit stop_regulator180();
bit adc_def_func();
bit adc_def_off_func();
bit Ki_def_func();

//один пункт підменю
typedef code struct submenu{
    char code*format;
    const bit(*pre_funct)();
    const bit(*post_funct)();
    char code *changeful;   //0-тільки відображується; 1-модифікується; 2-виконати пост-функцію по натисканню кнопки ввод;
    unsigned char xdata*number;
    unsigned char code*type;//молодша тетрада 0-нема числа 1-char;2-int;3-long;4-double; старша тетрада 0-unsigned; 1-signed;
    unsigned char code*frac;//число знаків після коми
    struct submenu code*next;//вказує на наступний пункт меню - гілка дeрева
    unsigned char code*size;//розмір натупного меню
};

typedef struct Umin_relay{
    unsigned char Ustart; 
    unsigned char du_dt;
    unsigned char Uon; 
    unsigned char Uoff;
    unsigned char Utol;
    unsigned char Utol_off;
    unsigned char Ulim;
};

typedef struct trv_027{
    unsigned char U;
    unsigned char du_dt;
    unsigned char du_dt_;
	unsigned int  Uoff;
    unsigned int  Uon;
	unsigned char Utol;
	unsigned int  Utest;
	unsigned int  UshG;
	unsigned char UtG;
	unsigned int  UshB;
	unsigned char UtB;
};

typedef struct c_052{
    unsigned char   U,
                    du_dt;
    unsigned int    Umin,
                    Umax;
    unsigned char   Uwork;
    unsigned int    Umin4,
                    Umax4;
};

typedef struct don_12x{
    unsigned char U_1281,
                  du_dt_1281,
                  Uend_1281;
    unsigned int  Umax_1281,
                  Umin_1281;
};

code unsigned int adc_scale_def[16]={2000,100,1000,1000,1000,1000,1000,1000,10000,10000,10000,2000,1000,1000,100,100};

//всі змінні описуються тут
xdata unsigned int rs232_timer1_reload;
xdata unsigned int adc_scale[16];
xdata int          adc_offset[16];
xdata unsigned int Tdiscrete180;
xdata unsigned int Tdiscrete50;
xdata unsigned int  Kir,
                    Kiu;
xdata signed int du_dt_0_mts0,
                 du_dt_0_mts1,
                 du_dt_0_mts2;//8
xdata struct Umin_relay b_2450_033;
xdata struct Umin_relay EAU_2_11;
xdata struct Umin_relay EAU_4_12;
xdata struct Umin_relay EAU_11_13;//26
xdata unsigned char Ustart_2450_33_t,
                   du_dt_2450_33_t;
xdata unsigned int Udest_2450_33_t;
xdata unsigned char Udelta_2450_33_t,
                   pause_2450_33_t;
xdata unsigned char U_212,
                    dU_dt_212,
                    U_dest212,
                    T_212,
                    I_212,
                    I_212_1,
                    U_212_06,
                    dU_dt_212_06,
                    U_dest212_06,
                    U_212_min,
                    U_212_max;
xdata struct c_052 cc052[10];
xdata unsigned char U_056;
xdata unsigned int Ir0,
                    Ir55,
                    Iv0,
                    Iv55;//56
xdata unsigned char Ir0_tol_old,//старі були замінені на двобайтові
                    Ir55_tol_old,
                    Iv0_tol_old,
                    Iv55_tol_old;
xdata struct trv_027 trv_[9];
xdata struct don_12x don_12_[2];

xdata unsigned int Ir0_tol,
                    Ir55_tol,
                    Iv0_tol,
                    Iv55_tol;

xdata unsigned char reserv[892];
xdata unsigned char ext_ram_buf[1024];//звідси ведеться запис в зов пам’ять
//УВАГА!!! у ВИПАДКУ АЛЬТЕРНАТИВНОЇ ОБРОБКИ ДАНИХ ФУНКЦІЯМИ pre_funct І post_funct
//для коректної роботи функцій переміщення курсора
//необхідно розміщувати формат в меню а у фунціях використовувати їх адресу

code char trv0[]={"Полное тестирование "};
code char trv1[]={"Регулир. напряжения "};
code char trv2[]={"Регул. тока генерат."};
code char trv3[]={"Регул. тока батареи "};
code char trv4[]={"Uначальное  %03.0f B   "};
code char trv5[]={"Скорость   %03.1f B/ceк"};
code char trv6[]={"ЦАП шунта  %02.0f мВ/cек"};

code char trv7[]={"Uсрабат.     %05.1f B"};
code char trv8[]={"Uотпускания  %05.1f B"};
code char trv9[]={"Допуск       %03.1f B  "};

code char trv10[]={"Uтестовое    %05.1f B"};
code char trv11[]={"Uшунта       %03.0f мB "};
code char trv12[]={"Допуск Uшунта  %02.0f мB"};

code char trv13[]={"Uтестовое    %05.1f B"};
code char trv14[]={"Uшунта       %03.0f мB "};
code char trv15[]={"Допуск Uшунта  %02.0f мB"};

code struct submenu menu_2460_027_28[]={
{trv13,  0,0,1,&trv_[8].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[8].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[8].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_18[]={
{trv10,  0,0,1,&trv_[8].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[8].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[8].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_08[]={
{trv7,  0,0,1,&trv_[8].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[8].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[8].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0278[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_08, sizeof(menu_2460_027_08)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_18, sizeof(menu_2460_027_18)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_28, sizeof(menu_2460_027_28)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[8].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[8].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[8].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_27[]={
{trv13,  0,0,1,&trv_[7].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[7].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[7].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_17[]={
{trv10,  0,0,1,&trv_[7].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[7].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[7].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_07[]={
{trv7,  0,0,1,&trv_[7].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[7].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[7].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0277[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_07, sizeof(menu_2460_027_07)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_17, sizeof(menu_2460_027_17)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_27, sizeof(menu_2460_027_27)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[7].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[7].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[7].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_26[]={
{trv13,  0,0,1,&trv_[6].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[6].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[6].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_16[]={
{trv10,  0,0,1,&trv_[6].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[6].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[6].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_06[]={
{trv7,  0,0,1,&trv_[6].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[6].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[6].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0276[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_06, sizeof(menu_2460_027_06)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_16, sizeof(menu_2460_027_16)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_26, sizeof(menu_2460_027_26)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[6].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[6].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[6].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_25[]={
{trv13,  0,0,1,&trv_[5].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[5].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[5].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_15[]={
{trv10,  0,0,1,&trv_[5].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[5].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[5].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_05[]={
{trv7,  0,0,1,&trv_[5].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[5].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[5].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0275[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_05, sizeof(menu_2460_027_05)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_15, sizeof(menu_2460_027_15)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_25, sizeof(menu_2460_027_25)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[5].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[5].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[5].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_24[]={
{trv13,  0,0,1,&trv_[4].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[4].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[4].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_14[]={
{trv10,  0,0,1,&trv_[4].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[4].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[4].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_04[]={
{trv7,  0,0,1,&trv_[4].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[4].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[4].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0274[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_04, sizeof(menu_2460_027_04)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_14, sizeof(menu_2460_027_14)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_24, sizeof(menu_2460_027_24)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[4].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[4].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[4].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_23[]={
{trv13,  0,0,1,&trv_[3].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[3].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[3].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_13[]={
{trv10,  0,0,1,&trv_[3].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[3].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[3].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_03[]={
{trv7,  0,0,1,&trv_[3].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[3].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[3].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0273[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_03, sizeof(menu_2460_027_03)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_13, sizeof(menu_2460_027_13)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_23, sizeof(menu_2460_027_23)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[3].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[3].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[3].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_22[]={
{trv13,  0,0,1,&trv_[2].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[2].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[2].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_12[]={
{trv10,  0,0,1,&trv_[2].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[2].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[2].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_02[]={
{trv7,  0,0,1,&trv_[2].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[2].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[2].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0272[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_02, sizeof(menu_2460_027_02)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_12, sizeof(menu_2460_027_12)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_22, sizeof(menu_2460_027_22)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[1].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[1].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[1].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_21[]={
{trv13,  0,0,1,&trv_[1].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[1].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[1].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_11[]={
{trv10,  0,0,1,&trv_[1].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[1].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[1].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_01[]={
{trv7,  0,0,1,&trv_[1].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[1].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[1].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0271[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_01, sizeof(menu_2460_027_01)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_11, sizeof(menu_2460_027_11)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_21, sizeof(menu_2460_027_21)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[1].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[1].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[1].du_dt_,0x01,0,0,0},
};

code struct submenu menu_2460_027_20[]={
{trv13,  0,0,1,&trv_[0].Utest,0x02,1,0,0},
{trv14,  0,0,1,&trv_[0].UshB,0x02,0,0,0},
{trv15,  0,0,1,&trv_[0].UtB,0x01,0,0,0},
};

code struct submenu menu_2460_027_10[]={
{trv10,  0,0,1,&trv_[0].Utest,0x02,1,0,0},
{trv11,  0,0,1,&trv_[0].UshG,0x02,0,0,0},
{trv12,  0,0,1,&trv_[0].UtG,0x01,0,0,0},
};

code struct submenu menu_2460_027_00[]={
{trv7,  0,0,1,&trv_[0].Uoff,0x02,1,0,0},
//{trv8,  0,0,1,&trv_[0].Uon,0x02,1,0,0},
{trv9,  0,0,1,&trv_[0].Utol,0x01,1,0,0},
};

code struct submenu menu_2460_0270[]={
{trv0,0,0,0,0,0,0,0,0},
{trv1,0,0,0,0,0,0,menu_2460_027_00, sizeof(menu_2460_027_00)/sizeof(struct submenu)-1},
{trv2,0,0,0,0,0,0,menu_2460_027_10, sizeof(menu_2460_027_10)/sizeof(struct submenu)-1},
{trv3,0,0,0,0,0,0,menu_2460_027_20, sizeof(menu_2460_027_20)/sizeof(struct submenu)-1},
{trv4,0,0,1,&trv_[0].U,0x01,0,0,0},
{trv5,0,0,1,&trv_[0].du_dt,0x01,1,0,0},
{trv6,0,0,1,&trv_[0].du_dt_,0x01,0,0,0},
};

code struct submenu *ptr_027[]={
&menu_2460_0270,
&menu_2460_0271,
&menu_2460_0272,
&menu_2460_0273,
&menu_2460_0274,
&menu_2460_0275,
&menu_2460_0276,
&menu_2460_0277,
&menu_2460_0278,
};

code struct submenu menu_2460_027[]={
{"Блок 2460.027/01    ",0,0,0,0,0,0,menu_2460_0270,sizeof(menu_2460_0270)/sizeof(struct submenu)-1},
{"Блок 2460.027/02    ",0,0,0,0,0,0,menu_2460_0271,sizeof(menu_2460_0271)/sizeof(struct submenu)-1},
{"Блок 2460.027/03    ",0,0,0,0,0,0,menu_2460_0272,sizeof(menu_2460_0272)/sizeof(struct submenu)-1},
{"Блок 2460.027/04    ",0,0,0,0,0,0,menu_2460_0273,sizeof(menu_2460_0273)/sizeof(struct submenu)-1},
{"Блок 2460.027/05    ",0,0,0,0,0,0,menu_2460_0274,sizeof(menu_2460_0274)/sizeof(struct submenu)-1},
{"Блок 2460.027/08    ",0,0,0,0,0,0,menu_2460_0275,sizeof(menu_2460_0275)/sizeof(struct submenu)-1},
{"Блок 2460.027/09    ",0,0,0,0,0,0,menu_2460_0276,sizeof(menu_2460_0276)/sizeof(struct submenu)-1},
{"Блок 2460.027/10    ",0,0,0,0,0,0,menu_2460_0277,sizeof(menu_2460_0277)/sizeof(struct submenu)-1},
{"Блок 2460.027/11    ",0,0,0,0,0,0,menu_2460_0278,sizeof(menu_2460_0278)/sizeof(struct submenu)-1},
};

code char don0[]={"Полное тестирование "};
code char don1[]={"Прямое прохождение  "};
code char don2[]={"Обратное прохождение"};
code char don3[]={"Uначальное  %03.0f B   "};
code char don4[]={"Скорость   %03.1f B/ceк"};
code char don5[]={"Uконечное  %03.0f B    "};
code char don6[]={"Umax.допуск %05.1f B "};
code char don7[]={"Umin.допуск %05.1f B "};

code struct submenu menu_2460_1281[]={
{don0,0,0,0,0,0,0,0,0},
{don1,0,0,0,0,0,0,0,0},
{don2,0,0,0,0,0,0,0,0},
{don3,0,0,1,&don_12_[1].U_1281,0x01,0,0,0},
{don4,0,0,1,&don_12_[1].du_dt_1281,0x01,1,0,0},
{don5,0,0,1,&don_12_[1].Uend_1281,0x01,0,0,0},
{don6,0,0,1,&don_12_[1].Umax_1281,0x02,1,0,0},
{don7,0,0,1,&don_12_[1].Umin_1281,0x02,1,0,0},
};

code struct submenu menu_2460_1280[]={
{don0,0,0,0,0,0,0,0,0},
{don1,0,0,0,0,0,0,0,0},
{don2,0,0,0,0,0,0,0,0},
{don3,0,0,1,&don_12_[0].U_1281,0x01,0,0,0},
{don4,0,0,1,&don_12_[0].du_dt_1281,0x01,1,0,0},
{don5,0,0,1,&don_12_[0].Uend_1281,0x01,0,0,0},
{don6,0,0,1,&don_12_[0].Umax_1281,0x02,1,0,0},
{don7,0,0,1,&don_12_[0].Umin_1281,0x02,1,0,0},
};

code struct submenu *ptr_128[]={
&menu_2460_1280,
&menu_2460_1281,
};

code struct submenu menu_2460_128[]={
{"Блок 2460.121       ",0,0,0,0,0,0,menu_2460_1280,sizeof(menu_2460_1280)/sizeof(struct submenu)-1},
{"Блок 2460.126       ",0,0,0,0,0,0,menu_2460_1281,sizeof(menu_2460_1281)/sizeof(struct submenu)-1},
};

code struct submenu menu_ki_default[]={
{"Восстан.  умолчания ",0,Ki_def_func,2,0,0,0,0,0},
};

code struct submenu menu_2450_056_k[]={
{"K.ус.ОУ  Ir  %05.2f  ",0,0,1,&Kir,0x02,2,0,0},
{"K.ус.OУ  Iu %06.2f  ",0,0,1,&Kiu,0x02,2,0,0},
{"К.ус.ОУ по умолчанию",0,0,0,0,0,0,menu_ki_default,sizeof(menu_ki_default)/sizeof(struct submenu)-1},
};

code struct submenu menu_2450_056_3[]={
{"Iu при +55С %06.3fмА",0,0,1,&Iv55,0x02,3,0,0},
{"Допуск      %05.3fмА ",0,0,1,&Iv55_tol,0x02,3,0,0},
};
code struct submenu menu_2450_056_2[]={
{"Iu при 0С  %06.3f мА",0,0,1,&Iv0,0x02,3,0,0},
{"Допуск     %05.3f мА ",0,0,1,&Iv0_tol,0x02,3,0,0},
};
code struct submenu menu_2450_056_1[]={
{"Ir при +55С %06.3fмА",0,0,1,&Ir55,0x02,3,0,0},
{"Допуск       %05.3fмА",0,0,1,&Ir55_tol,0x02,3,0,0},
};
code struct submenu menu_2450_056_0[]={
{"Ir при 0С  %06.3f мА",0,0,1,&Ir0,0x02,3,0,0},
{"Допуск     %05.3f мА ",0,0,1,&Ir0_tol,0x02,3,0,0},
};

code struct submenu menu_2450_056[]={
{"Полное тестирование ",0,0,0,0,0,0,0,0},
{"Ir при 0С           ",0,0,0,0,0,0,menu_2450_056_0, sizeof(menu_2450_056_0)/sizeof(struct submenu)-1},
{"Ir при +55С         ",0,0,0,0,0,0,menu_2450_056_1, sizeof(menu_2450_056_1)/sizeof(struct submenu)-1},
{"Iu при 0С           ",0,0,0,0,0,0,menu_2450_056_2, sizeof(menu_2450_056_2)/sizeof(struct submenu)-1},
{"Iu при +55С         ",0,0,0,0,0,0,menu_2450_056_3, sizeof(menu_2450_056_3)/sizeof(struct submenu)-1},
{"Раб.напряжение %03.0f B",0,0,1,&U_056,0x01,0,0,0},
{"Корректировка усил. ",0,0,0,0,0,0,menu_2450_056_k, sizeof(menu_2450_056_k)/sizeof(struct submenu)-1},
};

code char c052_0[]={"Полное тестирование "};
code char c052_1[]={"Напряжение срабатыв."};
code char c052_2[]={"Возврат реле        "};
code char c052_3[]={"Контроль работоспос."};
code char c052_4[]={"Uсраб. при вкл. 50Ом"};
code char c052_5[]={"Uначальное %03.0f B    "};
code char c052_6[]={"Скорость   %03.1f B/ceк"};

code char c052_7[]={"Uсраб.min. %05.1f B  "};
code char c052_8[]={"Uсраб.max. %05.1f B  "};
code char c052_9[]={"Раб.напряжение %03.0f B"};
code char c052_10[]={"Uсраб.min. %05.1f B  "};
code char c052_11[]={"Uсраб.max. %05.1f B  "};

code struct submenu menu_2450_052_29[]={
{&c052_10,0,0,1,&cc052[9].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[9].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_19[]={
{&c052_9,0,0,1,&cc052[9].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_09[]={
{&c052_7,0,0,1,&cc052[9].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[9].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0529[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_09, sizeof(menu_2450_052_09)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_19, sizeof(menu_2450_052_19)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_29, sizeof(menu_2450_052_29)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[9].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[9].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_28[]={
{&c052_10,0,0,1,&cc052[8].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[8].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_18[]={
{&c052_9,0,0,1,&cc052[8].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_08[]={
{&c052_7,0,0,1,&cc052[8].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[8].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0528[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_08, sizeof(menu_2450_052_08)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_18, sizeof(menu_2450_052_18)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_28, sizeof(menu_2450_052_28)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[8].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[8].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_27[]={
{&c052_10,0,0,1,&cc052[7].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[7].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_17[]={
{&c052_9,0,0,1,&cc052[7].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_07[]={
{&c052_7,0,0,1,&cc052[7].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[7].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0527[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_07, sizeof(menu_2450_052_07)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_17, sizeof(menu_2450_052_17)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_27, sizeof(menu_2450_052_27)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[7].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[7].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_26[]={
{&c052_10,0,0,1,&cc052[6].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[6].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_16[]={
{&c052_9,0,0,1,&cc052[6].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_06[]={
{&c052_7,0,0,1,&cc052[6].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[6].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0526[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_06, sizeof(menu_2450_052_06)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_16, sizeof(menu_2450_052_16)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_26, sizeof(menu_2450_052_26)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[6].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[6].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_25[]={
{&c052_10,0,0,1,&cc052[5].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[5].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_15[]={
{&c052_9,0,0,1,&cc052[5].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_05[]={
{&c052_7,0,0,1,&cc052[5].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[5].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0525[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_05, sizeof(menu_2450_052_05)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_15, sizeof(menu_2450_052_15)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_25, sizeof(menu_2450_052_25)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[5].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[5].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_24[]={
{&c052_10,0,0,1,&cc052[4].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[4].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_14[]={
{&c052_9,0,0,1,&cc052[3].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_04[]={
{&c052_7,0,0,1,&cc052[4].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[4].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0524[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_04, sizeof(menu_2450_052_04)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_14, sizeof(menu_2450_052_14)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_24, sizeof(menu_2450_052_24)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[4].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[4].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_23[]={
{&c052_10,0,0,1,&cc052[3].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[3].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_13[]={
{&c052_9,0,0,1,&cc052[3].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_03[]={
{&c052_7,0,0,1,&cc052[3].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[3].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0523[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_03, sizeof(menu_2450_052_03)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_13, sizeof(menu_2450_052_13)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_23, sizeof(menu_2450_052_23)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[3].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[3].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_22[]={
{&c052_10,0,0,1,&cc052[2].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[2].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_12[]={
{&c052_9,0,0,1,&cc052[2].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_02[]={
{&c052_7,0,0,1,&cc052[2].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[2].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0522[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_02, sizeof(menu_2450_052_02)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_12, sizeof(menu_2450_052_12)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_22, sizeof(menu_2450_052_22)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[2].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[2].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_21[]={
{&c052_10,0,0,1,&cc052[1].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[1].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_11[]={
{&c052_9,0,0,1,&cc052[1].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_01[]={
{&c052_7,0,0,1,&cc052[1].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[1].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0521[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_01, sizeof(menu_2450_052_01)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_11, sizeof(menu_2450_052_11)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_21, sizeof(menu_2450_052_21)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[1].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[1].du_dt,0x01,1,0,0},
};

code struct submenu menu_2450_052_20[]={
{&c052_10,0,0,1,&cc052[0].Umin4,0x02,1,0,0},
{&c052_11,0,0,1,&cc052[0].Umax4,0x02,1,0,0},
};
code struct submenu menu_2450_052_10[]={
{&c052_9,0,0,1,&cc052[0].Uwork,0x01,0,0,0},
};
code struct submenu menu_2450_052_00[]={
{&c052_7,0,0,1,&cc052[0].Umin,0x02,1,0,0},
{&c052_8,0,0,1,&cc052[0].Umax,0x02,1,0,0},
};

code struct submenu menu_2450_0520[]={
{&c052_0,0,0,0,0,0,0,0,0},
{&c052_1,0,0,0,0,0,0,menu_2450_052_00, sizeof(menu_2450_052_00)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_3,0,0,0,0,0,0,menu_2450_052_10, sizeof(menu_2450_052_10)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_4,0,0,0,0,0,0,menu_2450_052_20, sizeof(menu_2450_052_20)/sizeof(struct submenu)-1},
{&c052_2,0,0,0,0,0,0,0,0},
{&c052_5,0,0,1,&cc052[0].U,0x01,0,0,0},
{&c052_6,0,0,1,&cc052[0].du_dt,0x01,1,0,0},
};

code struct submenu *ptr_052[]={
&menu_2450_0520,
&menu_2450_0521,
&menu_2450_0522,
&menu_2450_0523,
&menu_2450_0524,
&menu_2450_0525,
&menu_2450_0526,
&menu_2450_0527,
&menu_2450_0528,
&menu_2450_0529,
};

code struct submenu menu_2450_052[]={
{"Блок 2450.052/01    ",0,0,0,0,0,0,menu_2450_0520,sizeof(menu_2450_0520)/sizeof(struct submenu)-1},
{"Блок 2450.052/02    ",0,0,0,0,0,0,menu_2450_0521,sizeof(menu_2450_0521)/sizeof(struct submenu)-1},
{"Блок 2450.052/03    ",0,0,0,0,0,0,menu_2450_0522,sizeof(menu_2450_0522)/sizeof(struct submenu)-1},
{"Блок 2450.052/04    ",0,0,0,0,0,0,menu_2450_0523,sizeof(menu_2450_0523)/sizeof(struct submenu)-1},
{"Блок 2450.052/05    ",0,0,0,0,0,0,menu_2450_0524,sizeof(menu_2450_0524)/sizeof(struct submenu)-1},
{"Блок 2450.052/06    ",0,0,0,0,0,0,menu_2450_0525,sizeof(menu_2450_0525)/sizeof(struct submenu)-1},
{"Блок 2450.052/07    ",0,0,0,0,0,0,menu_2450_0526,sizeof(menu_2450_0526)/sizeof(struct submenu)-1},
{"Блок 2450.052/08    ",0,0,0,0,0,0,menu_2450_0527,sizeof(menu_2450_0527)/sizeof(struct submenu)-1},
{"Блок 2450.052/09    ",0,0,0,0,0,0,menu_2450_0528,sizeof(menu_2450_0528)/sizeof(struct submenu)-1},
{"Блок 2450.052/10    ",0,0,0,0,0,0,menu_2450_0529,sizeof(menu_2450_0529)/sizeof(struct submenu)-1},
};

code struct submenu menu_2450_212_2[]={
{"Uначальное  %03.0f B   ",0,0,1,&U_212_06,0x01,0,0,0},
{"Скорость   %03.1f B/ceк",0,0,1,&dU_dt_212_06,0x01,1,0,0},
{"Uконечное  %03.0f B    ",0,0,1,&U_dest212_06,0x01,0,0,0},
{"Uсраб.min. %03.0f B    ",0,0,1,&U_212_min,0x01,0,0,0},
{"Uсраб.max. %03.0f B    ",0,0,1,&U_212_max,0x01,0,0,0},
};
code struct submenu menu_2450_212_1[]={
{"Порог тока %03.0f мА  ",0,0,1,&I_212_1,0x01,0,0,0},
};

code struct submenu menu_2450_212_0[]={
{"Порог тока %03.0f мкА ",0,0,1,&I_212,0x01,0,0,0},
};

code struct submenu menu_2450_212[]={
{"Полное тестирование ",0,0,0,0,0,0,0,0},
{"Ток утечки диода n2 ",0,0,0,0,0,0,menu_2450_212_0, sizeof(menu_2450_212_0)/sizeof(struct submenu)-1},
{"Измер. I потребления",0,0,0,0,0,0,menu_2450_212_1, sizeof(menu_2450_212_1)/sizeof(struct submenu)-1},
{"Втягивание реле     ",0,0,0,0,0,0,menu_2450_212_2, sizeof(menu_2450_212_2)/sizeof(struct submenu)-1},
{"Uначальное  %03.0f B   ",0,0,1,&U_212,0x01,0,0,0},
{"Скорость   %03.1f B/ceк",0,0,1,&dU_dt_212,0x01,1,0,0},
{"Uконечное  %03.0f B    ",0,0,1,&U_dest212,0x01,0,0,0},
{"Выдержка   %02.0f сек  ",0,0,1,&T_212,0x01,0,0,0},
};

code struct submenu menu_ml520[]={
{"Полное тестирование ",0,0,0,0,0,0,0,0},
{"Обрыв цепи датчиков ",0,0,0,0,0,0,0,0},
{"Нормальная работа   ",0,0,0,0,0,0,0,0},
{"Частичная закоротка ",0,0,0,0,0,0,0,0},
{"Перегрев датчика(ов)",0,0,0,0,0,0,0,0},
};

code struct submenu menu_Tset_out[]={
//{"Tдискрет. 50  %04.2f c",0,0,1,&Tdiscrete50,0x02,2,0,0},
{"Tдискрет.170  %04.2f c",0,0,1,&Tdiscrete180,0x02,2,0,0},
};
code struct submenu menu_Uset_out180[]={
{"Uзаданное    %05.1f B",0,0,1,&Uset_out180,0x02,1,0,0},
};

code struct submenu menu_Uset_out50[]={
{"Uзаданное     %04.1f B",0,0,1,&Uset_out50,0x02,1,0,0},
};

code struct submenu menu_Uset_out[]={
//{"Источник 50-70В max.",0,0,0,0,0,0,menu_Uset_out50,sizeof(menu_Uset_out50)/sizeof(struct submenu)-1},
{"Источник 50-70В max.",0,0,0,0,0,0,0,0},
{"Источник 170 В max. ",0,0,0,0,0,0,menu_Uset_out180,sizeof(menu_Uset_out180)/sizeof(struct submenu)-1},
{"Параметры регуляции ",0,0,0,0,0,0,menu_Tset_out,sizeof(menu_Tset_out)/sizeof(struct submenu)-1},
//{"Tдискрет     %05.2f с",0,0,1,&Tdiscrete180,0x02,2,0,0},
//{" ЗАПУСК  РЕГУЛЯТОРА ", 0,start_regulator180,2,0,0,0,0,0},
//{" ОСТАНОВ РЕГУЛЯТОРА ", 0,stop_regulator180,2,0,0,0,0,0},
};

code struct submenu menu2450_33_t[]={
{"Uначальное     %03.0f B",0,0,1,&Ustart_2450_33_t, 0x01,0,0,0},
{"Cкорость   %03.1f В/сек",0,0,1,&du_dt_2450_33_t, 0x01,1,0,0},
{"Uопускания   %05.1f B",  0,0,1,&Udest_2450_33_t, 0x02,1,0,0},
{"дельтаU нач.    %02.0f %%",0,0,1,&Udelta_2450_33_t, 0x01,0,0,0},
{"Пауза броска  %02.0f c",0,0,1,&pause_2450_33_t, 0x01,0,0,0},
};

code char Umin_m0[]={"Полное тестирование "};
code char Umin_m1[]={"Напряж. срабатывания"};
code char Umin_m2[]={"Напряжен. отпускания"};
code char Umin_m3[]={"Uначальное    %03.0f B "};
code char Umin_m4[]={"Cкорость   %03.1f В/сек"};
code char Umin_m5[]={"Допуск        %03.1f B "};
code char Umin_m6[]={"Огран.напряж. %03.0f B "};
code char Umin_m7[]={"Uотпускания   %03.0f B "};
code char Umin_m8[]={"Uсрабат.      %03.0f B "};

code struct submenu menu_eau_11_13_1[]={
{&Umin_m7,0,0,1,&EAU_11_13.Uoff,  0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_11_13.Utol_off,  0x01,1,0,0},
};
code struct submenu menu_eau_11_13_0[]={
{&Umin_m8,0,0,1,&EAU_11_13.Uon,   0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_11_13.Utol,  0x01,1,0,0},
};
code struct submenu menu_eau_11_13[]={
{&Umin_m0,0,0,0,0,0,0,0,0},
{&Umin_m1,0,0,0,0,0,0,menu_eau_11_13_0, sizeof(menu_eau_11_13_0)/sizeof(struct submenu)-1},
{&Umin_m2,0,0,0,0,0,0,menu_eau_11_13_1, sizeof(menu_eau_11_13_1)/sizeof(struct submenu)-1},
{&Umin_m3,0,0,1,&EAU_11_13.Ustart,0x01,0,0,0},
{&Umin_m4,0,0,1,&EAU_11_13.du_dt, 0x01,1,0,0},
{&Umin_m6,0,0,1,&EAU_11_13.Ulim,  0x01,0,0,0},
};

code struct submenu menu_eau_4_12_1[]={
{&Umin_m7,0,0,1,&EAU_4_12.Uoff,  0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_4_12.Utol_off,  0x01,1,0,0},
};
code struct submenu menu_eau_4_12_0[]={
{&Umin_m8,0,0,1,&EAU_4_12.Uon,   0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_4_12.Utol,  0x01,1,0,0},
};
code struct submenu menu_eau_4_12[]={
{&Umin_m0,0,0,0,0,0,0,0,0},
{&Umin_m1,0,0,0,0,0,0,menu_eau_4_12_0, sizeof(menu_eau_4_12_0)/sizeof(struct submenu)-1},
{&Umin_m2,0,0,0,0,0,0,menu_eau_4_12_1, sizeof(menu_eau_4_12_1)/sizeof(struct submenu)-1},
{&Umin_m3,0,0,1,&EAU_4_12.Ustart,0x01,0,0,0},
{&Umin_m4,0,0,1,&EAU_4_12.du_dt, 0x01,1,0,0},
{&Umin_m6,0,0,1,&EAU_4_12.Ulim,  0x01,0,0,0},
};

code struct submenu menu_eau_2_11_1[]={
{&Umin_m7,0,0,1,&EAU_2_11.Uoff,  0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_2_11.Utol_off,  0x01,1,0,0},
};
code struct submenu menu_eau_2_11_0[]={
{&Umin_m8,0,0,1,&EAU_2_11.Uon,   0x01,0,0,0},
{&Umin_m5,0,0,1,&EAU_2_11.Utol,  0x01,1,0,0},
};
code struct submenu menu_eau_2_11[]={
{&Umin_m0,0,0,0,0,0,0,0,0},
{&Umin_m1,0,0,0,0,0,0,menu_eau_2_11_0, sizeof(menu_eau_2_11_0)/sizeof(struct submenu)-1},
{&Umin_m2,0,0,0,0,0,0,menu_eau_2_11_1, sizeof(menu_eau_2_11_1)/sizeof(struct submenu)-1},
{&Umin_m3,0,0,1,&EAU_2_11.Ustart,0x01,0,0,0},
{&Umin_m4,0,0,1,&EAU_2_11.du_dt, 0x01,1,0,0},
{&Umin_m6,0,0,1,&EAU_2_11.Ulim,  0x01,0,0,0},
};

code struct submenu menu2450_33_1[]={
{&Umin_m7,0,0,1,&b_2450_033.Uoff,  0x01,0,0,0},
{&Umin_m5,0,0,1,&b_2450_033.Utol_off,  0x01,1,0,0},
};
code struct submenu menu2450_33_0[]={
{&Umin_m8,0,0,1,&b_2450_033.Uon,   0x01,0,0,0},
{&Umin_m5,0,0,1,&b_2450_033.Utol,  0x01,1,0,0},
};
code struct submenu menu2450_33[]={
{&Umin_m0,0,0,0,0,0,0,0,0},
{&Umin_m1,0,0,0,0,0,0,menu2450_33_0, sizeof(menu2450_33_0)/sizeof(struct submenu)-1},
{&Umin_m2,0,0,0,0,0,0,menu2450_33_1, sizeof(menu2450_33_1)/sizeof(struct submenu)-1},
{"Проверка времен     ",0,0,0,0,0,0,menu2450_33_t, sizeof(menu2450_33_t)/sizeof(struct submenu)-1},
{&Umin_m3,0,0,1,&b_2450_033.Ustart,0x01,0,0,0},
{&Umin_m4,0,0,1,&b_2450_033.du_dt, 0x01,1,0,0},
{&Umin_m6,0,0,1,&b_2450_033.Ulim,  0x01,0,0,0},
};

code struct submenu menu_Umin[]={
{"Блок 2450.033       ",0,0,0,0,0,0,menu2450_33,sizeof(menu2450_33)/sizeof(struct submenu)-1},
{"Блок EAU 2/11       ",0,0,0,0,0,0,menu_eau_2_11,sizeof(menu_eau_2_11)/sizeof(struct submenu)-1},
{"Блок EAU 4/12       ",0,0,0,0,0,0,menu_eau_4_12,sizeof(menu_eau_4_12)/sizeof(struct submenu)-1},
{"Блок EAU 11/13      ",0,0,0,0,0,0,menu_eau_11_13,sizeof(menu_eau_11_13)/sizeof(struct submenu)-1},
};

code struct submenu menu5[]={
{"Канал   U0  %6.1f В",0,0,0,&ml140_150.U[0],0x12,1,0},
{"Канал   U1  %6.1f В",0,0,0,&ml140_150.U[1],0x12,1,0},
{"Канал   U2  %6.1f В",0,0,0,&ml140_150.U[2],0x12,1,0},
{"Канал   U3  %6.1f В",0,0,0,&ml140_150.U[3],0x12,1,0},
{"Канал   U4  %6.1f В",0,0,0,&ml140_150.U[4],0x12,1,0},
{"Канал   U5  %6.1f В",0,0,0,&ml140_150.U[5],0x12,1,0},
{"Канал   U6  %6.1f В",0,0,0,&ml140_150.U[6],0x12,1,0},
{"Канал   U7  %6.1f В",0,0,0,&ml140_150.U[7],0x12,1,0},
{"Канал   U8  %6.3f В",0,0,0,&ml140_150.U[8],0x12,3,0},
{"Канал   U9  %6.3f В",0,0,0,&ml140_150.U[9],0x12,3,0},
{"Канал   U10 %6.3f В",0,0,0,&ml140_150.U[10],0x12,3,0},
{"Канал   U11 %6.1f В",0,0,0,&ml140_150.U[11],0x12,1,0},
{"Канал   U12 %6.1f В",0,0,0,&ml140_150.U[12],0x12,1,0},
{"Канал   U13 %6.1f В",0,0,0,&ml140_150.U[13],0x12,1,0},
{"Канал   U14 %6.1f В",0,0,0,&ml140_150.U[14],0x12,1,0},
{"Канал   U15 %6.1f В",0,0,0,&ml140_150.U[15],0x12,1,0},
};

code struct submenu menu_adc_default[]={
{"Коеф. АЦП умолчания ",0,adc_def_func,2,0,0,0,0,0},
{"Смещ. АЦП умолчания ",0,adc_def_off_func,2,0,0,0,0,0},
};

code struct submenu menu_offset_ch[]={
{"Смещение U0  %+04.0f мВ",0,0,1,&adc_offset[0],0x12,0,0},
{"Смещение U1  %+04.0f мВ",0,0,1,&adc_offset[1],0x12,0,0},
{"Смещение U2  %+04.0f мВ",0,0,1,&adc_offset[2],0x12,0,0},
{"Смещение U3  %+04.0f мВ",0,0,1,&adc_offset[3],0x12,0,0},
{"Смещение U4  %+04.0f мВ",0,0,1,&adc_offset[4],0x12,0,0},
{"Смещение U5  %+04.0f мВ",0,0,1,&adc_offset[5],0x12,0,0},
{"Смещение U6  %+04.0f мВ",0,0,1,&adc_offset[6],0x12,0,0},
{"Смещение U7  %+04.0f мВ",0,0,1,&adc_offset[7],0x12,0,0},
{"Смещение U8  %+05.1fмВ",0,0,1,&adc_offset[8],0x12,2,0},
{"Смещение U9  %+05.1fмВ",0,0,1,&adc_offset[9],0x12,2,0},
{"Смещение U10 %+05.1fмВ",0,0,1,&adc_offset[10],0x12,2,0},
{"Смещение U11 %+04.0f мВ",0,0,1,&adc_offset[11],0x12,0,0},
{"Смещение U12 %+04.0f мВ",0,0,1,&adc_offset[12],0x12,0,0},
{"Смещение U13 %+04.0f мВ",0,0,1,&adc_offset[13],0x12,0,0},
{"Смещение U14 %+04.0f мВ",0,0,1,&adc_offset[14],0x12,0,0},
{"Смещение U15 %+04.0f мВ",0,0,1,&adc_offset[15],0x12,0,0},
};

code struct submenu menu_scale[]={
{"Канал 0      %05.2f  ",0,0,1,&adc_scale[0],0x02,2,0},
{"Канал 1      %05.4f",0,0,1,&adc_scale[1],0x02,4,0},
{"Канал 2      %05.2f  ",0,0,1,&adc_scale[2],0x02,2,0},
{"Канал 3      %05.2f  ",0,0,1,&adc_scale[3],0x02,2,0},
{"Канал 4      %05.2f  ",0,0,1,&adc_scale[4],0x02,2,0},
{"Канал 5      %05.2f  ",0,0,1,&adc_scale[5],0x02,2,0},
{"Канал 6      %05.2f  ",0,0,1,&adc_scale[6],0x02,2,0},
{"Канал 7      %05.2f  ",0,0,1,&adc_scale[7],0x02,2,0},
{"Канал 8      %05.4f",0,0,1,&adc_scale[8],0x02,4,0},
{"Канал 9      %05.4f",0,0,1,&adc_scale[9],0x02,4,0},
{"Канал 10     %05.4f",0,0,1,&adc_scale[10],0x02,4,0},
{"Канал 11     %05.2f  ",0,0,1,&adc_scale[11],0x02,2,0},
{"Канал 12     %05.2f  ",0,0,1,&adc_scale[12],0x02,2,0},
{"Канал 13     %05.2f  ",0,0,1,&adc_scale[13],0x02,2,0},
{"Канал 14     %05.2f  ",0,0,1,&adc_scale[14],0x02,2,0},
{"Канал 15     %05.2f  ",0,0,1,&adc_scale[15],0x02,2,0},
};

code struct submenu menu_adc_16[]={
{"К.передачи каналов  ",0,0,0,0,0,0,menu_scale,sizeof(menu_scale)/sizeof(struct submenu)-1},
{"Смещение в каналах  ",0,0,0,0,0,0,menu_offset_ch,sizeof(menu_offset_ch)/sizeof(struct submenu)-1},
{"АЦП по умолчанию    ",0,0,0,0,0,0,menu_adc_default,sizeof(menu_adc_default)/sizeof(struct submenu)-1},
};
/*
code struct submenu menu4_2[]={
{"Cкорость %#+05.1f В/сек",0,0,1,&du_dt_0_mts2,0x12,1,0,0},
{"Запустити МТС2      ",0,start_mts2,2,&du_dt_0_mts2,0,1,0,0},
{"MTS[2].limit %06.0f ",0,0,1,&MTS[2].limit,0x02,0,0},
{"Напруга      %6.1f ",0,0,0,&ml140_150.U[0],0x02,1,0},
};

code struct submenu menu4_1[]={
{"Cкорость %#+05.1f В/сек",0,0,1,&du_dt_0_mts1,0x12,1,0,0},
{"Запустити МТС1      ",0,start_mts1,2,&du_dt_0_mts1,0,1,0,0},
{"MTS[1].limit %06.0f ",0,0,1,&MTS[1].limit,0x02,0,0},
{"Напруга      %6.1f ",0,0,0,&ml140_150.U[2],0x02,1,0},
};

code struct submenu menu4[]={
{"Cкорость %#+05.1f В/сек",0,0,1,&du_dt_0_mts0,0x12,1,0,0},
{"Запустити МТС0      ",0,start_mts0,2,&du_dt_0_mts0,0,1,0,0},
{"MTS[0].limit %06.0f ",0,0,1,&MTS[0].limit,0x02,0,0},
{"Напруга      %6.1f ",0,0,0,&ml140_150.U[1],0x02,1,0},
};
*/

code struct submenu menu3[]={
{"Скорость  %#06.0f бод",rs232_bit_rate, rs232_bit_rate_back,1,0,0,0,0,0},
//{"Напечатать  LOGO    ",0,printer,2,0,0,0,0,0},
{"Печать тест-страницы",0,printer1,2,0,0,0,0,0},
};

code struct submenu menu2[]={
{"Время    %#02d:%#02d:%#02d   ",time, time_back,1,0,0,0,0,0},
{"Дата     %#02d.%#02d.%#02d   ",daten,daten_back,1,0,0,0,0,0},
{"Калибровка   %#+03d ppm",calibr,calibr_back,1,0,0,0,0,0},
{0,week_day,0,0,0,0,0,0,0},
};

code struct submenu menu1[]={
{"Дата и время        ",0,0,0,0,0,0,menu2, sizeof(menu2)/sizeof(struct submenu)-1},
{"Интерфейс RS232C    ",0,0,0,0,0,0,menu3, sizeof(menu3)/sizeof(struct submenu)-1},
};

code struct submenu menu0[]={                 
{"000 Блок 2450 212   ",0,0,0,0,0,0,menu_2450_212,sizeof(menu_2450_212)/sizeof(struct submenu)-1},
{"001 Блок МЛ 520     ",0,0,0,0,0,0,menu_ml520,sizeof(menu_ml520)/sizeof(struct submenu)-1},
{"002 2450.033 / EAU  ",0,0,0,0,0,0,menu_Umin,sizeof(menu_Umin)/sizeof(struct submenu)-1},
{"003 Блоки 2450.052  ",0,0,0,0,0,0,menu_2450_052,sizeof(menu_2450_052)/sizeof(struct submenu)-1},
{"004 Блок 2450 056   ",0,0,0,0,0,0,menu_2450_056,sizeof(menu_2450_056)/sizeof(struct submenu)-1},
{"005 Блоки 2460.027  ",0,0,0,0,0,0,menu_2460_027,sizeof(menu_2460_027)/sizeof(struct submenu)-1},
{"006 Блоки ДОН       ",0,0,0,0,0,0,menu_2460_128,sizeof(menu_2460_128)/sizeof(struct submenu)-1},
{"007 Сист. параметры ",0,0,0,0,0,0,menu1, sizeof(menu1)/sizeof(struct submenu)-1},
{"008 Вход. напряжения",0,0,0,0,0,0,menu5,sizeof(menu5)/sizeof(struct submenu)-1},
{"009 Калибровка АЦП  ",0,0,0,0,0,0,menu_adc_16,sizeof(menu_adc_16)/sizeof(struct submenu)-1},
{"010 Напряжения МТС  ",0,0,0,0,0,0,menu_Uset_out, sizeof(menu_Uset_out)/sizeof(struct submenu)-1},
/*
{"010 МТС 0           ",0,0,0,0,0,0,menu4,sizeof(menu4)/sizeof(struct submenu)-1},
{"011 МТС 1           ",0,0,0,0,0,0,menu4_1,sizeof(menu4_1)/sizeof(struct submenu)-1},
{"012 МТС 2           ",0,0,0,0,0,0,menu4_2,sizeof(menu4_2)/sizeof(struct submenu)-1},
*/
};

//організація стека для повернення назад по меню
struct cell{
    unsigned char sublevel;      //верхній рядок на екрані
    unsigned char lcd_flash_row; //рядок, що моргає
    struct submenu code*menu_ptr;//адреса
    unsigned char menu_size;     //розмір
};

xdata struct cell map;//параметри поточного меню

#define menu_stack_size 16
xdata struct{
    struct cell one_cell[menu_stack_size];
    unsigned char index;
}menu_stack;

xdata struct menu_status{//маю намір вписувати сюди поточний стан меню
    unsigned char input_row;    //рядок де відбувається модифікація
    unsigned char fetch; //0 - звичайний режим 1- необхідно захопити рядок 2- відбулося захоплення рядка
    void (*alt)();//якщо  не дорівнює нулю то дисплей використовується альткрнативною функцією ареса її сюди і записана
}menu_status;

void cpy_xx(unsigned char xdata *xptr_dest, unsigned char xdata *xptr_src, unsigned int count)//копіює 
{
    do{*xptr_dest++ = *xptr_src++;}
    while(--count);
}

void clr_xx(unsigned char xdata *xptr_dest, unsigned char count)//обнуляє память
{
    do{*xptr_dest++ = 0;}
    while(--count);
}

unsigned char menu_push()//повертає одиницю при успішному закиданні в стек і 0 якщо більше нікуди
{
    if(menu_stack.index == menu_stack_size)return 0;
    cpy_xx((char xdata*)&menu_stack.one_cell[menu_stack.index++], (char xdata*)&map, sizeof(map));

    return 1;
}

unsigned char menu_pop()//повертає 1 при успішному вийманні і 0 при порожньому стеку
{
    if(menu_stack.index == 0)return 0;
    cpy_xx((char xdata*)&map, (char xdata*)&menu_stack.one_cell[--menu_stack.index],sizeof(map));
    clr_xx((char xdata*)&menu_stack.one_cell[menu_stack.index],sizeof(map));//очищення памяті
    return 1; 
}

void reload();

sfr16 RCAP2 = 0xca; 
void menu_init()
{
    unsigned char xdata *ptr;
    unsigned char i;
    
    i = sizeof(menu_stack);
    ptr = (char*)&menu_stack;
    do{*ptr = 0;ptr++;}while(--i);
    
    clr_xx((char xdata*)&menu_status,sizeof(menu_status));

    map.sublevel = 0;
    map.lcd_flash_row = 0;
    map.menu_ptr = &menu0;
    map.menu_size = sizeof(menu0)/sizeof(struct submenu)-1;

    flash_read(0x8000, (char*)&rs232_timer1_reload, 1024);
    RCAP2 = rs232_timer1_reload;
    //scratchpad_read(0, (char*)&rs232_timer1_reload, 256);
    /*
    //прочитати із зовнішній і2с пам'яті
    i2cdata.addr = 0;
    i2cdata.count = 56;
    i2cdata.ptr = (char*)&rs232_timer1_reload;
    i2c_func[1] = I2Cgetdata;
    i2c_func[2] = I2Cgetdata;
    i2c_func[3] = I2Cgetdata;
    i2c_func[4] = reload;//оновити регістри
    */
}

bdata unsigned char input_status;

sbit no_refresh = input_status^0;
sbit input      = input_status^1;//модифікація
sbit enter_bit  = input_status^2;
sbit request_for= input_status^3;
sbit was_make   = input_status^4;

double back;
xdata unsigned char view_num;



void menu_KEY(void);
void enter_key(void);
void right_key(void);
void up_key(void);
void down_key(void);
void left_key(void);
void UP();
void DOWN();
void RIGHT();
void LEFT();

void clculate_test()
{
    tst_stage = map.sublevel + map.lcd_flash_row;
    if(tst_stage > map.menu_size)tst_stage -= map.menu_size+1;
    if(tst_stage){
        tst_regime = 1;//зациклитись на одному етапі
        tst_stage--;
    }
    else tst_regime = 0;    
}
 
void processing(void)using 0
{
    unsigned char i,offset,k,key_1;
    bit (*pre_func)(); //вказівник на функцію обробника
    unsigned char type;
    union
    {
        unsigned char uc;
        signed char sc;
        unsigned int ui;
        signed int si;
        unsigned long ul;
        signed long sl;
        double d;
    }num;
    
    if(menu_status.alt){
        menu_status.alt();
        key_1 = keys_pop();
        switch(key_1){
            case SS:
                menu_status.alt = 0;
                clr_bit(EIE2,ET3);
                *timer_arr3[4].tick = 0;
                *timer_arr3[8].tick = 0;//зупинка регулятора
                *timer_arr3[9].tick = 0;//зупинка регулятора
                set_bit(EIE2,ET3);
            
                tst_tact = 0;
                tst_iter = 0;
                tst_stage = 0;
                tst_status = 0;
                MTS[0].status = 2;
                MTS[1].status = 2;
                MTS[2].status = 2;
                MTS[3].status = 2;
                ml140_140.relay16_23 = 0;
                ml140_140.relay8_15 = 0;
                ml140_140.relay0_7 = 0;
                ml140_140.dac_ad5312 = 0;
                ml140_130[0].dac_ad7243 = 0;
                ml140_130[1].dac_ad7243 = 0;
                ml140_130[2].dac_ad7243 = 0;
                ml140_140.dac_ad5312 = 0;
                print_func = 0;
                break;
            case PP:
                if(keys_pop() != PP){
                    clr_bit(EIE2,ET4);
                    keys_push(PP);
                    set_bit(EIE2,ET4);
                }
                break;
            case menu_key:
                MTS[0].value += 0x8000;
                MTS[2].value += 0x8000;
                break;
            case enter:            
                MTS[0].value -= 0x8000;
                MTS[2].value -= 0x8000;
                break;
            case 0:
                break;
            default:
                clr_bit(EIE2,ET4);
                keys_push(key_1);
                set_bit(EIE2,ET4);
                break;
        }
        return;
    }
    if(alarm_was){
        alarm_text();
        return;
    }
    for(i=0;i<4;i++){
        
        if(map.sublevel+i > map.menu_size)offset = map.sublevel+i - map.menu_size-1;
        else offset = map.sublevel+i;
        //якщо в меню меньше ніж чотири рядки
        if(i > map.menu_size){
            lcd_put_str(i,0,"                    ",0);
            continue;
        }

        pre_func = map.menu_ptr[offset].pre_funct;
        if(pre_func == 0 || (pre_func && pre_func()==0)){
            type = (unsigned char)map.menu_ptr[offset].type;//тип числа
            switch(type)
            {
                case 0:
                    sprintf(lcd_str,map.menu_ptr[offset].format);
                    break;
                case 1:
                    //обробка двобайтового цілого з умовно дробовою частиною
                    num.d = *(unsigned char*)map.menu_ptr[offset].number;//саме ціле число
                    type = (unsigned char)map.menu_ptr[offset].frac;//кількість дробових знаків
                    if(type){
                        do num.d /= 10; while(--type);
                    }
                    sprintf(lcd_str,map.menu_ptr[offset].format,num.d);                    
                    break;
                case 2:
                    //обробка двобайтового цілого з умовно дробовою частиною
                    num.d = *(unsigned int*)map.menu_ptr[offset].number;//саме ціле число
                    type = (unsigned char)map.menu_ptr[offset].frac;//кількість дробових знаків
                    if(type){
                        do num.d /= 10; while(--type);
                    }
                    sprintf(lcd_str,map.menu_ptr[offset].format,num.d);
                    break;
                case 4:
                    //обробка double
                    num.d = *(double*)map.menu_ptr[offset].number;//саме ціле число
                    if(_chkfloat_(num.d)>1)num.d = 0;
                    sprintf(lcd_str,map.menu_ptr[offset].format,num.d);
                    break;
                case 0x12:
                    //обробка двобайтового цілого з умовно дробовою частиною зі знаком
                    num.d = *(signed int*)map.menu_ptr[offset].number;//саме ціле число
                    type = (unsigned char)map.menu_ptr[offset].frac;//кількість дробових знаків
                    if(type){
                        do num.d /= 10; while(--type);
                    }
                    sprintf(lcd_str,map.menu_ptr[offset].format,num.d);
                    break;
            }
        }
        else map.menu_ptr[offset].pre_funct();
   
        if((menu_status.fetch == 1) && (offset == menu_status.input_row)){//якщо рядок відповідає і захоплення ще не було - захопити рядок
            lcd_str[21] = 0;//захист від переповнення
            strcpy(lcd_input_str,lcd_str);
            if(lcd_init_crsr_position())menu_status.fetch = 2;
            else menu_status.fetch = 0;//невдача
        }

        if((menu_status.fetch == 2) && (offset == menu_status.input_row)){//якщо йде процеc модифікації
            strcpy(lcd_str,lcd_input_str);//доправити захоплений рядок на підміну
            if(!lcd_blink_flag)lcd_str[lcd_crsr_position] = ' ';
            lcd_put_str(i,0,lcd_str,0);            
        }
        else{
            //виведення одного рядка на дисплей 
            if(i == map.lcd_flash_row){
                if(lcd_blink_flag)lcd_put_str(i,0,lcd_str,0); 
                else lcd_put_str(i,0,"                    ",0);
            }            
            else lcd_put_str(i,0,lcd_str,0);
        }
     }
    

    switch(keys_pop()){
        
        case menu_key: 
            menu_KEY(); 
            break;

        case PP:
            //тут перехід на виконання тестів - записується адреса альтернативної функції
            if(map.menu_ptr == menu2450_33){
                clculate_test();
                if(tst_stage < 2){
                    menu_status.alt = &relay_Umin_test;
                    cpy_xx(&relay_Umin.Ustart, &b_2450_033.Ustart, sizeof(struct relay_umin)-6);
                    
                    relay_Umin.print = 0;
                }
                if(tst_stage == 2)menu_status.alt = &relay_Umin_2450_033_t;
                relay_Umin.test = 0;
            }
            if(map.menu_ptr == menu_eau_2_11){
                clculate_test();
                if(tst_stage < 2){
                    menu_status.alt = &relay_Umin_test;
                    cpy_xx(&relay_Umin.Ustart, &EAU_4_12.Ustart, sizeof(struct relay_umin)-6);
                    relay_Umin.test = 1;
                    relay_Umin.print = 1;
                }
            }
            if(map.menu_ptr == menu_eau_4_12){
                clculate_test();
                if(tst_stage < 2){
                    menu_status.alt = &relay_Umin_test;
                    cpy_xx(&relay_Umin.Ustart, &EAU_4_12.Ustart, sizeof(struct relay_umin)-6);
                    relay_Umin.test = 1;
                    relay_Umin.print = 2;
                }
            }
            if(map.menu_ptr == menu_eau_11_13){
                clculate_test();
                if(tst_stage < 2){
                    menu_status.alt = &relay_Umin_test;
                    cpy_xx(&relay_Umin.Ustart, &EAU_11_13.Ustart, sizeof(struct relay_umin)-6);
                    relay_Umin.test = 1;
                    relay_Umin.print = 3;
                }
            }
            

            if(map.menu_ptr == menu_ml520){
                menu_status.alt = &ml520_test;
                clculate_test();
            }
            if(map.menu_ptr == menu_2450_212){
                clculate_test();
                if(tst_stage < 3)menu_status.alt = &relay_I_load2450_212;
            }
            
            for(k=0;k < (sizeof(ptr_052)/sizeof(struct submenu*));k++){
                if(map.menu_ptr == ptr_052[k]){
                    clculate_test();
                    if(tst_stage < 6)menu_status.alt = &test_2450_052;
                    cpy_xx((char*)&u052, (char*)&cc052[k], sizeof(struct c_052));
                }
            }
            
            if(map.menu_ptr == menu_2450_056){
                clculate_test();
                if(tst_stage < 4)menu_status.alt = &test_02450_056;
            }
            
            for(k=0;k < (sizeof(ptr_128)/sizeof(struct submenu*));k++){
                if(map.menu_ptr == ptr_128[k]){
                    clculate_test();
                    if(tst_stage < 2)menu_status.alt = &test_2460_1281;
                    cpy_xx((char*)&don12_, (char*)&don_12_[k], sizeof(struct don_12x));
                }
            }
            for(k=0;k < (sizeof(ptr_027)/sizeof(struct submenu*));k++){
                if(map.menu_ptr == ptr_027[k]){
                    clculate_test();
                    if(tst_stage < 3)menu_status.alt = &test_2460_027;
                    //cpy_xx(unsigned char xdata *xptr_dest, unsigned char xdata *xptr_src, unsigned int count);
                    cpy_xx((char*)&TRV, (char*)&trv_[k], sizeof(struct trv_027));
                }
            }
            if(map.menu_ptr == menu_Uset_out){
                clculate_test();
                if(tst_stage < 2){
                    if(tst_regime==1)tst_stage = 1;
                    menu_status.alt = &mts_run;
                }
            }
            break;

        case SS:
            menu_status.alt = 0;
            break;

        case enter:
            enter_key();
            break;

        case right:
            right_key();
            break;

        case up:
            up_key();   
            break;

        case down:
            down_key(); 
            break;

        case left:    
            left_key(); 
            break;
    }
}

void enter_key(void)
{
    unsigned int num;
    unsigned char i;
    unsigned char type;
    bit (*post_func)(); //вказівник на функцію обробника

    if(menu_status.fetch == 0){//якщо меню в режимі навігації
        
        i = map.sublevel+map.lcd_flash_row;//тобто номер піпункту меню 0..map.menu_size-1
        
        if(i > map.menu_size)i -= map.menu_size+1;
        
        if(map.menu_ptr[i].next){//якщо є наступна вітка дерева меню то рухатись по ній
            menu_push();//закинути в стек поточну позицію
            map.menu_size = (char)map.menu_ptr[i].size;
            map.menu_ptr = map.menu_ptr[i].next;
            map.lcd_flash_row = 0;
            map.sublevel = 0;
            return;
        }    
    
        switch((char)map.menu_ptr[i].changeful){
            case 1://параметр можна змінювати
                menu_status.input_row = i;//рядок де буде відбуватися зміна числа
                menu_status.fetch = 1;//необхідно захопити рядок для подальшої модифікаціі
                break;
            case 2://виконати пост-функцію по натисканню кнопки ввод
                post_func = map.menu_ptr[i].post_funct;
                if(post_func)post_func();
                break;
        }
    }
    else{//якщо меню в режимі модифікації
        
        back = atof(&lcd_input_str[strpos(map.menu_ptr[menu_status.input_row].format,'%')]);
        if(_chkfloat_(back)>1)back = 0;
        post_func = map.menu_ptr[menu_status.input_row].post_funct;
        
        if(post_func == 0 || (post_func && post_func()==0)){
            
            type = (unsigned char)map.menu_ptr[menu_status.input_row].type;//тип числа
            
            switch(type){
                case 0:
                    break;
                case 1:
                    type = (unsigned char)map.menu_ptr[menu_status.input_row].frac;//кількість дробових знаків
                    if(type){
                        do back *= 10; while(--type);
                    }
                    num = back;
                    if(num < back)num++;
                    *(unsigned char*)map.menu_ptr[menu_status.input_row].number = (char)num;
                    break;
                case 2:
                case 0x12:
                    //обробка двобайтового цілого з умовно дробовою частиною
                    type = (unsigned char)map.menu_ptr[menu_status.input_row].frac;//кількість дробових знаків
                    if(type){
                        do back *= 10; while(--type);
                    }
                    num = back;
                    if(num < back)num++;
                    *(unsigned int*)map.menu_ptr[menu_status.input_row].number = num;
                    break;
                case 4:
                    //обробка double
                    *(double*)map.menu_ptr[menu_status.input_row].number = back;
                    break;

            }
        }
        menu_status.fetch = 0;//повернення в режим навігації
        //зберегти в зовнішній і2с пам'яті
        cpy_xx(&ext_ram_buf, (char*)&rs232_timer1_reload, 1024);
        flash_erase();
        flash_write(&ext_ram_buf, 0x8000, 1024);
        flash_read(0x8000, (char*)&rs232_timer1_reload, 1024);
/*
        //зберегти в зовнішній і2с пам'яті
        cpy_xx(&ext_ram_buf, (char*)&rs232_timer1_reload, 56);
        i2cdata_w.addr = 0;
        i2cdata_w.count = 56;
        i2cdata_w.ptr = &ext_ram_buf;
        i2c_func[2] = I2Csetdata;
        i2c_func[3] = I2Csetdata;
        //прочитати із зовнішній і2с пам'яті
        i2cdata.addr = 0;
        i2cdata.count = 56;
        i2cdata.ptr = (char*)&rs232_timer1_reload;
        i2c_func[4] = I2Cgetdata;
*/
    }
}

void right_key(void)
{
    if(was_make)input_status = 0;
    if(menu_status.fetch == 2)RIGHT();
}

void up_key(void)
{
    if(was_make)input_status = 0;
    if(menu_status.fetch == 2)UP();
    else{
        //для поморгування рядком
        if(map.lcd_flash_row > 0)map.lcd_flash_row--;
        else{
            //якщо в меню рядків меньше ніж чотири
            if(map.menu_size <= 3)map.lcd_flash_row = map.menu_size;
            else{
                if(map.sublevel != 0) map.sublevel--;
                else map.sublevel = map.menu_size;
            }
        }
    }
}

void down_key(void)
{
    if(was_make)input_status = 0;
    if(menu_status.fetch == 2)DOWN();
    else{
        //для поморгування рядком
        if(map.lcd_flash_row < 3){
            if(map.lcd_flash_row < map.menu_size)map.lcd_flash_row++;
            else map.lcd_flash_row = 0; 
        }
        else{
            //якщо в меню рядків меньше ніж чотири
            if(map.menu_size <= 3)map.lcd_flash_row = 0;
            else{
                if(map.sublevel < map.menu_size) map.sublevel++;
                else map.sublevel = 0;
            }
        }
    }
}

void left_key(void)
{
    if(was_make)input_status = 0;
    if(menu_status.fetch == 2)LEFT();
}

void menu_KEY(void)
{
    if(menu_status.fetch == 0)menu_pop();
    else menu_status.fetch = 0;//прейти в режим навігації
}

void UP()
{
    if(lcd_input_str[lcd_crsr_position] == '9')lcd_input_str[lcd_crsr_position] = '0';
    else
        if(lcd_input_str[lcd_crsr_position] == '+')lcd_input_str[lcd_crsr_position] = '-';    
        else
            if(lcd_input_str[lcd_crsr_position] == '-')lcd_input_str[lcd_crsr_position] = '+';
            else lcd_input_str[lcd_crsr_position]++;
}

void DOWN()
{
    if(lcd_input_str[lcd_crsr_position] == '0')lcd_input_str[lcd_crsr_position] = '9';
    else
        if(lcd_input_str[lcd_crsr_position] == '+')lcd_input_str[lcd_crsr_position] = '-';
        else
            if(lcd_input_str[lcd_crsr_position] == '-')lcd_input_str[lcd_crsr_position] = '+';
            else lcd_input_str[lcd_crsr_position]--;
}

void RIGHT()
{
    lcd_crsr_position++;
    if((lcd_crsr_position > 19) || (lcd_input_str[lcd_crsr_position] <= 0x20)){
        lcd_crsr_position = strpos(map.menu_ptr[menu_status.input_row].format,'%');
        if(lcd_crsr_position == 0xff){
            lcd_crsr_position = 0;
            menu_status.fetch = 0;
            return;
        }
    }
    if(lcd_input_str[lcd_crsr_position]=='.' || lcd_input_str[lcd_crsr_position]== ':'||lcd_input_str[lcd_crsr_position] == 'e')lcd_crsr_position++;
}

void LEFT()
{
    if((lcd_crsr_position != 0)&&(lcd_input_str[lcd_crsr_position-1] != 0x20))lcd_crsr_position--;
    else
    {
        if(lcd_init_crsr_position() == 0){
            lcd_crsr_position = 0;
            menu_status.fetch = 0;
            return;
        }
    }
    if(lcd_input_str[lcd_crsr_position] == '.'||lcd_input_str[lcd_crsr_position] == ':'||lcd_input_str[lcd_crsr_position] == 'e')lcd_crsr_position--;
}

/*
void reload()//функція для оновлення регістрів котрі мають дублікати парамeтрів у меню
{
    RCAP2 = rs232_timer1_reload;
    i2c_management.kill_after_execution = 1;
}
*/
bit time()
{
    sprintf(lcd_str,menu2[0].format,
        (int)bcd_to_bin(datetime.hours),
        (int)bcd_to_bin(datetime.minutes),
        (int)bcd_to_bin(datetime.seconds)
        );
    return(1);
}

bit time_back()
{
    char *ptr_sc;
    unsigned char hour_,min,sec;

    //копіювання прочитаного часу в записуваний
    cpy_xx((char*)&datetime_w,(char*)&datetime,sizeof(datetime_w));
    
    ptr_sc = strchr(lcd_input_str,':');
    hour_ = atoi(ptr_sc-2);
    if(hour_ <= 23)datetime_w.hours = bin_to_bcd(hour_);

    min = atoi(ptr_sc+1);
    if(min <= 59)datetime_w.minutes = bin_to_bcd(min);

    ptr_sc = strchr(ptr_sc+1,':');
    sec = atoi(ptr_sc+1);
    if(sec <= 59)datetime_w.seconds = bin_to_bcd(sec);
    
    //ставить в чергу задач запис часу в зов. годинник, зніме себе після виконнаня
    i2c_func[1] = settime;
    return(1);
}

bit daten()
{
    sprintf(lcd_str,menu2[1].format,
        (int)bcd_to_bin(datetime.date),
        (int)bcd_to_bin(datetime.mounth),
        (int)bcd_to_bin(datetime.year)
        );
    return(1);
}

bit daten_back()
{
    char *ptr_sc;
    unsigned char year_,month_,date_;

    //копіювання прочитаної дати в записувану
    cpy_xx((char*)&datetime_w,(char*)&datetime,sizeof(datetime_w));

    ptr_sc = strchr(lcd_input_str,'.');
    date_ = atoi(ptr_sc-2);
    if((date_ <= 31)||(date_ > 0))datetime_w.date = bin_to_bcd(date_);

    month_ = atoi(ptr_sc+1);
    if((month_ <= 12)||(month_ > 0))datetime_w.mounth = bin_to_bcd(month_);

    ptr_sc = strchr(ptr_sc+1,'.');
    year_ = atoi(ptr_sc+1);
    if(year_ <= 99)datetime_w.year = bin_to_bcd(year_);
    
    datetime_w.day = dayOfWeek(dayOfYear(date_, month_, 2000+year_), 2000+year_);
    
    //ставить в чергу задач запис часу в зов. годинник, зніме себе після виконнаня
    i2c_func[1] = settime;
    return(1);
}

bit calibr()
{
    char calibr;
    
    //відсікти старших два біта і розмножити знак
    calibr = (datetime.calibrate & 0x20)? datetime.calibrate | 0xc0 : datetime.calibrate & 0x3f;
    sprintf(lcd_str,menu2[2].format,(int)calibr);
    
    return(1);
}

bit calibr_back()
{
    char calibr;

    //копіювання прочитаної дати в записувану
    cpy_xx((char*)&datetime_w,(char*)&datetime,sizeof(datetime_w));

    calibr = atoi(&lcd_input_str[strpos(map.menu_ptr[menu_status.input_row].format,'%')]);
    if((calibr > -32) && (calibr < 32)){
        datetime_w.calibrate = calibr;
        //ставить в чергу задач запис часу в зов. годинник, зніме себе після виконнаня
        i2c_func[1] = settime;
    }
    
    return(1);
}

code char all_week[][21]={
{"                    "},
{"         Понедельник"},
{"         Вторник    "},
{"         Среда      "},
{"         Четверг    "},
{"         Пятница    "},
{"         Суббота    "},
{"         Воскресение"},
};

bit week_day()
{
    
    sprintf(lcd_str,all_week[(datetime.day>8)? 0:datetime.day]);
    return(1);
}

code double sys_clock = ((double)(49000000/16));

bit rs232_bit_rate()
{
    double bit_rate;

    if(rs232_timer1_reload == 0)rs232_timer1_reload = 1;
    if(rs232_timer1_reload > 0xffe5){
        rs232_timer1_reload = 0xffE5;
        RCAP2 = rs232_timer1_reload;
    }
    bit_rate = sys_clock/((double)((unsigned int)0x0000 - rs232_timer1_reload));
    sprintf(lcd_str,"Скорость  %#06ld бод",(unsigned long)bit_rate);

    return 1;
}

bit rs232_bit_rate_back()
{
    unsigned int int_part;

    back = sys_clock/back;
    int_part = back;
    back -= int_part;

    if(back >= 0.5)int_part++;

    rs232_timer1_reload = (unsigned int)0x0000 - int_part;

    RCAP2 = rs232_timer1_reload;
    return 1;
}

/*
bit printer()
{

    tx_command = 2;//зображення

    tx_g.column = microlog[0];
    tx_g.row = microlog[1];
    tx_g.ptr = &microlog[2];
    tx_g.size = sizeof(microlog)-2;
    tx_count = 0;
    tx_comm_count = 0;
    melody_ptr = &beep;
    timer_arr[2].start_funct();         
    TI0 = 1;
    return 0;
}
*/
bit printer1()
{
    tx_command = 0;//текст

    tx.limit = sprintf(tx.buf,"\n       УСТРОЙСТВО\n    ДИАГНОСТИЧЕСКОГО\n        КОНТРОЛЯ\n       МЛ 410.110\n\nЧМП \"МИКРОЛОГ\"\nг.Хмельницкий,\nул.Свободы, 22 оф.2\n\nwww.microlog.km.ua\noffice@microlog.km.ua\n\n tel. +38 0382 700394\n tel. +38 0382 700395\n fax  +38 0382 700396\n\n       Programming\n  by Volodymyr Furman\n  tel.+38 097 9537752\n    furman@ukr.net \n\n    \251 MICROLOG 2005\n");

    tx_count = 0;//обнулити лічильник переданих байтів
    melody_ptr = &beep;
    timer_arr[2].start_funct();

    TI0 = 1;
    return 0;
}

/*
bit start_mts0()
{
    long delta;

    delta = delta_dac(900,du_dt_0_mts0);
    MTS[0].delta = delta;
    MTS[0].value = 0x00000000;
    //MTS[0].limit = 0x0fff;
    timer_arr3[0].start_funct(0);//запуск мтс

    return 0;    
}

bit start_mts1()
{
    long delta;

    delta = delta_dac(500,du_dt_0_mts1);
    MTS[1].delta = delta;
    MTS[1].value = 0x00000000;
    //MTS[1].limit = 0x0fff;
    timer_arr3[1].start_funct(1);//запуск мтс

    return 0;    
}

bit start_mts2()
{
    long delta;

    delta = delta_dac(900,du_dt_0_mts2);
    MTS[2].delta = delta;
    MTS[2].value = 0x00000000;
    //MTS[2].limit = 0x0ff;
    timer_arr3[2].start_funct(2);
    
    return 0;    
}
*/

bit adc_def_func()
{
    unsigned char i;

    for(i=0;i<16;i++)
        adc_scale[i] = adc_scale_def[i];
    melody_ptr = &beep;
    timer_arr[2].start_funct();

    return 0;
}
bit adc_def_off_func()
{
    unsigned char i;

    for(i=0;i<16;i++)
        adc_offset[i] = 0;

    melody_ptr = &beep;
    timer_arr[2].start_funct();

    return 0;
}                                    
/*
void all_paramert_send()
{
    tx_command = 0;//текст

    cpy_xx(&tx.buf, (char*)&rs232_timer1_reload, 1024);

    tx.limit = 1024;

    tx_count = 0;//обнулити лічильник переданих байтів
    melody_ptr = &beep;
    timer_arr[2].start_funct();

    TI0 = 1;
}
*/

bit Ki_def_func()
{
    Kir = 887;
    Kiu = 10100;
    melody_ptr = &beep;
    timer_arr[2].start_funct();
    return 0;
}
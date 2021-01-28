#include <c8051f120.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <intrins.h>
#include <ctype.h>
#include <lcd.h>
#include "spi.h"
#include "timer_4.h"
#include "timer_3.h"
#include "menu.h"
#include "macro.h"
#include "definition.h"
#include "uart_0.h"
#include "i2c.h"

#define left        1
#define up          2
#define down        3
#define right       4
#define menu_key    5
#define enter       6
#define SS          7
#define PP          8

sbit k30 = P1^4;
sbit k31 = P1^5;
sbit k32 = P1^6;
sbit k33 = P1^7;

void(*print_func)() = 0;//������ ������� ��� ����� �����

unsigned char   tst_stage = 0,  //����
                tst_iter = 0,   //�������� � ����
                tst_tact = 0,   //���� 
                tst_status = 0, //������ ��������������� ��� ����������� ���������� 
                tst_regime = 0; //0 - ������� 1 - ����������� �� ������ ����

//������� �������� ��������� �� �������� ������ �� ������� -����������� ��������
//��������� ��� 10�� ������ 1/10��=100
#define op_sec 100
//#define pog_dac ((unsigned long)(0x0cffffff/op_sec))
#define pog_dac ((unsigned long)(0x0fffffff/op_sec))

long delta_dac(unsigned int Umax, int du_dt)//�� ��� *10 (���� ������� ���� ���� ����) 
{
    return pog_dac/Umax*du_dt;     
}

unsigned int value_dac(float Umax, float U)
{
    float i;
/*
    if(U>Umax)return 0x0cff;
    else i = U/Umax * 0x0cff;
*/
    if(U>Umax)return 0x0fff;
    else i = U/Umax * 0x0fff;

    return i;
}
xdata unsigned int Uset_out180 = 0;

void PI_regulator()
{
    signed int delta;
    unsigned int value;    

    if(Uset_out180 >1700)Uset_out180 = 1700;
    if(Uset_out180 != 0){
        delta = (Uset_out180 - ml140_150.U[0]);
        value = (int)ml140_130[0].dac_ad7243 + delta;
        if(value > 0x0fff)value = 0x0fff;
    }
    else value = 0;
    ml140_130[0].dac_ad7243 = value;
    ml140_130[2].dac_ad7243 = value;
}
xdata unsigned int Uset_out50 = 0;
void PI_regulator50()
{
    signed int delta;
    unsigned int value;    

    Uset_out50 = ml140_150.U[2];//��� �� ���� ���� �������� ������ ������, �� �� �� �� ���� ��� � ���� ���������
    //if(Uset_out50 >500)Uset_out50 = 500;
    if(Uset_out50 != 0){
        delta = (Uset_out50 - ml140_150.U[2]);
        value = (int)ml140_130[1].dac_ad7243 + delta;
        //if(value > 0x0cff)value = 0x0cff;
        if(value > 0x0fff)value = 0x0fff;
    }
    else value = 0;
    ml140_130[1].dac_ad7243 = value;
}

//��������
#define StartYear 1
#define FirstDayOfStartYear 1
#define FirstYearIsLeap 0

//��������� �����
code char day_tab[2][13] =
{   
    { 0,31,28,31,30,31,30,31,31,30,31,30,31} ,
    { 0,31,29,31,30,31,30,31,31,30,31,30,31}
};

//���������� ��� �����
char dayOfWeek(int dayOfYear, int year)
{
     int iNumberOfLeap;
     int week_day;

     year -= StartYear;
     iNumberOfLeap = year/4 - year/100 + year/400 + FirstYearIsLeap;
     week_day = (year + iNumberOfLeap + FirstDayOfStartYear + 
                       (dayOfYear-1)) % 7;
     if (week_day == 0) week_day = 7;


     return week_day;
}

//���������� �� ���������� ��
char Leap(int year)
{
    char leap;    
    if ((year%4 == 0 && year%100 != 0) || year%400 == 0) leap = 1;
    else leap = 0;
    return leap;
}

//���������� ��� ����
int dayOfYear(char day, char month, int year)
{
     int dayofyear = 0;
     char ii, leap;

     /* reference Ritchie&Kernighan */
     leap = Leap(year);     

     for (ii = 1; ii < month; ii++) 
           dayofyear += day_tab[leap][ii];

     dayofyear += day;

     return dayofyear;
}

//����������� ���������
char alarm_was = 0;
void leds_and_alarm()
{
    unsigned char i=0;
    
    if((!k30|!k31|!k32)&!alarm_was){
        //��� �������� ���� 0 �� ��
        i |= (k30? 0:1)<<0;
        i &= ~((k30? 1:0)<<0);
        i |= (k31? 0:1)<<1;
        i &= ~((k31? 1:0)<<1);
        i |= (k32? 0:1)<<2;
        i &= ~((k32? 1:0)<<2);
        
        //emitation ���������� ������� ��
        clr_bit(EIE2,ET4);
        keys_push(SS);
        set_bit(EIE2,ET4);
        melody_ptr = &death;
        timer_arr[2].start_funct();
        *timer_arr3[5].tick = 65535;//�������� ��������
        alarm_was = 1;
    }
    else{
        if(*timer_arr3[5].tick)return;
        i |= (ml140_130[0].dac_ad7243? 1:0)<<4;
        i &= ~((ml140_130[0].dac_ad7243? 0:1)<<4);
        i |= (ml140_130[1].dac_ad7243? 1:0)<<5;
        i &= ~((ml140_130[1].dac_ad7243? 0:1)<<5);
        i |= (ml140_130[2].dac_ad7243? 1:0)<<6;
        i &= ~((ml140_130[2].dac_ad7243? 0:1)<<6);
    }
    io = i;
}

//������� ���������� ������
typedef struct relays{
    unsigned char relay16_23;
    unsigned char relay8_15;
    unsigned char relay0_7;
};

void commutation(struct relays code* relay)
{
    ml140_140.relay16_23 = relay->relay16_23;
    ml140_140.relay8_15 = relay->relay8_15;
    ml140_140.relay0_7 = relay->relay0_7;
}

void melody_ok()
{
        melody_ptr = &beep;
        timer_arr[2].start_funct();
        tst_tact++;
}

void melody_bad()
{
        //������� ���������� ����
        melody_ptr = &death;
        timer_arr[2].start_funct();
        tst_tact = 0xff;
}

void melody_final()
{
    melody_ptr = &final;
    timer_arr[2].start_funct();
}

void start_regulator(unsigned int u)
{
    Uset_out180 = u;
    *timer_arr3[8].tick = Tdiscrete180;
}

void stop_regulator()
{
    clr_bit(EIE2,ET3);
    *timer_arr3[8].tick = 0;
    set_bit(EIE2,ET3);
}
void formU_du_dt(unsigned char xdata*U, char du_dt) 
{
    MTS[0].delta = delta_dac(900,du_dt)/2;
    MTS[2].delta = MTS[0].delta;
    MTS[0].value = (long)value_dac(90,*U/2)<<16;
    MTS[2].value = MTS[0].value;
    MTS[0].limit = 0xfff;//0x0cff;
    MTS[2].limit = 0xfff;//0x0cff;
    //������ ����������� �������
    timer_arr3[0].start_funct(0);
    timer_arr3[2].start_funct(2);
}

void run_du_dt(char du_dt, unsigned char lim) 
{
    MTS[0].delta = delta_dac(900,du_dt)/2;
    MTS[2].delta = MTS[0].delta;

    MTS[0].value = (long)ml140_130[0].dac_ad7243<<16;
    MTS[2].value = MTS[0].value;
    
    MTS[0].limit = (du_dt >= 0)? value_dac(180, lim) : 0x0000;
    MTS[2].limit = MTS[0].limit;
    //������ ����������� �������
    timer_arr3[0].start_funct(0);
    timer_arr3[2].start_funct(2);
}

void end_test_func()
{
    char key_1;

    if((tst_tact == 128)&&(print_func != 0))sprintf(lcd_str,"��:������ CC:�����  ");
    else sprintf(lcd_str,"��:������ CC:�����  ");
    lcd_put_str(3,0,lcd_str,0);
    
    key_1 = keys_pop();
    switch(key_1){
    case PP: 
        if((tst_tact == 128)&&(print_func != 0)){
            print_func();
            //print_func = 0;
            break;
        }
        tst_tact = 0; 
        tst_iter = 0;
        if(tst_regime == 0){//���� ����� �������� �� �� �������
            tst_stage = 0;
        }
        break;//������
    case SS:
        clr_bit(EIE2,ET4);
        keys_push(SS);
        set_bit(EIE2,ET4);
        break;
    case menu_key:
    case enter:
        clr_bit(EIE2,ET4);
        keys_push(key_1);
        set_bit(EIE2,ET4);
        break;
    }
}


code struct relays Umins[2] = {
{0x00,0x20,0x00},{0x00,0x20,0x40},
};

struct relay_umin{
    unsigned char test;//����� �������� �����
    unsigned char Ustart;
    unsigned char du_dt;
    unsigned char Uon_e;
    unsigned char Uoff_e;
    unsigned char U_tol;
    unsigned char U_tol_off;
    unsigned char U_lim;
    unsigned int Uon;
    unsigned int Uoff;
    unsigned char print;
};

xdata struct relay_umin relay_Umin;


bit Umin_test_0()//������� ������ ���� �������� �������
{
    return(((ml140_150.U[4] > 100)&&(ml140_150.U[5] > 100)
    &&(ml140_150.U[6] > 100)&&(ml140_150.U[13] > 100))? 1:0);
}
bit Umin_test_1()
{
    return(((ml140_150.U[4] > 300)&&(ml140_150.U[5] > 300)
    &&(ml140_150.U[13] > 300))? 1:0);
}
bit(*relay_test[])()={Umin_test_0,Umin_test_1};

void relay_Umin_2450_033_t0()
{
    sprintf(lcd_str,"U24=%4.1f� U1=%5.1f� ",(float)ml140_150.U[2]/10,(float)ml140_150.U[0]/10);
    lcd_put_str(0,0,lcd_str,0);
}

void relay_Umin_s1_0()
{
    
    sprintf(lcd_str,"%4.1f %4.1f %4.1f %4.1f ",
    (float)ml140_150.U[13]/10,(float)ml140_150.U[4]/10,(float)ml140_150.U[5]/10,(float)ml140_150.U[6]/10);
    lcd_put_str(1,0,lcd_str,0);
            
}
void relay_Umin_s1_1()
{
    
    sprintf(lcd_str,"  %4.1fB %4.1fB %4.1fB ",
    (float)ml140_150.U[13]/10,(float)ml140_150.U[4]/10,(float)ml140_150.U[5]/10);
    lcd_put_str(1,0,lcd_str,0);
            
}

void(*relay_str1[])()={relay_Umin_s1_0,relay_Umin_s1_1};

code char head1[] = {"\n�������� �����������\n\n________________________\n"};
code char *block[] ={
{"\n2450.033 �______________\n"},
{"\nEAU 2/11 �______________\n"},
{"\nEAU 4/12 �______________\n"},
{"\nEAU 11/13 �_____________\n"},
};
code char coach[] = {"����� �_________________\n\n���������� ������������:\n"};

code char booter[] = {"\n\n�������� _______________\n\n\n������� ________________\n����  %02d.%02d.%4d\n����� %02d:%02d:%02d\n"};
code unsigned char format_Umin[]={"U������������ = %6.1f B\n\nU����������   = %6.1f B\n"};

void print_boot()
{
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));
}

void r2450_033_print()
{
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,head1);
    tx.limit += sprintf(&tx.buf[tx.limit],block[relay_Umin.print]);
    tx.limit += sprintf(&tx.buf[tx.limit],coach);
    tx.limit += sprintf(&tx.buf[tx.limit],format_Umin,
                    (float)relay_Umin.Uon/10,
                    (float)relay_Umin.Uoff/10
                    );
    print_boot();
    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}

code char *relay_Umin_mes[]={
{"�.�.�������� - ���� "},
{"������. ������������"},
{"�������� ����������!"},
{"    � � � � � �     "},
{"�.O.�������� - ���� "},
{"�������� ���������� "},
{"�������� ���������� "}
};
void relay_Umin_s2_0()
{
    sprintf(lcd_str,relay_Umin_mes[tst_status]);
    lcd_put_str(2,0,lcd_str,0);
}

void next_Umin()
{
    if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
        if(++tst_stage > 1){
            
            melody_final();

            tst_stage--;
            tst_tact = 128;
            print_func = r2450_033_print;
        }
        else{
            tst_tact = 0;
        }
    }
    else{
        tst_tact = 128;
    }
}

void relay_Umin_test_0()
{
	char flag_time;

    switch(tst_tact){
        
        case 0:
            tst_status = 0;
            relay_Umin.Uon = 0;
            relay_Umin.Uoff = 0;
            //��� �15
            commutation(&Umins[0]);
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            start_regulator((int)relay_Umin.Ustart*10);
            tst_tact++;
            break;

        case 1:
            timer_arr3[4].start_funct(20);//2c
            tst_tact++;
			flag_time = 0;
			break;

        case 2:    
            if(relay_test[relay_Umin.test]() && !flag_time){
				timer_arr3[4].start_funct(4);//2c
				flag_time = 1;
				/*
				*timer_arr3[4].tick = 0;
				timer_arr3[4].end_funct(0);
				*/
			}
			break;

        case 3:
            if(relay_test[relay_Umin.test]()){
                tst_status = 2;
                melody_ok();
            }
            else{
                tst_status = 3;
                melody_bad();
            }
            break;

        case 4:
            stop_regulator();
            run_du_dt(relay_Umin.du_dt,relay_Umin.U_lim);
            tst_status = 1;
            tst_tact++;
            break;
        
        case 5:
            if(!relay_test[relay_Umin.test]()){
                //�������� ����
                MTS[0].status = 2;
                MTS[2].status = 2;
                //�������
                melody_ok();
                break;
            }
            if((MTS[0].status == 2)&&(MTS[2].status == 2)){
                //������� ���������� ����
                tst_status = 3;
                melody_bad();
            }
            break;

        case 6:
            relay_Umin.Uon = ml140_150.U[0];
            tst_tact++;            
            break;
        case 7:
            if((relay_Umin.Uon <= (int)(relay_Umin.Uon_e)*10+relay_Umin.U_tol)&&(relay_Umin.Uon >= (int)(relay_Umin.Uon_e)*10-relay_Umin.U_tol)){
                tst_tact++;
                tst_status = 2;
            }
            else{
                melody_bad();
                tst_status = 3;
            }
            break;
        case 8:
            next_Umin();
            break;
        default:
            break;
    }

}

void relay_Umin_test_1()
{
    switch(tst_tact){
        case 0:
            tst_status = 4;
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            commutation(&Umins[1]);
            timer_arr3[4].start_funct(5);//2c
            if(ml140_150.U[0]<10)start_regulator((int)relay_Umin.Uon_e*10);
            tst_tact++;
            break;
        case 1:    
            break;
        case 2:
            if(relay_test[relay_Umin.test]()){
                tst_status = 2;
                melody_ok();
            }
            else{
                tst_status = 3;
                melody_bad();
            }
            break;
        case 3:
            stop_regulator();
            run_du_dt(-relay_Umin.du_dt,relay_Umin.U_lim);
            tst_status = 5;
            tst_tact++;
            break;

        case 4:
            if(!relay_test[relay_Umin.test]()){
                //�������� ����
                MTS[0].status = 2;
                MTS[2].status = 2;
                tst_status = 2;
                melody_ok();
                break;
            }
            if((MTS[0].status == 2)&&(MTS[2].status == 2)){
                tst_status = 3;
                melody_bad();
            }
            break;

        case 5:
            relay_Umin.Uoff = ml140_150.U[0];
            Udest_2450_33_t = relay_Umin.Uoff;
            tst_tact++;
            break;
        case 6:    
            if((relay_Umin.Uoff <= (int)(relay_Umin.Uoff_e)*10+relay_Umin.U_tol_off)&&(relay_Umin.Uoff >= (int)(relay_Umin.Uoff_e)*10-relay_Umin.U_tol_off)){
                tst_tact++;
                tst_status = 2;
            }
            else{
                melody_bad();
                tst_status = 3;
            }
            break;
        case 7:
            next_Umin();
            break;
        default:
            break;
    }

}

void relay_Umin_test()
{
    switch(tst_stage){
        case 0: relay_Umin_test_0();break;
        case 1: relay_Umin_test_1();break; 
    }
    
    relay_Umin_2450_033_t0();
    relay_str1[relay_Umin.test]();
    relay_Umin_s2_0();
    end_test_func();
}

void relay_Umin_2450_033_t()
{
    float a;
    static unsigned int dac_code,time_off;

    switch(tst_tact){
        
        case 0:
            tst_status = 4;
            commutation(&Umins[1]);
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            timer_arr3[4].start_funct(5);//2c
            start_regulator((int)Ustart_2450_33_t*10);
            tst_tact++;
            break;
        case 1:    
            break;
        case 2:
            if(relay_test[relay_Umin.test]()){
                tst_status = 2;
                melody_ok();
            }
            else{
                tst_status = 3;
                melody_bad();
            }
            break;
        case 3:
            stop_regulator();
            run_du_dt(-du_dt_2450_33_t,0);
            tst_status = 6;
            tst_tact++;
            break;
        case 4:
            if(ml140_150.U[0] <= Udest_2450_33_t){
                //�������� ����
                MTS[0].status = 2;
                MTS[2].status = 2;
                dac_code = (ml140_130[0].dac_ad7243 + ml140_130[2].dac_ad7243)/2;
                tst_status = 2;
                melody_ok();
                timer_arr3[4].start_funct(pause_2450_33_t);//��� ����������� � ������������ ���
                break;
            }
            if((MTS[0].status == 2)&&(MTS[2].status == 2)){
                tst_status = 3;
                melody_bad();
            }
            break;

        case 5:
            if(!relay_test[relay_Umin.test]()){
                tst_status = 3;
                //������� ���������� ����
                melody_bad();
                *timer_arr3[4].tick = 0;
            }
            break;
        case 6:
            //������� ���� �� ��������� ������� ��������
            ml140_130[0].dac_ad7243 = (float)dac_code * (float)(100-Udelta_2450_33_t)/100;
            ml140_130[2].dac_ad7243 = ml140_130[0].dac_ad7243;
            *timer_arr3[7].tick = 0xffff;//��������� ������
            tst_tact++;
            break;

        case 7:
            if(!relay_test[relay_Umin.test]()){
                //�������� ����
                //�������
                time_off = *timer_arr3[7].tick;
                time_off = (0xffff - time_off)/10;//��������� � �������� ���� ���� ���� ����
                *timer_arr3[7].tick = 0;//�������� ������
                melody_ok();
                break;
            }                                          
            a =(float)(0xffff - *timer_arr3[7].tick)/100;
            if(a > 180){
                tst_status = 3;
                melody_bad();
            }
            break;

        case 8:
            sprintf(lcd_str,"U���������� =%5.1f B",(float)Udest_2450_33_t/10); lcd_put_str(0,0,lcd_str,0);
            sprintf(lcd_str,"������U     = %2d %%  ",(int)Udelta_2450_33_t); lcd_put_str(1,0,lcd_str,0);
            sprintf(lcd_str,"����� ���.  =%5.1f c",(float)time_off/10); lcd_put_str(2,0,lcd_str,0);   
            goto pass_all;
            return;

        default:
            break;
    }
    relay_Umin_2450_033_t0();
    relay_str1[0]();
    switch(tst_tact){
        case 5: sprintf(lcd_str,"����� ������ %5d �",*timer_arr3[4].tick/100);lcd_put_str(2,0,lcd_str,0);break;
        case 7: sprintf(lcd_str,"��.���������� %4.1f c",a);lcd_put_str(2,0,lcd_str,0);break;
        default:
             relay_Umin_s2_0();
    }
pass_all:
    end_test_func();
}
code char header_520[] = {"\n�������� �����������\n\n________________________\n\n�� 520 �________________\n����� �_________________\n\n���������� ������������:\n"}; 
void ml520_print()
{
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,header_520);
    tx.limit += sprintf(&tx.buf[tx.limit],"\n���� ��������\n");              
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));

    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}


unsigned char ml520_sw_flag, ml520_sw_last_flag, ml520_sw_count;


bit ml520_f0()//����� � ���� ��������
{
    return ((ml520_sw_count>=3)? 1:0);//1 - ok
}

bit ml520_f1()//���������� ������
{
    return (((ml520_sw_count==0)&&(ml140_150.U[7]>250))? 1:0);//1 - ok
}

bit ml520_f2()//��������� ���������
{
    return ((ml520_sw_count>=3)? 1:0);//1 - ok
}

bit ml520_f3()//��������
{
    return (((ml520_sw_count==0)&&(ml140_150.U[7]<250))? 1:0);//1 - ok
}

//����� ������� ��� ������������ ��������� �����
bit(*ml520_func[4])()= {ml520_f0, ml520_f1, ml520_f2, ml520_f3};

void ml520_what_to_do()//�������� ����� �� �����
{
    if(ml520_func[tst_stage]()){
        //��������� ����������
        tst_status = 1; 
        melody_ptr = &beep;
        timer_arr[2].start_funct();
        timer_arr3[4].start_funct(1);
        tst_tact++;
    }
    else{                
        //������� ���������� ����
        tst_status = 4;
        melody_ptr = &death;
        timer_arr[2].start_funct();
        tst_tact = 0xff;
    }
}


code struct relays s[4][2] = {
{0,0x40,0},{0,0x40,0},
{0,0xc0,0},{0,0xc0,0},
{0,0x48,0},{0,0x48,0},
{0x01,0x40,0},{0x01,0xc0,0},
};

code unsigned int U_ml520[]={350,500,1100,1600};

code char ml520_name[][21]={
{"   ����� ��������   "},
{" ����������  ������ "},
{"��������� ��������� "},
{"�������� �������(��)"},
};

code char all_right[]={"�������� ����������!"};

code char ml520_mess[][21]={
{"  �������� ������   "},
{" ����.����. ������. "},
{" ����.�������. �.�. "},
{" �������� ��������� "},
{" �������� ��������� "},
{" ������  ���������� "},
{"       �����        "},
{"       ������       "}
};

code char code*mess_ptr[][5]={
&ml520_mess[0],&ml520_mess[6],&ml520_mess[6],&ml520_mess[5],&ml520_mess[7],
&ml520_mess[1],&ml520_mess[6],&ml520_mess[6],&ml520_mess[5],&ml520_mess[7],
&ml520_mess[2],&ml520_mess[6],&ml520_mess[6],&ml520_mess[5],&ml520_mess[7],
&ml520_mess[3],&ml520_mess[6],&ml520_mess[4],&ml520_mess[5],&ml520_mess[7],
};

void ml520_test()
{
    if(ml140_150.U[7]>250) ml520_sw_flag = 1;
    else ml520_sw_flag = 0;
        
    switch(tst_tact){
        case 0:
            tst_status = 0;
            ml140_140.relay16_23 = s[tst_stage][0].relay16_23;
            ml140_140.relay8_15 = s[tst_stage][0].relay8_15;
            ml140_140.relay0_7 = s[tst_stage][0].relay0_7;
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            //������ ����������                                               
            Uset_out180 = U_ml520[tst_iter];//��������
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(4);//�������� ��� ������� �������
            tst_tact++;
            break;

        case 1:
            *timer_arr3[7].tick = 200;//������������� ������ �� 2�.
            ml520_sw_count = 0;
            break;
        case 2:
            if(ml520_sw_flag != ml520_sw_last_flag)ml520_sw_count++;
            if(*timer_arr3[7].tick == 0)tst_tact++;
            break;
        case 3:
            ml520_what_to_do();
            break;
        case 4:
            break;//������ ����������� �������
        case 5:
            tst_status = 2;
            //�������� ���������
            ml140_140.relay16_23 = s[tst_stage][1].relay16_23;
            ml140_140.relay8_15 = s[tst_stage][1].relay8_15;
            ml140_140.relay0_7 = s[tst_stage][1].relay0_7;
            timer_arr3[4].start_funct(1);
            tst_tact++;
            break;
        case 6:
            break;//������ ����������� �������
        case 7:
            ml520_what_to_do();//���� �� ������� ��������
            break;
        case 8:
            break;//������ ����������� �������
        case 9:
            tst_status = 3;
            Uset_out180 = 0;//����� �������
            timer_arr3[4].start_funct(3);
            tst_tact++;
            break;
        case 10:
            break;//������ ����������� �������
        case 11:
            if(++tst_iter > (sizeof(U_ml520)/2-1)){
                if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
                    tst_iter = 0;
                    if(++tst_stage > (sizeof(U_ml520)/2-1)){
                        tst_stage = sizeof(U_ml520)/2-1;
                        melody_final();
                        tst_tact = 128;
                        print_func = ml520_print;
                    }
                    else tst_tact = 0;
                }
                else{
                    tst_tact = 128;
                    tst_iter = 0;
                }
            }
            else tst_tact = 0;//��������� ��� ����� ������            
            
            break;
        default:
            break;
    }

    sprintf(lcd_str,ml520_name[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    sprintf(lcd_str,"Up=%5.1f� Ui=%4.1f�  ",(float)ml140_150.U[0]/10,(float)ml140_150.U[7]/10);
    if(ml140_150.U[7]<250)lcd_str[19] = '*';    
    lcd_put_str(1,0,lcd_str,0);

    sprintf(lcd_str,mess_ptr[tst_stage][tst_status]);
    if(tst_tact == 128)sprintf(lcd_str,all_right);
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
    
    ml520_sw_last_flag = ml520_sw_flag;
}


code struct relays i_load[3] = {
{0,0x08,0x01},
{0,0x10,0x00},
{0,0x98,0x02},
};

code char r2450_212_name[][21]={
{"��� ������ ����� n2 "},
{"�����. I �����������"},
{"���������� ����     "},
};

code char r2450_212_mess[][21]={
{"����������� ������. "},
{"��������            "},
{"       �����        "},
{"       ������       "}
};


float I_back,I_cons,Uvtag;

bit r2450_212_w_0()
{
    if((MTS[0].status == 2)||(ml140_150.U[0] >= (int)U_dest212*10)){
        MTS[0].status = 2;
        MTS[2].status = 2;
        return 0;
    }
    else return 1;
}
bit r2450_212_w_2()
{
    if((ml140_150.U[13]>10)||(ml140_150.U[0] >= (int)U_dest212*10)){
        //�������� ����
        MTS[0].status = 2;
        MTS[2].status = 2;
        timer_arr3[4].start_funct(1);//��� ����������� � ������������ ���
        tst_tact++;//�������������
    }
    else{
        if(MTS[0].status == 2){
            timer_arr3[4].start_funct(1);//��� ����������� � ������������ ���
            tst_tact++;//�������������
        }
    }
    return 1;
}

//����� ������� ����������
bit(*r2450_212_w_func[])()= {r2450_212_w_0, r2450_212_w_0,r2450_212_w_2};

//������� ���������
void r2450_212_m_0()
{
    I_back = (float)ml140_150.U[14]/10/10/9.2*1000;
}
void r2450_212_m_1()
{
    I_cons = (float)ml140_150.U[15]/10/2/32*1000;
}
void r2450_212_m_2()
{

}
//����� ������� ���������� 
void(*r2450_212_m_func[])()= {r2450_212_m_0, r2450_212_m_1,r2450_212_m_2};

void r2450_212_s_0()
{
    sprintf(lcd_str,"U=%5.1f �  i=%3.0f ���",(float)ml140_150.U[0]/10,I_back);
}
void r2450_212_s_1()
{
    sprintf(lcd_str,"U=%5.1f �  i=%3.0f �� ",(float)ml140_150.U[0]/10,I_cons);
}
void r2450_212_s_2()
{
    sprintf(lcd_str,"U=%5.1f � (%5.1f �)",(float)ml140_150.U[0]/10,(float)ml140_150.U[13]/10);
}
//����� ������� �����������
void(*r2450_212_s_func[])()= {r2450_212_s_0, r2450_212_s_1, r2450_212_s_2};

//������� ��������� ������ ����������
void r2451_212_tst_0()
{
    if(I_back > I_212){
        tst_status = 3;
        //������� ���������� ����
        melody_bad();
        timer_arr[2].start_funct();
    }
    else{
        tst_status = 2; 
        melody_ok();
        timer_arr[2].start_funct();
    }
}

void r2451_212_tst_1()
{
    if(I_cons > I_212_1){
        tst_status = 3;
        //������� ���������� ����
        melody_bad();
        timer_arr[2].start_funct();
    }
    else{
        tst_status = 2; 
        melody_ok();
        timer_arr[2].start_funct();
    }
}

void r2451_212_tst_2()
{
    if(ml140_150.U[13]>10){
        //�������� ����
        Uvtag = (float)ml140_150.U[0]/10;
        if((Uvtag >= U_212_min)&&(Uvtag <= U_212_max)){
            tst_status = 2;
            //�������
            melody_ok();
            timer_arr[2].start_funct();
            return;
        }
    }
        tst_status = 3;
        //������� ���������� ����
        melody_bad();
        timer_arr[2].start_funct();              
}
//����� ������� ����������
void(*r2450_212_t_func[])()= {r2451_212_tst_0, r2451_212_tst_1, r2451_212_tst_2};
code unsigned char format_2450_212[]={"\n�������� �����������\n\n________________________\n\n2450.212 �______________\n����� �_________________\n\n���������� ������������:\nI������ ����� =  %3.0f ���\n\nI�����������  =  %4.0f ��\n\nU���������� �.= %6.1f B\n\n\n�������� _______________\n\n\n������� ________________\n����  %02d.%02d.%4d\n����� %02d:%02d:%02d\n"};
void r2450_212_print()
{
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,format_2450_212,
                    I_back,
                    I_cons,
                    Uvtag,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds)
                    );
    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}

void relay_I_load2450_212()
{
    r2450_212_m_func[tst_stage]();    

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&i_load[tst_stage]);
            //���.UTC1 = 50V;
            ml140_130[1].dac_ad7243 = 0xfff;//0x0cff;
            
            if(tst_stage == 2)start_regulator((int)U_212_06*10);
            else start_regulator((int)U_212*10);
            
            timer_arr3[4].start_funct(4);//2c
            tst_tact++;
            break;

        case 1:
            break;

        case 2:
            stop_regulator();
            if(tst_stage == 2)run_du_dt(dU_dt_212_06,U_dest212_06); 
            else run_du_dt(dU_dt_212,U_dest212);
            tst_tact++;
            break;

        case 3:
            if(r2450_212_w_func[tst_stage]())break;
            tst_status = 1;
            //������ ����������                                               
            if(tst_stage == 2)Uset_out180 = (int)U_dest212_06 * 10;//��������
            else Uset_out180 = (int)U_dest212 * 10;//��������
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(T_212);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 4:
            break;
        case 5:
            r2450_212_t_func[tst_stage]();
            break;
        case 6:
            if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
                if(++tst_stage > (sizeof(r2450_212_t_func)/sizeof(void*)-1)){
                    tst_stage = sizeof(r2450_212_t_func)/sizeof(void*)-1;
                    tst_tact = 128;
                    melody_final();
                    print_func = r2450_212_print;
                }
                else{
                    tst_tact = 0;
                }
            }
            else{
                tst_tact = 128;
            }
            *timer_arr3[8].tick = 0;//�������� ���������
            break;
        default:
            break;
    }

    sprintf(lcd_str,r2450_212_name[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    r2450_212_s_func[tst_stage]();
    lcd_put_str(1,0,lcd_str,0);

    sprintf(lcd_str,r2450_212_mess[tst_status]);
    if(tst_tact == 128)sprintf(lcd_str,all_right);
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
}
xdata struct c_052 u052;
xdata struct{
    float Uon1;
    float Uon2;
}U2450_052;

code struct relays relay_on[] = {{0,0x00,0x04},{0,0x00,0x08}};
code struct relays relay_off[] = {{0,0x00,0x0c},{0x01,0x00,0x1c}};
code struct relays relay_work = {0,0x00,0x28};
code struct relays relay_alt = {0,0x01,0x08};

code char r2450_052_name[][21]={
{"�������. U��������. "},
{"    ������� ����    "},
{"�������� �����������"},
{"    ������� ����    "},
{"���.���.���.�������."},
{"    ������� ����    "},
};

code char r2450_052_mess[][21]={
{" ������ ����������  "},
{"      ��������      "},
{" ����������� ������."},
{"       �����        "},
{"       ������       "},
{"   ���� ��������    "},
};

//����� ������� ���������� 
void(*r2450_052_m_func[])()= {0,0,0};

void r2450_052_s_0()
{
    /*if(tst_tact < 6 )sprintf(lcd_str,"U=%02d�  U3=%02dB U4=%02dB",ml140_150.U[2]/10,ml140_150.U[3]/10,ml140_150.U[4]/10);
    else */sprintf(lcd_str,"%05.1f� U3=%02dB U4=%02dB",(float)ml140_150.U[0]/10,ml140_150.U[3]/10,ml140_150.U[4]/10);
}
void r2450_052_s_1()
{
    sprintf(lcd_str,"%04.1f�  U3=%02dB U4=%02dB",(float)ml140_150.U[0]/10,ml140_150.U[3]/10,ml140_150.U[4]/10);
}
void r2450_052_s_2()
{
    sprintf(lcd_str,"%02dB  %5.1f�  U4=%02dB ",ml140_150.U[2]/10,(float)ml140_150.U[0]/10,ml140_150.U[4]/10);
}
void r2450_052_s_3()
{
    if(tst_tact < 3 )sprintf(lcd_str,"U=%04.1f�   U4=%04.1fB  ",(float)ml140_150.U[2]/10,(float)ml140_150.U[4]/10);
    else sprintf(lcd_str,"%05.1f�    U4=%04.1fB  ",(float)ml140_150.U[0]/10,(float)ml140_150.U[4]/10);
} 
//����� ������� �����������
void(*r2450_052_s_func[])()= {r2450_052_s_0, r2450_052_s_1, r2450_052_s_2, r2450_052_s_0, r2450_052_s_3, r2450_052_s_1};

//������� ��������� ������ ����������
void r2451_052_tst_0()
{
    if((ml140_150.U[3] > 400)&&(ml140_150.U[4] < 10)){
        tst_status = 3; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}

void r2451_052_tst_01()
{
    if((ml140_150.U[3] < 10)&&(ml140_150.U[4] < 10)){
        tst_status = 2; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}
void r2451_052_tst_02()
{
    if((ml140_150.U[0] <= u052.Umax)&&(ml140_150.U[0] >= u052.Umin)){
        tst_status = 3; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}

void r2451_052_tst_1()
{
    if((ml140_150.U[3] < 600)&&(ml140_150.U[4] > 400)){
        tst_status = 0;
        melody_ok();
    }
    else{
        if((ml140_150.U[3] > 400)&&(ml140_150.U[4] < 10)){
            tst_status = 5; 
            melody_ok();
        }
        else{
            tst_status = 4;
            melody_bad();
        }
    }
}

void r2451_052_tst_11()
{
    if((ml140_150.U[3] > 400)&&(ml140_150.U[4] < 10)){
        tst_status = 2; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}

void r2451_052_tst_2()
{
    if(ml140_150.U[4] > 400){
        tst_status = 2; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }               
}

void r2451_052_tst_3()
{
    if((ml140_150.U[0] <= u052.Umax4)&&(ml140_150.U[0] >= u052.Umin4)){
        tst_status = 3; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}

void r2451_052_tst_30()
{
    if((ml140_150.U[4] < 10)){
        tst_status = 3; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }
}

void stop_2450_052_func(unsigned char ch)
{
    if(ml140_150.U[ch] > 10){
        //�������� ����
        MTS[0].status = 2;
        MTS[2].status = 2;
        melody_ok();
    }
    else{
        if(MTS[0].status == 2)melody_bad();
    }
}

void screen()
{
    sprintf(lcd_str,r2450_052_name[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    r2450_052_s_func[tst_stage]();
    lcd_put_str(1,0,lcd_str,0);

    sprintf(lcd_str,r2450_052_mess[tst_status]);
    if(tst_tact == 128)sprintf(lcd_str,all_right);
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
}
/*
code char head1[] = {"\n�������� �����������\n\n________________________\n"};
code char *block[] ={
{"\n2450.033 �______________\n"},
{"\nEAU 2/11 �______________\n"},
{"\nEAU 4/12 �______________\n"},
{"\nEAU 11/13 �_____________\n"},
};
code char coach[] = {"����� �_________________\n\n���������� ������������:\n"};

code char booter[] = {"\n\n�������� _______________\n\n\n������� ________________\n����  %02d.%02d.%4d\n����� %02d:%02d:%02d\n"};
code unsigned char format_Umin[]={"U������������ = %6.1f B\n\nU����������   = %6.1f B\n"};

void print_boot()
{
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));
}
void r2450_033_print()
{
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,head1);
    tx.limit += sprintf(&tx.buf[tx.limit],block[relay_Umin.print]);
    tx.limit += sprintf(&tx.buf[tx.limit],coach);
    tx.limit += sprintf(&tx.buf[tx.limit],format_Umin,
                    (float)relay_Umin.Uon/10,
                    (float)relay_Umin.Uoff/10
                    );
    print_boot();
    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}
*/
//code unsigned char format_2450_052[]={"\n�������� �����������\n\n________________________\n\n2450.052__�_____________\n����� �_________________\n\n���������� ������������:\nU������������ =  %5.1f �\n\nU����������   =  %5.1f �\n\n\n�������� _______________\n\n\n������� ________________\n����  %02d.%02d.%4d\n����� %02d:%02d:%02d\n"};
code unsigned char format_2450_052[]={"2450.052__�_____________\n"};
code unsigned char format_2450_052_0[]={"\nU������������ = %5.1f B\n\nU����.���50�� = %5.1f B\n\nU����������   =  80.0 B\n"};
code unsigned char format_2450_052_1[]={"\nU������������ = %5.1f B\n\nU����������   =  80.0 B\n"};
void r2450_052_print()
{
    if((u052.Umax4 == 0)||(u052.Umin4 == 0))U2450_052.Uon2 = 0;
    tx_command = 0;//�����

    tx.limit = sprintf(tx.buf,head1);
    tx.limit += sprintf(&tx.buf[tx.limit],format_2450_052);
    tx.limit += sprintf(&tx.buf[tx.limit],coach);
    tx.limit += sprintf(&tx.buf[tx.limit],U2450_052.Uon2? format_2450_052_0 : format_2450_052_1,
                    U2450_052.Uon1,
                    U2450_052.Uon2
                    );
    print_boot();
    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}

void next_2450_052()
{            
    if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
        if((++tst_stage > 5)||((tst_stage == 3)&&((u052.Umax4 == 0)||(u052.Umin4 == 0)))){
            tst_stage--;
            melody_final();
            tst_tact = 128;
            print_func = r2450_052_print;
        }
        else{
            tst_tact = 0;
        }
    }
    else{
        tst_tact = 128;
    }
    *timer_arr3[8].tick = 0;//�������� ���������
}

void test_2450_052_0()
{
    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&relay_on[0]);
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            timer_arr3[4].start_funct(4);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            r2451_052_tst_0();
            break;
        case 3:
            commutation(&relay_on[1]);            
            timer_arr3[4].start_funct(1);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 4:
            break;
        case 5:
            r2451_052_tst_01();
            break;
        case 6:
            tst_status = 2;
            formU_du_dt(&u052.U,u052.du_dt);
            tst_tact++;
            break;
        case 7:
            stop_2450_052_func(4);
            break;
        case 8:
            r2451_052_tst_02();
            break;
        case 9:
            timer_arr3[4].start_funct(1);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 10:
            break;
        case 11:
            U2450_052.Uon1 = (float)ml140_150.U[0]/10;
            ml140_130[0].dac_ad7243 = 0;
            ml140_130[2].dac_ad7243 = 0;
            next_2450_052();
            break;
        default:
            break;
    }
    screen();
}

void test_2450_052_1()
{
    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&relay_off[0]);
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            timer_arr3[4].start_funct(4);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            r2451_052_tst_1();;
            break;
        case 3:
            commutation(&relay_off[1]);            
            //timer_arr3[4].start_funct(1);//�������� ��� ������� �������
            //������ 80�
            Uset_out180 = (unsigned int)800;
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(5);//�������� ��� ������� �������            
            tst_tact++;
            break;
        case 4:
            break;
        case 5:
            r2451_052_tst_11();
            break;
        case 6:
            ml140_130[0].dac_ad7243 = 0;
            ml140_130[2].dac_ad7243 = 0;
            next_2450_052();
            break;
        default:
            break;
    }
    screen();
}

void test_2450_052_2()
{
    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&relay_work);
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            
            //������ ����������                                               
            Uset_out180 = (unsigned int)(u052.Uwork) * 10;//��������
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(5);//�������� ��� ������� �������            
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            r2451_052_tst_2();;
            break;
        case 3:
            next_2450_052();
            break;
        default:
            break;
    }
    screen();
}

void test_2450_052_3()
{
    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&relay_alt);            
            //���.UTC1 = 50V;
            //ml140_130[1].dac_ad7243 = 0x0cff;
            Uset_out50 = 500;
            *timer_arr3[9].tick = Tdiscrete50;
            timer_arr3[4].start_funct(4);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            r2451_052_tst_30();
            break;
        case 3:
            tst_status = 2;
            formU_du_dt(&u052.U,u052.du_dt);
            tst_tact++;
            break;
        case 4:
            stop_2450_052_func(4);
            break;
        case 5:
            r2451_052_tst_3();
            break;
        case 6:
            timer_arr3[4].start_funct(1);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 7:
            break;
        case 8:
            U2450_052.Uon2 = (float)ml140_150.U[0]/10;
            ml140_130[0].dac_ad7243 = 0;
            ml140_130[2].dac_ad7243 = 0;
            next_2450_052();
            break;
        default:
            break;
    }
    screen();
}

void test_2450_052()
{
    switch(tst_stage){
        case 0: test_2450_052_0();break;
        case 1: test_2450_052_1();break;
        case 2: test_2450_052_2();break;
        case 3: test_2450_052_1();break;
        case 4: test_2450_052_3();break;
        case 5: test_2450_052_1();break;
    }        
}

xdata struct{
    float Ir0;
    float Ir55;
    float Iv0;
    float Iv55;
}I2450_056;
//float u;
code struct relays relay_056_0[] = {{0x00,0x00,0x08},{0x00,0x08,0x08},{0x00,0x00,0x08},{0x00,0x08,0x08}};

code char r2450_056_name[][21]={
{"  I r 0             "},
{"  I r 55            "},
{"  I u 0             "},
{"  I u 55            "},
};

code char r2450_056_str[][4]={"r0 ","r55","u0 ","u55"};
void r2450_056_s_0()
{
    float i;
    i = *(&I2450_056.Ir0 + tst_stage);
    //sprintf(lcd_str,"%5.1f� I%s=%6.3f��",(float)ml140_150.U[0]/10,r2450_056_str[tst_stage],i);
    //sprintf(lcd_str,"%5.1f %5.3f %6.3f  ",(float)ml140_150.U[0]/10,(float)ml140_150.U[8]/1000,i);
    sprintf(lcd_str,"%5.1f B   %7.3f ��",(float)ml140_150.U[0]/10,i);
}

void r2451_056_tst_0()
{
    float i,i_set,tol;

    i = *(&I2450_056.Ir0 + tst_stage);
    i_set = (float)(*(&Ir0 + tst_stage))/1000;
    tol = (float)(*(&Ir0_tol + tst_stage))/1000;

    if((i <= i_set+tol)&&(i >= i_set-tol)){
        tst_status = 3; 
        melody_ok();
    }
    else{
        tst_status = 4;
        melody_bad();
    }    
}


code char header[] = {"\n�������� �����������\n\n________________________\n\n2450.056 �______________\n����� �_________________\n\n���������� ������������:\n"}; 
//code char booter[] = {"\n\n�������� _______________\n\n\n������� ________________\n����  %02d.%02d.%4d\n����� %02d:%02d:%02d\n"};
code unsigned char format_2450_056[]={"%sIr   0\260C      =%6.3f ��\nIr +55\260C      =%6.3f ��\nIu   0\260C      =%6.3f ��\nIu +55\260C      =%6.3f ��\n"};
void r2450_056_print()
{
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,format_2450_056,header,
                    I2450_056.Ir0,
                    I2450_056.Ir55,
                    I2450_056.Iv0,
                    I2450_056.Iv55);                    
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));

    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}
void next_2450_056()
{            
    if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
        if(++tst_stage > 3){
            tst_stage--;
            melody_final();
            tst_tact = 128;
            print_func = r2450_056_print;
        }
        else{
            tst_tact = 0;
        }
    }
    else{
        tst_tact = 128;
    }
    *timer_arr3[8].tick = 0;//�������� ���������
}
    
void r2450_056_m_0()
{
/*    unsigned char i;
    unsigned long accu;
    unsigned int xdata*ptr;
    unsigned int result;
    //float u;

    accu = 0;
    ptr = &ml140_150.adc_arr[8][0];
    i = 32;
    do{
        accu += *ptr;
        ptr++;
    }while(--i);

    result = accu>>8;
    
    u = (float)result * (float)adc_scale[8]/10 /0xfff + (float)adc_offset[8]/1000;
    *(&I2450_056.Ir0 + tst_stage) =  u /((tst_stage<2)? ((float)Kir/1000):((float)Kiu/1000)); // ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
*/
    //*(&I2450_056.Ir0 + tst_stage) =  (float)ml140_150.U[8]/1000 /((tst_stage<2)? ((float)Kir/1000):((float)Kiu/1000)); // ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
    switch(tst_stage)
    {
        case 0: *(&I2450_056.Ir0 + tst_stage) =  (float)ml140_150.U[8]/1000 /((float)Kir/1000); break;// ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
        case 1: *(&I2450_056.Ir0 + tst_stage) =  (float)ml140_150.U[8]/1000 /((float)Kir/1000); break;// ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
        case 2: *(&I2450_056.Ir0 + tst_stage) =  (float)ml140_150.U[1]/1000 /((float)Kiu/1000); break; // ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
        case 3: *(&I2450_056.Ir0 + tst_stage) =  (float)ml140_150.U[1]/1000 /((float)Kiu/1000); break; // ������� �� 100 � ��������� �� 0.1 ��� ���� / 1000
    }
    //*(&I2450_056.Ir0 + tst_stage) = (float)ml140_150.U[8]/10/((tst_stage<2)? (7.25*0.1):(90.11*0.1));

}

void screen_02450_056()
{
    sprintf(lcd_str,r2450_056_name[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    r2450_056_s_0();
    lcd_put_str(1,0,lcd_str,0);

    sprintf(lcd_str,r2450_052_mess[tst_status]);
    if(tst_tact == 128)sprintf(lcd_str,all_right);
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
}



void test_02450_056()
{
    r2450_056_m_0();

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&relay_056_0[tst_stage]);
            
            //������ ����������                                               
            Uset_out180 = (unsigned int)(U_056) * 10;
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(5);//�������� ��� ������� �������            
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            r2451_056_tst_0();
            break;
        case 3:
            next_2450_056();
            break;
        default:
            break;
    }
    screen_02450_056();
}

xdata struct don_12x don12_;

struct Upare{
    unsigned int Uin;
    unsigned int Uup;
    unsigned int Udown;
};
code struct relays r1281= {0x00,0x01,0x00};
struct Upare Udon_up[3],Udon_down[3];
unsigned int Uold,Umax,Umin;

code char r1281_name[][21]={
{"  ���������� U���   "},
{"  ���������� U���   "},
};

void r2460_1281_jump_0()
{
    if(Uold > ml140_150.U[11]+20){
        //�������� ����
        MTS[0].status = 2;
        MTS[2].status = 2;
        melody_ok();
    }
    else{
        if((MTS[0].status == 2)||((ml140_150.U[0]/10) >= don12_.Uend_1281)){
			tst_tact = 8;//���� ������� ������� �� ����
			MTS[0].status = 2;
			MTS[2].status = 2;
		}

    }
    if(Uold < ml140_150.U[11])
        Uold = ml140_150.U[11];

}
void r2460_1281_jump_1()
{
    if(*timer_arr3[4].tick)return;
    if(Uold < ml140_150.U[11]-20){
        //�������� ����
        MTS[0].status = 2;
        MTS[2].status = 2;
        melody_ok();
    }
    else{
        if((ml140_150.U[0] - don12_.Umin_1281 < 16)||(MTS[0].status == 2)||((ml140_150.U[0]/10) <= don12_.U_1281)){
            tst_tact = 10;//���� ������� ������� �� ����
            MTS[0].status = 2;
            MTS[2].status = 2;        
        }
    }
    if(Uold > ml140_150.U[11])
        Uold = ml140_150.U[11];

}
void run_next() 
{
    //������ ����������� �������
    timer_arr3[0].start_funct(0);
    timer_arr3[2].start_funct(2);
}
void r2460_1281_s_0()
{
    struct Upare xdata * ptr; 
    
    ptr = &Udon_up + tst_stage*3+tst_iter;

    if(((tst_stage == 0)&&(tst_tact == 5))||((tst_stage == 1)&&(tst_tact == 7)))
        sprintf(lcd_str,"%5.1f  %5.1f  %5.1f ",
        (float)ptr->Uin/10,
        (float)ptr->Uup/10,
        (float)ptr->Udown/10);
    else sprintf(lcd_str,"U��=%5.1f U���=%5.1f",(float)ml140_150.U[0]/10,(float)ml140_150.U[11]/10);
}
code char header_1281[] = {"\n�������� �����������\n\n________________________\n\n2460.12__�______________\n����� �_________________\n\n���������� ������������:\n"}; 
code unsigned char format_2460_1281[]={"\nU��       = %5.1f �\nU��� \"-\"  = %5.1f �\nU��� \"+\"  = %5.1f �\n"};
void r2460_1281_print()
{
   /* struct Upare xdata * ptr;
    unsigned char i;*/
    
    tx_command = 0;//�����
    tx.limit = sprintf(tx.buf,header_1281);
    
    tx.limit += sprintf(&tx.buf[tx.limit],"\nU����     = 110 � \n");
    
    tx.limit += sprintf(&tx.buf[tx.limit],"\nUmin      = %5.1f �\nUmax      = %5.1f �\n",
        (float)Umin/10,(float)Umax/10
        );    
    /*
    tx.limit += sprintf(&tx.buf[tx.limit],"\n��� ���������� U���\n");
    ptr = &Udon_up;
    for(i=0;i<3;i++){
        if(ptr->Uin == 0)break;
        tx.limit += sprintf(&tx.buf[tx.limit],format_2460_1281,
                    (float)ptr->Uin/10,
                    (float)ptr->Uup/10,
                    (float)ptr->Udown/10);
        ptr++;                                
    }
    
    tx.limit += sprintf(&tx.buf[tx.limit],"\n��� ���������� U���\n");
    ptr = &Udon_down;
    for(i=0;i<3;i++){
        if(ptr->Uin == 0)break;
        tx.limit += sprintf(&tx.buf[tx.limit],format_2460_1281,
                    (float)ptr->Uin/10,
                    (float)ptr->Uup/10,
                    (float)ptr->Udown/10);
        ptr++;                                
    }
    */
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));

    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}
void next_2460_1281()
{            
    if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
        if(++tst_stage > 1){
            tst_stage--;
            melody_final();
            tst_tact = 128;
            print_func = r2460_1281_print;
        }
        else{
            tst_iter = 0;
            tst_tact = 0;
        }
    }
    else{
        tst_tact = 128;
    }
    *timer_arr3[8].tick = 0;//�������� ���������
}
code char m1281[][21]={
{"    U��� > Umax     "},
{"    U��� < Umin     "},
};

void screen_1281()
{
    sprintf(lcd_str,r1281_name[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    r2460_1281_s_0();
    lcd_put_str(1,0,lcd_str,0);

    if(tst_status==0)sprintf(lcd_str,"������������     %2d ",(int)tst_iter);
    else sprintf(lcd_str,&m1281[tst_status-1]);
    if(tst_tact == 128)sprintf(lcd_str,all_right);
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
}

void test_2460_1281_0()
{
    if(ml140_150.U[11] > Umax)Umax = ml140_150.U[11];

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&r1281);
            
            clr_xx((char xdata*)&Udon_up,sizeof(Udon_up));
            Umax = 0;
            Uold = 0;
            
            formU_du_dt(&don12_.U_1281,don12_.du_dt_1281);
            MTS[0].limit = value_dac(180, don12_.Uend_1281);
            MTS[2].limit = MTS[0].limit;            
            tst_tact++;
            break;
        case 1:
            r2460_1281_jump_0();
            break;
        case 2:
            timer_arr3[4].start_funct(1);//���e����
            tst_tact++;
            break;
        case 3:
            break;
        case 4:
            if(tst_iter < 3){
                Udon_up[tst_iter].Uin = ml140_150.U[0];
                Udon_up[tst_iter].Uup = Uold;
                Udon_up[tst_iter].Udown = ml140_150.U[11];
            }
            Uold = ml140_150.U[11];
            timer_arr3[4].start_funct(5);//���e����
            tst_tact++;
            break;
        case 5:
            break;
        case 6:
            tst_iter++;
            tst_tact++;
            break;
        case 7:
            run_next();//��������� ��������� ���
            tst_tact = 1;
            break;
        case 8:
            timer_arr3[4].start_funct(2);//���e����
            tst_tact++;
            break;
        case 9:
            break;    
        case 10:
            if(Umax > don12_.Umax_1281){     
                tst_status = 1;
                melody_bad();            
            }
            else melody_ok();
            break;
        case 11:
            next_2460_1281();
            break;
        default:
            break;
    }
}

void test_2460_1281_1()
{
    if(ml140_150.U[11] < Umin)Umin = ml140_150.U[11];

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&r1281);            
            //������ ����������                                               
            Uset_out180 = (unsigned int)(don12_.Uend_1281) * 10;
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(5);//�������� ��� ������� �������            
            tst_tact++;
        case 1:
            break;
        case 2:
            *timer_arr3[8].tick = 0;//�������� ���������

            clr_xx((char xdata*)&Udon_down,sizeof(Udon_down));
            Umin = ml140_150.U[11];
            Uold = ml140_150.U[11];
            
            MTS[0].value = *(unsigned long*)(&ml140_130[0].dac_ad7243);
            MTS[2].value = MTS[0].value;
            MTS[0].delta = delta_dac(900,-don12_.du_dt_1281)/2;
            MTS[2].delta = MTS[0].delta;
            MTS[0].limit = value_dac(180, don12_.U_1281);
            MTS[2].limit = MTS[0].limit;            
            //������ ����������� �������
            timer_arr3[0].start_funct(0);
            timer_arr3[2].start_funct(2);
            tst_tact++;
            break;
        case 3:
            r2460_1281_jump_1();
            break;
        case 4:
            timer_arr3[4].start_funct(1);//���e����
            tst_tact++;
            break;
        case 5:
            break;
        case 6:
            if(tst_iter < 3){
                Udon_down[tst_iter].Uin = ml140_150.U[0];
                Udon_down[tst_iter].Uup = Uold;
                Udon_down[tst_iter].Udown = ml140_150.U[11];
            }
            Uold = ml140_150.U[11];
            timer_arr3[4].start_funct(5);//���e����
            tst_tact++;
            break;
        case 7:
            break;
        case 8:
            tst_iter++;
            tst_tact++;
            break;
        case 9:
            run_next();//��������� ��������� ���
            tst_tact = 3;
            break;
        case 10:
            timer_arr3[4].start_funct(2);//���e����
            tst_tact++;
            break;
        case 11:
            break;
        case 12:
            if(Umin < don12_.Umin_1281){
                tst_status = 2;
                melody_bad();
            }
            else melody_ok();
            break;
        case 13:
            next_2460_1281();
            break;
        default:
            break;
    }
}
void test_2460_1281()
{
    switch(tst_stage){
        case 0: test_2460_1281_0(); break;
        case 1: test_2460_1281_1(); break;
    }                                     
    screen_1281();
}


xdata struct trv_027 TRV;

xdata struct{
    unsigned int Uoff;
    unsigned int Uon;
    unsigned int Ugen;
    unsigned int Ubat;
}result027;

code struct relays r027[]={ 
{0x00,0x00,0x00},
{0x00,0x02,0x00},
{0x00,0x04,0x00},
};

code char code *m027[]={
{"�������.  ����������"},
{"�����. ���� �������."},
{"�����. ���� ������� "},
};
code char m2_027_00[]= {" �������. ��������. "};
//code char m2_027_01[]= {" �������. ����������"};
code char m2_027_01[]= {"                    "};
code char m2_027_0[]= {" �������� ��������. "};
code char m2_027_1[]= {" ������. ���������! "};

code char code *m2_027[2][3]={
&m2_027_00, &m2_027_0, &m2_027_0,
&m2_027_01, &m2_027_1, &m2_027_1,
};
code char header_027[] = {"\n�������� �����������\n\n________________________\n\n2460.027 �______________\n����� �_________________\n\n���������� ������������:\n"}; 
//code unsigned char format_2460_027[]={"\nU������������ = %5.1f �\nU����������   = %5.1f �\nU����������   =  %3d ��\nU�������      =  %3d ��\n"};
code unsigned char format_2460_027[]={"\nU������������ = %5.1f �\nU����������   =  %3d ��\nU�������      =  %3d ��\n"};
void r2460_027_print()
{
    tx_command = 0;//�����
    
    tx.limit = sprintf(tx.buf,header_027);
    tx.limit += sprintf(&tx.buf[tx.limit],format_2460_027,
                    (float)result027.Uoff/10,
                    //(float)result027.Uon/10,
                    result027.Ugen,
                    result027.Ubat);                    
    tx.limit += sprintf(&tx.buf[tx.limit],booter,
                    (int)bcd_to_bin(datetime.date),(int)bcd_to_bin(datetime.mounth),(int)bcd_to_bin(datetime.year)+2000,
                    (int)bcd_to_bin(datetime.hours),(int)bcd_to_bin(datetime.minutes),(int)bcd_to_bin(datetime.seconds));

    tx_count = 0;//�������� �������� ��������� �����
    TI0 = 1;
}
void next_2460_027()
{            
    if(tst_regime == 0){//���� ����� �������� �� ������� �� ��������� ����
        if(++tst_stage > 2){
            tst_stage--;
            melody_final();
            tst_tact = 128;
            print_func = r2460_027_print;
        }
        else{
            tst_iter = 0;
            tst_tact = 0;
        }
    }
    else{
        tst_tact = 128;
    }
    *timer_arr3[8].tick = 0;//�������� ���������
    ml140_130[0].dac_ad7243 = 0;
    ml140_130[2].dac_ad7243 = 0;
            
    MTS[3].status = 2;//�������� �����
    ml140_140.dac_ad5312 = 0;
}


void screen_027()
{
    sprintf(lcd_str,m027[tst_stage]);
    lcd_put_str(0,0,lcd_str,0);

    if(tst_stage == 0)sprintf(lcd_str,"U��=%5.1f U���=%5.1f",(float)ml140_150.U[0]/10,(float)ml140_150.U[11]/10);
    //else sprintf(lcd_str,"%3.0fB U���=%3.0fB %3.0f��",(float)ml140_150.U[0]/10,(float)ml140_150.U[11]/10,(float)ml140_140.dac_ad5312*2.5/0x0fff*1000);
    else switch(tst_stage){
        case 1: sprintf(lcd_str,"%3.0fB U���=%3.0fB %3d��",(float)ml140_150.U[0]/10,(float)ml140_150.U[11]/10, ml140_150.U[10]); break;
        case 2: sprintf(lcd_str,"%3.0fB U���=%3.0fB %3d��",(float)ml140_150.U[0]/10,(float)ml140_150.U[11]/10, ml140_150.U[9]); break;
    }

    lcd_put_str(1,0,lcd_str,0);

    
    switch(tst_tact){
        case 128:   sprintf(lcd_str,all_right); break;
        case 255:   sprintf(lcd_str,"     � � � � � �    "); break;
        default:
            sprintf(lcd_str,m2_027[tst_status][tst_stage]); break;
    }
    lcd_put_str(2,0,lcd_str,0);
    
    end_test_func();
}

void test_2460_027_0()
{

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&r027[tst_stage]);

            formU_du_dt(&TRV.U,TRV.du_dt);
            timer_arr3[4].start_funct(1);//���e����
            tst_tact++;
            break;
        case 1:
            break;
        case 2:
            //if(ml140_150.U[11] < ml140_150.U[0]-20){
			if((ml140_150.adc_arr[11][0] < 0x300)||(ml140_150.adc_arr[11][15] < 0x300)){
                //�������� ����
                MTS[0].status = 2;
                MTS[2].status = 2;
                melody_ok();
            }
            else{
                if(MTS[0].status == 2)melody_bad();
            }
            break;
        case 3:
            tst_status = 1;
            timer_arr3[4].start_funct(4);//���e����
            tst_tact++;
            break;
        case 4:
            break;
        case 5:
            result027.Uoff = ml140_150.U[0];
            //tst_tact++;
			tst_tact = 10;
            break;
        case 6:
            MTS[0].delta = -MTS[0].delta;
            MTS[2].delta = -MTS[2].delta;
            MTS[0].limit = 0; 
            MTS[2].limit = 0;            
            //������ ����������� �������
            timer_arr3[0].start_funct(0);
            timer_arr3[2].start_funct(2);
            tst_tact++;
            break;
        case 7:
            if(ml140_150.U[11] > 10){
                //�������� ����
                MTS[0].status = 2;
                MTS[2].status = 2;
                melody_ok();
            }
            else{
                if(MTS[0].status == 2)melody_bad();
            }
            break;
        case 8:
            timer_arr3[4].start_funct(4);//���e����
            tst_tact++;
            break;
        case 9:
            break;
        case 10:
            result027.Uon = ml140_150.U[0];
            tst_tact++;
            break;
        case 11:
            //TRV.Utest = (result027.Uoff + result027.Uon)/2;//����� ������� �������
            tst_tact++;
        case 12:
            if((result027.Uoff >= TRV.Uoff - TRV.Utol)&&(result027.Uoff <= TRV.Uoff + TRV.Utol)//&&    
               /*(result027.Uon >= TRV.Uon - TRV.Utol)&&(result027.Uon <= TRV.Uon + TRV.Utol)*/){
                melody_ok();
            }
            else melody_bad();
            break;
        case 13:
            next_2460_027();
            break;
        default:
            break;
    }
}

void test_2460_027_1()
{
    unsigned int Uup,Udown;

    switch(tst_tact){
        case 0:
            tst_status = 0;
            commutation(&r027[tst_stage]);
            //������ ����������                                               
            Uset_out180 = TRV.Utest;//��������
            *timer_arr3[8].tick = Tdiscrete180;
            timer_arr3[4].start_funct(4);//�������� ��� ������� �������
            tst_tact++;
            break;
        case 1:
            break;//������ �� ���������
        case 2:
            MTS[3].delta = 0x0fffffff/op_sec*TRV.du_dt_/2500;
            MTS[3].value = 0x0000;//������ � ����
            MTS[3].limit = 0x028F;//�������� 400��
            //������ ����������� �������
            timer_arr3[0].start_funct(3);
            tst_tact++;
            break;
        case 3:
            //if(ml140_150.U[11] < ml140_150.U[0]-100){
			if((ml140_150.adc_arr[11][0] < 0x300)||(ml140_150.adc_arr[11][15] < 0x300)){            
                //�������� ����
                MTS[3].status = 2;
                melody_ok();
            }
            else{
                if(MTS[3].status == 2)melody_bad();
            }
            break;
        case 4:
            tst_status = 1;
            timer_arr3[4].start_funct(4);//���e����
            tst_tact++;
            break;
        case 5:
            break;
        case 6:
            switch(tst_stage){
                case 1: *(&result027.Ugen + tst_stage-1) = ml140_150.U[10];//��������� � ��
                    break;
                case 2: *(&result027.Ugen + tst_stage-1) = ml140_150.U[9];//��������� � ��
                    break;
            }
            tst_tact++;
            break;
        case 7:
            switch(tst_stage){
                case 1:
                    Uup = TRV.UshG + TRV.UtG;
                    Udown = TRV.UshG - TRV.UtG;
                    break;
                case 2:
                    Uup = TRV.UshB + TRV.UtB;
                    Udown = TRV.UshB - TRV.UtB;
                    break;                
            }
            if((*(&result027.Ugen + tst_stage-1) >= Udown)&&
               (*(&result027.Ugen + tst_stage-1) <= Uup)){
                 melody_ok();
            }
            else melody_bad();
            break;
        case 8:
            next_2460_027();
            break;
        default:
            break;
    }
}
void test_2460_027()
{
    switch(tst_stage){
        case 0: test_2460_027_0(); break;
        case 1: test_2460_027_1(); break;
        case 2: test_2460_027_1(); break;
    }                                     
    screen_027();
}
void stop_50_180(unsigned int xdata *U)
{
    switch(keys_pop()){
        case SS:
            clr_bit(EIE2,ET4);
            keys_push(SS);
            set_bit(EIE2,ET4);
            break;
        case up:
            (*U)++;
            break;
        case down:            
            if(*U)(*U)--;
            break;
    }
}

void mts_50_run()
{
    switch(tst_tact){
        case 0:
            *timer_arr3[9].tick = Tdiscrete50;
            tst_tact++;
            break;
        case 1:
            sprintf(lcd_str,"U �������� = %5.1f � ",(float)Uset_out50/10);
            lcd_put_str(1,0,lcd_str,0);
            sprintf(lcd_str,"   U���. = %5.1f �  ",(float)ml140_150.U[2]/10);
            lcd_put_str(2,0,lcd_str,0);
            stop_50_180(&Uset_out50);
            break;
    }
}


void mts_180_run()
{
    switch(tst_tact){
        case 0:
            *timer_arr3[8].tick = Tdiscrete180;
            tst_tact++;
            break;
        case 1:
            sprintf(lcd_str,"U �������� = %5.1f � ",(float)Uset_out180/10);
            lcd_put_str(1,0,lcd_str,0);
            sprintf(lcd_str,"      U���.= %5.1f �",/*(float)ml140_150.U[1]/10,*/(float)ml140_150.U[0]/10);
            lcd_put_str(2,0,lcd_str,0);
            stop_50_180(&Uset_out180);
            break;
    }
}
void mts_run()
{
    switch(tst_stage){
        case 0: mts_50_run(); break;
        case 1: mts_180_run(); break;
    }                                     
    sprintf(lcd_str," C�����������       ");
    lcd_put_str(0,0,lcd_str,0);

    sprintf(lcd_str,"          CC : �����");
    lcd_put_str(3,0,lcd_str,0);
}

code char on_off_str[]={"                ��� ������ �� ���������� ��������� ��������� � �������� ���� ����������                    "};
char i_str=0;

void alarm_text()
{

    if(*timer_arr3[10].tick)return;
    *timer_arr3[10].tick = 33;
    
    sprintf(lcd_str,"   � � � � � � !!!  ");
    lcd_put_str(0,0,lcd_str,0);
    sprintf(lcd_str,"���������� ����  ���");
    lcd_put_str(1,0,lcd_str,0);
    sprintf(lcd_str,"��������   ���������");
    lcd_put_str(2,0,lcd_str,0);
    
    sprintf(lcd_str,&on_off_str[i_str]);
    lcd_str[20] = 0;
    lcd_put_str(3,0,lcd_str,0);

    if(++i_str >= sizeof(on_off_str)-21)i_str = 0;
}
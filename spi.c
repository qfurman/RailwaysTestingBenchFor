#include <c8051f120.h>
#include <intrins.h>
#include "macro.h"
#include "definition.h"
#include "asm_sub.h"
#include "menu.h"

sfr MAC0CF = 0xc3;//вставив бо компілятор помінявся а в новому цього регістра нема
#define MAC0_PAGE         0x03         // MAC 0

sbit ld0 = P2^0;
sbit ld1 = P2^1;
sbit ld2 = P2^2;
sbit ld3 = P2^3;
sbit ldm = P2^4;
sbit ldrg = P2^5;
sbit rck = P2^6;
sbit g = P2^7;
sbit csadc = P3^0;
sbit r_c = P3^1;

sbit f_0 = P1^0;
sbit f_1 = P1^1;
sbit f_2 = P1^2;
sbit f_3 = P1^3;

xdata struct{
//використовується для організації циклічного буфера з 128 двобайтних  вибірок ацп
    unsigned int adc_arr[16][32];
    signed int U[16];
    unsigned char sample;//0..31 - 32 вибірки по два байти
    unsigned char channel;//0..15
    unsigned int adc_index;//для циклічного буфера адреса призначення вибірки
    unsigned char reg;
    unsigned int scale;//подільник 0 (1:1), 1 (1:10), 2 (1:20)
    signed int offset;
}ml140_150 _at_ 0x0000;

xdata struct{
    unsigned char task_number;
    unsigned char ml410_130_stage;
    unsigned char ml410_140_stage;
    unsigned char ml410_150_stage;
    unsigned char ml410_160_stage;
}spi_management;

//12-бітний цап але код 0х0000 до 0х0cff
xdata struct{
    unsigned int dac_ad7243; 
}ml140_130[3];

xdata struct{
    unsigned char relay16_23;
    unsigned char relay8_15;
    unsigned char relay0_7;
    unsigned int dac_ad5312;
}ml140_140;

xdata struct{
    unsigned char cool;
}ml140_160;

void spi0_init()
{
    unsigned char xdata *ptr;
    unsigned char i;
    unsigned int k;
    
    i = sizeof(spi_management);
    ptr = (char*)&spi_management;
    do{*ptr = 0;ptr++;}while(--i);

    i = sizeof(ml140_130);
    ptr = (char*)&ml140_130;
    do{*ptr = 0;ptr++;}while(--i);

    i = sizeof(ml140_140);
    ptr = (char*)&ml140_140;
    do{*ptr = 0;ptr++;}while(--i);

    WDTCN = 0xA5;//RELOAD WATCHDOG
    k = sizeof(ml140_150);
    ptr = (char*)&ml140_150;
    do{*ptr = 0;ptr++;}while(--k);
    
    i = sizeof(ml140_160);
    ptr = (char*)&ml140_160;
    do{*ptr = 0;ptr++;}while(--i);
    
    //ml140_140.dac_ad5312 = 0x0000;    

    SPIF = 1;        
}

void cool_control()
{
    unsigned char i;

    for(i=0;i<(sizeof(ml140_130)/2);i++)
         if(ml140_130[i].dac_ad7243)set_bit(ml140_160.cool,i);
         else clr_bit(ml140_160.cool,i);
}

void spi_410_130();
void spi_410_140();
void spi_410_150();
void spi_410_160();
//головна фішка сезону - масив функцій які використовують spi
//мають перевіряти spi на готовність, якщо зайнятий то з вийти функції
void(*spi_func[10])()= {spi_410_130,spi_410_140,spi_410_150,spi_410_160,0,0,0,0,0,0};

void spi_ISR() interrupt 6 using 2
{
    SPIF = 0;
    spi_func[spi_management.task_number]();
}

void spi_set_regime(unsigned char regime)
{
    unsigned char back_sfr;

    back_sfr = SFRPAGE; 
    SFRPAGE   = SPI0_PAGE;

    switch(regime){
        //0: Data centered on first edge of SCK period.
        //0: SCK line low in idle state.
        case 0: SPI0CFG &= ~0x30; break;
        //0: Data centered on first edge of SCK period.
        //1: SCK line high in idle state.
        case 1: SPI0CFG &= ~0x30; SPI0CFG |= 0x10; break;
        //1: Data centered on second edge of SCK period.
        //0: SCK line low in idle state.
        case 2: SPI0CFG &= ~0x30; SPI0CFG |= 0x20; break;
        //1: Data centered on second edge of SCK period.
        //1: SCK line high in idle state.
        case 3: SPI0CFG |= 0x30; break; 
    }
    
    SFRPAGE   = back_sfr;
}


void spi_410_130()
{
    static unsigned char mts;//для трьох

    if(mts > 2)mts = 0x00;

    switch(spi_management.ml410_130_stage){
        case 0:
            spi_set_regime(1);//0 був став 1 бо змінився цап
            spi_management.ml410_130_stage++;
            switch(mts){
                case 0:ld0 = 1;break;//cs
                case 1:ld1 = 1;break;//cs
                case 2:ld2 = 1;break;//cs
            }
            SPI0DAT = ~(*(unsigned char*)&ml140_130[mts].dac_ad7243);
            break;
        case 1:
            spi_management.ml410_130_stage++;
            SPI0DAT = ~(*((unsigned char*)&ml140_130[mts].dac_ad7243+1));
            break;
        case 2:
            spi_management.ml410_130_stage = 0;
            switch(mts){
                case 0:ld0 = 0;break;//фіксація даних
                case 1:ld1 = 0;break;//фіксація даних
                case 2:ld2 = 0;break;//фіксація даних
            }
            switch(mts){
                case 0:ld0 = 1;break;//cs
                case 1:ld1 = 1;break;//cs
                case 2:ld2 = 1;break;//cs
            }
            if(++mts == 3)spi_management.task_number++;
            //f1 = 0;
            SPIF = 1;
            break;
    }    
}

void spi_410_140()
{
    switch(spi_management.ml410_140_stage){
        case 0:
            spi_set_regime(1);
            spi_management.ml410_140_stage++;
            rck = 1;
            ld3 = 0;
            SPI0DAT = ~ml140_140.relay16_23;
            break;
        case 1:
            spi_management.ml410_140_stage++;
            SPI0DAT = ~ml140_140.relay8_15;
            break;
        case 2:
            spi_management.ml410_140_stage++;
            SPI0DAT = ~ml140_140.relay0_7;
            break;
        case 3:
            rck = 0;//фіксація даних в трьох релейних регістрах
            spi_set_regime(0);
            spi_management.ml410_140_stage++;
            rck = 1;
            ld3 = 1;//cs для ad5312
            SPI0DAT = ~(unsigned char)(ml140_140.dac_ad5312>>8);
            break;
        case 4:
            spi_management.ml410_140_stage++;
            SPI0DAT = ~(unsigned char)ml140_140.dac_ad5312;
            break;
        case 5:
            spi_management.ml410_140_stage = 0;
            spi_management.task_number++;
            ld3 = 0;//cs для ad5312 фіксація даних
            SPIF = 1;
            break;
    }
}
sfr16 MAC0A = 0xc1;
sfr16 MAC0B = 0x91;
sfr16 MAC0RND = 0XCE;//16- БІТНИЙ РЕГІСТР ОКРУГЛЕННЯ
void spi_410_150()
{
    switch(spi_management.ml410_150_stage){
        case 0:
            r_c = 1;//зупуск перетворення ацп
            csadc = 1;
            
            spi_set_regime(1);
            spi_management.ml410_150_stage++;
            
            //переключення каналів
            ml140_150.channel = (ml140_150.channel + 1) & 0x0f;//0..15
            if(ml140_150.channel == 0)ml140_150.sample = (ml140_150.sample + 1) & 0x1f;//0..31
            // 0000 0xxx - 0010 0xxx, 0000 1xxx - 0100 0xxx    
            ml140_150.reg = (ml140_150.channel & 0x08)? 
            (0x40 | (ml140_150.channel&0x07)) : (0x20 | (ml140_150.channel & 0x07));

            ldrg = 1;
            csadc = 0;
            r_c = 0;
            SPI0DAT = ml140_150.reg;
            break;
        case 1:
            ldrg = 0;
            csadc = 1;//вибір ацп
            spi_management.ml410_150_stage++;
            spi_set_regime(3);
            ldrg = 1;//фіксація байту в регістрі
            SPI0DAT = 0;//dummy
            break;
        case 2:
            spi_management.ml410_150_stage++;
            *((char*)(&ml140_150.adc_arr)+ ml140_150.adc_index) = SPI0DAT;
            //для тестів
            //*((unsigned int*)((char*)&ml140_150.adc_arr + ml140_150.adc_index)) = 0x0fff;//(ml140_150.channel * 0x100 - 1) & 0x0fff;//симуляція
            //*((unsigned int*)((char*)&ml140_150.adc_arr + ml140_150.adc_index)) = (ml140_150.channel * 0x100 - 1) & 0x0fff;//симуляція
            ml140_150.adc_index++;
            SPI0DAT = 0;//dummy
            break;
        case 3:
            spi_management.ml410_150_stage = 0;
            spi_management.task_number++;
            *((char*)(&ml140_150.adc_arr)+ ml140_150.adc_index) = SPI0DAT & 0xfc;
            csadc = 0;//un-вибір ацп
/*
АЦП код 12-біт приходить зміщеним на два біта вліво тому вони відсікаються маскою 0xfc (див.вище),

порібний код | подільник АЦП | масштабний коеф.на який множити| зсув і напрям | байти з результатом|
0x0064         1                100                             4->             rnd
0x03e8         10               1000                            4->             rnd
0x07d0         20               2000                            4->             rnd

*/
            //використання множення з накопиченням
            SFRPAGE = MAC0_PAGE;
            set_bit(MAC0CF,MAC0CA);//обнулити акамулятор
            //масштабування макимальний код АЦП відповідає 10.0В помножимо на 100
            MAC0A = ml140_150.scale;
            //передача адреси даних для обробки
            mac(ml140_150.adc_index&0xffc0, ml140_150.offset);//функція усереднення використовуючи апаратний помножувач з акамуляцією
            //для одердання 16-бітового результату  два зсуви ліворуч
            set_bit(MAC0CF,MAC0SD);//напрям зсуву ->
            set_bit(MAC0CF,MAC0SC);//зсув на один біт
            _nop_();
            set_bit(MAC0CF,MAC0SC);//зсув на один біт
            _nop_();
            set_bit(MAC0CF,MAC0SC);//зсув на один біт
            _nop_();
            set_bit(MAC0CF,MAC0SC);//зсув на один біт
            _nop_();
            _nop_();
                                     
            ml140_150.U[(ml140_150.channel-1) & 0x0f] = MAC0RND;
            SFRPAGE = SPI0_PAGE;
            //обчислення адреси для наступної вибікки таким чином щоб не було виходу за межі буфера в 1024 байти;
            //32семпли * 2байти = 64
            ml140_150.adc_index = ((unsigned int)ml140_150.channel * 64 + ml140_150.sample * 2) & 0x03ff;
            ml140_150.scale = adc_scale[ml140_150.channel];
            ml140_150.offset = adc_offset[ml140_150.channel];
            SPIF = 1;
            break;
    }
}


void spi_410_160()
{
    switch(spi_management.ml410_160_stage){
        case 0:
            spi_set_regime(0);
            spi_management.ml410_160_stage++;
            ldm = 0;
            SPI0DAT = ml140_160.cool;
            break;
        case 1:
            ldm = 1;//фіксація даних регістрі вентиляторів
            spi_management.ml410_160_stage = 0;
            spi_management.task_number = 0;
            ldm = 0;
            SPIF = 1;
            break;
    }
}


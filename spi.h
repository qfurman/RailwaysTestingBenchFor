
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

extern xdata struct{
//використовується для організації циклічного буфера з 128 двобайтних  вибірок ацп
    unsigned int adc_arr[16][32];
    signed int U[16];
    unsigned char sample;//0..31 - 32 вибірки по два байти
    unsigned char channel;//0..15
    unsigned int adc_index;//для циклічного буфера адреса призначення вибірки
    unsigned char reg;
    unsigned int scale;//подільник 0 (1:1), 1 (1:10), 2 (1:20)
    signed int offset;
}ml140_150;

extern xdata struct{
    unsigned char task_number;
    unsigned char ml410_130_stage;
    unsigned char ml410_140_stage;
    unsigned char ml410_150_stage;
    unsigned char ml410_160_stage;
}spi_management;

extern xdata struct{
    unsigned int dac_ad7243; 
}ml140_130[3];

extern xdata struct{
    unsigned char relay16_23;
    unsigned char relay8_15;
    unsigned char relay0_7;
    unsigned int dac_ad5312;
}ml140_140;

extern xdata struct{
    unsigned char cool;
}ml140_160;

void spi0_init();
void cool_control();




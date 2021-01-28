
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

extern xdata struct DAC MTS[],MKS;

code struct virtual_timer_p timer_arr3[];


void timer_3_init();

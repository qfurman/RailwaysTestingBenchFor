

/******************************************************************************/
//віртуальний таймер
typedef struct virtual_timer
{
    unsigned int xdata*tick;//відліки до нуля
    void(*start_funct)();   //фунція запуску
    void(*end_funct)();     //дія після досягнення нуля
};
/******************************************************************************/
extern code struct virtual_timer timer_arr[];
/******************************************************************************/


void timer_4_init();
/******************************************************************************/
typedef struct repeat{
    unsigned int first;
    unsigned int all_next;
};

extern xdata unsigned char io;

extern xdata struct repeat key_temp;
extern code struct repeat key_set;//зразкові інтервали перша 100*10мс = 1с всі наступні 20*10мс = 200мс
void keys_push(unsigned char key_);
unsigned char keys_pop();

/******************************************************************************/
typedef struct melody{
    unsigned char nota;     //12нот х 6 октав = 0..71; 128 - пауза 255 - кінець
    unsigned int duration; //1000-1с; 500-1/2с; 125-1/4с; 8-1/8с.
};
extern code struct melody death[];
extern code struct melody yesterday[];
extern code struct melody ole[];
extern code struct melody beep[];
extern code struct melody uuuup[];
extern code struct melody final[];
extern xdata struct melody *melody_ptr;    //сюди треба записати початок мелодії і запустити мелодійний таймер
/******************************************************************************/
extern xdata unsigned char lcd_blink_flag;
/******************************************************************************/
/******************************************************************************/
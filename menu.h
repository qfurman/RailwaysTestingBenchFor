
extern xdata struct menu_status{//маю намір вписувати сюди поточний стан меню
    unsigned char input_row;    //рядок де відбувається модифікація
    unsigned char fetch; //0 - звичайний режим 1- необхідно захопити рядок 2- відбулося захоплення рядка
    void (*alt)();//якщо  не дорівнює нулю то дисплей використовується альткрнативною функцією ареса її сюди і записана
}menu_status;

void clr_xx(unsigned char xdata *xptr_dest, unsigned char count);//обнуляє память

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

//всі змінні описуються тут
extern xdata unsigned int rs232_timer1_reload;
extern xdata unsigned int adc_scale[];
extern xdata int    adc_offset[];
extern xdata unsigned int Tdiscrete180;
extern xdata unsigned int Tdiscrete50;
extern xdata unsigned int  Kir,
                    Kiu;
extern xdata signed int du_dt_0_mts0,
                 du_dt_0_mts1,
                 du_dt_0_mts2;
extern xdata struct Umin_relay b_2450_033;
extern xdata struct Umin_relay EAU_2_11;
extern xdata struct Umin_relay EAU_4_12;
extern xdata struct Umin_relay EAU_11_13;
extern xdata unsigned char Ustart_2450_33_t,
                   du_dt_2450_33_t;
extern xdata unsigned int Udest_2450_33_t;
extern xdata unsigned char Udelta_2450_33_t,
                   pause_2450_33_t;
extern xdata unsigned char U_212,
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
extern xdata struct c_052 cc052[];
extern xdata unsigned char U_056;
extern xdata unsigned int Ir0,
                    Ir55,
                    Iv0,
                    Iv55;//56
extern xdata unsigned char Ir0_tol_old,
                    Ir55_tol_old,
                    Iv0_tol_old,
                    Iv55_tol_old;
extern xdata struct trv_027 trv_[];
extern xdata struct don_12x don_12_[];

extern xdata unsigned int Ir0_tol,
                    Ir55_tol,
                    Iv0_tol,
                    Iv55_tol;

void menu_init();
void processing(void);
void reload();
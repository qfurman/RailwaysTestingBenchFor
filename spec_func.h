
char dayOfWeek(int dayOfYear, int year);
int dayOfYear(char day, char month, int year);

long delta_dac(unsigned int Umax, int du_dt);//всі цілі *10 (один умовний знак після коми)

extern xdata unsigned int Uset_out180;
void PI_regulator();
extern xdata unsigned int Uset_out50;
void PI_regulator50();
extern unsigned char tst_stage,tst_iter,tst_tact,tst_status,tst_regime;


struct relay_umin{
    unsigned char test;//номер різновиду тесту
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

extern struct relay_umin relay_Umin;
extern xdata struct trv_027 TRV;
extern xdata struct c_052 u052;
extern xdata struct don_12x don12_;
extern char alarm_was;


void leds_and_alarm();
void relay_Umin_2450_033_u();
void relay_Umin_2450_033_t();
void ml520_test();
void relay_I_load2450_212();
void test_2450_052();
void test_02450_056();

extern void(*print_func)();
void test_2460_1281();
void test_2460_027();
void relay_Umin_test();

void mts_run();
void alarm_text();





extern xdata struct Tdatetime
{
	unsigned char	seconds;
	unsigned char	minutes;
	unsigned char	hours;
	unsigned char	day;
	unsigned char	date;
	unsigned char	mounth;
	unsigned char	year;
	unsigned char	calibrate;
} datetime,datetime_w;

extern xdata struct
{
    unsigned char task_number;//номер задачі
    unsigned char kill_after_execution;//зняти після виконання
}i2c_management;

extern xdata struct{
    unsigned char addr;
    unsigned char count;
    unsigned char xdata* ptr;
} i2cdata,i2cdata_w;

void i2c_init();
void i2c_reset();
void I2Csetdata();
void I2Cgetdata();
void settime();

void(*i2c_func[])();//потік і2с функцій закінчується нулем
void i2c_process();//запуск і2с функцій з черги
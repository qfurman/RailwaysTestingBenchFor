
sbit CTS = P3^6;
extern xdata unsigned char printer_not_ready;
void cts_process();

typedef struct uart{
    unsigned char buf[1024];
    unsigned int limit;
};

typedef struct uart_{
    char *ptr;
    unsigned int count;
};

typedef struct uart_g{
    unsigned char column;//кількість горизонтальних байтів
    unsigned char row;//кількість рядків у зображенні
    char code*ptr;//вказує на зображення
    unsigned int size;//кількість байтів зображення
};

extern xdata struct uart rx,tx;
extern xdata struct uart_ tx_,rx_;
extern xdata struct uart_g tx_g;
extern xdata unsigned char tx_command;
extern unsigned int tx_count;//для зображення - кількісь переданих байтів
extern unsigned char tx_comm_count;//лічильник для комнандної інформації
extern xdata unsigned char printer_not_ready;

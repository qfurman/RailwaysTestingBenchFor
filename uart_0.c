#include <c8051f120.h>

#include "menu.h"
#include "timer_3.h"

typedef struct uart{
    unsigned char buf[1024];//обовязково виправити і в h-файлі
    unsigned int limit;
};

typedef struct uart_{
    char *ptr;
    unsigned int count;
};

typedef struct uart_g{
    unsigned char column;//кількість горизонтальних байтів
    unsigned char row;//кількість рядків у зображенні
    unsigned char code*ptr;//вказує на зображення
    unsigned int size;//кількість байтів зображення
};

xdata struct uart tx,rx;
xdata struct uart_ tx_,rx_;
xdata struct uart_g tx_g;

sbit CTS = P3^6;
xdata unsigned char printer_not_ready = 0;
xdata unsigned char tx_command;//0-uart 1-uart_ 2-uart_g графіка

void HandleReceiveInterrupt(void);
void HandleTransmitInterrupt(void);

void cts_process()
{
    if(printer_not_ready && !CTS){
        printer_not_ready = 0;
        TI0 = 1;
    }
}

void uart0_isr (void) interrupt 4 using 1
{
    if  (TI0)    {TI0 = 0; HandleTransmitInterrupt ();}
    if  (RI0)    {RI0 = 0; HandleReceiveInterrupt ();}
}

void HandleReceiveInterrupt(void)
{

}

unsigned int tx_count;//для зображення - кількісь переданих байтів
unsigned char tx_comm_count;//лічильник для комнандної інформації


void HandleTransmitInterrupt(void)
{
    if(CTS){
        printer_not_ready = 1;
        *timer_arr3[6].tick = 200;//через 1,00c зняти задачу
        return;
    }

    switch(tx_command){
        case 0:
            if(tx_count < tx.limit)SBUF0 = tx.buf[tx_count++];
            break;
        case 1:
            if(tx_.ptr){
                if(tx_.count){
                    SBUF0 = *tx_.ptr;
                    tx_.ptr++;
                    tx_.count--;
                }
                else tx_.ptr = 0;
            }
            break;
        case 2:
            if(tx_count % tx_g.column == 0){//Графіка
                switch(tx_comm_count++){//всі рядки
                    case 0: if(tx_count < tx_g.size)SBUF0 = 0x1b;return;
                    case 1: SBUF0 = 0x57;return;
                    case 2: SBUF0 = tx_g.column;return;//кількість байтів на один рядок
                    case 3: SBUF0 = 0;return;//зміщення
                    case 4: break;
                }
                //запуск рядка
                SBUF0 = *tx_g.ptr; 
                tx_g.ptr++; 
                tx_count++; 
                tx_comm_count = 0;
            }
            else{
                SBUF0 = *tx_g.ptr; 
                tx_g.ptr++; 
                tx_count++;                                 
            }
            break;
    }
}
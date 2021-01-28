#include <c8051f120.h>

#include "menu.h"
#include "timer_3.h"

typedef struct uart{
    unsigned char buf[1024];//���������� ��������� � � h-����
    unsigned int limit;
};

typedef struct uart_{
    char *ptr;
    unsigned int count;
};

typedef struct uart_g{
    unsigned char column;//������� �������������� �����
    unsigned char row;//������� ����� � ���������
    unsigned char code*ptr;//����� �� ����������
    unsigned int size;//������� ����� ����������
};

xdata struct uart tx,rx;
xdata struct uart_ tx_,rx_;
xdata struct uart_g tx_g;

sbit CTS = P3^6;
xdata unsigned char printer_not_ready = 0;
xdata unsigned char tx_command;//0-uart 1-uart_ 2-uart_g �������

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

unsigned int tx_count;//��� ���������� - ������ ��������� �����
unsigned char tx_comm_count;//�������� ��� ��������� ����������


void HandleTransmitInterrupt(void)
{
    if(CTS){
        printer_not_ready = 1;
        *timer_arr3[6].tick = 200;//����� 1,00c ����� ������
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
            if(tx_count % tx_g.column == 0){//�������
                switch(tx_comm_count++){//�� �����
                    case 0: if(tx_count < tx_g.size)SBUF0 = 0x1b;return;
                    case 1: SBUF0 = 0x57;return;
                    case 2: SBUF0 = tx_g.column;return;//������� ����� �� ���� �����
                    case 3: SBUF0 = 0;return;//�������
                    case 4: break;
                }
                //������ �����
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
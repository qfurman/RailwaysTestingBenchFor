
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
    unsigned char column;//������� �������������� �����
    unsigned char row;//������� ����� � ���������
    char code*ptr;//����� �� ����������
    unsigned int size;//������� ����� ����������
};

extern xdata struct uart rx,tx;
extern xdata struct uart_ tx_,rx_;
extern xdata struct uart_g tx_g;
extern xdata unsigned char tx_command;
extern unsigned int tx_count;//��� ���������� - ������ ��������� �����
extern unsigned char tx_comm_count;//�������� ��� ��������� ����������
extern xdata unsigned char printer_not_ready;

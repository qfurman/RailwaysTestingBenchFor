

/******************************************************************************/
//���������� ������
typedef struct virtual_timer
{
    unsigned int xdata*tick;//����� �� ����
    void(*start_funct)();   //������ �������
    void(*end_funct)();     //�� ���� ���������� ����
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
extern code struct repeat key_set;//������� ��������� ����� 100*10�� = 1� �� ������� 20*10�� = 200��
void keys_push(unsigned char key_);
unsigned char keys_pop();

/******************************************************************************/
typedef struct melody{
    unsigned char nota;     //12��� � 6 ����� = 0..71; 128 - ����� 255 - �����
    unsigned int duration; //1000-1�; 500-1/2�; 125-1/4�; 8-1/8�.
};
extern code struct melody death[];
extern code struct melody yesterday[];
extern code struct melody ole[];
extern code struct melody beep[];
extern code struct melody uuuup[];
extern code struct melody final[];
extern xdata struct melody *melody_ptr;    //���� ����� �������� ������� ����䳿 � ��������� ��������� ������
/******************************************************************************/
extern xdata unsigned char lcd_blink_flag;
/******************************************************************************/
/******************************************************************************/
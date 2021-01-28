#include <c8051f120.h>

/*
������� ������ ����������� ����� ��������� � ���
����� ��� - ����� ��� �� 128 ���������� �������
��������� � 40-������ ���������� � ���� 1 � 2
*/
void mac(register unsigned int point)
{
                      
    #pragma asm
        mov     DPH,r6  
        mov     DPL,r7
        mov     r1,#32
    mac_loop:
        MOVX    A,@DPTR
        MOV     MAC0BH,A
        INC     DPTR;
        MOVX    A,@DPTR
        MOV     MAC0BL,A
        INC     DPTR;
    djnz    r1,mac_loop
    #pragma endasm

    point = 0;
    
}
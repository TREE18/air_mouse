#ifndef __SMOOTH_H
#define __SMOOTH_H
#include "sys.h"
#include "usart.h"	
#define RINGBUFF_LEN 2
#define FLASE 0
#define TRUE 1

typedef struct
{
    u16 Head;           
    u16 Tail;
    u16 Lenght;
    u8 Ring_Buff[RINGBUFF_LEN];
}RingBuff_t;

void RingBuff_Init(void);
//u8 Read_RingBuff(u8 *rData);
u8 Write_RingBuff(u8 data,RingBuff_t *ringBuff);
unsigned char filter1(u8 ad);
unsigned char filter2(u8 ad);

void TIM3_Int_Init(u16 arr,u16 psc);
#endif


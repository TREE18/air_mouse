#include "smooth.h"
#include "string.h"
#include "24l01.h"
#include "key.h"

//#define N 2
//#define x 8
//#define y 8

char tim_flag = 0; 
u8 KEY_TIM3_time = 0;
u8 ROLL_TIM3_Temp = 0;
char volatile ROLL_Allow_Flag;
extern u8 volatile tmp_buf[5];	 //����Ĵ���
//extern char KEY_TIM3_Allow;

//unsigned char value_buf[N];
//unsigned char value_buf2[N];
//unsigned char temp = 0;
//unsigned char temp2 = 0;
/*
unsigned char filter1(u8 ad)
{
//	unsigned char i;
//	unsigned char value;
//	int sum=0;
	
	if(ad < 2 )return 0;
//	if( ad < x ) return ad/2;
	
	return ad;
}

unsigned char filter2(u8 ad)
{
//	unsigned char i;
//	unsigned char value;
//	int sum=0;
	
	if(ad < 2 )return 0;
//	if( ad < y ) return ad/2;

	return ad;
}
*/


void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM3, ENABLE);				 
}

//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־
		
		if(tim_flag == 1)
		{	
			KEY_TIM3_time++;
											
			if(KEY_TIM3_time >= 18)
			{
				KEY_TIM3_time = 0;
				tim_flag = 0; //�ָ�������ȡ���ꡣ
//				printf("key_tim3 is ok\n");
//				KEY_TIM3_Allow = 0; //reflash
			}
		}
		
		ROLL_TIM3_Temp++;
		if( ROLL_TIM3_Temp >= 6)
		{
			ROLL_TIM3_Temp = 0;
			
//			ROLL_Allow_Flag = 1;
		    ROLL_Scan();
																	//NRF24L01_TxPacket(tmp_buf);//reflash
		}
	}
}



RingBuff_t ringBuff1;
void RingBuff_Init(void)
{
       ringBuff1.Head = 0;
       ringBuff1.Tail = 0;
       ringBuff1.Lenght = 0;
}
 
u8 Write_RingBuff(u8 data,RingBuff_t *ringBuff)
{
//   if(ringBuff->Lenght >= RINGBUFF_LEN)
//    {
//      return FLASE;
//    }
    ringBuff->Ring_Buff[ringBuff->Tail]=data;
//    ringBuff.Tail++;//�޷���ֹԽ�����
    ringBuff->Tail = (ringBuff->Tail+1)%RINGBUFF_LEN;
 //  ringBuff->Lenght++;
	
//	printf("Write_RingBuff is %d\n",data);
    return TRUE;
}


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
extern u8 volatile tmp_buf[5];	 //发射寄存器
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

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);				 
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志
		
		if(tim_flag == 1)
		{	
			KEY_TIM3_time++;
											
			if(KEY_TIM3_time >= 18)
			{
				KEY_TIM3_time = 0;
				tim_flag = 0; //恢复正常读取坐标。
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
//    ringBuff.Tail++;//无法防止越界访问
    ringBuff->Tail = (ringBuff->Tail+1)%RINGBUFF_LEN;
 //  ringBuff->Lenght++;
	
//	printf("Write_RingBuff is %d\n",data);
    return TRUE;
}


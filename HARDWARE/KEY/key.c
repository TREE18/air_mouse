#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "usart.h"
#include "24l01.h"

extern char tim_flag_before;
extern char tim_flag_next;
extern u8 volatile tmp_buf[5];	 //发射寄存器

int ROLL_Value_Before,Value_diff,ROLL_Value_Now;
char ROLL_i = 0;

TIM_ICInitTypeDef  TIM4_ICInitStructure;
void ROLL_Tim4_init()
{
	// TIM4
    // PB6 ch1  A,PB7 ch2   
    // TIMxCLK = 36MHZ
	
//	NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 0xFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision =TIM_CKD_DIV1 ;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,  TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);
		
    TIM_ICStructInit(&TIM4_ICInitStructure);
    TIM4_ICInitStructure.TIM_ICFilter = 6;//ICx_FILTER;
    TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设
//	
//	TIM_ITConfig(  //使能或者失能指定的TIM中断
//		TIM4, //TIM2
//		TIM_IT_Update  |  //TIM 中断源
//		TIM_IT_Trigger,   //TIM 触发中断源 
//		ENABLE  //使能
//		);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
}

//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);    
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	
	GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_5;//PB3~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!

u8 KEY_Scan(u8 mode1,u8 mode2)
{	 
	static u8 key_up1=1;//其余按键按松开标志
	if(mode1)key_up1=1;  //支持连按
	
	if(key_up1&&(KEY1==0||KEY2==0))
	{
		delay_ms(5);//去抖动 
		key_up1=0;
		
		tim_flag_before = 1;
		
		if(KEY1==0) return 2;
		else if(KEY2==0) return 1;
	}
	else if(KEY1==1&&KEY2==1)
	{	
		key_up1=1;
//		key_up2=1;
		tim_flag_before = 0;//reflash
	}
 	return 0;// 无按键按下
}

u8 CenterKey_Scan(u8 mode)
{
	static u8 key_up=1;
	u8 KeyTime=0;
	if(mode) key_up=1;
	if(key_up&&CENTER==0)
	{
		delay_ms(2);
		key_up=0;
		if(CENTER==0){
			while(CENTER==0){
				delay_ms(2);
				++KeyTime;
				if(KeyTime>=KEYTIME) return 2;
			}
			return 1;
		}
		
	}
	else if(CENTER==1) key_up=1;
	return 0;
}

u8 FunKey_Scan(u8 mode)
{
	static u8 key_up=1;
	u8 KeyTime=0;
	if(mode) key_up=1;
	if(key_up&&FUN==0)
	{
		delay_ms(2);
		key_up=0;
		if(FUN==0){
			while(FUN==0){
				delay_ms(2);
				++KeyTime;
				if(KeyTime>=KEYTIME) return 2;
			}
			return 1;
		}
		
	}
	else if(FUN==1) key_up=1;
	return 0;
}

void ROLL_Scan()
{
	tmp_buf[3]=0x00;
	if(ROLL_i == 0){ROLL_i++; ROLL_Value_Before = TIM_GetCounter(TIM4); }
	else 
	{
		ROLL_Value_Now = TIM_GetCounter(TIM4);
		Value_diff = ROLL_Value_Now - ROLL_Value_Before;
//		printf("ROLL_Value_Now is %d\r\n",ROLL_Value_Now);
				
		if(Value_diff > 0)	
		{
			if(ROLL_Value_Now >= 254 && ROLL_Value_Before <= 2)tmp_buf[3]=0xff;
			else tmp_buf[3]=0x01;
//			printf("ROLL_Value_Now is %d\r\n",ROLL_Value_Now);
		}
		else if(Value_diff < 0)
		{
			if(ROLL_Value_Before >= 254 && ROLL_Value_Now <= 2)tmp_buf[3]=0x01;
			else tmp_buf[3]=0xff;
//			printf("ROLL_Value_Now is %d\r\n",ROLL_Value_Now);
		}
		
		if(ROLL_Value_Before != ROLL_Value_Now)ROLL_Value_Before = ROLL_Value_Now;//reflash
	}
}

/*
void TIM2_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure1;

	TIM_DeInit(TIM2);
	printf("bbb1\r\n");
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	printf("bbb2\r\n");
	//定时器TIM3初始化
	NVIC_InitStructure1.NVIC_IRQChannel = TIM2_IRQn;  //TIM3中断	//中断优先级NVIC设置
	printf("bbb5\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级0级
	printf("bbb6\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 1;  //从优先级3级
	printf("bbb7\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	printf("bbb8\r\n");
	NVIC_Init(&NVIC_InitStructure1);  //初始化NVIC寄存器
	printf("bbb9\r\n");
	
	
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	printf("bbb3\r\n");
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
	printf("bbb4\r\n");


	TIM_Cmd(TIM2, DISABLE);		
	printf("bbb10\r\n");	
}
*/
/*
//定时器3中断服务程序
void TIM2_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		
		ROLL_Scan();
		NRF24L01_TxPacket(tmp_buf);//reflash
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 
//		printf("tim2/r/n");
	}
}
*/

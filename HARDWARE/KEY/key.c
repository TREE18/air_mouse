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
extern u8 volatile tmp_buf[5];	 //����Ĵ���

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
	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx����
//	
//	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
//		TIM4, //TIM2
//		TIM_IT_Update  |  //TIM �ж�Դ
//		TIM_IT_Trigger,   //TIM �����ж�Դ 
//		ENABLE  //ʹ��
//		);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3�ж�
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
//	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);    
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	
	GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_5;//PB3~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��KEY3���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!

u8 KEY_Scan(u8 mode1,u8 mode2)
{	 
	static u8 key_up1=1;//���ఴ�����ɿ���־
	if(mode1)key_up1=1;  //֧������
	
	if(key_up1&&(KEY1==0||KEY2==0))
	{
		delay_ms(5);//ȥ���� 
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
 	return 0;// �ް�������
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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��
	printf("bbb2\r\n");
	//��ʱ��TIM3��ʼ��
	NVIC_InitStructure1.NVIC_IRQChannel = TIM2_IRQn;  //TIM3�ж�	//�ж����ȼ�NVIC����
	printf("bbb5\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
	printf("bbb6\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 1;  //�����ȼ�3��
	printf("bbb7\r\n");
	NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	printf("bbb8\r\n");
	NVIC_Init(&NVIC_InitStructure1);  //��ʼ��NVIC�Ĵ���
	printf("bbb9\r\n");
	
	
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	printf("bbb3\r\n");
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�
	printf("bbb4\r\n");


	TIM_Cmd(TIM2, DISABLE);		
	printf("bbb10\r\n");	
}
*/
/*
//��ʱ��3�жϷ������
void TIM2_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		
		ROLL_Scan();
		NRF24L01_TxPacket(tmp_buf);//reflash
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
//		printf("tim2/r/n");
	}
}
*/

#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 

// 
//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define KEY3 PAin(0)	//PA0  WK_UP
 
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)//读取左键
#define KEY2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)//读取右键
//#define UP GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)//读取上键
//#define DOWN GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)//读取下键
#define FUN GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) //读取功能键
#define CENTER GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取中键

#define KEYTIME 200


#define KEY_LEFT		1
#define KEY_RIGHT		2
#define KEY_UP    	3
#define KEY_DOWN  	4
#define KEY_CENTER	5
#define KEY_FUN 6
#define KB_Value	0xff

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode1,u8 mode2);  	//按键扫描函数		
u8 CenterKey_Scan(u8 mode);//中间按键
u8 FunKey_Scan(u8 mode);
void ROLL_Tim4_init(void);
void ROLL_Scan(void);
//void TIM2_Int_Init(u16 arr,u16 psc);
//void ROLL_Scan(void);
#endif

#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "string.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "I2C.h"
#include "mpu6050.h"
#include "spi.h"
#include "24l01.h"
#include "anbt_dmp_fun.h"
#include "anbt_i2c.h"
#include "anbt_dmp_mpu6050.h"
#include "anbt_dmp_driver.h"
#include "led.h"
#include "smooth.h"
/***
version:v3
mender:TREE
time:2019-4-28
note:由于v2-4版本中的tim2定时器在初始化时NVIC_Init()处 卡住无法初始化导致程序无法运行，原因不详（猜测与key按键引脚配置冲突�
故将所有tim2中断程序放入tim3中一并处理。
***/
#define q30  1073741824.0f
extern char tim_flag;
extern RingBuff_t ringBuff1;
extern char volatile ROLL_Allow_Flag;

volatile u8  tmp_buf[5];	 //发射寄存器
char tim_flag_before = 0;
char tim_flag_next = 0;
//char KEY_TIM3_Allow = 0;
//计算x1,x2的绝对值
u32 usb_abs(u32 x1,u32 x2)
{
	if(x1>x2)return x1-x2;
	else return x2-x1;
}
//设置USB 连接/断线
//enable:0,断开
//       1,允许连接	   
void usb_port_set(u8 enable)
{  	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	if(enable)_SetCNTR(_GetCNTR()&(~(1<<1)));//退出断电模式
	else
	{	  
		_SetCNTR(_GetCNTR()|(1<<1));  // 断电模式
		GPIOA->CRH&=0XFFF00FFF;
		GPIOA->CRH|=0X00033000;
		//PAout(12)=0;    		  
	}
}  	

static void DMP_Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
	
}

 int main(void)
 {	

		short temp[3];
		int dip = 0;
	 
		u8 fun;
		u8 key,key1;
		u8 i=0;
		u8 spot_flag=0;
	 /**/
		delay_init();	    	 //延时函数初始化	  
		NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
		uart_init(9600);	 	//串口初始化为9600
	 /**/printf("aaa1\r\n");
	    KEY_Init();	 			 //按键初始化

	 /**/printf("aaa2\r\n");
		ANBT_I2C_Configuration();		//引用圆点博士的I2C程序,IIC初始化
		DMP_Delay (500);        		//	delay_ms(30); 	
		AnBT_DMP_MPU6050_Init();			//6050DMP初始化
	 /**/printf("aaa3\r\n");
		TIM3_Int_Init(200,7199);

		RingBuff_Init(); // ！对列初始化
		printf("aaa4\r\n");
		ROLL_Tim4_init();
		printf("aaa5\r\n");
		
	/**/
		//I2C_Init();
		//delay_ms(40);
		//MPU6050_Init();
		//delay_ms(40);
		printf("aaa6\r\n");
		NRF24L01_Init(); 
		printf("aaa7\r\n");
		while(NRF24L01_Check())	//
		{
				delay_ms(200);
		}
		NRF24L01_TX_Mode();

        printf("aaawhile\r\n");
		
		while(1)
		{
				for(i = 0 ;i<6 ;i++)//初始化tmp_buf
				{
					if(i == 3)continue;
					tmp_buf[i] = 0x00;
				}	
				
				MPU6050ReadGyro(temp);
				
						fun = FunKey_Scan(0);
						if(fun == 1){
							dip++;
							if(dip==4)dip=-1;
							
							printf("dip is %d\r\n",dip);
						}
						
						key1=CenterKey_Scan(0);
						if(key1==1&&spot_flag==0) 
						{
							tmp_buf[4]=1;
						}//轨迹识别
						else if(key1==2) 
						{		
							tmp_buf[4]=2;
							spot_flag=~spot_flag;
							printf("%d",spot_flag);
						}//突出显示
						else if(key1==1&&spot_flag==0xff)
						{
							tmp_buf[4]=3;
							printf("1");
						}
			
				key=KEY_Scan(1,1);//支持连按
				Write_RingBuff(tim_flag_before, &ringBuff1);
				if(key != 0)
				{
					printf(" THE KEY IS %d\r\n",key);
					if(key==KEY_LEFT){
						
								tmp_buf[0]=0X01;
						
								if(ringBuff1.Ring_Buff[0] != ringBuff1.Ring_Buff[1])	//按键按下拖动判断
								{
							
									tim_flag = 1;//	TIM_Cmd(TIM3, DISABLE);	KEY_TIM3_Allow = 1;	TIM_Cmd(TIM3, ENABLE);													
								}									
					}			//鼠标左键    
					else if(key==KEY_RIGHT){
						
								tmp_buf[0]=0X02;
						
								if(ringBuff1.Ring_Buff[0] != ringBuff1.Ring_Buff[1])	
								{
									tim_flag = 1;
								}
								
					}		//鼠标右键

				}
				else{
								if(ringBuff1.Ring_Buff[0] != ringBuff1.Ring_Buff[1])	
								{
									tim_flag = 1;
								}		
				}		
						
				if(tim_flag == 0)
				{
					tmp_buf[1]=(u8)((-temp[2]/15)-257);	
					tmp_buf[2]=(u8)((temp[0]/15)+4);
				}
				else 
				{
					tmp_buf[1] = 0 ;
					tmp_buf[2] = 0 ;
				}
				
				
				
				NRF24L01_TxPacket(tmp_buf);//reflash
				tmp_buf[3] = 0x00;
				delay_ms(20+(dip*10));
		}
	
}	   										    			    



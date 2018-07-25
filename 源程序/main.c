#include "main.h"
#include "uart.h"
#include "LCD12864_S.h"
#include "DS18B20.h"



//常量
#define Success 1U
#define Failure 0U

code char phoneNumber[] = "15934827892";		//替换成需要被拨打电话的号码
char msg[20] = "HIGH TEMP:00";		//短信内容


#define TEMP_MAX 30
#define CO_MAX 30

//定义变量
unsigned long  Time_Cont = 0;       //定时器计数器

unsigned char state = 0;
unsigned char state_pre = 0;

unsigned int Temp_Buffer = 0;
unsigned char key_num = 0;
bit flag_sw = 1;
unsigned int ADC_Buffer = 0;	

void main()
{
	ADC_CONTR = ADC_360T | ADC_ON;
	AUXR1 |= ADRJ;									//ADRJ = 1;			//10bitAD右对齐

  Uart_Init();

	LcmInit();								//液晶初始化
	LcmClearTXT();							//清除显示
	LcmClearBMP();
  delay_ms(100);
	PutStr(0,0,"。。初始化中。。");	


	if (sendCommand("AT\r\n", "OK\r\n", 3000, 10) == Success);
	else errorLog();
	delay_ms(10);

	if (sendCommand("AT+CPIN?\r\n", "READY", 1000, 10) == Success);
	else errorLog();
	delay_ms(10);

	if (sendCommand("AT+CREG?\r\n", "CREG: 1", 1000, 10) == Success);
	else errorLog();
	delay_ms(10);

	if (sendCommand("AT+CMGF=1\r\n", "OK\r\n", 1000, 10) == Success);
	else errorLog();
	delay_ms(10);

	if (sendCommand("AT+CSCS=\"GSM\"\r\n", "OK\r\n", 1000, 10) == Success);
	else errorLog();
	delay_ms(10);

	Temp_Buffer = Get_temp();
	delay_ms(1000);
	LcmClearTXT();							//清除显示

	while(1)
	{	
		Temp_Buffer = Get_temp();
    PutStr(0,0,"温度：");	
	
 		if( Temp_Buffer/1000 != 0 )					//如果第一位为0，忽略显示
 		{
 			WriteData(Temp_Buffer/1000+0X30);	   //显示温度百位值
 		}
 		else
 		{
 			if(flag_temper == 1)						//根据温度标志位显示温度正负
 			{
 				WriteData('-');
 			}
 			else
 			{
 				WriteData(' ');	
 			}	   
 		}
 			
 		WriteData(Temp_Buffer%1000/100+0X30);	   //显示温度十位值
 		WriteData(Temp_Buffer%100/10+0X30);	   //显示温度个位值
 		WriteData('.');						   //显示小数点
 		WriteData(Temp_Buffer%10+0X30);		   //显示温度十分位值
 		WriteData(' ');
		PutStr(0,7,"℃");

		if(flag_temper == 0)
		{
			if(Temp_Buffer/10 >= TEMP_MAX)
			{
				state = 1;
				PutStr(1,0,"温度超标");				
			}
			else
			{
				state = 0;
				PutStr(1,0,"温度合适");
			}

			if(Temp_Buffer/10 <= TEMP_MAX - 1) 	//低于报警值1度后发送短信功能才复位，防止不停的发短信
			{
				state_pre = 0;
			}
		}

		if(state == 1&& state_pre == 0)
		{
			memset(msg,0,20);       //短信语句
      sprintf(msg,"HIGH TEMP:  %d",Temp_Buffer/10);
			sendMessage(phoneNumber,msg);		//发送短信
			state_pre = state;	
		}
		ADC_Buffer = adc10_start(0);		// P1.0 ADC
    PutStr(2,0,"CO浓度：");	
		ADC_Buffer = ADC_Buffer*100/1024;
		WriteData(ADC_Buffer%100/10+0X30);
		WriteData(ADC_Buffer%10+0X30);
			if(ADC_Buffer >= CO_MAX)
			{
				state = 1;
				PutStr(3,0,"CO浓度超标");
			}
			else
			{
				state = 0;
				PutStr(3,0,"CO浓度合适");
			}

			if(Temp_Buffer/10 <= TEMP_MAX - 1) 	//低于报警值1度后发送短信功能才复位，防止不停的发短信
			{
				state_pre = 0;
			}
		
			if(state == 1&& state_pre == 0)
		{
			memset(msg,0,20);       //短信语句
      sprintf(msg,"HIGH CO:  %d",ADC_Buffer);
			sendMessage(phoneNumber,msg
			);		//发送短信
			state_pre = state;	
		}		
		delay_ms(500);

	}
}


//****************************************************
//做一次ADC转换
//****************************************************
unsigned int adc10_start(unsigned char channel)	//channel = 0~7
{
	unsigned int	adc;
	unsigned char	i;

	P1ASF = (1 << channel);			//12C5A60AD/S2系列模拟输入(AD)选择

	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = (ADC_CONTR & 0xe0) | ADC_START | channel;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	 

//	for(i=0; i<250; i++)		//13T/loop, 40*13=520T=23.5us @ 22.1184M
	i = 250;
	do{
		if(ADC_CONTR & ADC_FLAG)
		{
			ADC_CONTR &= ~ADC_FLAG;				//软件清零ADC_FLAG
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			adc = 0;
			adc = (ADC_RES << 8) | ADC_RESL;	//ADRJ_enable()


			return	adc;
		}
	}while(--i);
	return	1024;
}

void sendMessage(char *number,char *msg)
{
	xdata char send_buf[20] = {0};
	memset(send_buf, 0, 20);    //清空
	strcpy(send_buf, "AT+CMGS=\"");
	strcat(send_buf, number);
	strcat(send_buf, "\"\r\n");
	if (sendCommand(send_buf, ">", 3000, 10) == Success);
	else errorLog();

	SendString(msg);

	SendData(0x1A);
}

void phone(char *number)
{
	char send_buf[20] = {0};
	memset(send_buf, 0, 20);    //清空
	strcpy(send_buf, "ATD");
	strcat(send_buf, number);
	strcat(send_buf, ";\r\n");

	if (sendCommand(send_buf, "SOUNDER", 10000, 10) == Success);
	else errorLog();
}

void errorLog()
{
	while (1)
	{
	  	if (sendCommand("AT\r\n", "OK", 100, 10) == Success)
		{
			soft_reset();
		}
		delay_ms(200);
	}
}

void soft_reset(void)	 //制造重启命令
{
   ((void (code *) (void)) 0x0000) ();
}

unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	unsigned char n;
	CLR_Buf();
	for (n = 0; n < Retry; n++)
	{
		SendString(Command); 		//发送GPRS指令

		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			delay_ms(100);
			Time_Cont += 100;
			if (strstr(Rec_Buf, Response) != NULL)
			{
				
				CLR_Buf();
				return Success;
			}
			
		}
		Time_Cont = 0;
	}
	
	CLR_Buf();
	return Failure;
}

//****************************************************
//MS延时函数(12M晶振下测试)
//****************************************************
void delay_ms(unsigned int n)
{
	unsigned int  i,j,m;
	for(m = 0 ; m < 10 ; m++)
		for(i=0;i<n;i++)
			for(j=0;j<123;j++);
}
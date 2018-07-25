#include "uart.h"

#define Uart1_Buf_Max 100               //串口数据缓存长度
u8 xdata  Rec_Buf[Uart1_Buf_Max];  //串口数据缓存
u8 point1 = 0;             //绶存指针


void Uart_Init()	//115200@11.0592M				  			   
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;		//设定定时初值
	TH1 = 0xFD;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
	EA   = 1;     //打开全局中断控制
	ES   = 1;     //串行口中断	
}


/*----------------------------
发送串口数据
----------------------------*/
void SendData(unsigned char ch)
{
    SBUF = ch;                 //写数据到UART数据寄存器
		while(TI == 0);
		TI = 0;
}

/*----------------------------
发送字符串
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}

bit Hand(unsigned char *a)                   // 串口命令识别函数
{ 
    if(strstr(Rec_Buf,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // 串口缓存清理
{
	memset(Rec_Buf, 0, Uart1_Buf_Max);      //清空

    point1 = 0;                    
}

void RECEIVE_DATA(void) interrupt 4 using 1         //串口中断	  		
{    
	ES = 0;
	if (RI)
    {
      	RI = 0;                                //清除RI位
		Rec_Buf[point1] = SBUF; 
//		if (Rec_Buf[0] == 0xd9)
//			{
//				IAP_CONTR = 0x60;
//			}
			
		point1++;               
		if(point1>=Uart1_Buf_Max)          
		{
			point1 = 0;
		}           

    }
	ES =  1;

}





















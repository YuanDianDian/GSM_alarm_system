#ifndef __UART_H__
#define __UART_H__

#include "STC12C5A.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;


//函数或者变量声明
void Uart_Init();
void SendData(unsigned char ch);
void SendString(char *s);
bit Hand(unsigned char *a);
void CLR_Buf(void);


extern u8 xdata  Rec_Buf[];  //串口数据缓存
extern u8 point1;             //绶存指针





#endif
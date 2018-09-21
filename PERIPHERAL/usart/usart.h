#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define Usart1RxLen	256	
#define Usart1TxLen	256

#define Usart2RxLen	512	
#define Usart2TxLen	512


extern u16 Usart1RxCnt;					//串口1接收到的数据长度
extern u16 OldUsart1RxCnt;				//用于比较
extern u16 Usart1FrameLen;				//串口1接收到整帧数据的长度
extern u8 Usart1RxBuf[Usart1RxLen];		//串口1接收数据缓存
extern u8 Usart1TxBuf[Usart1TxLen];		//串口1发送数据缓存
extern u8 Usart1RecvEnd;				//串口1接收一帧数据完成标志
extern u8 Usart1Busy; 					//串口1忙标志
extern u16 Usart1SendLen;				//串口1将要发送的字符串长度
extern u16 Usart1SendNum;				//串口1已经发送数据的长度

extern u16 Usart2RxCnt;					//串口2接收到的数据长度
extern u16 OldUsart2RxCnt;				//用于比较
extern u16 Usart2FrameLen;				//串口2接收到整帧数据的长度
extern u8 Usart2RxBuf[Usart2RxLen];		//串口2接收数据缓存
extern u8 Usart2TxBuf[Usart2TxLen];		//串口2发送数据缓存
extern u8 Usart2RecvEnd;				//串口2接收一帧数据完成标志
extern u8 Usart2Busy; 					//串口2忙标志
extern u16 Usart2SendLen;				//串口2将要发送的字符串长度
extern u16 Usart2SendNum;				//串口2已经发送数据的长度
	  	

void USART1_Init(u32 bound);
void USART2_Init(u32 bound);
void Usart1ReciveFrameEnd(void);
void Usart2ReciveFrameEnd(void);

void Usart1FrameSend(void);	
void Usart2FrameSend(void);	
void TIM2_Init(u16 arr,u16 psc);

u8 UsartSendString(USART_TypeDef* USARTx,u8 *str, u16 len);

#endif



#include "sys.h"
#include "usart.h"
#include "common.h"
#include "bg96.h"


u16 Usart1RxCnt = 0;
u16 OldUsart1RxCnt = 0;
u16 Usart1FrameLen = 0;
u8 Usart1RxBuf[Usart1RxLen];
u8 Usart1TxBuf[Usart1TxLen];
u8 Usart1RecvEnd = 0;
u8 Usart1Busy = 0;
u16 Usart1SendLen = 0;
u16 Usart1SendNum = 0;

u16 Usart2RxCnt = 0;
u16 OldUsart2RxCnt = 0;
u16 Usart2FrameLen = 0;
u8 Usart2RxBuf[Usart2RxLen];
u8 Usart2TxBuf[Usart2TxLen];
u8 Usart2RecvEnd = 0;
u8 Usart2Busy = 0;
u16 Usart2SendLen = 0;
u16 Usart2SendNum = 0;

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕
    USART2->DR = (u8)ch;
	return ch;
}
#endif

void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	USART_Cmd(USART1, DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);		//使能USART1，GPIOA时钟
	USART_DeInit(USART1);  															//复位串口1

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 										//PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;									//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); 											//初始化PA9

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;										//PA.10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;							//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  										//初始化PA10

	USART_InitStructure.USART_BaudRate = bound;										//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

	USART_Init(USART1, &USART_InitStructure); 										//初始化串口
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//开启中断
	USART_Cmd(USART1, ENABLE);                    									//使能串口
}

void USART2_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	USART_Cmd(USART2, DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);							//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);							//使能USART2时钟
	USART_DeInit(USART2);  															//复位串口1

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 										//PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;									//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); 											//初始化PA2

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;										//PA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;							//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  										//初始化PA10

	USART_InitStructure.USART_BaudRate = bound;										//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

	USART_Init(USART2, &USART_InitStructure); 										//初始化串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);									//开启中断
	USART_Cmd(USART2, ENABLE);                    									//使能串口
}

u8 UsartSendString(USART_TypeDef* USARTx,u8 *str, u16 len)
{
	u16 i;
	for(i=0; i<len; i++)
    {
		USART_ClearFlag(USARTx,USART_FLAG_TC);
		USART_SendData(USARTx, str[i]);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
		USART_ClearFlag(USARTx,USART_FLAG_TC);
	}
	return 1;
}

void USART1_IRQHandler(void)
{
	u8 rxdata;

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  	{
		rxdata =USART_ReceiveData(USART1);

		if(Usart1RxCnt<Usart1RxLen && Usart1Busy == 0)
		{
			Usart1RxBuf[Usart1RxCnt]=rxdata;
			Usart1RxCnt++;

			if(Usart1RxCnt == 8)
			{
				Usart1RxCnt = Usart1RxCnt;
			}
		}
  	}

	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
	{
		Usart1FrameSend();
	}

	//以下为串口中断出错后的处理  经验之谈
	else if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
		rxdata = USART_ReceiveData(USART1);
		rxdata = rxdata;
		USART_ClearFlag(USART1, USART_FLAG_ORE);
	}
	else if(USART_GetFlagStatus(USART1, USART_FLAG_NE) != RESET)
	{
		USART_ClearFlag(USART1, USART_FLAG_NE);
	}
	else if(USART_GetFlagStatus(USART1, USART_FLAG_FE) != RESET)
	{
		USART_ClearFlag(USART1, USART_FLAG_FE);
	}
	else if(USART_GetFlagStatus(USART1, USART_FLAG_PE) != RESET)
	{
		USART_ClearFlag(USART1, USART_FLAG_PE);
	}
}

void USART2_IRQHandler(void)
{
	u8 rxdata;

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  	{
		if(bg96->init_ok == 1)
		{
			bg96->uart_interrupt_event(&bg96);
		}
		else
		{
			rxdata =USART_ReceiveData(USART2);
		}
  	}

	if(USART_GetITStatus(USART2,USART_IT_TC)!=RESET)
	{
		Usart2FrameSend();
	}

	//以下为串口中断出错后的处理  经验之谈
	else if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	{
		rxdata = USART_ReceiveData(USART2);
		rxdata = rxdata;
		USART_ClearFlag(USART2, USART_FLAG_ORE);
	}
	else if(USART_GetFlagStatus(USART2, USART_FLAG_NE) != RESET)
	{
		USART_ClearFlag(USART2, USART_FLAG_NE);
	}
	else if(USART_GetFlagStatus(USART2, USART_FLAG_FE) != RESET)
	{
		USART_ClearFlag(USART2, USART_FLAG_FE);
	}
	else if(USART_GetFlagStatus(USART2, USART_FLAG_PE) != RESET)
	{
		USART_ClearFlag(USART2, USART_FLAG_PE);
	}
}

void Usart1ReciveFrameEnd(void)
{
	if(Usart1RxCnt)
	{
		if(OldUsart1RxCnt == Usart1RxCnt)
		{
			Usart1FrameLen = Usart1RxCnt;
			OldUsart1RxCnt = 0;
			Usart1RxCnt = 0;
			Usart1RecvEnd = 0xAA;
		}
		else
		{
			OldUsart1RxCnt = Usart1RxCnt;
		}
	}
}

void Usart2ReciveFrameEnd(void)
{
	if(Usart2RxCnt)
	{
		if(OldUsart2RxCnt == Usart2RxCnt)
		{
			Usart2FrameLen = Usart2RxCnt;
			OldUsart2RxCnt = 0;
			Usart2RxCnt = 0;
			Usart2RecvEnd = 0xAA;
		}
		else
		{
			OldUsart2RxCnt = Usart2RxCnt;
		}
	}
}

void Usart1FrameSend(void)
{
	u8 send_data = 0;
	send_data = Usart1TxBuf[Usart1SendNum];
	USART_SendData(USART1,send_data);
	Usart1SendNum ++;
	if(Usart1SendNum >= Usart1SendLen)					//发送已经完成
	{
		Usart1Busy = 0;
		Usart1SendLen = 0;								//要发送的字节数清零
		Usart1SendNum = 0;								//已经发送的字节数清零
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);	//关闭数据发送中断
		memset(Usart1TxBuf,0,Usart1TxLen);
	}
}

void Usart2FrameSend(void)
{
	u8 send_data = 0;
	send_data = Usart2TxBuf[Usart2SendNum];
	USART_SendData(USART2,send_data);
	Usart2SendNum ++;
	if(Usart2SendNum >= Usart2SendLen)					//发送已经完成
	{
		Usart2Busy = 0;
		Usart2SendLen = 0;								//要发送的字节数清零
		Usart2SendNum = 0;								//已经发送的字节数清零
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);	//关闭数据发送中断
		memset(Usart2TxBuf,0,Usart2TxLen);
	}
}

void TIM2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 		//时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 					//设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 				//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 			//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM2,TIM_IT_Update ,ENABLE);

	TIM_Cmd(TIM2, ENABLE);  									//使能TIMx外设
}

void TIM2_IRQHandler(void)
{
	static u8 tick_10ms = 0;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 			//检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  			//清除TIMx的中断待处理位:TIM 中断源

		SysTick10msAdder();			//10ms滴答计数器累加

		Usart1ReciveFrameEnd();		//检测USART1是否接收数据结束

		tick_10ms ++;
		if(tick_10ms >= 10)
		{
			tick_10ms = 0;

			SysTick100msAdder();	//100ms滴答计数器累加
		}
	}
}




























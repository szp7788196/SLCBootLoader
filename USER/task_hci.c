#include "task_hci.h"
#include "delay.h"
#include "usart.h"
#include "inventr.h"
#include "at_protocol.h"


TaskHandle_t xHandleTaskHCI = NULL;

void vTaskHCI(void *pvParameters)
{
	u16 send_len1 = 0;
	u16 send_len4 = 0;
	
	AT_CommandInit();
	
	UsartSendString(USART1,"READY\r\n", 7);
	
	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;
			
			send_len1 = AT_CommandDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf,HoldReg);
			
			memset(Usart1RxBuf,0,Usart1FrameLen);
		}
		
		if(Usart4RecvEnd == 0xAA)
		{
			Usart4RecvEnd = 0;
			
			send_len4 = AT_CommandDataAnalysis(Usart4RxBuf,Usart4FrameLen,Usart4TxBuf,HoldReg);
			
			memset(Usart4RxBuf,0,Usart4FrameLen);
		}
		
		if(send_len1 != 0)
		{
			UsartSendString(USART1,Usart1TxBuf, send_len1);
			
			memset(Usart1TxBuf,0,send_len1);
			
			send_len1 = 0;
		}
		if(send_len4 != 0)
		{
			UsartSendString(UART4,Usart4TxBuf, send_len4);
			
			memset(Usart4TxBuf,0,send_len4);
			
			send_len4 = 0;
		}
		
		delay_ms(100);
	}
}







































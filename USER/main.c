#include "common.h"
#include "24cxx.h"
#include "led.h"
#include "usart.h"
#include "rtc.h"
#include "ota.h"

//u16 i = 0;
//u8 eepbuf[256];
u16 cnt = 0;
u8 led_s = 0;
RCC_ClocksTypeDef RCC_Clocks;
int main(void)
{
//	IWDG_Init(IWDG_Prescaler_128,625);	//128分频 312.5HZ 625为2秒
	RCC_GetClocksFreq(&RCC_Clocks);		//查看各个总线的时钟频率
	__set_PRIMASK(1);	//关闭全局中断

	NVIC_Configuration();
	delay_init(72);
	RTC_Init();
	AT24CXX_Init();
	LED_Init();
	TIM2_Init(99,7199);
	USART1_Init(115200);
	USART2_Init(115200);

	__set_PRIMASK(0);	//开启全局中断

	ResetOTAInfo(HoldReg);
//	mem_init();

	IWDG_Feed();				//喂看门狗
	
	if(NeedUpDateNewFirmWare() == 0xAA)
	{
//		ResetOTAInfo(HoldReg);
//		ReadOTAInfo(HoldReg);
		if(FirmWareUpDate() == 0xAA)
		{
			BootLoader_ExitInit();
			iap_load_app(AppFlashAdd);//执行FLASH APP代码
		}
		else
		{
			__disable_fault_irq();   
			NVIC_SystemReset();
		}
	}

	while(1)
	{
		delay_ms(100);
	}
}


























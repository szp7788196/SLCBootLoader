#include "task_sensor.h"
#include "delay.h"
#include "sht2x.h"
#include "bh1750.h"
#include "task_net.h"
#include "common.h"
#include "inventr.h"
#include "rtc.h"
#include "usart.h"


TaskHandle_t xHandleTaskSENSOR = NULL;

SensorMsg_S *p_tSensorMsg = NULL;	//用于装在传感器数据的结构体变量

void vTaskSENSOR(void *pvParameters)
{
	time_t times_sec = 0;
	u8 push_data_to_net = 0;
	
	SHT2x_Init();
	Bh1750_Init();
	
	p_tSensorMsg = (SensorMsg_S *)mymalloc(sizeof(SensorMsg_S));
	
	while(1)
	{
		if(GetSysTick1s() - times_sec >= UpLoadINCL)		//每隔UpLoadINCL秒向网络任务推送一次传感器数据
		{
			times_sec = GetSysTick1s();
			push_data_to_net = 1;
		}
		
		Temperature = Sht2xReadTemperature();				//读取温度
		Humidity = Sht2xReadHumidity();						//读取湿度
		Illumination = Bh1750ReadIllumination();			//读取光照
		
		InventrOutPutCurrent = InventrGetOutPutCurrent();	//读取电源输出电流
		delay_ms(500);
		InventrOutPutVoltage = InventrGetOutPutVoltage();	//读取电源输出电压
		delay_ms(500);
		
		if(push_data_to_net == 1)
		{
			push_data_to_net = 0;
			
			if(ConnectState == ON_SERVER)					//设备此时是在线状态
			{
				p_tSensorMsg->temperature = (u16)(Temperature * 10.0f + 0.5f);
				p_tSensorMsg->humidity = (u16)(Humidity * 10.0f + 0.5f);
				p_tSensorMsg->illumination = (u16)(Illumination + 0.5f);
				p_tSensorMsg->out_put_current = (u16)(InventrOutPutCurrent + 0.5f);
				p_tSensorMsg->out_put_voltage = (u16)(InventrOutPutVoltage + 0.5f);
				p_tSensorMsg->signal_intensity = SignalIntensity;
				p_tSensorMsg->hour = calendar.hour;
				p_tSensorMsg->minute = calendar.min;
				p_tSensorMsg->second = calendar.sec;
				
				memset(p_tSensorMsg->gps,0,32);
				
				if(GpsInfo != NULL && strlen((char *)GpsInfo) <= 32)
				{
					memcpy(p_tSensorMsg->gps,GpsInfo,strlen((char *)GpsInfo));
				}
				else
				{
					memcpy(p_tSensorMsg->gps,"3948.0975N11632.7539E",21);
				}

				if(xQueueSend(xQueue_sensor,(void *)p_tSensorMsg,(TickType_t)10) != pdPASS)
				{
#ifdef DEBUG_LOG
					UsartSendString(USART1,"send p_tSensorMsg fail 1.\r\n",27);
#endif
				}
			}
		}
	}
}







































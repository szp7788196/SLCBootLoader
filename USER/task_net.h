#ifndef __TASK_NET_H
#define __TASK_NET_H

#include "sys.h"
#include "rtos_task.h"
#include "bg96.h"
#include "task_sensor.h"

extern TaskHandle_t xHandleTaskNET;


extern CONNECT_STATE_E ConnectState;	//bg96������״̬
extern u8 SignalIntensity;				//bg96���ź�ǿ��

extern SensorMsg_S *p_tSensorMsgNet;

void vTaskNET(void *pvParameters);




u8 TryToConnectToServer(void);
void OnServerHandle(void);
void SendSensorData_HeartBeatPacket(void);

u8 GetGpsInfo(u8 **gps_info,u8 *gps_flag,u8 *time_flag);
u8 GetTimeInfo(char *server,u8 port, u8 *flag);































#endif

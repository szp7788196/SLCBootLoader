#ifndef __OTA_H
#define __OTA_H

#include "sys.h"


typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   



void BootLoader_ExitInit(void);								
void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize, u8 flag);	//在指定地址开始,写入bin
u8 NeedUpDateNewFirmWare(void);



u8 FirmWareUpDate(void);
u8 TryToConnectToServer(void);
u8 OnServerHandle(void);






























#endif

#include "ota.h"
#include "stmflash.h"
#include "common.h"
#include "24cxx.h"
#include "usart.h"
#include "malloc.h"
#include "delay.h"
#include "string.h"
#include "bg96.h"
#include "tcp.h"
#include "http.h"
#include "led.h"


iapfun jump2app;

u16 iapbuf[1024];
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize, u8 flag)
{
	u16 t = 0;
	u16 temp = 0;
	static u16 data_pos = 0;
	static u16 sector_pos = 0;
	static u16 cnt = 0;

	for(t = 0; t < appsize; t += 2)
	{
		temp = ((u16)(*(appbuf + t + 1)) << 8) + (u16)(*(appbuf + t));
		iapbuf[data_pos ++] = temp;
	}
	cnt ++;
	if(data_pos == 1024)
	{
		cnt = 0;
		data_pos = 0;
		STMFLASH_Write(AppFlashAdd + (sector_pos ++) * 2048,iapbuf,1024);
	}

	if(data_pos > 0 && flag == 1)
	{
		STMFLASH_Write(AppFlashAdd + (sector_pos ++) * 2048,iapbuf,(cnt * 128 + appsize) / 2);
		data_pos = 0;
		sector_pos = 0;
		cnt = 0;
	}

	if(flag == 1)
	{
		cnt = 0;
	}
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	jump2app = (iapfun) * (vu32*)(appxaddr + 4);	//用户代码区第二个字为程序开始地址(复位地址)
	MSR_MSP(*(vu32*)appxaddr);						//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
	jump2app();										//跳转到APP.
}

//BootLoader退出现场，对BootLoader资源进行回收
void BootLoader_ExitInit(void)
{
	TIM2->CR1 	= 0;
	TIM2->CR2 	= 0;
	TIM2->SMCR 	= 0;
	TIM2->DIER 	= 0;
	TIM2->SR 	= 0;
	TIM2->EGR 	= 0;
	TIM2->CCMR1 = 0;
	TIM2->CCMR2 = 0;
	TIM2->CCER 	= 0;
	TIM2->CNT 	= 0;
	TIM2->PSC 	= 0;
	TIM2->ARR 	= 0;
	TIM2->CCR1 	= 0;
	TIM2->CCR2 	= 0;
	TIM2->CCR3 	= 0;
	TIM2->CCR4 	= 0;
	TIM2->DCR 	= 0;
	TIM2->DMAR 	= 0;

	EXTI->IMR 	= 0;
	EXTI->EMR 	= 0;
	EXTI->RTSR 	= 0;
	EXTI->FTSR 	= 0;
	EXTI->SWIER = 0;
	EXTI->PR 	= 0;

	IWDG->KR 	= 0;
	IWDG->PR 	= 0;
	IWDG->RLR 	= 0;
	IWDG->SR 	= 0;

	__set_PRIMASK(1);
	delay_ms(10);
}

u8 NeedUpDateNewFirmWare(void)
{
	u8 ret = 0;

LOOP:
	if(ReadOTAInfo(HoldReg))
	{
		if(HaveNewFirmWare != 0xAA)	//无新固件需要更新
		{
			if(NewFirmWareAdd != 0xAA && NewFirmWareAdd != 0x55)
			{
				ResetOTAInfo(HoldReg);
				goto LOOP;
			}
			else
			{
				if(NewFirmWareAdd == 0xAA)
				{
					BootLoader_ExitInit();
					iap_load_app(APP1_FLASH_ADD);
				}
				else if(NewFirmWareAdd == 0x55)
				{
					BootLoader_ExitInit();
					iap_load_app(APP2_FLASH_ADD);
				}
			}
		}
		else if(HaveNewFirmWare == 0xAA)	//有新的固件需要更新
		{
			if(NewFirmWareAdd != 0xAA && NewFirmWareAdd != 0x55)
			{
				ResetOTAInfo(HoldReg);
				goto LOOP;
			}
			else
			{
				if((NewFirmWareVer <= 9999 && NewFirmWareVer > 0)\
					&& (NewFirmWareBagNum <= 896 && NewFirmWareBagNum > 0)\
					&& (LastBagByteNum <= 134 && LastBagByteNum > 0))	//版本号合法 并且 总包数合法 并且 末包字节数合法 //128 + 2 + 4 = 134
				{
					if(NewFirmWareAdd == 0xAA)
					{
						AppFlashAdd = APP1_FLASH_ADD;
					}
					else if(NewFirmWareAdd == 0x55)
					{
						AppFlashAdd = APP2_FLASH_ADD;
					}

					ret = 0xAA;
				}
				else					//上述三项有任何一项错误 都不能进入升级模式
				{
					ResetOTAInfo(HoldReg);
					goto LOOP;
				}
			}
		}
	}
	else
	{
		ResetOTAInfo(HoldReg);
		goto LOOP;
	}

	return ret;
}


CONNECT_STATE_E ConnectState = UNKNOW_ERROR;	//bg96的连接状态
u8 SignalIntensity = 99;						//bg96的信号强度

u8 FirmWareUpDate(void)
{
	u8 ret = 0;
	u8 res = 0;
	u8 led_s = 0;
	u8 get_fail_cnt = 0;
	
	BG96_InitStep1(&bg96);
	Http_Init(&bg96,&http);
	
	RE_HARD_INIT:
	BG96_InitStep2(&bg96);
	ret = http->init(&http);
	
	if(!ret)
	{
		goto RE_HARD_INIT;
	}

	while(ret)
	{
		SignalIntensity = bg96->get_AT_CSQ(&bg96);

		res = OnServerHandle();
		
		if(res == 0)
		{
			goto RE_HARD_INIT;
		}
		else if(res == 0xAA)
		{
			ret = res;
			break;
		}
		
		led_s = !led_s;
		
		if(led_s)
		{
			RUN_LED = 1;
		}
		else
		{
			RUN_LED = 0;
		}

		delay_ms(1000);
	}

	return ret;
}


//尝试连接服务器
u8 TryToConnectToServer(void)
{
	u8 ret = 0;

	if(ServerIP != NULL && ServerPort != NULL)
	{
		ret = tcp->connect(&tcp,(char *)ServerIP,(char *)ServerPort);
	}

	return ret;
}


u8 DownLoadBuf[1500];
u8 SendBuf[256];
u8 crc32_cal_buf[1024];
u8 data_buf[134];
u16 bag_pos = 0;
//在线处理进程
u8 OnServerHandle(void)
{
	static u8 store_times = 0;
	u8 ret = 0;
	u8 res = 0;
	u16 i = 0;
	u16 send_len = 0;
//	u16 recv_len = 0;
	u16 data_pos1 = 0;
	u16 data_pos2 = 0;
	u16 data_pos3 = 0;
	u8 data_len = 0;
	u16 crc16_cal = 0;
	u16 crc16_read = 0;
	u32 crc32_cal = 0xFFFFFFFF;
	u32 crc32_read = 0;
	u32 file_len = 0;
	u16 k_num = 0;
	u16 last_k_byte_num = 0;
	u8 srtA_B[2] = {0,0};

	if(bag_pos <= (NewFirmWareBagNum - 1))
	{
		memset(DownLoadBuf,0,512);
		memset(SendBuf,0,256);

		if(NewFirmWareAdd == 0xAA)
		{
			srtA_B[0] = 'A';
		}
		else if(NewFirmWareAdd == 0x55)
		{
			srtA_B[0] = 'B';
		}

//		sprintf((char *)SendBuf,"GET /hardware/SLC/V%02d.%02d%s/SLC%04d.bin HTTP/1.1\r\nHost: %s:%s\r\nUser-Agent: abc\r\nConnection: Keep-alive\r\nKeep-alive: timeout=10\r\n\r\n",\
//					NewFirmWareVer / 100,NewFirmWareVer % 100,srtA_B,bag_pos,ServerIP,ServerPort);
		
		sprintf((char *)SendBuf,"http://103.48.232.122:8080/nnlightctl/hardware/SLC/V%02d.%02d%s/SLC%04d.bin\r\n",\
					NewFirmWareVer / 100,NewFirmWareVer % 100,srtA_B,bag_pos);

		send_len = strlen((char *)SendBuf);
//		send_len = tcp->send(&tcp,SendBuf,send_len);
		if(send_len != 0)
		{
			UsartSendString(USART1,SendBuf,send_len);
			
//			delay_ms(1000);
//			recv_len = tcp->read(&tcp,DownLoadBuf);
			
			res = http->get(&http,(char *)SendBuf,60,30);
			
			if(res == 1)
			{
				delay_ms(100);
				res = http->read(&http,DownLoadBuf,80);
			}

			if(res == 1)
			{
//				UsartSendString(USART1,DownLoadBuf,recv_len);

				if(MyStrstr(DownLoadBuf, (u8 *)"HTTP/1.1 200 OK", 512, 15) != 0xFFFF && MyStrstr(DownLoadBuf, (u8 *)"Content-Length:", 512, 15) != 0xFFFF)
				{
					if(bag_pos <= (NewFirmWareBagNum - 2))
					{
						data_pos1 = MyStrstr(DownLoadBuf, (u8 *)"Content-Length:", 512, 15);
						data_pos2 = MyStrstr(DownLoadBuf, (u8 *)"\r\nDate: ", 512, 8);
						data_pos3 = MyStrstr(DownLoadBuf, (u8 *)"GMT\r\n\r\n", 512, 7);

						if(data_pos1 != 0xFFFF && data_pos2 != 0xFFFF && data_pos2 > data_pos1)
						{
							if(data_pos2 - data_pos1 == 19)
							{
								data_len = (DownLoadBuf[data_pos2 - 3] - 0x30) * 100 + (DownLoadBuf[data_pos2 - 2] - 0x30) * 10 + (DownLoadBuf[data_pos2 - 1] - 0x30);
								if(data_len == 130)
								{
									memset(data_buf,0,134);
									memcpy(data_buf,&DownLoadBuf[data_pos3 + 7],130);

									crc16_read = (u16)data_buf[128];//获取接收到的校验值
									crc16_read = crc16_read << 8;
									crc16_read = crc16_read | (u16)data_buf[129];

									crc16_cal = CRC16(data_buf,128);

									if(crc16_read == crc16_cal)
									{
										iap_write_appbin(AppFlashAdd + 128 * bag_pos,data_buf,128,0);
										bag_pos ++;
										
										ret = 1;
									}
								}
							}
						}
					}
					else if(bag_pos == (NewFirmWareBagNum - 1))
					{
						data_pos1 = MyStrstr(DownLoadBuf, (u8 *)"Content-Length:", 512, 15);
						data_pos2 = MyStrstr(DownLoadBuf, (u8 *)"\r\nDate: ", 512, 8);
						data_pos3 = MyStrstr(DownLoadBuf, (u8 *)"GMT\r\n\r\n", 512, 7);
						
						if(data_pos1 != 0xFFFF && data_pos2 != 0xFFFF && data_pos2 > data_pos1)
						{
							if(data_pos2 - data_pos1 == 19)
							{
								data_len = (DownLoadBuf[data_pos2 - 3] - 0x30) * 100 + (DownLoadBuf[data_pos2 - 2] - 0x30) * 10 + (DownLoadBuf[data_pos2 - 1] - 0x30);
							}
							else if(data_pos2 - data_pos1 == 18)
							{
								data_len = (DownLoadBuf[data_pos2 - 2] - 0x30) * 10 + (DownLoadBuf[data_pos2 - 1] - 0x30);
							}
							else if(data_pos2 - data_pos1 == 17)
							{
								data_len = (DownLoadBuf[data_pos2 - 1] - 0x30);
							}
							if(data_len > 0 && data_len <= 134)
							{
								memset(data_buf,0,134);
								memcpy(data_buf,&DownLoadBuf[data_pos3 + 7],data_len);
								crc16_read = (u16)data_buf[data_len - 6];//获取接收到的校验值
								crc16_read = crc16_read << 8;
								crc16_read = crc16_read | (u16)data_buf[data_len - 5];

								crc16_cal = CRC16(data_buf,data_len - 6);

								if(crc16_read == crc16_cal)
								{
									iap_write_appbin(AppFlashAdd + 128 * bag_pos,data_buf,data_len - 6,1);
									bag_pos ++;

									crc32_read = (((u32)data_buf[data_len - 4]) << 24) \
													+ (((u32)data_buf[data_len - 3]) << 16) \
													+ (((u32)data_buf[data_len - 2]) << 8) \
													+ (((u32)data_buf[data_len - 1]));

									file_len = 128 * (NewFirmWareBagNum - 1) + (data_len - 6);

									k_num = file_len / 1024;
									last_k_byte_num = file_len % 1024;
									if(last_k_byte_num > 0)
									{
										k_num += 1;
									}

									for(i = 0; i < k_num; i ++)
									{
										memset(crc32_cal_buf,0,1024);
										if(i < k_num - 1)
										{
											STMFLASH_ReadBytes(AppFlashAdd + 1024 * i,crc32_cal_buf,1024);
											crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,0);
										}
										if(i == k_num - 1)
										{
											if(last_k_byte_num == 0)
											{
												STMFLASH_ReadBytes(AppFlashAdd + 1024 * i,crc32_cal_buf,1024);
												crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,1);
											}
											else if(last_k_byte_num > 0)
											{
												STMFLASH_ReadBytes(AppFlashAdd + 1024 * i,crc32_cal_buf,last_k_byte_num);
												crc32_cal = CRC32(crc32_cal_buf,last_k_byte_num,crc32_cal,1);
											}
										}
									}

									if(crc32_read == crc32_cal)
									{
										__disable_irq();		//关闭全局中断，以免扰乱读写EEPROM
										do
										{
											ResetOTAInfo(HoldReg);
											ReadOTAInfo(HoldReg);

											if(ReadOTAInfo(HoldReg))
											{
												__enable_irq();	//恢复全局中断
												return 0xAA;
											}

											store_times ++;

											if(store_times > 5)
											{
												__enable_irq();	//恢复全局中断
												return 0xAA;
											}
										}
										while(!ReadOTAInfo(HoldReg));
									}
								}
							}
						}
					}
				}
			}
		}

//		printf("%3d/%3d   %3d%%\r",bag_pos,NewFirmWareBagNum,(u8)((float)bag_pos / (float)NewFirmWareBagNum * 100.0f));	//打印更新进度

	}

	return ret;
}














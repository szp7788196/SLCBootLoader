#include "http.h"
#include "usart.h"

pHttp http;


//初始化HTTP结构体
unsigned char Http_Init(pBg96 *bg96,pHttp *http)
{
	unsigned char ret = 0;
	
	(*http) = (pHttp)mymalloc(sizeof(Http));
	if((*http) != NET_NULL)
	{
		(*http)->bg96 = *bg96;
		(*http)->init = http_init;
		(*http)->get = http_get;
		(*http)->read = http_read;
		
		ret = 1;
	}
	return ret;
}

//初始化HTTP配置
unsigned char http_init(pHttp *http)
{
	unsigned char ret = 0;
	unsigned char fail_time = 0;
	unsigned char fail_cnt = 0;
	
	RE_INIT:
	
	if((fail_cnt ++) >= 3)
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http config failed.\r\n", 21);
#endif
		return ret;
	}
	
	delay_ms(100);
	
	fail_time = 0;
	while(!(*http)->bg96->set_AT_QHTTPCFG(&((*http)->bg96),"contextid",1))
	{
		fail_time ++;
		if(fail_time >= 3)
		{
			goto RE_INIT;
		}
	}
	delay_ms(100);

	fail_time = 0;
	while(!(*http)->bg96->set_AT_QHTTPCFG(&((*http)->bg96),"responseheader",1))
	{
		fail_time ++;
		if(fail_time >= 3)
		{
			goto RE_INIT;
		}
	}

	ret = 1;
	
#ifdef DEBUG_LOG
	UsartSendString(USART1, "http config success.\r\n", 22);
#endif

	return ret;
}

//发送URL内容和GET请求
unsigned char http_get(pHttp *http,char *url,unsigned char time_out1,unsigned char time_out2)
{
	unsigned char ret = 0;
	
	ret = (*http)->bg96->set_AT_QHTTPURL(&((*http)->bg96),url,strlen(url) - 2,time_out1);
	
	if(ret == 1)
	{
		delay_ms(100);
		
		ret = (*http)->bg96->get_AT_QHTTPGET(&((*http)->bg96),time_out2);
	}
	
	if(ret == 1)
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http request get success.\r\n", 27);
#endif
	}
	else
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http request get failed.\r\n", 26);
#endif
	}
	return ret;
}

//读取HTTP相应数据包
unsigned char http_read(pHttp *http, unsigned char *buf, unsigned char time_out)
{
	unsigned char ret = 0;
	
	ret = (*http)->bg96->get_AT_QHTTPREAD(&((*http)->bg96),buf,time_out);
	
	if(ret == 1)
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http read success.\r\n", 20);
#endif
	}
	else
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http read failed.\r\n", 19);
#endif
	}
	return ret;
}
























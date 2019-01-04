#include "http.h"
#include "usart.h"

pHttp http;


//初始化HTTP结构体
unsigned char Http_Init(pBg96 *bg96,pHttp *http,unsigned resp_head,unsigned char customize_req_head)
{
	unsigned char ret = 0;
	
	(*http) = (pHttp)mymalloc(sizeof(Http));
	if((*http) != NET_NULL)
	{
		(*http)->bg96 = *bg96;
		(*http)->init = http_init;
		(*http)->get = http_get;
		(*http)->resp_head = resp_head;
		(*http)->customize_req_head = customize_req_head;
		
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
	while(!(*http)->bg96->set_AT_QHTTPCFG(&((*http)->bg96),"contextid",1))									//PDP context ID. The range is 1-16, and the default value is 1
	{
		fail_time ++;
		if(fail_time >= 3)
		{
			goto RE_INIT;
		}
	}
	delay_ms(100);

	fail_time = 0;
	while(!(*http)->bg96->set_AT_QHTTPCFG(&((*http)->bg96),"responseheader",(*http)->resp_head))			//Disable or enable to output HTTP(S) response header
	{
		fail_time ++;
		if(fail_time >= 3)
		{
			goto RE_INIT;
		}
	}
	delay_ms(100);
	
	fail_time = 0;
	while(!(*http)->bg96->set_AT_QHTTPCFG(&((*http)->bg96),"requestheader",(*http)->customize_req_head))	//Disable or enable to customize HTTP(S) request header
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
unsigned short http_get(pHttp *http,char *url,unsigned char *out_buf,unsigned char rsptime,unsigned char readtime)
{
	unsigned short ret = 0;
	unsigned short recv_len = 0;
	
	if((*http)->customize_req_head == 0)
	{
		ret = (*http)->bg96->set_AT_QHTTPURL(&((*http)->bg96),url,strlen(url) - 2,60);
	
		if(ret == 1)
		{
			delay_ms(50);
			
			ret = (*http)->bg96->get_AT_QHTTPGET1(&((*http)->bg96),rsptime);
		}
	}
	else
	{
		ret = (*http)->bg96->get_AT_QHTTPGET2(&((*http)->bg96),rsptime,url,60);
	}
	
	if(ret)
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http request get success.\r\n", 27);
#endif
		delay_ms(50);
		
		recv_len = (*http)->bg96->get_AT_QHTTPREAD(&((*http)->bg96),out_buf,readtime,(*http)->resp_head);
		
		if(ret == recv_len)
		{
			ret = recv_len;
			
#ifdef DEBUG_LOG
			UsartSendString(USART1, "http receive data success.\r\n", 28);
#endif				
		}
		else
		{
#ifdef DEBUG_LOG
			UsartSendString(USART1, "http receive data failed.\r\n", 27);
#endif			
		}
	}
	else
	{
#ifdef DEBUG_LOG
		UsartSendString(USART1, "http request get failed.\r\n", 26);
#endif		
	}
	
	
	return ret;
}
























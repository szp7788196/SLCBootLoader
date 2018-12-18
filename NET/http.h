#ifndef __HTTP_H
#define __HTTP_H

#include "common.h"
#include "bg96.h"


#define Http struct HTTP
typedef struct HTTP *pHttp;
struct HTTP
{
	unsigned char 	(*init)(pHttp *http);
	unsigned short 	(*get)(pHttp *http,char *url,unsigned char *out_buf,unsigned char time_out1,unsigned char time_out2);
	
	unsigned char 	resp_head;
	unsigned char 	customize_req_head;
	
	pBg96 bg96;
};


extern pHttp http;


unsigned char 	Http_Init(pBg96 *bg96,pHttp *http,unsigned resp_head,unsigned char customize_req_head);

unsigned char 	http_init(pHttp *http);
unsigned short 	http_get(pHttp *http,char *url,unsigned char *out_buf,unsigned char time_out1,unsigned char time_out2);































#endif

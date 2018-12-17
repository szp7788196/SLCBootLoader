#ifndef __HTTP_H
#define __HTTP_H

#include "common.h"
#include "bg96.h"


#define Http struct HTTP
typedef struct HTTP *pHttp;
struct HTTP
{
	unsigned char 	(*init)(pHttp *http);
	unsigned char 	(*get)(pHttp *http,char *url,unsigned char time_out1,unsigned char time_out2);
	unsigned char 	(*read)(pHttp *http, unsigned char *buf, unsigned char time_out);
	
	pBg96 bg96;
};


extern pHttp http;


unsigned char 	Http_Init(pBg96 *bg96,pHttp *http);

unsigned char 	http_init(pHttp *http);
unsigned char 	http_get(pHttp *http,char *url,unsigned char time_out1,unsigned char time_out2);
unsigned char 	http_read(pHttp *http, unsigned char *buf, unsigned char time_out);































#endif

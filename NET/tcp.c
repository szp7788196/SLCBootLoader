#include "tcp.h"
#include "usart.h"

pTcp tcp;

unsigned char Tcp_Init(pBg96 *bg96,pTcp *tcp)
{
	unsigned char ret = 0;
	
	(*tcp) = (pTcp)mymalloc(sizeof(Tcp));
	if((*tcp) != NET_NULL)
	{
		(*tcp)->bg96 = *bg96;
		(*tcp)->connect = tcp_connect;
		(*tcp)->close = tcp_close;
		(*tcp)->send = tcp_send;
		(*tcp)->read = tcp_read;
		
		ret = 1;
	}
	return ret;
}

void Tcp_UnInit(pTcp *tcp)
{
	myfree(*tcp);
}

/**
	* connect to remote server or client .
	*
	* @param remote_ip - the ip of remote server or client .
	* @param remote_port - the port of remote server or client .
	* @param local_port - the port of local server or client .
	* @retval true - success.
	* @retval false - failure.
*/
unsigned char tcp_connect(pTcp *tcp,char *remote_ip, char *remote_port)
{
	unsigned char ret = 0;
	ret = (*tcp)->bg96->create_TCP(&((*tcp)->bg96),remote_ip,remote_port);
	
	if(ret == 1)
	{
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp connect success.\r\n", 22,1000);
#endif
	}
	else
	{
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp connect failed.\r\n", 21,1000);
#endif
	}
	return ret;
}

/**
	* disconnect to remote server or client .
	*
	* @param remote_ip - the ip of remote server or client .
	* @param remote_port - the port of remote server or client .
	* @param local_port - the port of local server or client .
	* @retval true - success.
	* @retval false - failure.
*/
unsigned char tcp_close(pTcp *tcp)
{
	unsigned char ret = 0;
	ret = (*tcp)->bg96->release_TCP(&((*tcp)->bg96));
	
	if(ret == 1)
	{
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp disconnect success.\r\n", 25,1000);
#endif
	}
	else
	{
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp disconnect failed.\r\n", 24,1000);
#endif
	}
	return ret;
}

/**
	* tcp send data.
	*
	* @param buf - buffer to send .
	* @param len - length to send .
	* @retval >0 - send length.
	* @retval 0 - failure.
*/
unsigned short tcp_send(pTcp *tcp, unsigned char *buf, unsigned short len)
{
	unsigned short ret;
	
	ret = (*tcp)->bg96->send(&((*tcp)->bg96),buf,len);
	
    if(ret == 1)
	{
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp send success.\r\n", 19,1000);
#endif
	}
    else
    {
#ifdef DEBUG_LOG
		HAL_UART_Transmit(&DEBUG_UART_HANDLER, "tcp send failed.\r\n", 18,1000);
#endif
        len = 0;
    }
    return len;
}

/**
	* tcp read data.
	*
	* @param buf - buffer to read .
	* @retval >0 - read length.
	* @retval 0 - failure.
*/
unsigned short tcp_read(pTcp *tcp,unsigned char *buf)
{
	unsigned short len;
	
	len = (*tcp)->bg96->read(&((*tcp)->bg96),buf);
	
	return len;
}





































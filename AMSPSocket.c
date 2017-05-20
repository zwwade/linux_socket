
/* =========================================================================

DESCRIPTION
   MSP String Implemention.

Copyright (c) 2017 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2017/05/15     weizhang        Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AMSPSocket.h"
#include <netinet/in.h>
#include "msp_list.h"
#include "msp_errors.h"
//#include "ip_util.h"
//#include "MSPMemory.h"
//#include "MSPStdio.h"
#include "AMSPSystem.h"
#include "MSPPlatform.h"
#include<fcntl.h>
#include <errno.h>


/*  linux */


/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */
#define MSPSOCK_STATUS_CONNECTED (1)
#define MSPSOCK_STATUS_SEND_OK (2)
#define MSPSOCK_STATUS_READABLE (3)
#define MSPSOCK_STATUS_CLOSED (4)
#define MSPSOCK_STATUS_ERROR (5)
#define MSP_ERROR_SET_SOCK_NONBLOCK (6)



/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
#define MALLOC(size) malloc( size )
#define MFREE(p) free( p )
#define ISP_LOGERR printf
#define ISP_LOGDBG printf
#define ISP_LOGWARN printf
#define msp_tickcount MSPSys_GetTickCount

//typedef int ISP_SOCKET;

/* 系统发送接收缓冲区大小 */
#define SYS_SENDBUF_SIZE		64 * 1024
#define SYS_RECVBUF_SIZE		30 * 1024
/* 单socket发送缓冲 */
//#define DEFAULT_SEND_BUFF_SIZE	64 * 1024

#define ISP_NGETLASTERR			errno

enum
{
	sock_state_notinit = 0,					//未初始化
	sock_state_opened,						//已打开
	sock_state_connecting,					//正在连接
	sock_state_connected,					//已连接
	sock_state_listen,						//监听
	sock_state_closed,						//关闭
	sock_state_failed,						//失败
};


/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Global Variable Definitions
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Function Definitions
** ------------------------------------------------------------------------ */
static list g_socket_list;
static NATIVE_MUTEX_HANDLE g_sock_mutex = NULL;


static int skt_handle_compare(void *udata, void *val)
{
	MSPSocket *mspskt = (MSPSocket*)val;
	if (mspskt->soc_handle== *(NATIVE_SOCKET_HANDLE*)(udata))
		return 1;

	return 0;
}
int socket_init()
{

	list_init(&g_socket_list);
	g_sock_mutex = native_mutex_create("socket_mutex", NULL);
	if (NULL == g_sock_mutex){
		return MSP_ERROR_CREATE_HANDLE;
	}
	return 0;
}

int socket_uninit()
{
	if(NULL != g_sock_mutex ){
		native_mutex_destroy(g_sock_mutex);
	}
	return 0;
}
static int SET_NOBLOCK(SOCKET_HANDLE s)
{
	MSPSocket * mspskt = (MSPSocket *)s;
	int opts = fcntl(mspskt->soc_handle,F_GETFL);
	if (opts < 0)
	{
		mspskt->last_error = ISP_NGETLASTERR;
		printf("fcntl failed\n",ISP_NGETLASTERR);
		return -1;
	}
	opts = opts|O_NONBLOCK;
	if (fcntl(mspskt->soc_handle,F_SETFL,opts) < 0)
	{
		mspskt->last_error = ISP_NGETLASTERR;
		printf("fcntl failed\n",ISP_NGETLASTERR);
		return -1;
	}
	return 0;




}

int socket_open( SOCKET_HANDLE *s, int domain, int type, skt_connted_fn connted_cb, skt_recv_fn recv_cb, skt_error_fn error_cb, void *user_data)
{
	MSPSocket *mspskt;
	list_node *temp;
	ISP_SOCKET sock_hd;
	int opt = 1, opt_len, ret = MSP_SUCCESS;
	int af = AF_INET, soc_type = SOCK_STREAM;
	int socError = 0;
	//	int opt;
	if (NULL == s || NULL == connted_cb || NULL == recv_cb || NULL == error_cb)
	{
		return MSP_ERROR_INVALID_PARA_VALUE;
	}
	mspskt = (MSPSocket *)MALLOC(sizeof(MSPSocket));
	if (NULL == mspskt)
	{
		ret = MSP_ERROR_NO_ENOUGH_BUFFER;
		goto error;
	}
	memset(mspskt, 0, sizeof(MSPSocket));
	mspskt->send_buff_ = (ISP_Sock_Send_Buff*)MALLOC(sizeof(ISP_Sock_Send_Buff));
	if (NULL == mspskt->send_buff_)
	{
		ret = MSP_ERROR_NO_ENOUGH_BUFFER;
		goto error;
	}
	memset(mspskt->send_buff_, 0, sizeof(ISP_Sock_Send_Buff));

	mspskt->connted_cb = connted_cb;
	mspskt->recv_cb = recv_cb;
	mspskt->error_cb = error_cb;
	mspskt->user_data = user_data;
	mspskt->domain = domain;
	mspskt->type = type;
	if (domain == MSPSOCK_AF_INET6)
		af = AF_INET6;
	if (type == MSPSOCK_TYPE_UDP)
		soc_type = SOCK_DGRAM;


	mspskt->soc_handle = sock_hd = socket(af, type, 0);
	if (mspskt->soc_handle == -1)
	{
		mspskt->last_error = ISP_NGETLASTERR;
		ret = MSP_ERROR_NET_OPENSOCK;
		goto error;
	}
	mspskt->stat = sock_state_opened;

	/* set socket recv buff size */
	opt = SYS_RECVBUF_SIZE;
	setsockopt(sock_hd, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt));
	opt_len = sizeof(opt);
	getsockopt(sock_hd, SOL_SOCKET, SO_RCVBUF, (char*)&opt, &opt_len);
	//	ISP_LOGDBG("_isp_socket_open | get recvbuf size = %d.\n", opt);

	opt = TRUE;
	if (SET_NOBLOCK(mspskt)!=0)
	{
		printf("SET_NOBLOCK failed");
		ret = MSP_ERROR_SET_SOCK_NONBLOCK;
		goto error;
	}
    
	temp  = list_node_new((void  *)mspskt,0,NULL);
	native_mutex_take(g_sock_mutex, INFINITE);
	list_push_back(&g_socket_list, temp);
	native_mutex_given(g_sock_mutex);
	*s = (SOCKET_HANDLE)mspskt;
	goto exit;
error:
	if (NULL != mspskt){
		if(NULL!=mspskt->send_buff_)
			MFREE(mspskt->send_buff_);
		MFREE(mspskt);
		mspskt = NULL;
	}
exit:
	if (mspskt->last_error)ISP_LOGDBG("last_error=%d\n", mspskt->last_error);
	return ret;
}


int socket_connect(SOCKET_HANDLE s, const char *ip, short port)
{
	int ret =  MSP_SUCCESS;
	MSPSocket *mspskt = s;
	mspsaddr saddr;
	int socRet = 0;
	int socError = 0;
	int timeout = 15000;
	list_node *target_socket = NULL;
	native_mutex_take(g_sock_mutex, 0x7fffffff);
	target_socket = list_search(&g_socket_list,skt_handle_compare,(void *)(&mspskt->soc_handle));
	native_mutex_given(g_sock_mutex);
	mspskt = (MSPSocket*) target_socket->val;
	char ip_buffer[64] = {0};
	int salen;

	if (NULL == s || NULL == ip)
	{
		return MSP_ERROR_INVALID_PARA_VALUE;
	}
	native_mutex_take(g_sock_mutex, INFINITE);
	if (MSPSOCK_AF_INET6 == mspskt->domain) {

		return -1;
	}
	else {
		salen = sizeof(saddr.sa4);
		memset((void *)&saddr, 0, salen);
		saddr.sa4.sin_family = AF_INET;
		//inet_pton()


		inet_pton(AF_INET, ip,(void *)&saddr.sa4.sin_addr );
		saddr.sa4.sin_port = htons(port);
	}
	
	ret = connect(mspskt->soc_handle, (struct sockaddr*)(&saddr), sizeof(struct sockaddr));	//开始连接;
	socError = ISP_NGETLASTERR;
	if (ret==0)
	{
     	mspskt->last_error = 0;
		mspskt->stat = sock_state_connected;	//socket状态置为连接
		ret = MSP_SUCCESS;
	}	
	else if (ret == -1 && (socError == EINPROGRESS || socError == EWOULDBLOCK) )//连接成功或尚未完成连接
	{
		//printf("当前的状态为： connecting,%d\nret =%d\n ",socError,ret);
		mspskt->last_error = 0;
		mspskt->stat = sock_state_connecting;	//socket状态置为连接中
		mspskt->conn_timeout = timeout;
		mspskt->conn_time = msp_tickcount();	//记录当前时间，用于检测超时
		ret = MSP_SUCCESS;

	}
	else
	{
		mspskt->last_error = socError;					//记录错误码
		mspskt->stat = sock_state_failed;		//连接失败
		ISP_LOGERR("_isp_socket_connect| leave, connect failed with error %d.\r\n", socError);
		ret = MSP_ERROR_NET_CONNECTSOCK;
	}

	native_mutex_given(g_sock_mutex);

	return ret;
}

int socket_send(SOCKET_HANDLE s, void *data, int len)
{
	MSPSocket *mspskt=s;
	int ret = MSP_SUCCESS;;
	list_node *target_socket = NULL;
	int tail_len = 0;
	const char* buff = (char *)data;
    native_mutex_take(g_sock_mutex, INFINITE);
	target_socket = list_search(&g_socket_list,skt_handle_compare,(void *)(&mspskt->soc_handle));
	native_mutex_given(g_sock_mutex);
	if (NULL == target_socket || NULL == data || 0 == len)
	{
		return MSP_ERROR_INVALID_PARA_VALUE;
	}
	mspskt = (MSPSocket*)(target_socket->val);
	mspskt = (MSPSocket *)s;
	if (mspskt->stat != sock_state_connecting && mspskt->stat != sock_state_connected)
	{
	
		ISP_LOGERR("_isp_sock_send| leave, not connected.\r\n");
		return MSP_ERROR_NET_SENDSOCK;	//未连接
	}
	native_mutex_take(g_sock_mutex, INFINITE);
	/* copy buffer to send buffer list */
	if (len > (int)(DEFAULT_SEND_BUFF_SIZE - mspskt->send_buff_->len_))
	{
		ISP_LOGERR("_isp_sock_send| leave, not enough send buffer, need %d, left %d.\r\n",
			len,
			DEFAULT_SEND_BUFF_SIZE - mspskt->send_buff_->len_);
		ret = MSP_ERROR_NO_ENOUGH_BUFFER;		//要发送的buffer比剩余空间大
		goto exit;
	}

	tail_len = DEFAULT_SEND_BUFF_SIZE - mspskt->send_buff_->wr_ptr_;	//从可写入的位置到尾部的长度
	if (len > (int)tail_len)	//如果buffer长度大于剩余长度，则把buffer分成两部分，前一部分拷到尾部，后一部分拷到头部，并调整各参数
	{
		printf("send starting copy\n");
		memcpy(mspskt->send_buff_->buff_ + mspskt->send_buff_->wr_ptr_, buff, tail_len);
		memcpy(mspskt->send_buff_->buff_,
			buff + tail_len,
			len - tail_len);
		mspskt->send_buff_->wr_ptr_ = len - tail_len;
		mspskt->send_buff_->len_ += len;
	}
	else	//否则直接拷到后面
	{
		ISP_LOGDBG("The Sending data added remained buff,the length of which is: %d\n",len);
		memcpy(mspskt->send_buff_->buff_ + mspskt->send_buff_->wr_ptr_, buff, len);
		mspskt->send_buff_->wr_ptr_ += len;
		mspskt->send_buff_->len_ += len;
	}
exit:
	native_mutex_given(g_sock_mutex);

	ISP_LOGDBG("_isp_sock_send| leave %d.\r\n",ret);
	ISP_LOGDBG(".............socket_send END...................\n");
	return ret;
	
}

int socket_run(void *data)
{
	
	MSPSocket* sock_unit;
	list_node *list_node_temp = NULL ;
	fd_set rd_set, wr_set, ex_set;
	ISP_SOCKET max_sock_hd = 0;
	int select_sock_count = 0;
	int tail_len, send_len;
	int ret = 0, error = 0, error_len = 0;
	struct timeval tv;

	//if (!b_inited)return MSP_ERROR_INVALID_OPERATION;
	FD_ZERO(&rd_set); FD_ZERO(&wr_set); FD_ZERO(&ex_set);
	native_mutex_take(g_sock_mutex, INFINITE);
	list_node_temp = list_peek_front(&g_socket_list);
	
	//sock_unit = (MSPSocket*)list_peek_front(&g_socket_list)->val;
	while (NULL != list_node_temp)
	{
		sock_unit = (MSPSocket*)list_node_temp->val;
		if (sock_unit->stat == sock_state_connecting)
		{
			FD_SET(sock_unit->soc_handle, &wr_set);	//当前socket添加到发送组中
		}
		else if (sock_unit->send_buff_->len_ > 0)
		{
			
			/* send buff if exist */
			ISP_LOGDBG("The Sending data length: %d\n",sock_unit->send_buff_->len_);
			send_len = sock_unit->send_buff_->len_;
			while (send_len > 0)
			{
				tail_len = DEFAULT_SEND_BUFF_SIZE - sock_unit->send_buff_->rd_ptr_;
				if (send_len < tail_len)
					tail_len = send_len; 
				{
					ret = send(sock_unit->soc_handle, sock_unit->send_buff_->buff_ + sock_unit->send_buff_->rd_ptr_, tail_len, 0);	//发送buffer尾部数据
				}

				ISP_LOGDBG("socket_thread| hd 0x%x send out %d bytes in reality.\r\n", sock_unit->soc_handle, ret);
				ISP_LOGDBG("socket_thread| hd 0x%x need send out %d bytes in total.\r\n", sock_unit->soc_handle, tail_len);
				if (ret == -1)
				{
					sock_unit->last_error = ISP_NGETLASTERR;
					ISP_LOGWARN("warning last_error=%d\r\n",sock_unit->last_error);
					if (sock_unit->last_error == EWOULDBLOCK || sock_unit->last_error == ENOBUFS)
					{
						FD_SET(sock_unit->soc_handle, &wr_set);
						/*if (sock_unit->error_cb)
						sock_unit->error_cb(sock_unit, MSP_ERROR_NET_WOULDBLOCK, "EWOULDBLOCK", sock_unit->user_data);*/
					}
					else
					{
						/* exception occured, callback */
						/*sock_unit->send_buff_->len_ = 0;
						if (sock_unit->error_cb)
						sock_unit->error_cb(sock_unit, ret, "error in isp_exception_cb", sock_unit->user_data);*/
						//异常回调
					}
					break;
				}
				else
				{
					printf("Send out[%d]: \n", ret);//, sock_unit->send_buff_->buff_ + sock_unit->send_buff_->rd_ptr_);
					sock_unit->send_buff_->rd_ptr_ += ret;
					if (sock_unit->send_buff_->rd_ptr_ == DEFAULT_SEND_BUFF_SIZE)
						sock_unit->send_buff_->rd_ptr_ = 0;//回到buffer头部
					send_len -= ret;
					sock_unit->send_buff_->len_ -= ret;	//数据调整
					/* no send over, SET and break */
					/*if (sock_unit->error_cb)
						sock_unit->error_cb(sock_unit, MSP_ERROR_NET_NOTBLOCK, "NOTBLOCK", sock_unit->user_data);*/
					if (ret != tail_len)
					{
						FD_SET(sock_unit->soc_handle, &wr_set);
						break;
					}
				}
			}
		}
		//if (sock_unit->recv_count_ == 0)//recv_count_永远为0
		{
			//ISP_LOGDBG("set rd");
			FD_SET(sock_unit->soc_handle, &rd_set);
		}
		//ISP_LOGDBG("set ex");
		FD_SET(sock_unit->soc_handle, &ex_set);
		if (sock_unit->soc_handle > max_sock_hd)
		{
			max_sock_hd = sock_unit->soc_handle;
			//ISP_LOGDBG("max_sock_hd = 0x%x", max_sock_hd);
		}
		select_sock_count++;
		list_node_temp = list_peek_next(&g_socket_list,list_node_temp);
		//sock_unit = (MSPSocket*)list_node_temp->val;
	}

	//对所有链表中创建的socket全部分组完毕分为以下几种情况：
	/* ------------------------------------------------------------------------
	** 1.如果socket为connecting，则加入到wr_set集合中
	** 2.如果socket位connected，则发送数据，如果send发生阻塞，实际发送的数据小于send传入要求发送的数据，则加入wr_set中
	** 1.所有的socket都加入可读和异常组中，监听是否可以recv
	** 1.

	** ------------------------------------------------------------------------ */
	//链表中的socket已经全部分组完毕，
	//没有处理完发送消息任务的socket都添加到【写入组】中，
	//recv_count_==0的socket添加到【读取组】中，
	//所有未删除的socket都添加到【异常组】中。
	if (select_sock_count == 0)
	{
		goto exit;
	}

	/* select all socket */
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	ret = select((int)max_sock_hd + 1, &rd_set, &wr_set, &ex_set, &tv);	//10微秒之内，筛选各个组存在的对应变化
	/* Process each socket */
	list_node_temp = list_peek_front(&g_socket_list);
	
	while (NULL != list_node_temp)
	{
		sock_unit = (MSPSocket*)list_node_temp->val;
		ISP_LOGDBG("sock_unit 0x%x, state = %d \n", sock_unit, sock_unit->stat);
		if (sock_unit->stat == sock_state_connecting)
		{
			//ISP_LOGDBG("conn check");
			/* Check connecting state */
			if (ret == 0 && msp_tickcount() - sock_unit->conn_time > sock_unit->conn_timeout)	//检查超时//检查超时,,当select的返回值位0的时候表示指定的时间已经超过，还没有描述符准备好
			{
				ISP_LOGERR("conn timeout\r\n");
				/* Connect Timeout */
				sock_unit->stat = sock_state_failed;
				sock_unit->last_error = MSP_ERROR_TIME_OUT;
				if (sock_unit->error_cb)
					sock_unit->error_cb(sock_unit, ret, "connect timeout", sock_unit->user_data);	//连接回调
			}
			else if (ret == -1)//当select的返回值位-1的时候表示出错
			{
				//ISP_LOGDBG("conn exception");
				///* Exception */
				//sock_unit->stat = sock_state_failed;
				//if ( sock_unit->exception_cb )
				//	(sock_unit->exception_cb)(sock_unit->hd_);
			}
			else if (ret > 0) //,当select的返回值大于0的时候表示已经准备好的描述符之和
			{
				int need_to_check = 0;
				//ISP_LOGDBG("conn got\r\n");

		        if (FD_ISSET(sock_unit->soc_handle, &rd_set))
			      need_to_check = 1;

				if (need_to_check)
				{
					sock_unit->stat = sock_state_failed;
					ret = getsockopt(sock_unit->soc_handle, SOL_SOCKET, SO_ERROR, (char*)&error, &error_len);	//获取错误状态
					if (ret < 0)
						sock_unit->last_error = -1;
					else
						sock_unit->last_error = error;
					ISP_LOGERR("conn err %d\r\n", sock_unit->last_error);
					if (sock_unit->error_cb)
						sock_unit->error_cb(sock_unit, sock_unit->last_error, "connect error", sock_unit->user_data);	//连接回调
				}
				else if (FD_ISSET(sock_unit->soc_handle, &wr_set))
				{
					sock_unit->stat = sock_state_connected;	//已连接
					sock_unit->last_error = 0;
					ISP_LOGDBG("conn ok.\r\n");
					{
						if (sock_unit->connted_cb)
							(sock_unit->connted_cb)(sock_unit,sock_unit->user_data);		//连接回调
					}
				}
			}
		}
		else if (sock_unit->stat == sock_state_connected)
		{
			//ISP_LOGDBG("snd rcv check");
			if (ret == 0)
			{
				//if (sock_unit->snd_rcv_to_ > 0 &&	//目前snd_rcv_to_值恒为-1，这里进不去
				//	msp_tickcount() - sock_unit->snd_rcv_time_ > sock_unit->snd_rcv_to_)
				//{
				//	ISP_LOGDBG("snd rcv timeout");
				//	sock_unit->last_error = MSP_ERROR_TIME_OUT;
				//	if (sock_unit->exception_cb)
				//		(sock_unit->exception_cb)(sock_unit->hd_);//异常回调
				//}
			}
			else if (ret < 0)
			{
				//ISP_LOGDBG("snd rcv exception, ret = %d", ISP_NGETLASTERR);
				///* Exception */
				//sock_unit->stat = (sock_unit->stat == sock_state_closed)? sock_state_closed : sock_state_failed;
				//sock_unit->last_error = ISP_NGETLASTERR;
				//if ( sock_unit->exception_cb )
				//	(sock_unit->exception_cb)(sock_unit->hd_);
			}
			else
			{
				if (FD_ISSET(sock_unit->soc_handle, &rd_set))	//当前socket可读
				{
#define RECV_BUFLEN (1024)
					char buf[RECV_BUFLEN];
					int len = RECV_BUFLEN;
					ISP_LOGDBG("snd rcv got recv\r\n");
					while (RECV_BUFLEN == len)
					{
						
						len = recv(sock_unit->soc_handle, buf, RECV_BUFLEN, 0);
						printf("len: %d\n",len);
						if (len == 0)	//连接终止
						{
							/* socket shutdown by remote */
							sock_unit->stat = sock_state_failed;
							sock_unit->last_error = ESHUTDOWN;
							ISP_LOGWARN("_isp_sock_recv| leave for socket closed.\r\n");
							break;
						}
						else if (len == -1)
						{
							sock_unit->last_error = ISP_NGETLASTERR;	//记录错误码

							if (sock_unit->last_error == EINTR || sock_unit->last_error == EAGAIN || sock_unit->last_error == EWOULDBLOCK || sock_unit->last_error == ENOBUFS)
							{
								ISP_LOGDBG("_isp_sock_recv| leave with block, ret = %d.\r\n", ret);
								break;
							}
							ISP_LOGERR("_isp_sock_recv| leave for error %d.\r\n", sock_unit->last_error);
							break;
						}
						if (sock_unit->recv_cb)
							(sock_unit->recv_cb)(sock_unit, buf, len, sock_unit->user_data);	//回调 
					}

				}
				if (FD_ISSET(sock_unit->soc_handle, &wr_set))	//当前socket可写
				{
					ISP_LOGDBG("snd rcv got send\r\n");
					//if (sock_unit->output_cb)
					//	(sock_unit->output_cb)(sock_unit->hd_);	//回调
				}
				if (FD_ISSET(sock_unit->soc_handle, &ex_set))	//当前socket有异常
				{
					ISP_LOGERR("snd rcv got exception\r\n");
					//sock_unit->last_error = ISP_NGETLASTERR;
					if (sock_unit->error_cb)
						sock_unit->error_cb(sock_unit, sock_unit->last_error, "socket error", sock_unit->user_data);
				}
			}
		}

		//ISP_LOGDBG("next sock");
		//pnode = msp_list_peek_next(&g_socket_list, pnode);
		//sock_unit = sock_unit->next;
		list_node_temp = list_peek_next(&g_socket_list,list_node_temp);
	}
exit:
	native_mutex_given(g_sock_mutex);
	//ISP_LOGVERBOSE("socket_run[out].\r\n");
	return 0;
}


int socket_close(SOCKET_HANDLE s)
{
	int ret = MSP_SUCCESS;
	list_node * target_socket= NULL; 
	MSPSocket *mspskt = s;
	native_mutex_take(g_sock_mutex, INFINITE);
	target_socket = list_search(&g_socket_list,skt_handle_compare,(void *)(&mspskt->soc_handle));
	native_mutex_given(g_sock_mutex);
	if (NULL == target_socket)
	{
		ISP_LOGDBG("\ntls_socket_close NULL == s\n");
		native_mutex_given(g_sock_mutex);
		return MSP_ERROR_INVALID_PARA_VALUE;
	}
	if(NULL == mspskt)
	{
		return MSP_ERROR_INVALID_HANDLE;
	}
	
	native_mutex_take(g_sock_mutex, INFINITE);
	mspskt = (MSPSocket*) target_socket->val;
	/*while(q->next != mspskt && q->next != NULL)
		q = q->next;
	if(q->next != NULL)
		q->next = mspskt->next;*/
	ret = close(mspskt->soc_handle);
	if (MSP_SUCCESS != ret)
	{
		ISP_LOGERR("\ntls_socket_close ret %d\n", ret);
	}
	mspskt->stat = sock_state_closed;

	native_mutex_given(g_sock_mutex);

	MFREE((void*)s);
error:
exit:
	return ret;
}

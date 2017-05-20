#ifndef _MSPSOCKET_H_q2oujlgja_
#define _MSPSOCKET_H_q2oujlgja_
/* =========================================================================

DESCRIPTION
   MSP Socket Header.

Copyright (c) 2014 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2012/08/21      chenzhang        Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include <netinet/in.h>

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
typedef void* SOCKET_HANDLE;

#define MSPSOCK_AF_INET (1)
#define MSPSOCK_AF_INET6 (2)

#define MSPSOCK_TYPE_TCP (1)
#define MSPSOCK_TYPE_UDP (2)


/* 单socket发送缓冲 */
#define DEFAULT_SEND_BUFF_SIZE	64 * 1024

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

typedef void ( * skt_connted_fn)(SOCKET_HANDLE s, void *user_data);
typedef void ( * skt_recv_fn)(SOCKET_HANDLE s, const void *buf, int len, void *user_data);
typedef void ( * skt_error_fn)(SOCKET_HANDLE s, int errcode, const char *detail, void *user_data);

typedef int ISP_SOCKET;
typedef ISP_SOCKET	NATIVE_SOCKET_HANDLE;

typedef struct
{
	char	buff_[DEFAULT_SEND_BUFF_SIZE];
	int		len_;							/* 数据长度 */
	int		wr_ptr_;						/* 写入指针，指向可写的缓冲偏移 */
	int		rd_ptr_;						/* 读取指针，指向可读的缓冲偏移 */
}ISP_Sock_Send_Buff;

typedef union _mspsaddr {
	struct sockaddr sa;
	struct sockaddr_in  sa4;
	struct sockaddr_in6 sa6;
} mspsaddr;

typedef struct _MSPSocket {
	int domain;
	int type;
	int last_error;
	skt_connted_fn connted_cb;
	skt_recv_fn recv_cb;
	skt_error_fn error_cb;
	int stat;
	int conn_timeout;
	int conn_time;
	ISP_Sock_Send_Buff		*send_buff_;		//发送buffer
	void *user_data;
	NATIVE_SOCKET_HANDLE soc_handle;
	struct _MSPSocket *next;
} MSPSocket;

/* ------------------------------------------------------------------------
** Function Declarations
** ------------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif
extern int tls_socket_init();
extern int tls_socket_uninit();
extern int socket_open(SOCKET_HANDLE *s, int domain, int type, skt_connted_fn connted_cb, skt_recv_fn recv_cb, skt_error_fn error_cb, void *user_data);
extern int socket_connect (SOCKET_HANDLE s, const char *ip, short port);
extern int socket_send(SOCKET_HANDLE s, void *data, int len);
extern int socket_close(SOCKET_HANDLE s);
extern int socket_run(void *data);

#ifdef __cplusplus
}
#endif

#endif /* !_MSPSOCKET_H_q2oujlgja_ */

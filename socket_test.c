#include "AMSPSystem.h"
#include "AMSPSocket.h"
#include "MSPPlatform.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
NATIVE_MUTEX_HANDLE	g_mutex;
int g_conok = 0;
int g_getok = 0;
int g_constart = 0;
int g_getstart = 0;
SOCKET_HANDLE g_sd = 0;
static void socket_recv_cb(SOCKET_HANDLE hd, const void *buf, int len, void *user_data)
{
	printf("recv:%d,%s\n", len, (char *)buf);

	native_mutex_take(g_mutex, INFINITE);
	g_getok = 1;
	native_mutex_given(g_mutex);
}

static void socket_connted_cb(SOCKET_HANDLE mspskt, void *user_data)
{

	//printf("socket_connted_cb,time:%d\n", MSPSys_GetTickCount());
	native_mutex_take(g_mutex, INFINITE);
	g_conok = 1;
	native_mutex_given(g_mutex);

}

static void socket_error_cb(SOCKET_HANDLE hd, int errcode, const char *detail, void *user_data)
{
	printf("socket_error_cb:%d,%s,time:%d\n", errcode, detail, MSPSys_GetTickCount());
}
void iot_socket_test_init()
{
	g_conok = 0;
	g_getok = 0;
	g_constart = 0;
	g_getstart = 0;
	g_sd = 0;
	g_mutex = native_mutex_create("global_mutex", NULL);
}
void iot_socket_test_fini()
{
	native_mutex_destroy(g_mutex);
}
int iot_socket_test_run()
{
	int conok = 0;
	int getok = 0;
	int ret = 0;

	native_mutex_take(g_mutex, INFINITE);
	conok = g_conok;
	native_mutex_given(g_mutex);
	if (0 == conok && 0 == g_constart)
	{
		printf("creat socket\n");
		ret = socket_open(&g_sd, MSPSOCK_TYPE_TCP, MSPSOCK_AF_INET, socket_connted_cb, socket_recv_cb, socket_error_cb, (void*)NULL);
		if (0 != ret) {
			printf("error:%d,%d\n", ret, __LINE__);
			goto err;
		}
		g_constart = 1;
		printf("starting connecting\n");
		ret = socket_connect(g_sd, "60.166.12.146", 80);
		if (0 != ret) {
			//DBG_LOG_ERROR("error:%d,%d\n", ret, __LINE__);
			goto err;
		}
		((MSPSocket*)g_sd)->connted_cb(g_sd,(void*)NULL);
	}
	native_mutex_take(g_mutex, INFINITE);
	conok = g_conok ;
	native_mutex_given(g_mutex);
	printf("conok: %d\n",conok);
	if (0 == conok)
		return 0;
	native_mutex_take(g_mutex, INFINITE);
	getok = g_getok;
	native_mutex_given(g_mutex);
	printf("getok : %d\n g_getstart: %d\n",getok,g_getstart);
	if (0 == getok && 0 == g_getstart)
	{
		const char* get_push_node_string = "GET /ipcfg.txt HTTP/1.1\r\nHost: dev.voicecloud.cn\r\nUser-Agent: iFLYCloud\r\nConnection: Close\r\n\r\n";
		printf("start sending\n");
		//const char* get_push_node_string = "GET /iot_pushnode.do?prot_type=tcp HTTP/1.1\r\nHost: openapi.linkspeech.cn\r\n\r\n";
		printf("send data to socket buff\n");
		ret = socket_send(g_sd, (void *)get_push_node_string, strlen(get_push_node_string));
		if (0 != ret) {
			printf("error:%d,%d\n", ret, __LINE__);
			goto err;
		}	
		g_getstart = 1;
	}
	socket_run(NULL);;
	if (0 == getok)
		return 0;
	printf("get ok\n");
	return 9999;
err:
	return ret;
}
void main()
{
	int ret = 0;
	iot_socket_test_init();
	while(ret != 9999)
	{
		ret = iot_socket_test_run();
		sleep(1);
	}
	iot_socket_test_fini();
}
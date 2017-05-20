/* =========================================================================

DESCRIPTION
   MSP System Implemention.

Copyright (c) 2017 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2017/05/15     weizhang       Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include "AMSPSystem.h"
#include "MSPPlatform.h"

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */
#define MSP_OS_TYPE_STRING  "linux"
#define MSP_OS_VER_STRING   "linux"

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

unsigned long MSPSys_GetTime(void)
{
	time_t t;
	time(&t);
	return (unsigned long)t;
}

unsigned int MSPSys_GetTickCount(void)
{
	struct timespec tv;
	//clock_gettime(CLOCK_REALTIME, &tv);
	return (tv.tv_sec * 1000 + tv.tv_nsec / (1000 * 1000) );

}

const char* MSPSys_GetOSType(void)
{
	return MSP_OS_TYPE_STRING;
}

const char* MSPSys_GetOSVersion(void)
{
	return MSP_OS_VER_STRING;
}

static int MSPSys_Gettimeofday(MSPTimeVal* time_val)
{

	struct timeval tv;
	if (NULL == time_val){
		return -1;
	}
	gettimeofday(&tv, 0);
	time_val->tv_sec = (long long)tv.tv_sec;
	time_val->tv_usec = (unsigned long)tv.tv_usec;
	return 0;
}

void MSPSys_GetLocalTime(MSPLocalTime *ltime)
{
	/*MSPTimeVal time_val;
	time_t st_t;
	struct tm *st;
	double timevalue= 0;
	if (NULL == ltime){
		return;
	}
	MSPSys_Gettimeofday(&time_val);
	timevalue = time_val.tv_sec + time_val.tv_usec/1.e6;
	timevalue = timevalue ;
	time_val.tv_sec = (long long)timevalue;
	time_val.tv_usec = (unsigned long)((timevalue - (long long)timevalue)*1.e6);
	st_t = (time_t)time_val.tv_sec;
	st = localtime(&st_t);
	ltime->year = st->tm_year + 1900;
	ltime->month = st->tm_mon + 1;
	ltime->day = st->tm_mday;
	ltime->day_idx = st->tm_wday;
	ltime->hour = st->tm_hour;
	ltime->min = st->tm_min;
	ltime->sec = st->tm_sec;
	ltime->msec = (unsigned short)(time_val.tv_usec/1000);*/
}


NATIVE_MUTEX_HANDLE native_mutex_create(const char *name, void *param)
{
	NATIVE_MUTEX_HANDLE hd = NULL;
	hd = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); 
	if (NULL == hd)
		return NULL;

	pthread_mutex_init(hd, NULL);
	return hd;
}

int native_mutex_destroy(NATIVE_MUTEX_HANDLE hd)
{

	if (NULL == hd)
		return -1;

	pthread_mutex_destroy(hd);
	free((void *)hd); /* we cant use  MSPMemory here! */
	return 0;
}

int native_mutex_take(NATIVE_MUTEX_HANDLE hd, int timeout)
{
	if (NULL == hd)
		return -1;

	return pthread_mutex_lock(hd);
}

int native_mutex_given(NATIVE_MUTEX_HANDLE hd)
{
	if (NULL == hd)
		return -1;

	return pthread_mutex_unlock(hd);
}

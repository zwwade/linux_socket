#ifndef _AMSP_SYSTEM_H_nliuqek_
#define _AMSP_SYSTEM_H_nliuqek_
/* =========================================================================

DESCRIPTION
   MSP System Header.

Copyright (c) 2012 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2012/09/24      chenzhang        Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
typedef struct _MSPLocalTime {
	unsigned short year;
	unsigned short month;
	unsigned short day;
	unsigned short day_idx;
	unsigned short hour;
	unsigned short min;
	unsigned short sec;
	unsigned short msec;
} MSPLocalTime;

typedef struct _MSPTimeVal {
	long long tv_sec;
	unsigned long tv_usec;
} MSPTimeVal;

typedef void* NATIVE_MUTEX_HANDLE;
/* ------------------------------------------------------------------------
** Function Declarations
** ------------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long MSPSys_GetTime(void);
extern unsigned int MSPSys_GetTickCount(void);
extern void MSPSys_GetMACaddress(char szAddr[64]);
extern const char* MSPSys_GetOSType(void);
extern const char* MSPSys_GetOSVersion(void);
extern int MSPSys_GetStrLocalTime(char localTime[64]);


extern NATIVE_MUTEX_HANDLE native_mutex_create(const char *name, void *param);
extern int native_mutex_destroy(NATIVE_MUTEX_HANDLE hd);
extern int native_mutex_take(NATIVE_MUTEX_HANDLE hd, int timeout);
extern int native_mutex_given(NATIVE_MUTEX_HANDLE hd);

#ifdef __cplusplus
}
#endif

#endif /* !_AMSP_SYSTEM_H_nliuqek_ */


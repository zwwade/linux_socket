#ifndef _MSPPLATFORM_H_14uj09jtr_
#define _MSPPLATFORM_H_14uj09jtr_
/* =========================================================================

DESCRIPTION
   MSP Platform Header.

Copyright (c) 2012 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2012/07/18      chenzhang        Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>


/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */


typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed long long    INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long long  UINT64, *PUINT64;

typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef unsigned short int WORD;
typedef unsigned long DWORD;


#define INFINITE 0xFFFFFFFF


typedef int BOOL;
#define TRUE	1
#define FALSE	0


typedef void *	ISP_HANDLE;


/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */


#  if !defined (MSP_OS_TYPE)
#    define MSP_OS_LINUX 1 
#    if !defined (MSP_OS_STRING)
#      define MSP_OS_STRING "Linux"
#    endif
#    define MSP_OS_TYPE MSP_OS_LINUX
#    pragma message("MSP_OS_LINUX macro activated!")
#  endif

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Function Declarations
** ------------------------------------------------------------------------ */

#endif /* _MSPPLATFORM_H_14uj09jtr_ */

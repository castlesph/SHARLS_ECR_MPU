/* 
 * File:   dymaniclib.h
 * Author: PeyJiun
 *
 * Created on 2010?~12??6??, ?U?? 8:57
 */

#ifndef _MYFILEFUNC__H
#define	_MYFILEFUNC__H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdarg.h>

//#include <EMVTypedef.h>

typedef enum
{
	CREDIT_HOST,	
}HOST_INDEX_TABLE;
/*==========================================*
 *              File Setting                *	
 *	            D E F I N E S               *
 *==========================================*/           
#define d_BUFF_SIZE 128  //Buffer Size

typedef enum
{
    RC_FILE_READ_OUT_WRONG_SIZE         = -1,
    RC_FILE_REC_OR_RECSIZE_IS_NULL      = -2,	
    RC_FILE_NOT_EXIST                   = -3,
    RC_FILE_EXIST                       = -4,
    RC_FILE_TYPE_WRONG                  = -5,
    RC_FILE_FILE_NOT_FOUND              = -6,
    RC_FILE_DATA_WRONG                  = -7,
    RC_FILE_READ_OUT_NO_DATA            = -8,
}RESPONSE_CODE_TABLE;
 

/*==========================================*
 *                  TCT                     *                             
 *          (Terminal Config Table)         *	
 *		        D E F I N E S               *
 *==========================================*/  
#define PWD_LEN 	6
#define DEBUG_LOG_OFF   0
#define DEBUG_LOG_ON    1
#define DEBUG_IGNORE    2

/*==========================================*
 *                  TCT Struct              *
 *==========================================*/
typedef struct
{
    BYTE    byRS232DebugPort;//0 not debug, 8= USB debug, 1 =COM1 debug, 2=COM2 debug 
    BYTE    byRS232ECRPort;//0 not debug, 8= USB debug, 1 =COM1 debug, 2=COM2 debug 
    LONG    inECRListenPort;//0 not debug, 8= USB debug, 1 =COM1 debug, 2=COM2 debug 
    int		inECRIPHeader;
    USHORT	inWaitTime;
} STRUCT_TCT ;

typedef struct
{
	int	HDTid ;
	int		inCommunicationMode;
	
} STRUCT_CPT;

typedef struct
{
	BYTE	szTerminalIP[30] ;
    BYTE	szGetWay[30] ;
    BYTE	szSubNetMask[30] ;
    BYTE	szHostDNS1[30] ;
    BYTE	szHostDNS2[30] ;
    BOOL	fDHCPEnable ;
    BYTE	szAPN[30] ;
    BYTE	szUserName[30] ;
    BYTE	szPassword[30] ;

} STRUCT_TCP;


/*-------------------------TCT End----------------------------*/

/*==================================================*
 *                  Other Func                       
 *==================================================*/
BOOL fGetNumber(BYTE *szInBuffer, void *szOutBuffer, int inInBufferLen);
BOOL fGetBoolean(BYTE *szInBuffer);
BOOL fGetString(BYTE *szInBuffer, BYTE *szOutBuffer, int inInBufferLen);


/*==================================================*
 *                  Global Var                       
 *==================================================*/
STRUCT_TCT      strTCT;
STRUCT_CPT      strCPT;
STRUCT_TCP      strTCP;


#endif	/* _MYFILEFUNC__H */


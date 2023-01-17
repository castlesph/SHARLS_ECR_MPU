
#ifndef _DATABASEFUNC__H
#define	_DATABASEFUNC__H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>

#include <typedef.h>

#include "myFileFunc.h"
#include "../Includes/ECRTypedef.h"

//��ע�⣬��Ҫ����ļ���Ĵ�Сд
#define DB_TERMINAL     "/data/data/pub/TERMINAL.S3DB"
#define DB_MULTIAP	"/data/data/pub/MULTIAP.S3DB"

int inTCTRead(int inSeekCnt);
int inCPTRead(int inSeekCnt);
int inTCPRead(int inSeekCnt);


int inMultiAP_Database_BatchDelete(void);
int inMultiAP_Database_BatchInsert(TRANS_DATA_TABLE *transData);
int inCTOSS_MultiAPGetData(void);
int inHDTCheckByHostName(char *szHostName);
int inHDTCheckByHostNameECR(char *szHostName);

#endif	/* _DATABASEFUNC__H */


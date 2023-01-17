#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <ctosapi.h>
	
#include <resolv.h>
	
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/socket.h>   //connect,send,recv,setsockopt?
#include <sys/types.h>      
#include <sys/ioctl.h>

#include <netinet/in.h>     // sockaddr_in, "man 7 ip" ,htons
#include <poll.h>             //poll,pollfd
#include <arpa/inet.h>   //inet_addr,inet_aton
#include <unistd.h>        //read,write
#include <netdb.h>         //gethostbyname

#include <error.h>         //perror
#include <stdio.h>
#include <errno.h>         //errno



#include "../Includes/ECRTypedef.h"
#include "../Includes/myFileFunc.h"
#include "../debug/debug.h"

#include "../ECR/MultiAptrans.h"

#include "Bluetooth.h"

#define IPC_COMM_SUCC   100
#define IPC_COMM_FAIL   101

DWORD gdwState = 0;

USHORT CTOSS_BluetoothTxData(BYTE* baData, USHORT usLen)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;

	vdDebug_LogPrintf("**CTOSS_BluetoothTxData START,usLen=[%d]**",usLen);	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	//sprintf(bInBuf,"%d",1);
	//usInLen = strlen(bInBuf);
	memcpy(bInBuf, baData, usLen);
	usInLen = usLen;
	
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_TXDATA, bInBuf, usInLen, bOutBuf, &usOutLen);

	return ST_SUCCESS;
}



USHORT CTOSS_BluetoothRxReady(USHORT* pusLen)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;

	//vdDebug_LogPrintf("**CTOSS_BluetoothRxReady START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_RXREADY, bInBuf, usInLen, bOutBuf, &usOutLen);

	//vdDebug_LogPrintf("usResult [%d] usOutLen [%d]", usResult, usOutLen);	
	//DebugAddHEX("bOutBuf", bOutBuf, usOutLen);

	memcpy(pusLen, &bOutBuf[1], 2);

	//vdDebug_LogPrintf("*pusLen [%d]", *pusLen);

	return ST_SUCCESS;
}


USHORT CTOSS_BluetoothRxData(BYTE* baData, USHORT* pusLen)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;

	vdDebug_LogPrintf("**CTOSS_BluetoothRxData START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_RXDATA, bInBuf, usInLen, bOutBuf, &usOutLen);

	vdDebug_LogPrintf("usResult [%d] usOutLen [%d]", usResult, usOutLen);	
	DebugAddHEX("bOutBuf", bOutBuf, usOutLen);

	memcpy(pusLen, &bOutBuf[1], 2);

	memcpy(baData, &bOutBuf[1+2], *pusLen);
	
	return ST_SUCCESS;
}


USHORT CTOSS_BluetoothOpen(void)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;

	vdDebug_LogPrintf("**CTOSS_BluetoothOpen START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_OPEN, bInBuf, usInLen, bOutBuf, &usOutLen);

	return usResult;
}


USHORT CTOSS_BluetoothListen(BYTE* baServiceName, BYTE* baUUID)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;

	vdDebug_LogPrintf("**CTOSS_BluetoothListen START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_LISTEN, bInBuf, usInLen, bOutBuf, &usOutLen);

	return ST_SUCCESS;
}


USHORT CTOSS_BluetoothStatus(DWORD* pdwState)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;
	BYTE szType[4+1];

	//vdDebug_LogPrintf("**CTOSS_BluetoothStatus START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_STATUS, bInBuf, usInLen, bOutBuf, &usOutLen);

	
    //vdDebug_LogPrintf("---CTOSS_BluetoothStatus  usResult[%d][%02X]", usResult,bOutBuf[0]);
    if(d_OK == usResult)
    {
        //status
        if(bOutBuf[0] == IPC_COMM_SUCC)
		{
			memset(szType,0x00,sizeof(szType));
			memcpy(szType, &bOutBuf[1], 4);
			*pdwState = atoi(szType);
			gdwState = *pdwState;
			//vdDebug_LogPrintf("CTOS_BluetoothStatus dwState[%x],szType=[%s]", *pdwState,szType);
            return ST_SUCCESS;
        }	
        else
		{
            return d_NO;
        }
    }

	return d_NO;
}

int inCTOSS_BluetoothStatus(void)
{
	BYTE bInBuf[1024];
	BYTE bOutBuf[1024];
	BYTE *ptr = NULL;
	USHORT usInLen = 0;
	USHORT usOutLen = 0;
	USHORT usResult;
	BYTE szType[4+1];

	//vdDebug_LogPrintf("**CTOSS_BluetoothStatus START**");	
	memset(bOutBuf, 0x00, sizeof(bOutBuf));
	memset(bInBuf, 0x00, sizeof(bInBuf));
	sprintf(bInBuf,"%d",1);
	usInLen = strlen(bInBuf);
	usResult = inMultiAP_RunIPCCmdTypes("com.Source.SHARLS_BT.SHARLS_BT", d_IPC_CMD_BT_STATUS, bInBuf, usInLen, bOutBuf, &usOutLen);

	
    //vdDebug_LogPrintf("---CTOSS_BluetoothStatus  usResult[%d][%02X]", usResult,bOutBuf[0]);
    if(d_OK == usResult)
    {
        //status
        if(bOutBuf[0] == IPC_COMM_SUCC)
		{
			memset(szType,0x00,sizeof(szType));
			memcpy(szType, &bOutBuf[1], 4);
			gdwState = atoi(szType);

			vdDebug_LogPrintf("inCTOSS_BluetoothStatus gdwState[%x],", gdwState);
			if((gdwState & d_BLUETOOTH_STATE_CONNECTED) == d_BLUETOOTH_STATE_CONNECTED)
				return 1;
			else
				return 0;
			//vdDebug_LogPrintf("CTOS_BluetoothStatus dwState[%x],szType=[%s]", *pdwState,szType);
        }	
        else
		{
            return 0;
        }
    }

	return 0;
	
}



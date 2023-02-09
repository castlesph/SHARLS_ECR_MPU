/*******************************************************************************

*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctosapi.h>
#include <EMVLib.h>
//#include <libxml/tree.h>

//#include "..\Includes\emvaplib.h"
//#include "..\Includes\EMVTrans.h"
#include "..\Includes\DataBaseFunc.h"
#include "..\Includes\myFileFunc.h"
#include "../debug/debug.h"
#include "..\Includes\utils.h"
#include "..\Includes\ECRTypedef.h"
#include "..\ECR\MultiAptrans.h"
#include "..\ECR\ECRTrans.h"
#include "..\TcpServer\VCommTCP.h"
#include "..\Bluetooth\Bluetooth.h"
#include "..\Md5\CRCmd5.h"
#include "../cJSON/cJSON.h"
#include "../Md5/CRCmd5.h"
//#include "..\cJSON\cJSON.h"

//#include <MultiAplib.h>
int g_IPCEventID;

BYTE ECRPort = d_DEBUG_PORT;
BYTE szECRRecvData[4096 + 2];
BYTE szECRSendData[4096 + 2];

USHORT lECRRecvLen;
LONG lECRSendLen;
int MultiECRflag = 0;

#define d_KBD_SPEC	0xFE
extern char g_szAPName[25];

#define d_ETHERNET						9
#define d_BLUETOOTH						6
extern BOOL initEcrComOk;
BOOL isEcrProcessing = FALSE;
char szAcquirerName[24+1];

int inECR_InitCOM(void)
{
	char szBaud[3];
	int inResult = d_OK;

//	strTCT.byRS232ECRPort=2;

    vdDebug_LogPrintf("inECR_InitCOM[%s]=[%d],listen[%d]", "SHARE_ECR" ,strTCT.byRS232ECRPort, strTCT.inECRListenPort);

	if (8 ==strTCT.byRS232ECRPort)
	{
		ECRPort = 0xFF;
		inResult = CTOS_USBOpen();
	}

	if (1 ==strTCT.byRS232ECRPort)
	{
		ECRPort = d_COM1;
		//inResult = CTOS_RS232Open(ECRPort, 115200, 'N', 8, 1);
		inResult = CTOS_RS232Open(ECRPort, 9600, 'N', 8, 1);
	}

	if (2 ==strTCT.byRS232ECRPort)
	{
		ECRPort = d_COM2;
		inResult = CTOS_RS232Open(ECRPort, 9600, 'N', 8, 1);
	}

	if (9 ==strTCT.byRS232ECRPort)
	{
		ECRPort = d_ETHERNET;
		inEthernet_InitTerminal();
    
    	inResult = TCP_StartTCPServer(0,strTCT.inECRListenPort);
    	vdDebug_LogPrintf("StartTCPServer = [%d]",inResult);
	}

	if (6 ==strTCT.byRS232ECRPort)
	{
		ECRPort = d_BLUETOOTH;
		//inEthernet_InitTerminal();
    
    	//inResult = TCP_StartTCPServer(0,strTCT.inECRListenPort);
    	//vdDebug_LogPrintf("StartTCPServer = [%d]",inResult);
    	CTOSS_BluetoothOpen();
		CTOSS_BluetoothListen("MP200", "");
	}

	//-----------------------------------------------------------
	// FOR BASE USB CDC -- sidumili
	//-----------------------------------------------------------
	if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort){
		ECRPort = 0xFF;
		inResult = CTOS_BaseUSBOpen();
		// Flush current tx/rx -- sidumili
		CTOS_BaseUSBTxFlush();  
		CTOS_BaseUSBRxFlush();
	}
	//-----------------------------------------------------------
	// FOR BASE USB CDC -- sidumili
	//-----------------------------------------------------------

	return inResult;
}

//return 1 is sale, 2 is void, 3 is refund, 4 is settle all, 5 is settle a host
//pTxnResult = payment command + payment type
int inCTOSS_CheckECREventEx(char *pTxnResult)
{
	ULONG tick;
	USHORT ret;
    unsigned char ucTemp;
	char sRemoteClientIp[50];
    char iRemoteClientPort[10];
    int EcrTxnType = 0;
	VS_BOOL fValidate = VS_TRUE;
//	lECRRecvLen = sizeof(szECRRecvData);
	MultiECRflag = 0;
	memset(sRemoteClientIp,0x00,sizeof(sRemoteClientIp));
    memset(iRemoteClientPort,0x00,sizeof(iRemoteClientPort));
//    vdDebug_LogPrintf("inCTOSS_CheckECREvent, strTCT.byRS232ECRPort = [%x], ECRPort=%d, initEcrComOk=%d", strTCT.byRS232ECRPort, ECRPort, initEcrComOk);
    int inContentLength=0;
  
	if (0 ==strTCT.byRS232ECRPort)
		return(ERR_COM_TIMEOUT);

	if(!initEcrComOk)
    {
        if(inECR_InitCOM() == d_OK)
        {
            initEcrComOk = TRUE;
        }
    }
	tick = CTOS_TickGet();
	do {
		CTOS_Delay(50);
		
		if (ECRPort == 0xFF){
		  if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_USBRxReady(&lECRRecvLen);
		  
		  // FOR BASE USB CDC -- sidumili	
		  if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_BaseUSBRxReady(&lECRRecvLen);
		  
		  // FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
		  ret = TCP_CheckServerAccepted(sRemoteClientIp, iRemoteClientPort);
		else if (ECRPort == d_BLUETOOTH)
			ret = CTOSS_BluetoothRxReady(&lECRRecvLen);		
		else			
		    ret = CTOS_RS232RxReady(ECRPort, &lECRRecvLen);
		if (ret == d_OK)
			break;
	//}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);// for speed
	}while (0);
//    vdDebug_LogPrintf("check ret=%d", ret);
	if (ret != d_OK)
		return(ERR_COM_TIMEOUT);

	if (lECRRecvLen <= 0 && ECRPort != d_ETHERNET)
		return(ERR_COM_TIMEOUT);

	memset(szECRRecvData, 0x00, sizeof(szECRRecvData));
//	lECRRecvLen = sizeof(szECRRecvData);
	
	if (ret == d_OK) {
		if (ECRPort == 0xFF){
			if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				CTOS_USBRxData(szECRRecvData, &lECRRecvLen);
			
			// FOR BASE USB CDC -- sidumili
			if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
		    	CTOS_BaseUSBRxData(szECRRecvData, &lECRRecvLen);
			
			// FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
		  	TCP_ServerRecv(szECRRecvData,4096,&lECRRecvLen,strTCT.inECRIPHeader,60);
		else if (ECRPort == d_BLUETOOTH)
			CTOSS_BluetoothRxData(szECRRecvData, &lECRRecvLen);
		else			
		{
			CTOS_Delay(50);
			lECRRecvLen = 2048;
			CTOS_RS232RxData(ECRPort, szECRRecvData, &lECRRecvLen);
		}
		tick = CTOS_TickGet();
		
 		//inPrintECRPacket("ECR Recv Data", szECRRecvData, lECRRecvLen);
 		vdPCIDebug_HexPrintf("ECR Recv Data", szECRRecvData, lECRRecvLen);

		vdDebug_LogPrintf("raw got ecr data, [%s]", szECRRecvData);
        vdDebug_LogPrintf("lECRRecvLen [%d]", lECRRecvLen);
		
#if 0
		vdDebug_LogPrintf("getMD5 [%s]", getMD5(szECRRecvData, "X-MD5"));
		
		inContentLength=getContentLength(szECRRecvData, "Content-Length");
		vdDebug_LogPrintf("getContentLength [%d]",inContentLength);
		
        vdDebug_LogPrintf("data [%s]", szECRRecvData+(lECRRecvLen-inContentLength));

		memset(szECRSendData, 0, sizeof(szECRSendData));
		strcpy(szECRSendData, "@");
		strcat(szECRSendData, getMD5(szECRRecvData, "X-MD5"));
		strcat(szECRSendData, szECRRecvData+(lECRRecvLen-inContentLength));
		vdDebug_LogPrintf("new request [%s]", szECRSendData);

		memset(szECRRecvData, 0, sizeof(szECRRecvData));
		strcpy(szECRRecvData, szECRSendData);
		lECRRecvLen=strlen(szECRRecvData);

		vdDebug_LogPrintf("2.new request [%s]", szECRSendData);
#endif

		
        if (fValidate == VS_TRUE) {
            if(lECRRecvLen > 0)
            {
                vdDebug_LogPrintf("got ecr data, [%d]", lECRSendLen);
            }
            if (inValidateJSONPacket(szECRRecvData, lECRRecvLen) != VS_SUCCESS) 
			{
				if (inValidateLastPacket() == VS_SUCCESS)
				{
	            	CTOS_KBDBufPut(d_KBD_SPEC);
	                ucTemp = ACK;
					if (ECRPort == 0xFF){
						if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
							CTOS_USBTxData((char *) &ucTemp, 1);
						
						// FOR BASE USB CDC -- sidumili
						if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
					    	CTOS_BaseUSBTxData((char *) &ucTemp, 1);
						
						// FOR BASE USB CDC -- sidumili
					}
					else if (ECRPort == d_ETHERNET)
						TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
					else if (ECRPort == d_BLUETOOTH)
						CTOSS_BluetoothTxData((char *) &ucTemp, 1);
					else
						CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
	                 vdDebug_LogPrintf("ACK");
					 
	                inECRReceiveAnalyse();
					vdDebug_LogPrintf("MultiECRflag=[%d]",MultiECRflag);
					/*
					if (MultiECRflag == 1)
					{
						if (ECRPort == d_ETHERNET)
							TCP_ServerCloseSocket();
						return(ERR_COM_TIMEOUT);
					}
					inECRSendAnalyse();

					if (ECRPort == d_ETHERNET)
						TCP_ServerCloseSocket();
					*/
				}
				else
				{
                	ucTemp = NAK;
					if (ECRPort == 0xFF){
						if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
							CTOS_USBTxData((char *) &ucTemp, 1);
						
						// FOR BASE USB CDC -- sidumili
						if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
					    	CTOS_BaseUSBTxData((char *) &ucTemp, 1);
						
						// FOR BASE USB CDC -- sidumili
					}
					else if (ECRPort == d_ETHERNET)
					{
			  			TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
						TCP_ServerCloseSocket();
					}
					else if (ECRPort == d_BLUETOOTH)
						CTOSS_BluetoothTxData((char *) &ucTemp, 1);
					else
						CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
					return(ERR_COM_WRITE);
				}
            }
			else
			{
//				if(isEcrProcessing)//NAK, if ecr is still processing
				if(FALSE)
				{
					vdDebug_LogPrintf("Still Busy....");
					ucTemp = NAK;
					if (ECRPort == 0xFF){
						if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
							CTOS_USBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
						if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
							CTOS_BaseUSBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
					}
					else if (ECRPort == d_ETHERNET)
					{
						TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
						TCP_ServerCloseSocket();
					}
					else if (ECRPort == d_BLUETOOTH)
						CTOSS_BluetoothTxData((char *) &ucTemp, 1);
					else
						CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
					return(ERR_COM_WRITE);
				}
            	CTOS_KBDBufPut(d_KBD_SPEC);
                ucTemp = ACK;

                vdDebug_LogPrintf("Validate Json ok[%x]", ACK);
				if (ECRPort == 0xFF){
					if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
						CTOS_USBTxData((char *) &ucTemp, 1);
					
					// FOR BASE USB CDC -- sidumili
					if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
						CTOS_BaseUSBTxData((char *) &ucTemp, 1);
					
					// FOR BASE USB CDC -- sidumili
				}
				else if (ECRPort == d_ETHERNET)
				{
					vdDebug_LogPrintf("x1.d_ETHERNET send ack");
					//TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
					vdDebug_LogPrintf("x2.d_ETHERNET send ack");
					//inECRSendResponse2();
				}
				else if (ECRPort == d_BLUETOOTH)
					CTOSS_BluetoothTxData((char *) &ucTemp, 1);
				else
					CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
                 vdDebug_LogPrintf("ACK");
                inECRReceiveAnalyse();
				vdDebug_LogPrintf("MultiECRflag=[%d],ECRReq.txn_code=[%s]",MultiECRflag, ECRReq.txn_code);
                //Start: Add Ecr Service
				if (MultiECRflag == 1)
				{
						if (ECRPort == d_ETHERNET)
							TCP_ServerCloseSocket();
						return(ERR_COM_TIMEOUT);
					}

				if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2))
				{
					strcat(pTxnResult, ECR_REQ_CMD_SALE);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 1;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2))
				{
					strcat(pTxnResult, ECR_REQ_CMD_VOID);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 2;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_REFUND_TAG, 2))
				{
					strcat(pTxnResult, ECR_REQ_CMD_REFUND);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 3;
				}
                else if(!memcmp(ECRReq.txn_code, ECR_SETTLE_ALL_TAG, 2))
                {
					strcat(pTxnResult, ECR_REQ_CMD_SETTLE_ALL);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 4;
                }
                else if(!memcmp(ECRReq.txn_code, ECR_SETTLEMENT_TAG, 2))
                {
					strcat(pTxnResult, ECR_REQ_CMD_SETTLE);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 5;
                }

				else if(!memcmp(ECRReq.txn_code, ECR_REPRINT_TAG, 2))
				{
					if(atoi(ECRReq.inv_no))
					{
						strcat(pTxnResult, ECR_REQ_CMD_REPRINTANY);
						strcat(pTxnResult, "|");
						strcat(pTxnResult, ECRReq.szTYPE);
						EcrTxnType = 6;
					}
					else
					{
						strcat(pTxnResult, ECR_REQ_CMD_REPRINTLAST);
						strcat(pTxnResult, "|");
						strcat(pTxnResult, ECRReq.szTYPE);
						EcrTxnType = 7;
					}
				}

                //To reprint last settle
                else if(!memcmp(ECRReq.txn_code, ECR_REPRINT_SETTLEMENT_TAG, 2))
                {
                    strcat(pTxnResult, ECR_REQ_CMD_REPRINT_SETTLE);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 8;
                }

                else if(!memcmp(ECRReq.txn_code, ECR_TIP_ADJUST_TAG, 2))
                {
                    strcat(pTxnResult, ECR_REQ_CMD_TIPS);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 9;
                }

                else if(!memcmp(ECRReq.txn_code, ECR_INSTALLMENT_TAG, 2)){
					strcat(pTxnResult, ECR_REQ_CMD_INSTALLMENT);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 10;
                }

                else if(!memcmp(ECRReq.txn_code, ECR_RETRIEVAL_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_RETRIEVAL);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 11;
                }

                else if(!memcmp(ECRReq.txn_code, ECR_TOTAL_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_BATCH_TOTAL);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 12;
                }

				else if(!memcmp(ECRReq.txn_code, ECR_PREAUTH_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_PREAUTH);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 13;
                }

				else if(!memcmp(ECRReq.txn_code, ECR_PREAUTHCOMP_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_PREAUTH_COMP);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 14;
                }

                else if(!memcmp(ECRReq.txn_code, ECR_ECHOTEST_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_ECHO_TEST);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 15;
                }
				else if(!memcmp(ECRReq.txn_code, ECR_GET_TOTAL_TAG, 2)){
					strcat(pTxnResult, ECR_REQ_CMD_GET_TOTAL);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 16;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_GET_LAST_TOTAL_TAG, 2)){
					strcat(pTxnResult, ECR_REQ_CMD_GET_LAST_TOTAL);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 17;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_GET_BARCODE_TAG, 2)){
					strcpy(ECRReq.szTYPE, "TYPE");
					strcat(pTxnResult, ECR_REQ_CMD_BARCODE);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 18;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_PRINTING_TAG, 2)){
					strcpy(ECRReq.szTYPE, "TYPE");
					strcat(pTxnResult, ECR_REQ_CMD_PRINTING);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 19;
				}
				else if(!memcmp(ECRReq.txn_code, ECR_READCARD_TAG, 2)){
					strcpy(ECRReq.szTYPE, "TYPE");
					strcat(pTxnResult, ECR_REQ_CMD_READCARD);
					strcat(pTxnResult, "|");
					strcat(pTxnResult, ECRReq.szTYPE);
					EcrTxnType = 20;
				}
				
				else if(!memcmp(ECRReq.txn_code, ECR_VOID_PREAUTH_TAG, 2)){
                    strcat(pTxnResult, ECR_REQ_CMD_VOID_PREAUTH);
                    strcat(pTxnResult, "|");
                    strcat(pTxnResult, ECRReq.szTYPE);
                    EcrTxnType = 21;
				}
                //End
/*				inECRSendAnalyse();

				if (ECRPort == d_ETHERNET)
					TCP_ServerCloseSocket();*/
			}
        }
	}
	//CTOS_RS232Close(d_DEBUG_PORT);
	
	lECRRecvLen = 0;
    vdDebug_LogPrintf("EcrTxnType=[%d]", EcrTxnType);
	return EcrTxnType;
}


int inCTOSS_CheckECREvent(void)
{
	ULONG tick;
	USHORT ret;
	unsigned char ucTemp;
	char sRemoteClientIp[50];
	char iRemoteClientPort[10];
	int isTriggerECR = 0;
	VS_BOOL fValidate = VS_TRUE;
//	lECRRecvLen = sizeof(szECRRecvData);
	MultiECRflag = 0;
	memset(sRemoteClientIp,0x00,sizeof(sRemoteClientIp));
	memset(iRemoteClientPort,0x00,sizeof(iRemoteClientPort));
	vdDebug_LogPrintf("inCTOSS_CheckECREvent, strTCT.byRS232ECRPort = [%x], ECRPort=%d", strTCT.byRS232ECRPort, ECRPort);
	if (0 ==strTCT.byRS232ECRPort)
		return(ERR_COM_TIMEOUT);

	tick = CTOS_TickGet();
	do {
		CTOS_Delay(50);

		if (ECRPort == 0xFF){
			if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				ret = CTOS_USBRxReady(&lECRRecvLen);

			// FOR BASE USB CDC -- sidumili
			if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
				ret = CTOS_BaseUSBRxReady(&lECRRecvLen);

			// FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
			ret = TCP_CheckServerAccepted(sRemoteClientIp, iRemoteClientPort);
		else if (ECRPort == d_BLUETOOTH)
			ret = CTOSS_BluetoothRxReady(&lECRRecvLen);
		else
			ret = CTOS_RS232RxReady(ECRPort, &lECRRecvLen);
		if (ret == d_OK)
			break;
		//}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);// for speed
	}while (0);

	if (ret != d_OK)
		return(ERR_COM_TIMEOUT);

	if (lECRRecvLen <= 0 && ECRPort != d_ETHERNET)
		return(ERR_COM_TIMEOUT);

	memset(szECRRecvData, 0x00, sizeof(szECRRecvData));
//	lECRRecvLen = sizeof(szECRRecvData);

	if (ret == d_OK) {
		if (ECRPort == 0xFF){
			if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				CTOS_USBRxData(szECRRecvData, &lECRRecvLen);

			// FOR BASE USB CDC -- sidumili
			if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
				CTOS_BaseUSBRxData(szECRRecvData, &lECRRecvLen);

			// FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
			TCP_ServerRecv(szECRRecvData,4096,&lECRRecvLen,strTCT.inECRIPHeader,60);
		else if (ECRPort == d_BLUETOOTH)
			CTOSS_BluetoothRxData(szECRRecvData, &lECRRecvLen);
		else
		{
			CTOS_Delay(50);
			lECRRecvLen = 2048;
			CTOS_RS232RxData(ECRPort, szECRRecvData, &lECRRecvLen);
		}
		tick = CTOS_TickGet();

		//inPrintECRPacket("ECR Recv Data", szECRRecvData, lECRRecvLen);
		vdPCIDebug_HexPrintf("ECR Recv Data", szECRRecvData, lECRRecvLen);

		if (fValidate == VS_TRUE) {
			if (inValidateJSONPacket(szECRRecvData, lECRRecvLen) != VS_SUCCESS)
			{
				if (inValidateLastPacket() == VS_SUCCESS)
				{
					CTOS_KBDBufPut(d_KBD_SPEC);
					ucTemp = ACK;
					if (ECRPort == 0xFF){
						if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
							CTOS_USBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
						if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
							CTOS_BaseUSBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
					}
					else if (ECRPort == d_ETHERNET)
						TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
					else if (ECRPort == d_BLUETOOTH)
						CTOSS_BluetoothTxData((char *) &ucTemp, 1);
					else
						CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
					vdDebug_LogPrintf("ACK");

					inECRReceiveAnalyse();
					vdDebug_LogPrintf("MultiECRflag=[%d]",MultiECRflag);
					/*
					if (MultiECRflag == 1)
					{
						if (ECRPort == d_ETHERNET)
							TCP_ServerCloseSocket();
						return(ERR_COM_TIMEOUT);
					}
					inECRSendAnalyse();

					if (ECRPort == d_ETHERNET)
						TCP_ServerCloseSocket();
					*/
				}
				else
				{
					ucTemp = NAK;
					if (ECRPort == 0xFF){
						if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
							CTOS_USBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
						if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
							CTOS_BaseUSBTxData((char *) &ucTemp, 1);

						// FOR BASE USB CDC -- sidumili
					}
					else if (ECRPort == d_ETHERNET)
					{
						TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
						TCP_ServerCloseSocket();
					}
					else if (ECRPort == d_BLUETOOTH)
						CTOSS_BluetoothTxData((char *) &ucTemp, 1);
					else
						CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
					return(ERR_COM_WRITE);
				}
			}
			else
			{
				CTOS_KBDBufPut(d_KBD_SPEC);
				ucTemp = ACK;
				isTriggerECR = 1;
				vdDebug_LogPrintf("Validate Json ok");
				if (ECRPort == 0xFF){
					if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
						CTOS_USBTxData((char *) &ucTemp, 1);

					// FOR BASE USB CDC -- sidumili
					if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
						CTOS_BaseUSBTxData((char *) &ucTemp, 1);

					// FOR BASE USB CDC -- sidumili
				}
				else if (ECRPort == d_ETHERNET)
					TCP_ServerSend((char *) &ucTemp,1,strTCT.inECRIPHeader);
				else if (ECRPort == d_BLUETOOTH)
					CTOSS_BluetoothTxData((char *) &ucTemp, 1);
				else
					CTOS_RS232TxData(ECRPort, (char *) &ucTemp, 1);
				vdDebug_LogPrintf("ACK");
				inECRReceiveAnalyse();
				vdDebug_LogPrintf("MultiECRflag=[%d]",MultiECRflag);
				if (MultiECRflag == 1)
				{
					if (ECRPort == d_ETHERNET)
						TCP_ServerCloseSocket();
					return(ERR_COM_TIMEOUT);
				}
/*				inECRSendAnalyse();

				if (ECRPort == d_ETHERNET)
					TCP_ServerCloseSocket();*/
			}
		}
	}
	//CTOS_RS232Close(d_DEBUG_PORT);

	lECRRecvLen = 0;
	vdDebug_LogPrintf("isTriggerECR=[%d]", isTriggerECR);
	return isTriggerECR;
}

/****************************************************************************
 * Routine Name:  ucGetLRC()
 * Description :  Computes the LRC of the packet of given size.
 * Input       :  Request packet    -  pucPacket
 *                Packet size       -  inSize
 * Output      :  Computed LRC byte.
 * Notes       :  LRC is the XOR of all the bytes in the packet.
 *****************************************************************************/
unsigned char ucGetLRC(char* pucPacket,int inSize) {
    unsigned char ucLRC = *pucPacket++;  

    while (--inSize > 0)
        ucLRC ^= *pucPacket++;
    return(ucLRC);
}

/****************************************************************************
 * Routine Name:  inValidatePacket()
 * Description :  Validates a packet of the form <STX><Data><ETX><LRC>. 
 * Input       :  Packet      - pucPacket
 *                Packet size - inSize
 * Output      :  VS_SUCCESS     on success ( correct packet format )
 *                VS_ERR         on error.
 * Notes       :  Uses a local buffer for all calculations. Because of this 
 *                all outer layer callers can use the source and destination 
 *                as the same buffer. The packet formed will be null 
 *                terminated also.
 *****************************************************************************/
int inValidatePacket(char* pucPkt,int inSize) {
    if (inSize < 4)
        return(
        VS_ERR);   /* Atleast a pcaket should have <STX><1 char><ETX><LRC> */
    if (pucPkt[0] != STX)
        return(VS_ERR);
    if (pucPkt[inSize - 2] != ETX)
        return(VS_ERR);
   if (ucGetLRC(&(pucPkt[1]), (inSize - 2)) != pucPkt[inSize - 1]) 
        return(VS_ERR);
    return(VS_SUCCESS);
}

#if 1
int inValidateJSONPacket(char* pucPkt, int inSize) 
{
   unsigned char uszMD5Data[1024 + 1];    
   unsigned char uszMD5Hash[32 + 1];
   int inCount=0;
   
   vdDebug_LogPrintf("inValidateJSONPacket inSize=[%d]", inSize);
   if (inSize < 33)
       return (VS_ERR);
   
   memset(uszMD5Data, 0x00, sizeof (uszMD5Data));
   memcpy(uszMD5Data, &pucPkt[33], inSize - 33);

   memset(uszMD5Hash, 0, sizeof(uszMD5Hash));
   memcpy(uszMD5Hash, &pucPkt[1], 32);
   
    for(inCount = 0; inCount < strlen(uszMD5Hash); inCount++) 
    {        
        uszMD5Hash[inCount] = toupper(uszMD5Hash[inCount]); 
    }
   
   vdDebug_LogPrintf("uszMD5Data =[%s]", uszMD5Data);
   vdDebug_LogPrintf("uszMD5Dataget =[%s]", MOLMD5String(uszMD5Data));
   
   //if (memcmp(MOLMD5String(uszMD5Data), &pucPkt[1], 32) == 0) 
   if (memcmp(MOLMD5String(uszMD5Data), uszMD5Hash, 32) == 0) 
   {
       lECRRecvLen = strlen(pucPkt) - 33;
       strcpy(szECRRecvData, uszMD5Data);
       vdDebug_LogPrintf("BINGO!!!");
       //setLCDPrint(12, 2, "BINGO!!!");
       return (VS_SUCCESS);
   } 
   else
   {
       //setLCDPrint(12, 2, "NOT BINGO");
       return (VS_ERR);
   }
}

#else
int inValidateJSONPacket(char* pucPkt,int inSize) {
	unsigned char uszMD5Data[500+1];

	vdDebug_LogPrintf("inValidateJSONPacket inSize=[%d]", inSize);
	if (inSize < 33)
		return(VS_ERR);
	
	memset(uszMD5Data, 0x00, sizeof(uszMD5Data));
	strncpy(uszMD5Data, &pucPkt[33] , strlen(pucPkt)-33);
	vdDebug_LogPrintf("uszMD5Data =[%s]", uszMD5Data);
	
	if (memcmp(MOLMD5String(uszMD5Data), &pucPkt[1], 32) == 0)
	{
		lECRRecvLen = strlen(pucPkt)-33;
		strcpy(szECRRecvData, uszMD5Data);
		vdDebug_LogPrintf("BINGO!!!");
		return(VS_SUCCESS);
	}
	else
        return(VS_ERR);
}
#endif

int inValidateLastPacket(void)
{
	char tmpbuf[256];
	int	inTemSize;
	int inSize;

	return(VS_ERR);
	
	inSize = lECRRecvLen;
	inTemSize = lECRRecvLen; 
	

	vdDebug_LogPrintf("inValidateLastPacket inSize=[%d],inTemSize=[%d]",inSize,inTemSize);
	if(inSize > 4)	
	{
		vdDebug_LogPrintf("inValidateLastPacket [inSize - 2]=[%02x]",szECRRecvData[inSize - 2]);
		if (szECRRecvData[inSize - 2] != ETX)
		{
			return(VS_ERR);
		}

		while (inTemSize--)
		{
			if(szECRRecvData[inTemSize] == STX)
			{
				break;
			}
		}

		vdDebug_LogPrintf("inTemSize=[%d]",inTemSize);
		if (inTemSize > 0) 
		{
			memset(tmpbuf,0x00,sizeof(tmpbuf));
			lECRRecvLen = inSize - inTemSize;
			memcpy(tmpbuf,&szECRRecvData[inTemSize],lECRRecvLen);

			memset(szECRRecvData,0x00,sizeof(szECRRecvData));
			memcpy(szECRRecvData,tmpbuf,lECRRecvLen);
			
			if(strTCT.byRS232DebugPort != 0)
					inPrintECRPacket("ECR Recv Data", szECRRecvData, lECRRecvLen);
			if (inValidatePacket(szECRRecvData, lECRRecvLen) != VS_SUCCESS)
				return(VS_ERR);
			else
			return(VS_SUCCESS);
		}
		else
		{
			vdDebug_LogPrintf("error message");
			return(VS_ERR);
		}
	}

	
}

int inCheckUSBorRS232RxReady(void)
{
	USHORT ret;
    unsigned char ucTemp;
    USHORT lTempLen = 0;

//	if (strTCT.fECR != 1)
//		return d_NO;
		
	if (ECRPort == 0xFF){
	  if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
	  	ret = CTOS_USBRxReady(&lTempLen);
	  
	  // FOR BASE USB CDC -- sidumili	
	  if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
	  	ret = CTOS_BaseUSBRxReady(&lTempLen);
	  
	  // FOR BASE USB CDC -- sidumili
	}
	else
	  ret = CTOS_RS232RxReady(ECRPort, &lTempLen);

    if (lTempLen > 0)
    {
    	vdDebug_LogPrintf("inCheckUSBorRS232RxReady lTempLen=[%d]",lTempLen);
        return d_OK;
    }
	return d_NO;
	
}

int inCTOSS_MultiAPSaveData_NOT_IPC(int IPC_EVENT_ID)
{
	BYTE bret;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;
    char szDataBuf[10];
	char szAPName[25];
	int inAPPID;
	int inResult;
	
	memset(outbuf,0x00,sizeof(outbuf));
	memset(szAPName,0x00,sizeof(szAPName));

//	inMultiAP_CurrentAPNamePID(szAPName, &inAPPID);

	vdDebug_LogPrintf("szAPName[%s]=[%s]", "V5S_VisaMaster" ,szAPName);
	
//	if (strcmp (strHDT.szAPName,szAPName) == 0)
//		return d_OK;

	bret= inMultiAP_Database_BatchDelete();
	vdDebug_LogPrintf("inMultiAP_Database_BatchDelete,bret=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchDelete ERR");
    	return bret;
	}
	
	bret = inMultiAP_Database_BatchInsert(&srTransRec);
	vdDebug_LogPrintf("inMultiAP_Database_BatchInsert=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchInsert ERR");
    	return bret;
	}

	vdDebug_LogPrintf("szAPName[%s],bret=[%d]", g_szAPName ,bret);

    memset(szDataBuf, 0x00, sizeof(szDataBuf));
	g_IPCEventID = IPC_EVENT_ID;
//	inMultiAP_RunIPCCmdTypesEx(g_szAPName ,IPC_EVENT_ID, szDataBuf, 0, outbuf,&out_len);
		
/*
	inTCTRead(1);

    vdDebug_LogPrintf("IPC_EVENT_ID [%d =? %d %d]", IPC_EVENT_ID, outbuf[0], outbuf[1]);
	
	if (outbuf[0] != IPC_EVENT_ID)
	{
//		vdSetErrorMessage("MultiAP Type ERR");
//		return d_NO;
	}

	if (outbuf[1] != d_SUCCESS)
	{
//		vdSetErrorMessage("MultiAP ERR");
//		return d_NO;
	}
*/	
	return d_OK;
}

int inCTOSS_MultiAPSaveData(int IPC_EVENT_ID)
{
	BYTE bret;
	BYTE outbuf[d_MAX_IPC_BUFFER];
	USHORT out_len = 0;
    char szDataBuf[10];
	char szAPName[25];
	int inAPPID;
	int inResult;
	
	memset(outbuf,0x00,sizeof(outbuf));
	memset(szAPName,0x00,sizeof(szAPName));

//	inMultiAP_CurrentAPNamePID(szAPName, &inAPPID);

	vdDebug_LogPrintf("szAPName[%s]=[%s]", "V5S_VisaMaster" ,szAPName);
	
//	if (strcmp (strHDT.szAPName,szAPName) == 0)
//		return d_OK;

	bret= inMultiAP_Database_BatchDelete();
	vdDebug_LogPrintf("inMultiAP_Database_BatchDelete,bret=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchDelete ERR");
    	return bret;
	}
	
	bret = inMultiAP_Database_BatchInsert(&srTransRec);
	vdDebug_LogPrintf("inMultiAP_Database_BatchInsert=[%d]", bret);
	if(d_OK != bret)
	{
//		vdSetErrorMessage("MultiAP BatchInsert ERR");
    	return bret;
	}

	vdDebug_LogPrintf("szAPName[%s],bret=[%d]", g_szAPName ,bret);

    memset(szDataBuf, 0x00, sizeof(szDataBuf));
	inMultiAP_RunIPCCmdTypesEx(g_szAPName ,IPC_EVENT_ID, szDataBuf, 0, outbuf,&out_len);
		

	inTCTRead(1);

    vdDebug_LogPrintf("IPC_EVENT_ID [%d =? %d %d]", IPC_EVENT_ID, outbuf[0], outbuf[1]);
	
	if (outbuf[0] != IPC_EVENT_ID)
	{
//		vdSetErrorMessage("MultiAP Type ERR");
//		return d_NO;
	}

	if (outbuf[1] != d_SUCCESS)
	{
//		vdSetErrorMessage("MultiAP ERR");
//		return d_NO;
	}
	
	return d_OK;
}

int inECRReceiveAnalyse(void)
{
	int inSize, inRcvLen, offset, retry;
	char szBuf[30];
	char chAmtFlag, chCBFlag, chInvFlag;
	char chResp;
	char szHandle[20];
	char szDSPBuf[100+1];
	char szHexBuf[100+1];
	long event;

	char stVerInfo[GEN_VER_SIZE];
	char szLogicalName[16];
	unsigned char szEventDataBuffer[512];
	int hConfFHandle = 0;
	int inOffset = 0;
	int inTestLen =0;
	int inEESLAPIReturnValue = 0;
	unsigned short ushEventDataSize = 0;
	unsigned short ushEESLAPIReturnValue = 0;
	int i;
	char szFulshBuf[256+1];
	char szTempLenBuf[4+1];
	unsigned char ucTemp;
	int inTemLen;
	int inRetVal = VS_SUCCESS;
	int inRet = VS_SUCCESS;
	cJSON * pJson = NULL;
	cJSON * pSub = NULL;

	long		lgTotalAmount = 0;
	long		lgBaseAmount = 0;
	long		lgTipAmount = 0;
	
	chECRFlag = 0;
	memset(&ECRResp,0x00, sizeof(ECRResp));
		
	memcpy(ECRResp.resp_code, ECR_UNKNOWN_ERR, 2);
	memcpy(ECRResp.resp_text, ECR_REMOVE_RESP, ECR_RESP_TEXT_SIZE);
	memcpy(ECRResp.date, "000000", DATE_SIZE);
	memcpy(ECRResp.time, "000000", TIME_SIZE);

	memset(ECRResp.prnt_text1, 0, ECR_PRNT_TEXT1_SIZE);
	memset(ECRResp.prnt_text2, 0, ECR_PRNT_TEXT2_SIZE);
	memset(ECRResp.prnt_text3, 0, ECR_PRNT_TEXT3_SIZE);
	memset(ECRResp.prnt_text4, 0, ECR_PRNT_TEXT4_SIZE);

	memcpy(&ECRResp.merchant_name[0], szBuf, 23);	
	memcpy(&ECRResp.merchant_name[23], szBuf, 23);	
	memcpy(&ECRResp.merchant_name[46], szBuf, 23);

	memset(szDSPBuf,0x00, sizeof(szDSPBuf));
	memset(szHexBuf,0x00, sizeof(szHexBuf));
	memset(szBuf,0x00,sizeof(szBuf));
	
	chAmtFlag = 0;
	chCBFlag = 0;
	chInvFlag = 0;
// patrick start parse JSON 
	 memset((char *)&ECRReq, 0, sizeof(ECR_REQ));
	 memset(&srTransRec, 0x00, sizeof(TRANS_DATA_TABLE));
	 lgTotalAmount = 0;
	 lgBaseAmount = 0;
	 lgTipAmount = 0;

	vdPCIDebug_HexPrintf("ECR Recv Data11", szECRRecvData, lECRRecvLen);
	vdDebug_LogPrintf("lECRRecvLen=[%d],szECRRecvData=[%s]",lECRRecvLen,szECRRecvData);
	 pJson = NULL;
	 pJson = cJSON_Parse(szECRRecvData);
	 if(NULL == pJson)																						   
	 {
			 vdDebug_LogPrintf("branch exit");
		// parse faild, return
		return BRANCH_EXIT;
	 }
	 
	 // get number from json
	 pSub = NULL;
	 pSub = cJSON_GetObjectItem(pJson, "AMT");
	 if(NULL == pSub)
	 {
			 vdDebug_LogPrintf("amt failed");
		 //get AMT from json faild
	 }
	 else
	{
		 vdDebug_LogPrintf("AMT : %012d\n", pSub->valueint);
		 lgBaseAmount = pSub->valueint;

		 sprintf(ECRReq.amount, "%012d", pSub->valueint);

		 ECRReq.amount[12] = 0;
		 
		 for (i=0; i<12;i++)
		 {
			 vdDebug_LogPrintf("ECRReq.amount[%d]=[%c]",i,ECRReq.amount[i]);
			 if (ECRReq.amount[i] < '0' || ECRReq.amount[i] > '9')
			 {
				 memset(ECRResp.resp_text, ' ', ECR_RESP_TEXT_SIZE);				 
				 strcpy(ECRResp.resp_text,"AMOUNT IS WRONG");
				 chECRFlag = 1;
				 return BRANCH_EXIT;
			 }
		 }
		 
		 if (atol(ECRReq.amount) == 0)
		 {
			 memset(ECRResp.resp_text, ' ', ECR_RESP_TEXT_SIZE);				 
			 strcpy(ECRResp.resp_text,"AMOUNT IS WRONG");
			 
			 chECRFlag = 1;
			 return BRANCH_EXIT;
		 }

		 wub_str_2_hex(ECRReq.amount, srTransRec.szBaseAmount, sizeof(ECRReq.amount));
		 DebugAddHEX("BaseAmt", srTransRec.szBaseAmount, 6);
		 chAmtFlag = 1;
	}
	
	 // get string from json
	 pSub = NULL;
	 pSub = cJSON_GetObjectItem(pJson, "ECRREF");
	if (NULL == pSub) {
		//get object named "ECRREF" faild
	} else {
		vdDebug_LogPrintf("ECRREF : %s\n", pSub->valuestring);
		strcpy(ECRReq.szECRREF, pSub->valuestring);
		strcpy(ECRReq.rref, pSub->valuestring);
		vdDebug_LogPrintf("ECR RRF:%s", ECRReq.rref);
	}

	// get string from json
	// JSON PARSER - CMD 
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "CMD");
	if (NULL == pSub) {
		//get object named "CMD" faild
	} else {
		vdDebug_LogPrintf("CMD : %s\n", pSub->valuestring);
		strcpy(ECRReq.szCMD, pSub->valuestring);

		if (memcmp(pSub->valuestring, "ECHOTEST", 4) == 0) {
			memcpy(ECRReq.txn_code, ECR_ECHOTEST_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}
		
		if (memcmp(pSub->valuestring, "SALE", 4) == 0) {
			memcpy(ECRReq.txn_code, ECR_SALE_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "VOID", 4) == 0) {
			if (memcmp(pSub->valuestring, "VOIDPREAUTH", 11) == 0) 
			{
				memcpy(ECRReq.txn_code, ECR_VOID_PREAUTH_TAG, TXN_CODE_SIZE);
			}
			else
			{
				memcpy(ECRReq.txn_code, ECR_VOID_TAG, TXN_CODE_SIZE);
			}
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "REFUND", 6) == 0) {
			memcpy(ECRReq.txn_code, ECR_REFUND_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "TIPS", 4) == 0) {
			memcpy(ECRReq.txn_code, ECR_TIP_ADJUST_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "INSTALMENT", 10) == 0) {
			memcpy(ECRReq.txn_code, ECR_INSTALLMENT_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "RETRIEVAL", 9) == 0) {
			memcpy(ECRReq.txn_code, ECR_RETRIEVAL_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "TOTAL", 5) == 0) {
			memcpy(ECRReq.txn_code, ECR_TOTAL_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

		if (memcmp(pSub->valuestring, "SETTLEMENT", 10) == 0) {
			memcpy(ECRReq.txn_code, ECR_SETTLE_ALL_TAG, TXN_CODE_SIZE);
			//memcpy(ECRReq.txn_code, ECR_SETTLEMENT_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}
		
		 //Reprint Any
		if (memcmp(pSub->valuestring, "REPRINT", 7) == 0) {
			memcpy(ECRReq.txn_code, ECR_REPRINT_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}
		
		//Reprint last settle
		vdDebug_PrintOnPaper("REPRINT_SETTLEMENT");
		if (memcmp(pSub->valuestring, "REPRINT_SETTLEMENT", 18) == 0) {
			vdDebug_PrintOnPaper("1.REPRINT_SETTLEMENT");
			memcpy(ECRReq.txn_code, ECR_REPRINT_SETTLEMENT_TAG, TXN_CODE_SIZE);
			vdDebug_PrintOnPaper("reprint settlement");
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}

    #ifdef ECR_PREAUTH_AND_COMP
		vdDebug_LogPrintf("pSub->valuestring:%s", pSub->valuestring);
		
		if (memcmp(pSub->valuestring, "PREAUTH", 7) == 0) {
			memcpy(ECRReq.txn_code, ECR_PREAUTH_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}
		
		if (memcmp(pSub->valuestring, "PREAUTHCOMP", 11) == 0) {
			memcpy(ECRReq.txn_code, ECR_PREAUTHCOMP_TAG, TXN_CODE_SIZE);
			ECRReq.txn_code[TXN_CODE_SIZE] = 0;
		}
    #endif
	}

	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "TYPE");
	if (NULL == pSub) {
		//get object named "TYPE" faild
	} else {
		vdDebug_LogPrintf("TYPE : (%s)\n", pSub->valuestring);
		strcpy(ECRReq.szTYPE, pSub->valuestring);

		if (memcmp(pSub->valuestring, "VMJ", 3) == 0) {
			//srTransRec.HDTid = inHDTCheckByHostName("CREDIT");
			srTransRec.HDTid = inHDTCheckByHostNameECR("CREDIT");
			vdDebug_PrintOnPaper("ECR CREDIT HDTid [%d]", srTransRec.HDTid);
//			  strcpy(srTransRec.szHostLabel, "CREDIT");
//			  srTransRec.HDTid = 1;
		}
		else if (memcmp(pSub->valuestring, "AE", 2) == 0) {
			if (memcmp(pSub->valuestring, "INSTALMENT", 10) == 0) {
				strcpy(srTransRec.szHostLabel, "AMEX EPP");
				srTransRec.HDTid = 6;
			} else {
				strcpy(srTransRec.szHostLabel, "AMEX");
				srTransRec.HDTid = 2;
			}
		} else if(memcmp(pSub->valuestring, "MPU", 3) == 0){
			//srTransRec.HDTid = inHDTCheckByHostName("MPU");
			srTransRec.HDTid = inHDTCheckByHostNameECR("MPU");		
			//vdDebug_PrintOnPaper("ECR MPU HDTid [%d]", srTransRec.HDTid);
		} else if ((memcmp(pSub->valuestring, "CUP", 3) == 0) || (memcmp(pSub->valuestring, "UPI", 3))) {
			//srTransRec.HDTid = inHDTCheckByHostName("CUP");
			srTransRec.HDTid = inHDTCheckByHostNameECR("CUP");
			if(!memcmp(ECRReq.txn_code, ECR_PREAUTHCOMP_TAG, 2))
			{
				srTransRec.HDTid = inHDTCheckByHostNameECR("UNIONPAY");
			}
			//vdDebug_PrintOnPaper("ECR CUP HDTid [%d]", srTransRec.HDTid);
		} else if(memcmp(pSub->valuestring, "JCB", 3) == 0){
			//srTransRec.HDTid = inHDTCheckByHostName("JCB");
			srTransRec.HDTid = inHDTCheckByHostNameECR("JCB");
			//vdDebug_PrintOnPaper("ECR JCB HDTid [%d]", srTransRec.HDTid);
		} else if (memcmp(pSub->valuestring, "EWALLET", 3) == 0) {
			strcpy(srTransRec.szHostLabel, "ALIPAY");
			srTransRec.HDTid = 10;
		} else if (memcmp(pSub->valuestring, "Alipay", 3) == 0) {
			strcpy(srTransRec.szHostLabel, "ALIPAY");
			srTransRec.HDTid = 10;
		} else if (memcmp(pSub->valuestring, "EWALLET", 3) == 0) {
			strcpy(srTransRec.szHostLabel, "ALIPAY");
			srTransRec.HDTid = 10;
		} else if (memcmp(pSub->valuestring, "WeChat", 3) == 0) {
			strcpy(srTransRec.szHostLabel, "WECHAT");
			srTransRec.HDTid = 12;
		} else if (memcmp(pSub->valuestring, "QQ", 3) == 0) {
			strcpy(srTransRec.szHostLabel, "QQWALLET");
			srTransRec.HDTid = 16;
		} else {
			strcpy(srTransRec.szHostLabel, "AMEX");
			srTransRec.HDTid = 2;
		}
	}

	ECRReq.HDTid=srTransRec.HDTid;
	vdDebug_LogPrintf("ECRReq.HDTid : (%d)\n", ECRReq.HDTid);
	
	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "TRACE");
	if (NULL == pSub) {
		//get object named "TRACE" faild
	} else {
		vdDebug_LogPrintf("TRACE : %s\n", pSub->valuestring);
		strcpy(ECRReq.inv_no, pSub->valuestring);
		vdDebug_LogPrintf("TRACE:%s", ECRReq.inv_no);
		chInvFlag = 1;
	}

	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "REFNUM");
	if (NULL == pSub) {
		//get object named "REFNUM" faild
	} else {
		vdDebug_LogPrintf("REFNUM : %s\n", pSub->valuestring);

		strcpy(ECRReq.rref, pSub->valuestring);
		vdDebug_LogPrintf("REFNUM:%s", ECRReq.rref);
		strcpy(srTransRec.szRRN, ECRReq.rref);
	}

	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "ORITID");
	if (NULL == pSub) {
		//get object named "ORITID" faild
	} else {
		vdDebug_LogPrintf("ORITID : %s\n", pSub->valuestring);
	}

	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "TENDER");
	if (NULL == pSub) {
		//get object named "TENDER" faild
	} else {
		srTransRec.byPrintType = pSub->valueint;
		//vdDebug_LogPrintf("TENDER : %s\nsrTransRec.byPrintType[%d]", pSub->valuestring, srTransRec.byPrintType);
	}

	// get string from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "RETRIEVAL");
	if (NULL == pSub) {
		//get object named "RETRIEVAL" faild
	} else {
		vdDebug_LogPrintf("RETRIEVAL : %s\n", pSub->valuestring);
	}

	// get number from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "TIPS");
	if (NULL == pSub) {
		//get TIPS from json faild
	} else {
		vdDebug_LogPrintf("TIPS : %012d\n", pSub->valueint);
		lgTipAmount = pSub->valueint;

		sprintf(ECRReq.amount, "%012d", pSub->valueint);
		wub_str_2_hex(ECRReq.amount, srTransRec.szTipAmount, sizeof (ECRReq.amount));
		strcpy(ECRReq.szTipAmount, ECRReq.amount);
	}

	vdDebug_LogPrintf("1.ECRReq.HDTid : (%d)\n", ECRReq.HDTid);

	// get number from json
	pSub = NULL;
	pSub = cJSON_GetObjectItem(pJson, "TOTALAMT");
	if (NULL == pSub) {
		//get TOTALAMT from json faild
	} else {
		vdDebug_LogPrintf("TOTALAMT : %012d\n", pSub->valueint);
		lgTotalAmount = pSub->valueint;

		sprintf(ECRReq.amount, "%012d", pSub->valueint);
		wub_str_2_hex(ECRReq.amount, srTransRec.szTotalAmount, sizeof (ECRReq.amount));

		if (lgTipAmount > 0) {
			lgBaseAmount = lgTotalAmount - lgTipAmount;
			sprintf(ECRReq.amount, "%012ld", lgBaseAmount);
			wub_str_2_hex(ECRReq.amount, srTransRec.szBaseAmount, sizeof (ECRReq.amount));
		}
	}

#ifdef ECR_PREAUTH_AND_COMP
	if(!memcmp(ECRReq.txn_code, ECR_PREAUTHCOMP_TAG, 2) || !memcmp(ECRReq.txn_code, ECR_VOID_PREAUTH_TAG, 2))
	{
		// get string from json
		pSub = NULL;
		pSub = cJSON_GetObjectItem(pJson, "APPCODE");
		if (NULL == pSub) 
		{
			//get object named "ORITID" faild
		} 
		else 
		{
			vdDebug_LogPrintf("APPCODE : %s\n", pSub->valuestring);
			strcpy(srTransRec.szAuthCode, pSub->valuestring);
			strcpy(ECRReq.szAuthCode, srTransRec.szAuthCode);
		}

		if((strlen(ECRReq.szAuthCode) == 0))
		{
			vdDebug_LogPrintf("APPCODE : EMPTY");
			return (VS_ERR);
		}

		if(strlen(ECRReq.rref) == 0)
		{
			vdDebug_LogPrintf("RRN : EMPTY");
			return (VS_ERR);
		}
		
		// get string from json
		pSub = NULL;
		pSub = cJSON_GetObjectItem(pJson, "MERCHANTID");
		if (NULL == pSub) 
		{
			//get object named "ORITID" faild
		} 
		else 
		{
			vdDebug_LogPrintf("MERCHANTID : %s\n", pSub->valuestring);
			strcpy(srTransRec.szMID, pSub->valuestring);
			strcpy(ECRReq.szMID, srTransRec.szMID);
		}
	}
#endif
	vdDebug_LogPrintf("2. ECRReq.HDTid : (%d)\n", ECRReq.HDTid);
	cJSON_Delete(pJson);

// patrick end parse JSON
	chECRFlag = 1;
	
	wub_str_2_hex(ECRReq.inv_no, srTransRec.szInvoiceNo, sizeof(ECRReq.inv_no));
	wub_str_2_hex(ECRReq.Pan, srTransRec.szPAN, sizeof(ECRReq.Pan));
	wub_str_2_hex(ECRReq.Expiry, srTransRec.szExpireDate, sizeof(ECRReq.Expiry));	
		vdDebug_LogPrintf("ECRReq.txn_code[%s] chAmtFlag[%d]",ECRReq.txn_code, chAmtFlag);

		// Start Transaction Process to Another Application, Set All Printing Flag to 0
//		  put_env_int("#P1FLAG", 0);
//		  put_env_int("#P2FLAG", 0);
//		  put_env_int("#P3FLAG", 0);
//		  put_env_int("#P4FLAG", 0);
		
		if(!memcmp(ECRReq.txn_code, ECR_ECHOTEST_TAG, 2))
	{	
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_ECHO);
			if(d_OK != inRet)
				return inRet;
		
			//inRet = inCTOSS_MultiAPGetData();
			return inRet;
	}
		
	if(!memcmp(ECRReq.txn_code, ECR_SALE_TAG, 2))
	{
		if(chAmtFlag)
		{	
			DebugAddHEX("ECR_SALE_TAG", srTransRec.szBaseAmount, 6);

			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_WAVE_SALE);
			vdDebug_LogPrintf("1.after inCTOSS_MultiAPSaveData");
			if(d_OK != inRet)
				return inRet;
			
			vdDebug_LogPrintf("2.after inCTOSS_MultiAPSaveData");
			
			//inRet = inCTOSS_MultiAPGetData();

			//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
			//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
			//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
			//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
			//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
			//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
			//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
	
//			  if(d_OK != inRet)
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_OFFLINE_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_OFFLINE_SALE);
			if(d_OK != inRet)
				return inRet;			

			//inRet = inCTOSS_MultiAPGetData();
//			if(d_OK != inRet)
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_REFUND_TAG, 2))
	{
			
		if(chAmtFlag)
		{
					vdDebug_LogPrintf("REFUND inCTOSS_MultiAPSaveData START");
					
//					  if (srTransRec.HDTid > 10)
						inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_WAVE_REFUND);
//					  else
//			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REFUND);
					
			if(d_OK != inRet)
				return inRet;			

			//inRet = inCTOSS_MultiAPGetData();
//			if(d_OK != inRet)
				return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_OCBC_IPP_TAG, 2))
	{
		if(chAmtFlag)
		{
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_INSTALLMENT);
			if(d_OK != inRet)
				return inRet;						

			//inRet = inCTOSS_MultiAPGetData();
			return inRet;
		}
	}

	if(!memcmp(ECRReq.txn_code, ECR_VOID_TAG, 2))
	{
		if(chInvFlag)
		{
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_VOID_SALE);
			if(d_OK != inRet)
				return inRet;									

			//inRet = inCTOSS_MultiAPGetData();
//			if(d_OK != inRet)
				return inRet;
		}
	}
	
	if(!memcmp(ECRReq.txn_code, ECR_TIP_ADJUST_TAG, 2))
	{
			vdDebug_LogPrintf("TIP ADJUST inCTOSS_MultiAPSaveData START");
			DebugAddHEX("ECR_Tip_Adjust Base Amount", srTransRec.szBaseAmount, 6);
			DebugAddHEX("ECR_Tip_Adjust Tip Amount", srTransRec.szTipAmount, 6);

//			inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_WAVE_SALE);
						inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_TIP_ADJUST);
			if(d_OK != inRet)
				return inRet;

			//inRet = inCTOSS_MultiAPGetData();

			//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
			//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.
			//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
			//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
			//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
			//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
			//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
	
			return inRet;
	}

	if(!memcmp(ECRReq.txn_code, ECR_INSTALLMENT_TAG, 2))
	{
			DebugAddHEX("ECR_INSTALLMENT_TAG", srTransRec.szBaseAmount, 6);

			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_INSTALLMENT);
			if(d_OK != inRet)
				return inRet;

			//inRet = inCTOSS_MultiAPGetData();

			//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
			//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
			//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
			//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
			//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
			//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
			//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
	
			return inRet;
	}			

	if(!memcmp(ECRReq.txn_code, ECR_RETRIEVAL_TAG, 2))
	{
//		DebugAddHEX("ECR_RETRIEVAL_TAG", srTransRec.szBaseAmount, 6);
//		DebugAddHEX("ECR_RETRIEVAL_TAG", srTransRec.szInvoiceNo, INVOICE_BCD_SIZE);

//		inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REPRINT_ANY); // d_IPC_CMD_TRX_RETRIEVAL
			
//			  if (memcmp(srTransRec.szInvoiceNo, "\x00\x00\x00", 3) != 0)
				inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_TRX_RETRIEVAL);
//			  else
//				  inRet = inCTOSS_MultiAPSaveData(d_IPC_CMD_REPRINT_LAST);
				
		if(d_OK != inRet)
			return inRet;

		//inRet = inCTOSS_MultiAPGetData();

		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

		return inRet;
	}

	if(!memcmp(ECRReq.txn_code, ECR_TOTAL_TAG, 2))
	{
		DebugAddHEX("ECR_TOTAL_TAG", srTransRec.szBaseAmount, 6);

		inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_BATCH_TOTAL);
		if(d_OK != inRet)
			return inRet;

		//inRet = inCTOSS_MultiAPGetData();

		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

		return inRet;
	}

	if(!memcmp(ECRReq.txn_code, ECR_SETTLE_ALL_TAG, 2))
	{
		DebugAddHEX("ECR_SETTLE_ALL_TAG", srTransRec.szBaseAmount, 6);

		inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_SETTLE_ALL);
		if(d_OK != inRet)
			return inRet;

		//inRet = inCTOSS_MultiAPGetData();

		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

		return inRet;
	}
	
	if(!memcmp(ECRReq.txn_code, ECR_SETTLEMENT_TAG, 2))
	{
		DebugAddHEX("ECR_SETTLEMENT_TAG", srTransRec.szBaseAmount, 6);

		if(get_env_int("#SETTLE_ALL") == 1)
		{
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_SETTLE_ALL);
			vdDebug_LogPrintf("SETTLE ALL");
		}
		else
		{
		   inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_SETTLEMENT);
		   vdDebug_LogPrintf("SETTLE ONE");
		}
		if(d_OK != inRet)
			return inRet;

		//inRet = inCTOSS_MultiAPGetData();

		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

		return inRet;
	}
		
		//To reprint any receipt
		if(!memcmp(ECRReq.txn_code, ECR_REPRINT_TAG, 2))
	{
				if(chInvFlag)
		{
					vdDebug_LogPrintf("Reprint ANY [%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
					vdDebug_LogPrintf("Length=[%d]",strlen(srTransRec.szInvoiceNo));
					inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_REPRINT_ANY);
					if(d_OK != inRet)
					{
						return inRet;
					}
		}
		 
				else
				{
					inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_REPRINT_LAST);
								  
				}
													
		//inRet = inCTOSS_MultiAPGetData();
				
				if(d_OK != inRet)
				{
					return inRet;
				}

		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

		return inRet;
	}
		
		//To reprint last settle
		if(!memcmp(ECRReq.txn_code, ECR_REPRINT_SETTLEMENT_TAG, 2))
	{
		vdDebug_PrintOnPaper("ECR_REPRINT_SETTLEMENT_TAG");
			inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_REPRINT_LAST_SETT);
			if(d_OK != inRet)
			{
				vdDebug_PrintOnPaper("MultiAPSave Data");
				return inRet;
				
			}
	
			//inRet = inCTOSS_MultiAPGetData();
			if(d_OK != inRet)
			{
				vdDebug_PrintOnPaper("MultiGet Data");
				return inRet;
			}
				
			//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
			//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
			//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
			//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
			//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
			//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
			//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
			
			return inRet;
	}

#ifdef ECR_PREAUTH_TAG
	if(!memcmp(ECRReq.txn_code, ECR_PREAUTH_TAG, 2))
	{
		vdDebug_LogPrintf("ECR_PREAUTH_TAG");
		vdDebug_LogPrintf("ECRReq.HDTid:%d", ECRReq.HDTid);
		
		if(ECRReq.HDTid != 7 && ECRReq.HDTid != 21)
		{
			setLCDPrint(5, 0, "TRANS NOT ALLOWED");
			vduiWarningSound();
			return BRANCH_EXIT;
		}
		
		
		inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_PRE_AUTH);
		if(d_OK != inRet)
		{
			vdDebug_LogPrintf("MultiAPSave Data");
			return inRet;
		
		}
		
		//inRet = inCTOSS_MultiAPGetData();
		if(d_OK != inRet)
		{
			vdDebug_LogPrintf("MultiGet Data");
			return inRet;
		}
		
		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
		
		return inRet;
	}
	
	if(!memcmp(ECRReq.txn_code, ECR_PREAUTHCOMP_TAG, 2))
	{
		vdDebug_LogPrintf("ECR_PREAUTHCOMP_TAG");
		vdDebug_LogPrintf("srTransRec.szAuthCode: %s\n", srTransRec.szAuthCode);
		vdDebug_LogPrintf("srTransRec.szMID: %s\n", srTransRec.szMID);
		
		vdDebug_LogPrintf("ECRReq.HDTid: %d\n", ECRReq.HDTid);
		
		if(ECRReq.HDTid != 7 && ECRReq.HDTid != 21)
		{
			setLCDPrint(5, 0, "TRANS NOT ALLOWED");
			vduiWarningSound();
			return BRANCH_EXIT;
		}
		
		inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_PRE_AUTH_COMP);
		if(d_OK != inRet)
		{
			vdDebug_LogPrintf("MultiAPSave Data");
			return inRet;
		}
		
		//inRet = inCTOSS_MultiAPGetData();
		if(d_OK != inRet)
		{
			vdDebug_LogPrintf("MultiGet Data");
			return inRet;
		}
		
		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
		
		return inRet;
	}

	if(!memcmp(ECRReq.txn_code, ECR_VOID_PREAUTH_TAG, 2))
	{
		vdDebug_PrintOnPaper("ECR_VOID_PREAUTH");
		vdDebug_LogPrintf("srTransRec.szAuthCode: %s\n", srTransRec.szAuthCode);
		vdDebug_LogPrintf("srTransRec.szMID: %s\n", srTransRec.szMID);
		
		vdDebug_LogPrintf("ECRReq.HDTid: %d\n", ECRReq.HDTid);
		
		if(ECRReq.HDTid != 7 && ECRReq.HDTid != 21)
		{
			setLCDPrint(5, 0, "TRANS NOT ALLOWED");
			vduiWarningSound();
			return BRANCH_EXIT;
		}
		
		inRet = inCTOSS_MultiAPSaveData_NOT_IPC(d_IPC_CMD_VOIDPREAUTH);
		if(d_OK != inRet)
		{
			vdDebug_PrintOnPaper("MultiAPSave Data");
			return inRet;
		}
		
		//inRet = inCTOSS_MultiAPGetData();
		if(d_OK != inRet)
		{
			vdDebug_PrintOnPaper("MultiGet Data");
			return inRet;
		}
		
		//vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
		//vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
		//vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
		//vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);			
		//vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
		//vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
		//vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);
		
		return inRet;
	}
#endif
	
	return BRANCH_EXIT;
}

int inECRSendAnalyse(void)
{
	int inRetval = d_NO;
	int iRetVal = 0;
	int currGrp = 0;
	unsigned char szTempBuff[50 + 1];

	vdDebug_LogPrintf("inECRSendAnalyse");

	vdDebug_LogPrintf("RecvPayment");

	strncpy(ECRResp.resp_code, srTransRec.szRespCode, ECR_RESP_CODE_SIZE);
    wub_hex_2_str(srTransRec.szInvoiceNo, ECRResp.inv_no, 3);
	strncpy(ECRResp.tid,srTransRec.szTID, TID_SIZE);
	strncpy(ECRResp.mid, srTransRec.szMID, MID_SIZE);
	strcpy(ECRResp.issuer, srTransRec.szCardLable);
    memset(ECRResp.merchant_name, 0x00, sizeof(ECRResp.merchant_name));
    strcpy(ECRResp.merchant_name, "MERCHANT");
	strcpy(ECRResp.card_no, srTransRec.szPAN);
    wub_hex_2_str(srTransRec.szExpireDate, ECRResp.exp_date, 2);
    wub_hex_2_str(srTransRec.szBatchNo, ECRResp.batch_no, 3);
    wub_hex_2_str(srTransRec.szTime, ECRResp.time, 3);
    wub_hex_2_str(srTransRec.szDate, ECRResp.date, 3);
	strncpy(ECRResp.rref, srTransRec.szRRN, RET_REF_SIZE);
    strncpy(ECRResp.auth_code, srTransRec.szAuthCode, AUTH_CODE_SIZE);
	strcpy(ECRResp.card_name, srTransRec.szCardholderName);

	vdDebug_LogPrintf("Resp PAN:%s",ECRResp.card_no);
	vdDebug_LogPrintf("Resp DE39:%s",ECRResp.resp_code);
	vdDebug_LogPrintf("Resp RREF:%s",ECRResp.rref);
    vdDebug_LogPrintf("Resp auth_code:%s",ECRResp.auth_code);
	vdDebug_LogPrintf("Resp ExpDate:%s",ECRResp.exp_date);
	vdDebug_LogPrintf("Resp MID:%s",ECRResp.tid);
	vdDebug_LogPrintf("Resp TID:%s",ECRResp.mid);
	vdDebug_LogPrintf("Inv Num:%s",ECRResp.inv_no);
    vdDebug_LogPrintf("issuer:%s",ECRResp.issuer);
    
    // Processing The Response Data
    
    // Masking PAN
    cardMasking(ECRResp.card_no, PRINT_CARD_MASKING_1);
	
	inECRSendResponse();
	return d_OK;
}


void vdSetLength(int inLength, char *out)
{
	char szTmp[5],szHex[5];
	
	sprintf((char *)szTmp, "%04d", inLength);
//	SVC_DSP_2_HEX((char *)szTmp ,(char *)szHex, 2);
	wub_str_2_hex((char *)szTmp ,(char *)szHex, 4);

	memcpy(out, szHex, 2);
}

#define SEND_LEN	100	//200

int inECRSendComPacket(char* pchMsg, int inMsgSize, VS_BOOL fPadStxEtx,int inACKTimeOut,VS_BOOL fWaitForAck,int inMaxRetries) {
    char chResp;
    int inSize = 0;
    int times = 0, i = 0;//inNumRetry = 0;
    char szECRSendBuf[4096 + 1];
	char szECRRecvBuf[4096 + 1];
    int inSendsize=0;	
    USHORT ret;
	ULONG tick;

    memset(szECRSendBuf, 0, (int)sizeof(szECRSendBuf));
    if (fPadStxEtx == VS_TRUE) {
    	
        szECRSendBuf[inSize] = STX; 
        inSize++;
        memcpy(&(szECRSendBuf[1]), pchMsg, inMsgSize); 
        inSize += inMsgSize;
        szECRSendBuf[inSize] = ETX; 
        inSize++;
    } else {
        memcpy(szECRSendBuf, pchMsg, inMsgSize); 
        inSize = inMsgSize;
    }

//    szECRSendBuf[inSize] = (char) SVC_CRC_CALC(0, &(szECRSendBuf[1]), (inSize - 1));
//    szECRSendBuf[inSize] = (char) ucGetLRC(&(szECRSendBuf[1]), (inSize - 1));
//    inSize++;
	
	vdDebug_LogPrintf("inECRSendComPacket strTCT.byRS232ECRPort=%d,ECRPort=%d", strTCT.byRS232ECRPort, ECRPort);
	if (1 ==strTCT.byRS232ECRPort || 2 ==strTCT.byRS232ECRPort)
	{
		vdDebug_LogPrintf("inECRSendComPacket strTCT.byRS232ECRPort=%d, ECRPort=%d", strTCT.byRS232ECRPort, ECRPort);
		if (ECRPort == d_COM1 || ECRPort == d_COM2)
		{
			vdDebug_LogPrintf("inECRSendComPacket ECRPort=%d", ECRPort);
			CTOS_RS232Close(ECRPort);
			CTOS_Delay(10);
			CTOS_RS232Open(ECRPort, 9600, 'N', 8, 1);
		}
	}

	do {
		if (ECRPort == d_ETHERNET || ECRPort == d_BLUETOOTH)
		{
			ret = d_OK;
			break;
		}

		if (ECRPort == 0xFF){
		  if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_USBTxReady();
		  
		  // FOR BASE USB CDC -- sidumili	
		  if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_BaseUSBTxReady();
		  
		  // FOR BASE USB CDC -- sidumili
		}
		//else if (ECRPort == d_BLUETOOTH)
		//{	
		//}
		else
		  ret = CTOS_RS232TxReady(ECRPort);
		if (ret == d_OK)
			break;
	}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);

	if (ret == d_OK) {
		if (ECRPort == 0xFF){
			if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				CTOS_USBTxData(szECRSendBuf, inSize);
			
			// FOR BASE USB CDC -- sidumili
			if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
				CTOS_BaseUSBTxData(szECRSendBuf, inSize);
			
			// FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
			TCP_ServerSend(szECRSendBuf,inSize,strTCT.inECRIPHeader);
		else if (ECRPort == d_BLUETOOTH)
			CTOSS_BluetoothTxData((char *) szECRSendBuf, inSize);
		else
		{
			//CTOS_RS232TxData(ECRPort, szECRSendBuf, inSize);
			
			//CTOS_Delay(20);

			DebugAddHEX("inECRSendComPacket COM port", szECRSendBuf, inSize);
			
			if (inSize > SEND_LEN)
			{
				times = inSize/SEND_LEN;
				for (i = 0;i<times;i++)
				{
					CTOS_RS232TxData(ECRPort, &szECRSendBuf[i*SEND_LEN], SEND_LEN);
					vdPCIDebug_HexPrintf("send",&szECRSendBuf[i*SEND_LEN],SEND_LEN);
				}

				times = inSize%SEND_LEN;
				if(times>0)
				{
					CTOS_RS232TxData(ECRPort, &szECRSendBuf[i*SEND_LEN], times);
					vdPCIDebug_HexPrintf("send",&szECRSendBuf[i*SEND_LEN],times);
				}
			}
			else
			{
				// Send data via COM1 port 
				if(CTOS_RS232TxData(ECRPort, szECRSendBuf, inSize) != d_OK) 
				{ 
					DebugAddHEX("inECRSendComPacket FAIL", szECRSendBuf, inSize);

					//vdDisplayErrorMsg(1, 8, "COM Send Buf Error");
					//Flushing the RS232 receive buffer // 
					CTOS_RS232FlushRxBuffer(ECRPort); 
					return d_NO; 
				}
			}
		}
		
		if (fWaitForAck == VS_TRUE) {
			 chResp = NAK;				/* Wait for reply till Timeout */

			 if (ECRPort == 0xFF){
			 	if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				 	CTOS_USBRxData(&chResp, &lECRRecvLen);
			 	
				// FOR BASE USB CDC -- sidumili
				if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
					 CTOS_BaseUSBRxData(&chResp, &lECRRecvLen);
				
				// FOR BASE USB CDC -- sidumili
			 }
			 else if (ECRPort == d_ETHERNET)
			 	 TCP_ServerRecv(&chResp,4096,&lECRRecvLen,strTCT.inECRIPHeader,60);
			 else if (ECRPort == d_BLUETOOTH)
			 {
			 	tick = CTOS_TickGet();
			 	while (1)
			 	{
			 	memset(szECRRecvBuf,0x00,sizeof(szECRRecvBuf));
				 CTOSS_BluetoothRxData(&szECRRecvBuf, &lECRRecvLen);
				 vdDebug_LogPrintf("CTOSS_BluetoothRxData,lECRRecvLen=[%d]", lECRRecvLen);
				 if (lECRRecvLen > 0)
				 {
				 	chResp = szECRRecvBuf[0];
				 	break;
				 }
				 if (CTOS_TickGet() - tick > d_READY_TIMEOUT)
				 {
				 	vdDebug_LogPrintf("timeout....");
				 	break;
				 }
			 	}
			 }
			 else
				 CTOS_RS232RxData(ECRPort, &chResp, &lECRRecvLen);
		 } else
			 chResp = ACK;
	}

	//inPrintECRPacket("ECR Send Response", szECRSendBuf, inSize);
	vdDebug_LogPrintf("chResp=[%d][%c][%x]", chResp, chResp, chResp);
	vdDebug_LogPrintf("inSize[%d]", inSize);
	vdPCIDebug_HexPrintf("ECR Send Response", szECRSendBuf, inSize);
    vdDebug_LogPrintf("szECRSendBuf:%s", szECRSendBuf);
    if (chResp == ACK)
        return (VS_SUCCESS);
    else
        return (ERR_COM_NAK);  /* Too many NAKs, so indicate to the app */   	
}



int inECRSendComPacketEx(char* pchMsg, int inMsgSize, VS_BOOL fPadStxEtx,int inACKTimeOut,VS_BOOL fWaitForAck,int inMaxRetries) {
    char chResp;
    int inSize = 0;
    int times = 0, i = 0;//inNumRetry = 0;
    char szECRSendBuf[4096 + 1];
	char szECRRecvBuf[4096 + 1];
    int inSendsize=0;	
    USHORT ret;
	ULONG tick;

    memset(szECRSendBuf, 0, (int)sizeof(szECRSendBuf));
    if (fPadStxEtx == VS_TRUE) {
    	
        szECRSendBuf[inSize] = STX; 
        inSize++;
        memcpy(&(szECRSendBuf[1]), pchMsg, inMsgSize); 
        inSize += inMsgSize;
        szECRSendBuf[inSize] = ETX; 
        inSize++;
    } else {
        memcpy(szECRSendBuf, pchMsg, inMsgSize); 
        inSize = inMsgSize;
    }

//    szECRSendBuf[inSize] = (char) SVC_CRC_CALC(0, &(szECRSendBuf[1]), (inSize - 1));
//    szECRSendBuf[inSize] = (char) ucGetLRC(&(szECRSendBuf[1]), (inSize - 1));
//    inSize++;
	
	vdDebug_LogPrintf("inECRSendComPacket strTCT.byRS232ECRPort=%d,ECRPort=%d", strTCT.byRS232ECRPort, ECRPort);
	if (1 ==strTCT.byRS232ECRPort || 2 ==strTCT.byRS232ECRPort)
	{
		vdDebug_LogPrintf("inECRSendComPacket strTCT.byRS232ECRPort=%d, ECRPort=%d", strTCT.byRS232ECRPort, ECRPort);
		if (ECRPort == d_COM1 || ECRPort == d_COM2)
		{
			vdDebug_LogPrintf("inECRSendComPacket ECRPort=%d", ECRPort);
			CTOS_RS232Close(ECRPort);
			CTOS_Delay(10);
			CTOS_RS232Open(ECRPort, 9600, 'N', 8, 1);
		}
	}

	do {
		if (ECRPort == d_ETHERNET || ECRPort == d_BLUETOOTH)
		{
			ret = d_OK;
			break;
		}

		if (ECRPort == 0xFF){
		  if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_USBTxReady();
		  
		  // FOR BASE USB CDC -- sidumili	
		  if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
		  	ret = CTOS_BaseUSBTxReady();
		  
		  // FOR BASE USB CDC -- sidumili
		}
		//else if (ECRPort == d_BLUETOOTH)
		//{	
		//}
		else
		  ret = CTOS_RS232TxReady(ECRPort);
		if (ret == d_OK)
			break;
	}while ((CTOS_TickGet() - tick) < d_READY_TIMEOUT);

	if (ret == d_OK) {
		if (ECRPort == 0xFF){
			if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				CTOS_USBTxData(szECRSendBuf, inSize);
			
			// FOR BASE USB CDC -- sidumili
			if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
				CTOS_BaseUSBTxData(szECRSendBuf, inSize);
			
			// FOR BASE USB CDC -- sidumili
		}
		else if (ECRPort == d_ETHERNET)
			TCP_ServerSend(szECRSendBuf,inSize,strTCT.inECRIPHeader);
		else if (ECRPort == d_BLUETOOTH)
			CTOSS_BluetoothTxData((char *) szECRSendBuf, inSize);
		else
		{
			//CTOS_RS232TxData(ECRPort, szECRSendBuf, inSize);
			
			//CTOS_Delay(20);

			DebugAddHEX("inECRSendComPacket COM port", szECRSendBuf, inSize);
			
			if (inSize > SEND_LEN)
			{
				times = inSize/SEND_LEN;
				for (i = 0;i<times;i++)
				{
					CTOS_RS232TxData(ECRPort, &szECRSendBuf[i*SEND_LEN], SEND_LEN);
					vdPCIDebug_HexPrintf("send",&szECRSendBuf[i*SEND_LEN],SEND_LEN);
				}

				times = inSize%SEND_LEN;
				if(times>0)
				{
					CTOS_RS232TxData(ECRPort, &szECRSendBuf[i*SEND_LEN], times);
					vdPCIDebug_HexPrintf("send",&szECRSendBuf[i*SEND_LEN],times);
				}
			}
			else
			{
				// Send data via COM1 port 
				if(CTOS_RS232TxData(ECRPort, szECRSendBuf, inSize) != d_OK) 
				{ 
					DebugAddHEX("inECRSendComPacket FAIL", szECRSendBuf, inSize);

					//vdDisplayErrorMsg(1, 8, "COM Send Buf Error");
					//Flushing the RS232 receive buffer // 
					CTOS_RS232FlushRxBuffer(ECRPort); 
					return d_NO; 
				}
			}
		}
		
		if (fWaitForAck == VS_TRUE) {
			 chResp = NAK;				/* Wait for reply till Timeout */

			 if (ECRPort == 0xFF){
			 	if (ECR_USB_TERMINAL_PORT == strTCT.byRS232ECRPort)
				 	CTOS_USBRxData(&chResp, &lECRRecvLen);
			 	
				// FOR BASE USB CDC -- sidumili
				if (ECR_USB_BASE_PORT == strTCT.byRS232ECRPort)
					 CTOS_BaseUSBRxData(&chResp, &lECRRecvLen);
				
				// FOR BASE USB CDC -- sidumili
			 }
			 else if (ECRPort == d_ETHERNET)
			 	 TCP_ServerRecv(&chResp,4096,&lECRRecvLen,strTCT.inECRIPHeader,60);
			 else if (ECRPort == d_BLUETOOTH)
			 {
			 	tick = CTOS_TickGet();
			 	while (1)
			 	{
			 	memset(szECRRecvBuf,0x00,sizeof(szECRRecvBuf));
				 CTOSS_BluetoothRxData(&szECRRecvBuf, &lECRRecvLen);
				 vdDebug_LogPrintf("CTOSS_BluetoothRxData,lECRRecvLen=[%d]", lECRRecvLen);
				 if (lECRRecvLen > 0)
				 {
				 	chResp = szECRRecvBuf[0];
				 	break;
				 }
				 if (CTOS_TickGet() - tick > d_READY_TIMEOUT)
				 {
				 	vdDebug_LogPrintf("timeout....");
				 	break;
				 }
			 	}
			 }
			 else
				 CTOS_RS232RxData(ECRPort, &chResp, &lECRRecvLen);
		 } else
			 chResp = ACK;
	}

	//inPrintECRPacket("ECR Send Response", szECRSendBuf, inSize);
	vdDebug_LogPrintf("chResp=[%d][%c][%x]", chResp, chResp, chResp);
	vdDebug_LogPrintf("inSize[%d]", inSize);
	vdPCIDebug_HexPrintf("ECR Send Response", szECRSendBuf, inSize);

    if (chResp == ACK)
        return (VS_SUCCESS);
    else
        return (ERR_COM_NAK);  /* Too many NAKs, so indicate to the app */   	
}

//extern int ClientSocket;

#if 1
int inECRSendResponse(void)
{
#define CARD_ENTRY_MSR                  1
#define CARD_ENTRY_MANUAL               2
#define CARD_ENTRY_ICC                  3
#define CARD_ENTRY_FALLBACK             4
#define CARD_ENTRY_WAVE                 5
#define CARD_ENTRY_EASY_ICC         	6

    int inSaleCnt, inSaleCUPCnt, inRefundCnt, inCashAdvCnt;
	int offset, inSize;
	int inSerialPort=0;
        int inPrintingFlag=0;
        int inPrintDataLen=0;
    CTOS_RTC SetRTC;

	cJSON * pJsonRoot = NULL;
	unsigned char uszJsonString[MAX_DATA_LEN+1];
	unsigned char uszECRReceiveData[1024+1];
	unsigned char uszMD5Data[1024+1];

	unsigned char szDate[12 + 1];
	unsigned char szTime[6 + 1];
 
	char * p = NULL;	
	char * c = NULL;

	BYTE   EMVtagVal[64];
	USHORT EMVtagLen; 
	char szTemp[44 + 1];
        BYTE szPrintData[40 + 1];
        BYTE szAmountData[40 + 1];
        char szTemp2[4+1];
	char szCardType[20+1];
	char szCBBSettleData1[54];
	char szMPUSettleData1[54];
	char szUPISettleData1[54];
	char szMPUPISettleData1[54];
	char szJCBSettleData1[54];
	char szMMKSettleData1[54];
	char szUSDSettleData1[54];

	int inCBBSettled=0, inMPUSettled=0, inUPISettled=0, inMPUPISettled=0, inJCBSettled=0;
	cJSON * pJsonCBB = NULL;
	cJSON * pJsonMPU = NULL;
	cJSON * pJsonUPI = NULL;
	cJSON * pJsonMPUPI = NULL;
	cJSON * pJsonJCB = NULL;
	cJSON * pJsonMMK = NULL;
	cJSON * pJsonUSD = NULL;

	char sztempdata1 [8+1];
	char szTempdata2 [3+1];
	char szTempdata3 [12+1];
	char saleamt [12+1];
	char voidamt [12+1];
	char refundamt [12+1];
	int salecnt=0, voidcnt=0, refundcnt=0;

	
	
	vdDebug_LogPrintf("ECRResp.req_resp[%s] ECRResp.auth_code[%s]", ECRResp.resp_code, ECRResp.auth_code);

	 p = NULL;
 
 // pack ECR JSON response data
 
	 pJsonRoot = NULL;
	 pJsonRoot = cJSON_CreateObject();
	 if(NULL == pJsonRoot)
	 {
		 //error happend here
		 return 0;
	 }

	if(strcmp(ECRReq.szCMD, "SETTLEMENT") != 0 && strcmp(ECRReq.szCMD, "TOTAL") != 0) {
		 if ((!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE)) || (!memcmp(ECRResp.auth_code,"Y1",ECR_RESP_CODE_SIZE)))	 	
			 cJSON_AddStringToObject(pJsonRoot, "STATUS", "Approved");
		 else
			 cJSON_AddStringToObject(pJsonRoot, "STATUS", "Declined");
	}
         
         if(memcmp(ECRReq.txn_code, ECR_ECHOTEST_TAG, 2) == 0){
//             cJSON_AddStringToObject(pJsonRoot, "CMD", "ECHOTEST");
             BYTE tmpbuf[16 + 1];
             getTerminalSerialNo(tmpbuf);
             cJSON_AddStringToObject(pJsonRoot, "CMD", ECRReq.szCMD);
             cJSON_AddStringToObject(pJsonRoot, "SERIAL", tmpbuf);
             goto lblDONE;
         }

         // Non Settlement & Total Transaction
	 if ((strcmp(ECRReq.szCMD, "TOTAL") != 0) && (strcmp(ECRReq.szCMD, "SETTLEMENT") != 0))
	 {
             vdDebug_LogPrintf("Non Settlement Start");
		cJSON_AddStringToObject(pJsonRoot, "RESP", ECRResp.resp_code);

		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
		{
			//sprintf(szTemp, "%06ld", srTransRec.ulTraceNum);	 
			//vdDebug_LogPrintf("TRACE[%c]", szTemp);
			cJSON_AddStringToObject(pJsonRoot, "TRACE", ECRResp.inv_no);//cJSON_AddStringToObject(pJsonRoot, "TRACE", szTemp);
		}
		else
			cJSON_AddStringToObject(pJsonRoot, "TRACE", "");

		//Read the date and the time //
		CTOS_RTCGet(&SetRTC);	

		//memset(szDate 0x00, sizeof(szDate));
		//memset(szTime 0x00, sizeof(szTime));
		sprintf(szDate, "%02d-%02d-20%02d", (int)SetRTC.bDay, (int)SetRTC.bMonth, (int)SetRTC.bYear);
		sprintf(szTime, "%02d:%02d", SetRTC.bHour, SetRTC.bMinute);

		cJSON_AddStringToObject(pJsonRoot, "DATE", szDate);
		cJSON_AddStringToObject(pJsonRoot, "TIME", szTime);
		cJSON_AddStringToObject(pJsonRoot, "APPCODE", srTransRec.szAuthCode);
		cJSON_AddStringToObject(pJsonRoot, "PAN", ECRResp.card_no);
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
			cJSON_AddStringToObject(pJsonRoot, "EXPDATE", ECRResp.exp_date);
		else
			cJSON_AddStringToObject(pJsonRoot, "EXPDATE", "");	 

		wub_hex_2_str(srTransRec.szTotalAmount, szTemp, 6);
		cJSON_AddNumberToObject(pJsonRoot, "AMT", atol(szTemp));

		wub_hex_2_str(srTransRec.szTipAmount, szTemp, 6);
		cJSON_AddNumberToObject(pJsonRoot, "TIPS", atol(szTemp));

		vdDebug_LogPrintf("ENTRYMODE[%ld]", srTransRec.byEntryMode);
		if (srTransRec.byEntryMode == CARD_ENTRY_MSR)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "S");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_MANUAL)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "M");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_ICC)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "C");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_FALLBACK)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "F");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_WAVE)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "W");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_EASY_ICC)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "E");	
		}
		 
		cJSON_AddStringToObject(pJsonRoot, "TERMINALID", srTransRec.szTID);
		cJSON_AddStringToObject(pJsonRoot, "MERCHANTID", srTransRec.szMID);
		cJSON_AddStringToObject(pJsonRoot, "CARDHOLDERNAME", ECRResp.card_name);

		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
			cJSON_AddStringToObject(pJsonRoot, "BATCHNO", ECRResp.batch_no);
		else
			cJSON_AddStringToObject(pJsonRoot, "BATCHNO", "");
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
		{
			if (strcmp(ECRReq.szTYPE, "EWALLET") == 0)
				cJSON_AddStringToObject(pJsonRoot, "REFNUM", srTransRec.szCardLable);
			else
				cJSON_AddStringToObject(pJsonRoot, "REFNUM", srTransRec.szRRN);//cJSON_AddStringToObject(pJsonRoot, "REFNUM", ECRResp.inv_no);
		}
		else
			cJSON_AddStringToObject(pJsonRoot, "REFNUM", "");

		cJSON_AddStringToObject(pJsonRoot, "ECRREF", ECRReq.szECRREF);
		if (strcmp(ECRReq.szTYPE, "EWALLET") == 0)
			cJSON_AddStringToObject(pJsonRoot, "APP", ECRResp.issuer);
		else
			cJSON_AddStringToObject(pJsonRoot, "APP", srTransRec.szCardLable);

		memset(EMVtagVal, 0x00, sizeof(EMVtagVal));
		EMVtagLen = srTransRec.stEMVinfo.T84_len;
		memcpy(EMVtagVal, srTransRec.stEMVinfo.T84, EMVtagLen);
		wub_hex_2_str(EMVtagVal, szTemp, EMVtagLen);
		vdDebug_LogPrintf("AID[%s]", ECRResp.resp_code);

		cJSON_AddStringToObject(pJsonRoot, "AID", szTemp);

		wub_hex_2_str(srTransRec.stEMVinfo.T9F26, szTemp, 8);
		vdDebug_LogPrintf("TC[%s]", szTemp);
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	  
			cJSON_AddStringToObject(pJsonRoot, "TC", szTemp);
		else	
			cJSON_AddStringToObject(pJsonRoot, "TC", "");
		cJSON_AddStringToObject(pJsonRoot, "CURRCODE", srTransRec.stEMVinfo.T9F29);
		cJSON_AddStringToObject(pJsonRoot, "FXRATE", ECRResp.DCC_ConvRate);
		cJSON_AddNumberToObject(pJsonRoot, "FOREIGNAMT", atol(ECRResp.DCC_ConvAmt));
		cJSON_AddStringToObject(pJsonRoot, "DCCMSG", "");

		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%02d", srTransRec.byPrintType);	//epp term
		cJSON_AddStringToObject(pJsonRoot, "TENDER", szTemp);//cJSON_AddStringToObject(pJsonRoot, "TENDER", "");	
		
		memset(szCardType, 0, sizeof(szCardType));
		if(srTransRec.HDTid == 17 || srTransRec.HDTid == 18 || srTransRec.HDTid == 19)
		{
			if(srTransRec.HDTid == 18)
				strcpy(szCardType, "JCB");
			else if(srTransRec.HDTid == 19)
			{
				strcpy(szCardType, "MPUPI");
			}
			else
			    strcpy(szCardType, "MPU");
		}
		else if(srTransRec.HDTid == 7)
			strcpy(szCardType, "UPI");
		else
		{
			if(srTransRec.IITid == 1)
				strcpy(szCardType, "VISA");
			else if(srTransRec.IITid == 2)
				strcpy(szCardType, "MASTERCARD");
		}
		
		cJSON_AddStringToObject(pJsonRoot, "CARDTYPE", szCardType);//cJSON_AddStringToObject(pJsonRoot, "TENDER", "");
	}
	else // Settlement & Total Transaction
 	{ 	
             // Retrieve Needed Data
             // SALE
             inSaleCnt =  get_env_int("#STMTSALECNT");
             vdDebug_LogPrintf("#STMTSALECNT =[%d]",inSaleCnt);
             
             // SALE CUP
             inSaleCUPCnt = get_env_int("#STMTSALECUPCNT");
             vdDebug_LogPrintf("#STMTSALECUPCNT =[%d]",inSaleCUPCnt);
             
             // Refund
             inRefundCnt = get_env_int("#STMTREFUNDCNT");
             vdDebug_LogPrintf("#STMTREFUNDCNT =[%d]",inRefundCnt);
             
             // Cash Advance
             inCashAdvCnt = get_env_int("#STMTCASHADVCNT");
             vdDebug_LogPrintf("#STMTCASHADVCNT =[%d]",inCashAdvCnt);
             
             vdDebug_LogPrintf("Settlement Start");
 		if ((strcmp(ECRReq.szCMD, "SETTLEMENT") == 0) && ((strcmp(ECRReq.szTYPE, "Alipay") == 0) || (strcmp(ECRReq.szTYPE, "WeChat") == 0) || (strcmp(ECRReq.szTYPE, "QQ") == 0)))
		{
			// for Alipay, WeChat, QQ, all amount are in srTransRec.szTrack3Data because srTransRec.szTrack1Data and srTransRec.szTrack2Data are in use
			cJSON_AddNumberToObject(pJsonRoot, "SALECNT", inSaleCnt);
			if (srTransRec.usTrack1Len == 0)
			{
				memset(srTransRec.baChipData,0x00,sizeof(srTransRec.baChipData));
				srTransRec.baChipData[0] = '0';
			}
//			cJSON_AddStringToObject(pJsonRoot, "SALEAMT", srTransRec.baChipData);
                        
                        memset(szAmountData, 0x00, sizeof(szAmountData));
                        get_env("#STMTSALEAMT", szAmountData, 40);
                        cJSON_AddStringToObject(pJsonRoot, "SALEAMT", szAmountData);
			
			cJSON_AddNumberToObject(pJsonRoot, "VOIDCNT", srTransRec.usTrack2Len);
			if (srTransRec.usTrack2Len == 0)
			{
				memset(srTransRec.baAdditionalData,0x00,sizeof(srTransRec.baAdditionalData));
				srTransRec.baAdditionalData[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "VOIDAMT", srTransRec.baAdditionalData);
			
//			cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", srTransRec.usTrack3Len);
                        cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", inRefundCnt);
			if (srTransRec.usTrack3Len == 0)
			{
				memset(srTransRec.szTrack3Data,0x00,sizeof(srTransRec.szTrack3Data));
				srTransRec.szTrack3Data[0] = '0';
			}
//			cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
                        
                        memset(szAmountData, 0x00, sizeof(szAmountData));
                        get_env("#STMTREFUNDAMT", szAmountData, 40);
                        cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
		}
		else
		{  
#if 1
			int inCBBSettled=0, inMPUSettled=0, inUPISettled=0, inMPUPISettled=0, inJCBSettled=0, inFinUsdSettled=0, inFinMMKSettled = 0;

            inCBBSettled=get_env_int("CBB_SETTLED");
			//inCBBSettled=1;
			if(inCBBSettled == 1)
			{
				#if 0
				cJSON* jarr = cJSON_CreateArray();
				
                /** add item to an obj */
                //cJSON_AddItemToObject(pJsonRoot, "arrName", jarr);
                
                /** add string to an obj */
                cJSON_AddStringToObject(pJsonRoot, "strName", "string");
                
                /** add number to an obj */
                cJSON_AddNumberToObject(pJsonRoot, "numName", 42);

				/** add obj to an array */
				cJSON_AddItemToArray(jarr, pJsonRoot);
				#else
				//tine - trial codes
				put_env_int("CBB_SETTLED", 0);
				memset(szCBBSettleData1, 0x00, sizeof(szCBBSettleData1));
				get_env("S_CBB", szCBBSettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szCBBSettleData1);
				
				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szCBBSettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szCBBSettleData1[0], 3);		//total sale count
				
				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szCBBSettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szCBBSettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szCBBSettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szCBBSettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szCBBSettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);
				
				pJsonCBB = NULL;
				pJsonCBB = cJSON_CreateObject();
				
				cJSON_AddStringToObject(pJsonCBB, "STATUS", sztempdata1);			
				cJSON_AddNumberToObject(pJsonCBB, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonCBB, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonCBB, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonCBB, "VOIDAMT", voidamt);				
				cJSON_AddNumberToObject(pJsonCBB, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonCBB, "REFUNDAMT", refundamt);
                vdDebug_LogPrintf("salecnt =[%d]，saleamt=[%s],voidcnt[%d],voidamt=%s ",salecnt, saleamt, voidcnt, voidamt);
				cJSON_AddItemToObject(pJsonRoot, "CBB", pJsonCBB);

				put_env("S_CBB", " ", 1);
			
				#endif
				
				#if 0
				 cJSON *resolutions = NULL;
				 cJSON *resolution = NULL;
				 
				 resolutions = cJSON_CreateArray();
                 if (resolutions == NULL)
                 {
				 	vdDebug_LogPrintf("resolutions == NULL");
                     //goto end;
                 }
				 else
				 {
				 	vdDebug_LogPrintf("resolutions == not NULL");
				 //cJSON_AddStringToObject(pJsonRoot, "CBB", ""STATUS"":\"Declined\",\"SALECNT\":2,\"SALEAMT\":\"2.00\",\"VOIDCNT\":0,\"VOIDAMT\":\"0\",\"REFUNDCNT\":0");
				 cJSON_AddItemToObject(pJsonRoot, "CBB", resolutions);
				 }
				 
				 resolution = cJSON_CreateObject();
				 if (resolution == NULL)
				 {
					 //goto end;
				 }
                 cJSON_AddItemToArray(resolutions, resolution);
				 
                 cJSON_AddItemToObject(resolution, "width", "1");
				 #endif
			}

            inMPUSettled=get_env_int("MPU_SETTLED");
			if(inMPUSettled == 1)
			{
				put_env_int("MPU_SETTLED", 0);
				memset(szMPUSettleData1, 0x00, sizeof(szMPUSettleData1));
				get_env("S_MPU", szMPUSettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szMPUSettleData1);
				
				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szMPUSettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUSettleData1[0], 3);		//total sale count
				
				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUSettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUSettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUSettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUSettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUSettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);
				
				pJsonMPU = NULL;
				pJsonMPU = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonMPU, "STATUS", sztempdata1);				
				cJSON_AddNumberToObject(pJsonMPU, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonMPU, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonMPU, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonMPU, "VOIDAMT", voidamt);				
				cJSON_AddNumberToObject(pJsonMPU, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonMPU, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "MPU", pJsonMPU);

				put_env("S_MPU", " ", 1);

			}
			
            inUPISettled=get_env_int("UPI_SETTLED");
			if(inUPISettled == 1)
			{
				put_env_int("UPI_SETTLED", 0);
				memset(szUPISettleData1, 0x00, sizeof(szUPISettleData1));
				get_env("S_UPI", szUPISettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szUPISettleData1);
				
				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szUPISettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUPISettleData1[0], 3);		//total sale count
				
				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUPISettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUPISettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUPISettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUPISettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUPISettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);
				
				pJsonUPI = NULL;
				pJsonUPI = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonUPI, "STATUS", sztempdata1);				
				cJSON_AddNumberToObject(pJsonUPI, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonUPI, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonUPI, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonUPI, "VOIDAMT", voidamt);				
				cJSON_AddNumberToObject(pJsonUPI, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonUPI, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "UPI", pJsonUPI);

				put_env("S_UPI", " ", 1);
			}

            inMPUPISettled=get_env_int("MPUPI_SETTLED");
            if(inMPUPISettled == 1)
            {
				put_env_int("MPUPI_SETTLED", 0);
				memset(szMPUPISettleData1, 0x00, sizeof(szMPUPISettleData1));
				get_env("S_MPUPI", szMPUPISettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szMPUPISettleData1);
				
				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szMPUPISettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUPISettleData1[0], 3);		//total sale count
				
				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUPISettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUPISettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUPISettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMPUPISettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMPUPISettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);
				
				pJsonMPUPI = NULL;
				pJsonMPUPI = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonMPUPI, "STATUS", sztempdata1);				
				cJSON_AddNumberToObject(pJsonMPUPI, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonMPUPI, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonMPUPI, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonMPUPI, "VOIDAMT", voidamt);				
				cJSON_AddNumberToObject(pJsonMPUPI, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonMPUPI, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "MPUPI", pJsonMPUPI);

				put_env("S_MPUPI", " ", 1);
            }
            
            inJCBSettled=get_env_int("JCB_SETTLED");
            if(inJCBSettled == 1)
            {
				put_env_int("JCB_SETTLED", 0);
				memset(szJCBSettleData1, 0x00, sizeof(szJCBSettleData1));
				get_env("S_JCB", szJCBSettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szJCBSettleData1);
				
				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szJCBSettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szJCBSettleData1[0], 3);		//total sale count
				
				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szJCBSettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szJCBSettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szJCBSettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szJCBSettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szJCBSettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);
				
				pJsonJCB = NULL;
				pJsonJCB = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonJCB, "STATUS", sztempdata1);				
				cJSON_AddNumberToObject(pJsonJCB, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonJCB, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonJCB, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonJCB, "VOIDAMT", voidamt);				
				cJSON_AddNumberToObject(pJsonJCB, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonJCB, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "JCB", pJsonJCB);

				put_env("S_JCB", " ", 1);
            }

			inFinMMKSettled=get_env_int("MMK_SETTLED");
			vdDebug_LogPrintf("inFinMMKSettled =[%d]",inFinMMKSettled);
			if(inFinMMKSettled == 1)
			{
				put_env_int("MMK_SETTLED", 0);
				memset(szMMKSettleData1, 0x00, sizeof(szMMKSettleData1));
				get_env("S_MMK", szMMKSettleData1, 54);
				vdDebug_LogPrintf("szMMKSettleData1[%s]", szMMKSettleData1);
				//vdDebug_PrintOnPaper("[%s]", szJCBSettleData1);

				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szMMKSettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMMKSettleData1[0], 3);		//total sale count

				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);
				vdDebug_LogPrintf("szTempdata2[%s],salecnt=%d", szTempdata2, salecnt);
				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMMKSettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);
				vdDebug_LogPrintf("szTempdata3[%s],[%d], saleamt=%s", szTempdata3, atoi(szTempdata3), saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMMKSettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMMKSettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szMMKSettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szMMKSettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);

				pJsonMMK = NULL;
				pJsonMMK = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonMMK, "STATUS", sztempdata1);
				cJSON_AddNumberToObject(pJsonMMK, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonMMK, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonMMK, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonMMK, "VOIDAMT", voidamt);
				cJSON_AddNumberToObject(pJsonMMK, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonMMK, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "MMK", pJsonMMK);

				put_env("S_MMK", " ", 1);
			}
			inFinUsdSettled=get_env_int("USD_SETTLED");
			if(inFinUsdSettled == 1)
			{
				put_env_int("USD_SETTLED", 0);
				memset(szUSDSettleData1, 0x00, sizeof(szUSDSettleData1));
				get_env("S_USD", szUSDSettleData1, 54);
				//vdDebug_PrintOnPaper("[%s]", szUSDSettleData1);

				memset(sztempdata1, 0x00, sizeof(sztempdata1));
				memcpy(sztempdata1, &szUSDSettleData1[45], 8);		//status
				//vdDebug_PrintOnPaper("[%s]", sztempdata1);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUSDSettleData1[0], 3);		//total sale count

				salecnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", salecnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUSDSettleData1[3], 12);	//total sale amount
				memset(saleamt, 0x00, sizeof(saleamt));
				sprintf(saleamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", saleamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUSDSettleData1[15], 3);		//total void count
				voidcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", voidcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUSDSettleData1[18], 12);		//total void amount
				memset(voidamt, 0x00, sizeof(voidamt));
				sprintf(voidamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", voidamt);

				memset(szTempdata2, 0x00, sizeof(szTempdata2));
				memcpy(szTempdata2, &szUSDSettleData1[30], 3);		//total refund count
				refundcnt = atoi(szTempdata2);
				//vdDebug_PrintOnPaper("[%d]", refundcnt);

				memset(szTempdata3, 0x00, sizeof(szTempdata3));
				memcpy(szTempdata3, &szUSDSettleData1[33], 12);		//total refund amount
				//vdDebug_PrintOnPaper("[%s]", szTempdata3);
				memset(refundamt, 0x00, sizeof(refundamt));
				sprintf(refundamt, "%d", atoi(szTempdata3));
				//vdDebug_PrintOnPaper("[%s]", refundamt);

				pJsonUSD = NULL;
				pJsonUSD = cJSON_CreateObject();
				cJSON_AddStringToObject(pJsonUSD, "STATUS", sztempdata1);
				cJSON_AddNumberToObject(pJsonUSD, "SALECNT", salecnt);
				cJSON_AddStringToObject(pJsonUSD, "SALEAMT", saleamt);
				cJSON_AddNumberToObject(pJsonUSD, "VOIDCNT", voidcnt);
				cJSON_AddStringToObject(pJsonUSD, "VOIDAMT", voidamt);
				cJSON_AddNumberToObject(pJsonUSD, "REFUNDCNT", refundcnt);
				cJSON_AddStringToObject(pJsonUSD, "REFUNDAMT", refundamt);

				cJSON_AddItemToObject(pJsonRoot, "USD", pJsonUSD);

				put_env("S_USD", " ", 1);
			}
#else
			cJSON_AddNumberToObject(pJsonRoot, "SALECNT", inSaleCnt);
			if (srTransRec.usTrack1Len == 0)
			{
				memset(srTransRec.szTrack1Data,0x00,sizeof(srTransRec.szTrack1Data));
				srTransRec.szTrack1Data[0] = '0';
			}
//			cJSON_AddStringToObject(pJsonRoot, "SALEAMT", srTransRec.szTrack1Data);
                        
                        memset(szAmountData, 0x00, sizeof(szAmountData));
                        get_env("#STMTSALEAMT", szAmountData, 40);
                        cJSON_AddStringToObject(pJsonRoot, "SALEAMT", szAmountData);
			
			cJSON_AddNumberToObject(pJsonRoot, "VOIDCNT", srTransRec.usTrack2Len);
			if (srTransRec.usTrack2Len == 0)
			{
				memset(srTransRec.szTrack2Data,0x00,sizeof(srTransRec.szTrack2Data));
				srTransRec.szTrack2Data[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "VOIDAMT", srTransRec.szTrack2Data);
			
//			cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", srTransRec.usTrack3Len);
                        cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", inRefundCnt);
			if (srTransRec.usTrack3Len == 0)
			{
				memset(srTransRec.szTrack3Data,0x00,sizeof(srTransRec.szTrack3Data));
				srTransRec.szTrack3Data[0] = '0';
			}
//			cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
                        
                        memset(szAmountData, 0x00, sizeof(szAmountData));
                        get_env("#STMTREFUNDAMT", szAmountData, 40);
                        cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
#endif
		}
 	}
	 
	 cJSON_AddStringToObject(pJsonRoot, "CMD", ECRReq.szCMD);
	 if (strcmp(ECRReq.szTYPE, "EWALLET") == 0) //SALE, VOID, REFUND, TIP ADJUST, RETRIEVAL (print any) type for "EWALLET" must specify E-Wallet type (i.e. Alipay, WeChat, QQ) 
         {
             vdDebug_LogPrintf("srTransRec.szTrack3Data=[%s]", srTransRec.szTrack3Data);
             cJSON_AddStringToObject(pJsonRoot, "TYPE", srTransRec.szTrack3Data);
         }
         else
         {
             cJSON_AddStringToObject(pJsonRoot, "TYPE", ECRReq.szTYPE);
         }

		 if(strcmp(ECRReq.szCMD, "SETTLEMENT") == 0 || strcmp(ECRReq.szCMD, "TOTAL") == 0) 
		 	goto lblDONE;
         
         if ((memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE)) || (memcmp(ECRResp.auth_code,"Y1",ECR_RESP_CODE_SIZE))){
             put_env("P1TEXT", " ", 1);
             put_env("P2TEXT", " ", 1);
             put_env("P3TEXT", " ", 1);
             put_env("P4TEXT", " ", 1);
         }
 
         // Add Printing Text Object if Exists
         inPrintingFlag = get_env_int("#P1FLAG");
         if (inPrintingFlag == 1) {
             // Printing Text Part 1
             memset(szPrintData, 0x00, sizeof(szPrintData));
             inPrintDataLen = get_env_int("#P1LEN");
             memset(szTemp2, 0x00, sizeof(szTemp2));
             sprintf(szTemp2, "%04d", inPrintDataLen);
             get_env("#P1TEXT", szPrintData, sizeof(inPrintDataLen));
             
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART1TYPE", "7E");
//             cJSON_AddNumberToObject(pJsonRoot, "PRINTTEXTPART1LENGTH", inPrintDataLen);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART1LENGTH", szTemp2);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART1VALUE", szPrintData);
         }
         
         inPrintingFlag = get_env_int("#P2FLAG");
         if (inPrintingFlag == 1) {
             // Printing Text Part 2
             memset(szPrintData, 0x00, sizeof(szPrintData));
             inPrintDataLen = get_env_int("#P2LEN");
             memset(szTemp2, 0x00, sizeof(szTemp2));
             sprintf(szTemp2, "%04d", inPrintDataLen);
             get_env("#P2TEXT", szPrintData, sizeof(inPrintDataLen));
             
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART2TYPE", "7F");
//             cJSON_AddNumberToObject(pJsonRoot, "PRINTTEXTPART2LENGTH", inPrintDataLen);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART2LENGTH", szTemp2);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART2VALUE", szPrintData);
         }
         
         inPrintingFlag = get_env_int("#P3FLAG");
         if (inPrintingFlag == 1) {
             // Printing Text Part 3
             memset(szPrintData, 0x00, sizeof(szPrintData));
             inPrintDataLen = get_env_int("#P3LEN");
             memset(szTemp2, 0x00, sizeof(szTemp2));
             sprintf(szTemp2, "%04d", inPrintDataLen);
             get_env("#P3TEXT", szPrintData, sizeof(inPrintDataLen));
             
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART3TYPE", "7G");
//             cJSON_AddNumberToObject(pJsonRoot, "PRINTTEXTPART3LENGTH", inPrintDataLen);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART3LENGTH", szTemp2);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART3VALUE", szPrintData);
         }
         
         inPrintingFlag = get_env_int("#P4FLAG");
         if (inPrintingFlag == 1) {
             // Printing Text Part 4
             memset(szPrintData, 0x00, sizeof(szPrintData));
             inPrintDataLen = get_env_int("#P4LEN");
             memset(szTemp2, 0x00, sizeof(szTemp2));
             sprintf(szTemp2, "%04d", inPrintDataLen);
             get_env("#P4TEXT", szPrintData, sizeof(inPrintDataLen));
             
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART4TYPE", "7H");
//             cJSON_AddNumberToObject(pJsonRoot, "PRINTTEXTPART4LENGTH", inPrintDataLen);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART4LENGTH", szTemp2);
             cJSON_AddStringToObject(pJsonRoot, "PRINTTEXTPART4VALUE", szPrintData);
         }
         
         lblDONE:
         
	 p = NULL;
	 p = cJSON_Print(pJsonRoot);
	 if(NULL == p)
	 {
		 cJSON_Delete(pJsonRoot);		 
		 pJsonRoot = NULL;
		 return 0;
	 }
	 
	 cJSON_Delete(pJsonRoot);	 
	 pJsonRoot = NULL;
 
	 //printf("%s\n", p);
	 vdDebug_LogPrintf("JSON =[%s]", p);
 
	 memset(uszJsonString, 0x00, sizeof(uszJsonString));
	 sprintf(uszJsonString, "@%s%s", MOLMD5String(p), p);
	 vdDebug_LogPrintf("ECR COMMAND =[%s]", uszJsonString);
 
	 free(p); 
	 p = NULL;
         
    vdDebug_LogPrintf("ECR send offset[%d], isEcrProcessing=%d", offset, isEcrProcessing);
	
	inECRSendComPacket(uszJsonString, strlen(uszJsonString), VS_FALSE, 5,VS_FALSE, 3); 
    //inECRSendComPacket(szWebServiceResponse, strlen(szWebServiceResponse), VS_FALSE, 5,VS_FALSE, 3);
	
    vdDebug_LogPrintf("inECRSendResponse End");
	isEcrProcessing = FALSE;
    TCP_ServerCloseSocket();

    return VS_SUCCESS;
}

#else
int inECRSendResponse(void)
{
#define CARD_ENTRY_MSR                  1
#define CARD_ENTRY_MANUAL               2
#define CARD_ENTRY_ICC                  3
#define CARD_ENTRY_FALLBACK             4
#define CARD_ENTRY_WAVE                 5
#define CARD_ENTRY_EASY_ICC         	6

	int offset = 0, inSize;
	int inSerialPort=0;
    CTOS_RTC SetRTC;

	cJSON * pJsonRoot = NULL;
	unsigned char uszJsonString[1024+1];
	unsigned char uszECRReceiveData[1024+1];
	unsigned char uszMD5Data[1024+1];
    char szWebServiceResponse[2048+1];
	char szTemp[44 + 1];
	char szTemp2[44 + 1];
	
	unsigned char szDate[12 + 1];
	unsigned char szTime[6 + 1];

    char szSettleData[128+1];
	char szTempdata[12+1];
	char * p = NULL;		

	BYTE   EMVtagVal[64];
	USHORT EMVtagLen; 
	
	
	int inLen=0;
	int i=0;
    int inCount=0;
	int inUOBVMJSettled=0, inAmexSettled=0;
	int inUOBVMJSettledApproved=0, inAmexSettledApproved=0;
		
	cJSON * pJsonUOB_VMJ = NULL;
	cJSON * pJsonAMEX = NULL;
	cJSON * pJsonRootArray = NULL;
	
	vdDebug_LogPrintf("ECRResp.req_resp[%s] ECRResp.auth_code[%s]", ECRResp.resp_code, ECRResp.auth_code);

	 p = NULL;
 
 // pack ECR JSON response data
 
	 pJsonRoot = NULL;
	 pJsonRoot = cJSON_CreateObject();
	 if(NULL == pJsonRoot)
	 {
		 //error happend here
		 return 0;
	 }

	 if ((!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE)) || (!memcmp(ECRResp.auth_code,"Y1",ECR_RESP_CODE_SIZE)))
		 cJSON_AddStringToObject(pJsonRoot, "STATUS", "Approved");
	 else
     {
          if(strcmp(ECRReq.szCMD, "BARCODE") == 0 || strcmp(ECRReq.szCMD, "READCARD") == 0)
          {
		  	if(!memcmp(ECRResp.resp_code,"CN",ECR_RESP_CODE_SIZE))
		  	    cJSON_AddStringToObject(pJsonRoot, "STATUS", "Cancelled");
			else
				cJSON_AddStringToObject(pJsonRoot, "STATUS", "Declined");
          }
		  else if(strcmp(ECRReq.szCMD, "GET_TOTAL") == 0 || strcmp(ECRReq.szCMD, "SETTLE") == 0 || strcmp(ECRReq.szCMD, "GET_LAST_TOTAL") == 0) /*removed STATUS for GET_TOTAL command*/
		  {
		  }
		  else
              cJSON_AddStringToObject(pJsonRoot, "STATUS", "Declined");
     }

    if(strcmp(ECRReq.szCMD, "ECHOTEST") == 0)
    {
        vdDebug_LogPrintf("inECRSendResponse: ECHOTEST");
    }
    else if(strcmp(ECRReq.szCMD, "BARCODE") == 0)
    {
        vdDebug_LogPrintf("inECRSendResponse: ECHOTEST");
    }	
    else if(strcmp(ECRReq.szCMD, "GET_TOTAL") == 0)
    {
        vdDebug_LogPrintf("inECRSendResponse: GET_TOTAL");
		if(memcmp(ECRResp.resp_code, "99", 2) == 0)
			cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "N");
		else
            cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "Y");
    }
	else if(strcmp(ECRReq.szCMD, "GET_LAST_TOTAL") == 0)
	{
        vdDebug_LogPrintf("inECRSendResponse: GET_TOTAL");
		if(memcmp(ECRResp.resp_code, "99", 2) == 0)
			cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "N");
		else
            cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "Y");
	}
    else if(strcmp(ECRReq.szCMD, "SETTLE") == 0)
    {
        inUOBVMJSettledApproved=get_env_int("ACQ01_SETTLED_APPROVED");
        inAmexSettledApproved=get_env_int("ACQ02_SETTLED_APPROVED");

		vdDebug_LogPrintf("inUOBVMJSettledApproved:%d", inUOBVMJSettledApproved);
		vdDebug_LogPrintf("inAmexSettledApproved:%d", inAmexSettledApproved);

        if(memcmp(szAcquirerName, "ALL", 3) == 0)
        {
			if(inUOBVMJSettledApproved == 1 && inAmexSettledApproved == 1)
				cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "Y");
			else
				cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "N");
        }
		else if(memcmp(szAcquirerName, "UOB VMJ", 7) == 0)
		{
			if(inUOBVMJSettledApproved == 1)
			    cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "Y");
			else
				cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "N");
		}
		else if(memcmp(szAcquirerName, "AMEX", 4) == 0)
		{
			if(inAmexSettledApproved == 1)
			    cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "Y");
			else
				cJSON_AddStringToObject(pJsonRoot, "SUCCESS", "N");
		}

		inUOBVMJSettled=get_env_int("ACQ01_SETTLED");
		inAmexSettled=get_env_int("ACQ02_SETTLED");

		vdDebug_LogPrintf("inUOBVMJSettled:%d", inUOBVMJSettled);
		vdDebug_LogPrintf("inAmexSettled:%d", inAmexSettled);

        /*create array*/	 
        pJsonRootArray = NULL;
        pJsonRootArray = cJSON_CreateArray();
		
        if(inUOBVMJSettled == 1)
        {
            /*create object*/	
            pJsonUOB_VMJ = NULL;
            pJsonUOB_VMJ = cJSON_CreateObject();

            memset(szSettleData, 0, sizeof(szSettleData));
			get_env("ACQ01_SETTLED_DATA", szSettleData, 128);
			vdDebug_LogPrintf("HOST_UOB_VMJ szSettleData:%s", szSettleData);
            //memset(szTemp, 0x00, sizeof(szTemp));
            //memcpy(szTemp, &szCBBSettleData1[45], 8);		//status
            //vdDebug_PrintOnPaper("[%s]", sztempdata1);

			cJSON_AddStringToObject(pJsonAMEX, "ACQNAME", "VISA VMJ");
			
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[0], 3);		//total sale count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "SALECNT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[3], 12);	//total sale amount
            sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonUOB_VMJ, "SALEAMT", szTemp);
			
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[15], 3);		//total void count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "VOIDCNT", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[18], 12);	//total void amount
            sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonUOB_VMJ, "VOIDAMT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[30], 3);		//total refund count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "REFUNDCNT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[33], 12);	//total void amount
            sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonUOB_VMJ, "REFUNDAMT", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memcpy(szTemp, &szSettleData[45], 8);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "TERMINALID", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memcpy(szTemp, &szSettleData[53], 15);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "MERCHANTID", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[68], 6);		//batch number
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonUOB_VMJ, "BATCHNO", szTemp);

			if(inUOBVMJSettledApproved == 1)
				cJSON_AddStringToObject(pJsonUOB_VMJ, "STATUS", "Approved");
			else
				cJSON_AddStringToObject(pJsonUOB_VMJ, "STATUS", "Declined");
			
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "STATUS", szStatus);			 
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "SALECNT", "1");
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "SALEAMT", "000000000001");
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "VOIDCNT", "1");
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "VOIDAMT", "000000000002");				 
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "REFUNDCNT", "1");
            //cJSON_AddStringToObject(pJsonUOB_VMJ, "REFUNDAMT", "000000000003");
            
            /*add UOB_VMJ to array*/
            cJSON_AddItemToArray(pJsonRootArray, pJsonUOB_VMJ);
            
            //put_env_int("HOST_UOB_VMJ_SETTLED", 0);
			put_env("ACQ01_SETTLED_DATA", " ", 1);
        }			 
        
        if(inAmexSettled)
        {
            /*create object*/	
            pJsonAMEX= NULL;
            pJsonAMEX = cJSON_CreateObject();
            
            memset(szSettleData, 0, sizeof(szSettleData));
			get_env("ACQ01_SETTLED_DATA", szSettleData, 128);
			vdDebug_LogPrintf("HOST_AMEX szSettleData:%s", szSettleData);
			
            //memset(szTemp, 0x00, sizeof(szTemp));
            //memcpy(szTemp, &szCBBSettleData1[45], 8);		//status
            //vdDebug_PrintOnPaper("[%s]", sztempdata1);

			cJSON_AddStringToObject(pJsonAMEX, "ACQNAME", "AMEX");
			
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[0], 3);		//total sale count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonAMEX, "SALECNT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[3], 12);	//total sale amount
            //sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonAMEX, "SALEAMT", szTemp);
			
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[15], 3);		//total void count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonAMEX, "VOIDCNT", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[18], 12);	//total void amount
            //sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonAMEX, "VOIDAMT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[30], 3);		//total refund count
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonAMEX, "REFUNDCNT", szTemp);
            
            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[33], 12);	//total void amount
            //sprintf(szTemp2, "%.0f", atol(szTemp2));
			vdCTOSS_FormatAmount("NN,NNN,NNN,NNn.nn", szTemp2, szTemp);
            cJSON_AddStringToObject(pJsonAMEX, "REFUNDAMT", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memcpy(szTemp, &szSettleData[45], 8);
			cJSON_AddStringToObject(pJsonAMEX, "TERMINALID", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memcpy(szTemp, &szSettleData[53], 15);
			cJSON_AddStringToObject(pJsonAMEX, "MERCHANTID", szTemp);

            memset(szTemp, 0x00, sizeof(szTemp));
			memset(szTemp2, 0x00, sizeof(szTemp2));
            memcpy(szTemp2, &szSettleData[68], 6);		//batch number
            inCount = atoi(szTemp2);
			sprintf(szTemp, "%d", inCount);
			cJSON_AddStringToObject(pJsonAMEX, "BATCHNO", szTemp);

			if(inAmexSettledApproved == 1)
				cJSON_AddStringToObject(pJsonAMEX, "STATUS", "Approved");
			else
				cJSON_AddStringToObject(pJsonAMEX, "STATUS", "Declined");
			
            /*add UOB_VMJ to array*/
            cJSON_AddItemToArray(pJsonRootArray, pJsonAMEX);
            
            //put_env_int("HOST_AMEX_SETTLED", 0);
			put_env("ACQ01_SETTLED_DATA", " ", 1);
        }
        
        if(inUOBVMJSettled == 1 || inAmexSettled == 1)
        {
            cJSON_AddItemToObject(pJsonRoot, "BATCH", pJsonRootArray);
        }

		put_env_int("ACQ01_SETTLED", 0);
		put_env_int("ACQ02_SETTLED", 0);
		
		put_env_int("ACQ01_SETTLED_APPROVED", 0);
		put_env_int("ACQ02_SETTLED_APPROVED", 0);
    }
    else if(strcmp(ECRReq.szCMD, "PRINTING") == 0)
    {
    
    }
    else if(strcmp(ECRReq.szCMD, "READCARD") == 0)
    {
    
    }
	 else if ((strcmp(ECRReq.szCMD, "TOTAL") != 0) && (strcmp(ECRReq.szCMD, "SETTLEMENT") != 0))
	 {
		cJSON_AddStringToObject(pJsonRoot, "RESP", ECRResp.resp_code);

		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))
		{
			//sprintf(szTemp, "%06ld", srTransRec.ulTraceNum);	 
			//vdDebug_LogPrintf("TRACE[%c]", szTemp);
			cJSON_AddStringToObject(pJsonRoot, "TRACE", ECRResp.inv_no);//cJSON_AddStringToObject(pJsonRoot, "TRACE", szTemp);
		}
		else
			cJSON_AddStringToObject(pJsonRoot, "TRACE", "");

		//Read the date and the time //
		CTOS_RTCGet(&SetRTC);	

		//memset(szDate 0x00, sizeof(szDate));
		//memset(szTime 0x00, sizeof(szTime));
		sprintf(szDate, "%02d-%02d-20%02d", (int)SetRTC.bDay, (int)SetRTC.bMonth, (int)SetRTC.bYear);
		sprintf(szTime, "%02d:%02d", SetRTC.bHour, SetRTC.bMinute);

		cJSON_AddStringToObject(pJsonRoot, "DATE", szDate);
		cJSON_AddStringToObject(pJsonRoot, "TIME", szTime);
		cJSON_AddStringToObject(pJsonRoot, "APPCODE", srTransRec.szAuthCode);
		cJSON_AddStringToObject(pJsonRoot, "PAN", ECRResp.card_no);
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
			cJSON_AddStringToObject(pJsonRoot, "EXPDATE", ECRResp.exp_date);
		else
			cJSON_AddStringToObject(pJsonRoot, "EXPDATE", "");	 

		wub_hex_2_str(srTransRec.szTotalAmount, szTemp, 6);
		cJSON_AddNumberToObject(pJsonRoot, "AMT", atol(szTemp));

		wub_hex_2_str(srTransRec.szTipAmount, szTemp, 6);
		cJSON_AddNumberToObject(pJsonRoot, "TIPS", atol(szTemp));

		vdDebug_LogPrintf("ENTRYMODE[%ld]", srTransRec.byEntryMode);
		if (srTransRec.byEntryMode == CARD_ENTRY_MSR)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "S");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_MANUAL)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "M");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_ICC)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "C");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_FALLBACK)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "F");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_WAVE)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "W");	
		}
		else if (srTransRec.byEntryMode == CARD_ENTRY_EASY_ICC)
		{
			cJSON_AddStringToObject(pJsonRoot, "ENTRYMODE", "E");	
		}
		 
		cJSON_AddStringToObject(pJsonRoot, "TERMINALID", srTransRec.szTID);
		cJSON_AddStringToObject(pJsonRoot, "MERCHANTID", srTransRec.szMID);
		cJSON_AddStringToObject(pJsonRoot, "CARDHOLDERNAME", ECRResp.card_name);

		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
			cJSON_AddStringToObject(pJsonRoot, "BATCHNO", ECRResp.batch_no);
		else
			cJSON_AddStringToObject(pJsonRoot, "BATCHNO", "");
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	 
		{
			if (strcmp(ECRReq.szTYPE, "EWALLET") == 0)
				cJSON_AddStringToObject(pJsonRoot, "REFNUM", srTransRec.szCardLable);
			else
				cJSON_AddStringToObject(pJsonRoot, "REFNUM", srTransRec.szRRN);//cJSON_AddStringToObject(pJsonRoot, "REFNUM", ECRResp.inv_no);
		}
		else
			cJSON_AddStringToObject(pJsonRoot, "REFNUM", "");

		cJSON_AddStringToObject(pJsonRoot, "ECRREF", ECRReq.szECRREF);
		if (strcmp(ECRReq.szTYPE, "EWALLET") == 0)
			cJSON_AddStringToObject(pJsonRoot, "APP", ECRResp.issuer);
		else
			cJSON_AddStringToObject(pJsonRoot, "APP", srTransRec.szCardLable);

		memset(EMVtagVal, 0x00, sizeof(EMVtagVal));
		EMVtagLen = srTransRec.stEMVinfo.T84_len;
		memcpy(EMVtagVal, srTransRec.stEMVinfo.T84, EMVtagLen);
		wub_hex_2_str(EMVtagVal, szTemp, EMVtagLen);
		vdDebug_LogPrintf("AID[%s]", ECRResp.resp_code);

		cJSON_AddStringToObject(pJsonRoot, "AID", szTemp);

		wub_hex_2_str(srTransRec.stEMVinfo.T9F26, szTemp, 8);
		vdDebug_LogPrintf("TC[%s]", szTemp);
		if(!memcmp(ECRResp.resp_code,"00",ECR_RESP_CODE_SIZE))	  
			cJSON_AddStringToObject(pJsonRoot, "TC", szTemp);
		else	
			cJSON_AddStringToObject(pJsonRoot, "TC", "");
		cJSON_AddStringToObject(pJsonRoot, "CURRCODE", srTransRec.stEMVinfo.T9F29);
		cJSON_AddStringToObject(pJsonRoot, "FXRATE", ECRResp.DCC_ConvRate);
		cJSON_AddNumberToObject(pJsonRoot, "FOREIGNAMT", atol(ECRResp.DCC_ConvAmt));
		cJSON_AddStringToObject(pJsonRoot, "DCCMSG", "");

		memset(szTemp, 0x00, sizeof(szTemp));
		sprintf(szTemp, "%02d", srTransRec.byPrintType);	//epp term
		cJSON_AddStringToObject(pJsonRoot, "TENDER", szTemp);//cJSON_AddStringToObject(pJsonRoot, "TENDER", "");	 
	}
	else
 	{ 	
 		if ((strcmp(ECRReq.szCMD, "SETTLEMENT") == 0) && ((strcmp(ECRReq.szTYPE, "Alipay") == 0) || (strcmp(ECRReq.szTYPE, "WeChat") == 0) || (strcmp(ECRReq.szTYPE, "QQ") == 0)))
		{
			// for Alipay, WeChat, QQ, all amount are in srTransRec.szTrack3Data because srTransRec.szTrack1Data and srTransRec.szTrack2Data are in use
			cJSON_AddNumberToObject(pJsonRoot, "SALECNT", srTransRec.usTrack1Len);
			if (srTransRec.usTrack1Len == 0)
			{
				memset(srTransRec.baChipData,0x00,sizeof(srTransRec.baChipData));
				srTransRec.baChipData[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "SALEAMT", srTransRec.baChipData);
			
			cJSON_AddNumberToObject(pJsonRoot, "VOIDCNT", srTransRec.usTrack2Len);
			if (srTransRec.usTrack2Len == 0)
			{
				memset(srTransRec.baAdditionalData,0x00,sizeof(srTransRec.baAdditionalData));
				srTransRec.baAdditionalData[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "VOIDAMT", srTransRec.baAdditionalData);
			
			cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", srTransRec.usTrack3Len);
			if (srTransRec.usTrack3Len == 0)
			{
				memset(srTransRec.szTrack3Data,0x00,sizeof(srTransRec.szTrack3Data));
				srTransRec.szTrack3Data[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
		}
		else
		{
			cJSON_AddNumberToObject(pJsonRoot, "SALECNT", srTransRec.usTrack1Len);
			if (srTransRec.usTrack1Len == 0)
			{
				memset(srTransRec.szTrack1Data,0x00,sizeof(srTransRec.szTrack1Data));
				srTransRec.szTrack1Data[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "SALEAMT", srTransRec.szTrack1Data);
			
			cJSON_AddNumberToObject(pJsonRoot, "VOIDCNT", srTransRec.usTrack2Len);
			if (srTransRec.usTrack2Len == 0)
			{
				memset(srTransRec.szTrack2Data,0x00,sizeof(srTransRec.szTrack2Data));
				srTransRec.szTrack2Data[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "VOIDAMT", srTransRec.szTrack2Data);
			
			cJSON_AddNumberToObject(pJsonRoot, "REFUNDCNT", srTransRec.usTrack3Len);
			if (srTransRec.usTrack3Len == 0)
			{
				memset(srTransRec.szTrack3Data,0x00,sizeof(srTransRec.szTrack3Data));
				srTransRec.szTrack3Data[0] = '0';
			}
			cJSON_AddStringToObject(pJsonRoot, "REFUNDAMT", srTransRec.szTrack3Data);
		}
 	}
	 
	 cJSON_AddStringToObject(pJsonRoot, "CMD", ECRReq.szCMD);

    if(strcmp(ECRReq.szCMD, "BARCODE") == 0)
    {
		cJSON_AddStringToObject(pJsonRoot, "CONTENT", srTransRec.szStoreID);
    }
	if(strcmp(ECRReq.szCMD, "READCARD") == 0)
	{
		cJSON_AddStringToObject(pJsonRoot, "CONTENT", srTransRec.szPAN);
	}
	else
	{
        if (strcmp(ECRReq.szTYPE, "EWALLET") == 0)	//SALE, VOID, REFUND, TIP ADJUST, RETRIEVAL (print any) type for "EWALLET" must specify E-Wallet type (i.e. Alipay, WeChat, QQ) 
        {
            //cJSON_AddStringToObject(pJsonRoot, "TYPE", srTransRec.szTrack3Data);
            cJSON_AddStringToObject(pJsonRoot, "TYPE", ECRReq.szTYPE);
        }
		else if(strcmp(ECRReq.szCMD, "PRINTING") == 0 || strcmp(ECRReq.szCMD, "BARCODE") == 0)
		{
		}
        else
        {
            cJSON_AddStringToObject(pJsonRoot, "TYPE", ECRReq.szTYPE);
        }
    }
	 
	 p = NULL;
	 p = cJSON_Print(pJsonRoot);
	 if(NULL == p)
	 {
		 cJSON_Delete(pJsonRoot);		 
		 pJsonRoot = NULL;
		 return 0;
	 }
	 
	 cJSON_Delete(pJsonRoot);	 
	 pJsonRoot = NULL;
 
	 //printf("%s\n", p);
	 vdDebug_LogPrintf("JSON =[%s]", p);
     
	 memset(uszJsonString, 0x00, sizeof(uszJsonString));
	 sprintf(uszJsonString, "@%s%s", MOLMD5String(p), p);
	 vdDebug_LogPrintf("ECR COMMAND =[%s], len[%d]", uszJsonString, strlen(uszJsonString));

     
	 while(uszJsonString[i]) {
		//putchar (toupper(szValue[i]));
		//putchar (toupper(szValue[i]));
		if(uszJsonString[i] == 0x7d)
		     break;
		i++;
	 }
     
     vdDebug_LogPrintf("uszJsonString len:%d", i);
#if 0
 HTTP/1.1 200 OK
Server: nginx/1.12.1
Date: Sun, 19 Aug 2018 02:59:07 GMT
Content-Type: text/plain; charset=utf-8
Content-Length: 1353
Connection: keep-alive
#endif

	 
#if 0
memset(szWebServiceResponse, 0, sizeof(szWebServiceResponse));

strcpy(szWebServiceResponse, 
HTTP/1.1 201 
Content-Type: application/json;charset=UTF-8
Content-Length: 350
X-MD5:8C7C3D75872FD6707C416069D3963D9E

{
        "STATUS":    "Approved",
        "RESP":    "00",
        "TRACE":    "",
        "DATE":    "21-04-2021",
        "TIME":    "18:19",
        "APPCODE":    "181929",
        "PAN":    "",
        "EXPDATE":    "",
        "AMT":    100,
        "TIPS":    0,
        "ENTRYMODE":    "W",
        "TERMINALID":    "11112222",
        "MERCHANTID":    "111122223333333",
        "CARDHOLDERNAME":    "",
        "BATCHNO":    "",
        "REFNUM":    "210421000026",
        "ECRREF":    "",
        "APP":    "VISA OFFUS",
        "AID":    "A0000000031010",
        "TC":    "A0E8F975843B4411",
        "CURRCODE":    "SGD",
        "FXRATE":    "",
        "FOREIGNAMT":    0,
        "DCCMSG":    "",
        "TENDER":    "00",
        "CMD":    "SALE",
        "TYPE":    "VMJ" 
}

#else
     memset(szWebServiceResponse, 0, sizeof(szWebServiceResponse));
     strcpy(szWebServiceResponse, "HTTP/1.1 201");
	 strcat(szWebServiceResponse, "\x0D\x0A");

	 //strcat(szWebServiceResponse, "User-Agent: Jakarta Commons-HttpClient/3.1");
     //strcat(szWebServiceResponse, "\x0D\x0A");

     //strcat(szWebServiceResponse, "Host: 192.168.0.27:7000");
     //strcat(szWebServiceResponse, "\x0D\x0A");

     strcat(szWebServiceResponse, "Content-Type: application/json;charset=UTF-8");
     strcat(szWebServiceResponse, "\x0D\x0A");
     #if 0
	 strcat(szWebServiceResponse, "Content-Length: 350");
	 strcat(szWebServiceResponse, "\x0D\x0A");
	 #else
     memset(szTemp, 0, sizeof(szTemp));
	 sprintf(szTemp, "%d", (strlen(uszJsonString)-33));
	 strcat(szWebServiceResponse, "Content-Length: ");
	 strcat(szWebServiceResponse, szTemp);
	 strcat(szWebServiceResponse, "\x0D\x0A");
     #endif
  
     strcat(szWebServiceResponse, "X-MD5: ");
     strcat(szWebServiceResponse, MOLMD5String(p));
     strcat(szWebServiceResponse, "\x0D\x0A\x0D\x0A");
	 
     vdDebug_LogPrintf("1. szWebServiceResponse len[%d]", strlen(szWebServiceResponse));
	 
     strcat(szWebServiceResponse, p);
	 
	 vdDebug_LogPrintf("2. szWebServiceResponse len[%d]", strlen(szWebServiceResponse));
#endif

	 vdDebug_LogPrintf("szWebServiceResponse[%s]", szWebServiceResponse);

	 free(p); 
	 p = NULL;
 
	vdDebug_LogPrintf("ECR send offset[%d], isEcrProcessing=%d, szWebServiceResponse=%d", offset, isEcrProcessing, strlen(szWebServiceResponse));

	
	//inECRSendComPacket(uszJsonString, strlen(uszJsonString), VS_FALSE, 5,VS_FALSE, 3);
	DebugAddHEX("szWebServiceResponse",szWebServiceResponse, strlen(szWebServiceResponse));
	inECRSendComPacket(szWebServiceResponse, strlen(szWebServiceResponse), VS_FALSE, 5,VS_FALSE, 3);

	
    vdDebug_LogPrintf("inECRSendResponse End");
	isEcrProcessing = FALSE;
    TCP_ServerCloseSocket();

    return VS_SUCCESS;
}
#endif

int inECRFillResponse(void)
{
    memcpy(ECRResp.resp_code, srTransRec.szRespCode, 2);
}

#define DISPLAY_POSITION_LEFT 0
#define DISPLAY_POSITION_CENTER 1
#define DISPLAY_POSITION_RIGHT 2
#define DISPLAY_LINE_SIZE 16

void setLCDPrint(int line,int position, char *pbBuf)
{
    int iInitX = 0;
    int lens = 0;
    switch(position)
    {
        case DISPLAY_POSITION_LEFT:
            CTOS_LCDTPrintXY(1, line, pbBuf);
            break;
        case DISPLAY_POSITION_CENTER:
            lens = strlen(pbBuf);
            iInitX = (16 - lens) / 2 + 1;
            CTOS_LCDTPrintXY(iInitX, line, pbBuf);
            break;
        case DISPLAY_POSITION_RIGHT:
            lens = strlen(pbBuf);
            iInitX = 16 - lens + 1;
            CTOS_LCDTPrintXY(iInitX, line, pbBuf);
            break;
    }
}

void vduiWarningSound(void)
{
	CTOS_LEDSet(d_LED1, d_ON);
	CTOS_LEDSet(d_LED2, d_ON);
	CTOS_LEDSet(d_LED3, d_ON);
//	CTOS_BackLightSet (d_BKLIT_LCD, d_ON);
	//CTOS_Sound(6000, 50); // in 10 ms
	//CTOS_Delay(500);
	//CTOS_Sound(6000, 50); // in 10 ms
	CTOS_Beep();
	CTOS_Delay(300);
	CTOS_Beep();
	CTOS_LEDSet(d_LED1, d_OFF);
	CTOS_LEDSet(d_LED2, d_OFF);
	CTOS_LEDSet(d_LED3, d_OFF);
//	CTOS_BackLightSet (d_BKLIT_LCD, d_OFF);
}

void vduiLightOn(void)
{
  	CTOS_BackLightSetEx(d_BKLIT_LCD,d_ON,80000);
}

USHORT usCTOSS_ShareECRInitialize(void)
{
    BYTE	byEMVConfig[30];
    BYTE    bStatus[4];
    USHORT  usResult;
    USHORT  usLen;

	usResult = inECR_InitCOM();
    
    return usResult;
}


/////////////////////////////////////////////////////////////////////////////////////////
short shPrintCheckPaper(void)
{
	unsigned short inRet;
	unsigned char key;
	
	while(1)
	{
		inRet = CTOS_PrinterStatus();
		if (inRet==d_OK)
			return 0;
		else if(inRet==d_PRINTER_PAPER_OUT)
		{
			return -1;	
		}		
	}	
}

int inPrintECRPacket(char *szTitle, unsigned char *pucMessage, int inLen)
{
	char ucLineBuffer[24 + 1 + 1];
	unsigned char *pucBuff;
	int inBuffPtr = 0;

	shPrintCheckPaper();

    vdDebug_LogPrintf("szTitle:[%s] Len[%d]", szTitle, inLen);
    
	if (inLen <= 0)
		return (d_OK);

	CTOS_PrinterPutString(szTitle);

	CTOS_PrinterPutString("\n----\n");
	pucBuff = pucMessage + inLen;

	while (pucBuff > pucMessage)
	{
		memset(ucLineBuffer, 0x00, sizeof(ucLineBuffer));
	  	for (inBuffPtr = 0; (inBuffPtr < 24) && (pucBuff > pucMessage); inBuffPtr += 3)
	  	{
	    	sprintf(&ucLineBuffer[inBuffPtr], "%02X ", *pucMessage);
	      	pucMessage++;
	  	}
	  	ucLineBuffer[24] = '\n';
		
	  	CTOS_PrinterPutString(ucLineBuffer);
	}

	CTOS_PrinterPutString("\n----\n\n\n");

	return (d_OK);
}

char *getMD5(char* szData, char* szKey)
{
   char sStartTag[512 + 1] = {0};
   char sEndTag[512 + 1] = {0};
   char *Startptr;
   char *Endptr;
   char szValue[512 + 1];

   int fFound = FALSE;
   int inStartPos=0, inEndPos=0;
   int inLen=0,inTotalLen;

	int i = 0;

    //printf("szData:%s, szKey:%s\n",szData, szKey);

    inTotalLen=strlen(szData);
    Startptr = strstr(szData, sStartTag);

    inStartPos=strlen(Startptr)-strlen(sStartTag);
    //printf("sStartTag is: %s\nlen:%d\n", Startptr, inStartPos);

	Startptr = strstr(szData, szKey);

    inEndPos=strlen(Startptr);
    //printf("sEndTag is: %s\nlen:%d\n", Startptr+strlen(szKey)+2, inEndPos);

    inLen=inStartPos-inEndPos;

    //printf("inLen is: %d\n", inLen);

    memset(szValue, 0, sizeof(szValue));
    memcpy(szValue, szData+(inLen+strlen(szKey)+2), 32);
    //printf("%s=(%s)\n",szKey, szValue);

	while(szValue[i]) {
	   //putchar (toupper(szValue[i]));
	   //putchar (toupper(szValue[i]));
	   szValue[i]=toupper(szValue[i]);
	   i++;
	}

	return szValue;
}

int getContentLength(char* szData, char* szKey)
{
   char sStartTag[512 + 1] = {0};
   char sEndTag[512 + 1] = {0};
   char *Startptr;
   char *Endptr;
   char szValue[512 + 1];

   int fFound = FALSE;
   int inStartPos=0, inEndPos=0;
   int inLen=0,inTotalLen;

    //printf("szData:%s, szKey:%s\n",szData, szKey);

    inTotalLen=strlen(szData);
    Startptr = strstr(szData, sStartTag);

    inStartPos=strlen(Startptr)-strlen(sStartTag);
    //printf("sStartTag is: %s\nlen:%d\n", Startptr, inStartPos);

	Startptr = strstr(szData, szKey);

    inEndPos=strlen(Startptr);
    //printf("sEndTag is: %s\nlen:%d\n", Startptr+strlen(szKey)+2, inEndPos);

    inLen=inStartPos-inEndPos;

    //printf("inLen is: %d\n", inLen);

    memset(szValue, 0, sizeof(szValue));
    memcpy(szValue, szData+(inLen+strlen(szKey)+2), 4);
    printf("%s=(%s)\n",szKey, szValue);

	return atoi(szValue);
}

int inECRSendResponse2(void)
{
#define CARD_ENTRY_MSR                  1
#define CARD_ENTRY_MANUAL               2
#define CARD_ENTRY_ICC                  3
#define CARD_ENTRY_FALLBACK             4
#define CARD_ENTRY_WAVE                 5
#define CARD_ENTRY_EASY_ICC         	6

	int offset = 0, inSize;
	int inSerialPort=0;
    CTOS_RTC SetRTC;

	cJSON * pJsonRoot = NULL;
	unsigned char uszJsonString[1024+1];
	unsigned char uszECRReceiveData[1024+1];
	unsigned char uszMD5Data[1024+1];

	unsigned char szDate[12 + 1];
	unsigned char szTime[6 + 1];

	char * p = NULL;		

	BYTE   EMVtagVal[64];
	USHORT EMVtagLen; 
	char szTemp[44 + 1];
	char szWebServiceResponse[2048+1];
	int inLen=0;
	
	vdDebug_LogPrintf("ECRResp.req_resp[%s] ECRResp.auth_code[%s]", ECRResp.resp_code, ECRResp.auth_code);

#if 0
memset(szWebServiceResponse, 0, sizeof(szWebServiceResponse));

strcpy(szWebServiceResponse, 
HTTP/1.1 201 
Content-Type: application/json;charset=UTF-8
Content-Length: 350
X-MD5:8C7C3D75872FD6707C416069D3963D9E

{
        "STATUS":    "Approved",
        "RESP":    "00",
        "TRACE":    "",
        "DATE":    "21-04-2021",
        "TIME":    "18:19",
        "APPCODE":    "181929",
        "PAN":    "",
        "EXPDATE":    "",
        "AMT":    100,
        "TIPS":    0,
        "ENTRYMODE":    "W",
        "TERMINALID":    "11112222",
        "MERCHANTID":    "111122223333333",
        "CARDHOLDERNAME":    "",
        "BATCHNO":    "",
        "REFNUM":    "210421000026",
        "ECRREF":    "",
        "APP":    "VISA OFFUS",
        "AID":    "A0000000031010",
        "TC":    "A0E8F975843B4411",
        "CURRCODE":    "SGD",
        "FXRATE":    "",
        "FOREIGNAMT":    0,
        "DCCMSG":    "",
        "TENDER":    "00",
        "CMD":    "SALE",
        "TYPE":    "VMJ" 
}

#else
     memset(szWebServiceResponse, 0, sizeof(szWebServiceResponse));
     strcpy(szWebServiceResponse, "HTTP/1.1 301");
	 strcat(szWebServiceResponse, "\x0D\x0A");

	 //strcat(szWebServiceResponse, "User-Agent: Jakarta Commons-HttpClient/3.1");
     //strcat(szWebServiceResponse, "\x0D\x0A");

     #if 1
	 strcat(szWebServiceResponse, "Content-Length: 0");
	 strcat(szWebServiceResponse, "\x0D\x0A");
	 #else
     memset(szTemp, 0, sizeof(szTemp));
	 sprintf(szTemp, "%d", strlen(uszJsonString));
	 strcat(szWebServiceResponse, "Content-Length: ");
	 strcat(szWebServiceResponse, szTemp);
	 strcat(szWebServiceResponse, "\x0D\x0A");
     #endif

	 //strcat(szWebServiceResponse, "Location: http://www.bibsonomy.org/api/users/subwiz/posts");
	 strcat(szWebServiceResponse, "Location: Philippines");
     strcat(szWebServiceResponse, "\x0D\x0A");

	 strcat(szWebServiceResponse, "Connection: close");
	 strcat(szWebServiceResponse, "\x0D\x0A");
#endif

	 vdDebug_LogPrintf("szWebServiceResponse[%s]", szWebServiceResponse);
 
	 vdDebug_LogPrintf("ECR send offset[%d], isEcrProcessing=%d", offset, isEcrProcessing);

	
	//inECRSendComPacket(uszJsonString, strlen(uszJsonString), VS_FALSE, 5,VS_FALSE, 3);
	inECRSendComPacket(szWebServiceResponse, strlen(szWebServiceResponse), VS_FALSE, 5,VS_FALSE, 3);

	
    vdDebug_LogPrintf("inECRSendResponse End");
	isEcrProcessing = FALSE;
    TCP_ServerCloseSocket();

    return VS_SUCCESS;
}

int inGetAcqrHostIndex(int Index)
{
	char szAcquirerIndex[24+1];
    int inAcqIdx=0;
	
	memset(szAcquirerIndex, 0, sizeof(szAcquirerIndex));
	sprintf(szAcquirerIndex, "ACQ%02d_HOST_IDX", Index); /*ex. ACQ01_HOST_IDX*/
    inAcqIdx=get_env_int(szAcquirerIndex);
 
	vdDebug_LogPrintf("szAcquirerIndex: %s, inAcqIdx: %d",szAcquirerIndex, inAcqIdx);

    return inAcqIdx;	
}

//Get Terminal serial no.
void getTerminalSerialNo(BYTE* sNo)
{
        BYTE serialNo[17+1];
        
        int i,len;
        
        memset(serialNo, 0x00, sizeof(serialNo));
	memset(sNo, 0x00, sizeof(sNo));
           
	CTOS_CTMSGetConfig(d_CTMS_SERIALNUM, serialNo);
        if(strlen(serialNo) <= 0)
        {
	    CTOS_GetFactorySN(serialNo);
            
        }
	
        for (i=0; i<strlen(serialNo); i++)
	{
		if (serialNo[i] < 0x30 || serialNo[i] > 0x39)
		{
			serialNo[i] = 0;
			break;
		}
	}
	len = strlen(serialNo);
	strcpy(sNo,"0000000000000000");
	memcpy(&sNo[16-len],serialNo,len);
       

}


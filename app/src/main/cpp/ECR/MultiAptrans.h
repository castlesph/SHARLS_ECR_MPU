
#ifndef MULTIAPTRANS_H
#define MULTIAPTRANS_H

#define d_MAX_APP				25
#define	d_AP_FLAG_DEF_SEL_EX	0xC0

#define	d_SUCCESS				100
#define	d_FAIL					-101
#define	d_NOT_RECORD			-102

#define d_MAX_IPC_BUFFER		2048

typedef enum
{
	d_IPC_CMD_ECR_Initialize		    = 0x02,
	d_IPC_CMD_ECR_Event					= 0x03,
	d_IPC_CMD_ECR_SendResponse 			= 0x04,
    d_IPC_CMD_ECR_GetPackageData		= 0x08,
    d_IPC_CMD_ECR_SetPackageData	    = 0x09,
    
    d_IPC_CMD_BT_STATUSEx		    	= 0x97,
    d_IPC_CMD_SETFONT	    = 0x99,
}IPC_ECRCMD_TYPES;

typedef enum
{
    d_IPC_CMD_ONLINES_SALE		= 0x01,
	d_IPC_CMD_OFFLINE_SALE		= 0x02,
	d_IPC_CMD_VOID_SALE			= 0x03,
    d_IPC_CMD_PRE_AUTH			= 0x04,
    d_IPC_CMD_REFUND			= 0x05,
    d_IPC_CMD_TIP_ADJUST		= 0x06,
    d_IPC_CMD_SETTLEMENT		= 0x07,
    d_IPC_CMD_BATCH_REVIEW		= 0x08,
    d_IPC_CMD_BATCH_TOTAL		= 0x09,
    d_IPC_CMD_REPRINT_LAST		= 0x0A,
    d_IPC_CMD_REPRINT_ANY		= 0x0B,
    d_IPC_CMD_REPRINT_LAST_SETT	= 0x0C,
    d_IPC_CMD_SUMMARY_REPORT	= 0x0D,
    d_IPC_CMD_DETAIL_REPORT		= 0x0E,
    d_IPC_CMD_GET_PID			= 0x10,
    d_IPC_CMD_POWER_FAIL		= 0x11,
    d_IPC_CMD_DELETE_BATCH		= 0x12,
    d_IPC_CMD_DELETE_REVERSAL	= 0x13,
	d_IPC_CMD_SETTLE_ALL		= 0x14,
	d_IPC_CMD_WAVE_SALE 		= 0x16,
	d_IPC_CMD_WAVE_REFUND		= 0x17,
    //d_IPC_CMD_ECHO                  = 0x1F,
    d_IPC_CMD_ECHO              = 0x21, //changed value from 0x1F to 0x21.
    d_IPC_CMD_BARCODE           = 0X22,  
	d_IPC_CMD_PRINTING		    = 0x23,
	d_IPC_CMD_READCARD		    = 0x24,
    d_IPC_CMD_INSTALLMENT		= 0x2D,
	d_IPC_CMD_PRE_AUTH_COMP 		= 0x2F,
	d_IPC_CMD_TRX_RETRIEVAL                         = 0x52,
    d_IPC_CMD_VOIDPREAUTH       = 0x53,
}IPC_CMD_TYPES;

int inMultiAP_RunIPCCmdTypes(char *Appname, int IPC_EVENT_ID, BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen);
int inMultiAP_RunIPCCmdTypesEx(char *Appname, int IPC_EVENT_ID, BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen);

USHORT inMultiAP_HandleIPC(BYTE *inbuf, USHORT inlen, BYTE *outbuf, USHORT *outlen);
int inMultiAP_GetMainroutine(void);

int inMultiAP_GetMainroutineEx(BYTE *str , int *len);

int inCTOS_MultiAPALLAppEventID(int IPC_EVENT_ID);
int inCTOS_MultiAPBatchSearch(void);

#endif



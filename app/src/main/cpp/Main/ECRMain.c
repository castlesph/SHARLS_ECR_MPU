/*******************************************************************************

*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdarg.h>

//#include "../Includes/EMVTypedef.h"
#include "../Includes/Utils.h"
#include "../Includes/DataBaseFunc.h"
//#include "../Includes/EMVTrans.h"
#include "../debug/debug.h"
#include "../ECR/MultiAptrans.h"
#include "../ECR/ECRTrans.h"
#include "../Includes/myFileFunc.h"
#include "../Bluetooth/Bluetooth.h"

#define d_BLUETOOTH						6
extern BYTE ECRPort;

int poweron = 1;
BOOL initEcrComOk = FALSE;
int main(int argc,char *argv[])
{
    //---------------------------
    BYTE Device_num = 0;
    stDeviceInfo sDeviceInfo;
    stDeviceInfo *spDeviceInfo = &sDeviceInfo;
    BYTE DeviceName[] = "VEGA-3000";
    BYTE Mac[] = "98:52:B1:01:4C:9F";
    BYTE ServiceName[] = "BTInsecure";
	USHORT  usRtn,usResult = d_NO;
	BOOL fPortable;
	BOOL fPCI;
	USHORT mkHWSupport;
    unsigned char my_uuid[] = { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00,
         0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
    USHORT usMode;
	DWORD btStatus = 0;

	inSetTextMode();
    inTCTRead(1);

	usRtn = CTOS_HWSettingGet(&fPortable,&fPCI,&mkHWSupport);
	vdDebug_LogPrintf("CTOS_HWSettingGet,usRtn=[%x],fPortable[%d],fPCI=[%d],mkHWSupport=[%04x]", usRtn,fPortable,fPCI,mkHWSupport);

    //CTOS_Delay(3000);
	vdDebug_LogPrintf("-----Share ECR main-----"); 

	if(inECR_InitCOM() == d_OK)
    {
        initEcrComOk = TRUE;
    }
	vdDebug_LogPrintf("inWaitTime =[%d]",strTCT.inWaitTime);
	if (strTCT.inWaitTime <= 0)
		strTCT.inWaitTime = 150;

// patrick testing pack JSON message and unpack JSON message
//	cJSONtest();

	//while(1)
	//{
	//	vdTCP_TestServer();
	//}
	//return;
    while(0)
    {   
        inMultiAP_GetMainroutine();
		//CTOS_Delay(strTCT.inWaitTime);
		//CTOS_Delay(100);
		CTOS_Delay(50);
		vdDebug_LogPrintf("-----inCTOSS_CheckECREvent---byRS232ECRPort=[%d]-ECRPort=[%d]-",strTCT.byRS232ECRPort,ECRPort); 

//  cjeck sharls bt exist? if com port 6?
// {
#if 0
	if (6 ==strTCT.byRS232ECRPort)
	{
		//vdDebug_LogPrintf("mkHWSupport & d_MK_HW_BT=[%04x]", mkHWSupport & d_MK_HW_BT);
		if ((mkHWSupport & d_MK_HW_BT) == d_MK_HW_BT)
		{
			{
				CTOSS_BluetoothStatus(&btStatus);
				vdDebug_LogPrintf("CTOSS_BluetoothStatus,btStatus=[%x][%x][%x]",btStatus,(btStatus & 0x00000002),(btStatus & 0x00000004)); 
				if((btStatus & d_BLUETOOTH_STATE_CONNECTED) == d_BLUETOOTH_STATE_CONNECTED)	//d_BLUETOOTH_STATE_CONNECTED = 0x00000002
				{
					poweron = 0;
					//put_env_int("BTCONNECTED",1);//for sharls_com get the BT  connected
				}
				else
				{
					//put_env_int("BTCONNECTED",0);
					poweron = 1;
					if((btStatus & d_BLUETOOTH_STATE_LISTENING) != d_BLUETOOTH_STATE_LISTENING)
						CTOSS_BluetoothListen("MP200", "");
				}
			}
		}

		if (ECRPort == d_BLUETOOTH)
			inCTOSS_CheckECREvent();
	
	}
	else
		inCTOSS_CheckECREvent();
//        usleep(100*1000);
#endif	
    }
//	CTOS_BluetoothClose();
	
}


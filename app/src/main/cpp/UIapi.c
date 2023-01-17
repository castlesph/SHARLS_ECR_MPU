
#include <string.h>
#include <stdio.h>
#include <ctosapi.h>
//#include "FeliCa/Util.h"
#include "Debug/debug.h"
#include "patrick-lib.h"

USHORT CTOS_LCDTSetReverse(BOOL boReverse)
{
    return d_OK;
    BYTE szMultipleMsgStr[512];

    memset(szMultipleMsgStr, 0x00, sizeof(szMultipleMsgStr));
    strcpy(szMultipleMsgStr, "|");

    strcat(szMultipleMsgStr, "test11111");
    strcat(szMultipleMsgStr, "|");

    vdDebug_LogPrintf("CTOS_LCDTSetReverse.......");
//    inCallJAVA_DisplayMultipleMessage(szMultipleMsgStr);
}

USHORT CTOS_LCDTPrintAligned(USHORT usY, UCHAR* pbBuf, BYTE bMode)
{

    BYTE szMultipleMsgStr[512];

    memset(szMultipleMsgStr, 0x00, sizeof(szMultipleMsgStr));
    strcpy(szMultipleMsgStr, "|");

    strcat(szMultipleMsgStr, "test11111");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test22222");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test33333");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test44444");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test55555");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test66666");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test77777");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test88888");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test99999");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "test00000");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testaaaaa");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testbbbbb");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testccccc");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testddddd");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testeeeee");
    strcat(szMultipleMsgStr, "|");
    strcat(szMultipleMsgStr, "testfffff");
    strcat(szMultipleMsgStr, "|");
    vdDebug_LogPrintf("CTOS_LCDTPrintAligned.....");
//    inCallJAVA_DisplayMultipleMessage(szMultipleMsgStr);
}

USHORT CTOS_LCDSelectModeEx(BYTE bMode,BOOL fClear)
{
}

USHORT CTOS_LCDGShowBMPPic(USHORT usX, USHORT usY, BYTE *baFilename)
{
}

USHORT CTOS_LCDTPrintXY(USHORT usX, USHORT usY, UCHAR* pbBuf){
    vdDebug_LogPrintf("Dummy DISP FUNC [%s]", pbBuf);
}

USHORT CTOS_LCDTClearDisplay(void){

}

USHORT CTOS_LCDBackGndColor(ULONG ulColor){

}

USHORT CTOS_LCDForeGndColor(ULONG ulColor){

}

USHORT CTOS_LCDFontSelectMode(BYTE bMode){

}

USHORT CTOS_LCDGShowPic(USHORT usX, USHORT usY, BYTE* baPat, ULONG ulPatLen, USHORT usXSize){

}

USHORT CTOS_LCDTClear2EOL(void){

}

USHORT CTOS_LCDTGotoXY(USHORT usX,USHORT usY){

}

USHORT CTOS_LCDTPutchXY (USHORT usX, USHORT usY, UCHAR bChar){

}

USHORT CTOS_LCDTPrint(UCHAR* sBuf){

}

USHORT CTOS_LCDSetContrast(BYTE bValue){

}

USHORT CTOS_LCDTSelectFontSize(USHORT usFontSize){

}

USHORT CTOS_LCDTTFSwichDisplayMode(USHORT usMode){

}

USHORT CTOS_LCDTTFSelect(BYTE *baFilename, BYTE bIndex){

}

USHORT CTOS_LCDGShowPicEx(UCHAR bMode,USHORT usX, USHORT usY, BYTE* baPat, ULONG ulPatLen, USHORT usXSize)
{
}

USHORT CTOS_LCDSelectMode(BYTE bMode)
{
}

USHORT CTOS_LCDGSetBox(USHORT usX, USHORT usY, USHORT usXSize, USHORT usYSize, BYTE bFill)
{
}



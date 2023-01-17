#ifndef __UI_API_H__
#define __UI_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ctosapi.h>

USHORT CTOS_LCDTSetReverse(BOOL boReverse);
USHORT CTOS_LCDTPrintAligned(USHORT usY, UCHAR* pbBuf, BYTE bMode);

USHORT CTOS_LCDSelectModeEx(BYTE bMode,BOOL fClear);
USHORT CTOS_LCDGShowBMPPic(USHORT usX, USHORT usY, BYTE *baFilename);
USHORT CTOS_LCDTPrintXY(USHORT usX, USHORT usY, UCHAR* pbBuf);
USHORT CTOS_LCDTClearDisplay(void);
USHORT CTOS_LCDTTFSwichDisplayMode(USHORT usMode);
USHORT CTOS_LCDTTFSelect(BYTE *baFilename, BYTE bIndex);
USHORT CTOS_LCDTPrintAligned(USHORT usY, UCHAR* pbBuf, BYTE bMode);
USHORT CTOS_LCDBackGndColor(ULONG ulColor);
USHORT CTOS_LCDForeGndColor(ULONG ulColor);
USHORT CTOS_LCDFontSelectMode(BYTE bMode);
USHORT CTOS_LCDGShowPic(USHORT usX, USHORT usY, BYTE* baPat, ULONG ulPatLen, USHORT usXSize);
USHORT CTOS_LCDTClear2EOL(void);
USHORT CTOS_LCDTGotoXY(USHORT usX,USHORT usY);
USHORT CTOS_LCDTPutchXY (USHORT usX, USHORT usY, UCHAR bChar);
USHORT CTOS_LCDTPrint(UCHAR* sBuf);
USHORT CTOS_LCDTSetReverse(BOOL boReverse);
USHORT CTOS_LCDSetContrast(BYTE bValue);
USHORT CTOS_LCDTSelectFontSize(USHORT usFontSize);


#ifdef __cplusplus
}
#endif

#endif


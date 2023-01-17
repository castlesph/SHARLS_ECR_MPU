/* 
 * File:   debug.h
 * Author: PeyJiun
 *
 */

#ifndef _DEBUG_H
#define	_DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "typedef.h"

/****************
* if bPort == 0xFF --> USB mode
****************/
void SetDebugMode(BYTE bMode, BYTE bPort);
void DebugInit(void);
void DebugAddHEX(BYTE *title, BYTE *hex, USHORT len);
void DebugAddINT(BYTE *title, LONG value);
void DebugAddIntX(BYTE *title, LONG value);
void DebugAddSTR(BYTE *title, BYTE *baMsg, USHORT len);
//void vdDebug_LogPrintf(const char* fmt, ...);
BYTE byGetDebugMode(void);
void vdPCIDebug_HexPrintf(char *display,char *szHexbuf, int len);

void PrintDebugMessage(const char* filePath, int lineNumber, const char * functionName, const char* format, ...);

//#define vdDebug_LogPrintf(...)

#define vdDebug_LogPrintf(...)  PrintDebugMessage(__FILE__, __LINE__,__FUNCTION__, __VA_ARGS__)

//#define vdDebug_LogPrintf(...)

//#define LOG_PRINTF((...)) vdDebug_LogPrintf(...)

void vdPinPadDebugStatus(int inStatus, char *pszMsg, int inLine);
void vdDebug_PrintOnPaper(const char* format, ...);
void vdPCIDebug_HexPrintfEx(char *display,char *szHexbuf, int len);

#if _DEBUG_LOG_ == 1

void DebugInit(void);
void DebugAddHEX(BYTE *title, BYTE *hex, USHORT len);
void DebugAddINT(BYTE *title, ULONG value);
void DebugAddSTR(BYTE *title);

#define __DebugInit						DebugInit
#define __DebugAddHEX					DebugAddHEX
#define __DebugAddINT					DebugAddINT
#define __DebugAddSTR					DebugAddSTR

#else

#define __DebugInit()
#define __DebugAddHEX(title, hex, len)
#define __DebugAddINT(title, value)
#define __DebugAddSTR(title)

#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _DEBUG_H */


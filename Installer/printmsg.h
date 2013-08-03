#ifndef _PRINTMSG_H_
#define _PRINTMSG_H_

#include "cmnhdr.h"


typedef VOID (*OutputRoutine)(LPCWSTR lpOutputString);
VOID StdoutPrint(LPCWSTR lpOutputString);
VOID StderrPrint(LPCWSTR lpOutputString);
VOID DebugWindowPrint(LPCWSTR lpOutputString);

VOID InitializeMessagePrintModule(BOOL bThreadSafe, OutputRoutine fnOutputRoutine);
VOID CleanupMessagePrintModule(VOID);

#define SYSERROR(msg) PrintSystemError(__WFILE__, __LINE__, (msg), GetLastError(), TRUE)
#define WSAERROR(msg) PrintSystemError(__WFILE__, __LINE__, (msg), WSAGetLastError(), TRUE)
#define USERERROR(msg) PrintSystemError(__WFILE__, __LINE__, (msg), 0, FALSE)

VOID PrintMessage(LPCWSTR lpFormat, ...);
VOID PrintSystemError(LPCWSTR lpFileName, 
					                       DWORD dwLineNumber, 
					                       LPCWSTR lpUserMessage, 
                                           DWORD dwErrorCode,
					                       BOOL bOutputSystemError);

#endif  /* _PRINTMSG_H_ */
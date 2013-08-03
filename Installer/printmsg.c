#include "printmsg.h"
#include <stdio.h>
#include <stdarg.h>

 static CRITICAL_SECTION g_csOutput;
 static BOOL g_bThreadSafe = FALSE;
 static VOID StdoutPrint(LPCWSTR lpOutputString);
 static OutputRoutine g_fnOutputRoutine = StdoutPrint;

VOID InitializeMessagePrintModule(BOOL bThreadSafe, OutputRoutine fnOutputRoutine)
 {
     if (g_bThreadSafe = bThreadSafe) {
         InitializeCriticalSection(&g_csOutput);
	 }
     g_fnOutputRoutine = (fnOutputRoutine ? fnOutputRoutine : g_fnOutputRoutine);
 }

 VOID CleanupMessagePrintModule(VOID)
 {
    if (g_bThreadSafe)
        DeleteCriticalSection(&g_csOutput);
 }

 VOID PrintMessage(LPCWSTR lpFormat, ...)
 {
	 TCHAR szBuffer[4096];
	 int cch = 0;
	 va_list vlArgs;

	 va_start(vlArgs, lpFormat);
	 cch = _vsnwprintf(szBuffer, _countof(szBuffer) - 1, lpFormat, vlArgs);
	 va_end(vlArgs);

	 if (cch < 0 || cch == (_countof(szBuffer) - 1))
		 szBuffer[_countof(szBuffer) - 1] = L'\0';

     if (g_bThreadSafe) {
        EnterCriticalSection(&g_csOutput);
        g_fnOutputRoutine(szBuffer);
        LeaveCriticalSection(&g_csOutput);
	 } else {
         g_fnOutputRoutine(szBuffer);
	 }
 }

 VOID PrintSystemError(LPCWSTR lpFileName, 
	                                        DWORD dwLineNumber, 
	                                        LPCWSTR lpUserMessage, 
	                                        DWORD dwError,
	                                        BOOL bOutputSystemError)
 {
	 WCHAR szSystemError[256];
	 LPCWSTR lpBaseName = L"";

	 if (lpFileName) {
		 PCWSTR p = NULL;
		 if ((p = wcsrchr(lpFileName, L'\\')) || (p = wcsrchr(lpFileName, L'/'))) {
			 lpBaseName = p + 1;
		 } else {
			 lpBaseName = lpFileName;
		 }
	 }

	 if (bOutputSystemError) {
		 if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,        
			 NULL,
			 dwError,
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			 szSystemError,
			 _countof(szSystemError),
			 NULL) == 0)
		 {
			 _snwprintf(szSystemError, _countof(szSystemError), L"Last error number: %d", dwError);
		 }
	 }

	 if (lpUserMessage && bOutputSystemError) {
		 PrintMessage( L"(%s:%lu) %s: %s\n", lpBaseName, dwLineNumber, lpUserMessage, szSystemError);
	 } else if (lpUserMessage) {
		 PrintMessage(L"(%s:%lu) %s\n", lpBaseName, dwLineNumber, lpUserMessage);
	 } else if (bOutputSystemError) {
		 PrintMessage( L"(%s:%lu) %s\n", lpBaseName, dwLineNumber, szSystemError);
	 } else {
		 /* print nothing */
	 }
 }


VOID StdoutPrint(LPCWSTR lpOutputString)
{
    wprintf(L"%s", lpOutputString);
}

VOID StderrPrint(LPCWSTR lpOutputString)
{
	fwprintf(stderr, L"%s", lpOutputString);
}

VOID DebugWindowPrint(LPCWSTR lpOutputString)
{
    OutputDebugString(lpOutputString);
}

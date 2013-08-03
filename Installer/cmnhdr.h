#ifndef _CMNHDR_H_
#define _CMNHDR_H_

//#define	_WIN32_WINNT _WIN32_WINNT_WINXP

#include <WinSock2.h>
#include <windows.h>
#include <wchar.h>

typedef int (*CompareFunction)(LPCVOID, LPCVOID);

/* widen string literal */
#define _WIDENSTR(x) L ## x
#define STRTOWSTR(x) _WIDENSTR(x)

#define __WFILE__ STRTOWSTR(__FILE__)

#endif /* _CMNHDR_H_ */

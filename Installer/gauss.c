#include "gauss.h"
#include "catalog.h"
#include <assert.h>
#include "mem.h"
#include "printmsg.h"
#include <string.h>
#include <stdio.h>
#include "array.h"
#include <WS2spi.h>
#include <SpOrder.h>

const GUID g_GaussTcpIpGuid = {
	0x13f789d3,
	0x5b13,
	0x462f,
	{0x9d, 0x79, 0xeb, 0xe7, 0x94, 0xdc, 0x3b, 0xf }
};

const GUID g_GaussLayeredGuid = {
	0x30ea9dae,
	0xdabf, 
	0x4fb4,
	{0x97, 0xcb, 0xf5, 0xd8, 0x88, 0x35, 0xc2, 0xa7}
};


static BOOL CheckFileExist(LPCWSTR pFilePath);
static BOOL GaussInstallLayeredProtocol(LPCWSTR pGaussDllPath);
static BOOL GaussInstallProtocolChains(LPCWSTR pGaussDllPath);
static BOOL GaussReorderCatalog(VOID);

BOOL GaussGetInstallationPath(WCHAR szInstallationPath[], DWORD nCount)
{
    BOOL bWow64 = FALSE;
    WCHAR lpDllDirectory[MAX_PATH];
    WCHAR szBuffer[MAX_PATH];
    DWORD nTryCount = 0;

    if (IsWow64Process(GetCurrentProcess(), &bWow64) == 0) {
        SYSERROR(L"IsWow64Process");
        return FALSE;
	}

    if (bWow64) {
        ExpandEnvironmentStrings(L"%SystemRoot%\\SysWOW64\\",
                                                           lpDllDirectory, MAX_PATH);
	} else {
		ExpandEnvironmentStrings(L"%SystemRoot%\\System32\\",
			                                              lpDllDirectory, MAX_PATH);
	}

    for (; nTryCount < GAUSS_POSSIBLE_FILE_NAME_COUNT; nTryCount++) {
        _snwprintf(szBuffer, MAX_PATH, L"%sgauss.%d.dll", lpDllDirectory, nTryCount);
        if (!CheckFileExist(szBuffer)) {
			if (nCount < wcslen(szBuffer) + 1) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                SYSERROR(L"GaussGetInstallationPath");
                return FALSE;
			} else {
                wcscpy(szInstallationPath, szBuffer);
                return TRUE;
			}
		}
	}

    USERERROR(L"all possible file names had  already been occupied");
    return FALSE;
}

BOOL GaussInstall(VOID)
{
    WCHAR szInstallationPath[MAX_PATH];
    PWCHAR p;
    WCHAR szCRTPath[MAX_PATH];

    if (!GaussGetInstallationPath(szInstallationPath, MAX_PATH))
        return FALSE;

	wcscpy(szCRTPath, szInstallationPath);
	p = wcsrchr(szCRTPath, L'\\');
	*p = 0;
	wcscat(szCRTPath, L"/msvcr110.dll");
	if (!CheckFileExist(szCRTPath)) {
		if (!CheckFileExist(L"msvcr110.dll")) {
			USERERROR(L"msvcr110.dll not found");
			return FALSE;
		}
		if (!CopyFile(L"msvcr110.dll", szCRTPath, TRUE)) {
			SYSERROR(L"CopyFile");
			return FALSE;
		}
	}

    if (!CheckFileExist(L"gauss.dll")) {
        USERERROR(L"gauss.dll not found");
        return FALSE;
	}

    if (!CopyFile(L"gauss.dll", szInstallationPath, TRUE)) {
        SYSERROR(L"CopyFile");
        return FALSE;
	}

    if (!GaussCatalogCleanup())
        goto cleanup;

    if (!GaussInstallLayeredProtocol(szInstallationPath))
        goto installfailed;

    if (!GaussInstallProtocolChains(szInstallationPath))
        goto installfailed;

    if (!GaussReorderCatalog())
        goto installfailed;

    return TRUE;

installfailed:
    CatalogCleanup();

cleanup:
    if (!DeleteFile(szInstallationPath))
        SYSERROR(L"DeleteFile");

    return FALSE;
}

BOOL GaussCatalogCleanup(VOID)
{
    /*
	WCHAR szGaussDllPath[MAX_PATH];
	WCHAR szExpandedGaussDllPath[MAX_PATH];
	int iGaussDllPathLen = _countof(szGaussDllPath);
	int iErrno, rc;
    */
	BOOL bOK = TRUE;

    /*
	rc = WSCGetProviderPath((LPGUID)&g_GaussLayeredGuid,
		                                             szGaussDllPath,
		                                             &iGaussDllPathLen,
		                                             &iErrno);     
	if (rc == 0) {
		ExpandEnvironmentStrings(szGaussDllPath, szExpandedGaussDllPath, MAX_PATH);
	} else if (iErrno != WSAEINVAL) {
        PrintSystemError(__WFILE__, __LINE__, L"WSCGetProviderPath", iErrno, TRUE);
	}
    */

	bOK = CatalogCleanup();

    /*
	if (bOK && rc == 0)
		MoveFileEx(szExpandedGaussDllPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        */

	return bOK;
}

BOOL VerifyGaussInstallation(Array catalog)
{
     LPWSAPROTOCOL_INFO apGaussTcpIpProtocols[3];
     LPWSAPROTOCOL_INFO pGaussLayeredProtocol;
     LPWSAPROTOCOL_INFO apMSTcpIpProtocols[3];
     int i;

     FindGaussProtocols(catalog, &pGaussLayeredProtocol,
		                                  &apGaussTcpIpProtocols[0],
		                                  &apGaussTcpIpProtocols[1],
		                                  &apGaussTcpIpProtocols[2]);

     if (!pGaussLayeredProtocol) {
         USERERROR(L"missing Gauss layered protocol");
         return FALSE;
	 }

     FindMSTcpIpProtocols(catalog, &apMSTcpIpProtocols[0], 
		                                        &apMSTcpIpProtocols[1], 
		                                        &apMSTcpIpProtocols[2]);

     for (i = 0; i < 3; i++) {
         if (apMSTcpIpProtocols[i] == NULL) {
             USERERROR(L"missing one or more Microsoft tcpip protocols");
             return FALSE;
		 }

		 if (apGaussTcpIpProtocols[i] == NULL) {
			 USERERROR(L"missing one or more Gauss tcpip protocols");
            return FALSE;
		 }

         if (apGaussTcpIpProtocols[i]->ProtocolChain.ChainEntries[1] != apMSTcpIpProtocols[i]->dwCatalogEntryId) {
             USERERROR(L"Gauss does not lay over Microsoft TcpIp");
             return FALSE; 
		 }
	 }

     return TRUE;
}

VOID FindGaussProtocols(Array catalog,
						                        LPWSAPROTOCOL_INFO *ppGaussLayeredProtocol,
                                                LPWSAPROTOCOL_INFO *ppGaussTcpProtocol,
						                        LPWSAPROTOCOL_INFO *ppGaussUdpProtocol,
						                        LPWSAPROTOCOL_INFO *ppGaussRawIpProtocol)
{
    int n = PROTOCOL_COUNT(catalog);
    int i;

    if (ppGaussLayeredProtocol)
        *ppGaussLayeredProtocol = NULL;

    if (ppGaussTcpProtocol)
        *ppGaussTcpProtocol = NULL;

    if (ppGaussUdpProtocol) 
        *ppGaussUdpProtocol = NULL;

    if (ppGaussRawIpProtocol)
        *ppGaussRawIpProtocol = NULL;

    for (i = 0; i < n; i++) 
	{
        LPWSAPROTOCOL_INFO pProtocol = PROTOCOL_NTH(catalog, i);

        if (IS_BASE_PROTOCOL(pProtocol))
            continue;

        if (IS_LAYERED_PROTOCOL(pProtocol) && 
			IsEqualGUID(&g_GaussLayeredGuid, &pProtocol->ProviderId)) 
		{
            if (ppGaussLayeredProtocol)
                *ppGaussLayeredProtocol = pProtocol;
		} 
		else if (IS_PROTOCOL_CHAIN(pProtocol) && 
			         IsEqualGUID(&g_GaussTcpIpGuid, &pProtocol->ProviderId))
		{
			switch (pProtocol->iSocketType)
			{
			case SOCK_STREAM:
				if (ppGaussTcpProtocol) {
					*ppGaussTcpProtocol = pProtocol;
					break;
				}
			case SOCK_DGRAM:
				if (ppGaussUdpProtocol) {
					*ppGaussUdpProtocol = pProtocol;
					break;
				}
			case  SOCK_RAW:
				if (ppGaussRawIpProtocol) {
					*ppGaussRawIpProtocol = pProtocol;
				}
			default:
				break;
			}
		}
	}
}

static BOOL GaussInstallLayeredProtocol(LPCWSTR pGaussDllPath)
{
    WSAPROTOCOL_INFO piGaussLayeredProtocol;
    LPWSAPROTOCOL_INFO lpMSTcpProtocol;
    Array catalog;
    BOOL bOK = TRUE;
    int iErrno, rc;

	if (!(catalog = CatalogNew()))
        return FALSE;
        
	FindMSTcpIpProtocols(catalog, &lpMSTcpProtocol, NULL, NULL);
    if (lpMSTcpProtocol == NULL) {
        USERERROR(L"Microsoft tcp protocol not found");
        bOK = FALSE;
        goto cleanup;
	}

    memcpy(&piGaussLayeredProtocol, lpMSTcpProtocol, sizeof(WSAPROTOCOL_INFO));
    piGaussLayeredProtocol.dwServiceFlags1 &= (~XP1_IFS_HANDLES);
	piGaussLayeredProtocol.iSocketType = 0;
	piGaussLayeredProtocol.iProtocol = 0;
	piGaussLayeredProtocol.dwProviderFlags |= PFL_HIDDEN;
	piGaussLayeredProtocol.dwProviderFlags &= (~PFL_MATCHES_PROTOCOL_ZERO);
	piGaussLayeredProtocol.ProtocolChain.ChainLen = LAYERED_PROTOCOL;
	wcscpy(piGaussLayeredProtocol.szProtocol, GAUSS_LSP_NAME);

	rc = WSCInstallProvider((LPGUID)&g_GaussLayeredGuid,
		                                        pGaussDllPath,
		                                        &piGaussLayeredProtocol,
		                                        1,
		                                        &iErrno);
	if (rc == SOCKET_ERROR) {
		PrintSystemError(__WFILE__, __LINE__, L"Install gauss layered protocol failed",
                                        iErrno, TRUE);
        bOK = FALSE;
        goto cleanup;
	}

cleanup:
    ArrayFree(catalog);

    return bOK;
}

static BOOL GaussInstallProtocolChains(LPCWSTR pGaussDllPath)
{
    Array catalog;
    LPWSAPROTOCOL_INFO pGaussLayeredProtocol;
    /******************************************************
     * 0: tcp protocol
     * 1: udp protocol
     * 2: rawip protocol
     ******************************************************/
    LPWSAPROTOCOL_INFO apMSTcpIpProtocol[3];
    WSAPROTOCOL_INFO apiGaussTcpIpProtocol[3];
    BOOL bOK = TRUE;
    int i;
    int iErrno, rc;

    if (!(catalog = CatalogNew()))
        return FALSE;

    FindGaussProtocols(catalog, &pGaussLayeredProtocol, NULL, NULL, NULL);
    if (pGaussLayeredProtocol == NULL) {
        USERERROR(L"Gauss layered protocol must be installed first");
        bOK = FALSE;
        goto cleanup;
	}

    FindMSTcpIpProtocols(catalog, &apMSTcpIpProtocol[0], &apMSTcpIpProtocol[1],
		                                       &apMSTcpIpProtocol[2]);
    if (apMSTcpIpProtocol[0] == NULL ||
         apMSTcpIpProtocol[1] == NULL ||
         apMSTcpIpProtocol[2] == NULL) {
        USERERROR(L"Missing one or more Microsoft tcpip protocols");
        bOK = FALSE;
        goto cleanup;
	}

    for (i = 0; i < 3; i++) {
        memcpy(&apiGaussTcpIpProtocol[i], apMSTcpIpProtocol[i], sizeof(WSAPROTOCOL_INFO));
        _snwprintf(apiGaussTcpIpProtocol[i].szProtocol, WSAPROTOCOL_LEN + 1, L"%s over [%s]", 
			               GAUSS_LSP_NAME, apMSTcpIpProtocol[i]->szProtocol);
        apiGaussTcpIpProtocol[i].ProtocolChain.ChainEntries[0] = pGaussLayeredProtocol->dwCatalogEntryId;
        apiGaussTcpIpProtocol[i].ProtocolChain.ChainEntries[1] = apMSTcpIpProtocol[i]->dwCatalogEntryId;
        apiGaussTcpIpProtocol[i].ProtocolChain.ChainLen = 2;
        apiGaussTcpIpProtocol[i].dwServiceFlags1 &= (~XP1_IFS_HANDLES);
	}

	rc = WSCInstallProvider((LPGUID)&g_GaussTcpIpGuid,
		                                        pGaussDllPath,
		                                        apiGaussTcpIpProtocol,
		                                        3,
		                                        &iErrno);

	if (rc == SOCKET_ERROR) {
		PrintSystemError(__WFILE__, __LINE__, L"Install gauss protocol chain failed",
			iErrno, TRUE);
		bOK = FALSE;
		goto cleanup;
	}

cleanup:
    ArrayFree(catalog);
    
    return bOK;
}

static BOOL GaussReorderCatalog(VOID)
{
    Array catalog;
    int nProtocols;
    LPWSAPROTOCOL_INFO pGaussLayeredProtocol;
    LPWSAPROTOCOL_INFO pGaussTcpProtocol;
    LPWSAPROTOCOL_INFO pGaussUdpProtocol;
    LPWSAPROTOCOL_INFO pGaussRawIpProtocol;
    Array aEntryIds = NULL;
    BOOL bOK = TRUE;
    int i, j;
    int rc;

    if (!(catalog = CatalogNew()))
        return FALSE;

    nProtocols = PROTOCOL_COUNT(catalog);

    FindGaussProtocols(catalog, &pGaussLayeredProtocol, &pGaussTcpProtocol,
		                                 &pGaussUdpProtocol, &pGaussRawIpProtocol);

    if (pGaussLayeredProtocol == NULL || pGaussTcpProtocol == NULL ||
		 pGaussUdpProtocol == NULL || pGaussRawIpProtocol == NULL) {
        USERERROR(L"missing one or more gauss protocols");
        bOK = FALSE;
        goto cleanup;
	}

    if (!(aEntryIds = ArrayNew(nProtocols, sizeof(DWORD)))) {
        bOK = FALSE;
        goto cleanup;
	}

    ArrayPut(aEntryIds, 0, &pGaussTcpProtocol->dwCatalogEntryId);
    ArrayPut(aEntryIds, 1, &pGaussUdpProtocol->dwCatalogEntryId);
    ArrayPut(aEntryIds, 2, &pGaussRawIpProtocol->dwCatalogEntryId);
    ArrayPut(aEntryIds, aEntryIds->iLength - 1, &pGaussLayeredProtocol->dwCatalogEntryId);

    for (i = 0, j = 3; i < nProtocols && j < (nProtocols - 1); i++) {
        LPWSAPROTOCOL_INFO pProtocol = PROTOCOL_NTH(catalog, i);
        if (IS_BASE_PROTOCOL(pProtocol))
            ArrayPut(aEntryIds, j++, &pProtocol->dwCatalogEntryId);
	}

    rc = WSCWriteProviderOrder((DWORD *)aEntryIds->pStorage, nProtocols);
	if (rc != ERROR_SUCCESS) {
        PrintSystemError(__WFILE__, __LINE__, L"WSCWriteProviderOrder", rc, TRUE);
        bOK = FALSE;
	}

cleanup:
    ArrayFree(catalog);
    if (aEntryIds)
        ArrayFree(aEntryIds);

    return bOK;
}

static BOOL CheckFileExist(LPCWSTR pFilePath)
{
	FILE *pFile = _wfopen(pFilePath, L"rb");
	if(pFile) {
		fclose(pFile);
        return TRUE;
	} else {
        return FALSE;
	}
}
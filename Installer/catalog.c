#include "catalog.h"
#include "printmsg.h"
#include <Ws2spi.h>
#include <objbase.h> /* for StringFromGUID2 */
#include "netutils.h"
#include "array.h"

const GUID g_MSTcpIpGuid = {
	0xE70F1AA0, 
	0xAB8B, 
	0x11CF, 
	{0x8C, 0xA3, 0x00, 0x80, 0x5F, 0x48, 0xA1, 0x92}
};

static int CompareGUID(const void *guid1, const void *guid2);

Array CatalogNew(void)
{
    DWORD dwBufferLength = 0;
    int iErrno;
    int nProtocols;
    Array catalog = NULL;

    nProtocols = WSCEnumProtocols(NULL, NULL, &dwBufferLength, &iErrno);

    if (nProtocols == SOCKET_ERROR && iErrno != WSAENOBUFS) {
        WSAERROR(L"WSCEnumProtocols");
        return NULL;
	}

    catalog = ArrayNew(dwBufferLength, 1);
    if (!catalog) {
        USERERROR(L"Memory allocation for catalog failed\n");
        return NULL;
	}

    nProtocols = WSCEnumProtocols(NULL, PROTOCOL_ARRAY(catalog), 
		                                                          &dwBufferLength, &iErrno);

     if (nProtocols == SOCKET_ERROR) {
         WSAERROR(L"WSCEnumProtocols");
         ArrayFree(catalog);
         return NULL;
	 }

     catalog->iLength = nProtocols;
     catalog->cbElementSize = sizeof(WSAPROTOCOL_INFO);

    return catalog;
}

/* Remove layered protocol and protocol chain */
BOOL CatalogCleanup(VOID)
{
    Array catalog = NULL;
    Array aGuids = NULL;
    int i, j, k, n;
    BOOL bOK = TRUE; 
	int rc, iErrno;

    if (!(catalog = CatalogNew())) {
        return FALSE;
	}

    n = PROTOCOL_COUNT(catalog);

    if (!(aGuids = ArrayNew(n, sizeof(GUID)))) {
        USERERROR(L"Alloc memory for guid array failed");
        bOK = FALSE;
        goto cleanup;
	}

    for (i = 0, j = 0; i < n; i++) {
        LPWSAPROTOCOL_INFO pProtocol = PROTOCOL_NTH(catalog, i);

        if (IS_LAYERED_PROTOCOL(pProtocol)) {
             if (ArrayLinearSearch(aGuids, &pProtocol->ProviderId, CompareGUID) < 0)
                 ArrayPut(aGuids, j++, &pProtocol->ProviderId);
		} else if (IS_PROTOCOL_CHAIN(pProtocol)) {
             if (ArrayLinearSearch(aGuids, &pProtocol->ProviderId, CompareGUID) < 0) {
                 ArrayShift(aGuids, 0, j++, 1);
                 ArrayPut(aGuids, 0, &pProtocol->ProviderId);
			 }
		}
	}

    for (k = 0; k < j; k++) {
        LPGUID pGuid = ArrayGet(aGuids, k);
        rc = WSCDeinstallProvider(pGuid, &iErrno);
		if (rc != 0 && iErrno != WSAEINVAL) {
            PrintSystemError(__WFILE__, __LINE__, L"WSCDeinstallProvider", iErrno, TRUE);
            bOK = FALSE;
            goto cleanup;
		}
	}

cleanup:
    ArrayFree(catalog);
    if (aGuids)
        ArrayFree(aGuids);

    return bOK;
}

VOID CatalogDump(Array catalog)
{
    int i;

    for (i = 0; i < PROTOCOL_COUNT(catalog); i++) {
        ProtocolDump(PROTOCOL_NTH(catalog, i));
	}
}

VOID ProtocolDump(LPWSAPROTOCOL_INFO pProtocol)
{
    TCHAR szBuffer[MAX_PATH];
    int iBufferLen = MAX_PATH - 1;
    int iErrno;
    int i;

    if (WSCGetProviderPath(&pProtocol->ProviderId,
		                                         szBuffer,
												 &iBufferLen,
												 &iErrno) != 0)
	{
        _snwprintf(szBuffer, MAX_PATH - 1, L"(error)");
	}

    PrintMessage(L"\nProtocol: %s\n", pProtocol->szProtocol);
    PrintMessage(L"==============================\n");
    PrintMessage(L"%27s: %s\n", L"Path", szBuffer);
    PrintMessage(L"%27s: %s\n", L"Address Family", 
		                      AddressFamilyToString(pProtocol->iAddressFamily));
    
    if (pProtocol->iAddressFamily == AF_INET ||
		 pProtocol->iAddressFamily == AF_INET6) {
        PrintMessage(L"%27s: %s\n", L"Protocol", IPProtocolToString(pProtocol->iProtocol));
	} else {
        PrintMessage(L"%27s: UNKNOWN: %d\n", L"Protocol", pProtocol->iProtocol);
	}

    PrintMessage(L"%27s: %s\n", L"Socket Type", SockTypeToString(pProtocol->iSocketType));

#define YESORNO(x) (pProtocol->dwServiceFlags1 & (x) ? L"YES" : L"NO")
#define ROOTEDORNON(x) (pProtocol->dwServiceFlags1 & (x) ? L"ROOTED" : L"NON-ROOTED")
    PrintMessage(L"%27s: %s\n", L"Connectionless", YESORNO(XP1_CONNECTIONLESS));
    PrintMessage(L"%27s: %s\n", L"Guaranteed Delivery", YESORNO(XP1_GUARANTEED_DELIVERY));
    PrintMessage(L"%27s: %s\n", L"Guaranteed Order", YESORNO(XP1_GUARANTEED_ORDER));
    PrintMessage(L"%27s: %s\n", L"Message Oriented", YESORNO(XP1_MESSAGE_ORIENTED));
    PrintMessage(L"%27s: %s\n", L"Pseudo Stream", YESORNO(XP1_PSEUDO_STREAM));
    PrintMessage(L"%27s: %s\n", L"Graceful Close", YESORNO(XP1_GRACEFUL_CLOSE));
    PrintMessage(L"%27s: %s\n", L"Expedited Data", YESORNO(XP1_EXPEDITED_DATA));
    PrintMessage(L"%27s: %s\n", L"Connect Data", YESORNO(XP1_CONNECT_DATA));
    PrintMessage(L"%27s: %s\n", L"Disconnect Data", YESORNO(XP1_DISCONNECT_DATA));
    PrintMessage(L"%27s: %s\n", L"Supports Broadcast", YESORNO(XP1_SUPPORT_BROADCAST));
    PrintMessage(L"%27s: %s\n", L"Supports Multipoint", YESORNO(XP1_SUPPORT_MULTIPOINT));
    PrintMessage(L"%27s: %s\n", L"Multipoint Control Plane", ROOTEDORNON(XP1_MULTIPOINT_CONTROL_PLANE));
    PrintMessage(L"%27s: %s\n", L"Multipoint Data Plane", ROOTEDORNON(XP1_MULTIPOINT_DATA_PLANE));
    PrintMessage(L"%27s: %s\n", L"QoS Supported", YESORNO(XP1_QOS_SUPPORTED));
    PrintMessage(L"%27s: %s\n", L"Unidirectional Sends", YESORNO(XP1_UNI_SEND));
    PrintMessage(L"%27s: %s\n", L"Unidirection Receives", YESORNO(XP1_UNI_RECV));
    PrintMessage(L"%27s: %s\n", L"IFS Handles", YESORNO(XP1_IFS_HANDLES));
    PrintMessage(L"%27s: %s\n", L"Partial Messages", YESORNO(XP1_PARTIAL_MESSAGE));
    PrintMessage(L"%27s: %s\n", L"IFS Handles", YESORNO(XP1_IFS_HANDLES));
#undef YESORNO
#undef ROOTEDORNON

    PrintMessage(L"%27s: ", L"Provider Flags");
    if (pProtocol->dwProviderFlags & PFL_MULTIPLE_PROTO_ENTRIES)
        PrintMessage(L"%s ", L"PFL_MULTIPLE_PROTO_ENTRIES");
	if (pProtocol->dwProviderFlags & PFL_RECOMMENDED_PROTO_ENTRY)
        PrintMessage(L"%s ", L"PFL_RECOMMENDED_PROTO_ENTRY");
	if (pProtocol->dwProviderFlags & PFL_HIDDEN)
        PrintMessage(L"%s ", L"PFL_HIDDEN");
	if (pProtocol->dwProviderFlags & PFL_MATCHES_PROTOCOL_ZERO)
    PrintMessage(L"%s ", L"PFL_MATCHES_PROTOCOL_ZERO");
#undef PROVIDERFLAG
	if (pProtocol->dwProviderFlags == 0) {
		PrintMessage(L"NONE");
	}
    PrintMessage(L"\n");

    StringFromGUID2( &pProtocol->ProviderId, szBuffer, MAX_PATH - 1);
	PrintMessage(L"%27s: %s\n", L"Provider Id", szBuffer);

	PrintMessage(L"%27s: %lu\n", L"Catalog Entry Id", pProtocol->dwCatalogEntryId);

	PrintMessage(L"%27s: %d  {", L"Number of Chain Entries", 
		                    pProtocol->ProtocolChain.ChainLen);
	for(i=0; i < pProtocol->ProtocolChain.ChainLen; i++)
		PrintMessage(L"%lu ", pProtocol->ProtocolChain.ChainEntries[i]);
    PrintMessage(L"}\n");

	PrintMessage(L"%27s: %d\n", L"Version", pProtocol->iVersion);
	PrintMessage(L"%27s: %d\n", L"Max Socket Address Length", 
		                    pProtocol->iMaxSockAddr);
	PrintMessage(L"%27s: %d\n", L"Min Socket Address Length", 
		                    pProtocol->iMinSockAddr);
	PrintMessage(L"%27s: %d\n", L"Protocol Max Offset", 
		                    pProtocol->iProtocolMaxOffset);

	PrintMessage( L"%27s: %s\n", L"Network Byte Order", 
            pProtocol->iNetworkByteOrder == 0 ? L"BIG-ENDIAN" : L"LITLE-ENDIAN");

	PrintMessage(L"%27s: ", L"Security Scheme");
    if (pProtocol->iSecurityScheme == SECURITY_PROTOCOL_NONE)
        PrintMessage(L"NONE\n");
	else
        PrintMessage(L"%d\n", pProtocol->iSecurityScheme);
		                  
    PrintMessage(L"%27s: ", L"Message Size");
	if (pProtocol->dwMessageSize == 0)
		PrintMessage(L"N/A (Stream Oriented)\n");
	else if (pProtocol->dwMessageSize == 1)
		PrintMessage(L"Depended on underlying MTU\n");
	else if (pProtocol->dwMessageSize == 0xFFFFFFFF)
		PrintMessage(L"No limit\n");
	else
		PrintMessage(L"%lu\n", pProtocol->dwMessageSize);

    PrintMessage(L"%27s: 0x%lx\n", L"Provider Reserved", pProtocol->dwProviderReserved);
}

VOID FindMSTcpIpProtocols( Array catalog, 
						                               LPWSAPROTOCOL_INFO *ppMSTcpProtocol,
													   LPWSAPROTOCOL_INFO *ppMSUdpProtocol, 
													   LPWSAPROTOCOL_INFO *ppMSRawIpProtocol )
{
	int i;

	if (ppMSTcpProtocol)
		*ppMSTcpProtocol = NULL;
	if (ppMSUdpProtocol)
		*ppMSTcpProtocol = NULL;
	if (ppMSRawIpProtocol)
		*ppMSTcpProtocol = NULL;

	for (i = 0; i < PROTOCOL_COUNT(catalog); i++) {
		LPWSAPROTOCOL_INFO pProtocol = PROTOCOL_NTH(catalog, i);
		if (IsEqualGUID(&pProtocol->ProviderId, &g_MSTcpIpGuid)) {
			switch (pProtocol->iSocketType)
			{
			case SOCK_STREAM:
				if (ppMSTcpProtocol) {
					*ppMSTcpProtocol = pProtocol;
					break;
				}
			case SOCK_DGRAM:
				if (ppMSUdpProtocol) {
					*ppMSUdpProtocol = pProtocol;
					break;
				}
			case  SOCK_RAW:
				if (ppMSRawIpProtocol) {
					*ppMSRawIpProtocol = pProtocol;
				}
			default:
				break;
			}
		}
	}
}

static int CompareGUID(LPCVOID a, LPCVOID b)
{
    LPCGUID pGuid1 = a;
    LPCGUID pGuid2 = b;

    return IsEqualGUID(pGuid1, pGuid2) ?  0 :  1;
}

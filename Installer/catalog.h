#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "cmnhdr.h"
#include "array.h"
#include <Guiddef.h>

extern const GUID g_MSTcpIpGuid;

#define IS_LAYERED_PROTOCOL(pProtocol) ((pProtocol)->ProtocolChain.ChainLen == 0)
#define IS_BASE_PROTOCOL(pProtocol) ((pProtocol)->ProtocolChain.ChainLen == 1)
#define IS_PROTOCOL_CHAIN(pProtocol) ((pProtocol)->ProtocolChain.ChainLen > 1)

#define PROTOCOL_ARRAY(array) ((LPWSAPROTOCOL_INFO)((array)->pStorage))
#define PROTOCOL_NTH(array, n) ((LPWSAPROTOCOL_INFO)ArrayGet((array), (n)))
#define PROTOCOL_COUNT(array) ((array)->iLength)

BOOL CatalogCleanup(VOID);
Array CatalogNew(VOID);
VOID CatalogDump(Array catalog);
VOID ProtocolDump(LPWSAPROTOCOL_INFO pProtocol);

VOID FindMSTcpIpProtocols(Array catalog, 
						                             LPWSAPROTOCOL_INFO *ppMSTcpProtocol, 
													 LPWSAPROTOCOL_INFO *ppMSUdpProtocol, 
													 LPWSAPROTOCOL_INFO *ppMSRawIpProtocol);


#endif /* _PROTOCOL_H_ */
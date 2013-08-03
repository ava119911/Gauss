#ifndef _GAUSS_H_
#define _GAUSS_H_

#include "cmnhdr.h"
#include <Guiddef.h>
#include "array.h"

#define GAUSS_LSP_NAME L"Gauss"
#define GAUSS_POSSIBLE_FILE_NAME_COUNT 10

extern const GUID g_GaussTcpIpGuid;
extern const GUID g_GaussLayeredGuid;

BOOL GaussGetInstallationPath(WCHAR szFullPath[], DWORD nCount);

BOOL GaussInstall(VOID);

BOOL GaussCatalogCleanup(VOID);

BOOL VerifyGaussInstallation(Array catalog);

VOID FindGaussProtocols(Array catalog,
						                        LPWSAPROTOCOL_INFO *ppGaussLayeredProtocol,
												LPWSAPROTOCOL_INFO *ppGaussTcpProtocol,
												LPWSAPROTOCOL_INFO *ppGaussIpProtocol,
												LPWSAPROTOCOL_INFO *ppGaussRawIpProtocol);

 #endif /* _GAUSS_H_ */
#include "cmnhdr.h"
#include <locale.h>
#include "printmsg.h"
#include "getopt.h"
#include "catalog.h"
#include "array.h"
#include "gauss.h"

#pragma comment(lib, "Ws2_32")

static VOID Usage(VOID);
static BOOL ListCatalog(BOOL bVerbose);

int wmain(int argc, PWSTR argv[])
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    LPCWSTR options = L"hlvri";
    BOOL bVerbose = FALSE;
    int iExitCode = 0;
    int rc = -1;
    enum {LIST, REMOVE, INSTALL, DUMMY} action = DUMMY;

    setlocale(LC_ALL, "");

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        WSAERROR(L"WSAStartup");
        return -1;
	}

    InitializeMessagePrintModule(FALSE, NULL);

    opterr = 0;
    while ((rc = getopt(argc, argv, options)) != -1) {
        switch (rc) 
		{
		case L'h':
            Usage();
            goto cleanup;
		case L'v':
            bVerbose = TRUE;
            break;
		case L'l':
            action = LIST;
            break;
		case L'r':
            action = REMOVE;
            break;
		case L'i':
            action = INSTALL;
            break;
		case L'?':
			PrintMessage(L"Option '-%c' is unknown or miss argument.\n", optopt);
			Usage();
			iExitCode = -1;
			goto cleanup;
		}
	}

    if (optind == 1) {
        Usage();
        goto cleanup;
	}

    switch (action)
	{
	case LIST:
        if (!ListCatalog(bVerbose)) {
            PrintMessage(L"List catalog failed");
            iExitCode = -1;
		}
        break;
	case REMOVE:
        if (!GaussCatalogCleanup()) {
            PrintMessage(L"Cleanup catalog failed");
            iExitCode = -1;
		} else {
            PrintMessage(L"Cleanup catalog successfully");
		}
        break;
	case INSTALL:
        if (!GaussInstall()) {
            PrintMessage(L"Install Gauss failed");
            iExitCode = -1;
		} else {
            PrintMessage(L"Install Gauss successfully");
		}
        break;
	default:
        // do nothing
        break;
	}

cleanup:
    CleanupMessagePrintModule();
    WSACleanup();

    return iExitCode;
}

static VOID Usage(VOID)
{
    PrintMessage(L"Usage: instlsp [-lvirh]\n");
    PrintMessage(L"%7s%-12s%s\n", L"", L"-l", L"list catalog summary");
    PrintMessage(L"%7s%-12s%s\n", L"", L"-v", L"list catalog detail, together with '-l'");
    PrintMessage(L"%7s%-12s%s\n", L"", L"-i", L"install gauss.dll");
    PrintMessage(L"%7s%-12s%s\n", L"", L"-r", L"cleanup catalog");
    PrintMessage(L"%7s%-12s%s\n", L"", L"-h", L"print this usage");
}

static BOOL ListCatalog(BOOL bVerbose) 
{
    Array catalog;

    if (!(catalog = CatalogNew()))
        return FALSE;

    if (bVerbose) {
        CatalogDump(catalog);
	} else {
        int i;

        for (i = 0; i < PROTOCOL_COUNT(catalog); i++) {
            LPWSAPROTOCOL_INFO pProtocol = PROTOCOL_NTH(catalog, i);
            PrintMessage(L"%lu  %d  %s\n", 
				                      pProtocol->dwCatalogEntryId, 
				                      pProtocol->ProtocolChain.ChainLen,
									  pProtocol->szProtocol);
		}
	}

    ArrayFree(catalog);

    return TRUE;
}

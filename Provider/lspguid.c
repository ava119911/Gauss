// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: lspguid.cpp
//
// Description:
//
//      This module defines the GUID under which the LSP's hidden dummy
//      entry is to be installed under. This GUID must be hard coded in
//      this file so that the DLL instance can find itself in the Winsock 
//      catalog. Once it finds the dummy hidden entry, it can then find
//      the layered protocol entries belonging to the LSP (since these
//      entries will reference the catalog ID of the hidden dummy entry).
//      
//      It is possible to create multiple LSPs by simply copying this project
//      to another directory, changing the GUID in this file, and changing
//      the DLL name. The LSP installer (instlsp.exe) can then install each
//      LSP with the -d flag which is the full path to each LSP to install.
//
//      A new export has been added: GetLspGuid. This allows the LSP installer
//      to install an arbitrary LSP as long as it exports this function as the
//      installer needs to find the GUID to install the hidden dummy entry
//      with. The GUIDs for the layered protocol entries are generated on the
//      fly.
//
#include "lspdef.h"
#include <windows.h>


//
// This is the hardcoded guid for our dummy (hidden) catalog entry
//
GUID g_GaussLayeredGuid = {
	0x30ea9dae,
	0xdabf, 
	0x4fb4,
	{0x97, 0xcb, 0xf5, 0xd8, 0x88, 0x35, 0xc2, 0xa7}
};

//
// Function: GetLspGuid
//
// Description:
//      This function is exported by the DLL and it returns the GUID under
//      which the LSP (hidden) entry is to be installed. Note that this export
//      is not required to write an LSP but it is here to make the installer code
//      easier since the LSP DLL and the installer at least need to know the GUID
//      of the hidden dummy entry of the LSP. Using this export allows the installer
//      to query each LSP instance of its GUID so that it may install it.
//
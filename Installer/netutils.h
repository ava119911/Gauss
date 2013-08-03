#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "cmnhdr.h"

LPCTSTR AddressFamilyToString(int iAddressFamily);
LPCTSTR IPProtocolToString(int iIPProtocol);
LPCTSTR SockTypeToString(int iSockType);

#endif /* _NETUTILS_H_ */
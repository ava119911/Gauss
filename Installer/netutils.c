#include "netutils.h"

LPCTSTR AddressFamilyToString(int iAddressFamily)
{
	switch (iAddressFamily) 
	{
	case AF_UNSPEC:
		return TEXT("AF_UNSPEC");
	case AF_UNIX: 
		return TEXT("AF_UNIX");
	case AF_INET:
		return TEXT("AF_INET");
	case AF_IMPLINK:
		return TEXT("AF_IMPLINK");
	case AF_PUP:
		return TEXT("AF_PUP");
	case AF_CHAOS:
		return TEXT("AF_CHAOS");
	case AF_IPX: 
		return TEXT("AF_NS or AF_IPX");
	case AF_ISO: 
		return TEXT("AF_ISO or AF_OSI");
	case AF_ECMA: 
		return TEXT("AF_ECMA");
	case AF_DATAKIT: 
		return TEXT("AF_DATAKIT");
	case AF_CCITT: 
		return TEXT("AF_CCITT");
	case AF_SNA:
		return TEXT("AF_SNA");
	case AF_DECnet: 
		return TEXT("AF_DECnet");
	case AF_DLI:
		return TEXT("AF_DLI");
	case AF_LAT:
		return TEXT("AF_LAT");
	case AF_HYLINK:
		return TEXT("AF_HYLINK");
	case AF_APPLETALK:
		return TEXT("AF_APPLETALK");
	case AF_NETBIOS:
		return TEXT("AF_NETBIOS");
	case AF_VOICEVIEW:
		return TEXT("AF_VOICEVIEW");
	case AF_FIREFOX:
		return TEXT("AF_FIREFOX");
	case AF_UNKNOWN1:
		return TEXT("AF_UNKNOWN1");
	case AF_BAN:
		return TEXT("AF_BAN");
	case AF_ATM:
		return TEXT("AF_ATM");
	case AF_INET6:
		return TEXT("AF_INET6");
	case AF_CLUSTER:
		return TEXT("AF_CLUSTER");
	case AF_12844:
		return TEXT("AF_12844");
	case AF_IRDA:
		return TEXT("AF_IRDA");
	case AF_NETDES:
		return TEXT("AF_NETDES");

#if (_WIN32_WINNT < 0x0501)
		/* switch end */
#else

	case AF_TCNPROCESS:
		return TEXT("AF_TCNPROCESS");
	case AF_TCNMESSAGE:
		return TEXT("AF_TCNMESSAGE");
	case AF_ICLFXBM:
		return TEXT("AF_ICLFXBM");

#if (_WIN32_WINNT < 0x0600)
		/* switch end here when _WIN32_WINNT < 0x0600 */
#else

	case AF_BTH:
		return TEXT("AF_BTH");

#if (_WIN32_WINNT < 0x0601)
		/* switch end here when _WIN32_WINNT < 0x0601 */
#else

	case AF_LINK:
		return TEXT("AF_LINK");

    /* switch end here when _WIN32_WINNT >= 0x0601 */

#endif  /* (_WIN32_WINNT < 0x0601) */
#endif  /* (_WIN32_WINNT < 0x0600) */
#endif  /* (_WIN32_WINNT < 0x0501) */
	}

	return TEXT("UNKNOWN");
}

LPCTSTR IPProtocolToString(int iIPProtocol)
{
	switch (iIPProtocol) 
	{
	case IPPROTO_ICMP:
		return TEXT("IPPROTO_ICMP");
	case IPPROTO_IGMP:
		return TEXT("IPPROTO_IGMP");
	case IPPROTO_GGP:
		return TEXT("IPPROTO_GGP");
	case IPPROTO_TCP:
		return TEXT("IPPROTO_TCP");
	case IPPROTO_PUP:
		return TEXT("IPPROTO_PUP");
	case IPPROTO_UDP:
		return TEXT("IPPROTO_UDP");
	case IPPROTO_IDP:
		return TEXT("IPPROTO_IDP");
	case IPPROTO_ND:
		return TEXT("IPPROTO_ND");
	case IPPROTO_RAW:
		return TEXT("IPPROTO_RAW");
	case IPPROTO_MAX:
		return TEXT("IPPROTO_MAX");
	case IPPROTO_RESERVED_RAW:
		return TEXT("IPPROTO_RESERVED_RAW");
	case IPPROTO_RESERVED_IPSEC:
		return TEXT("IPPROTO_RESERVED_IPSEC");
	case IPPROTO_RESERVED_IPSECOFFLOAD:
		return TEXT("IPPROTO_RESERVED_IPSECOFFLOAD");
	case IPPROTO_RESERVED_MAX:
		return TEXT("IPPROTO_RESERVED_MAX");

#if (_WIN32_WINNT >= 0x0501)
	case IPPROTO_HOPOPTS:
		return TEXT("IPPROTO_IP or IPPROTO_HOPOPTS");
	case IPPROTO_IPV4:
		return TEXT("IPPROTO_IPV4");
	case IPPROTO_IPV6:
		return TEXT("IPPROTO_IPV6");
	case IPPROTO_ROUTING:
		return TEXT("IPPROTO_ROUTING");
	case IPPROTO_FRAGMENT:
		return TEXT("IPPROTO_FRAGMENT");
	case IPPROTO_ESP:
		return TEXT("IPPROTO_ESP");
	case IPPROTO_AH:
		return TEXT("IPPROTO_AH");
	case IPPROTO_ICMPV6:
		return TEXT("IPPROTO_ICMPV6");
	case IPPROTO_NONE:
		return TEXT("IPPROTO_NONE");
	case IPPROTO_DSTOPTS:
		return TEXT("IPPROTO_DSTOPTS");
	case IPPROTO_ICLFXBM:
		return TEXT("IPPROTO_ICLFXBM");
#endif  /* (_WIN32_WINNT >= 0x0501) */

#if (_WIN32_WINNT >= 0x0600)
	case IPPROTO_ST:
		return TEXT("IPPROTO_ST");
	case IPPROTO_CBT:
		return TEXT("IPPROTO_CBT");
	case IPPROTO_EGP:
		return TEXT("IPPROTO_EGP");
	case IPPROTO_IGP:
		return TEXT("IPPROTO_IGP");
	case IPPROTO_RDP:
		return TEXT("IPPROTO_RDP");
	case IPPROTO_PIM:
		return TEXT("IPPROTO_PIM");
	case IPPROTO_PGM:
		return TEXT("IPPROTO_PGM");
	case IPPROTO_L2TP:
		return TEXT("IPPROTO_L2TP");
	case IPPROTO_SCTP:
		return TEXT("IPPROTO_SCTP");
#endif  /* (_WIN32_WINNT >= 0x0600) */
	}

	return TEXT("UNKNOWN");
};

LPCTSTR SockTypeToString(int iSockType)
{
	switch (iSockType)
	{
	case SOCK_STREAM:
		return TEXT("SOCK_STREAM");
    case SOCK_DGRAM:
		return TEXT("SOCK_DGRAM");
    case SOCK_RAW:
		return TEXT("SOCK_RAW");
    case SOCK_RDM:
		return TEXT("SOCK_RDM");
    case SOCK_SEQPACKET:
		return TEXT("SOCK_SEQPACKET");
	}

	return TEXT("UNKNOWN");
}

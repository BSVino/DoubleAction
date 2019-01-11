#ifndef P2P_H
#define P2P_H
#ifdef _WIN32
#pragma once
#endif

#include "steam/steam_api.h"
#include "blockingudpsocket.h"

#if defined(_WIN32) && !defined(_X360)
#define WIN32_LEAN_AND_MEAN
#undef INVALID_HANDLE_VALUE
#undef GetCommandLine
#undef ReadConsoleInput
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegSetValue
#undef RegSetValueEx
#include <winsock.h>
#elif POSIX
//#define INVALID_SOCKET -1
//#define SOCKET_ERROR -1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//#define closesocket close
#endif


class P2PTunnel
{
public:
	P2PTunnel(CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay);
	~P2PTunnel();

	void StartThreads();
	void Run1();
	void Run2();

private:
	CSteamID m_steamIDTarget;
	int m_nVirtualPort;
	int m_nTimeoutSec;
	bool m_bAllowUseOfPacketRelay;
	SNetSocket_t m_socketP2P;
	CBlockingUDPSocket m_socketNet;
	sockaddr_in m_sockaddrInRecipient;
};

#endif

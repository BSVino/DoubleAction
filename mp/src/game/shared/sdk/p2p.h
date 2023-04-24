#ifndef P2P_H
#define P2P_H
#ifdef _WIN32
#pragma once
#endif

#include "steam/steam_api.h"
#include "blockingudpsocket.h"

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
	struct sockaddr_in* m_sockaddrInRecipient;
};

#endif

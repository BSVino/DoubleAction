#include "cbase.h"

#include "p2p.h"

#include "vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

P2PTunnel::P2PTunnel(CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay) :
	m_steamIDTarget(steamIDTarget),
	m_nVirtualPort(nVirtualPort),
	m_nTimeoutSec(nTimeoutSec),
	m_bAllowUseOfPacketRelay(bAllowUseOfPacketRelay)
{
	m_socketP2P = steamapicontext->SteamNetworking()->CreateP2PConnectionSocket(m_steamIDTarget, m_nVirtualPort, m_nTimeoutSec, m_bAllowUseOfPacketRelay);

	memset(&m_sockaddrInRecipient, 0, sizeof(m_sockaddrInRecipient));
	m_sockaddrInRecipient.sin_family = AF_INET;
	m_sockaddrInRecipient.sin_port = htons(27015);
	m_sockaddrInRecipient.sin_addr.s_addr = inet_addr("127.0.0.1");
}

P2PTunnel::~P2PTunnel() {
	steamapicontext->SteamNetworking()->DestroySocket(m_socketP2P, true);
}

void P2PTunnel::StartThreads() {
	CreateSimpleThread([] (void* params) -> unsigned int {
		P2PTunnel* tunnel = (P2PTunnel*)params;
		tunnel->Run1();
		delete tunnel;

		return 0;
	}, this);

	CreateSimpleThread([] (void* params) -> unsigned int {
		P2PTunnel* tunnel = (P2PTunnel*)params;
		tunnel->Run2();
		delete tunnel;

		return 0;
	}, this);
}

void P2PTunnel::Run1() {
	while (m_socketNet.WaitForMessage(FP_INFINITE)) // TODO block until packet arrives
	{
		// Read from m_socketNet
		unsigned char data[4096];
		sockaddr_in packet_from;
		uint32 length = m_socketNet.ReceiveSocketMessage(&packet_from, &data[0], sizeof(data));

		// Write to m_socketP2P
		steamapicontext->SteamNetworking()->SendP2PPacket(m_steamIDTarget, data, length, k_EP2PSendUnreliable);
	}
}
void P2PTunnel::Run2() {
	uint32 length;

	while (!steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&length))
	{
		// Read from m_socketP2P
		unsigned char *data = new unsigned char[length];
		uint32 actualLength;
		CSteamID actualSteamID;

		steamapicontext->SteamNetworking()->ReadP2PPacket(data, length, &actualLength, &actualSteamID);
		Assert(length == actualLength);
		Assert(m_steamIDTarget == actualSteamID);

		// Write to m_socketNet
		m_socketNet.SendSocketMessage(m_sockaddrInRecipient, data, actualLength);
	}
}

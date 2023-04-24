#include "cbase.h"

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
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define closesocket close
#endif

#include "p2p.h"
#include "vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

sockaddr_in* CreateSockaddrIn(const char* addr, unsigned short port)
{
	sockaddr_in* ret = new sockaddr_in{};

	ret->sin_family = AF_INET;
	ret->sin_port = htons(port);
	ret->sin_addr.s_addr = inet_addr(addr);
	return ret;
}



P2PTunnel::P2PTunnel(CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay) :
	m_steamIDTarget(steamIDTarget),
	m_nVirtualPort(nVirtualPort),
	m_nTimeoutSec(nTimeoutSec),
	m_bAllowUseOfPacketRelay(bAllowUseOfPacketRelay)
{
	m_socketP2P = steamapicontext->SteamNetworking()->CreateP2PConnectionSocket(m_steamIDTarget, m_nVirtualPort, m_nTimeoutSec, m_bAllowUseOfPacketRelay);
	m_sockaddrInRecipient = CreateSockaddrIn("127.0.0.1", 27015);
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
	while (m_socketNet.WaitForMessage(FP_INFINITE))
	{
		// Read from m_socketNet
		unsigned char data[4096];
		sockaddr_in packet_from;
		uint32 length = m_socketNet.ReceiveSocketMessage(&packet_from, &data[0], sizeof(data));

		// TODO: compare packet_from with *m_sockaddrInRecipient

		// Write to m_socketP2P
		steamapicontext->SteamNetworking()->SendP2PPacket(m_steamIDTarget, data, length, k_EP2PSendUnreliable);
	}
}

void P2PTunnel::Run2() {
	uint32 length;

	while (true)
	{
		while (!steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&length)) ;
		// Read from m_socketP2P
		unsigned char *data = new unsigned char[length];
		uint32 actualLength;
		CSteamID actualSteamID;

		if (!steamapicontext->SteamNetworking()->ReadP2PPacket(data, length, &actualLength, &actualSteamID)) {
			return;
		}
		Assert(length == actualLength);
		Assert(m_steamIDTarget == actualSteamID);
		if (m_steamIDTarget != actualSteamID) {
			return;
		}

		// Write to m_socketNet
		m_socketNet.SendSocketMessage(*m_sockaddrInRecipient, data, actualLength);
	}
}

CON_COMMAND(da_test_p2p, "")
{
	//da_test_p2p 76561197967388217
	uint64 steamID64;
	sscanf(args[1], "%llu", &steamID64);

	CSteamID steamIDTarget(steamID64);
	Msg("test %llu\n", steamIDTarget.ConvertToUint64());

	P2PTunnel* tunnel = new P2PTunnel(steamIDTarget, 1234, 20, true);
	tunnel->StartThreads();
}

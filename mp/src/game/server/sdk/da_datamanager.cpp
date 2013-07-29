#include "cbase.h"

#ifdef WITH_DATA_COLLECTION

#undef min
#undef max

#include <time.h>

#include "da_datamanager.h"

#include "../datanetworking/math.pb.h"
#include "../datanetworking/data.pb.h"

#include "sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef WITH_DATA_COLLECTION

void FillProtoBufVector(da::protobuf::Vector* pVector, const Vector& vecFill)
{
	pVector->set_x(vecFill.x);
	pVector->set_y(vecFill.y);
	pVector->set_z(vecFill.z);
}

CDataManager g_DataManager( "CDataManager" );

extern bool DASendData(const da::protobuf::GameData& pbGameData, std::string& sError);
static void SendData( CFunctor **pData, unsigned int nCount )
{
	da::protobuf::GameData pbGameData;
	pbGameData.set_map_name(STRING(gpGlobals->mapname));

#ifdef _DEBUG
	pbGameData.set_debug(true);
#else
	pbGameData.set_debug(false);
#endif

	const ConVar* pHostname = cvar->FindVar( "hostname" );
	pbGameData.set_server_name(pHostname->GetString());

	pbGameData.set_timestamp((unsigned)time(NULL));

	da::protobuf::PlayerPositions* pPlayerPositions = pbGameData.mutable_positions();

	const CUtlVector<Vector>& avecPlayerPositions = g_DataManager.GetPlayerPositions();

	auto pPositions = pPlayerPositions->mutable_position();
	pPositions->Reserve(avecPlayerPositions.Count());

	size_t iDataSize = avecPlayerPositions.Count();

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pPositions->Add(), avecPlayerPositions[i]);

	g_DataManager.ClearData();

	std::string sError;
	if (!DASendData(pbGameData, sError))
		Msg("Error sending game data: %s", sError.c_str());
}

CDataManager::CDataManager( char const* name )
	: CAutoGameSystemPerFrame(name)
{
	m_pSendData = nullptr;

	m_flNextPositionsUpdate = 0;
}

void CDataManager::LevelInitPostEntity( void )
{
	// If the thread is executing, then wait for it to finish
	if ( m_pSendData )
	{
		m_pSendData->WaitForFinishAndRelease();
		m_pSendData = NULL;
	}
}

void CDataManager::FrameUpdatePostEntityThink( void )
{
	if (gpGlobals->curtime > m_flNextPositionsUpdate)
		SavePositions();
}

ConVar da_data_positions_interval("da_data_positions_interval", "10", FCVAR_DEVELOPMENTONLY, "How often to query player positions");

void CDataManager::SavePositions()
{
	if (IsSendingData())
		return;

	m_flNextPositionsUpdate = gpGlobals->curtime + da_data_positions_interval.GetFloat();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex( i ));
		if (!pPlayer)
			continue;

		if (pPlayer->IsBot())
			continue;

		if (!pPlayer->IsAlive())
			continue;

		m_avecPlayerPositions.AddToTail(pPlayer->GetAbsOrigin());
	}
}

void CDataManager::LevelShutdownPostEntity()
{
	if (!gpGlobals->maxClients)
		return;

	// If the thread is executing, then wait for it to finish
	if ( m_pSendData )
		m_pSendData->WaitForFinishAndRelease();

	m_pSendData = ThreadExecute( &SendData, (CFunctor**)NULL, 0 );
}

bool CDataManager::IsSendingData()
{
	return !!m_pSendData;
}

void CDataManager::ClearData()
{
	m_avecPlayerPositions.SetCount(0);
}

#endif

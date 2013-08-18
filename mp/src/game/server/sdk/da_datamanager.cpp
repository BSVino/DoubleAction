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

CDataManager& DataManager()
{
	return g_DataManager;
}

extern bool DASendData(const da::protobuf::GameData& pbGameData, std::string& sError);
static void SendData( CFunctor **pData, unsigned int nCount )
{
	da::protobuf::GameData pbGameData;

	g_DataManager.FillProtoBuffer(&pbGameData);

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

	ConVarRef sv_cheats("sv_cheats");
	m_bCheated |= sv_cheats.GetBool();
}

void CDataManager::AddCharacterChosen(const char* pszCharacter)
{
	m_apszCharactersChosen.AddToTail(pszCharacter);
}

void CDataManager::AddWeaponChosen(SDKWeaponID eWeapon)
{
	m_aeWeaponsChosen.AddToTail(eWeapon);
}

void CDataManager::AddSkillChosen(SkillID eSkill)
{
	m_aeSkillsChosen.AddToTail(eSkill);
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

void CDataManager::FillProtoBuffer(da::protobuf::GameData* pbGameData)
{
	pbGameData->set_map_name(STRING(gpGlobals->mapname));

#ifdef _DEBUG
	pbGameData->set_debug(true);
#else
	pbGameData->set_debug(false);
#endif

	pbGameData->set_cheats(m_bCheated);

	const ConVar* pHostname = cvar->FindVar( "hostname" );
	pbGameData->set_server_name(pHostname->GetString());

	pbGameData->set_timestamp((unsigned)time(NULL));

	auto pPositions = pbGameData->mutable_positions()->mutable_position();
	size_t iDataSize = m_avecPlayerPositions.Count();
	pPositions->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pPositions->Add(), m_avecPlayerPositions[i]);

	auto pCharacters = pbGameData->mutable_characters_chosen();
	iDataSize = m_apszCharactersChosen.Count();
	pCharacters->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pCharacters->Add()->assign(m_apszCharactersChosen[i]);

	auto pWeapons = pbGameData->mutable_weapons_chosen();
	iDataSize = m_aeWeaponsChosen.Count();
	pWeapons->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pWeapons->Add(m_aeWeaponsChosen[i]);

	auto pSkills = pbGameData->mutable_skills_chosen();
	iDataSize = m_aeSkillsChosen.Count();
	pSkills->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pSkills->Add(m_aeSkillsChosen[i]);

	ClearData();
}

void CDataManager::ClearData()
{
	m_avecPlayerPositions.SetCount(0);
}

#endif

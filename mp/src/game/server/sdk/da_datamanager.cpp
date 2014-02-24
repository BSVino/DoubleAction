#include "cbase.h"

using namespace std;

#ifdef WITH_DATA_COLLECTION

#undef min
#undef max

#include <time.h>

#include "da_datamanager.h"

#include "sdk_player.h"

#include "../datanetworking/math.pb.h"
#include "../datanetworking/data.pb.h"

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

static bool Account_LessFunc( AccountID_t const &a, AccountID_t const &b )
{
	return a < b;
}

CDataManager::CDataManager( char const* name )
	: CAutoGameSystemPerFrame(name)
{
	m_aiConnectedClients.SetLessFunc(Account_LessFunc);

	m_pSendData = NULL;

	d = NULL;

	m_bLevelStarted = false;
	ClearData();
}

CDataManager::~CDataManager()
{
	delete d;
}

void CDataManager::LevelInitPostEntity( void )
{
	// If the thread is executing, then wait for it to finish
	if ( m_pSendData )
	{
		m_pSendData->WaitForFinishAndRelease();
		m_pSendData = NULL;
	}

	m_bLevelStarted = true;
	d->m_flStartTime = gpGlobals->curtime;
	d->m_flNextPositionsUpdate = gpGlobals->curtime;

	CUtlMap<AccountID_t, char>::IndexType_t it = m_aiConnectedClients.FirstInorder();
	while (it != m_aiConnectedClients.InvalidIndex())
	{
		// This player is gone for good. Remove him from the list and we'll
		// count him as unique next time he shows up.
		if (m_aiConnectedClients[it] == 0)
		{
			CUtlMap<AccountID_t, char>::IndexType_t iRemove = it;
			m_aiConnectedClients.RemoveAt(iRemove);
			it = m_aiConnectedClients.NextInorder(it);
			continue;
		}

		// This player will be a unique player for the next map.
		d->m_iUniquePlayers++;

		it = m_aiConnectedClients.NextInorder(it);
	}
}

void CDataManager::FrameUpdatePostEntityThink( void )
{
	if (gpGlobals->curtime > d->m_flNextPositionsUpdate)
		SavePositions();
}

ConVar da_data_positions_interval("da_data_positions_interval", "10", FCVAR_DEVELOPMENTONLY, "How often to query player positions");

void CDataManager::SavePositions()
{
	if (IsSendingData())
		return;

	d->m_flNextPositionsUpdate = gpGlobals->curtime + da_data_positions_interval.GetFloat();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex( i ));
		if (!pPlayer)
			continue;

		if (pPlayer->IsBot())
			continue;

		if (!pPlayer->IsAlive())
			continue;

		d->m_avecPlayerPositions.AddToTail(pPlayer->GetAbsOrigin());

		if (pPlayer->IsInThirdPerson())
			d->m_iThirdPersonActive++;
		else
			d->m_iThirdPersonInactive++;
	}

	ConVarRef sv_cheats("sv_cheats");
	d->m_bCheated |= sv_cheats.GetBool();
}

void CDataManager::AddPlayerKill(const Vector& vecPosition)
{
	d->m_avecPlayerKills.AddToTail(vecPosition);
}

void CDataManager::AddPlayerDeath(const Vector& vecPosition)
{
	d->m_avecPlayerDeaths.AddToTail(vecPosition);
}

void CDataManager::AddCharacterChosen(const char* pszCharacter)
{
	CUtlMap<CUtlString, int>::IndexType_t it = d->m_asCharactersChosen.Find(CUtlString(pszCharacter));
	if (it == d->m_asCharactersChosen.InvalidIndex())
		d->m_asCharactersChosen.Insert(CUtlString(pszCharacter), 1);
	else
		d->m_asCharactersChosen[it]++;
}

void CDataManager::AddWeaponChosen(SDKWeaponID eWeapon)
{
	d->m_aeWeaponsChosen.AddToTail(eWeapon);
}

void CDataManager::AddSkillChosen(SkillID eSkill)
{
	d->m_aeSkillsChosen.AddToTail(eSkill);
}

void CDataManager::ClientConnected(AccountID_t eAccountID)
{
	// ClientConnected is called for every non-bot client every time a map loads, even with changelevel.
	// So we have to eliminate duplicate connections.
	CUtlMap<AccountID_t, char>::IndexType_t it = m_aiConnectedClients.Find(eAccountID);

	if (it == m_aiConnectedClients.InvalidIndex() || m_aiConnectedClients[it] == 0)
	{
		// This client was not previously in the list, so he has truly connected.

		if (it == m_aiConnectedClients.InvalidIndex())
		{
			// This client has not disconnected and reconnected.
			// We want to eliminate repeated connections as extra information.
			d->m_iConnections++;
			d->m_iUniquePlayers++;

			m_aiConnectedClients.Insert(eAccountID, 1);
		}
		else
			m_aiConnectedClients[eAccountID] = 1;
	}
}

void CDataManager::ClientDisconnected(AccountID_t eAccountID)
{
	// This is called only once for each client, never just for changelevels.
	CUtlMap<AccountID_t, char>::IndexType_t it = m_aiConnectedClients.Find(eAccountID);

	if (it == m_aiConnectedClients.InvalidIndex())
		m_aiConnectedClients.Insert(eAccountID, 0);
	else
		m_aiConnectedClients[eAccountID] = 0;

	d->m_iDisconnections++;
}

void CDataManager::SetTeamplay(bool bOn)
{
	d->m_bTeamplay = bOn;
}

void CDataManager::VotePassed(const char* pszIssue, const char* pszDetails)
{
	int i = d->m_aVoteResults.AddToTail();

	d->m_aVoteResults[i].m_bResult = true;
	d->m_aVoteResults[i].m_sIssue = pszIssue;
	d->m_aVoteResults[i].m_sDetails = pszDetails;
}

void CDataManager::VoteFailed(const char* pszIssue)
{
	int i = d->m_aVoteResults.AddToTail();

	d->m_aVoteResults[i].m_bResult = false;
	d->m_aVoteResults[i].m_sIssue = pszIssue;
}

ConVar da_data_enabled("da_data_enabled", "1", 0, "Turn on and off data sending.");

void CDataManager::LevelShutdownPostEntity()
{
	if (!gpGlobals->maxClients)
		return;

	if (!da_data_enabled.GetBool())
		return;

	// This function is sometimes called twice for every LevelInitPostEntity(), so remove duplicates.
	if (!m_bLevelStarted)
		return;

	// If the thread is executing, then wait for it to finish
	if ( m_pSendData )
		m_pSendData->WaitForFinishAndRelease();

	m_pSendData = ThreadExecute( &SendData, (CFunctor**)NULL, 0 );

	m_bLevelStarted = false;
}

bool CDataManager::IsSendingData()
{
	return !!m_pSendData;
}

void CDataManager::FillProtoBuffer(da::protobuf::GameData* pbGameData)
{
	pbGameData->set_da_version(atoi(DA_VERSION));

	pbGameData->set_map_name(STRING(gpGlobals->mapname));
	pbGameData->set_map_time(gpGlobals->curtime - d->m_flStartTime);

#ifdef _DEBUG
	pbGameData->set_debug(true);
#else
	pbGameData->set_debug(false);
#endif

	pbGameData->set_cheats(d->m_bCheated);

	const ConVar* pHostname = cvar->FindVar( "hostname" );
	pbGameData->set_server_name(pHostname->GetString());

	pbGameData->set_timestamp((unsigned)time(NULL));

	pbGameData->set_connections(d->m_iConnections);
	pbGameData->set_disconnections(d->m_iDisconnections);
	pbGameData->set_unique_players_this_map(d->m_iUniquePlayers);

	pbGameData->set_teamplay(d->m_bTeamplay);

	pbGameData->set_thirdperson_active(d->m_iThirdPersonActive);
	pbGameData->set_thirdperson_inactive(d->m_iThirdPersonInactive);

	google::protobuf::RepeatedPtrField<da::protobuf::Vector>* pPositions = pbGameData->mutable_positions()->mutable_position();
	size_t iDataSize = d->m_avecPlayerPositions.Count();
	pPositions->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pPositions->Add(), d->m_avecPlayerPositions[i]);

	google::protobuf::RepeatedPtrField<da::protobuf::Vector>* pKills = pbGameData->mutable_kills()->mutable_position();
	iDataSize = d->m_avecPlayerKills.Count();
	pKills->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pKills->Add(), d->m_avecPlayerKills[i]);

	google::protobuf::RepeatedPtrField<da::protobuf::Vector>* pDeaths = pbGameData->mutable_deaths()->mutable_position();
	iDataSize = d->m_avecPlayerDeaths.Count();
	pDeaths->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pDeaths->Add(), d->m_avecPlayerDeaths[i]);

	google::protobuf::RepeatedPtrField<std::string>* pCharacters = pbGameData->mutable_characters_chosen();
	iDataSize = d->m_asCharactersChosen.Count();
	pCharacters->Reserve(iDataSize);

	for (CUtlMap<CUtlString, int>::IndexType_t it = d->m_asCharactersChosen.FirstInorder(); it != d->m_asCharactersChosen.InvalidIndex(); it = d->m_asCharactersChosen.NextInorder(it))
	{
		for (int i = 0; i < d->m_asCharactersChosen[it]; i++)
			pCharacters->Add()->assign(d->m_asCharactersChosen.Key(it).String());
	}

	google::protobuf::RepeatedPtrField<std::string>* pWeapons = pbGameData->mutable_weapons_chosen_s();
	iDataSize = d->m_aeWeaponsChosen.Count();
	pWeapons->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pWeapons->Add()->assign(WeaponIDToAlias(d->m_aeWeaponsChosen[i]));

	google::protobuf::RepeatedPtrField<std::string>* pSkills = pbGameData->mutable_skills_chosen_s();
	iDataSize = d->m_aeSkillsChosen.Count();
	pSkills->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pSkills->Add()->assign(SkillIDToAlias(d->m_aeSkillsChosen[i]));

	google::protobuf::RepeatedPtrField<da::protobuf::VoteResult>* pVotes = pbGameData->mutable_votes();
	iDataSize = d->m_aVoteResults.Count();
	pVotes->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
	{
		da::protobuf::VoteResult* pVR = pVotes->Add();
		pVR->set_result(d->m_aVoteResults[i].m_bResult);
		pVR->set_issue(d->m_aVoteResults[i].m_sIssue);
		pVR->set_details(d->m_aVoteResults[i].m_sDetails);
	}

	ClearData();
}

void CDataManager::ClearData()
{
	// Delete the data container and re-create it every level
	// to remove the possibility of old data remaining.
	delete d;
	d = new CDataContainer();
}

#endif

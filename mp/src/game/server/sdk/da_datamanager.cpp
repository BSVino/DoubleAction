#include "cbase.h"

using namespace std;

#ifdef WITH_DATA_COLLECTION

#undef min
#undef max

#include <time.h>

#include "da_datamanager.h"

#include "sdk_player.h"
#include "weapon_grenade.h"
#include "sdk_gamerules.h"
#include "da_briefcase.h"

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
	d->z.m_flStartTime = gpGlobals->curtime;
	d->z.m_flNextPositionsUpdate = gpGlobals->curtime;

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
		d->z.m_iUniquePlayers++;

		it = m_aiConnectedClients.NextInorder(it);
	}
}

void CDataManager::FrameUpdatePostEntityThink( void )
{
	if (gpGlobals->curtime > d->z.m_flNextPositionsUpdate)
		SavePositions();
}

ConVar da_data_positions_interval("da_data_positions_interval", "10", FCVAR_DEVELOPMENTONLY, "How often to query player positions");

void CDataManager::SavePositions()
{
	if (IsSendingData())
		return;

	d->z.m_flNextPositionsUpdate = gpGlobals->curtime + da_data_positions_interval.GetFloat();

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
			d->z.m_iThirdPersonActive += da_data_positions_interval.GetFloat();
		else
			d->z.m_iThirdPersonInactive += da_data_positions_interval.GetFloat();

		if (pPlayer->m_bUsingVR)
			d->z.m_iVRActive += da_data_positions_interval.GetFloat();
		else
			d->z.m_iVRInactive += da_data_positions_interval.GetFloat();

		if (pPlayer->m_iPlatform == 1)
			d->z.m_iWindows += da_data_positions_interval.GetFloat();
		else if (pPlayer->m_iPlatform == 2)
			d->z.m_iLinux += da_data_positions_interval.GetFloat();
		else if (pPlayer->m_iPlatform == 3)
			d->z.m_iMac += da_data_positions_interval.GetFloat();
	}

	ConVarRef sv_cheats("sv_cheats");
	d->z.m_bCheated |= sv_cheats.GetBool();
}

int GetFlags(CSDKPlayer* pPlayer)
{
	unsigned long long flags = 0;

	if (pPlayer->IsInThirdPerson())
		flags |= 1<<da::protobuf::KILL_THIRDPERSON;

	if (pPlayer->m_Shared.IsAimedIn())
		flags |= 1<<da::protobuf::KILL_AIMIN;

	if (pPlayer->m_Shared.IsDiving())
		flags |= 1<<da::protobuf::KILL_DIVING;

	if (pPlayer->m_Shared.IsRolling())
		flags |= 1<<da::protobuf::KILL_ROLLING;

	if (pPlayer->m_Shared.IsSliding())
		flags |= 1<<da::protobuf::KILL_SLIDING;

	if (pPlayer->m_Shared.IsWallFlipping(true))
		flags |= 1<<da::protobuf::KILL_FLIPPING;

	if (pPlayer->m_Shared.IsSuperFalling())
		flags |= 1<<da::protobuf::KILL_SUPERFALLING;

	if (pPlayer->IsStyleSkillActive())
		flags |= 1<<da::protobuf::KILL_SKILL_ACTIVE;

	if (pPlayer->m_Shared.m_bSuperSkill)
		flags |= 1<<da::protobuf::KILL_SUPER_SKILL_ACTIVE;

	if (SDKGameRules()->GetBountyPlayer() == pPlayer)
		flags |= 1<<da::protobuf::KILL_IS_TARGET;

	if (pPlayer->HasBriefcase())
		flags |= 1<<da::protobuf::KILL_HAS_BRIEFCASE;

	if (pPlayer->IsBot())
		flags |= 1<<da::protobuf::KILL_IS_BOT;

	return flags;
}

void FillPlayerInfo(da::protobuf::PlayerInfo* pbPlayerInfo, CSDKPlayer* pPlayer)
{
	FillProtoBufVector(pbPlayerInfo->mutable_position(), pPlayer->GetAbsOrigin());
	pbPlayerInfo->set_health(pPlayer->GetHealth());
	pbPlayerInfo->set_flags(GetFlags(pPlayer));
	pbPlayerInfo->set_skill(SkillIDToAlias((SkillID)pPlayer->m_Shared.m_iStyleSkill.Get()));
	pbPlayerInfo->set_style(pPlayer->GetStylePoints());
	pbPlayerInfo->set_total_style(pPlayer->GetTotalStyle());
	pbPlayerInfo->set_kills(pPlayer->m_iKills);
	pbPlayerInfo->set_deaths(pPlayer->m_iDeaths);

	if (pPlayer->GetActiveSDKWeapon())
		pbPlayerInfo->set_weapon(WeaponIDToAlias(pPlayer->GetActiveSDKWeapon()->GetWeaponID()));

	if (!pPlayer->IsBot())
	{
		CSteamID ID;
		pPlayer->GetSteamID(&ID);
		pbPlayerInfo->set_accountid(ID.GetAccountID());
	}

	if (SDKGameRules()->GetWaypoint(0))
	{
		pbPlayerInfo->set_waypoint(pPlayer->m_iRaceWaypoint);
		FillProtoBufVector(pbPlayerInfo->mutable_objective_position(), SDKGameRules()->GetWaypoint(pPlayer->m_iRaceWaypoint)->GetAbsOrigin());
	}

	if (pPlayer->HasBriefcase())
		FillProtoBufVector(pbPlayerInfo->mutable_objective_position(), SDKGameRules()->GetCaptureZone()->GetAbsOrigin());

	if (pPlayer->m_iSlowMoType == SLOWMO_STYLESKILL)
		pbPlayerInfo->set_slowmo_type("super");
	else if (pPlayer->m_iSlowMoType == SLOWMO_ACTIVATED)
		pbPlayerInfo->set_slowmo_type("active");
	else if (pPlayer->m_iSlowMoType == SLOWMO_SUPERFALL)
		pbPlayerInfo->set_slowmo_type("superfall");
	else if (pPlayer->m_iSlowMoType == SLOWMO_PASSIVE)
		pbPlayerInfo->set_slowmo_type("passive");
	else if (pPlayer->m_iSlowMoType == SLOWMO_PASSIVE_SUPER)
		pbPlayerInfo->set_slowmo_type("passivesuper");
	else if (pPlayer->m_iSlowMoType == SLOWMO_NONE)
		pbPlayerInfo->set_slowmo_type("none");
	else
		pbPlayerInfo->set_slowmo_type("unknown");

	if (pPlayer->m_flSlowMoTime)
		pbPlayerInfo->set_slowmo_seconds(pPlayer->m_flSlowMoTime - gpGlobals->curtime);
	else
		pbPlayerInfo->set_slowmo_seconds(pPlayer->m_flSlowMoSeconds);
}

void CDataManager::AddKillInfo(const CTakeDamageInfo& info, CSDKPlayer* pVictim)
{
	d->m_apKillInfos.AddToTail(new da::protobuf::KillInfo());
	da::protobuf::KillInfo* pbKillInfo = d->m_apKillInfos.Tail();

	CBaseEntity* pAttacker = info.GetAttacker();

	da::protobuf::PlayerInfo* pbVictimInfo = pbKillInfo->mutable_victim();

	FillPlayerInfo(pbVictimInfo, pVictim);

	unsigned long long flags = pbVictimInfo->flags();

	if (dynamic_cast<CBaseGrenadeProjectile*>(info.GetInflictor()))
	{
		flags |= 1<<da::protobuf::KILL_BY_GRENADE;
		FillProtoBufVector(pbKillInfo->mutable_grenade_position(), info.GetInflictor()->GetAbsOrigin());
	}

	if (info.GetDamageType() == DMG_CLUB)
		flags |= 1<<da::protobuf::KILL_BY_BRAWL;

	if (pAttacker == pVictim)
		flags |= 1<<da::protobuf::KILL_IS_SUICIDE;

	pbVictimInfo->set_flags(flags);

	CSDKPlayer* pPlayerAttacker = ToSDKPlayer(pAttacker);
	if (pPlayerAttacker && pPlayerAttacker != pVictim)
		FillPlayerInfo(pbKillInfo->mutable_killer(), pPlayerAttacker);
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

da::protobuf::PlayerList* CDataManager::GetPlayerInList(CSDKPlayer* pPlayer)
{
	if (pPlayer->IsBot())
		return NULL;

	CSteamID ID;
	pPlayer->GetSteamID(&ID);

	if (!ID.IsValid())
		return NULL;

	if (ID.GetEUniverse() != k_EUniversePublic)
		return NULL;

	if (ID.GetEAccountType() != k_EAccountTypeIndividual)
		return NULL;

	CUtlMap<AccountID_t, class da::protobuf::PlayerList*>::IndexType_t it = d->m_apPlayerList.Find(ID.GetAccountID());
	if (it == d->m_apPlayerList.InvalidIndex())
	{
		it = d->m_apPlayerList.Insert(ID.GetAccountID(), new da::protobuf::PlayerList());

		da::protobuf::PlayerList* pbPlayerInfo = d->m_apPlayerList[it];

		pbPlayerInfo->set_accountid(ID.GetAccountID());
		pbPlayerInfo->set_name(pPlayer->GetPlayerName());
	}

	return d->m_apPlayerList[it];
}

void CDataManager::AddStyle(CSDKPlayer* pPlayer, float flStyle)
{
	da::protobuf::PlayerList* pbPlayerInfo = GetPlayerInList(pPlayer);

	if (!pbPlayerInfo)
		return;

	pbPlayerInfo->set_style(pbPlayerInfo->style() + flStyle);
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
			d->z.m_iConnections++;
			d->z.m_iUniquePlayers++;

			m_aiConnectedClients.Insert(eAccountID, 1);
		}
		else
			m_aiConnectedClients[it] = 1;
	}
}

void CDataManager::ClientDisconnected(AccountID_t eAccountID)
{
	// This is called only once for each client, never just for changelevels.
	CUtlMap<AccountID_t, char>::IndexType_t it = m_aiConnectedClients.Find(eAccountID);

	if (it == m_aiConnectedClients.InvalidIndex())
		m_aiConnectedClients.Insert(eAccountID, 0);
	else
		m_aiConnectedClients[it] = 0;

	d->z.m_iDisconnections++;
}

void CDataManager::SetTeamplay(bool bOn)
{
	d->z.m_bTeamplay = bOn;
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
	pbGameData->set_map_time(gpGlobals->curtime - d->z.m_flStartTime);

#ifdef _DEBUG
	pbGameData->set_debug(true);
#else
	pbGameData->set_debug(false);
#endif

	pbGameData->set_cheats(d->z.m_bCheated);

	const ConVar* pHostname = cvar->FindVar( "hostname" );
	pbGameData->set_server_name(pHostname->GetString());

	pbGameData->set_timestamp((unsigned)time(NULL));

	pbGameData->set_connections(d->z.m_iConnections);
	pbGameData->set_disconnections(d->z.m_iDisconnections);
	pbGameData->set_unique_players_this_map(d->z.m_iUniquePlayers);

	pbGameData->set_teamplay(d->z.m_bTeamplay);

	pbGameData->set_thirdperson_active(d->z.m_iThirdPersonActive);
	pbGameData->set_thirdperson_inactive(d->z.m_iThirdPersonInactive);

	pbGameData->set_vr_active(d->z.m_iVRActive);
	pbGameData->set_vr_inactive(d->z.m_iVRInactive);

	pbGameData->set_platform_windows(d->z.m_iWindows);
	pbGameData->set_platform_linux(d->z.m_iLinux);
	pbGameData->set_platform_osx(d->z.m_iMac);

	google::protobuf::RepeatedPtrField<da::protobuf::Vector>* pPositions = pbGameData->mutable_positions()->mutable_position();
	size_t iDataSize = d->m_avecPlayerPositions.Count();
	pPositions->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		FillProtoBufVector(pPositions->Add(), d->m_avecPlayerPositions[i]);

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

	google::protobuf::RepeatedPtrField<da::protobuf::KillInfo>* pKillInfos = pbGameData->mutable_kill_details();
	iDataSize = d->m_apKillInfos.Count();
	pKillInfos->Reserve(iDataSize);

	for (size_t i = 0; i < iDataSize; i++)
		pKillInfos->Add()->CopyFrom(*d->m_apKillInfos[i]);

	google::protobuf::RepeatedPtrField<da::protobuf::PlayerList>* pPlayerList = pbGameData->mutable_player_list();
	iDataSize = d->m_apPlayerList.Count();
	pPlayerList->Reserve(iDataSize);

	for (CUtlMap<AccountID_t, class da::protobuf::PlayerList*>::IndexType_t it = d->m_apPlayerList.FirstInorder(); it != d->m_apPlayerList.InvalidIndex(); it = d->m_apPlayerList.NextInorder(it))
		pPlayerList->Add()->CopyFrom(*d->m_apPlayerList[it]);

	ClearData();
}

void CDataManager::ClearData()
{
	// Delete the data container and re-create it every level
	// to remove the possibility of old data remaining.
	delete d;
	d = new CDataContainer();
}

CDataManager::CDataContainer::~CDataContainer()
{
	m_apKillInfos.PurgeAndDeleteElements();
	m_apPlayerList.PurgeAndDeleteElements();
}

#endif

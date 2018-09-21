//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "weapon_sdkbase.h"
#include "vprof.h"


#ifdef CLIENT_DLL

	#include "precache_register.h"
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"

#else
	
	#include "voice_gamemgr.h"
	#include "sdk_player.h"
	#include "sdk_team.h"
	#include "sdk_playerclass_info_parse.h"
	#include "player_resource.h"
	#include "mapentities.h"
	#include "sdk_basegrenade_projectile.h"
	#include "da_spawngenerator.h"
	#include "bots/bot_main.h"
	#include "da_briefcase.h"
	#include "vote_controller.h"
	#include "da_datamanager.h"

#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifndef CLIENT_DLL

class CSpawnPoint : public CPointEntity
{
public:
	bool IsDisabled() { return m_bDisabled; }
	void InputEnable( inputdata_t &inputdata ) { m_bDisabled = false; }
	void InputDisable( inputdata_t &inputdata ) { m_bDisabled = true; }

private:
	bool m_bDisabled;
	DECLARE_DATADESC();
};

BEGIN_DATADESC(CSpawnPoint)

	// Keyfields
	DEFINE_KEYFIELD( m_bDisabled,	FIELD_BOOLEAN,	"StartDisabled" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

END_DATADESC();

	LINK_ENTITY_TO_CLASS( info_player_deathmatch, CSpawnPoint );
#if defined( SDK_USE_TEAMS )
	LINK_ENTITY_TO_CLASS( info_player_blue, CSpawnPoint );
	LINK_ENTITY_TO_CLASS( info_player_red, CSpawnPoint );
#endif

#endif


REGISTER_GAMERULES_CLASS( CSDKGameRules );


BEGIN_NETWORK_TABLE_NOBASE( CSDKGameRules, DT_SDKGameRules )
#if defined ( CLIENT_DLL )
		RecvPropInt( RECVINFO( m_eCurrentMiniObjective ) ),
		RecvPropFloat( RECVINFO( m_flGameStartTime ) ),
		RecvPropBool( RECVINFO( m_bIsTeamplay ) ),
		RecvPropBool( RECVINFO( m_bCoderHacks ) ),
		RecvPropEHandle( RECVINFO( m_hBriefcase ) ),
		RecvPropEHandle( RECVINFO( m_hCaptureZone ) ),
		RecvPropEHandle( RECVINFO( m_hBountyPlayer ) ),
		RecvPropVector( RECVINFO( m_vecLowestSpawnPoint ) ),
		RecvPropArray3( RECVINFO_ARRAY(m_ahWaypoint1RaceLeaders), RecvPropEHandle( RECVINFO(m_ahWaypoint1RaceLeaders[0]))),
		RecvPropArray3( RECVINFO_ARRAY(m_ahWaypoint2RaceLeaders), RecvPropEHandle( RECVINFO(m_ahWaypoint2RaceLeaders[0]))),
		RecvPropEHandle( RECVINFO( m_hRaceWaypoint1 ) ),
		RecvPropEHandle( RECVINFO( m_hRaceWaypoint2 ) ),
		RecvPropEHandle( RECVINFO( m_hRaceWaypoint3 ) ),
#else
		SendPropInt( SENDINFO( m_eCurrentMiniObjective ) ),
		SendPropFloat( SENDINFO( m_flGameStartTime ), 32, SPROP_NOSCALE ),
		SendPropBool( SENDINFO( m_bIsTeamplay ) ),
		SendPropBool( SENDINFO( m_bCoderHacks ) ),
		SendPropEHandle( SENDINFO( m_hBriefcase ) ),
		SendPropEHandle( SENDINFO( m_hCaptureZone ) ),
		SendPropEHandle( SENDINFO( m_hBountyPlayer ) ),
		SendPropVector( SENDINFO( m_vecLowestSpawnPoint ) ),
		SendPropArray3( SENDINFO_ARRAY3( m_ahWaypoint1RaceLeaders ), SendPropEHandle(SENDINFO_ARRAY(m_ahWaypoint1RaceLeaders)) ),
		SendPropArray3( SENDINFO_ARRAY3( m_ahWaypoint2RaceLeaders ), SendPropEHandle(SENDINFO_ARRAY(m_ahWaypoint2RaceLeaders)) ),
		SendPropEHandle( SENDINFO( m_hRaceWaypoint1 ) ),
		SendPropEHandle( SENDINFO( m_hRaceWaypoint2 ) ),
		SendPropEHandle( SENDINFO( m_hRaceWaypoint3 ) ),
#endif
END_NETWORK_TABLE()

#if defined ( SDK_USE_PLAYERCLASSES )
	ConVar mp_allowrandomclass( "mp_allowrandomclass", "1", FCVAR_REPLICATED, "Allow players to select random class" );
#endif


LINK_ENTITY_TO_CLASS( sdk_gamerules, CSDKGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( SDKGameRulesProxy, DT_SDKGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_SDKGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		RecvPropDataTable( "sdk_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_SDKGameRules ), RecvProxy_SDKGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_SDKGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		SendPropDataTable( "sdk_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_SDKGameRules ), SendProxy_SDKGameRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL
	ConVar sk_plr_dmg_grenade( "sk_plr_dmg_grenade","0");		
#endif

ConVar mp_limitteams( 
	"mp_limitteams", 
	"2", 
	FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Max # of players 1 team can have over another (0 disables check)",
	true, 0,	// min value
	true, 30	// max value
);

static CSDKViewVectors g_SDKViewVectors(

	Vector( 0, 0, 58 ),			//VEC_VIEW
								
	Vector(-16, -16, 0 ),		//VEC_HULL_MIN
	Vector( 16,  16,  72 ),		//VEC_HULL_MAX
													
	Vector(-16, -16, 0 ),		//VEC_DUCK_HULL_MIN
	Vector( 16,  16, 45 ),		//VEC_DUCK_HULL_MAX
	Vector( 0, 0, 34 ),			//VEC_DUCK_VIEW
													
	Vector(-10, -10, -10 ),		//VEC_OBS_HULL_MIN
	Vector( 10,  10,  10 ),		//VEC_OBS_HULL_MAX
													
	Vector(-16, -16, 0  ),		//VEC_DIVE_HULL_MIN
	Vector( 16,  16, 24 ),		//VEC_DIVE_HULL_MAX
	Vector( 0, 0, 10 ),			//VEC_DIVE_VIEW

	Vector( 0, 0, 14 )			//VEC_DEAD_VIEWHEIGHT
#if defined ( SDK_USE_PRONE )			
	,Vector(-16, -16, 0 ),		//VEC_PRONE_HULL_MIN
	Vector( 16,  16, 24 ),		//VEC_PRONE_HULL_MAX
	Vector( 0,	0, 10 )			//VEC_PRONE_VIEW
#endif
);

ConVar da_globalslow("da_globalslow", "1", FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED, "Global slow motion");

const CViewVectors* CSDKGameRules::GetViewVectors() const
{
	return (CViewVectors*)GetSDKViewVectors();
}

const CSDKViewVectors *CSDKGameRules::GetSDKViewVectors() const
{
	return &g_SDKViewVectors;
}

ConVar da_miniobjective_time("da_miniobjective_time", "3", FCVAR_DEVELOPMENTONLY, "Time until the next miniobjective begins, in minutes.");
ConVar da_miniobjective_teamplay_time("da_miniobjective_teamplay_time", "1.5", FCVAR_DEVELOPMENTONLY, "Time until the next miniobjective begins, in minutes.");

#ifdef CLIENT_DLL


#else

// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// Dead players can only be heard by other dead team mates
		/* ROUNDS PLAY ONLY
		if ( pTalker->IsAlive() == false )
		{
			if ( pListener->IsAlive() == false )
				return ( pListener->InSameTeam( pTalker ) );

			return false;
		}*/
		if ( gpGlobals->teamplay )
			return ( pListener->InSameTeam( pTalker ) );
		else
			return true;
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;



// --------------------------------------------------------------------------------------------------- //
// Globals.
// --------------------------------------------------------------------------------------------------- //
static const char *s_PreserveEnts[] =
{
	"player",
	"viewmodel",
	"worldspawn",
	"soundent",
	"ai_network",
	"ai_hint",
	"sdk_gamerules",
	"sdk_team_manager",
	"sdk_team_unassigned",
	"sdk_team_blue",
	"sdk_team_red",
	"sdk_team_deathmatch",
	"sdk_player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sprite",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_illusionary",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_red",
	"info_player_blue",
	"point_viewcontrol",
	"shadow_control",
	"sky_camera",
	"scene_manager",
	"trigger_soundscape",
	"point_commentary_node",
	"func_precipitation",
	"func_team_wall",
	"", // END Marker
};

// --------------------------------------------------------------------------------------------------- //
// Global helper functions.
// --------------------------------------------------------------------------------------------------- //

// World.cpp calls this but we don't use it in SDK.
void InitBodyQue()
{
}


// --------------------------------------------------------------------------------------------------- //
// CSDKGameRules implementation.
// --------------------------------------------------------------------------------------------------- //
extern ConVar teamplay;

CSDKGameRules::CSDKGameRules()
{
	m_bLevelInitialized = false;

#if defined ( SDK_USE_TEAMS )
	m_iSpawnPointCount_Blue = 0;
	m_iSpawnPointCount_Red = 0;
	m_bIsTeamplay = false;// teamplay.GetBool();
#ifdef GAME_DLL
	gpGlobals->teamplay = m_bIsTeamplay;
#endif
#endif // SDK_USE_TEAMS

	InitTeams();

	m_flGameStartTime = 0;

	m_flNextSlowMoUpdate = 0;

	m_flNextMiniObjectiveStartTime = 0;

	m_bChangelevelDone = false;
	m_bNextMapVoteDone = false;
}

void RegisterVoteIssues();

void CSDKGameRules::LevelInitPostEntity()
{
	ConVarRef("sk_player_head").SetValue("1.3");

	BaseClass::LevelInitPostEntity();

	//Tony; initialize the level
	CheckLevelInitialized();

	if (m_bIsTeamplay)
		InitTeamSpawns();

	//Tony; do any post stuff
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	//TheBots->ServerActivate();

	m_eCurrentMiniObjective = MINIOBJECTIVE_NONE;
	m_ePreviousMiniObjective = MINIOBJECTIVE_NONE;

	m_flNextMiniObjectiveStartTime = gpGlobals->curtime + (da_miniobjective_time.GetFloat() + RandomFloat(-1, 1)) * 60;

#ifndef CLIENT_DLL
	RegisterVoteIssues();

	CBaseEntity* pSpot = NULL;
	pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_deathmatch" );

	if (pSpot)
	{
		m_vecLowestSpawnPoint = pSpot->GetAbsOrigin();

		while ((pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_deathmatch" )) != NULL)
		{
			if (pSpot->GetAbsOrigin().z < m_vecLowestSpawnPoint.Get().z)
				m_vecLowestSpawnPoint = pSpot->GetAbsOrigin();
		}
	}

#ifdef WITH_DATA_COLLECTION
	DataManager().SetTeamplay(m_bIsTeamplay);
#endif
#endif
}

bool CSDKGameRules::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	bool bConnected = BaseClass::ClientConnected(pEntity, pszName, pszAddress, reject, maxrejectlen);

#ifdef WITH_DATA_COLLECTION
	if (bConnected)
		DataManager().ClientConnected(engine->GetClientSteamID(pEntity)->GetAccountID());
#endif

	return bConnected;
}

void CSDKGameRules::ClientDisconnected( edict_t *pClient )
{
	BaseClass::ClientDisconnected(pClient);

	CoderHacksUpdate();

	CSDKPlayer* pSDKPlayer = ToSDKPlayer(CBaseEntity::Instance( pClient ));
	pSDKPlayer->DropBriefcase();

	if (pSDKPlayer == GetBountyPlayer())
		CleanupMiniObjective();

#ifdef WITH_DATA_COLLECTION
	if (!pSDKPlayer->IsBot())
		DataManager().ClientDisconnected(engine->GetClientSteamID(pSDKPlayer->edict())->GetAccountID());
#endif
}

bool CSDKGameRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
	return true;
}

void CSDKGameRules::CoderHacksUpdate()
{
	bool bCoderHacks = false;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (pPlayer->m_bCoderHacks)
		{
			bCoderHacks = true;
			break;
		}
	}

	if (bCoderHacks == m_bCoderHacks)
		return;

	m_bCoderHacks = bCoderHacks;

	if (m_bCoderHacks)
		UTIL_SayTextAll("Dev mode on.\n");
	else
		UTIL_SayTextAll("Dev mode off.\n");
}

// Enabling this causes bots to always give slowmo to other players, for debugging purposes.
//#define SLOWMO_DEBUG

ConVar da_slow_force_distance("da_slow_force_distance", "300", FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED, "Global slow motion");

void CSDKGameRules::ReCalculateSlowMo()
{
	// Reset all passive players to none, to prevent circular activations
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		CSDKPlayer* pSDKPlayer = static_cast<CSDKPlayer*>(pPlayer);

		if (pSDKPlayer->GetSlowMoType() == SLOWMO_PASSIVE || pSDKPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
			pSDKPlayer->SetSlowMoType(SLOWMO_NONE);
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		CSDKPlayer* pSDKPlayer = static_cast<CSDKPlayer*>(pPlayer);

#ifdef SLOWMO_DEBUG
		if (pSDKPlayer->IsBot())
			GiveSlowMoToNearbyPlayers(pSDKPlayer);
		else
#endif

		if (pSDKPlayer->GetSlowMoType() == SLOWMO_NONE)
			CalculateSlowMoForPlayer(pSDKPlayer);
		else
			GiveSlowMoToNearbyPlayers(pSDKPlayer);
	}

	m_flNextSlowMoUpdate = gpGlobals->curtime + 0.2f;
}

void CSDKGameRules::CalculateSlowMoForPlayer(CSDKPlayer* pPlayer)
{
	if (!pPlayer)
		return;

	if (!pPlayer->IsAlive())
	{
		pPlayer->SetSlowMoType(SLOWMO_NONE);
		return;
	}

	// If I activated slow then I get to keep my slow level.
	if (pPlayer->GetSlowMoType() == SLOWMO_ACTIVATED)
		return;

	if (pPlayer->GetSlowMoType() == SLOWMO_STYLESKILL)
		return;

	// Players who haven't activated anything are at the whims of those who have.

	if (gpGlobals->eLoadType == MapLoad_Background)
	{
		pPlayer->SetSlowMoType(SLOWMO_NONE);
		return;
	}

	slowmo_type eOtherInSlow = SLOWMO_NONE;

	CUtlVector<CSDKPlayer*> apOthersInPVS;

	CBaseEntity* pOther = NULL;

	while ((pOther = UTIL_EntitiesInPVS(pPlayer, pOther)) != NULL)
	{
		CSDKPlayer* pOtherPlayer = ToSDKPlayer(pOther);
		if (!pOtherPlayer)
			continue;

		if (pOtherPlayer == pPlayer)
			continue;

		apOthersInPVS.AddToTail(pOtherPlayer);
	}

	for (int i = 0; i < apOthersInPVS.Size(); i++)
	{
		CSDKPlayer* pOtherPlayer = apOthersInPVS[i];

		if (!pOtherPlayer->IsAlive())
			continue;

		if (pOtherPlayer->GetSlowMoType() != SLOWMO_NONE)
		{
			if ((pOtherPlayer->GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr() < da_slow_force_distance.GetFloat()*da_slow_force_distance.GetFloat() || pOtherPlayer->IsVisible(pPlayer))
			{
				if (pOtherPlayer->GetSlowMoType() == SLOWMO_STYLESKILL || pOtherPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
					eOtherInSlow = SLOWMO_PASSIVE_SUPER;
				else
					eOtherInSlow = SLOWMO_PASSIVE;

				break;
			}
		}

#ifdef SLOWMO_DEBUG
		if (pOtherPlayer->IsBot())
		{
			if ((pOtherPlayer->GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr() < da_slow_force_distance.GetFloat()*da_slow_force_distance.GetFloat() || pOtherPlayer->IsVisible(pPlayer))
			{
				bOtherInSlow = true;
				break;
			}
		}
#endif
	}

	// If any of these players are in slow then I'm in slow too.
	if (eOtherInSlow == SLOWMO_NONE)
		pPlayer->SetSlowMoType(SLOWMO_NONE);
	else
		pPlayer->SetSlowMoType(eOtherInSlow);
}

void CSDKGameRules::PlayerSlowMoUpdate(CSDKPlayer* pPlayer)
{
	if (!pPlayer)
		return;

	if (pPlayer->GetSlowMoType() == SLOWMO_NONE)
		// I turned off my slow-mo. Circular activations are possible. Just re-calculate for all players.
		ReCalculateSlowMo();
	else
		GiveSlowMoToNearbyPlayers(pPlayer);
}

void CSDKGameRules::GiveSlowMoToNearbyPlayers(CSDKPlayer* pPlayer)
{
	if (!pPlayer)
		return;

	if (!pPlayer->IsAlive())
	{
		pPlayer->SetSlowMoType(SLOWMO_NONE);
		return;
	}

	if (pPlayer->GetSlowMoType() == SLOWMO_NONE)
		return;

	// I have some slowmo on me. Pass it to other players nearby.

	CUtlVector<CSDKPlayer*> apOthersInPVS;

	CBaseEntity* pOther = NULL;

	slowmo_type eGiveType;

	if (pPlayer->GetSlowMoType() == SLOWMO_STYLESKILL || pPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
		eGiveType = SLOWMO_PASSIVE_SUPER;
	else
		eGiveType = SLOWMO_PASSIVE;

	while ((pOther = UTIL_EntitiesInPVS(pPlayer, pOther)) != NULL)
	{
		CSDKPlayer* pOtherPlayer = ToSDKPlayer(pOther);
		if (!pOtherPlayer)
			continue;

		if (pOtherPlayer == pPlayer)
			continue;

		// If they already have slow mo, we don't need to pass it to them.
		if (pOtherPlayer->GetSlowMoType() == SLOWMO_STYLESKILL)
			continue;

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_ACTIVATED)
			continue;

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
			continue;

		if (pOtherPlayer->GetSlowMoType() == eGiveType)
			continue;

		if ((pOtherPlayer->GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr() > da_slow_force_distance.GetFloat()*da_slow_force_distance.GetFloat())
		{
			if (!pOtherPlayer->IsVisible(pPlayer))
				continue;
		}

		apOthersInPVS.AddToTail(pOtherPlayer);
	}

	for (int i = 0; i < apOthersInPVS.Size(); i++)
	{
		CSDKPlayer* pOtherPlayer = apOthersInPVS[i];

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_STYLESKILL)
			continue;

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_ACTIVATED)
			continue;

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
			continue;

		if (pOtherPlayer->GetSlowMoType() == SLOWMO_SUPERFALL)
		{
			// He has superfall. Superfall is special and we only give it to him under certain conditions.

			// Do I have a passive? Then don't pass it on, sucks to be me.
			if (pPlayer->GetSlowMoType() == SLOWMO_PASSIVE || pPlayer->GetSlowMoType() == SLOWMO_PASSIVE_SUPER)
				continue;

			// Are we both in superfall? Good that's the way we want to stay.
			if (pPlayer->GetSlowMoType() == SLOWMO_SUPERFALL)
				continue;
		}

		if (pOtherPlayer->GetSlowMoType() == eGiveType)
			continue;

		// This guy is superfalling and we want to give him some slowmo? Give him superfall-slowmo instead.
		if ((eGiveType == SLOWMO_PASSIVE || eGiveType == SLOWMO_PASSIVE_SUPER) && pOtherPlayer->m_Shared.IsSuperFalling())
			pOtherPlayer->SetSlowMoType(SLOWMO_SUPERFALL);
		else
			pOtherPlayer->SetSlowMoType(eGiveType);

		GiveSlowMoToNearbyPlayers(pOtherPlayer);
	}
}

void CSDKGameRules::GoToIntermission( void )
{
	BaseClass::GoToIntermission();

	// set all players to FL_FROZEN
	for ( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( pPlayer )
		{
			pPlayer->AddFlag( FL_FROZEN );
		}
	}
}

void CSDKGameRules::CheckLevelInitialized()
{
	if ( !m_bLevelInitialized )
	{
#if defined ( SDK_USE_TEAMS )
		// Count the number of spawn points for each team
		// This determines the maximum number of players allowed on each

		CBaseEntity* ent = NULL;

		m_iSpawnPointCount_Blue		= 0;
		m_iSpawnPointCount_Red		= 0;

		while ( ( ent = gEntList.FindEntityByClassname( ent, "info_player_blue" ) ) != NULL )
		{
			if ( IsSpawnPointValid( ent, NULL ) )
			{
				m_iSpawnPointCount_Blue++;
			}
			else
			{
				Warning("Invalid blue spawnpoint at (%.1f,%.1f,%.1f)\n",
					ent->GetAbsOrigin()[0],ent->GetAbsOrigin()[2],ent->GetAbsOrigin()[2] );
			}
		}

		while ( ( ent = gEntList.FindEntityByClassname( ent, "info_player_red" ) ) != NULL )
		{
			if ( IsSpawnPointValid( ent, NULL ) ) 
			{
				m_iSpawnPointCount_Red++;
			}
			else
			{
				Warning("Invalid red spawnpoint at (%.1f,%.1f,%.1f)\n",
					ent->GetAbsOrigin()[0],ent->GetAbsOrigin()[2],ent->GetAbsOrigin()[2] );
			}
		}
#endif // SDK_USE_TEAMS
		m_bLevelInitialized = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKGameRules::~CSDKGameRules()
{
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: TF2 Specific Client Commands
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CSDKGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	CSDKPlayer *pPlayer = ToSDKPlayer( pEdict );
#if 0
	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "somecommand" ) )
	{
		if ( args.ArgC() < 2 )
			return true;

		// Do something here!

		return true;
	}
	else 
#endif
	// Handle some player commands here as they relate more directly to gamerules state
	if ( pPlayer->ClientCommand( args ) )
	{
		return true;
	}
	else if ( BaseClass::ClientCommand( pEdict, args ) )
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just spawned. Equip them.
//-----------------------------------------------------------------------------

void CSDKGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore )
{
	RadiusDamage( info, vecSrcIn, flRadius, iClassIgnore, NULL );
}

#define ROBUST_RADIUS_PROBE_DIST 32.0f // If a solid surface blocks the explosion, this is how far to creep along the surface looking for another way to the target

ConVar da_grenade_wall_protection("da_grenade_wall_protection", "3", FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED, "Scale wall thickness by this before subtracting it from the amount of damage.");

void CSDKGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT&(~CONTENTS_HITBOX);
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;
	Vector		vecToTarget;
	Vector		vecEndPos;

	Vector vecSrc = vecSrcIn;

	if ( flRadius )
		falloff = info.GetDamage() / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) & MASK_WATER) ? true : false;

	if( bInWater )
	{
		// Only muffle the explosion if deeper than 2 feet in water.
		if( !(UTIL_PointContents(vecSrc + Vector(0, 0, 24)) & MASK_WATER) )
		{
			bInWater = false;
		}
	}

	vecSrc.z += 1;// in case grenade is lying on the ground

	// iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity == pEntityIgnore )
			continue;

		if ( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// UNDONE: this should check a damage mask, not an ignore
		if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
		{// houndeyes don't hurt other houndeyes with their attack
			continue;
		}

		// blast's don't tavel into or out of water
		if (bInWater && pEntity->GetWaterLevel() == 0)
			continue;

		if (!bInWater && pEntity->GetWaterLevel() == 3)
			continue;

		// Check that the explosion can 'see' this entity.
		vecSpot = pEntity->BodyTarget( vecSrc, false );
		UTIL_TraceLine( vecSrc, vecSpot, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );

		bool bHit = false;

		vecEndPos = tr.endpos;

		Vector vecToTarget = vecSpot - tr.endpos;
		VectorNormalize( vecToTarget );

		if( tr.fraction == 1.0 || tr.m_pEnt == pEntity )
			bHit = true;

		if (!bHit)
		{
			// We're going to deflect the blast along the surface that 
			// interrupted a trace from explosion to this target.
			Vector vecUp, vecDeflect;
			CrossProduct( vecToTarget, tr.plane.normal, vecUp );
			CrossProduct( tr.plane.normal, vecUp, vecDeflect );
			VectorNormalize( vecDeflect );

			// Trace along the surface that intercepted the blast...
			UTIL_TraceLine( tr.endpos, tr.endpos + vecDeflect * ROBUST_RADIUS_PROBE_DIST, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );
			//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, false, 10 );

			// ...to see if there's a nearby edge that the explosion would 'spill over' if the blast were fully simulated.
			UTIL_TraceLine( tr.endpos, vecSpot, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );
			//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 10 );

			if( tr.fraction == 1.0 || tr.m_pEnt == pEntity )
				bHit = true;
		}

		vecToTarget = ( vecEndPos - vecSrc );

		float flDamage = info.GetDamage();

		// If we didn't find him, no damage.
		if (!bHit)
			flDamage = 0;

		// decrease damage for an ent that's farther from the bomb.
		flAdjustedDamage = vecToTarget.Length() * falloff;
		flAdjustedDamage = flDamage - flAdjustedDamage;

		if ( flAdjustedDamage < 0 )
			continue;

		CTakeDamageInfo adjustedInfo = info;
		adjustedInfo.SetDamage( flAdjustedDamage );

		Vector dir = vecToTarget;
		VectorNormalize( dir );

		// If we don't have a damage force, manufacture one
		if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
		{
			CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc, 1.5	/* explosion scale! */ );
		}
		else
		{
			// Assume the force passed in is the maximum force. Decay it based on falloff.
			float flForce = adjustedInfo.GetDamageForce().Length() * falloff;

			if (!bHit)
				flForce /= 2;

			adjustedInfo.SetDamageForce( dir * flForce );
			adjustedInfo.SetDamagePosition( vecSrc );
		}

		pEntity->TakeDamage( adjustedInfo );

		// Now hit all triggers along the way that respond to damage... 
		pEntity->TraceAttackToTriggers( adjustedInfo, vecSrc, vecEndPos, dir );
	}
}

void CSDKGameRules::Think()
{
	// Not terribly happy about this but also not terribly distraught.
	// Can't put it in PostEntity because it gets wiped if it's there.
	if (!FStrEq(TheNavMesh->GetPlayerSpawnName(), "info_player_deathmatch"))
		TheNavMesh->SetPlayerSpawnName("info_player_deathmatch");

	// Skip CMultiplayGameRules since we're doing the intermission logic ourselves.
	CGameRules::Think();

	for (int i = 0; i < GetNumberOfTeams(); i++)
	{
		if (!GetGlobalSDKTeam(i))
			continue;

		int iStyle = 0;

		for (int j = 0; j < GetGlobalSDKTeam(i)->GetNumPlayers(); j++)
		{
			if (!GetGlobalSDKTeam(i)->GetPlayer(j))
				continue;

			iStyle += ToSDKPlayer(GetGlobalSDKTeam(i)->GetPlayer(j))->GetTotalStyle();
		}

		GetGlobalSDKTeam(i)->SetScore(iStyle);
	}

	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			if ( !m_bChangelevelDone )
			{
				// Count up connected players
				int iConnected = 0;
				for (int i = 1; i < gpGlobals->maxClients; i++)
				{
					CBasePlayer* pPlayer = UTIL_PlayerByIndex( i );
					if (!pPlayer)
						continue;

					iConnected++;
				}

				// If we don't have a lot of players active, reset teamplay. It's no fun without enough players.
				if (iConnected < 6)
				{
					ConVarRef mp_teamplay("mp_teamplay");
					mp_teamplay.SetValue(false);
				}

				ChangeLevel(); // intermission is over
				m_bChangelevelDone = true;
			}
		}

		return;
	}

	if ( !m_bNextMapVoteDone && GetMapRemainingTime() && GetMapRemainingTime() < 2 * 60 )
	{
		if (g_voteController->CreateVote(DEDICATED_SERVER, "nextlevel", ""))
			m_bNextMapVoteDone = true;
	}

	if ( GetMapRemainingTime() < 0 )
	{
		GoToIntermission();
		return;
	}

	if (gpGlobals->curtime > m_flNextSlowMoUpdate)
		ReCalculateSlowMo();

	if (!m_eCurrentMiniObjective && m_flNextMiniObjectiveStartTime && gpGlobals->curtime > m_flNextMiniObjectiveStartTime)
		StartMiniObjective();

	if (m_eCurrentMiniObjective)
		MaintainMiniObjective();
}

// The bots do their processing after physics simulation etc so their visibility checks don't recompute
// bone positions multiple times a frame.
void CSDKGameRules::EndGameFrame( void )
{
	Bot_RunAll();

	BaseClass::EndGameFrame();
}

Vector DropToGround( 
					CBaseEntity *pMainEnt, 
					const Vector &vPos, 
					const Vector &vMins, 
					const Vector &vMaxs )
{
	trace_t trace;
	UTIL_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );
	return trace.endpos;
}


void TestSpawnPointType( const char *pEntClassName )
{
	// Find the next spawn spot.
	CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, pEntClassName );

	while( pSpot )
	{
		// check if pSpot is valid
		if( g_pGameRules->IsSpawnPointValid( pSpot, NULL ) )
		{
			// the successful spawn point's location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 0, 255, 0, 100, 60 );

			// drop down to ground
			Vector GroundPos = DropToGround( NULL, pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX );

			// the location the player will spawn at
			NDebugOverlay::Box( GroundPos, VEC_HULL_MIN, VEC_HULL_MAX, 0, 0, 255, 100, 60 );

			// draw the spawn angles
			QAngle spotAngles = pSpot->GetLocalAngles();
			Vector vecForward;
			AngleVectors( spotAngles, &vecForward );
			NDebugOverlay::HorzArrow( pSpot->GetAbsOrigin(), pSpot->GetAbsOrigin() + vecForward * 32, 10, 255, 0, 0, 255, true, 60 );
		}
		else
		{
			// failed spawn point location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 255, 0, 0, 100, 60 );
		}

		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}
}

void TestSpawns()
{
	TestSpawnPointType( "info_player_deathmatch" );
#if defined ( SDK_USE_TEAMS )
	TestSpawnPointType( "info_player_blue" );
	TestSpawnPointType( "info_player_red" );
#endif // SDK_USE_TEAMS
}
ConCommand cc_TestSpawns( "map_showspawnpoints", TestSpawns, "Dev - test the spawn points, draws for 60 seconds", FCVAR_CHEAT );

CBaseEntity *CSDKGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	// get valid spawn point
	CBaseEntity *pSpawnSpot = pPlayer->EntSelectSpawnPoint();
	Assert( pSpawnSpot );

	// let us fall to the ground so our movement state gets initialized properly
	pPlayer->SetLocalOrigin( pSpawnSpot->GetAbsOrigin() + Vector(0,0,2) );
	pPlayer->SetAbsVelocity( vec3_origin );

	// reset view punch and let spawnpoint dictate our direction on spawn
	pPlayer->SetLocalAngles( pSpawnSpot->GetLocalAngles() );
	pPlayer->m_Local.m_vecPunchAngle = vec3_angle;
	pPlayer->m_Local.m_vecPunchAngleVel = vec3_angle;
	pPlayer->SnapEyeAngles( pSpawnSpot->GetLocalAngles() );

	return pSpawnSpot;
}

// checks if the spot is clear of players
bool CSDKGameRules::IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer )
{
	if ( !pSpot->IsTriggered( pPlayer ) )
	{
		return false;
	}

	// Check if it is disabled by Enable/Disable
	CSpawnPoint *pSpawnPoint = dynamic_cast< CSpawnPoint * >( pSpot );
	if ( pSpawnPoint )
	{
		if ( pSpawnPoint->IsDisabled() )
		{
			return false;
		}
	}

	// in teamplay only use deathmatch spawns that are near team mates
	bool bNeedTeamMate = (IsTeamplay() && !stricmp(pSpot->GetClassname(),"info_player_deathmatch"));

	bool bTeammatesAlive;
	if (pPlayer)
		bTeammatesAlive = !!(pPlayer->GetTeam()->GetAliveMembers()-1); // Subtract 1 because the spawning player is considered alive by this point.
	else
		bTeammatesAlive = false;

	bNeedTeamMate &= bTeammatesAlive;

	for (int i = 0; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pOtherPlayer = UTIL_PlayerByIndex( i );
		if (!pOtherPlayer)
			continue;

		CSDKPlayer* pOtherSDKPlayer = ToSDKPlayer(pOtherPlayer);

		if (pOtherPlayer == pPlayer)
			continue;

		if (!pOtherSDKPlayer->IsAlive())
			continue;

		if ((pSpot->GetAbsOrigin() - pOtherPlayer->GetAbsOrigin()).LengthSqr() > 512*512)
			continue;

		trace_t tr;
		UTIL_TraceLine( pSpot->WorldSpaceCenter(), pOtherSDKPlayer->WorldSpaceCenter(), MASK_VISIBLE_AND_NPCS, pPlayer, COLLISION_GROUP_NONE, &tr );
		if (tr.m_pEnt == pOtherPlayer)
		{
			if (PlayerRelationship(pPlayer, pOtherPlayer) != GR_TEAMMATE)
				return false;
		}

		if (PlayerRelationship(pPlayer, pOtherPlayer) == GR_TEAMMATE)
		{
			if (bNeedTeamMate)
			{
				if ((pSpot->GetAbsOrigin() - pOtherPlayer->GetAbsOrigin()).LengthSqr() <= 384*384)
					bNeedTeamMate = false;
			}
			continue;
		}
	}

	// if we need a team mate and didn't find one don't use this point
	if (bNeedTeamMate)
		return false;

	// should we still check this even though you can't be hurt by pre-existing grenades?
	/*CBaseEntity* pGrenade = gEntList.FindEntityByClassname( NULL, "grenade_projectile" );
	while (pGrenade)
	{
		if ((pSpot->GetAbsOrigin() - pGrenade->GetAbsOrigin()).LengthSqr() < 500*500)
			return false;

		pGrenade = gEntList.FindEntityByClassname( pGrenade, "grenade_projectile" );
	}*/

	// Don't spawn players on top of the briefcase.
	CBaseEntity* pBriefcase = gEntList.FindEntityByClassname( NULL, "da_briefcase" );
	while (pBriefcase)
	{
		if ((pSpot->GetAbsOrigin() - pBriefcase->GetAbsOrigin()).LengthSqr() < 300*300)
			return false;

		pBriefcase = gEntList.FindEntityByClassname( pBriefcase, "da_briefcase" );
	}

	// Don't start me near a capture point, it's probably a hot area.
	CBaseEntity* pCapture = gEntList.FindEntityByClassname( NULL, "da_briefcase_capture" );
	while (pCapture)
	{
		if ((pSpot->GetAbsOrigin() - pCapture->GetAbsOrigin()).LengthSqr() < 500*500)
			return false;

		pCapture = gEntList.FindEntityByClassname( pCapture, "da_briefcase_capture" );
	}

	Vector mins = GetViewVectors()->m_vHullMin;
	Vector maxs = GetViewVectors()->m_vHullMax;

	Vector vTestMins = pSpot->GetAbsOrigin() + mins;
	Vector vTestMaxs = pSpot->GetAbsOrigin() + maxs;

	// First test the starting origin.
	return UTIL_IsSpaceEmpty( pPlayer, vTestMins, vTestMaxs );
}

bool CSDKGameRules::InitTeamSpawns()
{	
	CBaseEntity *pRefSpot = NULL;
	bool bSuccess = false;
	int iNumSpawns = 20;
	
	// Pick a (fairly) random spawnpoint to start this process
	for ( int i = random->RandomInt(1,32); i > 0; i-- )
		pRefSpot = gEntList.FindEntityByClassname( pRefSpot, "info_player_deathmatch" );
	if ( !pRefSpot )  // skip over the null point
		pRefSpot = gEntList.FindEntityByClassname( pRefSpot, "info_player_deathmatch" );

	// the second refpoint will tend to have a pattern so mix up which team gets it
	int iLastTeam = random->RandomInt(SDK_TEAM_BLUE,SDK_TEAM_RED);

	// create spawn grid
	CSpawnPointGenerator m_SpawnGen(pRefSpot, iLastTeam, iNumSpawns);
	bSuccess = (m_SpawnGen.SpawnsCreated() >= 1);
	m_SpawnGen.~CSpawnPointGenerator();

	// find the furthest spawn point from our initial spot
	CBaseEntity *pCheckSpot = gEntList.FindEntityByClassname( NULL, "info_player_deathmatch" );
	CBaseEntity *pFirstSpot = pRefSpot;
	Vector vecRefOrig = pRefSpot->GetAbsOrigin();
	Vector vecToSpot;
	float flCheckDistSqr;

	// size of farthest spawns list (ordered farthest to closest)
	const int iSetSize = 6;
	CUtlVector<CBaseEntity*> arrFarthestSet;
	float arrDistSqr[iSetSize];

	for (int i = 0; i < iSetSize; i++)
	{
		arrDistSqr[i] = 0;
	}

	// iterate the list of spawn points
	while(pCheckSpot)
	{
		if (pCheckSpot != pFirstSpot)
		{
			vecToSpot = pCheckSpot->GetAbsOrigin() - vecRefOrig;
			flCheckDistSqr = vecToSpot.LengthSqr();
		
			for (int i = 0; i<iSetSize; i++)
			{
				if(flCheckDistSqr > arrDistSqr[i])
				{
					arrFarthestSet.InsertBefore(i, pCheckSpot);

					for (int k = (iSetSize-1); k > i; k--)
						arrDistSqr[k] = arrDistSqr[k-1];
					
					arrDistSqr[i] = flCheckDistSqr;
					break;
				}
			}
		}

		// get the next spawn point
		pCheckSpot = gEntList.FindEntityByClassname( pCheckSpot, "info_player_deathmatch" );
	}

	// choose a random spawn from our set of spawns farthest from the initial point
	pRefSpot = arrFarthestSet[random->RandomInt(0,(iSetSize-1))];

	// use whichever team we didn't use last time
	iLastTeam = (iLastTeam == SDK_TEAM_BLUE ? SDK_TEAM_RED : SDK_TEAM_BLUE);
	m_SpawnGen = CSpawnPointGenerator(pRefSpot, iLastTeam, iNumSpawns);

	bSuccess = (m_SpawnGen.SpawnsCreated() >= 1);
	
	return (bSuccess);
}

void CSDKGameRules::PlayerSpawn( CBasePlayer *p )
{	
	CSDKPlayer *pPlayer = ToSDKPlayer( p );

	int team = pPlayer->GetTeamNumber();

	if( team != TEAM_SPECTATOR )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		if( pPlayer->m_Shared.DesiredPlayerClass() == PLAYERCLASS_RANDOM )
		{
			ChooseRandomClass( pPlayer );
			ClientPrint( pPlayer, HUD_PRINTTALK, "#game_now_as", GetPlayerClassName( pPlayer->m_Shared.PlayerClass(), team ) );
		}
		else
		{
			pPlayer->m_Shared.SetPlayerClass( pPlayer->m_Shared.DesiredPlayerClass() );
		}

		int playerclass = pPlayer->m_Shared.PlayerClass();

		if( playerclass != PLAYERCLASS_UNDEFINED )
		{
			//Assert( PLAYERCLASS_UNDEFINED < playerclass && playerclass < NUM_PLAYERCLASSES );

			CSDKTeam *pTeam = GetGlobalSDKTeam( team );
			const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( playerclass );

			Assert( pClassInfo.m_iTeam == team );

			pPlayer->SetModel( pClassInfo.m_szPlayerModel );
			pPlayer->SetHitboxSet( 0 );

			char buf[64];
			int bufsize = sizeof(buf);

			//Give weapons

			// Primary weapon
			Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iPrimaryWeapon) );
			CBaseEntity *pPrimaryWpn = pPlayer->GiveNamedItem( buf );
			Assert( pPrimaryWpn );

			// Secondary weapon
			CBaseEntity *pSecondaryWpn = NULL;
			if ( pClassInfo.m_iSecondaryWeapon != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iSecondaryWeapon) );
				pSecondaryWpn = pPlayer->GiveNamedItem( buf );
			}

			// Melee weapon
			if ( pClassInfo.m_iMeleeWeapon )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iMeleeWeapon) );
				pPlayer->GiveNamedItem( buf );
			}

			CWeaponSDKBase *pWpn = NULL;

			// Primary Ammo
			pWpn = dynamic_cast<CWeaponSDKBase *>(pPrimaryWpn);

			if( pWpn )
			{
				int iNumClip = pWpn->GetSDKWpnData().m_iDefaultAmmoClips - 1;	//account for one clip in the gun
				int iClipSize = pWpn->GetSDKWpnData().iMaxClip1;
				pPlayer->GiveAmmo( iNumClip * iClipSize, pWpn->GetSDKWpnData().szAmmo1 );
			}

			// Secondary Ammo
			if ( pSecondaryWpn )
			{
				pWpn = dynamic_cast<CWeaponSDKBase *>(pSecondaryWpn);

				if( pWpn )
				{
					int iNumClip = pWpn->GetSDKWpnData().m_iDefaultAmmoClips - 1;	//account for one clip in the gun
					int iClipSize = pWpn->GetSDKWpnData().iMaxClip1;
					pPlayer->GiveAmmo( iNumClip * iClipSize, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}				

			// Grenade Type 1
			if ( pClassInfo.m_iGrenType1 != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iGrenType1) );
				CBaseEntity *pGrenade = pPlayer->GiveNamedItem( buf );
				Assert( pGrenade );
				
				pWpn = dynamic_cast<CWeaponSDKBase *>(pGrenade);

				if( pWpn )
				{
					pPlayer->GiveAmmo( pClassInfo.m_iNumGrensType1 - 1, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}

			// Grenade Type 2
			if ( pClassInfo.m_iGrenType2 != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iGrenType2) );
				CBaseEntity *pGrenade2 = pPlayer->GiveNamedItem( buf );
				Assert( pGrenade2 );
				
				pWpn = dynamic_cast<CWeaponSDKBase *>(pGrenade2);

				if( pWpn )
				{
					pPlayer->GiveAmmo( pClassInfo.m_iNumGrensType2 - 1, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}

			pPlayer->Weapon_Switch( (CBaseCombatWeapon *)pPrimaryWpn );

//			DevMsg("setting spawn armor to: %d\n", pClassInfo.m_iArmor );
			pPlayer->SetSpawnArmorValue( pClassInfo.m_iArmor );

		}
		else
		{
//			Assert( !"Player spawning with PLAYERCLASS_UNDEFINED" );
			pPlayer->SetModel( SDK_PLAYER_MODEL );
		}
#else
		pPlayer->GiveDefaultItems();
#endif // SDK_USE_PLAYERCLASSES
		pPlayer->SetMaxSpeed( 600 );
	}
}

ConVar da_max_grenades("da_max_grenades", "2", FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT, "Max grenades without Nitrophiliac");

bool CSDKGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(pItem);
	CSDKPlayer* pSDKPlayer = ToSDKPlayer(pPlayer);

	if (pWeapon && pWeapon->GetWeaponID() == SDK_WEAPON_GRENADE && pSDKPlayer->m_Shared.m_iStyleSkill != SKILL_TROLL)
	{
		// If the player doesn't have Nitrophiliac, max 2 grenades.
		if (pSDKPlayer->GetAmmoCount("grenades") >= da_max_grenades.GetInt())
			return false;
	}

	return BaseClass::CanHavePlayerItem(pPlayer, pItem);
}

#if defined ( SDK_USE_PLAYERCLASSES )
void CSDKGameRules::ChooseRandomClass( CSDKPlayer *pPlayer )
{
	int i;
	int numChoices = 0;
	int choices[16];
	int firstclass = 0;

	CSDKTeam *pTeam = GetGlobalSDKTeam( pPlayer->GetTeamNumber() );

	int lastclass = pTeam->GetNumPlayerClasses();

	int previousClass = pPlayer->m_Shared.PlayerClass();

	// Compile a list of the classes that aren't full
	for( i=firstclass;i<lastclass;i++ )
	{
		// don't join the same class twice in a row
		if ( i == previousClass )
			continue;

		if( CanPlayerJoinClass( pPlayer, i ) )
		{	
			choices[numChoices] = i;
			numChoices++;
		}
	}

	// If ALL the classes are full
	if( numChoices == 0 )
	{
		Msg( "Random class found that all classes were full - ignoring class limits for this spawn\n" );

		pPlayer->m_Shared.SetPlayerClass( random->RandomFloat( firstclass, lastclass ) );
	}
	else
	{
		// Choose a slot randomly
		i = random->RandomInt( 0, numChoices-1 );

		// We are now the class that was in that slot
		pPlayer->m_Shared.SetPlayerClass( choices[i] );
	}
}
bool CSDKGameRules::CanPlayerJoinClass( CSDKPlayer *pPlayer, int cls )
{
	if( cls == PLAYERCLASS_RANDOM )
	{
		return mp_allowrandomclass.GetBool();
	}

	if( ReachedClassLimit( pPlayer->GetTeamNumber(), cls ) )
		return false;

	return true;
}

bool CSDKGameRules::ReachedClassLimit( int team, int cls )
{
	Assert( cls != PLAYERCLASS_UNDEFINED );
	Assert( cls != PLAYERCLASS_RANDOM );

	// get the cvar
	int iClassLimit = GetClassLimit( team, cls );

	// count how many are active
	int iClassExisting = CountPlayerClass( team, cls );

	if( iClassLimit > -1 && iClassExisting >= iClassLimit )
	{
		return true;
	}

	return false;
}

int CSDKGameRules::CountPlayerClass( int team, int cls )
{
	int num = 0;
	CSDKPlayer *pSDKPlayer;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pSDKPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		if (pSDKPlayer == NULL)
			continue;

		if (FNullEnt( pSDKPlayer->edict() ))
			continue;

		if( pSDKPlayer->GetTeamNumber() != team )
			continue;

		if( pSDKPlayer->m_Shared.DesiredPlayerClass() == cls )
			num++;
	}

	return num;
}

int CSDKGameRules::GetClassLimit( int team, int cls )
{
	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	Assert( pTeam );

	const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( cls );

	int iClassLimit;

	ConVar *pLimitCvar = ( ConVar * )cvar->FindVar( pClassInfo.m_szLimitCvar );

	Assert( pLimitCvar );

	if( pLimitCvar )
		iClassLimit = pLimitCvar->GetInt();
	else
		iClassLimit = -1;

	return iClassLimit;
}

bool CSDKGameRules::IsPlayerClassOnTeam( int cls, int team )
{
	if( cls == PLAYERCLASS_RANDOM )
		return true;

	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	return ( cls >= 0 && cls < pTeam->GetNumPlayerClasses() );
}

#endif // SDK_USE_PLAYERCLASSES

void CSDKGameRules::InitTeams( void )
{
	Assert( g_Teams.Count() == 0 );

	g_Teams.Purge();	// just in case

#if defined ( SDK_USE_PLAYERCLASSES )
	// clear the player class data
	ResetFilePlayerClassInfoDatabase();
#endif // SDK_USE_PLAYERCLASSES

	// Create the team managers

	//Tony; we have a special unassigned team incase our mod is using classes but not teams.
	CTeam *pUnassigned = static_cast<CTeam*>(CreateEntityByName( "sdk_team_unassigned" ));
	Assert( pUnassigned );
	pUnassigned->Init( pszTeamNames[TEAM_UNASSIGNED], TEAM_UNASSIGNED );
	g_Teams.AddToTail( pUnassigned );

	//Tony; just use a plain ole sdk_team_manager for spectators
	CTeam *pSpectator = static_cast<CTeam*>(CreateEntityByName( "sdk_team_manager" ));
	Assert( pSpectator );
	pSpectator->Init( pszTeamNames[TEAM_SPECTATOR], TEAM_SPECTATOR );
	g_Teams.AddToTail( pSpectator );

	//Tony; don't create these two managers unless teams are being used!
#if defined ( SDK_USE_TEAMS )
	if( IsTeamplay() )
	{
		//Tony; create the blue team
		CTeam *pBlue = static_cast<CTeam*>(CreateEntityByName( "sdk_team_blue" ));
		Assert( pBlue );
		pBlue->Init( pszTeamNames[SDK_TEAM_BLUE], SDK_TEAM_BLUE );
		g_Teams.AddToTail( pBlue );

		//Tony; create the red team
		CTeam *pRed = static_cast<CTeam*>(CreateEntityByName( "sdk_team_red" ));
		Assert( pRed );
		pRed->Init( pszTeamNames[SDK_TEAM_RED], SDK_TEAM_RED );
		g_Teams.AddToTail( pRed );
	}
#endif
	// clean this up later.. anyone who's in the game and playing should be on SDK_TEAM_DEATHMATCH
	CTeam *pDeathmatch = static_cast<CTeam*>(CreateEntityByName( "sdk_team_deathmatch" ));
	Assert( pDeathmatch );
	pDeathmatch->Init( pszTeamNames[SDK_TEAM_BLUE], SDK_TEAM_BLUE );
	g_Teams.AddToTail( pDeathmatch );
}

/* create some proxy entities that we use for transmitting data */
void CSDKGameRules::CreateStandardEntities()
{
	// Create the player resource
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create( "sdk_player_manager", vec3_origin, vec3_angle );

	// Create the entity that will send our data to the client.
#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
		CBaseEntity::Create( "sdk_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
}
int CSDKGameRules::SelectDefaultTeam()
{
	int team = TEAM_UNASSIGNED;

#if defined ( SDK_USE_TEAMS )
	if (gpGlobals->teamplay)
	{
		CSDKTeam *pBlue = GetGlobalSDKTeam(SDK_TEAM_BLUE);
		CSDKTeam *pRed = GetGlobalSDKTeam(SDK_TEAM_RED);

		int iNumBlue = pBlue->GetNumPlayers();
		int iNumRed = pRed->GetNumPlayers();

		int iBluePoints = pBlue->GetScore();
		int iRedPoints  = pRed->GetScore();

		// Choose the team that's lacking players
		if ( iNumBlue < iNumRed )
		{
			team = SDK_TEAM_BLUE;
		}
		else if ( iNumBlue > iNumRed )
		{
			team = SDK_TEAM_RED;
		}
		// choose the team with fewer points
		else if ( iBluePoints < iRedPoints )
		{
			team = SDK_TEAM_BLUE;
		}
		else if ( iBluePoints > iRedPoints )
		{
			team = SDK_TEAM_RED;
		}
		else
		{
			// Teams and scores are equal, pick a random team
			team = ( random->RandomInt(0,1) == 0 ) ? SDK_TEAM_BLUE : SDK_TEAM_RED;		
		}

		if ( TeamFull( team ) )
		{
			// Pick the opposite team
			if ( team == SDK_TEAM_BLUE )
			{
				team = SDK_TEAM_RED;
			}
			else
			{
				team = SDK_TEAM_BLUE;
			}

			// No choices left
			if ( TeamFull( team ) )
				return TEAM_UNASSIGNED;
		}
	}
#endif // SDK_USE_TEAMS
	return team;
}
#if defined ( SDK_USE_TEAMS )
//Tony; we only check this when using teams, unassigned can never get full.
bool CSDKGameRules::TeamFull( int team_id )
{
	switch ( team_id )
	{
	case SDK_TEAM_BLUE:
		{
			//int iNumBlue = GetGlobalSDKTeam(SDK_TEAM_BLUE)->GetNumPlayers();
			return false;//iNumBlue >= m_iSpawnPointCount_Blue;
		}
	case SDK_TEAM_RED:
		{
			//int iNumRed = GetGlobalSDKTeam(SDK_TEAM_RED)->GetNumPlayers();
			return false;//iNumRed >= m_iSpawnPointCount_Red;
		}
	}
	return false;
}

//checks to see if the desired team is stacked, returns true if it is
bool CSDKGameRules::TeamStacked( int iNewTeam, int iCurTeam  )
{
	//players are allowed to change to their own team
	if(iNewTeam == iCurTeam)
		return false;

#if defined ( SDK_USE_TEAMS )
	if (!IsTeamplay())
		return false;

	int iTeamLimit = mp_limitteams.GetInt();

	// Tabulate the number of players on each team.
	int iNumBlue = GetGlobalTeam( SDK_TEAM_BLUE )->GetNumPlayers();
	int iNumRed = GetGlobalTeam( SDK_TEAM_RED )->GetNumPlayers();

	switch ( iNewTeam )
	{
	case SDK_TEAM_BLUE:
		if( iCurTeam != TEAM_UNASSIGNED && iCurTeam != TEAM_SPECTATOR )
		{
			if((iNumBlue + 1) > (iNumRed + iTeamLimit - 1))
				return true;
			else
				return false;
		}
		else
		{
			if((iNumBlue + 1) > (iNumRed + iTeamLimit))
				return true;
			else
				return false;
		}
		break;
	case SDK_TEAM_RED:
		if( iCurTeam != TEAM_UNASSIGNED && iCurTeam != TEAM_SPECTATOR )
		{
			if((iNumRed + 1) > (iNumBlue + iTeamLimit - 1))
				return true;
			else
				return false;
		}
		else
		{
			if((iNumRed + 1) > (iNumBlue + iTeamLimit))
				return true;
			else
				return false;
		}
		break;
	}
#endif // SDK_USE_TEAMS

	return false;
}

#endif // SDK_USE_TEAMS
//-----------------------------------------------------------------------------
// Purpose: determine the class name of the weapon that got a kill
//-----------------------------------------------------------------------------
const char *CSDKGameRules::GetKillingWeaponName( const CTakeDamageInfo &info, CSDKPlayer *pVictim, int *iWeaponID )
{
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = SDKGameRules()->GetDeathScorer( pKiller, pInflictor, pVictim );

	const char *killer_weapon_name = "world";
	*iWeaponID = SDK_WEAPON_NONE;

	if ( pScorer && pInflictor && ( pInflictor == pScorer ) )
	{
		// If the inflictor is the killer,  then it must be their current weapon doing the damage
		if ( pScorer->GetActiveWeapon() )
		{
			killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname(); 
			if ( pScorer->IsPlayer() )
			{
				*iWeaponID = ToSDKPlayer(pScorer)->GetActiveSDKWeapon()->GetWeaponID();
			}
		}
	}
	else if ( pInflictor )
	{
		killer_weapon_name = STRING( pInflictor->m_iClassname );

		CWeaponSDKBase *pWeapon = dynamic_cast< CWeaponSDKBase * >( pInflictor );
		if ( pWeapon )
		{
			*iWeaponID = pWeapon->GetWeaponID();
		}
		else
		{
			CBaseGrenadeProjectile *pBaseGrenade = dynamic_cast<CBaseGrenadeProjectile*>( pInflictor );
			if ( pBaseGrenade )
			{
				*iWeaponID = pBaseGrenade->GetWeaponID();
			}
		}
	}

	// strip certain prefixes from inflictor's classname
	const char *prefix[] = { "weapon_", "NPC_", "func_" };
	for ( int i = 0; i< ARRAYSIZE( prefix ); i++ )
	{
		// if prefix matches, advance the string pointer past the prefix
		int len = Q_strlen( prefix[i] );
		if ( strncmp( killer_weapon_name, prefix[i], len ) == 0 )
		{
			killer_weapon_name += len;
			break;
		}
	}

	// grenade projectiles need to be translated to 'grenade' 
	if ( 0 == Q_strcmp( killer_weapon_name, "grenade_projectile" ) )
	{
		killer_weapon_name = "grenade";
	}

	return killer_weapon_name;
}

void CSDKGameRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	if (pVictim && pVictim == GetBountyPlayer())
	{
		CSDKPlayer::SendBroadcastSound("MiniObjective.BountyKilled");

		if (pVictim == info.GetAttacker())
		{
			RestartMiniObjective();
		}
		else
		{
			CSDKPlayer* pPlayerKiller = ToSDKPlayer(info.GetAttacker());
			if (pPlayerKiller)
			{
				GiveMiniObjectiveRewardPlayer(pPlayerKiller);

				CSDKPlayer::SendBroadcastNotice(NOTICE_BOUNTY_COLLECTED, pPlayerKiller);
			}
			else
				CSDKPlayer::SendBroadcastNotice(NOTICE_BOUNTY_LOST);

			CleanupMiniObjective();
		}
	}
	else if (GetBountyPlayer() && GetBountyPlayer() == ToSDKPlayer(info.GetAttacker()))
	{
		GetBountyPlayer()->TakeHealth(25, 0);
	}

	CSDKPlayer* pLeader = GetLeader();

	RemovePlayerFromLeaders(m_ahWaypoint1RaceLeaders, ToSDKPlayer(pVictim));
	RemovePlayerFromLeaders(m_ahWaypoint2RaceLeaders, ToSDKPlayer(pVictim));

	CSDKPlayer* pNewLeader = GetLeader();

	if (pNewLeader && pLeader && pNewLeader != pLeader)
	{
		CSDKPlayer::SendBroadcastNotice(NOTICE_RATRACE_PLAYER_LEAD, pNewLeader);
		CSDKPlayer::SendBroadcastSound("MiniObjective.Begin");
	}

	BaseClass::PlayerKilled(pVictim, info);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//			*pKiller - 
//			*pInflictor - 
//-----------------------------------------------------------------------------
void CSDKGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	int killer_ID = 0;

	// Find the killer & the scorer
	CSDKPlayer *pSDKPlayerVictim = ToSDKPlayer( pVictim );
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor, pVictim );
//	CSDKPlayer *pAssister = ToSDKPlayer( GetAssister( pVictim, pScorer, pInflictor ) );

	// Work out what killed the player, and send a message to all clients about it
	int iWeaponID;
	const char *killer_weapon_name = GetKillingWeaponName( info, pSDKPlayerVictim, &iWeaponID );

	if ( pScorer )	// Is the killer a client?
	{
		killer_ID = pScorer->GetUserID();
	}

	IGameEvent * event = gameeventmanager->CreateEvent( "player_death" );

	if ( event )
	{
		event->SetInt( "userid", pVictim->GetUserID() );
		event->SetInt( "attacker", killer_ID );
//		event->SetInt( "assister", pAssister ? pAssister->GetUserID() : -1 );
		event->SetString( "weapon", killer_weapon_name );
		event->SetInt( "weaponid", iWeaponID );
		event->SetInt( "brawl", !!(info.GetDamageType() & DMG_CLUB) );
		event->SetInt( "grenade", !!dynamic_cast<CBaseGrenadeProjectile*>(info.GetInflictor()) );
		event->SetInt( "damagebits", info.GetDamageType() );
		event->SetInt( "customkill", info.GetDamageCustom() );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted
		gameeventmanager->FireEvent( event );
	}		
}
#endif


bool CSDKGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		::V_swap(collisionGroup0,collisionGroup1);
	}

	//Don't stand on COLLISION_GROUP_WEAPON
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

//Tony; keep this in shared space.
#if defined ( SDK_USE_PLAYERCLASSES )
const char *CSDKGameRules::GetPlayerClassName( int cls, int team )
{
	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	if( cls == PLAYERCLASS_RANDOM )
	{
		return "#class_random";
	}

	if( cls < 0 || cls >= pTeam->GetNumPlayerClasses() )
	{
		Assert( false );
		return NULL;
	}

	const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( cls );

	return pClassInfo.m_szPrintName;
}
#endif

float CSDKGameRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	CSDKPlayer* pSDKPlayer = ToSDKPlayer(pPlayer);

	float flFallVelocity = pSDKPlayer->m_Local.m_flFallVelocity;
	float flMaxSafeFallSpeed = 700;
	float flMaxSafeDiveFallSpeed = 800;

	float flExcessSpeed = flFallVelocity - flMaxSafeFallSpeed;

	if (pSDKPlayer->m_Shared.IsDiving())
		flExcessSpeed = flFallVelocity - flMaxSafeDiveFallSpeed;

	float flDamage;

	if (pSDKPlayer->m_Shared.IsDiving())
		flDamage = RemapValClamped(flExcessSpeed, 0, 300, 0, 10);
	else
		flDamage = RemapValClamped(flExcessSpeed, 0, 300, 0, 50);

	return flDamage;
}

void CSDKGameRules::OverrideSoundParams( const EmitSound_t& ep, CSoundParameters& oParams )
{
#ifdef CLIENT_DLL
	if (FStrEq(ep.m_pSoundName, "SlowMo.Start"))
		return;

	if (FStrEq(ep.m_pSoundName, "SlowMo.End"))
		return;

	if (FStrEq(ep.m_pSoundName, "SlowMo.Loop"))
		return;

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();

	oParams.pitch *= pLocalPlayer->GetSlowMoMultiplier();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Init CS ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION         50

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef* GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		def.AddAmmoType( "9x19mm",   DMG_BULLET,   TRACER_LINE_AND_WHIZ, 0, 0, INFINITE_AMMO, BULLET_IMPULSE(120, 400),    0 );
		def.AddAmmoType( "762x51mm", DMG_BULLET,   TRACER_LINE_AND_WHIZ, 0, 0, INFINITE_AMMO, BULLET_IMPULSE(140, 800),    0 );
		def.AddAmmoType( "45acp",    DMG_BULLET,   TRACER_LINE_AND_WHIZ, 0, 0, INFINITE_AMMO, BULLET_IMPULSE(200, 1225),   0 );

		def.AddAmmoType( "buckshot", DMG_BUCKSHOT, TRACER_LINE_AND_WHIZ, 0, 0, INFINITE_AMMO, BULLET_IMPULSE(526/9, 1300), 0 );
		def.AddAmmoType( "grenades", DMG_BLAST,    TRACER_NONE,          0, 0, 4,             1,                           0 );
	}

	return &def;
}


#ifndef CLIENT_DLL

const char *CSDKGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	//Tony; no prefix for now, it isn't needed.
	return "";
}

const char *CSDKGameRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
		return NULL;

	const char *pszFormat = NULL;

	if ( bTeamOnly )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			pszFormat = "DA_Chat_Spec";
		else
		{
			/* only needed in round based play
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "DA_Chat_Team_Dead";
			else
			*/
				pszFormat = "DA_Chat_Team";
		}
	}
	else
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
			pszFormat = "DA_Chat_All_Spec";
		else
		{
			/* only needed in round based play
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "DA_Chat_All_Dead";
			else
			*/
				pszFormat = "DA_Chat_All";
		}
	}

	return pszFormat;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CSDKGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	if (pPlayer == pTarget)
		return GR_TEAMMATE;

#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
		return GR_TEAMMATE;

#endif

	return GR_NOTTEAMMATE;
}

float CSDKGameRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	if (m_eCurrentMiniObjective != MINIOBJECTIVE_NONE)
		return 0;

	// timelimit is in minutes
	float flTimeLeft =  ( m_flGameStartTime + mp_timelimit.GetInt() * 60 ) - gpGlobals->curtime;

	if ( flTimeLeft <= 0 )
		flTimeLeft = -1;

	return flTimeLeft;
}

float CSDKGameRules::GetMapElapsedTime( void )
{
	return gpGlobals->curtime;
}

Vector CSDKGameRules::GetLowestSpawnPoint()
{
	return m_vecLowestSpawnPoint;
}

#ifndef CLIENT_DLL
void CSDKGameRules::StartMiniObjective(const char* pszObjective)
{
	CleanupMiniObjective();

	RandomSeed((int)(gpGlobals->curtime*1000));

	float flNextObjectiveTime = da_miniobjective_time.GetFloat();
	if (IsTeamplay())
		flNextObjectiveTime = da_miniobjective_teamplay_time.GetFloat();

	m_flNextMiniObjectiveStartTime = gpGlobals->curtime + (flNextObjectiveTime + RandomFloat(-1, 1)) * 60;

	bool bResult = false;

	int iPlayers = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		iPlayers++;
	}

	if (iPlayers < 2)
		return;

	miniobjective_t eObjective = (miniobjective_t)RandomInt(1, MINIOBJECTIVE_MAX-1);
	for (int i = 0; i < 3; i++)
	{
		do {
			eObjective = (miniobjective_t)RandomInt(1, MINIOBJECTIVE_MAX-1);
		} while (m_ePreviousMiniObjective == eObjective);

		if (pszObjective)
		{
			if (FStrEq(pszObjective, "ctb"))
				eObjective = MINIOBJECTIVE_BRIEFCASE;
			else if (FStrEq(pszObjective, "briefcase"))
				eObjective = MINIOBJECTIVE_BRIEFCASE;
			else if (FStrEq(pszObjective, "bounty"))
				eObjective = MINIOBJECTIVE_BOUNTY;
			else if (FStrEq(pszObjective, "wanted"))
				eObjective = MINIOBJECTIVE_BOUNTY;
			else if (FStrEq(pszObjective, "race"))
				eObjective = MINIOBJECTIVE_RATRACE;
			else if (FStrEq(pszObjective, "ratrace"))
				eObjective = MINIOBJECTIVE_RATRACE;
		}

		bResult = false;
		if (eObjective == MINIOBJECTIVE_BRIEFCASE)
			bResult = SetupMiniObjective_Briefcase();
		else if (eObjective == MINIOBJECTIVE_BOUNTY)
			bResult = SetupMiniObjective_Bounty();
		else if (eObjective == MINIOBJECTIVE_RATRACE)
			bResult = SetupMiniObjective_RatRace();
		else
			AssertMsg(false, "Unknown mini objective to set up.");

		if (bResult)
			break;
	}

	if (!bResult)
		return;

	m_eCurrentMiniObjective = m_ePreviousMiniObjective = eObjective;

	CSDKPlayer::SendBroadcastNotice(GetNoticeForMiniObjective(m_eCurrentMiniObjective));

	CSDKPlayer::SendBroadcastSound("MiniObjective.Begin");
}

notice_t CSDKGameRules::GetNoticeForMiniObjective(miniobjective_t eObjective)
{
	if (eObjective == MINIOBJECTIVE_BRIEFCASE)
		return NOTICE_CAPTURE_BRIEFCASE;

	if (eObjective == MINIOBJECTIVE_BOUNTY)
		return NOTICE_BOUNTY_ON_PLAYER;

	if (eObjective == MINIOBJECTIVE_RATRACE)
		return NOTICE_RATRACE_START;

	AssertMsg(false, "Unknown notice for objective.");
	return NOTICE_CAPTURE_BRIEFCASE;
}

void CSDKGameRules::MaintainMiniObjective()
{
	if (!m_eCurrentMiniObjective)
		return;

	int iPlayers = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		iPlayers++;
	}

	if (iPlayers == 0)
	{
		CleanupMiniObjective();
		return;
	}

	if (m_eCurrentMiniObjective == MINIOBJECTIVE_BRIEFCASE)
		MaintainMiniObjective_Briefcase();
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_BOUNTY)
		MaintainMiniObjective_Bounty();
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_RATRACE)
		MaintainMiniObjective_RatRace();
	else
		AssertMsg(false, "Unknown mini objective to maintain.");
}

void CSDKGameRules::CleanupMiniObjective()
{
	if (!m_eCurrentMiniObjective)
		return;

	if (m_eCurrentMiniObjective == MINIOBJECTIVE_BRIEFCASE)
		CleanupMiniObjective_Briefcase();
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_BOUNTY)
		CleanupMiniObjective_Bounty();
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_RATRACE)
		CleanupMiniObjective_RatRace();
	else
		AssertMsg(false, "Unknown mini objective to maintain.");

	m_eCurrentMiniObjective = MINIOBJECTIVE_NONE;

	float flNextObjectiveTime = da_miniobjective_time.GetFloat();
	if (IsTeamplay())
		flNextObjectiveTime = da_miniobjective_teamplay_time.GetFloat();

	m_flNextMiniObjectiveStartTime = gpGlobals->curtime + (flNextObjectiveTime + RandomFloat(-1, 1)) * 60;
}

void CSDKGameRules::RestartMiniObjective()
{
	const char* objective_name = NULL;
	if (m_eCurrentMiniObjective == MINIOBJECTIVE_BRIEFCASE)
		objective_name = "ctb";
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_BOUNTY)
		objective_name = "wanted";
	else if (m_eCurrentMiniObjective == MINIOBJECTIVE_RATRACE)
		objective_name = "ratrace";

	CleanupMiniObjective();

	if (!objective_name)
		return;

	for (int i = 0; i < 3; i++)
	{
		StartMiniObjective(objective_name);
		if (m_eCurrentMiniObjective)
			break;
	}
}

void CSDKGameRules::GiveMiniObjectiveRewardTeam(CSDKPlayer* pTeam)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (PlayerRelationship(pPlayer, pTeam) == GR_TEAMMATE)
			GiveMiniObjectiveRewardPlayer(pPlayer);
	}
}

void CSDKGameRules::GiveMiniObjectiveRewardPlayer(CSDKPlayer* pPlayer)
{
	pPlayer->m_Shared.m_bSuperSkill = true;

	ConVarRef da_stylemeteractivationcost("da_stylemeteractivationcost");
	pPlayer->AddStylePoints(da_stylemeteractivationcost.GetFloat() + 1, STYLE_SOUND_LARGE, ANNOUNCEMENT_STYLISH, STYLE_POINT_STYLISH);

	pPlayer->SetStylePoints(da_stylemeteractivationcost.GetFloat());

	if (!pPlayer->IsStyleSkillActive())
		pPlayer->ActivateMeter();

	if ( pPlayer->Weapon_OwnsThisType("weapon_grenade") )
		pPlayer->CBasePlayer::GiveAmmo(1, "grenades");
	else
		pPlayer->GiveNamedItem( "weapon_grenade" );

	pPlayer->GiveSlowMo(3);

	// Make sure the player's health is 150%
	pPlayer->TakeHealth(150, 0);

	pPlayer->m_bHasSuperSlowMo = true;
}

bool CSDKGameRules::SetupMiniObjective_Briefcase()
{
	// Find a spawn point to place the briefcase in.

	CUtlVector<CBaseEntity*> apBriefcaseSpawnPoints;

	CBaseEntity* pSpot = NULL;
	while ((pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_deathmatch" )) != NULL)
	{
		bool bUse = true;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
			if (!pPlayer)
				continue;

			float flDistance = (pSpot->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

			if (flDistance < 200)
			{
				bUse = false;
				break;
			}

			if (flDistance < 500 && pPlayer->IsVisible(pSpot->GetAbsOrigin(), true))
			{
				bUse = false;
				break;
			}
		}

		if (!bUse)
			continue;

		apBriefcaseSpawnPoints.AddToTail(pSpot);
	}

	if (apBriefcaseSpawnPoints.Count() == 0)
		return false;

	RandomSeed((int)(gpGlobals->curtime*1000));
	int iSpot = RandomInt(0, apBriefcaseSpawnPoints.Count()-1);

	if (!ChooseRandomCapturePoint(apBriefcaseSpawnPoints[iSpot]->GetAbsOrigin()))
		return false;

	CBriefcase* pBriefcase = (CBriefcase*)CreateEntityByName("da_briefcase");
	pBriefcase->SetAbsOrigin(apBriefcaseSpawnPoints[iSpot]->GetAbsOrigin() + Vector(0, 0, 50));
	pBriefcase->Spawn();

	pBriefcase->Touch();

	m_hBriefcase = pBriefcase;

	return true;
}

void CSDKGameRules::MaintainMiniObjective_Briefcase()
{
	if (!m_hBriefcase)
	{
		CleanupMiniObjective();
		return;
	}

	if (!m_hBriefcase->GetOwnerEntity() && gpGlobals->curtime > m_hBriefcase->GetLastTouchedTime() + 60)
	{
		if (m_hBriefcase->GetAbsOrigin().z < SDKGameRules()->GetLowestSpawnPoint().z - 400)
			SDKGameRules()->RestartMiniObjective();
		else
			CleanupMiniObjective();
		return;
	}
}

void CSDKGameRules::CleanupMiniObjective_Briefcase()
{
	UTIL_Remove(m_hBriefcase);
	UTIL_Remove(m_hCaptureZone);
}

bool CSDKGameRules::ChooseRandomCapturePoint(Vector vecBriefcaseLocation)
{
	CUtlVector<CBaseEntity*> apCapturePoints;

	float flAverageDistance = 0;
	CBaseEntity* pSpot = NULL;
	while ((pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_deathmatch" )) != NULL)
	{
		apCapturePoints.AddToTail(pSpot);

		flAverageDistance += (pSpot->GetAbsOrigin() - vecBriefcaseLocation).Length();
	}

	flAverageDistance /= apCapturePoints.Count();

	// Valid capture points are the farthest ones from the spawn point.
	CUtlVector<CBaseEntity*> apValidCapturePoints;

	for (int i = 0; i < apCapturePoints.Count(); i++)
	{
		if ((apCapturePoints[i]->GetAbsOrigin() - vecBriefcaseLocation).Length() < flAverageDistance)
			continue;

		apValidCapturePoints.AddToTail(apCapturePoints[i]);
	}

	if (apValidCapturePoints.Count() == 0)
		return false;

	if (!!m_hCaptureZone)
		UTIL_Remove(m_hCaptureZone);

	int iCapture = RandomInt(0, apValidCapturePoints.Count()-1);

	CBriefcaseCaptureZone* pCaptureZone = (CBriefcaseCaptureZone*)CreateEntityByName("da_briefcase_capture");
	pCaptureZone->SetAbsOrigin(apValidCapturePoints[iCapture]->GetAbsOrigin());
	pCaptureZone->Spawn();

	m_hCaptureZone = pCaptureZone;

	return true;
}

void CSDKGameRules::PlayerCapturedBriefcase(CSDKPlayer* pPlayer)
{
	if (pPlayer)
	{
		GiveMiniObjectiveRewardTeam(pPlayer);

		CSDKPlayer::SendBroadcastNotice(NOTICE_PLAYER_CAPTURED_BRIEFCASE, pPlayer);
		CSDKPlayer::SendBroadcastSound("MiniObjective.BriefcaseCapture");
	}

	CleanupMiniObjective();
}

int PlayerKDCompare(CSDKPlayer*const* l, CSDKPlayer*const* r)
{
	float flLRatio = (*l)->GetDKRatio();
	float flRRatio = (*r)->GetDKRatio();

	return flLRatio > flRRatio;
}

bool CSDKGameRules::SetupMiniObjective_Bounty()
{
	// Find a player with a high K:D ratio.

	CUtlVector<CSDKPlayer*> apPlayers;

	int iPlayingPlayers = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
			continue;

		iPlayingPlayers++;

#ifndef _DEBUG
		// Never choose a bot instead of a player
		if (pPlayer->IsBot())
			continue;
#endif

		if (!pPlayer->IsAlive())
			continue;

		if (pPlayer->m_iDeaths + pPlayer->m_iKills < 5)
			continue;

		if (pPlayer->m_Shared.m_bSuperFalling)
			continue;

		apPlayers.AddToTail(pPlayer);
	}

	// Need a few players before this mode is any fun.
	if (iPlayingPlayers < 4)
		return false;

	if (apPlayers.Count() == 0)
		return false;

	apPlayers.Sort(&PlayerKDCompare);

	if (iPlayingPlayers <= 6)
	{
		if (apPlayers.Count() >= 2)
			// Six or fewer players, choose from top 2
			apPlayers.RemoveMultipleFromTail(apPlayers.Count()-2);
	}
	else
	{
		if (apPlayers.Count() >= 3)
			// More players, choose from top 3
			apPlayers.RemoveMultipleFromTail(apPlayers.Count()-3);
	}

	RandomSeed((int)(gpGlobals->curtime*1000));
	CSDKPlayer* pChosen = apPlayers[RandomInt(0, apPlayers.Count()-1)];

	m_hBountyPlayer = pChosen;

	GiveMiniObjectiveRewardPlayer(pChosen);

	if (IsTeamplay())
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
			if (!pPlayer)
				continue;

			if (PlayerRelationship(pPlayer, pChosen) == GR_TEAMMATE)
			{
				CSingleUserRecipientFilter filter(pPlayer);
				filter.MakeReliable();

				UserMessageBegin( filter, "Notice" );
					WRITE_LONG( NOTICE_BOUNTY_PROTECT_PLAYER );
					WRITE_BYTE( pChosen->entindex() );
				MessageEnd();
			}
			else
			{
				CSingleUserRecipientFilter filter(pPlayer);
				filter.MakeReliable();

				UserMessageBegin( filter, "Notice" );
					WRITE_LONG( NOTICE_BOUNTY_ON_PLAYER );
					WRITE_BYTE( pChosen->entindex() );
				MessageEnd();
			}
		}
	}
	else
		CSDKPlayer::SendBroadcastNotice(NOTICE_BOUNTY_ON_PLAYER, pChosen);

	return true;
}

void CSDKGameRules::MaintainMiniObjective_Bounty()
{
	if (!m_hBountyPlayer)
	{
		CleanupMiniObjective();
		return;
	}

	if (!m_hBountyPlayer->IsAlive())
	{
		CleanupMiniObjective();
		return;
	}
}

void CSDKGameRules::CleanupMiniObjective_Bounty()
{
	m_hBountyPlayer = NULL;
}

static Vector g_vecSortPoint;

int DistanceToPoint(CBaseEntity*const* l, CBaseEntity*const* r)
{
	float flLDistanceSqr = ((*l)->GetAbsOrigin() - g_vecSortPoint).LengthSqr();
	float flRDistanceSqr = ((*r)->GetAbsOrigin() - g_vecSortPoint).LengthSqr();

	return flLDistanceSqr < flRDistanceSqr;
}

bool CSDKGameRules::SetupMiniObjective_RatRace()
{
	if (IsTeamplay())
		return false;

	// Find a spawn point to place the briefcase in.

	CUtlVector<CBaseEntity*> apWaypoints;

	CBaseEntity* pSpot = NULL;
	while ((pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_deathmatch" )) != NULL)
	{
		bool bUse = true;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
			if (!pPlayer)
				continue;

			float flDistance = (pSpot->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

			if (flDistance < 200)
			{
				bUse = false;
				break;
			}

			if (flDistance < 500 && pPlayer->IsVisible(pSpot->GetAbsOrigin(), true))
			{
				bUse = false;
				break;
			}
		}

		if (!bUse)
			continue;

		apWaypoints.AddToTail(pSpot);
	}

	if (apWaypoints.Count() < 3)
		return false;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		pPlayer->m_iRaceWaypoint = 0;
	}

	RandomSeed((int)(gpGlobals->curtime*1000));
	int iSpot1 = RandomInt(0, apWaypoints.Count()-1);

	m_hRaceWaypoint1 = (CRatRaceWaypoint*)CreateEntityByName("da_ratrace_waypoint");
	m_hRaceWaypoint1->SetAbsOrigin(apWaypoints[iSpot1]->GetAbsOrigin());
	m_hRaceWaypoint1->SetWaypoint(0);
	m_hRaceWaypoint1->Spawn();

	apWaypoints.Remove(iSpot1);

	g_vecSortPoint = m_hRaceWaypoint1->GetAbsOrigin();
	apWaypoints.Sort(&DistanceToPoint);

	int iSpot2;
	if (apWaypoints.Count() < 6)
		iSpot2 = 0;
	else
		iSpot2 = RandomInt(0, 2);

	m_hRaceWaypoint2 = (CRatRaceWaypoint*)CreateEntityByName("da_ratrace_waypoint");
	m_hRaceWaypoint2->SetAbsOrigin(apWaypoints[iSpot2]->GetAbsOrigin());
	m_hRaceWaypoint2->SetWaypoint(1);
	m_hRaceWaypoint2->Spawn();

	apWaypoints.Remove(iSpot2);

	g_vecSortPoint = m_hRaceWaypoint2->GetAbsOrigin();
	apWaypoints.Sort(&DistanceToPoint);

	int iSpot3;
	if (apWaypoints.Count() < 6)
		iSpot3 = 0;
	else
		iSpot3 = RandomInt(0, 2);

	m_hRaceWaypoint3 = (CRatRaceWaypoint*)CreateEntityByName("da_ratrace_waypoint");
	m_hRaceWaypoint3->SetAbsOrigin(apWaypoints[iSpot3]->GetAbsOrigin());
	m_hRaceWaypoint3->SetWaypoint(2);
	m_hRaceWaypoint3->Spawn();

	m_flLastPlayerWaypointTouch = gpGlobals->curtime;

	return true;
}

void CSDKGameRules::MaintainMiniObjective_RatRace()
{
	if (gpGlobals->curtime - m_flLastPlayerWaypointTouch > 120)
		CleanupMiniObjective();
}

void CSDKGameRules::CleanupMiniObjective_RatRace()
{
	Assert(m_ahWaypoint1RaceLeaders.Count() == m_ahWaypoint2RaceLeaders.Count());
	for (int i = 0; i < m_ahWaypoint1RaceLeaders.Count(); i++)
	{
		m_ahWaypoint1RaceLeaders.GetForModify(i).Set(NULL);
		m_ahWaypoint2RaceLeaders.GetForModify(i).Set(NULL);
	}

	UTIL_Remove(m_hRaceWaypoint1);
	UTIL_Remove(m_hRaceWaypoint2);
	UTIL_Remove(m_hRaceWaypoint3);
}

void CSDKGameRules::PlayerReachedWaypoint(CSDKPlayer* pPlayer, CRatRaceWaypoint* pWaypoint)
{
	if (!pPlayer || !pWaypoint)
		return;

	if (pPlayer->m_iRaceWaypoint == pWaypoint->GetWaypoint())
	{
		pPlayer->m_iRaceWaypoint++;

		pPlayer->AddStylePoints(ConVarRef("da_stylemeteractivationcost").GetFloat()/5, STYLE_SOUND_LARGE, ANNOUNCEMENT_NONE, STYLE_POINT_LARGE);

		if (pPlayer->m_iRaceWaypoint == 1)
		{
			if (!m_ahWaypoint2RaceLeaders[0] && !m_ahWaypoint1RaceLeaders[0])
			{
				CSDKPlayer::SendBroadcastSound("MiniObjective.Begin");
				CSDKPlayer::SendBroadcastNotice(NOTICE_RATRACE_PLAYER_LEAD, pPlayer);
			}
			else
			{
				CSingleUserRecipientFilter filter(pPlayer);
				filter.MakeReliable();

				UserMessageBegin( filter, "Notice" );
					WRITE_LONG( NOTICE_RATRACE_PLAYER_WAYPOINT_1_REACHED );
					WRITE_BYTE( 0 );
				MessageEnd();
			}

			WaypointLeadersPush(m_ahWaypoint1RaceLeaders, pPlayer);

			if (!pPlayer->IsBot())
				m_flLastPlayerWaypointTouch = gpGlobals->curtime;
		}
		else if (pPlayer->m_iRaceWaypoint == 2)
		{
			if (!m_ahWaypoint2RaceLeaders[0])
			{
				CSDKPlayer::SendBroadcastSound("MiniObjective.Begin");
				CSDKPlayer::SendBroadcastNotice(NOTICE_RATRACE_PLAYER_POINT_2, pPlayer);
			}
			else
			{
				CSingleUserRecipientFilter filter(pPlayer);
				filter.MakeReliable();

				UserMessageBegin( filter, "Notice" );
					WRITE_LONG( NOTICE_RATRACE_PLAYER_WAYPOINT_2_REACHED );
					WRITE_BYTE( 0 );
				MessageEnd();
			}

			WaypointLeadersPush(m_ahWaypoint2RaceLeaders, pPlayer);
			RemovePlayerFromLeaders(m_ahWaypoint1RaceLeaders, pPlayer);

			if (!pPlayer->IsBot())
				m_flLastPlayerWaypointTouch = gpGlobals->curtime;
		}
		else if (pPlayer->m_iRaceWaypoint == 3)
		{
			CSDKPlayer::SendBroadcastNotice(NOTICE_RATRACE_OVER, pPlayer);
			GiveMiniObjectiveRewardPlayer(pPlayer);
			CleanupMiniObjective();
		}
	}
}

template <typename T>
void CSDKGameRules::WaypointLeadersPush(T& ahWaypointLeaders, CSDKPlayer* pPlayer)
{
	for (int i = 0; i < ahWaypointLeaders.Count(); i++)
	{
		if (ahWaypointLeaders.Get(i) == NULL)
		{
			ahWaypointLeaders.GetForModify(i) = pPlayer;
			DebugCheckLeaders(ahWaypointLeaders);
			return;
		}
	}

	// If there's no room in the list for the player that's okay,
	// the list is only for displaying leaders on the HUD.
	// CSDKPlayer::m_iRaceWaypoint is what determines the winner.
	DebugCheckLeaders(ahWaypointLeaders);
}

template <typename T>
void CSDKGameRules::RemovePlayerFromLeaders(T& ahWaypointLeaders, CSDKPlayer* pPlayer)
{
	for (int i = 0; i < ahWaypointLeaders.Count(); i++)
	{
		if (ahWaypointLeaders.Get(i) == pPlayer)
			ahWaypointLeaders.GetForModify(i) = NULL;
	}

	CompressLeaders(ahWaypointLeaders);
}

template <typename T>
void CSDKGameRules::CompressLeaders(T& ahWaypointLeaders)
{
	for (int i = 0; i < ahWaypointLeaders.Count(); i++)
	{
		if (ahWaypointLeaders.Get(i) == NULL)
		{
			// Find the next occupied space.
			bool bFound = false;
			for (int j = i+1; j < ahWaypointLeaders.Count(); j++)
			{
				if (ahWaypointLeaders.Get(j) != NULL)
				{
					ahWaypointLeaders.GetForModify(i) = ahWaypointLeaders[j];
					ahWaypointLeaders.GetForModify(j) = NULL;
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				// The list is compressed. Return.
				DebugCheckLeaders(ahWaypointLeaders);
				return;
			}
		}
	}

	DebugCheckLeaders(ahWaypointLeaders);
}

template <typename T>
void CSDKGameRules::DebugCheckLeaders(T& ahWaypointLeaders)
{
#ifndef _DEBUG
	return;
#endif

	// Check leaders list invariants
	bool bInFront = true;

	for (int i = 0; i < ahWaypointLeaders.Count(); i++)
	{
		if (bInFront && !ahWaypointLeaders.Get(i))
			bInFront = false;

		if (!bInFront && ahWaypointLeaders.Get(i))
			AssertMsg(false, "Leaders list is not compact.");
	}
}

void CC_MiniObjective(const CCommand &args)
{
	if (args.ArgC() > 1)
		SDKGameRules()->StartMiniObjective(args[1]);
	else
		SDKGameRules()->StartMiniObjective();
}

static ConCommand da_miniobjective("da_miniobjective", CC_MiniObjective, "", FCVAR_GAMEDLL|FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT);
#endif

CBriefcase* CSDKGameRules::GetBriefcase() const
{
	return m_hBriefcase;
}

CBriefcaseCaptureZone* CSDKGameRules::GetCaptureZone() const
{
	return m_hCaptureZone;
}

CSDKPlayer* CSDKGameRules::GetBountyPlayer() const
{
	return m_hBountyPlayer;
}

CRatRaceWaypoint* CSDKGameRules::GetWaypoint(int i) const
{
	if (i == 1)
		return m_hRaceWaypoint2;

	if (i == 2)
		return m_hRaceWaypoint3;

	return m_hRaceWaypoint1;
}

CSDKPlayer* CSDKGameRules::GetLeader() const
{
	// If anyone has reached the second point, the first person to do so is the leader.
	if (m_ahWaypoint2RaceLeaders[0])
		return m_ahWaypoint2RaceLeaders[0];

	// Otherwise whoever the first person is to reach the second point is the leader.
	return m_ahWaypoint1RaceLeaders[0];
}

CSDKPlayer* CSDKGameRules::GetFrontRunner1() const
{
	if (m_ahWaypoint2RaceLeaders[0])
	{
		// If someone has reached the second point, then they are the leader.
		if (m_ahWaypoint2RaceLeaders[1])
			// Second place position is whoever the next person was to reach the second point.
			return m_ahWaypoint2RaceLeaders[1];
		else
			// If nobody has reached the second point, second place is whoever reached the first point first.
			return m_ahWaypoint1RaceLeaders[0];
	}

	// If nobody has reached the second point...
	return m_ahWaypoint1RaceLeaders[1];
}

CSDKPlayer* CSDKGameRules::GetFrontRunner2() const
{
	if (m_ahWaypoint2RaceLeaders[0])
	{
		// If someone has reached the second point, then they are the leader.
		if (m_ahWaypoint2RaceLeaders[1])
		{
			// The second place person has reached the second point.
			if (m_ahWaypoint2RaceLeaders[2])
				// If there is a third person to reach the second point, they are in third place.
				return m_ahWaypoint2RaceLeaders[2];
			else
				// Otherwise third place is the first person to reach the first point.
				return m_ahWaypoint1RaceLeaders[0];
		}
		else
			// Only one person has reached the second point.
			// Second place is the first person at the first point.
			// Third place is the second person at the first point.
			return m_ahWaypoint2RaceLeaders[2];
	}

	// If nobody has reached the second point...
	return m_ahWaypoint1RaceLeaders[2];
}

#ifndef CLIENT_DLL
class CDAIssue : public CBaseIssue
{
public:
	CDAIssue(const char* pszName) : CBaseIssue(pszName) {}

public:
	virtual void OnVoteFailed( int iEntityHoldingVote )
	{
#ifdef WITH_DATA_COLLECTION
		DataManager().VoteFailed(m_szTypeString);
#endif

		CBaseIssue::OnVoteFailed(iEntityHoldingVote);
	}

	virtual void ExecuteCommand( void )
	{
		const char* pszDetails = m_szDetailsString;
		if (FStrEq(m_szTypeString, "kick"))
		{
			CBasePlayer *pPlayer = UTIL_PlayerByUserId( atoi(pszDetails) );
			if (!pPlayer)
				return;

			CSteamID id;
			pPlayer->GetSteamID(&id);

			if (pPlayer->IsBot())
				pszDetails = UTIL_VarArgs("%s (BOT)", pPlayer->GetPlayerName());
			else
				pszDetails = UTIL_VarArgs("%s (Account id: %d)", pPlayer->GetPlayerName(), (int)id.GetAccountID());
		}

#ifdef WITH_DATA_COLLECTION
		DataManager().VotePassed(m_szTypeString, pszDetails);
#endif
	}
};

class CTeamplayModeVoteIssue : public CDAIssue
{
public:
	CTeamplayModeVoteIssue()
		: CDAIssue("teamplay_mode")
	{
	}

public:
	virtual bool		IsEnabled( void ) { return true; }
	virtual const char *GetDisplayString( void ) { return "#DA_VoteIssue_Teamplay_Display"; }
	virtual const char *GetVotePassedString( void ) { return "#DA_VoteIssue_Teamplay_Passed"; }

	virtual void		ExecuteCommand( void )
	{
		CDAIssue::ExecuteCommand();

		ConVarRef mp_teamplay("mp_teamplay");
		mp_teamplay.SetValue(!mp_teamplay.GetBool());

		ConVarRef nextlevel("nextlevel");
		nextlevel.SetValue(STRING(gpGlobals->mapname));

		SDKGameRules()->ChangeLevel();
	}

	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}
};

class CNextMapVoteIssue : public CDAIssue
{
public:
	CNextMapVoteIssue()
		: CDAIssue("nextlevel")
	{
	}

public:
	virtual bool		IsEnabled( void ) { return false; }
	virtual bool		IsYesNoVote( void ) { return false; }
	virtual const char *GetDisplayString( void ) { return "#DA_VoteIssue_NextLevel_Display "; }
	virtual const char *GetVotePassedString( void ) { return "#DA_VoteIssue_NextLevel_Passed"; }

	virtual bool        GetVoteOptions( CUtlVector <const char*> &vecNames )
	{
		CUtlVector<char*> apszMapList;
		apszMapList.AddVectorToTail(SDKGameRules()->GetMapList());

		static char szNextMap[MAX_MAP_NAME];
		SDKGameRules()->GetNextLevelName( szNextMap, sizeof( szNextMap ) );
		vecNames.AddToTail( szNextMap );

		while (vecNames.Count() < 5)
		{
			if (!apszMapList.Count())
				break;

			int iRandom = RandomInt(0, apszMapList.Count()-1);

			if (FStrEq(apszMapList[iRandom], szNextMap))
			{
				apszMapList.Remove(iRandom);
				continue;
			}

			vecNames.AddToTail( apszMapList[iRandom] );
			apszMapList.Remove(iRandom);
		}

		return true;
	}

	virtual void		ExecuteCommand( void )
	{
		CDAIssue::ExecuteCommand();

		ConVarRef nextlevel("nextlevel");
		nextlevel.SetValue(m_szDetailsString);
	}

	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}
};

class CChangelevelVoteIssue : public CDAIssue
{
public:
	CChangelevelVoteIssue()
		: CDAIssue("changelevel")
	{
	}

public:
	virtual bool		IsEnabled( void ) { return true; }
	virtual const char *GetDisplayString( void ) { return "#DA_VoteIssue_ChangeLevel_Display"; }
	virtual const char *GetVotePassedString( void ) { return "#DA_VoteIssue_ChangeLevel_Passed"; }

	virtual void		ExecuteCommand( void )
	{
		CDAIssue::ExecuteCommand();

		ConVarRef nextlevel("nextlevel");
		nextlevel.SetValue(m_szDetailsString);
		SDKGameRules()->ChangeLevel();
	}

	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}
};

ConVar da_vote_kick_plurality("da_vote_kick_plurality", "0.7", FCVAR_GAMEDLL, "What percentage of players is required to vote yes for a kick vote to pass?");

class CKickPlayerVoteIssue : public CDAIssue
{
private:
	CUtlString networkIDString;

public:
	CKickPlayerVoteIssue()
		: CDAIssue("kick")
	{
	}

public:
	virtual bool		IsEnabled( void ) { return true; }
	virtual const char *GetDisplayString( void ) { return "#DA_VoteIssue_Kick_Display"; }
	virtual const char *GetVotePassedString( void ) { return "#DA_VoteIssue_Kick_Passed"; }
	virtual float       GetRequiredPlurality() { return da_vote_kick_plurality.GetFloat(); }

	virtual bool		GetVoteOptions(CUtlVector <const char*> &vecNames)
	{
		if (!CDAIssue::GetVoteOptions(vecNames))
			return false;

		vecNames.AddToTail("Abstain");

		return true;
	}

	virtual void		ExecuteCommand(void)
	{
		CDAIssue::ExecuteCommand();

		int iUserID = atoi(GetDetailsString());
		CBasePlayer* pPlayer = UTIL_PlayerByUserId(iUserID);
		if (pPlayer && pPlayer->IsBot())
		{
			ConVarRef bot_quota("bot_quota");
			bot_quota.SetValue(bot_quota.GetInt()-1);
			return;
		}

		engine->ServerCommand(UTIL_VarArgs( "banid 30 %s kick\n", networkIDString.String() ));
		engine->ServerCommand("writeip\n");
		engine->ServerCommand("writeid\n");
	}

	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}

	virtual void SetIssueDetails(const char *pszDetails)
	{
		CDAIssue::SetIssueDetails(pszDetails);

		int iUserID = atoi(pszDetails);
		CBasePlayer* pPlayer = UTIL_PlayerByUserId(iUserID);
		networkIDString = pPlayer->GetNetworkIDString();
	}
};

class CAddBotVoteIssue : public CDAIssue
{
public:
	CAddBotVoteIssue()
		: CDAIssue("addbot")
	{
	}

public:
	virtual bool		IsEnabled( void ) { return true; }
	virtual const char *GetDisplayString( void ) { return "#DA_VoteIssue_AddBot_Display"; }
	virtual const char *GetVotePassedString( void ) { return "#DA_VoteIssue_AddBot_Passed"; }

	virtual void		ExecuteCommand( void )
	{
		CDAIssue::ExecuteCommand();

		ConVarRef bot_quota("bot_quota");
		bot_quota.SetValue(bot_quota.GetInt()+1);
	}

	virtual void		ListIssueDetails( CBasePlayer *pForWhom )
	{
		AssertMsg(false, "Unimplemented");
		ClientPrint( pForWhom, HUD_PRINTCONSOLE, "Nothing here.\n" );
	}
};

void RegisterVoteIssues()
{
	CVoteController* pController = (CVoteController*)CreateEntityByName( "vote_controller" );
	pController->Spawn();

	//new CTeamplayModeVoteIssue();
	new CNextMapVoteIssue();
	new CChangelevelVoteIssue();
	new CKickPlayerVoteIssue();
	new CAddBotVoteIssue();
}
#endif

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning

#include "cbase.h"

#include "da_bot.h"
#include "nav_area.h"
#include "sdk_gamerules.h"
#include "shared_util.h"
#include "KeyValues.h"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef _WIN32
#pragma warning (disable:4701)				// disable warning that variable *may* not be initialized 
#endif

CBotManager *TheBots = NULL;

bool CDABotManager::m_isMapDataLoaded = false;

int g_nClientPutInServerOverrides = 0;


void DrawOccupyTime( void );
ConVar bot_show_occupy_time( "bot_show_occupy_time", "0", FCVAR_GAMEDLL | FCVAR_CHEAT, "Show when each nav area can first be reached by each team." );

void DrawBattlefront( void );
ConVar bot_show_battlefront( "bot_show_battlefront", "0", FCVAR_GAMEDLL | FCVAR_CHEAT, "Show areas where rushing players will initially meet." );

int UTIL_CFSBotsInGame( void );

ConVar bot_join_delay( "bot_join_delay", "0", FCVAR_GAMEDLL, "Prevents bots from joining the server for this many seconds after a map change." );

/**
 * Determine whether bots can be used or not
 */
inline bool AreBotsAllowed()
{
	// If they pass in -nobots, don't allow bots.  This is for people who host servers, to
	// allow them to disallow bots to enforce CPU limits.
	const char *nobots = CommandLine()->CheckParm( "-nobots" );
	if ( nobots )
	{
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
void InstallBotControl( void )
{
	if ( TheBots != NULL )
		delete TheBots;

	TheBots = new CDABotManager;
}


//--------------------------------------------------------------------------------------------------------------
void RemoveBotControl( void )
{
	if ( TheBots != NULL )
		delete TheBots;

	TheBots = NULL;
}


//--------------------------------------------------------------------------------------------------------------
CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
{
	CBasePlayer *pPlayer = TheBots->AllocateAndBindBotEntity( pEdict );
	if ( pPlayer )
	{
		pPlayer->SetPlayerName( playername );
	}
	++g_nClientPutInServerOverrides;

	return pPlayer;
}

//--------------------------------------------------------------------------------------------------------------
// Constructor
CDABotManager::CDABotManager()
{
	m_zoneCount = 0;
	m_serverActive = false;

	m_roundStartTimestamp = 0.0f;

	m_eventListenersEnabled = true;
	m_commonEventListeners.AddToTail( &m_PlayerFootstepEvent );
	m_commonEventListeners.AddToTail( &m_PlayerRadioEvent );
	m_commonEventListeners.AddToTail( &m_PlayerFallDamageEvent );
	m_commonEventListeners.AddToTail( &m_DoorMovingEvent );
	m_commonEventListeners.AddToTail( &m_BreakPropEvent );
	m_commonEventListeners.AddToTail( &m_BreakBreakableEvent );
	m_commonEventListeners.AddToTail( &m_WeaponFireEvent );
	m_commonEventListeners.AddToTail( &m_WeaponFireOnEmptyEvent );
	m_commonEventListeners.AddToTail( &m_WeaponReloadEvent );
	m_commonEventListeners.AddToTail( &m_WeaponZoomEvent );
	m_commonEventListeners.AddToTail( &m_BulletImpactEvent );
	m_commonEventListeners.AddToTail( &m_GrenadeBounceEvent );
	m_commonEventListeners.AddToTail( &m_NavBlockedEvent );

	TheBotPhrases = new BotPhraseManager;
	TheBotProfiles = new BotProfileManager;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when a new round begins
 */
void CDABotManager::RestartRound( void )
{
	// extend
	CBotManager::RestartRound();

	m_lastSeenEnemyTimestamp = -9999.9f;

	m_roundStartTimestamp = gpGlobals->curtime;

	// randomly decide if defensive team wants to "rush" as a whole
	const float defenseRushChance = 33.3f;	// 25.0f;
	m_isDefenseRushing = (RandomFloat( 0.0f, 100.0f ) <= defenseRushChance) ? true : false;

	TheBotPhrases->OnRoundRestart();

	m_isRoundOver = false;
}

//--------------------------------------------------------------------------------------------------------------

void UTIL_DrawBox( Extent *extent, int lifetime, int red, int green, int blue )
{
	int darkRed = red/2;
	int darkGreen = green/2;
	int darkBlue = blue/2;

	Vector v[8];
	v[0].x = extent->lo.x; v[0].y = extent->lo.y; v[0].z = extent->lo.z;
	v[1].x = extent->hi.x; v[1].y = extent->lo.y; v[1].z = extent->lo.z;
	v[2].x = extent->hi.x; v[2].y = extent->hi.y; v[2].z = extent->lo.z;
	v[3].x = extent->lo.x; v[3].y = extent->hi.y; v[3].z = extent->lo.z;
	v[4].x = extent->lo.x; v[4].y = extent->lo.y; v[4].z = extent->hi.z;
	v[5].x = extent->hi.x; v[5].y = extent->lo.y; v[5].z = extent->hi.z;
	v[6].x = extent->hi.x; v[6].y = extent->hi.y; v[6].z = extent->hi.z;
	v[7].x = extent->lo.x; v[7].y = extent->hi.y; v[7].z = extent->hi.z;

	static int edge[] = 
	{
		1, 2, 3, 4, -1,
		5, 6, 7, 8, -5,
		1, -5,
		2, -6,
		3, -7,
		4, -8,
		0
	};

	Vector from, to;
	bool restart = true;
	for( int i=0; edge[i] != 0; ++i )
	{
		if (restart)
		{
			to = v[ edge[i]-1 ];
			restart = false;
			continue;
		}
		
		from = to;

		int index = edge[i];
		if (index < 0)
		{
			restart = true;
			index = -index;
		}

		to = v[ index-1 ];

		NDebugOverlay::Line( from, to, darkRed, darkGreen, darkBlue, true, 0.1f );
		NDebugOverlay::Line( from, to, red, green, blue, false, 0.15f );
	}
}

//--------------------------------------------------------------------------------------------------------------
void CDABotManager::EnableEventListeners( bool enable )
{
	if ( m_eventListenersEnabled == enable )
	{
		return;
	}

	m_eventListenersEnabled = enable;

	// enable/disable the most frequent event listeners, to improve performance when no bots are present.
	for ( int i=0; i<m_commonEventListeners.Count(); ++i )
	{
		if ( enable )
		{
			gameeventmanager->AddListener( m_commonEventListeners[i], m_commonEventListeners[i]->GetEventName(), true );
		}
		else
		{
			gameeventmanager->RemoveListener( m_commonEventListeners[i] );
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Called each frame
 */
void CDABotManager::StartFrame( void )
{
	if ( !AreBotsAllowed() )
	{
		EnableEventListeners( false );
		return;
	}

	// EXTEND
	CBotManager::StartFrame();

	MaintainBotQuota();
	EnableEventListeners( UTIL_CFSBotsInGame() > 0 );

	// debug zone extent visualization
	if (cv_bot_debug.GetInt() == 5)
	{
		for( int z=0; z<m_zoneCount; ++z )
		{
			Zone *zone = &m_zone[z];

			if ( zone->m_isBlocked )
			{
				UTIL_DrawBox( &zone->m_extent, 1, 255, 0, 200 );
			}
			else
			{
				UTIL_DrawBox( &zone->m_extent, 1, 255, 100, 0 );
			}
		}
	}

	if (bot_show_occupy_time.GetBool())
	{
		DrawOccupyTime();
	}

	if (bot_show_battlefront.GetBool())
	{
		DrawBattlefront();
	}

	if ( m_checkTransientAreasTimer.IsElapsed() && !nav_edit.GetBool() )
	{
		CUtlVector< CNavArea * >& transientAreas = TheNavMesh->GetTransientAreas();
		for ( int i=0; i<transientAreas.Count(); ++i )
		{
			CNavArea *area = transientAreas[i];
			if ( area->GetAttributes() & NAV_MESH_TRANSIENT )
			{
				area->UpdateBlocked();
			}
		}

		m_checkTransientAreasTimer.Start( 2.0f );
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if the bot can use this weapon
 */
bool CDABotManager::IsWeaponUseable( const CWeaponSDKBase *weapon ) const
{
	if (weapon == NULL)
		return false;

	if (weapon->GetWeaponType() == SDK_WEAPON_BRAWL)
		return true;

	if ((!AllowPistols() && weapon->GetWeaponType() == WT_PISTOL))
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this player is on "defense"
 */
bool CDABotManager::IsOnDefense( const CSDKPlayer *player ) const
{
	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this player is on "offense"
 */
bool CDABotManager::IsOnOffense( const CSDKPlayer *player ) const
{
	return !IsOnDefense( player );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when a map has just been loaded
 */
void CDABotManager::ServerActivate( void )
{
	m_isMapDataLoaded = false;

	// load the database of bot radio chatter
	TheBotPhrases->Reset();
	TheBotPhrases->Initialize( "BotChatter.db", 0 );

	TheBotProfiles->Reset();
	TheBotProfiles->FindVoiceBankIndex( "BotChatter.db" ); // make sure default voice bank is first
	const char *filename;
	if ( false ) // g_engfuncs.pfnIsCareerMatch() )
	{
		filename = "MissionPacks/BotPackList.db";
	}
	else
	{
		filename = "BotPackList.db";
	}

	// read in the list of bot profile DBs
	FileHandle_t file = filesystem->Open( filename, "r" );

	if ( !file )
	{
		TheBotProfiles->Init( "BotProfile.db" );
	}
	else
	{
		int dataLength = filesystem->Size( filename );
		char *dataPointer = new char[ dataLength ];

		filesystem->Read( dataPointer, dataLength, file );
		filesystem->Close( file );

		const char *dataFile = SharedParse( dataPointer );
		const char *token;

		while ( dataFile )
		{
			token = SharedGetToken();
			char *clone = CloneString( token );
			TheBotProfiles->Init( clone );
			delete[] clone;
			dataFile = SharedParse( dataFile );
		}

		delete [] dataPointer;
	}

	// Now that we've parsed all the profiles, we have a list of the voice banks they're using.
	// Go back and parse the custom voice speakables.
	const BotProfileManager::VoiceBankList *voiceBanks = TheBotProfiles->GetVoiceBanks();
	for ( int i=1; i<voiceBanks->Count(); ++i )
	{
		TheBotPhrases->Initialize( (*voiceBanks)[i], i );
	}

	TheNavMesh->SetPlayerSpawnName( "info_player_deathmatch" );

	ExtractScenarioData();

	RestartRound();

	TheBotPhrases->OnMapChange();

	m_serverActive = true;
}


void CDABotManager::ServerDeactivate( void )
{
	m_serverActive = false;
}

void CDABotManager::ClientDisconnect( CBaseEntity *entity )
{
/*
	if ( FBitSet( entity->GetFlags(), FL_FAKECLIENT ) )
	{
		FREE_PRIVATE( entity );
	}
*/

	/*
	// make sure voice feedback is turned off
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pEntity );
	if ( pPlayer && pPlayer->IsBot() )
	{
		CDABot *pBot = static_cast<CDABot *>(pPlayer);
		if (pBot)
		{
			pBot->EndVoiceFeedback( true );
		}
	}
	*/
}



//--------------------------------------------------------------------------------------------------------------
/**
* Parses out bot name/template/etc params from the current ConCommand
*/
void BotArgumentsFromArgv( const CCommand &args, const char **name, weapontype_t *weaponType, BotDifficultyType *difficulty, int *team = NULL, bool *all = NULL )
{
	static char s_name[MAX_PLAYER_NAME_LENGTH];

	s_name[0] = 0;
	*name = s_name;
	*difficulty = NUM_DIFFICULTY_LEVELS;
	if ( team )
	{
		*team = TEAM_UNASSIGNED;
	}
	if ( all )
	{
		*all = false;
	}

	*weaponType = WT_NONE;

	for ( int arg=1; arg<args.ArgC(); ++arg )
	{
		bool found = false;

		const char *token = args[arg];
		if ( all && FStrEq( token, "all" ) )
		{
			*all = true;
			found = true;
		}

		for( int i=0; i<NUM_DIFFICULTY_LEVELS && !found; ++i )
		{
			if (!stricmp( BotDifficultyName[i], token ))
			{
				*difficulty = (BotDifficultyType)i;
				found = true;
			}
		}

		if ( !found )
		{
			*weaponType = CWeaponSDKBase::GetWeaponType(AliasToWeaponID( token ));
			if ( *weaponType != WT_NONE )
			{
				found = true;
			}
		}

		if ( !found )
		{
			Q_strncpy( s_name, token, sizeof( s_name ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( bot_add, "bot_add [type|difficulty|name] - Adds a bot matching the given criteria.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	const char *name;
	BotDifficultyType difficulty;
	weapontype_t weaponType;
	int team;
	BotArgumentsFromArgv( args, &name, &weaponType, &difficulty, &team );
	TheDABots()->BotAddCommand( team, FROM_CONSOLE, name, weaponType, difficulty );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Collects all bots matching the given criteria (player name, profile template name, difficulty, and team)
 */
class CollectBots
{
public:
	CollectBots( const char *name, weapontype_t weaponType, BotDifficultyType difficulty, int team )
	{
		m_name = name;
		m_difficulty = difficulty;
		m_team = team;
		m_weaponType = weaponType;
	}

	bool operator() ( CBasePlayer *player )
	{
		if ( !player->IsBot() )
		{
			return true;
		}

		CDABot *bot = dynamic_cast< CDABot * >(player);
		if ( !bot || !bot->GetProfile() )
		{
			return true;
		}

		if ( m_name && *m_name )
		{
			// accept based on name
			if ( FStrEq( m_name, bot->GetProfile()->GetName() ) )
			{
				m_bots.RemoveAll();
				m_bots.AddToTail( bot );
				return false;
			}

			// Reject based on profile template name
			if ( !bot->GetProfile()->InheritsFrom( m_name ) )
			{
				return true;
			}
		}

		// reject based on difficulty
		if ( m_difficulty != NUM_DIFFICULTY_LEVELS )
		{
			if ( !bot->GetProfile()->IsDifficulty( m_difficulty ) )
			{
				return true;
			}
		}

		// reject based on weapon preference
		if ( m_weaponType != WT_NONE )
		{
			if ( !bot->GetProfile()->GetWeaponPreferenceCount() )
			{
				return true;
			}

			if ( m_weaponType != CWeaponSDKBase::GetWeaponType( (SDKWeaponID)bot->GetProfile()->GetWeaponPreference( 0 ) ) )
			{
				return true;
			}
		}

		// A match!
		m_bots.AddToTail( bot );

		return true;
	}

	CUtlVector< CDABot * > m_bots;

private:
	const char *m_name;
	weapontype_t m_weaponType;
	BotDifficultyType m_difficulty;
	int m_team;
};

//--------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( bot_kill, "bot_kill <all> <t|ct> <type> <difficulty> <name> - Kills a specific bot, or all bots, matching the given criteria.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	const char *name;
	BotDifficultyType difficulty;
	weapontype_t weaponType;
	int team;
	bool all;

	BotArgumentsFromArgv( args, &name, &weaponType, &difficulty, &team, &all );
	if ( (!name || !*name) && team == TEAM_UNASSIGNED && difficulty == NUM_DIFFICULTY_LEVELS )
	{
		all = true;
	}

	CollectBots collector( name, weaponType, difficulty, team );
	ForEachPlayer( collector );

	for ( int i=0; i<collector.m_bots.Count(); ++i )
	{
		CDABot *bot = collector.m_bots[i];
		if ( !bot->IsAlive() )
			continue;

		bot->CommitSuicide();
		if ( !all )
		{
			return;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( bot_kick, "bot_kick <all> <t|ct> <type> <difficulty> <name> - Kicks a specific bot, or all bots, matching the given criteria.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	const char *name;
	BotDifficultyType difficulty;
	weapontype_t weaponType;
	int team;
	bool all;

	BotArgumentsFromArgv( args, &name, &weaponType, &difficulty, &team, &all );
	if ( (!name || !*name) && team == TEAM_UNASSIGNED && difficulty == NUM_DIFFICULTY_LEVELS )
	{
		all = true;
	}

	CollectBots collector( name, weaponType, difficulty, team );
	ForEachPlayer( collector );

	for ( int i=0; i<collector.m_bots.Count(); ++i )
	{
		CDABot *bot = collector.m_bots[i];
		engine->ServerCommand( UTIL_VarArgs( "kick \"%s\"\n", bot->GetPlayerName() ) );
		if ( !all )
		{
			// adjust bot quota so kicked bot is not immediately added back in
			int newQuota = cv_bot_quota.GetInt() - 1;
			cv_bot_quota.SetValue( clamp( newQuota, 0, cv_bot_quota.GetInt() ) );
			return;
		}
	}

	// adjust bot quota so kicked bot is not immediately added back in
	if ( all && (!name || !*name) && team == TEAM_UNASSIGNED && difficulty == NUM_DIFFICULTY_LEVELS )
	{
		cv_bot_quota.SetValue( 0 );
	}
	else
	{
		int newQuota = cv_bot_quota.GetInt() - collector.m_bots.Count();
		cv_bot_quota.SetValue( clamp( newQuota, 0, cv_bot_quota.GetInt() ) );
	}
}


//--------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( bot_goto_mark, "Sends a bot to the selected nav area (useful for testing navigation meshes)", FCVAR_GAMEDLL | FCVAR_CHEAT )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	// tell the first bot we find to go to our marked area
	CNavArea *area = TheNavMesh->GetMarkedArea();
	if (area)
	{
		for ( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

			if (player == NULL)
				continue;
	
			if (player->IsBot())
			{
				CDABot *bot = dynamic_cast<CDABot *>( player );

				if ( bot )
				{
					bot->MoveTo( area->GetCenter(), FASTEST_ROUTE );
				}

				break;
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
#if 0
CON_COMMAND_F( bot_memory_usage, "Reports on the bots' memory usage", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	Msg( "Memory usage:\n" );

	Msg( "  %d bytes per bot\n", sizeof(CDABot) );

	Msg( "  %d Navigation Areas @ %d bytes each = %d bytes\n", 
					TheNavMesh->GetNavAreaCount(),
					sizeof( CNavArea ),
					TheNavMesh->GetNavAreaCount() * sizeof( CNavArea ) );

	Msg( "  %d Hiding Spots @ %d bytes each = %d bytes\n", 
					TheHidingSpotList.Count(),
					sizeof( HidingSpot ),
					TheHidingSpotList.Count() * sizeof( HidingSpot ) );

/*
	unsigned int encounterMem = 0;
	FOR_EACH_LL( TheNavAreaList, it )
	{
		CNavArea *area = TheNavAreaList[ it ];

		FOR_EACH_LL( area->m_spotEncounterList, it )
		{
			SpotEncounter *se = area->m_spotEncounterList[ it ];

			encounterMem += sizeof( SpotEncounter );
			encounterMem += se->spotList.Count() * sizeof( SpotOrder );
		}
	}

	Msg( "  Encounter Spot data = %d bytes\n", encounterMem );
*/
}
#endif


bool CDABotManager::ServerCommand( const char *cmd )
{
	return false;
}


bool CDABotManager::ClientCommand( CBasePlayer *player, const CCommand &args )
{
	return false;
}


/**
 * Process the "bot_add" console command
 */
bool CDABotManager::BotAddCommand( int team, bool isFromConsole, const char *profileName, weapontype_t weaponType, BotDifficultyType difficulty )
{
	if ( !TheNavMesh->IsLoaded() )
	{
		// If there isn't a Navigation Mesh in memory, create one
		if ( !TheNavMesh->IsGenerating() )
		{
			if ( !m_isMapDataLoaded )
			{
				TheNavMesh->BeginGeneration();
				m_isMapDataLoaded = true;
			}
			return false;
		}
	}

	// dont allow bots to join if the Navigation Mesh is being generated
	if (TheNavMesh->IsGenerating())
		return false;

	const BotProfile *profile = NULL;

	if ( !isFromConsole )
	{
		profileName = NULL;
		difficulty = GetDifficultyLevel();
	}
	else
	{
		if ( difficulty == NUM_DIFFICULTY_LEVELS )
		{
			difficulty = GetDifficultyLevel();
		}

		// if team not specified, check bot_join_team cvar for preference
		if (team == TEAM_UNASSIGNED)
		{
			//team = SDKGameRules()->SelectBestTeam();
		}
	}

	if ( profileName && *profileName )
	{
		// in career, ignore humans, since we want to add anyway
		bool ignoreHumans = false;
		if (UTIL_IsNameTaken( profileName, ignoreHumans ))
		{
			if ( isFromConsole )
			{
				Msg( "Error - %s is already in the game.\n", profileName );
			}
			return true;
		}

		// try to add a bot by name
		profile = TheBotProfiles->GetProfile( profileName, team );
		if ( !profile )
		{
			// try to add a bot by template
			profile = TheBotProfiles->GetProfileMatchingTemplate( profileName, team, difficulty );
			if ( !profile )
			{
				if ( isFromConsole )
				{
					Msg( "Error - no profile for '%s' exists.\n", profileName );
				}
				return true;
			}
		}
	}
	else
	{
		// if team not specified, check bot_join_team cvar for preference
		if (team == TEAM_UNASSIGNED)
		{
			//team = SDKGameRules()->SelectBestTeam();
		}

		profile = TheBotProfiles->GetRandomProfile( difficulty, team, weaponType );
		if (profile == NULL)
		{
			if ( isFromConsole )
			{
				Msg( "All bot profiles at this difficulty level are in use.\n" );
			}
			return true;
		}
	}

	bool bTeamOkay = !(team == TEAM_SPECTATOR);
	if (SDKGameRules()->IsTeamplay())
		bTeamOkay &= !(team == TEAM_UNASSIGNED);

	if (!bTeamOkay)
	{
		if ( isFromConsole )
		{
			Msg( "Could not add bot to the game: The game is full\n" );
		}
		return false;
	}

	// create the actual bot
	CDABot *bot = CreateBot<CDABot>( profile, team );

	if (bot == NULL)
	{
		if ( isFromConsole )
		{
			Msg( "Error: CreateBot() failed.\n" );
		}
		return false;
	}

	if (isFromConsole)
	{
		// increase the bot quota to account for manually added bot
		cv_bot_quota.SetValue( cv_bot_quota.GetInt() + 1 );
	}

	return true;
}

int UTIL_CFSBotsInGame()
{
	int count = 0;

	for (int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CDABot *player = dynamic_cast<CDABot *>(UTIL_PlayerByIndex( i ));

		if ( player == NULL )
			continue;

		count++;
	}

	return count;
}

bool UTIL_CFKickBotFromTeam( int kickTeam )
{
	int i;

	// try to kick a dead bot first
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CDABot *player = dynamic_cast<CDABot *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (!player->IsAlive() && player->GetTeamNumber() == kickTeam)
		{
			// its a bot on the right team - kick it
			engine->ServerCommand( UTIL_VarArgs( "kick \"%s\"\n", player->GetPlayerName() ) );

			return true;
		}
	}

	// no dead bots, kick any bot on the given team
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CDABot *player = dynamic_cast<CDABot *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (player->GetTeamNumber() == kickTeam)
		{
			// its a bot on the right team - kick it
			engine->ServerCommand( UTIL_VarArgs( "kick \"%s\"\n", player->GetPlayerName() ) );

			return true;
		}
	}

	return false;
}

int UTIL_HumansInGame( bool ignoreSpectators = false )
{
	int iCount = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *entity = ToSDKPlayer(UTIL_PlayerByIndex( i ));

		if ( entity && !FNullEnt( entity->edict() ) )
		{
			if ( FStrEq( entity->GetPlayerName(), "" ) )
				continue;

			if ( FBitSet( entity->GetFlags(), FL_FAKECLIENT ) )
				continue;

			if ( ignoreSpectators && entity->GetTeamNumber() == TEAM_SPECTATOR )
				continue;

			iCount++;
		}
	}

	return iCount;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Keep a minimum quota of bots in the game
 */
void CDABotManager::MaintainBotQuota( void )
{
	if ( !AreBotsAllowed() )
		return;

	if (TheNavMesh->IsGenerating())
		return;

	int totalHumansInGame = UTIL_HumansInGame();
	int humanPlayersInGame = UTIL_HumansInGame( true );

	// don't add bots until local player has been registered, to make sure he's player ID #1
	if (!engine->IsDedicatedServer() && totalHumansInGame == 0)
		return;

	// new players can't spawn immediately after the round has been going for some time
	if ( !SDKGameRules() || !TheDABots() )
	{
		return;
	}

	int desiredBotCount = cv_bot_quota.GetInt();
	int botsInGame = UTIL_CFSBotsInGame();

	/// isRoundInProgress is true if the round has progressed far enough that new players will join as dead.
	bool isRoundInProgress = false;

	if ( FStrEq( cv_bot_quota_mode.GetString(), "fill" ) )
	{
		// If bot_quota_mode is 'fill', we want the number of bots and humans together to equal bot_quota
		// unless the round is already in progress, in which case we play with what we've been dealt
		if ( !isRoundInProgress )
		{
			desiredBotCount = max( 0, desiredBotCount - humanPlayersInGame );
		}
		else
		{
			desiredBotCount = botsInGame;
		}
	}
	else if ( FStrEq( cv_bot_quota_mode.GetString(), "match" ) )
	{
		// If bot_quota_mode is 'match', we want the number of bots to be bot_quota * total humans
		// unless the round is already in progress, in which case we play with what we've been dealt
		if ( !isRoundInProgress )
		{
			desiredBotCount = (int)max( 0, cv_bot_quota.GetFloat() * humanPlayersInGame );
		}
		else
		{
			desiredBotCount = botsInGame;
		}
	}

	// wait for a player to join, if necessary
	if (cv_bot_join_after_player.GetBool())
	{
		if (humanPlayersInGame == 0)
			desiredBotCount = 0;
	}

	// if bots will auto-vacate, we need to keep one slot open to allow players to join
	if (cv_bot_auto_vacate.GetBool())
		desiredBotCount = min( desiredBotCount, gpGlobals->maxClients - (humanPlayersInGame + 1) );
	else
		desiredBotCount = min( desiredBotCount, gpGlobals->maxClients - humanPlayersInGame );

	// add bots if necessary
	if (desiredBotCount > botsInGame)
	{
		TheDABots()->BotAddCommand( TEAM_UNASSIGNED );
	}
	else if (desiredBotCount < botsInGame)
	{
		// kick a bot to maintain quota
		
		// first remove any unassigned bots
		if (UTIL_CFKickBotFromTeam( TEAM_UNASSIGNED ))
			return;

		// Maybe we'll have some teams to kick from later.
		Assert(!"Unimplemented");
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Collect all nav areas that overlap the given zone
 */
class CollectOverlappingAreas
{
public:
	CollectOverlappingAreas( CDABotManager::Zone *zone )
	{
		m_zone = zone;

		zone->m_areaCount = 0;
	}

	bool operator() ( CNavArea *area )
	{
		const Extent &areaExtent = area->GetExtent();

		if (areaExtent.hi.x >= m_zone->m_extent.lo.x && areaExtent.lo.x <= m_zone->m_extent.hi.x &&
			areaExtent.hi.y >= m_zone->m_extent.lo.y && areaExtent.lo.y <= m_zone->m_extent.hi.y &&
			areaExtent.hi.z >= m_zone->m_extent.lo.z && areaExtent.lo.z <= m_zone->m_extent.hi.z)
		{
			// area overlaps m_zone
			m_zone->m_area[ m_zone->m_areaCount++ ] = area;
			if (m_zone->m_areaCount == CDABotManager::MAX_ZONE_NAV_AREAS)
			{
				return false;
			}
		}

		return true;
	}

private:
	CDABotManager::Zone *m_zone;
};


//--------------------------------------------------------------------------------------------------------------
/**
 * Search the map entities to determine the game scenario and define important zones.
 */
void CDABotManager::ExtractScenarioData( void )
{
	if (!TheNavMesh->IsLoaded())
		return;

	m_zoneCount = 0;
	m_gameScenario = SCENARIO_DEATHMATCH;


	//
	// Search all entities in the map and set the game type and
	// store all zones (bomb target, etc).
	//
	CBaseEntity *entity;
	int i;
	for( i=1; i<gpGlobals->maxEntities; ++i )
	{
		entity = CBaseEntity::Instance( engine->PEntityOfEntIndex( i ) );

		if (entity == NULL)
			continue;

		bool found = false;
		bool isLegacy = false;

		if (found)
		{
			if (m_zoneCount < MAX_ZONES)
			{
				Vector absmin, absmax;
				entity->CollisionProp()->WorldSpaceAABB( &absmin, &absmax );

				m_zone[ m_zoneCount ].m_isBlocked = false;
				m_zone[ m_zoneCount ].m_center = (isLegacy) ? entity->GetAbsOrigin() : (absmin + absmax)/2.0f;
				m_zone[ m_zoneCount ].m_isLegacy = isLegacy;
				m_zone[ m_zoneCount ].m_index = m_zoneCount;
				m_zone[ m_zoneCount++ ].m_entity = entity;
			}
			else
				Msg( "Warning: Too many zones, some will be ignored.\n" );
		}
	}

	//
	// Collect nav areas that overlap each zone
	//
	for( i=0; i<m_zoneCount; ++i )
	{
		Zone *zone = &m_zone[i];

		if (zone->m_isLegacy)
		{
			const float legacyRange = 256.0f;
			zone->m_extent.lo.x = zone->m_center.x - legacyRange;
			zone->m_extent.lo.y = zone->m_center.y - legacyRange;
			zone->m_extent.lo.z = zone->m_center.z - legacyRange;
			zone->m_extent.hi.x = zone->m_center.x + legacyRange;
			zone->m_extent.hi.y = zone->m_center.y + legacyRange;
			zone->m_extent.hi.z = zone->m_center.z + legacyRange;
		}
		else
		{
			Vector absmin, absmax;
			zone->m_entity->CollisionProp()->WorldSpaceAABB( &absmin, &absmax );

			zone->m_extent.lo = absmin;
			zone->m_extent.hi = absmax;
		}

		// ensure Z overlap
		const float zFudge = 50.0f;
		zone->m_extent.lo.z -= zFudge;
		zone->m_extent.hi.z += zFudge;

		// build a list of nav areas that overlap this zone
		CollectOverlappingAreas collector( zone );
		TheNavMesh->ForAllAreas( collector );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the zone that contains the given position
 */
const CDABotManager::Zone *CDABotManager::GetZone( const Vector &pos ) const
{
	for( int z=0; z<m_zoneCount; ++z )
	{
		if (m_zone[z].m_extent.Contains( pos ))
		{
			return &m_zone[z];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the closest zone to the given position
 */
const CDABotManager::Zone *CDABotManager::GetClosestZone( const Vector &pos ) const
{
	const Zone *close = NULL;
	float closeRangeSq = 999999999.9f;

	for( int z=0; z<m_zoneCount; ++z )
	{
		if ( m_zone[z].m_isBlocked )
			continue;

		float rangeSq = (m_zone[z].m_center - pos).LengthSqr();

		if (rangeSq < closeRangeSq)
		{
			closeRangeSq = rangeSq;
			close = &m_zone[z];
		}
	}

	return close;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return a random position inside the given zone
 */
const Vector *CDABotManager::GetRandomPositionInZone( const Zone *zone ) const
{
	static Vector pos;

	if (zone == NULL)
		return NULL;

	if (zone->m_areaCount == 0)
		return NULL;

	// pick a random overlapping area
	CNavArea *area = GetRandomAreaInZone(zone);

	// pick a location inside both the nav area and the zone
	/// @todo Randomize this

	if (zone->m_isLegacy)
	{
		/// @todo It is possible that the radius might not overlap this area at all...
		area->GetClosestPointOnArea( zone->m_center, &pos );
	}
	else
	{
		const Extent &areaExtent = area->GetExtent();
		Extent overlap;
		overlap.lo.x = max( areaExtent.lo.x, zone->m_extent.lo.x );
		overlap.lo.y = max( areaExtent.lo.y, zone->m_extent.lo.y );
		overlap.hi.x = min( areaExtent.hi.x, zone->m_extent.hi.x );
		overlap.hi.y = min( areaExtent.hi.y, zone->m_extent.hi.y );

		pos.x = (overlap.lo.x + overlap.hi.x)/2.0f;
		pos.y = (overlap.lo.y + overlap.hi.y)/2.0f;
		pos.z = area->GetZ( pos );
	}

	return &pos;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return a random area inside the given zone
 */
CNavArea *CDABotManager::GetRandomAreaInZone( const Zone *zone ) const
{
	int areaCount = zone->m_areaCount;
	if( areaCount == 0 )
	{
		assert( false && "CDABotManager::GetRandomAreaInZone: No areas for this zone" );
		return NULL;
	}

	// Random, but weighted.  Jump areas score zero, since you aren't ever meant to stop on one of those.
	// Avoid areas score 1 to a normal area's 20 because pathfinding treats Avoid as a 20x penalty.
	int totalWeight = 0;
	for( int areaIndex = 0; areaIndex < areaCount; areaIndex++ )
	{
		CNavArea *currentArea = zone->m_area[areaIndex];
		if( currentArea->GetAttributes() & NAV_MESH_JUMP )
			totalWeight += 0;
		else if( currentArea->GetAttributes() & NAV_MESH_AVOID )
			totalWeight += 1;
		else
			totalWeight += 20;
	}

	if( totalWeight == 0 )
	{
		assert( false && "CDABotManager::GetRandomAreaInZone: No real areas for this zone" );
		return NULL;
	}

	int randomPick = RandomInt( 1, totalWeight );

	for( int areaIndex = 0; areaIndex < areaCount; areaIndex++ )
	{
		CNavArea *currentArea = zone->m_area[areaIndex];
		if( currentArea->GetAttributes() & NAV_MESH_JUMP )
			randomPick -= 0;
		else if( currentArea->GetAttributes() & NAV_MESH_AVOID )
			randomPick -= 1;
		else
			randomPick -= 20;

		if( randomPick <= 0 )
			return currentArea;
	}

	// Won't ever get here, but the compiler will cry without it.
	return zone->m_area[0];
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnServerShutdown( IGameEvent *event )
{
	if ( !engine->IsDedicatedServer() )
	{
		// Since we're a listenserver, save some config info for the next time we start up
		static const char *botVars[] =
		{
			"bot_quota",
			"bot_difficulty",
			"bot_chatter",
			"bot_prefix",
			"bot_join_team",
			"bot_defer_to_human",
#ifdef CF_SHIELD_ENABLED
			"bot_allow_shield",
#endif // CF_SHIELD_ENABLED
			"bot_join_after_player",
			"bot_allow_rogues",
			"bot_allow_pistols",
			"bot_allow_shotguns",
			"bot_allow_sub_machine_guns",
			"bot_allow_machine_guns",
			"bot_allow_rifles",
			"bot_allow_snipers",
			"bot_allow_grenades"
		};
		
		KeyValues *data = new KeyValues( "ServerConfig" );

		// load the config data
		if (data)
		{
			data->LoadFromFile( filesystem, "ServerConfig.vdf", "GAME" );
			for ( int i=0; i<sizeof(botVars)/sizeof(botVars[0]); ++i )
			{
				const char *varName = botVars[i];
				if ( varName )
				{
					ConVar *var = cvar->FindVar( varName );
					if ( var )
					{
						data->SetString( varName, var->GetString() );
					}
				}
			}
			data->SaveToFile( filesystem, "ServerConfig.vdf", "GAME" );
			data->deleteThis();
		}
		return;
	}
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnPlayerFootstep( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnPlayerFootstep, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnPlayerRadio( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnPlayerRadio, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnPlayerDeath( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnPlayerDeath, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnPlayerFallDamage( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnPlayerFallDamage, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnRoundEnd( IGameEvent *event )
{
	m_isRoundOver = true;

	CDABotMANAGER_ITERATE_BOTS( OnRoundEnd, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnRoundStart( IGameEvent *event )
{
	RestartRound();

	CDABotMANAGER_ITERATE_BOTS( OnRoundStart, event );
}


//--------------------------------------------------------------------------------------------------------------
static CBaseEntity * SelectSpawnSpot( const char *pEntClassName )
{
	CBaseEntity* pSpot = NULL;

	// Find the next spawn spot.
	pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	if ( pSpot == NULL ) // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	CBaseEntity *pFirstSpot = pSpot;
	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( pSpot->GetAbsOrigin() == Vector( 0, 0, 0 ) )
			{
				pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
				continue;
			}

			// if so, go to pSpot
			return pSpot;
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	return NULL;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Pathfind from each zone to a spawn point to ensure it is valid.  Assumes that every spawn can pathfind to
 * every other spawn.
 */
void CDABotManager::CheckForBlockedZones( void )
{
	CBaseEntity *pSpot = SelectSpawnSpot( "info_player_counterterrorist" );
	if ( !pSpot )
		pSpot = SelectSpawnSpot( "info_player_terrorist" );

	if ( !pSpot )
		return;

	Vector spawnPos = pSpot->GetAbsOrigin();
	CNavArea *spawnArea = TheNavMesh->GetNearestNavArea( spawnPos );
	if ( !spawnArea )
		return;

	ShortestPathCost costFunc;

	for( int i=0; i<m_zoneCount; ++i )
	{
		if (m_zone[i].m_areaCount == 0)
			continue;

		// just use the first overlapping nav area as a reasonable approximation
		float dist = NavAreaTravelDistance( spawnArea, m_zone[i].m_area[0], costFunc );
		m_zone[i].m_isBlocked = (dist < 0.0f );

		if ( cv_bot_debug.GetInt() == 5 )
		{
			if ( m_zone[i].m_isBlocked )
				DevMsg( "%.1f: Zone %d, area %d (%.0f %.0f %.0f) is blocked from spawn area %d (%.0f %.0f %.0f)\n",
					gpGlobals->curtime, i, m_zone[i].m_area[0]->GetID(),
					m_zone[i].m_area[0]->GetCenter().x, m_zone[i].m_area[0]->GetCenter().y, m_zone[i].m_area[0]->GetCenter().z,
					spawnArea->GetID(),
					spawnPos.x, spawnPos.y, spawnPos.z );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnRoundFreezeEnd( IGameEvent *event )
{
	bool reenableEvents = m_NavBlockedEvent.IsEnabled();

	m_NavBlockedEvent.Enable( false ); // don't listen to nav_blocked events - there could be several, and we don't have bots pathing
	CUtlVector< CNavArea * >& transientAreas = TheNavMesh->GetTransientAreas();
	for ( int i=0; i<transientAreas.Count(); ++i )
	{
		CNavArea *area = transientAreas[i];
		if ( area->GetAttributes() & NAV_MESH_TRANSIENT )
		{
			area->UpdateBlocked();
		}
	}
	if ( reenableEvents )
	{
		m_NavBlockedEvent.Enable( true );
	}

	CheckForBlockedZones();
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnNavBlocked( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnNavBlocked, event );
	CheckForBlockedZones();
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnDoorMoving( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnDoorMoving, event );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Check all nav areas inside the breakable's extent to see if players would now fall through
 */
class CheckAreasOverlappingBreakable
{
public:
	CheckAreasOverlappingBreakable( CBaseEntity *breakable )
	{
		m_breakable = breakable;
		ICollideable *collideable = breakable->GetCollideable();
		collideable->WorldSpaceSurroundingBounds( &m_breakableExtent.lo, &m_breakableExtent.hi );

		const float expand = 10.0f;
		m_breakableExtent.lo += Vector( -expand, -expand, -expand );
		m_breakableExtent.hi += Vector(  expand,  expand,  expand );
	}

	bool operator() ( CNavArea *area )
	{
		const Extent &areaExtent = area->GetExtent();

		if (areaExtent.hi.x >= m_breakableExtent.lo.x && areaExtent.lo.x <= m_breakableExtent.hi.x &&
			areaExtent.hi.y >= m_breakableExtent.lo.y && areaExtent.lo.y <= m_breakableExtent.hi.y &&
			areaExtent.hi.z >= m_breakableExtent.lo.z && areaExtent.lo.z <= m_breakableExtent.hi.z)
		{
			// area overlaps the breakable
			area->CheckFloor( m_breakable );
		}

		return true;
	}

private:
	Extent m_breakableExtent;
	CBaseEntity *m_breakable;
};


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnBreakBreakable( IGameEvent *event )
{
	CheckAreasOverlappingBreakable collector( UTIL_EntityByIndex( event->GetInt( "entindex" ) ) );
	TheNavMesh->ForAllAreas( collector );

	CDABotMANAGER_ITERATE_BOTS( OnBreakBreakable, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnBreakProp( IGameEvent *event )
{
	CheckAreasOverlappingBreakable collector( UTIL_EntityByIndex( event->GetInt( "entindex" ) ) );
	TheNavMesh->ForAllAreas( collector );

	CDABotMANAGER_ITERATE_BOTS( OnBreakProp, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnWeaponFire( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnWeaponFire, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnWeaponFireOnEmpty( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnWeaponFireOnEmpty, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnWeaponReload( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnWeaponReload, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnWeaponZoom( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnWeaponZoom, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnBulletImpact( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnBulletImpact, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnHEGrenadeDetonate( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnHEGrenadeDetonate, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnFlashbangDetonate( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnFlashbangDetonate, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnSmokeGrenadeDetonate( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnSmokeGrenadeDetonate, event );
}


//--------------------------------------------------------------------------------------------------------------
void CDABotManager::OnGrenadeBounce( IGameEvent *event )
{
	CDABotMANAGER_ITERATE_BOTS( OnGrenadeBounce, event );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if player is important to scenario (VIP, bomb carrier, etc)
 */
bool CDABotManager::IsImportantPlayer( CSDKPlayer *player ) const
{
	// everyone is equally important in a deathmatch
	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return priority of player (0 = max pri)
 */
unsigned int CDABotManager::GetPlayerPriority( CBasePlayer *player ) const
{
	const unsigned int lowestPriority = 0xFFFFFFFF;

	if (!player->IsPlayer())
		return lowestPriority;

	// human players have highest priority
	if (!player->IsBot())
		return 0;

	CDABot *bot = dynamic_cast<CDABot *>( player );

	if ( !bot )
		return 0;

	// everyone else is ranked by their unique ID (which cannot be zero)
	return 1 + bot->GetID();
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Returns a random spawn point for the given team (no arg means use both team spawnpoints)
 */
CBaseEntity *CDABotManager::GetRandomSpawn( int team ) const
{
	CUtlVector< CBaseEntity * > spawnSet;
	CBaseEntity *spot;

	for( spot = gEntList.FindEntityByClassname( NULL, "info_player_teamspawn" );
		 spot;
		 spot = gEntList.FindEntityByClassname( spot, "info_player_teamspawn" ) )
	{
		if (spot->GetTeamNumber() != team)
			continue;

		spawnSet.AddToTail( spot );			
	}

	if (spawnSet.Count() == 0)
	{
		return NULL;
	}

	// select one at random
	int which = RandomInt( 0, spawnSet.Count()-1 );
	return spawnSet[ which ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Display nav areas as they become reachable by each team
 */
void DrawOccupyTime( void )
{
	FOR_EACH_LL( TheNavAreaList, it )
	{
		CNavArea *area = TheNavAreaList[ it ];

		int r, g, b;

/*		if (TheDABots()->GetElapsedRoundTime() > area->GetEarliestOccupyTime( TEAM_1 ))
		{
			if (TheDABots()->GetElapsedRoundTime() > area->GetEarliestOccupyTime( TEAM_2 ))
			{
				r = 255; g = 0; b = 255;
			}
			else
			{
				r = 255; g = 0; b = 0;
			}
		}
		else if (TheDABots()->GetElapsedRoundTime() > area->GetEarliestOccupyTime( TEAM_2 ))
		{
			r = 0; g = 0; b = 255;
		}
		else*/
		{
			continue;
		}

		const Vector &nw = area->GetCorner( NORTH_WEST );
		const Vector &ne = area->GetCorner( NORTH_EAST );
		const Vector &sw = area->GetCorner( SOUTH_WEST );
		const Vector &se = area->GetCorner( SOUTH_EAST );

		NDebugOverlay::Line( nw, ne, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( nw, sw, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( se, sw, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( se, ne, r, g, b, true, 0.1f );		
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Display areas where players will likely have initial battles
 */
void DrawBattlefront( void )
{
//	const float epsilon = 1.0f;
	int r = 255, g = 50, b = 0;
	
	FOR_EACH_LL( TheNavAreaList, it )
	{
		CNavArea *area = TheNavAreaList[ it ];

//		if ( fabs(area->GetEarliestOccupyTime( TEAM_1 ) - area->GetEarliestOccupyTime( TEAM_2 )) > epsilon )
		{
			continue;
		}


		const Vector &nw = area->GetCorner( NORTH_WEST );
		const Vector &ne = area->GetCorner( NORTH_EAST );
		const Vector &sw = area->GetCorner( SOUTH_WEST );
		const Vector &se = area->GetCorner( SOUTH_EAST );

		NDebugOverlay::Line( nw, ne, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( nw, sw, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( se, sw, r, g, b, true, 0.1f );		
		NDebugOverlay::Line( se, ne, r, g, b, true, 0.1f );		
	}
}

//--------------------------------------------------------------------------------------------------------------
static bool CheckAreaAgainstAllZoneAreas(CNavArea *queryArea)
{
	// A marked area means they just want to double check this one spot
	int goalZoneCount = TheDABots()->GetZoneCount();

	for( int zoneIndex = 0; zoneIndex < goalZoneCount; zoneIndex++ )
	{
		const CDABotManager::Zone *currentZone = TheDABots()->GetZone(zoneIndex);

		int zoneAreaCount = currentZone->m_areaCount;
		for( int areaIndex = 0; areaIndex < zoneAreaCount; areaIndex++ )
		{
			CNavArea *zoneArea = currentZone->m_area[areaIndex];
			// We need to be connected to every area in the zone, since we don't know what other code might pick for an area
			ShortestPathCost cost;
			if( NavAreaTravelDistance(queryArea, zoneArea, cost) == -1.0f )
			{
				Msg( "Area #%d is disconnected from goal area #%d.\n", 
					queryArea->GetID(), 
					zoneArea->GetID()
					);
				return false;
			}
		}

	}
	return true;
}

CON_COMMAND_F( nav_check_connectivity, "Checks to be sure every (or just the marked) nav area can get to every goal area for the map (hostages or bomb site).", FCVAR_CHEAT )
{
	//Nav command in here since very CF specific.

	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if ( TheNavMesh->GetMarkedArea() )
	{
		CNavArea *markedArea = TheNavMesh->GetMarkedArea();
		bool fine = CheckAreaAgainstAllZoneAreas( markedArea );
		if( fine )
		{
			Msg( "Area #%d is connected to all goal areas.\n", markedArea->GetID() );
		}
	}
	else
	{
		// Otherwise, loop through every area, and make sure they can all get to the goal.
		float start = engine->Time();
		FOR_EACH_LL( TheNavAreaList, nit )
		{
			CheckAreaAgainstAllZoneAreas(TheNavAreaList[ nit ]);
		}

		float end = engine->Time();
		float time = (end - start) * 1000.0f;
		Msg( "nav_check_connectivity took %2.2f ms\n", time );
	}
}





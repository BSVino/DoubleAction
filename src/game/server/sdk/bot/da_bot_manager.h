//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#pragma once


#include "bot_manager.h"
#include "nav_area.h"
#include "bot_util.h"
#include "bot_profile.h"
#include "sdk_shareddefs.h"
#include "sdk_player.h"

extern ConVar friendlyfire;

class CBasePlayerWeapon;

/**
 * Given one team, return the other
 */
inline int OtherTeam( int team )
{
	//Assert(!"No teams");
	//return TEAM_UNASSIGNED;
	return (team == SDK_TEAM_BLUE) ? SDK_TEAM_RED : SDK_TEAM_BLUE;
}

class CDABotManager;

// accessor for CF-specific bots
inline CDABotManager *TheDABots( void )
{
	return reinterpret_cast< CDABotManager * >( TheBots );
}

//--------------------------------------------------------------------------------------------------------------
class BotEventInterface : public IGameEventListener2
{
public:
	virtual const char *GetEventName( void ) const = 0;
};

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
/**
 * Macro to set up an OnEventClass() in TheDABots.
 */
#define DECLARE_BOTMANAGER_EVENT_LISTENER( BotManagerSingleton, EventClass, EventName ) \
	public: \
	virtual void On##EventClass( IGameEvent *data ); \
	private: \
	class EventClass##Event : public BotEventInterface \
	{ \
		bool m_enabled; \
	public: \
		EventClass##Event( void ) \
		{ \
			gameeventmanager->AddListener( this, #EventName, true ); \
			m_enabled = true; \
		} \
		~EventClass##Event( void ) \
		{ \
			if ( m_enabled ) gameeventmanager->RemoveListener( this ); \
		} \
		virtual const char *GetEventName( void ) const \
		{ \
			return #EventName; \
		} \
		void Enable( bool enable ) \
		{ \
			m_enabled = enable; \
			if ( enable ) \
				gameeventmanager->AddListener( this, #EventName, true ); \
			else \
				gameeventmanager->RemoveListener( this ); \
		} \
		bool IsEnabled( void ) const { return m_enabled; } \
		void FireGameEvent( IGameEvent *event ) \
		{ \
			BotManagerSingleton()->On##EventClass( event ); \
		} \
	}; \
	EventClass##Event m_##EventClass##Event;


//--------------------------------------------------------------------------------------------------------------
#define DECLARE_CFBOTMANAGER_EVENT_LISTENER( EventClass, EventName ) DECLARE_BOTMANAGER_EVENT_LISTENER( TheDABots, EventClass, EventName )


//--------------------------------------------------------------------------------------------------------------
/**
 * Macro to propogate an event from the bot manager to all bots
 */
#define CDABotMANAGER_ITERATE_BOTS( Callback, arg1 ) \
	{ \
		for ( int idx = 1; idx <= gpGlobals->maxClients; ++idx ) \
		{ \
			CBasePlayer *player = UTIL_PlayerByIndex( idx ); \
			if (player == NULL) continue; \
			if (!player->IsBot()) continue; \
			CDABot *bot = dynamic_cast< CDABot * >(player); \
			if ( !bot ) continue; \
			bot->Callback( arg1 ); \
		} \
	}


//--------------------------------------------------------------------------------------------------------------
//
// The manager for Counter-Strike specific bots
//
class CDABotManager : public CBotManager
{
public:
	CDABotManager();

	virtual CBasePlayer *AllocateBotEntity( void );			///< factory method to allocate the appropriate entity for the bot

	virtual void ClientDisconnect( CBaseEntity *entity );
	virtual bool ClientCommand( CBasePlayer *player, const CCommand &args );

	virtual void ServerActivate( void );
	virtual void ServerDeactivate( void );
	virtual bool ServerCommand( const char *cmd );
	bool IsServerActive( void ) const { return m_serverActive; }

	virtual void RestartRound( void );						///< (EXTEND) invoked when a new round begins
	virtual void StartFrame( void );						///< (EXTEND) called each frame

	virtual unsigned int GetPlayerPriority( CBasePlayer *player ) const;	///< return priority of player (0 = max pri)
	virtual bool IsImportantPlayer( CSDKPlayer *player ) const;				///< return true if player is important to scenario (VIP, bomb carrier, etc)

	void ExtractScenarioData( void );							///< search the map entities to determine the game scenario and define important zones

	// difficulty levels -----------------------------------------------------------------------------------------
	static BotDifficultyType GetDifficultyLevel( void )		
	{ 
		if (cv_bot_difficulty.GetFloat() < 0.9f)
			return BOT_EASY;
		if (cv_bot_difficulty.GetFloat() < 1.9f)
			return BOT_NORMAL;
		if (cv_bot_difficulty.GetFloat() < 2.9f)
			return BOT_HARD;

		return BOT_EXPERT;
	}

	// the supported game scenarios ------------------------------------------------------------------------------
	enum GameScenarioType
	{
		SCENARIO_DEATHMATCH,
	};
	GameScenarioType GetScenario( void ) const		{ return m_gameScenario; }

	// "zones" ---------------------------------------------------------------------------------------------------
	// depending on the game mode, these are bomb zones, rescue zones, etc.

	enum { MAX_ZONES = 4 };										///< max # of zones in a map
	enum { MAX_ZONE_NAV_AREAS = 16 };							///< max # of nav areas in a zone
	struct Zone
	{
		CBaseEntity *m_entity;									///< the map entity
		CNavArea *m_area[ MAX_ZONE_NAV_AREAS ];					///< nav areas that overlap this zone
		int m_areaCount;
		Vector m_center;
		bool m_isLegacy;										///< if true, use pev->origin and 256 unit radius as zone
		int m_index;
		bool m_isBlocked;
		Extent m_extent;
	};

	const Zone *GetZone( int i ) const				{ return &m_zone[i]; }
	const Zone *GetZone( const Vector &pos ) const;				///< return the zone that contains the given position
	const Zone *GetClosestZone( const Vector &pos ) const;		///< return the closest zone to the given position
	const Zone *GetClosestZone( const CBaseEntity *entity ) const;	///< return the closest zone to the given entity
	int GetZoneCount( void ) const					{ return m_zoneCount; }
	void CheckForBlockedZones( void );


	const Vector *GetRandomPositionInZone( const Zone *zone ) const;	///< return a random position inside the given zone
	CNavArea *GetRandomAreaInZone( const Zone *zone ) const;			///< return a random area inside the given zone

	/**
	 * Return the zone closest to the given position, using the given cost heuristic
	 */
	template< typename CostFunctor >
	const Zone *GetClosestZone( CNavArea *startArea, CostFunctor costFunc, float *travelDistance = NULL ) const
	{
		const Zone *closeZone = NULL;
		float closeDist = 99999999.9f;

		if (startArea == NULL)
			return NULL;

		for( int i=0; i<m_zoneCount; ++i )
		{
			if (m_zone[i].m_areaCount == 0)
				continue;

			if ( m_zone[i].m_isBlocked )
				continue;

			// just use the first overlapping nav area as a reasonable approximation
			float dist = NavAreaTravelDistance( startArea, m_zone[i].m_area[0], costFunc );

			if (dist >= 0.0f && dist < closeDist)
			{
				closeZone = &m_zone[i];
				closeDist = dist;
			}
		}

		if (travelDistance)
			*travelDistance = closeDist;

		return closeZone;
	}

	/// pick a zone at random and return it
	const Zone *GetRandomZone( void ) const
	{
		if (m_zoneCount == 0)
			return NULL;

		int i;
		CUtlVector< const Zone * > unblockedZones;
		for ( i=0; i<m_zoneCount; ++i )
		{
			if ( m_zone[i].m_isBlocked )
				continue;

			unblockedZones.AddToTail( &(m_zone[i]) );
		}

		if ( unblockedZones.Count() == 0 )
			return NULL;

		return unblockedZones[ RandomInt( 0, unblockedZones.Count()-1 ) ];
	}


	/// returns a random spawn point for the given team (no arg means use both team spawnpoints)
	CBaseEntity *GetRandomSpawn( int team = MAX_TEAMS ) const;


	float GetLastSeenEnemyTimestamp( void ) const	{ return m_lastSeenEnemyTimestamp; }	///< return the last time anyone has seen an enemy
	void SetLastSeenEnemyTimestamp( void ) 			{ m_lastSeenEnemyTimestamp = gpGlobals->curtime; }

	float GetRoundStartTime( void ) const			{ return m_roundStartTimestamp; }
	float GetElapsedRoundTime( void ) const			{ return gpGlobals->curtime - m_roundStartTimestamp; }	///< return the elapsed time since the current round began

	bool AllowRogues( void ) const					{ return cv_bot_allow_rogues.GetBool(); }
	bool AllowPistols( void ) const					{ return cv_bot_allow_pistols.GetBool(); }

	bool AllowFriendlyFireDamage( void ) const		{ return friendlyfire.GetBool(); }

	bool IsWeaponUseable( const CWeaponSDKBase *weapon ) const;	///< return true if the bot can use this weapon

	bool IsDefenseRushing( void ) const				{ return m_isDefenseRushing; }		///< returns true if defense team has "decided" to rush this round
	bool IsOnDefense( const CSDKPlayer *player ) const;		///< return true if this player is on "defense"
	bool IsOnOffense( const CSDKPlayer *player ) const;		///< return true if this player is on "offense"

	bool IsRoundOver( void ) const					{ return m_isRoundOver; }		///< return true if the round has ended

	#define FROM_CONSOLE true
	bool BotAddCommand( int team, bool isFromConsole = false, const char *profileName = NULL, weapontype_t weaponType = WT_NONE, BotDifficultyType difficulty = NUM_DIFFICULTY_LEVELS );	///< process the "bot_add" console command

private:
	enum SkillType { LOW, AVERAGE, HIGH, RANDOM };

	void MaintainBotQuota( void );

	static bool m_isMapDataLoaded;							///< true if we've attempted to load map data
	bool m_serverActive;									///< true between ServerActivate() and ServerDeactivate()

	GameScenarioType m_gameScenario;						///< what kind of game are we playing

	Zone m_zone[ MAX_ZONES ];							
	int m_zoneCount;

	bool m_isRoundOver;										///< true if the round has ended

	CountdownTimer m_checkTransientAreasTimer;				///< when elapsed, all transient nav areas should be checked for blockage

	float m_lastSeenEnemyTimestamp;
	float m_roundStartTimestamp;							///< the time when the current round began

	bool m_isDefenseRushing;								///< whether defensive team is rushing this round or not

	// Event Handlers --------------------------------------------------------------------------------------------
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( PlayerFootstep,		player_footstep )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( PlayerRadio,			player_radio )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( PlayerDeath,			player_death )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( PlayerFallDamage,		player_falldamage )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( RoundEnd,				round_end )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( RoundStart,			round_start )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( RoundFreezeEnd,		round_freeze_end )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( DoorMoving,			door_moving )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( BreakProp,				break_prop )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( BreakBreakable,		break_breakable )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( WeaponFire,			weapon_fire )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( WeaponFireOnEmpty,		weapon_fire_on_empty )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( WeaponReload,			weapon_reload )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( WeaponZoom,			weapon_zoom )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( BulletImpact,			bullet_impact )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( HEGrenadeDetonate,		hegrenade_detonate )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( FlashbangDetonate,		flashbang_detonate )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( SmokeGrenadeDetonate,	smokegrenade_detonate )
	DECLARE_CFBOTMANAGER_EVENT_LISTENER( GrenadeBounce,			grenade_bounce )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( NavBlocked,			nav_blocked )

	DECLARE_CFBOTMANAGER_EVENT_LISTENER( ServerShutdown,		server_shutdown )

	CUtlVector< BotEventInterface * > m_commonEventListeners;	// These event listeners fire often, and can be disabled for performance gains when no bots are present.
	bool m_eventListenersEnabled;
	void EnableEventListeners( bool enable );
};

inline CBasePlayer *CDABotManager::AllocateBotEntity( void )
{
	return static_cast<CBasePlayer *>( CreateEntityByName( "da_bot" ) );
}

inline const CDABotManager::Zone *CDABotManager::GetClosestZone( const CBaseEntity *entity ) const
{
	if (entity == NULL)
		return NULL;

	Vector centroid = entity->GetAbsOrigin();
	centroid.z += HalfHumanHeight;
	return GetClosestZone( centroid );
}

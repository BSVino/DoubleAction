//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_GAMERULES_H
#define SDK_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "weapon_sdkbase.h"
#include "da.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
	#include "c_da_briefcase.h"
#else
	#include "player.h"
	#include "sdk_player.h"
	#include "utlqueue.h"
	#include "playerclass_info_parse.h"

#endif


#ifdef CLIENT_DLL
	#define CSDKGameRules C_SDKGameRules
	#define CSDKGameRulesProxy C_SDKGameRulesProxy
#endif

class CBriefcaseCaptureZone;
class CRatRaceWaypoint;

class CSDKGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CSDKGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CSDKViewVectors : public CViewVectors
{
public:
	CSDKViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDiveHullMin,
		Vector vDiveHullMax,
		Vector vDiveView,
		Vector vDeadViewHeight
#if defined ( SDK_USE_PRONE )
		,Vector vProneHullMin,
		Vector vProneHullMax,
		Vector vProneView
#endif
		) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
#if defined( SDK_USE_PRONE )
		m_vProneHullMin = vProneHullMin;
		m_vProneHullMax = vProneHullMax;
		m_vProneView = vProneView;
#endif 

		m_vSlideHullMin = vProneHullMin;
		m_vSlideHullMax = vProneHullMax;
		m_vSlideView = Vector( 0, 0, 20 );

		m_vDiveHullMin = vDiveHullMin;
		m_vDiveHullMax = vDiveHullMax;
		m_vDiveView = vDiveView;
	}
#if defined ( SDK_USE_PRONE )
	Vector m_vProneHullMin;
	Vector m_vProneHullMax;	
	Vector m_vProneView;
#endif

	Vector m_vSlideHullMin;
	Vector m_vSlideHullMax;	
	Vector m_vSlideView;

	Vector m_vDiveHullMin;
	Vector m_vDiveHullMax;	
	Vector m_vDiveView;	
};

class CSDKGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CSDKGameRules, CTeamplayRules );

	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual bool	IsTeamplay( void )
	{ 
#if defined ( SDK_USE_TEAMS )
		return m_bIsTeamplay;
#else
		return false;	
#endif
	}
	// Get the view vectors for this mod.
	virtual const CViewVectors* GetViewVectors() const;
	virtual const CSDKViewVectors *GetSDKViewVectors() const;
	//Tony; define a default encryption key.
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"a1b2c3d4"; }

	//Tony; in shared space
#if defined ( SDK_USE_PLAYERCLASSES )
	const char *GetPlayerClassName( int cls, int team );
#endif

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );

	virtual void OverrideSoundParams(const EmitSound_t& ep, CSoundParameters& oParams);

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
	
	CSDKGameRules();
	virtual ~CSDKGameRules();
	virtual const char *GetGameDescription( void ) { return SDK_GAME_DESCRIPTION; } 
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual void Think();
	virtual void GoToIntermission( void );
	virtual void EndGameFrame();

	void InitTeams( void );

	void CreateStandardEntities( void );

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );
	bool IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual bool InitTeamSpawns( void );

	virtual bool CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );

#if defined ( SDK_USE_PLAYERCLASSES )
	bool IsPlayerClassOnTeam( int cls, int team );
	bool CanPlayerJoinClass( CSDKPlayer *pPlayer, int cls );
	void ChooseRandomClass( CSDKPlayer *pPlayer );
	bool ReachedClassLimit( int team, int cls );
	int CountPlayerClass( int team, int cls );
	int GetClassLimit( int team, int cls );
#endif 
	bool TeamFull( int team_id );
	bool TeamStacked( int iNewTeam, int iCurTeam );
	int SelectDefaultTeam( void );

	virtual void LevelInitPostEntity();
	virtual bool ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual void ClientDisconnected( edict_t *pClient );

	virtual bool IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer );

	void    CoderHacksUpdate();

	void	ReCalculateSlowMo();
	void	CalculateSlowMoForPlayer(CSDKPlayer* pPlayer);	// Is this player being slowed by those around?
	void	PlayerSlowMoUpdate(CSDKPlayer* pPlayer);		// This player activated or deactivated slowmo, update surrounding players.

protected:
	void	GiveSlowMoToNearbyPlayers(CSDKPlayer* pPlayer);

protected:
	void CheckPlayerPositions( void );

private:
	void CheckLevelInitialized( void );
	bool m_bLevelInitialized;

	Vector2D	m_vecPlayerPositions[MAX_PLAYERS];

#if defined ( SDK_USE_TEAMS )
	int	m_iSpawnPointCount_Blue;	//number of blue spawns on the map
	int	m_iSpawnPointCount_Red;	//number of red spawns on the map
#endif // SDK_USE_TEAMS

	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore );

public:
	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	const char *GetKillingWeaponName( const CTakeDamageInfo &info, CSDKPlayer *pVictim, int *iWeaponID );

	const CUtlVector<char*>& GetMapList() const { return m_MapList; }

#endif

public:
	float GetMapRemainingTime();	// time till end of map, -1 if timelimit is disabled
	float GetMapElapsedTime();		// How much time has elapsed since the map started.

	Vector GetLowestSpawnPoint();

	bool    CoderHacks() { return m_bCoderHacks; }

private:
	float m_flNextMiniObjectiveStartTime;
	CNetworkVar( miniobjective_t, m_eCurrentMiniObjective );
	miniobjective_t m_ePreviousMiniObjective;

public:
	void StartMiniObjective(const char* pszObjective = NULL);
	notice_t GetNoticeForMiniObjective(miniobjective_t eObjective);
	void MaintainMiniObjective();
	void CleanupMiniObjective();
	void GiveMiniObjectiveReward(CSDKPlayer* pPlayer);

	bool SetupMiniObjective_Briefcase();
	void MaintainMiniObjective_Briefcase();
	void CleanupMiniObjective_Briefcase();

	bool ChooseRandomCapturePoint(Vector vecBriefcaseLocation);
	void PlayerCapturedBriefcase(CSDKPlayer* pPlayer);
	CBriefcase* GetBriefcase() const;
	CBriefcaseCaptureZone* GetCaptureZone() const;

	bool SetupMiniObjective_Bounty();
	void MaintainMiniObjective_Bounty();
	void CleanupMiniObjective_Bounty();

	CSDKPlayer* GetBountyPlayer() const;

	bool SetupMiniObjective_RatRace();
	void MaintainMiniObjective_RatRace();
	void CleanupMiniObjective_RatRace();

	void PlayerReachedWaypoint(CSDKPlayer* pPlayer, CRatRaceWaypoint* pWaypoint);

	// These helper template functions allow me to use the two race leader lists
	// as if they were the same type since C++ template functions use duck typing.
	template<typename T> void WaypointLeadersPush(T& ahWaypointLeaders, CSDKPlayer* pPlayer);
	template<typename T> void RemovePlayerFromLeaders(T& ahWaypointLeaders, CSDKPlayer* pPlayer);
	template<typename T> void CompressLeaders(T& ahWaypointLeaders);
	template<typename T> void DebugCheckLeaders(T& ahWaypointLeaders);

	CRatRaceWaypoint* GetWaypoint(int i) const;
	CSDKPlayer* GetLeader() const;
	CSDKPlayer* GetFrontRunner1() const;
	CSDKPlayer* GetFrontRunner2() const;

private:
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar( bool, m_bIsTeamplay );
	CNetworkVar( bool, m_bCoderHacks );

	CNetworkHandle( CBriefcase, m_hBriefcase );
	CNetworkHandle( CBriefcaseCaptureZone, m_hCaptureZone );

	CNetworkHandle( CSDKPlayer, m_hBountyPlayer );

	CNetworkArray( CHandle<CSDKPlayer>, m_ahWaypoint1RaceLeaders, 16 );
	CNetworkArray( CHandle<CSDKPlayer>, m_ahWaypoint2RaceLeaders, 16 );
	CNetworkHandle( CRatRaceWaypoint, m_hRaceWaypoint1 );
	CNetworkHandle( CRatRaceWaypoint, m_hRaceWaypoint2 );
	CNetworkHandle( CRatRaceWaypoint, m_hRaceWaypoint3 );

	float	m_flNextSlowMoUpdate;

#ifndef CLIENT_DLL
	bool m_bChangelevelDone;
	bool m_bNextMapVoteDone;
#endif

	CNetworkVar( Vector, m_vecLowestSpawnPoint );
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CSDKGameRules* SDKGameRules()
{
	return static_cast<CSDKGameRules*>(g_pGameRules);
}

extern ConVar da_globalslow;

#endif // SDK_GAMERULES_H

#pragma once

#include "sdk_player.h"
#include "bot_main.h"

// This is our bot class.
class CSDKBot : public CSDKPlayer
{
public:
	DECLARE_CLASS( CSDKBot, CSDKPlayer );

public:
	void Initialize();

	void BotThink();

protected:
	bool HasEnemy() { return (hEnemy.Get() != NULL && hEnemy.Get()->IsAlive()); }
	bool RecheckEnemy() { return m_flTimeToRecheckEnemy < gpGlobals->curtime; }

	CBasePlayer *GetEnemy() { return dynamic_cast<CBasePlayer *>(hEnemy.Get()); }

	void Spawn();

	void RunPlayerMove( CUserCmd &cmd, float frametime );

	void HandleRespawn( CUserCmd &cmd );
	void InfoGathering();

	void ResetNavigationParams();
	void AddWaypoint( Vector center, NavTraverseType transient, int attribute, int id, bool AddToTail = false );
	void ResetWaypoints( void ) { m_Waypoints.RemoveAll(); }

	void DealWithObstacles( CBaseEntity *pTouchEnt, CUserCmd &cmd );
	void AddRandomPath( float randomStartAngle = 0 );
	bool BotOnLadder();
	bool ArrivedToWaypoint();
	void CheckStuck( CUserCmd &cmd );
	void CheckNavMeshAttrib( trace_t *ptr, CUserCmd &cmd );
	bool CreatePath( CBasePlayer *pPlayer, Vector OptionalOrg = vec3_origin );
	bool CreateHidePath( Vector &HiDeSpot );
	void SelectSchedule( bool forcePath = false );
	bool SafePathAhead( Vector origin );
	void Navigation( CUserCmd &cmd  );

	bool AcquireEnemy();
	void Attack( CUserCmd &cmd );

protected:
	float			m_flNextStrafeTime;
	float			m_flStrafeSkillRelatedTimer;
	float			m_flSideMove;

	QAngle			m_ForwardAngle;
	QAngle			m_LastAngles;

	// behaviour - capabilities
	int m_nBotState;
	int m_nBotSchedule;
	float m_flSkill[BOT_SKILL_MAX_TOKENS];

	// navigation
	float m_flNextJump;
	bool m_bIsOnLadder;
	CUtlVector <NavAreaData_t> m_Waypoints;
	float m_flNextPathCheck;
	float m_flDontUseDirectNav;
	float m_flTimeToRecheckStuck;
	float m_flDesiredYaw;
	float m_flNextDealObstacles;
	int m_nIsHiding;
	CUtlVector<int> m_AlreadyCheckedHideSpots;
	float m_flCreateRandomPathCoolDown;
	float m_flNextProximityCheck;
	float m_flBotToEnemyDist;
	float m_flDistTraveled;
	float m_flHeightDifToEnemy;

	// combat
	EHANDLE hEnemy;
	float m_flTimeToRecheckEnemy;
	bool m_bEnemyOnSights;	
	bool m_bInRangeToAttack;
	float m_flNextBotAttack;
	float m_flMinRangeAttack;
};

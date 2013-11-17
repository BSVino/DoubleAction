#include "cbase.h"
#include "sdk_bot.h"

#include "movehelper_server.h"
#include "datacache/imdlcache.h"

#include "BasePropDoor.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( bot, CSDKBot );

void CSDKBot::Initialize()
{
	// set bot skills
	m_flSkill[BOT_SKILL_YAW_RATE] = 10;
	m_flSkill[BOT_SKILL_SPEED] = SDK_DEFAULT_PLAYER_SPRINTSPEED;
	m_flSkill[BOT_SKILL_STRAFE] = 5;

	JoinTeam(0);

	ClearLoadout();
	BuyRandom();

	PickRandomCharacter();
	PickRandomSkill();

	State_Transition( STATE_ACTIVE );
}

void CSDKBot::Spawn()
{
	BaseClass::Spawn();

	hEnemy.Set(NULL);
	ResetNavigationParams();
	m_AlreadyCheckedHideSpots.RemoveAll();
	m_flNextDealObstacles = 0;
	m_flCreateRandomPathCoolDown = 0;
	m_flNextProximityCheck = 0;
	m_flDistTraveled = 0; // distance this bot has traveled recently, since last stuck check
	m_flMinRangeAttack = 60.0f;
	m_bInRangeToAttack = false;
	m_flNextStrafeTime = 0;
	m_flStrafeSkillRelatedTimer = 0;
	m_flNextBotMeleeAttack = 0;
}

void CSDKBot::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed(info);

	m_flLastDeathTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input : *fakeclient -
// *viewangles -
// forwardmove -
// m_flSideMove -
// upmove -
// buttons -
// impulse -
// msec -
// Output : virtual void
//-----------------------------------------------------------------------------
void CSDKBot::RunPlayerMove( CUserCmd &cmd, float frametime )
{
	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( this );

	MDLCACHE_CRITICAL_SECTION();

	PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}

void CSDKBot::HandleRespawn( CUserCmd &cmd )
{
	if (gpGlobals->curtime < m_flLastDeathTime + 5)
		return;

	ClearLoadout();
	BuyRandom();

	PickRandomCharacter();
	PickRandomSkill();

	State_Transition( STATE_ACTIVE );
}

// here bot updates important info that is used multiple times along the thinking process
void CSDKBot::InfoGathering()
{
	if (!GetEnemy())
	{
		m_flBotToEnemyDist = 9999;
		m_flHeightDifToEnemy = 0;
		m_bEnemyOnSights = false;

		m_flDistTraveled += fabs(GetLocalVelocity().Length()); // this is used for stuck checking,
		return;
	}

	m_flBotToEnemyDist = (GetLocalOrigin() - GetEnemy()->GetLocalOrigin()).Length();

	trace_t tr;
	UTIL_TraceHull( EyePosition(), GetEnemy()->EyePosition() - Vector(0,0,20), -BotTestHull, BotTestHull, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	if( tr.m_pEnt == GetEnemy() ) // vision line between both
		m_bEnemyOnSights = true;
	else
		m_bEnemyOnSights = false;

	m_bInRangeToAttack = (m_flBotToEnemyDist < m_flMinRangeAttack) && FInViewCone( GetEnemy() );

	m_flDistTraveled += fabs(GetLocalVelocity().Length()); // this is used for stuck checking,

	m_flHeightDifToEnemy = GetLocalOrigin().z - GetEnemy()->GetLocalOrigin().z;
}

//-----------------------------------------------------------------------------
// Run this Bot's AI for one tick.
//-----------------------------------------------------------------------------
void CSDKBot::BotThink()
{
	// Make sure we stay being a bot
	AddFlag( FL_FAKECLIENT );

	if ( IsEFlagSet(EFL_BOT_FROZEN) )
		return;

	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );

	ConVarRef bot_freeze("bot_freeze");

	if ( !IsAlive() )
	{
		HandleRespawn(cmd);
	}
	else if (bot_mimic.GetBool())
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt()  );
		if ( pPlayer && pPlayer->GetLastUserCommand() )
		{
			cmd = *pPlayer->GetLastUserCommand();

			ConVarRef bot_mimic_yaw_offset("bot_mimic_yaw_offset");
			cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

			ConVarRef bot_crouch("bot_crouch");
			if( bot_crouch.GetInt() )
				cmd.buttons |= IN_DUCK;
		}
	}
	else if (!bot_freeze.GetBool())
	{
		trace_t tr_front;
		Vector Forward;
		AngleVectors(GetLocalAngles(), &Forward);
		UTIL_TraceHull( GetLocalOrigin()+Vector(0,0,5), GetLocalOrigin() + Vector(0,0,5) + (Forward * 50), GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr_front );

		// enemy acquisition
		if( !GetEnemy() || RecheckEnemy() || !GetEnemy()->IsAlive() )
		{
			if( GetEnemy() && !GetEnemy()->IsAlive() )
				ResetNavigationParams();

			AcquireEnemy();

			m_flTimeToRecheckEnemy = gpGlobals->curtime + 1.0f;
		}

		// assume we have an enemy from now on

		InfoGathering();

		Attack(cmd);

		if( m_flTimeToRecheckStuck < gpGlobals->curtime )
			CheckStuck(cmd);

		if( m_flNextDealObstacles < gpGlobals->curtime )
			DealWithObstacles(tr_front.m_pEnt, cmd);

		Navigation(cmd);

		CheckNavMeshAttrib(&tr_front, cmd);
	}

	// debug waypoint related position
	/*for( int i=0; i<m_Waypoints.Count(); i++ )
	{
	NDebugOverlay::Cross3DOriented( m_Waypoints[i].Center, QAngle(0,0,0), 5*i+1, 200, 0, 0, false, -1 );
	}*/

	RunPlayerMove( cmd, gpGlobals->frametime );
}

#include "cbase.h"
#include "sdk_bot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CSDKBot::Initialize()
{
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
	m_flNextAttack = gpGlobals->curtime;
	m_flNextStrafeTime = 0;
	m_flStrafeSkillRelatedTimer = 0;
}

void CSDKBot::ResetNavigationParams()
{
	m_Waypoints.RemoveAll();
	m_flNextJump = 0;
	m_bIsOnLadder = false;
	m_flNextPathCheck = 0;
	m_flDontUseDirectNav = 0;
}

void CSDKBot::AddWaypoint( Vector center, NavTraverseType transient, int attribute, int id, bool AddToTail)
{
	NavAreaData_t data;
	VectorCopy( center, data.Center );
	data.TransientType = transient;
	data.AttributeType = attribute;
	data.Id = id;

	if( AddToTail )
		m_Waypoints.AddToTail(data);
	else
		m_Waypoints.AddToHead(data);
}

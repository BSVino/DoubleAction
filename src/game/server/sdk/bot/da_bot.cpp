//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#include "cbase.h"
#include "sdk_gamerules.h"
#include "func_breakablesurf.h"
#include "obstacle_pushaway.h"

#include "da_bot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( da_bot, CDABot );

BEGIN_DATADESC( CDABot )

END_DATADESC()


//--------------------------------------------------------------------------------------------------------------
/**
 * Return the number of bots following the given player
 */
int GetBotFollowCount( CSDKPlayer *leader )
{
	int count = 0;

	for( int i=1; i <= gpGlobals->maxClients; ++i )
	{
		CBaseEntity *entity = UTIL_PlayerByIndex( i );

		if (entity == NULL)
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>( entity );

		if (!player->IsBot())
			continue;

 		if (!player->IsAlive())
 			continue;

		CDABot *bot = dynamic_cast<CDABot *>( player );
		if (bot && bot->GetFollowLeader() == leader)
			++count;
	}

	return count;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Change movement speed to walking
 */
void CDABot::Walk( void )
{
	if (m_mustRunTimer.IsElapsed())
	{
		BaseClass::Walk();
	}
	else
	{
		// must run
		Run();
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if jump was started.
 * This is extended from the base jump to disallow jumping when in a crouch area.
 */
bool CDABot::Jump( bool mustJump )
{
	// prevent jumping if we're crouched, unless we're in a crouchjump area - jump wins
	bool inCrouchJumpArea = (m_lastKnownArea && 
		(m_lastKnownArea->GetAttributes() & NAV_MESH_CROUCH) &&
		(m_lastKnownArea->GetAttributes() & NAV_MESH_JUMP));

	if ( !IsUsingLadder() && IsDucked() && !inCrouchJumpArea )
	{
		return false;
	}

	return BaseClass::Jump( mustJump );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when injured by something
 * NOTE: We dont want to directly call Attack() here, or the bots will have super-human reaction times when injured
 */
int CDABot::OnTakeDamage( const CTakeDamageInfo &info )
{
	CBaseEntity *attacker = info.GetInflictor();

	// getting hurt makes us alert
	BecomeAlert();
	StopWaiting();

	// if we were attacked by a teammate, rebuke
	if (attacker->IsPlayer())
	{
		CSDKPlayer *player = static_cast<CSDKPlayer *>( attacker );
		
		if (InSameTeam( player ) && !player->IsBot())
			GetChatter()->FriendlyFire();
	}

	if (attacker->IsPlayer() && IsEnemy( attacker ))
	{
		// Track previous attacker so we don't try to panic multiple times for a shotgun blast
		CSDKPlayer *lastAttacker = m_attacker;
		float lastAttackedTimestamp = m_attackedTimestamp;

		// keep track of our last attacker
		m_attacker = reinterpret_cast<CSDKPlayer *>( attacker );
		m_attackedTimestamp = gpGlobals->curtime;

		// no longer safe
		AdjustSafeTime();

		if ( !IsSurprised() && (m_attacker != lastAttacker || m_attackedTimestamp != lastAttackedTimestamp) )
		{
			CSDKPlayer *enemy = static_cast<CSDKPlayer *>( attacker );

			// being hurt by an enemy we can't see causes panic
			if (!IsVisible( enemy, true ))
			{
				// if not attacking anything, look around to try to find attacker
				if (!IsAttacking())
				{
					Panic();
				}
				else	// we are attacking
				{
					if (!IsEnemyVisible())
					{
						// can't see our current enemy, panic to acquire new attacker
						Panic();
					}
				}
			}
		}
	}

	// extend
	return BaseClass::OnTakeDamage( info );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when killed
 */
void CDABot::Event_Killed( const CTakeDamageInfo &info )
{ 
//	PrintIfWatched( "Killed( attacker = %s )\n", STRING(pevAttacker->netname) );

	GetChatter()->OnDeath();

	// increase the danger where we died
	const float deathDanger = 1.0f;
	const float deathDangerRadius = 500.0f;
	TheNavMesh->IncreaseDangerNearby( GetTeamNumber(), deathDanger, m_lastKnownArea, GetAbsOrigin(), deathDangerRadius );

	// end voice feedback
	m_voiceEndTimestamp = 0.0f;

	// extend
	BaseClass::Event_Killed( info );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if line segment intersects rectagular volume
 */
#define HI_X	0x01
#define LO_X 0x02
#define HI_Y	0x04
#define LO_Y 0x08
#define HI_Z	0x10
#define LO_Z 0x20

inline bool IsIntersectingBox( const Vector& start, const Vector& end, const Vector& boxMin, const Vector& boxMax )
{
	unsigned char startFlags = 0;
	unsigned char endFlags = 0;

	// classify start point
	if (start.x < boxMin.x)
		startFlags |= LO_X;
	if (start.x > boxMax.x)
		startFlags |= HI_X;

	if (start.y < boxMin.y)
		startFlags |= LO_Y;
	if (start.y > boxMax.y)
		startFlags |= HI_Y;

	if (start.z < boxMin.z)
		startFlags |= LO_Z;
	if (start.z > boxMax.z)
		startFlags |= HI_Z;

	// classify end point
	if (end.x < boxMin.x)
		endFlags |= LO_X;
	if (end.x > boxMax.x)
		endFlags |= HI_X;

	if (end.y < boxMin.y)
		endFlags |= LO_Y;
	if (end.y > boxMax.y)
		endFlags |= HI_Y;

	if (end.z < boxMin.z)
		endFlags |= LO_Z;
	if (end.z > boxMax.z)
		endFlags |= HI_Z;

	// trivial reject
	if (startFlags & endFlags)
		return false;

	/// @todo Do exact line/box intersection check

	return true;
}


extern void UTIL_DrawBox( Extent *extent, int lifetime, int red, int green, int blue );

//--------------------------------------------------------------------------------------------------------------
/**
 * When bot is touched by another entity.
 */
void CDABot::Touch( CBaseEntity *other )
{
	// EXTEND
	BaseClass::Touch( other );

	// if we have touched a higher-priority player, make way
	/// @todo Need to account for reaction time, etc.
	if (other->IsPlayer())
	{
		// if we are on a ladder, don't move
		if (IsUsingLadder())
			return;

		CSDKPlayer *player = static_cast<CSDKPlayer *>( other );

		// get priority of other player
		unsigned int otherPri = TheDABots()->GetPlayerPriority( player );

		// get our priority
		unsigned int myPri = TheDABots()->GetPlayerPriority( this );

		// if our priority is better, don't budge
		if (myPri < otherPri)
			return;

		// they are higher priority - make way, unless we're already making way for someone more important
		if (m_avoid != NULL)
		{
			unsigned int avoidPri = TheDABots()->GetPlayerPriority( static_cast<CBasePlayer *>( static_cast<CBaseEntity *>( m_avoid ) ) );
			if (avoidPri < otherPri)
			{
				// ignore 'other' because we're already avoiding someone better
				return;
			}
		}

		m_avoid = other;
		m_avoidTimestamp = gpGlobals->curtime;
	}

	// Check for breakables we're actually touching
	// If we're not stuck or crouched, we don't care
	if ( !m_isStuck && !IsCrouching() && !IsOnLadder() )
		return;

	// See if it's breakable
	if ( IsBreakableEntity( other ) )
	{
		// it's breakable - try to shoot it.
		SetLookAt( "Breakable", other->WorldSpaceCenter(), PRIORITY_HIGH, 0.1f, false, 5.0f, true );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we are busy doing something important
 */
bool CDABot::IsBusy( void ) const
{
	if (IsAttacking() || 
		IsBuying() ||
		IsSniping())
	{
		return true;
	}

	return false;
}

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

//--------------------------------------------------------------------------------------------------------------
void CDABot::BotDeathThink( void )
{
	if (!IsAlive() && !IsBuying())
		Buy();

	if (IsBuying())
		m_state->OnUpdate( this );

	if (m_iLoadoutWeight)
		State_Transition( STATE_ACTIVE );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Try to join the given team
 */
void CDABot::TryToJoinTeam( int team )
{
	m_desiredTeam = team;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Assign given player as our current enemy to attack
 */
void CDABot::SetBotEnemy( CSDKPlayer *enemy )
{
	if (m_enemy != enemy)
	{
		m_enemy = enemy; 
		m_currentEnemyAcquireTimestamp = gpGlobals->curtime;

		PrintIfWatched( "SetBotEnemy: %s\n", (enemy) ? enemy->GetPlayerName() : "(NULL)" );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * If we are not on the navigation mesh (m_currentArea == NULL),
 * move towards last known area.
 * Return false if off mesh.
 */
bool CDABot::StayOnNavMesh( void )
{
	if (m_currentArea == NULL)
	{
		// move back onto the area map

		// if we have no lastKnownArea, we probably started off
		// of the nav mesh - find the closest nav area and use it
		CNavArea *goalArea;
		if (!m_currentArea && !m_lastKnownArea)
		{
			goalArea = TheNavMesh->GetNearestNavArea( GetCentroid() );
			PrintIfWatched( "Started off the nav mesh - moving to closest nav area...\n" );
		}
		else
		{
			goalArea = m_lastKnownArea;
			PrintIfWatched( "Getting out of NULL area...\n" );
		}

		if (goalArea)
		{
			Vector pos;
			goalArea->GetClosestPointOnArea( GetCentroid(), &pos );

			// move point into area
			Vector to = pos - GetCentroid();
			to.NormalizeInPlace();

			const float stepInDist = 5.0f;		// how far to "step into" an area - must be less than min area size
			pos = pos + (stepInDist * to);

			MoveTowardsPosition( pos );
		}

		// if we're stuck, try to get un-stuck
		// do stuck movements last, so they override normal movement
		if (m_isStuck)
			Wiggle();
				
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we will do scenario-related tasks
 */
bool CDABot::IsDoingScenario( void ) const
{
	// if we are deferring to humans, and there is a live human on our team, don't do the scenario
	if (cv_bot_defer_to_human.GetBool())
	{
		if (UTIL_HumansOnTeam( GetTeamNumber(), IS_ALIVE ))
			return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return last enemy that hurt us
 */
CSDKPlayer *CDABot::GetAttacker( void ) const
{
	if (m_attacker && m_attacker->IsAlive())
		return m_attacker;

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Immediately jump off of our ladder, if we're on one
 */
void CDABot::GetOffLadder( void )
{
	if (IsUsingLadder())
	{
		Jump( MUST_JUMP );
		DestroyPath();
	}
}



//--------------------------------------------------------------------------------------------------------------
/**
 * Return time when given spot was last checked
 */
float CDABot::GetHidingSpotCheckTimestamp( HidingSpot *spot ) const
{
	for( int i=0; i<m_checkedHidingSpotCount; ++i )
		if (m_checkedHidingSpot[i].spot->GetID() == spot->GetID())
			return m_checkedHidingSpot[i].timestamp;

	return -999999.9f;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Set the timestamp of the given spot to now.
 * If the spot is not in the set, overwrite the least recently checked spot.
 */
void CDABot::SetHidingSpotCheckTimestamp( HidingSpot *spot )
{
	int leastRecent = 0;
	float leastRecentTime = gpGlobals->curtime + 1.0f;

	for( int i=0; i<m_checkedHidingSpotCount; ++i )
	{
		// if spot is in the set, just update its timestamp
		if (m_checkedHidingSpot[i].spot->GetID() == spot->GetID())
		{
			m_checkedHidingSpot[i].timestamp = gpGlobals->curtime;
			return;
		}

		// keep track of least recent spot
		if (m_checkedHidingSpot[i].timestamp < leastRecentTime)
		{
			leastRecentTime = m_checkedHidingSpot[i].timestamp;
			leastRecent = i;
		}
	}

	// if there is room for more spots, append this one
	if (m_checkedHidingSpotCount < MAX_CHECKED_SPOTS)
	{
		m_checkedHidingSpot[ m_checkedHidingSpotCount ].spot = spot;
		m_checkedHidingSpot[ m_checkedHidingSpotCount ].timestamp = gpGlobals->curtime;
		++m_checkedHidingSpotCount;
	}
	else
	{
		// replace the least recent spot
		m_checkedHidingSpot[ leastRecent ].spot = spot;
		m_checkedHidingSpot[ leastRecent ].timestamp = gpGlobals->curtime;
	}
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we are outnumbered by enemies
 */
bool CDABot::IsOutnumbered( void ) const
{
	return (GetNearbyFriendCount() < GetNearbyEnemyCount()-1) ? true : false;		
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of enemies we are outnumbered by
 */
int CDABot::OutnumberedCount( void ) const
{
	if (IsOutnumbered())
		return (GetNearbyEnemyCount()-1) - GetNearbyFriendCount();

	return 0;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return the closest "important" enemy for the given scenario (bomb carrier, VIP, hostage escorter)
 */
CSDKPlayer *CDABot::GetImportantEnemy( bool checkVisibility ) const
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );
	CSDKPlayer *nearEnemy = NULL;
	float nearDist = 999999999.9f;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *entity = UTIL_PlayerByIndex( i );

		if (entity == NULL)
			continue;

//		if (FNullEnt( entity->pev ))
//			continue;

//		if (FStrEq( STRING( entity->pev->netname ), "" ))
//			continue;

		// is it a player?
		if (!entity->IsPlayer())
			continue;

		CSDKPlayer *player = static_cast<CSDKPlayer *>( entity );

		// is it alive?
		if (!player->IsAlive())
			continue;

		// skip friends
		if (InSameTeam( player ))
			continue;

		// is it "important"
		if (!ctrl->IsImportantPlayer( player ))
			continue;

		// is it closest?
		Vector d = GetAbsOrigin() - player->GetAbsOrigin();
		float distSq = d.x*d.x + d.y*d.y + d.z*d.z;
		if (distSq < nearDist)
		{
			if (checkVisibility && !IsVisible( player, true ))
				continue;

			nearEnemy = player;
			nearDist = distSq;
		}
	}

	return nearEnemy;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Sets our current disposition
 */
void CDABot::SetDisposition( DispositionType disposition ) 
{ 
	m_disposition = disposition;

	if (m_disposition != IGNORE_ENEMIES)
		m_ignoreEnemiesTimer.Invalidate();
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return our current disposition
 */
CDABot::DispositionType CDABot::GetDisposition( void ) const
{
	if (!m_ignoreEnemiesTimer.IsElapsed())
		return IGNORE_ENEMIES;
	
	return m_disposition;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Ignore enemies for a short durationy
 */
void CDABot::IgnoreEnemies( float duration )
{
	m_ignoreEnemiesTimer.Start( duration );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Increase morale one step
 */
void CDABot::IncreaseMorale( void )
{
	if (m_morale < EXCELLENT)
		m_morale = static_cast<MoraleType>( m_morale + 1 );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Decrease morale one step
 */
void CDABot::DecreaseMorale( void )
{
	if (m_morale > TERRIBLE)
		m_morale = static_cast<MoraleType>( m_morale - 1 );
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we are acting like a rogue (not listening to teammates, not doing scenario goals)
 * @todo Account for morale
 */
bool CDABot::IsRogue( void ) const
{ 
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );
	if (!ctrl->AllowRogues())
		return false;

	// periodically re-evaluate our rogue status
	if (m_rogueTimer.IsElapsed())
	{
		m_rogueTimer.Start( RandomFloat( 10.0f, 30.0f ) );

		// our chance of going rogue is inversely proportional to our teamwork attribute
		const float rogueChance = 100.0f * (1.0f - GetProfile()->GetTeamwork());

		m_isRogue = (RandomFloat( 0, 100 ) < rogueChance);
	}

	return m_isRogue; 
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we are in a hurry 
 */
bool CDABot::IsHurrying( void ) const
{
	if (!m_hurryTimer.IsElapsed())
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if it is the early, "safe", part of the round
 */
bool CDABot::IsSafe( void ) const
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );

	if (ctrl->GetElapsedRoundTime() < m_safeTime)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if it is well past the early, "safe", part of the round
 */
bool CDABot::IsWellPastSafe( void ) const
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );

	if (ctrl->GetElapsedRoundTime() > 2.0f * m_safeTime)
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we were in the safe time last update, but not now
 */
bool CDABot::IsEndOfSafeTime( void ) const
{
	return m_wasSafe && !IsSafe();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the amount of "safe time" we have left
 */
float CDABot::GetSafeTimeRemaining( void ) const
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );

	return m_safeTime - ctrl->GetElapsedRoundTime();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Called when enemy seen to adjust safe time for this round
 */
void CDABot::AdjustSafeTime( void )
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );

	// if we spotted an enemy sooner than we thought possible, adjust our notion of "safe" time
	if (ctrl->GetElapsedRoundTime() < m_safeTime)
	{
		// since right now is not safe, adjust safe time to be a few seconds ago
		m_safeTime = ctrl->GetElapsedRoundTime() - 2.0f;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if we haven't seen an enemy for "a long time"
 */
bool CDABot::HasNotSeenEnemyForLongTime( void ) const
{
	const float longTime = 30.0f;
	return (GetTimeSinceLastSawEnemy() > longTime);
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Pick a random zone and hide near it
 */
bool CDABot::GuardRandomZone( float range )
{
	CDABotManager *ctrl = static_cast<CDABotManager *>( TheDABots() );

	const CDABotManager::Zone *zone = ctrl->GetRandomZone();
	if (zone)
	{
		CNavArea *rescueArea = ctrl->GetRandomAreaInZone( zone );
		if (rescueArea)
		{
			Hide( rescueArea, -1.0f, range );
			return true;
		}
	}

	return false;
}



//--------------------------------------------------------------------------------------------------------------
class CollectRetreatSpotsFunctor
{
public:
	CollectRetreatSpotsFunctor( CDABot *me, float range )
	{
		m_me = me;
		m_count = 0;
		m_range = range;
	}

	enum { MAX_SPOTS = 256 };

	bool operator() ( CNavArea *area )
	{
		// collect all the hiding spots in this area
		const HidingSpotList *list = area->GetHidingSpotList();

		FOR_EACH_LL( (*list), it )
		{
			const HidingSpot *spot = (*list)[ it ];

			if (m_count >= MAX_SPOTS)
				break;

			// make sure hiding spot is in range
			if (m_range > 0.0f)
				if ((spot->GetPosition() - GetCentroid( m_me )).IsLengthGreaterThan( m_range ))
					continue;

			// if a Player is using this hiding spot, don't consider it
			if (IsSpotOccupied( m_me, spot->GetPosition() ))
			{
				// player is in hiding spot
				/// @todo Check if player is moving or sitting still
				continue;
			}

			if (SDKGameRules()->IsTeamplay())
			{
				// don't select spot if an enemy can see it
				if (UTIL_IsVisibleToTeam( spot->GetPosition() + Vector( 0, 0, HalfHumanHeight ), OtherTeam( m_me->GetTeamNumber() ) ))
					continue;
			}

			// don't select spot if it is closest to an enemy
			CBasePlayer *owner = UTIL_GetClosestPlayer( spot->GetPosition() );
			if (owner && !m_me->InSameTeam( owner ))
				continue;

			m_spot[ m_count++ ] = &spot->GetPosition();
		}

		// if we've filled up, stop searching
		if (m_count == MAX_SPOTS)
			return false;

		return true;
	}

	CDABot *m_me;
	float m_range;

	const Vector *m_spot[ MAX_SPOTS ];
	int m_count;
};


/**
 * Do a breadth-first search to find a good retreat spot.
 * Don't pick a spot that a Player is currently occupying.
 */
const Vector *FindNearbyRetreatSpot( CDABot *me, float maxRange )
{
	CNavArea *area = me->GetLastKnownArea();
	if (area == NULL)
		return NULL;

	// collect spots that enemies cannot see
	CollectRetreatSpotsFunctor collector( me, maxRange );
	SearchSurroundingAreas( area, GetCentroid( me ), collector, maxRange );

	if (collector.m_count == 0)
		return NULL;

	// select a hiding spot at random
	int which = RandomInt( 0, collector.m_count-1 );
	return collector.m_spot[ which ];
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return string describing current task
 * NOTE: This MUST be kept in sync with the CDABot::TaskType enum
 */
const char *CDABot::GetTaskName( void ) const
{
	static char *name[ NUM_TASKS ] = 
	{
		"SEEK_AND_DESTROY",
		"GUARD_INITIAL_ENCOUNTER",
		"HOLD_POSITION",
		"FOLLOW",
		"MOVE_TO_LAST_KNOWN_ENEMY_POSITION",
		"MOVE_TO_SNIPER_SPOT",
		"SNIPING",
	};

	return name[ (int)GetTask() ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return string describing current disposition
 * NOTE: This MUST be kept in sync with the CDABot::DispositionType enum
 */
const char *CDABot::GetDispositionName( void ) const
{
	static char *name[ NUM_DISPOSITIONS ] = 
	{
		"ENGAGE_AND_INVESTIGATE",
		"OPPORTUNITY_FIRE",
		"SELF_DEFENSE",
		"IGNORE_ENEMIES"
	};

	return name[ (int)GetDisposition() ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Return string describing current morale
 * NOTE: This MUST be kept in sync with the CDABot::MoraleType enum
 */
const char *CDABot::GetMoraleName( void ) const
{
	static char *name[ EXCELLENT - TERRIBLE + 1 ] = 
	{
		"TERRIBLE",
		"BAD",
		"NEGATIVE",
		"NEUTRAL",
		"POSITIVE",
		"GOOD",
		"EXCELLENT"
	};

	return name[ (int)GetMorale() + 3 ];
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Fill in a CUserCmd with our data
 */
void CDABot::BuildUserCmd( CUserCmd& cmd, const QAngle& viewangles, float forwardmove, float sidemove, float upmove, int buttons, byte impulse )
{
	Q_memset( &cmd, 0, sizeof( cmd ) );
	if ( RunMimicCommand( cmd ) )
		return;

	if ( ForceFireWeapon( cmd ) )
		return;

	// Don't walk when ducked - it's painfully slow
	if ( m_Local.m_bDucked || m_Local.m_bDucking )
	{
		buttons &= ~IN_SPEED;
	}

	cmd.command_number = gpGlobals->tickcount;
	cmd.forwardmove = forwardmove;
	cmd.sidemove = sidemove;
	cmd.upmove = upmove;
	cmd.buttons = buttons;
	cmd.impulse = impulse;

	VectorCopy( viewangles, cmd.viewangles );
	cmd.random_seed = random->RandomInt( 0, 0x7fffffff );
}

// If bots are being forced to fire a weapon, see if I have it
bool CDABot::ForceFireWeapon( CUserCmd &cmd )
{
	if ( FStrEq(cv_bot_forcefireweapon.GetString(), "") )
		return false;

	CBaseCombatWeapon *pWeapon = Weapon_OwnsThisType( cv_bot_forcefireweapon.GetString() );
	if ( pWeapon )
	{
		// Switch to it if we don't have it out
		CBaseCombatWeapon *pPrimaryWeapon = GetActiveSDKWeapon();

		// Switch?
		if ( pPrimaryWeapon != pWeapon )
		{
			Weapon_Switch( pWeapon );
		}
		else
		{
			// Start firing
			// Some weapons require releases, so randomise firing
			static bool bFire = false;
			bFire = !bFire;
			if ( cv_bot_forceattackon.GetBool() || bFire )
			{
				cmd.buttons |= cv_bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------

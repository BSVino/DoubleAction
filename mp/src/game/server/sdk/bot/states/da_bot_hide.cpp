//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#include "cbase.h"
#include "da_bot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//--------------------------------------------------------------------------------------------------------------
/**
 * Begin moving to a nearby hidey-hole.
 * NOTE: Do not forget this state may include a very long "move-to" time to get to our hidey spot!
 */
void HideState::OnEnter( CDABot *me )
{
	m_isAtSpot = false;
	m_isLookingOutward = false;

	// if duration is "infinite", set it to a reasonably long time to prevent infinite camping
	if (m_duration < 0.0f)
	{
		m_duration = RandomFloat( 30.0f, 60.0f );
	}

	// decide whether to "ambush" or not - never set to false so as not to override external setting
	if (RandomFloat( 0.0f, 100.0f ) < 50.0f)
	{
		m_isHoldingPosition = true;
	}

	// if we are holding position, decide for how long
	if (m_isHoldingPosition)
	{
		m_holdPositionTime = RandomFloat( 3.0f, 10.0f );
	}
	else
	{
		m_holdPositionTime = 0.0f;
	}

	m_heardEnemy = false;
	m_firstHeardEnemyTime = 0.0f;
	m_retry = 0;

	if (me->IsFollowing())
	{
		m_leaderAnchorPos = GetCentroid( me->GetFollowLeader() );
	}

	// if we are a sniper, we need to periodically pause while we retreat to squeeze off a shot or two
	if (me->IsSniper())
	{
		// start off paused to allow a final shot before retreating
		m_isPaused = false;
		m_pauseTimer.Invalidate();
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Move to a nearby hidey-hole.
 * NOTE: Do not forget this state may include a very long "move-to" time to get to our hidey spot!
 */
void HideState::OnUpdate( CDABot *me )
{
	Vector myOrigin = GetCentroid( me );

	// wait until finished reloading to leave hide state
	if (!me->IsReloading())
	{
		// if we are momentarily hiding while following someone, check to see if he has moved on
		if (me->IsFollowing())
		{
			CSDKPlayer *leader = static_cast<CSDKPlayer *>( static_cast<CBaseEntity *>( me->GetFollowLeader() ) );
			Vector leaderOrigin = GetCentroid( leader );

			// BOTPORT: Determine walk/run velocity thresholds
			float runThreshold = 200.0f;
			if (leader->GetAbsVelocity().IsLengthGreaterThan( runThreshold ))
			{
				// leader is running, stay with him
				me->Follow( leader );
				return;			
			}

			// if leader has moved, stay with him
			const float followRange = 250.0f;
			if ((m_leaderAnchorPos - leaderOrigin).IsLengthGreaterThan( followRange ))
			{
				me->Follow( leader );
				return;
			}
		}

		// if we see a nearby buddy in combat, join him
		/// @todo - Perhaps tie in to TakeDamage(), so it works for human players, too

		bool isSettledInSniper = (me->IsSniper() && m_isAtSpot) ? true : false;

		// only investigate noises if we are initiating attacks, and we aren't a "settled in" sniper
		// dont investigate noises if we are reloading
		if (!me->IsReloading() && 
			!isSettledInSniper && 
			me->GetDisposition() == CDABot::ENGAGE_AND_INVESTIGATE)
		{
			// if we are holding position, and have heard the enemy nearby, investigate after our hold time is up
			if (m_isHoldingPosition && m_heardEnemy && (gpGlobals->curtime - m_firstHeardEnemyTime > m_holdPositionTime))
			{
				/// @todo We might need to remember specific location of last enemy noise here
				me->InvestigateNoise();
				return;
			}

			// investigate nearby enemy noises
			if (me->HeardInterestingNoise())
			{
				// if we are holding position, check if enough time has elapsed since we first heard the enemy
				if (m_isAtSpot && m_isHoldingPosition)
				{
					if (!m_heardEnemy)
					{
						// first time we heard the enemy
						m_heardEnemy = true;
						m_firstHeardEnemyTime = gpGlobals->curtime;
						me->PrintIfWatched( "Heard enemy, holding position for %f2.1 seconds...\n", m_holdPositionTime );
					}
				}
				else
				{
					// not holding position - investigate enemy noise
					me->InvestigateNoise();
					return;
				}
			}
		}
	}	// end reloading check

	// look around
	me->UpdateLookAround();

	// if we are at our hiding spot, crouch and wait
	if (m_isAtSpot)
	{
		me->ResetStuckMonitor();

		CNavArea *area = TheNavMesh->GetNavArea( m_hidingSpot );
		if ( !area || !( area->GetAttributes() & NAV_MESH_STAND ) )
		{
			me->Crouch();
		}

		// check if duration has expired
		if (m_hideTimer.IsElapsed()) 
		{
			me->Idle();
			return;
		}

/*
		// if we are watching for an approaching noisy enemy, anticipate and fire before they round the corner
		/// @todo Need to check if we are looking at an ENEMY_NOISE here
		const float veryCloseNoise = 250.0f;
		if (me->IsLookingAtSpot() && me->GetNoiseRange() < veryCloseNoise)
		{
			// fire!
			me->PrimaryAttack();
			me->PrintIfWatched( "Firing at anticipated enemy coming around the corner!\n" );
		}
*/

		// while sitting at our hiding spot, if we are being attacked but can't see our attacker, move somewhere else
		const float hurtRecentlyTime = 1.0f;
		if (!me->IsEnemyVisible() && me->GetTimeSinceAttacked() < hurtRecentlyTime)
		{
			me->Idle();
			return;
		}
	}
	else
	{
		// we are moving to our hiding spot

		// snipers periodically pause and fire while retreating
		if (me->IsSniper() && me->IsEnemyVisible())
		{
			if (m_isPaused)
			{
				if (m_pauseTimer.IsElapsed())
				{
					// get moving
					m_isPaused = false;
					m_pauseTimer.Start( RandomFloat( 1.0f, 3.0f ) );
				}
				else
				{
					me->Wait( 0.2f );
				}
			}
			else
			{
				if (m_pauseTimer.IsElapsed())
				{
					// pause for a moment
					m_isPaused = true;
					m_pauseTimer.Start( RandomFloat( 0.5f, 1.5f ) );
				}
			}
		}

		// if a Player is using this hiding spot, give up
		float range;
		CSDKPlayer *camper = static_cast<CSDKPlayer *>( UTIL_GetClosestPlayer( m_hidingSpot, &range ) );

		const float closeRange = 75.0f;
		if (camper && camper != me && range < closeRange && me->IsVisible( camper, true ))
		{
			// player is in our hiding spot
			me->PrintIfWatched( "Someone's in my hiding spot - picking another...\n" );

			const int maxRetries = 3;
			if (m_retry++ >= maxRetries)
			{
				me->PrintIfWatched( "Can't find a free hiding spot, giving up.\n" );
				me->Idle();
				return;
			}

			// pick another hiding spot near where we were planning on hiding
			me->Hide( TheNavMesh->GetNavArea( m_hidingSpot ) );

			return;
		}

		Vector toSpot;
		toSpot.x = m_hidingSpot.x - myOrigin.x;
		toSpot.y = m_hidingSpot.y - myOrigin.y;
		toSpot.z = m_hidingSpot.z - me->GetFeetZ(); // use feet location
		range = toSpot.Length();

		// look outwards as we get close to our hiding spot
		if (!me->IsEnemyVisible() && !m_isLookingOutward)
		{
			const float lookOutwardRange = 200.0f;
			const float nearSpotRange = 10.0f;
			if (range < lookOutwardRange && range > nearSpotRange)
			{
				m_isLookingOutward = true;

				toSpot.x /= range;
				toSpot.y /= range;
				toSpot.z /= range;

				me->SetLookAt( "Face outward", me->EyePosition() - 1000.0f * toSpot, PRIORITY_HIGH, 3.0f );
			}
		}

		const float atDist = 20.0f;
		if (range < atDist)
		{
			//-------------------------------------
			// Just reached our hiding spot
			//
			m_isAtSpot = true;
			m_hideTimer.Start( m_duration );

			// make sure our approach points are valid, since we'll be watching them
			me->ComputeApproachPoints();
			me->ClearLookAt();

			// ready our weapon and prepare to attack
			me->EquipBestWeapon( me->IsUsingGrenade() );
			me->SetDisposition( CDABot::OPPORTUNITY_FIRE );

			// if we are a sniper, update our task
			if (me->GetTask() == CDABot::MOVE_TO_SNIPER_SPOT)
			{
				me->SetTask( CDABot::SNIPING );
			}
			else if (me->GetTask() == CDABot::GUARD_INITIAL_ENCOUNTER)
			{
				const float campChatterChance = 20.0f;
				if (RandomFloat( 0, 100 ) < campChatterChance)
				{
					me->GetChatter()->Say( "WaitingHere" );
				}
			}

			
			// determine which way to look
			trace_t result;
			float outAngle = 0.0f;
			float outAngleRange = 0.0f;
			for( float angle = 0.0f; angle < 360.0f; angle += 45.0f )
			{
				UTIL_TraceLine( me->EyePosition(), me->EyePosition() + 1000.0f * Vector( BotCOS(angle), BotSIN(angle), 0.0f ), MASK_PLAYERSOLID, me, COLLISION_GROUP_NONE, &result );

				if (result.fraction > outAngleRange)
				{
					outAngle = angle;
					outAngleRange = result.fraction;
				}
			}

			me->SetLookAheadAngle( outAngle );

		}

		// move to hiding spot
		if (me->UpdatePathMovement() != CDABot::PROGRESSING && !m_isAtSpot)
		{
			// we couldn't get to our hiding spot - pick another
			me->PrintIfWatched( "Can't get to my hiding spot - finding another...\n" );

			// search from hiding spot, since we know it was valid
			const Vector *pos = FindNearbyHidingSpot( me, m_hidingSpot, m_range, me->IsSniper() );
			if (pos == NULL)
			{
				// no available hiding spots
				me->PrintIfWatched( "No available hiding spots - hiding where I'm at.\n" );

				// hide where we are
				m_hidingSpot.x = myOrigin.x;
				m_hidingSpot.x = myOrigin.y;
				m_hidingSpot.z = me->GetFeetZ();
			}
			else
			{
				m_hidingSpot = *pos;
			}

			// build a path to our new hiding spot
			if (me->ComputePath( m_hidingSpot, FASTEST_ROUTE ) == false)
			{
				me->PrintIfWatched( "Can't pathfind to hiding spot\n" );
				me->Idle();
				return;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
void HideState::OnExit( CDABot *me )
{
	m_isHoldingPosition = false;

	me->StandUp();
	me->ResetStuckMonitor();
	//me->ClearLookAt();
	me->ClearApproachPoints();
}

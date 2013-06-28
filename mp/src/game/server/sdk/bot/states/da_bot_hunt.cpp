//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#include "cbase.h"
#include "da_bot.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//--------------------------------------------------------------------------------------------------------------
/**
 * Begin the hunt
 */
void HuntState::OnEnter( CDABot *me )
{
	me->StandUp();
	me->SetDisposition( CDABot::ENGAGE_AND_INVESTIGATE );
	me->SetTask( CDABot::SEEK_AND_DESTROY );

	me->DestroyPath();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Hunt down our enemies
 */
void HuntState::OnUpdate( CDABot *me )
{
	// if we've been hunting for a long time, drop into Idle for a moment to
	// select something else to do
	const float huntingTooLongTime = 30.0f;
	if (gpGlobals->curtime - me->GetStateTimestamp() > huntingTooLongTime)
	{
		// stop being a rogue and do the scenario, since there must not be many enemies left to hunt
		me->PrintIfWatched( "Giving up hunting.\n" );
		me->SetRogue( false );
		me->Idle();
		return;
	}

	// listen for enemy noises
	if (me->HeardInterestingNoise())
	{
		me->InvestigateNoise();
		return;
	}
		
	// look around
	me->UpdateLookAround();

	// if we have reached our destination area, pick a new one
	// if our path fails, pick a new one
	if (me->GetLastKnownArea() == m_huntArea || me->UpdatePathMovement() != CDABot::PROGRESSING)
	{
		// pick a new hunt area
		const float earlyGameTime = 45.0f;
		if (SDKGameRules()->IsTeamplay() && TheDABots()->GetElapsedRoundTime() < earlyGameTime && !me->HasVisitedEnemySpawn())
		{
			// in the early game, rush the enemy spawn
			CBaseEntity *enemySpawn = TheDABots()->GetRandomSpawn( OtherTeam( me->GetTeamNumber() ) );

			//ADRIAN: REVISIT
			if ( enemySpawn )
			{
				m_huntArea = TheNavMesh->GetNavArea( enemySpawn->WorldSpaceCenter() );
			}
		}
		else
		{
			m_huntArea = NULL;
			float oldest = 0.0f;

			int areaCount = 0;
			const float minSize = 150.0f;

			FOR_EACH_LL( TheNavAreaList, it )
			{
				CNavArea *area = TheNavAreaList[ it ];

				++areaCount;

				// skip the small areas
				const Extent &extent = area->GetExtent();
				if (extent.hi.x - extent.lo.x < minSize || extent.hi.y - extent.lo.y < minSize)
					continue;

				// keep track of the least recently cleared area
				float age = gpGlobals->curtime - area->GetClearedTimestamp( me->GetTeamNumber()-1 );
				if (age > oldest)
				{
					oldest = age;
					m_huntArea = area;
				}
			}

			// if all the areas were too small, pick one at random
			int which = RandomInt( 0, areaCount-1 );

			areaCount = 0;
			FOR_EACH_LL( TheNavAreaList, hit )
			{
				m_huntArea = TheNavAreaList[ hit ];

				if (which == areaCount)
					break;

				--which;
			}
		}

		if (m_huntArea)
		{
			// create a new path to a far away area of the map
			me->ComputePath( m_huntArea->GetCenter() );
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Done hunting
 */
void HuntState::OnExit( CDABot *me )
{
}

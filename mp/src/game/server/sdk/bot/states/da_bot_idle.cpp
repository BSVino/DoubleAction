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

// range for snipers to select a hiding spot
const float sniperHideRange = 2000.0f;

//--------------------------------------------------------------------------------------------------------------
/**
 * The Idle state.
 * We never stay in the Idle state - it is a "home base" for the state machine that
 * does various checks to determine what we should do next.
 */
void IdleState::OnEnter( CDABot *me )
{
	me->DestroyPath();
	me->SetBotEnemy( NULL );

	//
	// Since Idle assigns tasks, we assume that coming back to Idle means our task is complete
	//
	me->SetTask( CDABot::SEEK_AND_DESTROY );
	me->SetDisposition( CDABot::ENGAGE_AND_INVESTIGATE );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Determine what we should do next
 */
void IdleState::OnUpdate( CDABot *me )
{
	// all other states assume GetLastKnownArea() is valid, ensure that it is
	if (me->GetLastKnownArea() == NULL && me->StayOnNavMesh() == false)
		return;

	// zombies never leave the Idle state
	if (cv_bot_zombie.GetBool())
	{
		me->ResetStuckMonitor();
		return;
	}

	// if round is over, hunt
	if (me->GetGameState()->IsRoundOver())
	{
		me->Hunt();
		return;
	}

	const float offenseSniperCampChance = 10.0f;

	// if we were following someone, continue following them
	if (me->IsFollowing())
	{
		me->ContinueFollowing();
		return;
	}

	//
	// Scenario logic
	//
	switch (TheDABots()->GetScenario())
	{
		case 0:
		default:	// deathmatch
		{
			// sniping check
			if (me->GetFriendsRemaining() && me->IsSniper() && RandomFloat( 0, 100.0f ) < offenseSniperCampChance)
			{
				me->SetTask( CDABot::MOVE_TO_SNIPER_SPOT );
				me->Hide( me->GetLastKnownArea(), RandomFloat( 10.0f, 30.0f ), sniperHideRange );
				me->SetDisposition( CDABot::OPPORTUNITY_FIRE );
				me->PrintIfWatched( "Sniping!\n" );
				return;
			}
			break;
		}
	}

	// if we have nothing special to do, go hunting for enemies
	me->Hunt();
}


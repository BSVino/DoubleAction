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


/**
 * Face the entity and "use" it
 * NOTE: This state assumes we are standing in range of the entity to be used, with no obstructions.
 */
void UseEntityState::OnEnter( CDABot *me )
{
}

void UseEntityState::OnUpdate( CDABot *me )
{
	// in the very rare situation where two or more bots "used" a hostage at the same time,
	// one bot will fail and needs to time out of this state
	const float useTimeout = 5.0f;
	if (me->GetStateTimestamp() - gpGlobals->curtime > useTimeout)
	{
		me->Idle();
		return;
	}

	if (m_entity == NULL)
	{
		me->Idle();
		return;
	}

	// look at the entity
	Vector pos = m_entity->EyePosition();
	me->SetLookAt( "Use entity", pos, PRIORITY_HIGH );

	// if we are looking at the entity, "use" it and exit
	if (me->IsLookingAtPosition( pos ))
	{
		me->UseEnvironment();
		me->Idle();
	}
}

void UseEntityState::OnExit( CDABot *me )
{
	me->ClearLookAt();
	me->ResetStuckMonitor();
}




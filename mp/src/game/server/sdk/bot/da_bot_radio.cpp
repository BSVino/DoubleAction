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

extern int gmsgBotVoice;

//--------------------------------------------------------------------------------------------------------------
/**
 * Decide if we should move to help the player, return true if we will
 */
bool CDABot::RespondToHelpRequest( CSDKPlayer *them, Place place, float maxRange )
{
	if (IsRogue())
		return false;

	// if we're busy, ignore
	if (IsBusy())
		return false;

	Vector themOrigin = them->GetCentroid();

	// if we are too far away, ignore
	if (maxRange > 0.0f)
	{
		// compute actual travel distance
		PathCost cost(this);
		float travelDistance = NavAreaTravelDistance( m_lastKnownArea, TheNavMesh->GetNearestNavArea( themOrigin ), cost );
		if (travelDistance < 0.0f)
			return false;

		if (travelDistance > maxRange)
			return false;
	}


	if (place == UNDEFINED_PLACE)
	{
		// if we have no "place" identifier, go directly to them

		// if we are already there, ignore
		float rangeSq = (them->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
		const float close = 750.0f * 750.0f;
		if (rangeSq < close)
			return true;

		MoveTo( themOrigin, FASTEST_ROUTE );
	}
	else
	{
		// if we are already there, ignore
		if (GetPlace() == place)
			return true;

		// go to where help is needed
		const Vector *pos = GetRandomSpotAtPlace( place );
		if (pos)
		{
			MoveTo( *pos, FASTEST_ROUTE );
		}
		else
		{
			MoveTo( themOrigin, FASTEST_ROUTE );
		}
	}

	// acknowledge
	GetChatter()->Say( "OnMyWay" );

	return true;
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Send voice chatter.  Also sends the entindex and duration for voice feedback.
 */
void CDABot::SpeakAudio( const char *voiceFilename, float duration, int pitch )
{
	if( !IsAlive() )
		return;

	if ( IsObserver() )
		return;

	CRecipientFilter filter;

	UserMessageBegin ( filter, "RawAudio" );
		WRITE_BYTE( pitch );
		WRITE_BYTE( entindex() );
		WRITE_FLOAT( duration );
		WRITE_STRING( voiceFilename );
	MessageEnd();

	GetChatter()->ResetRadioSilenceDuration();

	m_voiceEndTimestamp = gpGlobals->curtime + duration;
}


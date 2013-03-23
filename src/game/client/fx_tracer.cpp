//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "basecombatweapon_shared.h"
#include "baseviewmodel_shared.h"
#include "particles_new.h"
#include "c_sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	TRACER_SPEED			5000 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector GetTracerOrigin( const CEffectData &data )
{
	Vector vecStart = data.m_vStart;
	QAngle vecAngles;

	int iAttachment = data.m_nAttachmentIndex;;

	// Attachment?
	if ( data.m_fFlags & TRACER_FLAG_USEATTACHMENT )
	{
		C_BaseViewModel *pViewModel = NULL;

		// If the entity specified is a weapon being carried by this player, use the viewmodel instead
		IClientRenderable *pRenderable = data.GetRenderable();
		if ( !pRenderable )
			return vecStart;

		C_BaseEntity *pEnt = data.GetEntity();

		// This check should probably be for all multiplayer games, investigate later
		// 10/09/2008: It should.
		if ( gpGlobals->maxClients > 1 )
		{
			if ( pEnt && pEnt->IsDormant() )
				return vecStart;
		}

		C_BaseCombatWeapon *pWpn = dynamic_cast<C_BaseCombatWeapon *>( pEnt );
		if ( pWpn && pWpn->IsCarriedByLocalPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( pWpn->GetOwner() );

			pViewModel = player ? player->GetViewModel( 0 ) : NULL;
			if ( pViewModel )
			{
				// Get the viewmodel and use it instead
				pRenderable = pViewModel;
			}
		}

		// Get the attachment origin
		if ( !pRenderable->GetAttachment( iAttachment, vecStart, vecAngles ) )
		{
			DevMsg( "GetTracerOrigin: Couldn't find attachment %d on model %s\n", iAttachment, 
				modelinfo->GetModelName( pRenderable->GetModel() ) );
		}
	}

	return vecStart;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TracerCallback( const CEffectData &data )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	int iEntIndex = data.entindex();

	if ( iEntIndex && iEntIndex == player->index )
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles( vangles );
		AngleVectors( vangles, &vforward, &vright, &vup );

		VectorMA( data.m_vStart, 4, vright, foo );
		foo[2] -= 0.5f;

		FX_PlayerTracer( foo, (Vector&)data.m_vOrigin );
		return;
	}
	
	// Use default velocity if none specified
	if ( !flVelocity )
	{
		flVelocity = TRACER_SPEED;
	}

	// Do tracer effect
	FX_Tracer( (Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz );
}

DECLARE_CLIENT_EFFECT( "Tracer", TracerCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ParticleTracerCallback( const CEffectData &data )
{
	C_SDKPlayer *player = ToSDKPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( !player )
		return;

	// Grab the data
	//Vector vecStart = GetTracerOrigin( data ); we'll account for the view model in this function instead
	Vector vecStart = data.m_vStart;
	Vector vecEnd = data.m_vOrigin;
	int iAttachment = data.m_nAttachmentIndex;

	// Adjust view model tracers
	C_BaseEntity *pEntity = data.GetEntity();
	if(player)
	{
		if ( data.entindex() && data.entindex() == player->index )
		{
			QAngle	vangles;
			Vector	vforward, vright, vup;

			engine->GetViewAngles( vangles );
			AngleVectors( vangles, &vforward, &vright, &vup );

			VectorMA( data.m_vStart, 4, vright, vecStart );
			vecStart[2] -= 0.5f;
		}
		
		C_BaseViewModel *pViewModel = NULL;
		C_BaseCombatWeapon *pWpn = dynamic_cast<C_BaseCombatWeapon *>( pEntity );
		if ( pWpn && pWpn->IsCarriedByLocalPlayer() )
		{
			pViewModel = player ? player->GetViewModel( 0 ) : NULL;
			if ( pViewModel && !player->IsInThirdPerson() )
			{
				// Get the viewmodel and use it instead
				pEntity = pViewModel;
			}
		}
	}

	// Create the particle effect
	QAngle vecAngles;
	Vector vecToEnd = vecEnd - vecStart;
	VectorNormalize(vecToEnd);
	VectorAngles( vecToEnd, vecAngles );

	// if this tracer originates from a gun (in this case whiz is for whizard)
	if ( data.m_fFlags & TRACER_FLAG_WHIZ )
		DispatchParticleEffect( data.m_nHitBox, vecEnd, PATTACH_POINT_ORIGIN, pEntity, iAttachment, false);
	// if this tracer originates from bullet penetration
	else
		DispatchParticleEffect( data.m_nHitBox, vecStart, vecEnd, vecAngles, pEntity );

	FX_TracerSound( vecStart, vecEnd, TRACER_TYPE_DEFAULT );	
}

DECLARE_CLIENT_EFFECT( "ParticleTracer", ParticleTracerCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void TracerSoundCallback( const CEffectData &data )
{
	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	
	// Do tracer effect
	FX_TracerSound( vecStart, (Vector&)data.m_vOrigin, data.m_fFlags );
}

DECLARE_CLIENT_EFFECT( "TracerSound", TracerSoundCallback );


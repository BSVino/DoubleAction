//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "da_briefcase.h"
#include "sdk_player.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CBriefcase )
END_DATADESC()

LINK_ENTITY_TO_CLASS( da_briefcase, CBriefcase );

IMPLEMENT_SERVERCLASS_ST(CBriefcase, DT_Briefcase)
END_SEND_TABLE()

PRECACHE_REGISTER(da_briefcase);

CBriefcase::CBriefcase()
{
}

void CBriefcase::Precache( void )
{
	CBaseEntity::PrecacheModel( "particle/briefcase.vmt" );
	CBaseEntity::PrecacheModel( "models/briefcase/briefcase_01.mdl" );
}

void CBriefcase::Spawn( void )
{
	BaseClass::Spawn( );

	m_flLastTouched = -1;

	SetModel("models/briefcase/briefcase_01.mdl");
	VPhysicsDestroyObject();

	AddEffects( EF_BONEMERGE_FASTCULL );
}

int CBriefcase::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

bool CBriefcase::MyTouch( CBasePlayer *pPlayer )
{
	CSDKPlayer* pSDKPlayer = ToSDKPlayer(pPlayer);

	if (!pSDKPlayer)
		return false;

	if (!pPlayer->IsAlive())
		return false;

	VPhysicsDestroyObject();
	pSDKPlayer->PickUpBriefcase(this);

	return true;
}

void CBriefcase::Dropped( CSDKPlayer* pPlayer )
{
	m_flLastTouched = gpGlobals->curtime;

	FollowEntity(NULL);

	if (pPlayer)
		SetAbsOrigin(pPlayer->GetAbsOrigin());

	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 20));
	UTIL_DropToFloor(this, MASK_SOLID, this);

	SetCollisionBounds(-Vector(10, 10, 10), Vector(10, 10, 10));
	VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );

	if (pPlayer)
	{
		Vector vecVelocity = pPlayer->GetAbsVelocity()*2 + Vector(0, 0, random->RandomFloat(50, 200));
		vecVelocity *= 3;
		if (VPhysicsGetObject())
		{
			AngularImpulse angImp( 200, 200, 200 );
			VPhysicsGetObject()->AddVelocity( &vecVelocity, &angImp );
		}
		else
			SetAbsVelocity(vecVelocity);
	}

	SetMoveType( MOVETYPE_VPHYSICS );
	SetSolid( SOLID_BBOX );
	SetBlocksLOS( false );
	AddEFlags( EFL_NO_ROTORWASH_PUSH );

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( true, 24 );
	SetTouch(&CItem::ItemTouch);
}

BEGIN_DATADESC( CBriefcaseCaptureZone )
END_DATADESC()

LINK_ENTITY_TO_CLASS( da_briefcase_capture, CBriefcaseCaptureZone );

IMPLEMENT_SERVERCLASS_ST(CBriefcaseCaptureZone, DT_BriefcaseCaptureZone)
	SendPropFloat( SENDINFO(m_flCaptureRadius) ),
END_SEND_TABLE()

PRECACHE_REGISTER(da_briefcase_capture);

CBriefcaseCaptureZone::CBriefcaseCaptureZone()
{
	m_flCaptureRadius = 100;
}

void CBriefcaseCaptureZone::Precache( void )
{
	CBaseEntity::PrecacheModel( "da/capturezone.vmt" );
}

void CBriefcaseCaptureZone::Spawn( void )
{
	BaseClass::Spawn( );

	SetThink(&CBriefcaseCaptureZone::CaptureThink);
	SetNextThink( gpGlobals->curtime + 0.1f );

	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 20));
	UTIL_DropToFloor(this, MASK_SOLID, this);
}

int CBriefcaseCaptureZone::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CBriefcaseCaptureZone::CaptureThink()
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	float flCaptureRadiusSqr = m_flCaptureRadius * m_flCaptureRadius;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (pPlayer->HasBriefcase() && (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flCaptureRadiusSqr)
		{
			SDKGameRules()->PlayerCapturedBriefcase(pPlayer);
			return;
		}
	}
}

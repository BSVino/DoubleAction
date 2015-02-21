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
#include "te_effect_dispatch.h"

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
	CBaseEntity::PrecacheModel( "models/briefcase/briefcase_01_on_player.mdl" );
	PrecacheParticleSystem( "dinero_trail" );
	PrecacheParticleSystem( "dinero_splode" );
}

void CBriefcase::Spawn( void )
{
	BaseClass::Spawn( );

	m_flLastTouched = -1;

	SetModel("models/briefcase/briefcase_01.mdl");
	VPhysicsDestroyObject();

	AddEffects( EF_BONEMERGE_FASTCULL );

	SetSequence(LookupSequence("closed_idle"));
}

int CBriefcase::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CBriefcase::AnimateThink()
{
	// Why is this necessary.

	if (GetCycle() >= 1)
	{
		SetSequence(LookupSequence("open_idle"));
		SetCycle(0);
		return;
	}

	SetPlaybackRate(1);

	SetNextThink(gpGlobals->curtime);

	StudioFrameAdvance();
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

	SetModel("models/briefcase/briefcase_01_on_player.mdl");
	SetSequence(LookupSequence("idle"));

	DispatchParticleEffect( "dinero_trail", PATTACH_ABSORIGIN_FOLLOW, this );

	return true;
}

void CBriefcase::Touch()
{
	m_flLastTouched = gpGlobals->curtime;
}

static ConVar da_ctb_changecap("da_ctb_changecap", "0.0", FCVAR_NOTIFY|FCVAR_DEVELOPMENTONLY, "Probability to change capture point location when the briefcase is dropped.");

void CBriefcase::Dropped( CSDKPlayer* pPlayer )
{
	Touch();

	FollowEntity(NULL);

	if (pPlayer)
		SetAbsOrigin(pPlayer->GetAbsOrigin() + Vector(0, 0, 60));
	else
	{
		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 20));
		UTIL_DropToFloor(this, MASK_SOLID, this);
	}

	SetModel("models/briefcase/briefcase_01.mdl");

	SetCollisionBounds(-Vector(10, 10, 10), Vector(10, 10, 10));
	VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags() | FSOLID_TRIGGER, false );

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
	SetSolid( SOLID_VPHYSICS );
	SetBlocksLOS( false );
	AddEFlags( EFL_NO_ROTORWASH_PUSH );

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( true, 24 );
	SetTouch(&CItem::ItemTouch);

	if (pPlayer)
	{
		SetSequence(LookupSequence("opening"));
		SetThink(&CBriefcase::AnimateThink);
		SetNextThink( gpGlobals->curtime );
	}

	StopParticleEffects( this );
	DispatchParticleEffect( "dinero_splode", GetAbsOrigin(), GetAbsAngles() );

	if (random->RandomFloat(0, 1) < da_ctb_changecap.GetFloat())
		SDKGameRules()->ChooseRandomCapturePoint(GetAbsOrigin());

	if (GetAbsOrigin().z < SDKGameRules()->GetLowestSpawnPoint().z - 400)
		SDKGameRules()->RestartMiniObjective();
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
	m_flCaptureRadius = 80;
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

		if (!pPlayer->IsAlive())
			continue;

		if (pPlayer->HasBriefcase() && (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flCaptureRadiusSqr)
		{
			StopParticleEffects( SDKGameRules()->GetBriefcase() );
			DispatchParticleEffect( "dinero_splode", GetAbsOrigin(), GetAbsAngles() );

			SDKGameRules()->PlayerCapturedBriefcase(pPlayer);
			return;
		}
	}
}

BEGIN_DATADESC( CRatRaceWaypoint )
END_DATADESC()

LINK_ENTITY_TO_CLASS( da_ratrace_waypoint, CRatRaceWaypoint );

IMPLEMENT_SERVERCLASS_ST(CRatRaceWaypoint, DT_RatRaceWaypoint)
	SendPropFloat( SENDINFO(m_flRadius) ),
	SendPropInt( SENDINFO(m_iWaypoint) ),
END_SEND_TABLE()

PRECACHE_REGISTER(da_ratrace_waypoint);

CRatRaceWaypoint::CRatRaceWaypoint()
{
	m_flRadius = 40;
}

void CRatRaceWaypoint::Precache( void )
{
	CBaseEntity::PrecacheModel( "da/capturezone.vmt" );
}

void CRatRaceWaypoint::Spawn( void )
{
	BaseClass::Spawn( );

	SetThink(&CRatRaceWaypoint::WaypointThink);
	SetNextThink( gpGlobals->curtime + 0.1f );

	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 20));
	UTIL_DropToFloor(this, MASK_SOLID, this);
}

int CRatRaceWaypoint::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CRatRaceWaypoint::WaypointThink()
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	float flRadiusSqr = m_flRadius * m_flRadius;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if ((pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flRadiusSqr)
			SDKGameRules()->PlayerReachedWaypoint(pPlayer, this);
	}
}

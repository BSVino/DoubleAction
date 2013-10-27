//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_GRENADE_H
#define WEAPON_GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef GAME_DLL
#include "sdk_basegrenade_projectile.h"
#endif

#ifdef CLIENT_DLL
	
	#define CWeaponGrenade C_WeaponGrenade

#endif

#define GRENADE_TIMER 2.0f //Seconds

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponGrenade : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenade, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponGrenade() {}

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_GRENADE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponGrenade( const CWeaponGrenade & ) {}
};

#ifdef GAME_DLL

class CGrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CGrenadeProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{	return SDK_WEAPON_GRENADE; }

	// Overrides.
public:
	virtual void Spawn();

	virtual void Precache();

	// Grenade stuff.
public:

	static CGrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer );
};

#endif

#endif // WEAPON_GRENADE_H

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


#ifdef CLIENT_DLL
	
	#define CWeaponGrenade C_WeaponGrenade

#endif

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


#endif // WEAPON_GRENADE_H

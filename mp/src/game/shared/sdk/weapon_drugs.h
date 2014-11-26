//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_DRUGS_H
#define WEAPON_DRUGS_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef CLIENT_DLL
	
#define CWeaponDrugs C_WeaponDrugs

#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponDrugs : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS(CWeaponDrugs, CBaseSDKGrenade);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponDrugs() {}

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_DRUGS; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponDrugs(const CWeaponDrugs &) {}
};

#endif // WEAPON_DRUGS_H

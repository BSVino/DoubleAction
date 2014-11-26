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

#include "weapon_basedagrenade.h"

#ifdef CLIENT_DLL
	
#define CWeaponDrugs C_WeaponDrugs

#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponDrugs : public CBaseDAGrenade
{
public:
	DECLARE_CLASS(CWeaponDrugs, CBaseDAGrenade);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponDrugs() {}

	virtual DAWeaponID GetWeaponID( void ) const		{ return DA_WEAPON_DRUGS; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponDABase *pWeapon );
	
#endif

	CWeaponDrugs(const CWeaponDrugs &) {}
};

#endif // WEAPON_DRUGS_H

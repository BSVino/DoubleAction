//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_SHOTGUN_H
#define WEAPON_SHOTGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase.h"


#if defined( CLIENT_DLL )

	#define CWeaponShotgun C_WeaponShotgun
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"
	#include "te_firebullets.h"

#endif


class CWeaponShotgun : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponShotgun, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponShotgun();

	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual bool WeaponSpreadFixed() const { return true; }
	virtual bool FullAimIn() { return true; }

private:

	CWeaponShotgun( const CWeaponShotgun & );

	float m_flPumpTime;
	CNetworkVar( int, m_iInSpecialReload );

};

#endif

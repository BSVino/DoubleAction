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

#include "weapon_dabase.h"


#if defined( CLIENT_DLL )

	#define CWeaponShotgun C_WeaponShotgun
	#include "c_da_player.h"

#else

	#include "da_player.h"
	#include "te_firebullets.h"

#endif


class CWeaponShotgun : public CWeaponDABase
{
public:
	DECLARE_CLASS( CWeaponShotgun, CWeaponDABase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponShotgun();

	virtual void PrimaryAttack();
	virtual void StartSwing(bool bIsSecondary, bool bIsStockAttack = false);
	virtual bool Reload();
	virtual void WeaponIdle();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual bool WeaponSpreadFixed() const { return true; }
	virtual bool FullAimIn() { return true; }

private:

	CWeaponShotgun( const CWeaponShotgun & );

	float m_flPumpTime;
	CNetworkVar( int, m_iInSpecialReload );

};

#endif

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "da_ammo_pickup.h"
#include "ammodef.h"
#include "da_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CAmmoPickup )
END_DATADESC()

LINK_ENTITY_TO_CLASS( da_ammo_pickup, CAmmoPickup );
PRECACHE_REGISTER(da_ammo_pickup);

CAmmoPickup::CAmmoPickup()
{
	memset(m_aiAmmoCounts, 0, sizeof(m_aiAmmoCounts));
}

void CAmmoPickup::Precache( void )
{
	PrecacheModel ("models/items/boxsrounds.mdl");
}

void CAmmoPickup::Spawn( void )
{
	Precache( );
	SetModel( "models/items/boxsrounds.mdl" );
	BaseClass::Spawn( );
}

bool CAmmoPickup::TakeAmmoFromPlayer( CDAPlayer* pPlayer )
{
	int iGrenadesIndex = GetAmmoDef()->Index("grenades");

	bool bTookAmmo = false;

	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (i == iGrenadesIndex)
			continue;

		int iAmmoCount = pPlayer->GetAmmoCount(i);
		if (!iAmmoCount)
			continue;

		m_aiAmmoCounts[i] += iAmmoCount;

		pPlayer->RemoveAmmo(iAmmoCount, i);

		bTookAmmo = true;
	}

	return bTookAmmo;
}

bool CAmmoPickup::MyTouch( CBasePlayer *pPlayer )
{
	if (!pPlayer->IsAlive())
		return false;

	bool bPickup = false;

	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!m_aiAmmoCounts[i])
			continue;

		bool bHasWeaponUsingThisAmmo = false;
		for (int j = 0; j < pPlayer->WeaponCount(); j++)
		{
			CBaseCombatWeapon* pWeapon = pPlayer->GetWeapon(j);
			if (!pWeapon)
				continue;

			CWeaponDABase* pSDKWeapon = dynamic_cast<CWeaponDABase*>(pWeapon);
			if (!pSDKWeapon)
				continue;

			if (pSDKWeapon->GetSDKWpnData().iAmmoType == i)
			{
				bHasWeaponUsingThisAmmo = true;
				break;
			}
		}

		if (!bHasWeaponUsingThisAmmo)
			continue;

		if (pPlayer->GiveAmmo( m_aiAmmoCounts[i], i ))
			bPickup = true;
	}

	if (!bPickup)
		return false;

	UTIL_Remove(this);

	return true;
}

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Satchel Charge
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#pragma once

#include "items.h"

class CAmmoPickup : public CItem
{
	DECLARE_CLASS( CAmmoPickup, CItem );
	DECLARE_DATADESC();

public:
	CAmmoPickup();

public:
	void Precache( void );
	void Spawn( void );

	bool TakeAmmoFromPlayer( class CDAPlayer* pPlayer );

	bool MyTouch( CBasePlayer *pPlayer );

private:
	int m_aiAmmoCounts[MAX_AMMO_TYPES];
};

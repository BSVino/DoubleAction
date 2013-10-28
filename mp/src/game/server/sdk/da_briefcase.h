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

class CBriefcase : public CItem
{
	DECLARE_CLASS( CBriefcase, CItem );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CBriefcase();

public:
	void Precache( void );
	void Spawn( void );

	int UpdateTransmitState();

	bool MyTouch( CBasePlayer *pPlayer );
};

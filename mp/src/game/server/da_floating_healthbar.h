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


class CFloatingHealthbar : public CBaseEntity
{
	DECLARE_CLASS(CFloatingHealthbar, CBaseEntity);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CFloatingHealthbar();

public:
	void Precache(void);
	void Spawn(void);

	int UpdateTransmitState();

	void CaptureThink();

private:
	CNetworkVar(float, m_flCaptureRadius);
};

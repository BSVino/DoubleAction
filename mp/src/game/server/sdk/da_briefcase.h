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

	void AnimateThink();
	bool MyTouch( CBasePlayer *pPlayer );

	void Touch();
	void Dropped(class CSDKPlayer*);

	float GetLastTouchedTime() const { return m_flLastTouched; }

private:
	float m_flLastTouched;
};

class CBriefcaseCaptureZone : public CBaseEntity
{
	DECLARE_CLASS( CBriefcaseCaptureZone, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CBriefcaseCaptureZone();

public:
	void Precache( void );
	void Spawn( void );

	int UpdateTransmitState();

	void CaptureThink();

private:
	CNetworkVar( float, m_flCaptureRadius );
};

class CRatRaceWaypoint : public CBaseEntity
{
	DECLARE_CLASS( CRatRaceWaypoint, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CRatRaceWaypoint();

public:
	void Precache( void );
	void Spawn( void );

	int UpdateTransmitState();

	void WaypointThink();

	void SetWaypoint(int i) { m_iWaypoint = i; }
	int  GetWaypoint() { return m_iWaypoint; }

private:
	CNetworkVar( float, m_flRadius );
	CNetworkVar( float, m_iWaypoint );
};

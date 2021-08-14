//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "da_floating_healthbar.h"
#include "sdk_player.h"
#include "sdk_gamerules.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



BEGIN_DATADESC(CFloatingHealthbar)
END_DATADESC()

LINK_ENTITY_TO_CLASS(da_briefcase_capture, CFloatingHealthbar);

IMPLEMENT_SERVERCLASS_ST(CFloatingHealthbar, DT_FloatingHealthbar)
SendPropFloat(SENDINFO(m_flCaptureRadius)),
END_SEND_TABLE()

PRECACHE_REGISTER(da_briefcase_capture);

CFloatingHealthbar::CFloatingHealthbar()
{
	m_flCaptureRadius = 80;
}

void CFloatingHealthbar::Precache(void)
{
	CBaseEntity::PrecacheModel("da/capturezone.vmt");
}

void CFloatingHealthbar::Spawn(void)
{
	BaseClass::Spawn();

	SetThink(&CFloatingHealthbar::CaptureThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 20));
	UTIL_DropToFloor(this, MASK_SOLID_BRUSHONLY, this);
}

int CFloatingHealthbar::UpdateTransmitState()
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

void CFloatingHealthbar::CaptureThink()
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	float flCaptureRadiusSqr = m_flCaptureRadius * m_flCaptureRadius;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if (pPlayer->HasBriefcase() && (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flCaptureRadiusSqr)
		{
			DispatchParticleEffect("dinero_splode", GetAbsOrigin(), GetAbsAngles());

			SDKGameRules()->PlayerCapturedBriefcase(pPlayer);
			return;
		}
	}
}
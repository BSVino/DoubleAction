#include "cbase.h"

#include "dove.h"

#include "datacache/imdlcache.h"

#include "sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CDove )
END_DATADESC()

LINK_ENTITY_TO_CLASS( dove, CDove );
PRECACHE_REGISTER( dove );

void CDove::Precache()
{
	PrecacheModel("models/pigeon.mdl");
}

void CDove::Spawn()
{
	SetModel("models/pigeon.mdl");

	BaseClass::Spawn();

	SetThink(&CDove::FlyThink);
	SetNextThink(gpGlobals->curtime);

	m_flSpeed = 200;

	MDLCACHE_CRITICAL_SECTION();
	if (random->RandomInt(0, 1) == 0)
		SetSequence(LookupSequence("fly01"));
	else
		SetSequence(LookupSequence("soar"));
}

void CDove::FlyTo(const Vector& vecEnd)
{
	CSDKPlayer* pOwner = ToSDKPlayer(GetOwnerEntity());
	Assert(pOwner);
	if (pOwner)
	{
		m_flStartTime = pOwner->GetCurrentTime();
		m_flEndTime = m_flStartTime + 2.0f + random->RandomFloat(-0.5f, 0.5f);
		m_flChangeAnimTime = m_flStartTime + random->RandomFloat(0.1, 1);
	}

	m_vecStartPoint = GetAbsOrigin();
	m_vecEndPoint = vecEnd;
	m_vecDirection = (m_vecEndPoint - m_vecStartPoint).Normalized();

	QAngle angDirection;
	VectorAngles(m_vecDirection, angDirection);
	SetAbsAngles(angDirection);
}

void CDove::FlyThink()
{
	SetNextThink(gpGlobals->curtime);

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwnerEntity());
	if (pOwner)
		m_flSlowMoMultiplier = pOwner->GetSlowMoMultiplier();

	float flCurrentTime = gpGlobals->curtime;
	if (pOwner)
		flCurrentTime = pOwner->GetCurrentTime();

	Vector vecDirection = (m_vecEndPoint - GetAbsOrigin()).Normalized();

	float flFadeInTime = 0.1f;
	float flFadeOutTime = 0.2f;

	float flLerp = RemapValClamped(flCurrentTime, m_flStartTime, m_flEndTime, 0, 1);

	if (flLerp < flFadeInTime)
	{
		m_nRenderMode = kRenderTransAlpha;
		SetRenderColorA( (byte)RemapValClamped(flLerp, 0, flFadeInTime, 0, 255) );
	}
	else if (flLerp > flFadeOutTime)
	{
		m_nRenderMode = kRenderTransAlpha;
		SetRenderColorA( (byte)RemapValClamped(flLerp, flFadeOutTime, 1, 255, 0) );
	}
	else
	{
		m_nRenderMode = kRenderNormal;
		SetRenderColorA( 255 );
	}

	SetAbsOrigin(GetAbsOrigin() + m_vecDirection * (gpGlobals->frametime * m_flSlowMoMultiplier * m_flSpeed));

	if (m_flChangeAnimTime > 0 && flCurrentTime > m_flChangeAnimTime)
	{
		m_flChangeAnimTime = flCurrentTime + random->RandomFloat(0.5, 2);

		if (GetSequence() == LookupSequence("fly01"))
			SetSequence(LookupSequence("soar"));
		else
			SetSequence(LookupSequence("fly01"));
	}

	SetPlaybackRate(m_flSlowMoMultiplier);

	if (GetCycle() >= 1)
		SetCycle(0);

	StudioFrameAdvance();

	if (flCurrentTime > m_flEndTime)
		UTIL_Remove(this);
}

static ConVar da_debug_doves("da_debug_doves", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CDove::SpawnDoves(CSDKPlayer* pPlayer)
{
	bool bStill = false;
	Vector vecVelocity = pPlayer->GetAbsVelocity();

	if (vecVelocity.LengthSqr() < 1)
	{
		bStill = true;

		float flYaw = random->RandomFloat(0, 360);
		vecVelocity = Vector(cos(flYaw), sin(flYaw), 0);
	}

	vecVelocity.z = 0;

	Vector vecDirection = vecVelocity.Normalized();

	// Doves fly slightly up.
	vecDirection.z += 0.2f;
	vecDirection = vecDirection.Normalized();

	for (int i = 0; i < 10; i++)
	{
		Vector vecStart = pPlayer->GetAbsOrigin() + Vector(0, 0, random->RandomFloat(10, 60));

		// If diving or wall flipping, lower the doves so the player falls through them.
		if (pPlayer->m_Shared.IsDiving() || pPlayer->m_Shared.IsWallFlipping(true))
		{
			if (pPlayer->GetAbsVelocity().z < 0)
				vecStart -= Vector(0, 0, 50);
			else
				vecStart -= Vector(0, 0, 100);
		}

		if (bStill || pPlayer->m_Shared.IsWallFlipping(true))
			vecStart -= vecDirection * 100;
		else
			vecStart += vecDirection * 50;

		if (!pPlayer->IsInThirdPerson() || pPlayer->m_Shared.IsAimedIn())
		{
			// Move the doves forward a bit so they're more visible to the player.
			Vector vecForward;
			pPlayer->GetVectors(&vecForward, NULL, NULL);
			vecStart += vecForward * 50;
		}

		float flYaw = random->RandomFloat(0, 360);
		vecStart += Vector(cos(flYaw), sin(flYaw), 0) * random->RandomFloat(40, 100);

		Vector vecEnd = vecStart + vecDirection * 100;

		CDove* pDove = (CDove*)CreateEntityByName("dove");
		pDove->SetOwnerEntity( pPlayer );
		pDove->Spawn();

		pDove->SetAbsOrigin(vecStart);
		pDove->FlyTo(vecEnd);

		if (da_debug_doves.GetBool())
		{
			NDebugOverlay::Box( vecStart, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 200, 0, 127, 15 );
			NDebugOverlay::Line( vecStart, vecEnd, 0, 200, 0, false, 15 );
		}
	}
}

void CC_SpawnDoves_f(const CCommand &args)
{
	CSDKPlayer* pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	CDove::SpawnDoves(pPlayer);
}

static ConCommand doves("doves", CC_SpawnDoves_f, "Spawn doves", FCVAR_CHEAT);

#include "cbase.h"

#include "da_viewmodel.h"
#include "sdk_gamerules.h"
#include "weapon_akimbobase.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( da_viewmodel, CDAViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( DAViewModel, DT_DAViewModel )

BEGIN_NETWORK_TABLE( CDAViewModel, DT_DAViewModel )
END_NETWORK_TABLE()

CDAViewModel::CDAViewModel()
{
	m_vecPlayerVelocityLerp = Vector(0, 0, 0);
}

float CDAViewModel::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	float flSlow = 1;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());
	if (pOwner)
		flSlow *= pOwner->GetSlowMoMultiplier();

	return BaseClass::GetSequenceCycleRate(pStudioHdr, iSequence) * flSlow;
}

void CDAViewModel::DoMuzzleFlash()
{
#ifdef CLIENT_DLL
	int id;
	switch (GetDAWeapon()->GetWeaponType())
	{
	case WT_PISTOL:
	default:
		id = GetDAWeapon ()->GetWeaponID ();
		if (SDK_WEAPON_AKIMBO_BERETTA == id || SDK_WEAPON_AKIMBO_M1911 == id)
		{/*HACK: Alternate attachment for akimbos, where else to put this?*/
			if (((CAkimboBase *)GetDAWeapon ())->shootright)
				ParticleProp()->Create ("muzzleflash_pistol", PATTACH_POINT_FOLLOW, "2");
			else
				ParticleProp()->Create ("muzzleflash_pistol", PATTACH_POINT_FOLLOW, "1");
		}
		else ParticleProp()->Create( "muzzleflash_pistol", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_SMG:
		ParticleProp()->Create( "muzzleflash_smg", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_RIFLE:
		ParticleProp()->Create( "muzzleflash_rifle", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_SHOTGUN:
		ParticleProp()->Create( "muzzleflash_shotgun", PATTACH_POINT_FOLLOW, "1" );
		break;
	}
#endif
}

ConVar da_weaponlag( "da_weaponlag", "0.005", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon bob magnitude." );
ConVar da_weaponbob( "da_weaponbob", "0.7", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon bob magnitude." );
ConVar da_weapondrop( "da_weapondrop", "1", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon drop while running." );
ConVar da_weaponoffset( "da_weaponoffset", "0.5", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon offset, creates movement while looking around." );
ConVar da_weapontilt( "da_weapontilt", "16", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much does the weapon tilt when diving laterally?" );

ConVar da_weapon_grenadethrow_drop( "da_weapon_grenadethrow_drop", "-5", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much does the weapon drop when throwing a grenade?" );
ConVar da_weapon_grenadethrow_tilt( "da_weapon_grenadethrow_tilt", "90", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much does the weapon drop when throwing a grenade?" );

float RemapGainedValClamped( float flInput, float flGain, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
	float flNormalized = RemapValClamped(flInput, flInLo, flInHi, 0, 1);
	float flLerped = Gain(flNormalized, flGain);
	return RemapVal(flLerped, 0, 1, flOutLo, flOutHi);
}

void CDAViewModel::AddViewModelBob( CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles )
{
	CSDKPlayer* pOwner = ToSDKPlayer(owner);
	if (!pOwner)
		return;

	CWeaponSDKBase* pWeapon = GetDAWeapon();
	if (pWeapon && pWeapon->GetGrenadeThrowStart() > 0)
	{
		float flThrowStart = GetDAWeapon()->GetGrenadeThrowStart();
		float flHolsterTime = GetDAWeapon()->GetGrenadeThrowWeaponHolsterTime();
		float flDeployTime = GetDAWeapon()->GetGrenadeThrowWeaponDeployTime();
		float flThrowEnd = GetDAWeapon()->GetGrenadeThrowEnd();

		float flGain = 0.7f;
		if (pOwner->GetCurrentTime() < flHolsterTime)
		{
			eyePosition -= Vector(0, 0, 1) * RemapGainedValClamped( pOwner->GetCurrentTime(), flGain, flThrowStart, flHolsterTime, 0, da_weapon_grenadethrow_drop.GetFloat());
			eyeAngles.x += RemapGainedValClamped( pOwner->GetCurrentTime(), flGain, flThrowStart, flHolsterTime, 0, da_weapon_grenadethrow_tilt.GetFloat());
		}
		else if (pOwner->GetCurrentTime() > flDeployTime)
		{
			eyePosition -= Vector(0, 0, 1) * RemapGainedValClamped( pOwner->GetCurrentTime(), flGain, flDeployTime, flThrowEnd, da_weapon_grenadethrow_drop.GetFloat(), 0);
			eyeAngles.x += RemapGainedValClamped( pOwner->GetCurrentTime(), flGain, flDeployTime, flThrowEnd, da_weapon_grenadethrow_tilt.GetFloat(), 0);
		}
	}

	// Offset it a tad so that it moves while looking around.
	eyePosition.x += da_weaponoffset.GetFloat();

	Vector vecViewForward, vecViewRight, vecViewUp;
	AngleVectors(EyeAngles(), &vecViewForward, &vecViewRight, &vecViewUp);

	Vector vecViewDirection(vecViewForward.x, vecViewForward.y, 0);
	vecViewDirection.NormalizeInPlace();

	float flMaxVelocity = 100;
	Vector vecOwnerVelocity = pOwner->GetAbsVelocity();
	if (vecOwnerVelocity.LengthSqr() > flMaxVelocity*flMaxVelocity)
		vecOwnerVelocity = (vecOwnerVelocity / vecOwnerVelocity.Length()) * flMaxVelocity;

	m_vecPlayerVelocityLerp.x = Approach(vecOwnerVelocity.x, m_vecPlayerVelocityLerp.x, 1000*gpGlobals->frametime);
	m_vecPlayerVelocityLerp.y = Approach(vecOwnerVelocity.y, m_vecPlayerVelocityLerp.y, 1000*gpGlobals->frametime);
	m_vecPlayerVelocityLerp.z = Approach(vecOwnerVelocity.z, m_vecPlayerVelocityLerp.z, 1000*gpGlobals->frametime);

	Vector vecPlayerVelocityLerp = m_vecPlayerVelocityLerp;
	vecPlayerVelocityLerp.NormalizeInPlace();

	float flViewVelocityDot = fabs(vecPlayerVelocityLerp.Dot(vecViewRight));
	eyePosition += m_vecPlayerVelocityLerp * da_weaponlag.GetFloat() * flViewVelocityDot;

	if (pOwner->m_Shared.GetViewBobRamp() && pOwner->m_Shared.GetRunSpeed())
	{
		float flViewBobMagnitude = pOwner->m_Shared.GetViewBobRamp() * da_weaponbob.GetFloat();

		float flRunPeriod = M_PI * 3;
		float flRunUpBob = sin(pOwner->GetCurrentTime() * flRunPeriod * 2) * (flViewBobMagnitude / 2);
		float flRunRightBob = sin(pOwner->GetCurrentTime() * flRunPeriod) * flViewBobMagnitude;

		float flWalkPeriod = M_PI * 1.5f;
		float flWalkUpBob = sin(pOwner->GetCurrentTime() * flWalkPeriod * 2) * (flViewBobMagnitude / 2);
		float flWalkRightBob = sin(pOwner->GetCurrentTime() * flWalkPeriod) * flViewBobMagnitude;

		// 0 is walk, 1 is run.
		float flRunRamp = RemapValClamped(pOwner->m_Shared.GetViewBobRamp(), pOwner->m_Shared.GetAimInSpeed()/pOwner->m_Shared.GetRunSpeed(), 1.0f, 0.0f, 1.0f);

		float flRightBob = RemapValClamped(flRunRamp, 0, 1, flWalkRightBob, flRunRightBob);
		float flUpBob = RemapValClamped(flRunRamp, 0, 1, flWalkUpBob, flRunUpBob);

		eyePosition += vecViewRight * flRightBob + vecViewUp * (flUpBob - pOwner->m_Shared.GetViewBobRamp() * da_weapondrop.GetFloat());
	}

	if (pOwner->m_Shared.GetViewTilt())
	{
		Vector vecDiveRight = Vector(0, 0, 1).Cross(pOwner->m_Shared.GetDiveDirection());

		float flRightDot = vecViewDirection.Dot(vecDiveRight);
		float flUpDot = vecViewDirection.Dot(pOwner->m_Shared.GetDiveDirection());

		eyeAngles.z += flRightDot * pOwner->m_Shared.GetViewTilt() * da_weapontilt.GetFloat();;

		eyePosition += (vecViewUp * (flUpDot * 0.5f) + vecViewDirection * (flUpDot * 0.5f)) * pOwner->m_Shared.GetViewTilt();

		float flDiveBobMagnitude = 0.5f * pOwner->m_Shared.GetViewTilt();
		float flDiveBobPeriod = M_PI * 0.5f;
		float flDiveUpBob = sin(pOwner->GetCurrentTime() * flDiveBobPeriod * 2) * (flDiveBobMagnitude / 2);
		float flDiveRightBob = cos(pOwner->GetCurrentTime() * flDiveBobPeriod * 2) * (flDiveBobMagnitude / 2);

		eyePosition += vecViewRight * flDiveRightBob + vecViewUp * flDiveUpBob;
	}
}

#include "cbase.h"

#include "dab_viewmodel.h"
#include "sdk_gamerules.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( dab_viewmodel, CDABViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( DABViewModel, DT_DABViewModel )

BEGIN_NETWORK_TABLE( CDABViewModel, DT_DABViewModel )
END_NETWORK_TABLE()

float CDABViewModel::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	float flSlow = 1;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());
	if (pOwner)
		flSlow *= pOwner->GetSlowMoMultiplier();

	return BaseClass::GetSequenceCycleRate(pStudioHdr, iSequence) * m_flPlaybackRate * flSlow;
}

void CDABViewModel::DoMuzzleFlash()
{
#ifdef CLIENT_DLL
	switch (GetDAWeapon()->GetWeaponType())
	{
	case WT_PISTOL:
	default:
		ParticleProp()->Create( "muzzleflash_pistol", PATTACH_POINT_FOLLOW, "1" );
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

ConVar da_weaponbob( "da_weaponbob", "0.7", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon bob magnitude." );
ConVar da_weapondrop( "da_weapondrop", "1", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Weapon drop while running." );

void CDABViewModel::AddViewModelBob( CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles )
{
	CSDKPlayer* pOwner = ToSDKPlayer(owner);
	if (!pOwner)
		return;

	if (pOwner->m_Shared.GetViewBobRamp())
	{
		Vector vecRight, vecUp;
		AngleVectors(EyeAngles(), NULL, &vecRight, &vecUp);

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

		eyePosition += vecRight * flRightBob + vecUp * flUpBob - vecUp * pOwner->m_Shared.GetViewBobRamp() * da_weapondrop.GetFloat();
	}
}

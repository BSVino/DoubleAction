#include "cbase.h"

#include "da_viewmodel.h"
#include "sdk_gamerules.h"
#include "weapon_akimbobase.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#include "eventlist.h"
#include "effect_dispatch_data.h"
#else
#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( da_viewmodel, CDAViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( DAViewModel, DT_DAViewModel )

BEGIN_NETWORK_TABLE( CDAViewModel, DT_DAViewModel )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CDAViewModel *model = (CDAViewModel *)pStruct;
	if (pData->m_Value.m_Int != model->GetSequence())
	{
		MDLCACHE_CRITICAL_SECTION();

		model->SetSequence(pData->m_Value.m_Int);

		if (model->m_flResumeAnimTime)
			model->m_flAnimTime = model->m_flResumeAnimTime;
		else
			model->m_flAnimTime = gpGlobals->curtime;
		model->m_flResumeAnimTime = 0;

		model->SetCycle(model->m_flResumeCycle);
		model->m_flResumeCycle = 0;
	}
}
#endif

CDAViewModel::CDAViewModel()
{
	m_vecPlayerVelocityLerp = Vector(0, 0, 0);
	m_angLastPlayerEyeAngles = QAngle(0, 0, 0);

	m_bPaused = false;

#ifdef CLIENT_DLL
	m_flResumeCycle = 0;
	m_flResumeAnimTime = 0;
#endif
}

float CDAViewModel::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	float flSlow = 1;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());
	if (pOwner)
		flSlow *= pOwner->GetSlowMoMultiplier();

	return BaseClass::GetSequenceCycleRate(pStudioHdr, iSequence) * flSlow;
}

void CDAViewModel::PauseAnimation()
{
	m_bPaused = true;
	m_flPauseCycle = GetCycle();
	m_nPauseSequence = GetSequence();
	m_flPauseAnimTime = GetAnimTime();
}

void CDAViewModel::UnpauseAnimation(float flRewind)
{
	m_bPaused = false;
	SetSequence(m_nPauseSequence);
	SetCycle(m_flPauseCycle - flRewind * BaseClass::GetSequenceCycleRate(GetModelPtr(), m_nPauseSequence));
	SetAnimTime(m_flPauseAnimTime - flRewind);

#ifdef CLIENT_DLL
	// Remember this so that when the message comes in from the server
	// we can set it back to this.
	m_flResumeCycle = m_flPauseCycle;
	m_flResumeAnimTime = m_flPauseAnimTime;
#endif
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

#ifdef CLIENT_DLL
int CDAViewModel::DrawModel(int flags)
{
	int iReturn = BaseClass::DrawModel(flags);

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if (pLocalPlayer && pLocalPlayer->UseVRHUD())
	{
		Vector vecAmmo1, vecAmmo2;

		int iAmmo1 = LookupAttachment("ammo_1");
		int iAmmo2 = LookupAttachment("ammo_2");
		int iAmmoL1 = LookupAttachment("ammo_1_l");
		int iAmmoL2 = LookupAttachment("ammo_2_l");
		int iAmmoR1 = LookupAttachment("ammo_1_r");
		int iAmmoR2 = LookupAttachment("ammo_2_r");

		if (GetAttachment(iAmmo1, vecAmmo1) && GetAttachment(iAmmo2, vecAmmo2))
			CWeaponSDKBase::DrawVRBullets(vecAmmo1, vecAmmo2, GetActiveWeapon()->Clip1(), GetActiveWeapon()->GetMaxClip1(), true);

		if (GetAttachment(iAmmoR1, vecAmmo1) && GetAttachment(iAmmoR2, vecAmmo2))
		{
			CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(GetActiveWeapon());
			if (pWeapon)
				CWeaponSDKBase::DrawVRBullets(vecAmmo1, vecAmmo2, pWeapon->rightclip, pWeapon->GetMaxClip1()/2, true);
		}

		if (GetAttachment(iAmmoL1, vecAmmo1) && GetAttachment(iAmmoL2, vecAmmo2))
		{
			CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(GetActiveWeapon());
			if (pWeapon)
				CWeaponSDKBase::DrawVRBullets(vecAmmo1, vecAmmo2, pWeapon->leftclip, pWeapon->GetMaxClip1()/2, false);
		}
	}

	return iReturn;
}

void DispatchEffect( const char *pName, const CEffectData &data );

void CDAViewModel::FireObsoleteEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	Vector attachOrigin;
	QAngle attachAngles; 

	switch( event )
	{
	// Obsolete. Use the AE_CL_CREATE_PARTICLE_EFFECT event instead, which uses the artist driven particle system & editor.
	case AE_CLIENT_EFFECT_ATTACH:
		{
			int iAttachment = -1;
			int iParam = 0;
			char token[128];
			char effectFunc[128];

			const char *p = options;

			p = nexttoken(token, p, ' ');

			if( token ) 
			{
				Q_strncpy( effectFunc, token, sizeof(effectFunc) );
			}

			p = nexttoken(token, p, ' ');

			if( token )
			{
				iAttachment = atoi(token);
			}

			p = nexttoken(token, p, ' ');

			if( token )
			{
				iParam = atoi(token);
			}

			matrix3x4_t m;
			if ( GetAttachment( iAttachment, attachOrigin, attachAngles ) )
			{
				// Fill out the generic data
				CEffectData data;
				data.m_vOrigin = attachOrigin;
				data.m_vAngles = attachAngles;
				AngleVectors( attachAngles, &data.m_vNormal );
				data.m_hEntity = GetRefEHandle();
				data.m_nAttachmentIndex = iAttachment + 1;
				data.m_fFlags = iParam;

				DispatchEffect( effectFunc, data );
			}
		}
		break;
	}
}
#endif

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
	if (pWeapon && pWeapon->IsThrowingGrenade())
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

	// For mysterious reasons that I don't care to investigate, the eye angles
	// are sometimes slammed to (0, 0, 0) for a frame or two. If this should
	// happen, use the previous eye angles instead.
	QAngle angEye = EyeAngles();

	if (angEye.x == 0 && angEye.y == 0 && angEye.z == 0)
		angEye = m_angLastPlayerEyeAngles;
	else
		m_angLastPlayerEyeAngles = angEye;

	Vector vecViewForward, vecViewRight, vecViewUp;
	AngleVectors(angEye, &vecViewForward, &vecViewRight, &vecViewUp);

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

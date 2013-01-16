//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_sdkbase.h"
#include "ammodef.h"

#include "sdk_fx_shared.h"
#include "sdk_gamerules.h"

#if defined( CLIENT_DLL )

	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"
	#include "te_effect_dispatch.h"

#endif

// ----------------------------------------------------------------------------- //
// CWeaponSDKBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBase, DT_WeaponSDKBase )

BEGIN_NETWORK_TABLE( CWeaponSDKBase, DT_WeaponSDKBase )
#ifdef CLIENT_DLL
  	RecvPropFloat( RECVINFO( m_flDecreaseShotsFired ) ),
  	RecvPropFloat( RECVINFO( m_flAccuracyDecay ) ),
#else
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropFloat( SENDINFO( m_flDecreaseShotsFired ) ),
	SendPropFloat( SENDINFO( m_flAccuracyDecay ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSDKBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flAccuracyDecay, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sdk_base, CWeaponSDKBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponSDKBase )

		// New weapon Think and Touch Functions go here..

	END_DATADESC()

#endif

#ifdef CLIENT_DLL
bool CWeaponSDKBase::ShouldPredict()
{
       if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
               return true;

       return BaseClass::ShouldPredict();
}
#endif
// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponSDKBase::CWeaponSDKBase()
{
	SetPredictionEligible( true );

	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_flAccuracyDecay = 0;
}

const CSDKWeaponInfo &CWeaponSDKBase::GetSDKWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CSDKWeaponInfo *pSDKInfo;

	#ifdef _DEBUG
		pSDKInfo = dynamic_cast< const CSDKWeaponInfo* >( pWeaponInfo );
		Assert( pSDKInfo );
	#else
		pSDKInfo = static_cast< const CSDKWeaponInfo* >( pWeaponInfo );
	#endif

	return *pSDKInfo;
}

bool CWeaponSDKBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	
	return 0;
}

CSDKPlayer* CWeaponSDKBase::GetPlayerOwner() const
{
	return dynamic_cast< CSDKPlayer* >( GetOwner() );
}

#ifdef CLIENT_DLL
void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle	final = in + punch;

	//Clip each component
	for ( int i = 0; i < 3; i++ )
	{
		if ( final[i] > clip[i] )
		{
			final[i] = clip[i];
		}
		else if ( final[i] < -clip[i] )
		{
			final[i] = -clip[i];
		}

		//Return the result
		in[i] = final[i] - punch[i];
	}
}
#endif

ConVar dab_fulldecay( "dab_fulldecay", "0.5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The maximum accuracy decay." );
ConVar dab_coldaccuracymultiplier( "dab_coldaccuracymultiplier", "0.25", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The accuracy of a cold barrel as a multiplier of the original accuracy." );
ConVar dab_decayrate( "dab_decayrate", "2", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "A multiplier for the accuracy decay rate of weapons as they fire." );

//Tony; added as a default primary attack if it doesn't get overridden, ie: by CSDKWeaponMelee
void CWeaponSDKBase::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	//Tony; check firemodes -- 
	switch(GetFireMode())
	{
	case FM_SEMIAUTOMATIC:
		if (pPlayer->GetShotsFired() > 0)
			return;
		break;
		//Tony; added an accessor to determine the max burst on a per-weapon basis.
	case FM_BURST:
		if (pPlayer->GetShotsFired() > MaxBurstShots())
			return;
		break;
	}
#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif

	SendWeaponAnim( GetPrimaryAttackActivity() );

	if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
	{
		// Marksmen don't consume ammo while their skill is active.
	}
	else
	{
		// Make sure we don't fire more than the amount in the clip
		if ( UsesClipsForAmmo1() )
			m_iClip1 --;
		else
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType );
	}

	pPlayer->IncreaseShotsFired();

	float flSpread = GetWeaponSpread();

	if (pPlayer->m_Shared.IsAimedIn() && !WeaponSpreadFixed())
	{
		if (GetSDKWpnData().m_bAimInSpreadBonus)
			flSpread *= RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, 0.3f);
		else
			flSpread *= RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, 0.8f);
	}

	if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
		flSpread *= 0.5f;

	if (!WeaponSpreadFixed())
		flSpread *= RemapValClamped(m_flAccuracyDecay, 0, dab_fulldecay.GetFloat(), dab_coldaccuracymultiplier.GetFloat(), 1);

	QAngle angShoot = pPlayer->EyeAngles() + pPlayer->GetPunchAngle();

	if (pPlayer->IsInThirdPerson())
	{
		// First find where the camera should be.
		Vector vecCamera = pPlayer->GetThirdPersonCameraPosition();

		Vector vecShoot;
		AngleVectors(angShoot, &vecShoot);

		trace_t tr;
		UTIL_TraceLine( vecCamera, vecCamera + vecShoot * 99999, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr );

		Vector vecBulletDirection = tr.endpos - pPlayer->Weapon_ShootPosition();
		vecBulletDirection.NormalizeInPlace();

		VectorAngles(vecBulletDirection, angShoot);
	}

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		angShoot,
		GetWeaponID(),
		0, //Tony; fire mode - this is unused at the moment, left over from CSS when SDK* was created in the first place.
		CBaseEntity::GetPredictionRandomSeed() & 255,
		flSpread
		);

	//Add our view kick in
	AddViewKick();

	//Tony; update our weapon idle time
	SetWeaponIdleTime( GetCurrentTime() + SequenceDuration() );

	float flFireRate = GetFireRate();
	if (pPlayer->m_Shared.IsAimedIn() && HasAimInFireRateBonus())
	{
		// We lerp from .8 instead of 1 to be a bit more forgiving when the player first taps the aim button.
		float flFireRateMultiplier = RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 0.8f, 0.7f);

		flFireRate *= flFireRateMultiplier;

		CBaseViewModel* vm = pPlayer->GetViewModel( m_nViewModelIndex );

		if (vm)
			vm->SetPlaybackRate( 1/flFireRateMultiplier );
	}

	if (m_flAccuracyDecay < 0)
		m_flAccuracyDecay = 0;

	// Weapons that fire quickly should decay slower.
	m_flAccuracyDecay += (flFireRate * dab_decayrate.GetFloat());

	if (m_flAccuracyDecay > dab_fulldecay.GetFloat())
		m_flAccuracyDecay = dab_fulldecay.GetFloat();

	m_flNextPrimaryAttack = GetCurrentTime() + flFireRate;
	m_flNextSecondaryAttack = GetCurrentTime() + flFireRate;
}

void CWeaponSDKBase::SecondaryAttack()
{
	Swing(true);
}

#define MELEE_HULL_DIM		16

static const Vector g_meleeMins(-MELEE_HULL_DIM,-MELEE_HULL_DIM,-MELEE_HULL_DIM);
static const Vector g_meleeMaxs(MELEE_HULL_DIM,MELEE_HULL_DIM,MELEE_HULL_DIM);

void CWeaponSDKBase::Swing(bool bIsSecondary)
{
	trace_t traceHit;

	// Try a ray
	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return;

	if (pOwner->m_Shared.IsRolling() || pOwner->m_Shared.IsProne() || pOwner->m_Shared.IsGoingProne())
		return;

	if (bIsSecondary && pOwner->m_Shared.IsDiving())
		return;

	pOwner->Instructor_LessonLearned("brawl");

	pOwner->ReadyWeapon();

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	pOwner->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );

#ifndef CLIENT_DLL
	// Like bullets, melee traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetMeleeDamage( bIsSecondary ), DMG_CLUB );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin );
#endif

	if ( traceHit.fraction == 1.0 )
	{
		float meleeHullRadius = 1.732f * MELEE_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * meleeHullRadius / 2;

		UTIL_TraceHull( swingStart, swingEnd, g_meleeMins, g_meleeMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.6f )
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				ChooseIntersectionPointAndActivity( traceHit, g_meleeMins, g_meleeMaxs, pOwner );
			}
		}
	}

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange();
		
		// See if we happened to hit water
		ImpactWater( swingStart, testEnd );

		WeaponSound( MELEE_MISS );

		// Use less charge than if it's a hit
		if (pOwner->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
			pOwner->UseStyleCharge(5);
	}
	else
	{
		Hit( traceHit, bIsSecondary );

		if (pOwner->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
			pOwner->UseStyleCharge(10);
	}

	// Send the anim
	if (!SendWeaponAnim( ACT_VM_HITCENTER ))
		SendWeaponAnim( ACT_VM_DRAW );	// If the animation is missing, play the draw animation instead as a placeholder.

	// Cancel it quickly before attacking again so that it doesn't just restart the gesture.
	pOwner->DoAnimationEvent( PLAYERANIMEVENT_CANCEL );
	if (bIsSecondary)
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_SECONDARY );
	else
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	AddMeleeViewKick();

	float flFireRate;
	if (bIsSecondary)
		flFireRate = GetBrawlSecondaryFireRate();
	else
		flFireRate = GetBrawlFireRate();

	if (pOwner->IsStyleSkillActive() && pOwner->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
		flFireRate *= 0.7f;

	//Setup our next attack times
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetCurrentTime() + flFireRate;

	pOwner->FreezePlayer(0.6f, flFireRate*3/2);
}

Activity CWeaponSDKBase::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CSDKPlayer *pOwner )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}

void CWeaponSDKBase::Hit( trace_t &traceHit, bool bIsSecondary )
{
	CSDKPlayer *pPlayer = ToSDKPlayer( GetOwner() );
	
	//Do view kick
//	AddViewKick();

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

#ifndef CLIENT_DLL
		float flDamage = GetMeleeDamage( bIsSecondary );

		CTakeDamageInfo info( GetOwner(), GetOwner(), flDamage, DMG_CLUB );

		if( pPlayer && pHitEntity->IsNPC() )
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );
#endif
		WeaponSound( MELEE_HIT );
	}

	// Apply an impact effect
	ImpactEffect( traceHit );

#if 0
#ifndef CLIENT_DLL
	bool bWeaponDisarms = true;
	if (GetWeaponType() == WT_RIFLE)
		bWeaponDisarms = false;
	if (GetWeaponType() == WT_SHOTGUN)
		bWeaponDisarms = false;

	if (bIsSecondary && bWeaponDisarms)
	{
		CSDKPlayer* pVictim = ToSDKPlayer(pHitEntity);
		if (pVictim && pVictim->IsAlive())
			pVictim->Disarm();
	}
#endif
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSDKBase::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace( &traceHit, DMG_CLUB );
}

bool CWeaponSDKBase::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if ( UTIL_PointContents( start ) & (CONTENTS_WATER|CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if ( !(UTIL_PointContents( end ) & (CONTENTS_WATER|CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
#ifndef CLIENT_DLL
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
#endif
	}

	return true;
}

float CWeaponSDKBase::GetMeleeDamage( bool bIsSecondary ) const
{
	CSDKPlayer *pPlayer = ToSDKPlayer( GetOwner() );

	// The heavier the damage the more it hurts.
	float flDamage = RemapVal(GetSDKWpnData().iWeight, 0, 20, 10, 60);

	if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
		flDamage *= 2.0f;

	if (!pPlayer->GetGroundEntity())
		flDamage *= 1.2f;

	return flDamage;
}

float CWeaponSDKBase::GetBrawlFireRate()
{
	// This is overridden with melee and unused with brawl for firearms
	Assert(false);

	return 1;
}

float CWeaponSDKBase::GetBrawlSecondaryFireRate()
{
	// The heavier it is the longer it takes to swing.
	float flWeight = GetSDKWpnData().iWeight;

	float flTime = RemapVal(flWeight, 5, 20, 0.15f, 0.6f);

	return flTime;
}

void CWeaponSDKBase::AddViewKick()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if ( pPlayer )
	{
		// Update punch angles.
		QAngle angle = pPlayer->GetPunchAngle();

		angle.x -= SharedRandomInt( "PunchAngle", 4, 6 );

		float flPunchBonus = 1;
		float flRecoilBonus = 1;
		if (GetPlayerOwner()->m_Shared.IsAimedIn())
		{
			if (HasAimInRecoilBonus())
			{
				flPunchBonus = RemapValClamped(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 1, 1, 0.2f);
				flRecoilBonus = RemapValClamped(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 0.8f, 1, 0.2f);
			}
			else
			{
				flPunchBonus = RemapValClamped(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 1, 1, 0.8f);
				flRecoilBonus = RemapValClamped(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 1, 1, 0.6f);
			}
		}

		if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
		{
			flPunchBonus *= 0.5f;
			flRecoilBonus *= 0.5f;
		}

		pPlayer->SetPunchAngle( angle * GetViewPunchMultiplier() * flPunchBonus );
		pPlayer->m_Shared.SetRecoil(SharedRandomFloat("Recoil", 1, 1.1f) * GetRecoil() * flRecoilBonus);
	}
}

void CWeaponSDKBase::AddMeleeViewKick()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer )
		return;

	// Update punch angles.
	QAngle angle = pPlayer->GetPunchAngle();

	angle.x -= SharedRandomInt( "PunchAngle", 4, 6 );

	float flRecoilBonus = 1;

	pPlayer->SetPunchAngle( angle * 0.4f * flRecoilBonus );
}

float CWeaponSDKBase::GetWeaponSpread()
{
	return GetSDKWpnData().m_flSpread;
}

#ifdef CLIENT_DLL
void CWeaponSDKBase::CreateMove(float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles)
{
	BaseClass::CreateMove(flInputSampleTime, pCmd, vecOldViewAngles);

	if (!GetPlayerOwner())
		return;

	Vector vecRecoil = GetPlayerOwner()->m_Shared.GetRecoil(flInputSampleTime);
	pCmd->viewangles[PITCH] -= vecRecoil.y;
	pCmd->viewangles[YAW] += vecRecoil.x;
}
#endif

float CWeaponSDKBase::GetViewPunchMultiplier()
{
	return GetSDKWpnData().m_flViewPunchMultiplier;
}

float CWeaponSDKBase::GetRecoil()
{
	return GetSDKWpnData().m_flRecoil;
}

bool CWeaponSDKBase::HasAimInSpeedPenalty()
{
	return GetSDKWpnData().m_bAimInSpeedPenalty;
}

bool CWeaponSDKBase::HasAimInFireRateBonus()
{
	return GetSDKWpnData().m_bAimInFireRateBonus;
}

bool CWeaponSDKBase::HasAimInRecoilBonus()
{
	return GetSDKWpnData().m_bAimInRecoilBonus;
}

weapontype_t CWeaponSDKBase::GetWeaponType() const
{
	return GetSDKWpnData().m_eWeaponType;
}

weapontype_t CWeaponSDKBase::GetWeaponType(SDKWeaponID eWeapon)
{
	CSDKWeaponInfo *pFileInfo = CSDKWeaponInfo::GetWeaponInfo(eWeapon);

	if (!pFileInfo)
		return WT_NONE;

	return pFileInfo->m_eWeaponType;
}

ConVar dab_decaymultiplier( "dab_decaymultiplier", "0.7", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The multiplier for the recoil decay rate." );

//Tony; added so we can have base functionality without implementing it into every weapon.
void CWeaponSDKBase::ItemPostFrame( void )
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	//
	//Tony; totally override the baseclass
	//

	if ( UsesClipsForAmmo1() )
		CheckReload();

	// A multiplier of 1 means that for every second of firing the player needs to wait one second to get back to full accuracy.
	m_flAccuracyDecay -= (gpGlobals->frametime * dab_decaymultiplier.GetFloat() * pPlayer->GetSlowMoMultiplier());

	if (m_flAccuracyDecay < 0)
		m_flAccuracyDecay = 0;

	bool bFired = false;

	// Secondary attack has priority
	if ((pPlayer->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
	{
		if (UsesSecondaryAmmo() && pPlayer->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < GetCurrentTime())
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = GetCurrentTime() + 0.5;
			}
		}
		else if (pPlayer->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = GetCurrentTime() + 0.2;
		}
		else
		{
			bFired = true;
			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}
	
	if ( !bFired && (pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() && (( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			HandleFireOnEmpty();
			pPlayer->ReadyWeapon();
		}
		else if (pPlayer->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = GetCurrentTime() + 0.2;
		}
		else
		{
			PrimaryAttack();
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( pPlayer->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pPlayer->m_nButtons & IN_ATTACK) || (pPlayer->m_nButtons & IN_ATTACK2) || (pPlayer->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}

	// Tony; decrease shots fired count - tweak the time as necessary.
	if ( !( pPlayer->m_nButtons & IN_ATTACK ) )
	{
		//Tony; check firemodes -- If we're semi or burst, we will clear shots fired now that the player has let go of the button.
		switch(GetFireMode())
		{
		case FM_SEMIAUTOMATIC:
			if (pPlayer->GetShotsFired() > 0)
				pPlayer->ClearShotsFired();
			break;
			//Tony; TODO; add an accessor to determine the max burst on a per-weapon basis!!
			//DONE!
		case FM_BURST:
			if (pPlayer->GetShotsFired() > MaxBurstShots())
				pPlayer->ClearShotsFired();
			break;
		}

		m_bFireOnEmpty = false;
		if ( (pPlayer->GetShotsFired() > 0) && (m_flDecreaseShotsFired < GetCurrentTime())	)
		{
			m_flDecreaseShotsFired = GetCurrentTime() + 0.05495;
			pPlayer->DecreaseShotsFired();
		}
	}
}

extern bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer );

void CWeaponSDKBase::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
#ifdef GAME_DLL
	CSDKPlayer *pPlayer = ToSDKPlayer( pActivator );
	
	if ( !pPlayer )
		return;

	// Can I have this weapon type?
	if ( !pPlayer->IsAllowedToPickupWeapons() )
		return;

	if ( GetOwner() || !pPlayer->Weapon_CanUse( this ) || !g_pGameRules->CanHavePlayerItem( pPlayer, this ) )
		return;

	// Don't let the player touch the item unless unobstructed
	if ( !UTIL_ItemCanBeTouchedByPlayer( this, pPlayer ) )
		return;

	int iWeaponsWeight = 0;
	for (int i = 0; i < pPlayer->WeaponCount(); i++)
	{
		if (!pPlayer->GetWeapon(i))
			continue;

		iWeaponsWeight += static_cast<CWeaponSDKBase*>(pPlayer->GetWeapon(i))->GetWeight();
	}

	if (GetWeight() + iWeaponsWeight > MAX_LOADOUT_WEIGHT)
		return;

	// ----------------------------------------
	// If I already have it just take the ammo
	// ----------------------------------------
	if (pPlayer->Weapon_OwnsThisType( GetClassname(), GetSubType())) 
	{
		if ( pPlayer->Weapon_EquipAmmoOnly( this ) )
		{
			// Only remove me if I have no ammo left
			if ( HasPrimaryAmmo() )
				return;

			UTIL_Remove( this );
			OnPickedUp( pPlayer );
			return;
		}
		else
			return;
	}
	// -------------------------
	// Otherwise take the weapon
	// -------------------------
	else 
	{
		CheckRespawn();

		AddSolidFlags( FSOLID_NOT_SOLID );
		AddEffects( EF_NODRAW );

		pPlayer->Weapon_Equip( this );
		if ( pPlayer->IsInAVehicle() )
		{
			Holster(NULL);
		}
		else
		{
			// If it uses clips, load it full. (this is the first time you've picked up this type of weapon)
			if ( UsesClipsForAmmo1() )
			{
				m_iClip1 = GetMaxClip1();
			}

			pPlayer->Weapon_Switch( this );
		}

		OnPickedUp( pPlayer );
	}
#endif
}

void CWeaponSDKBase::WeaponIdle( void )
{
	//Idle again if we've finished
	if ( HasWeaponIdleTimeElapsed() )
	{
		SendWeaponAnim( GetIdleActivity() );
		SetWeaponIdleTime( GetCurrentTime() + SequenceDuration() );
	}
}

bool CWeaponSDKBase::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), GetReloadActivity() );
	if ( fRet )
	{
		// Player gets a reload speed boost with Adrenaline.
		if (ToSDKPlayer(GetOwner()) && ToSDKPlayer(GetOwner())->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
			ToSDKPlayer(GetOwner())->UseStyleCharge(5);

		CSDKPlayer* pSDKOwner = ToSDKPlayer(GetOwner());

		if (pSDKOwner)
			pSDKOwner->ReadyWeapon();

		float flSpeedMultiplier = GetSDKWpnData().m_flReloadTimeMultiplier;

		if (GetPlayerOwner()->IsStyleSkillActive() && GetPlayerOwner()->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
			flSpeedMultiplier *= 0.6f;

		float flSequenceEndTime = GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;

		if (pSDKOwner)
		{
			CBaseViewModel *vm = pSDKOwner->GetViewModel( m_nViewModelIndex );

			if (vm)
				vm->SetPlaybackRate( 1/flSpeedMultiplier );

			pSDKOwner->Instructor_LessonLearned("reload");
		}

		MDLCACHE_CRITICAL_SECTION();
		if (GetPlayerOwner())
			GetPlayerOwner()->SetNextAttack( flSequenceEndTime );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

		SendReloadEvents();

		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
		
		WeaponSound( RELOAD );
		if (GetPlayerOwner()) 
			GetPlayerOwner()->ClearShotsFired();
	}

	return fRet;
}

void CWeaponSDKBase::FinishReload()
{
	BaseClass::FinishReload();

	if (GetPlayerOwner())
		GetPlayerOwner()->ReadyWeapon();
}

void CWeaponSDKBase::SendReloadEvents()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#ifdef GAME_DLL
	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );

	UserMessageBegin( filter, "ReloadEffect" );
	WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();
#endif
	// Make the player play his reload animation.
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

void CWeaponSDKBase::CheckReload()
{
	if ( m_bReloadsSingly )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( !pOwner )
			return;

		if ((m_bInReload) && (m_flNextPrimaryAttack <= GetCurrentTime()))
		{
			if ( pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0 )
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <=0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				// Add them to the clip
				m_iClip1 += 1;
				pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				m_flNextPrimaryAttack	= GetCurrentTime();
				m_flNextSecondaryAttack = GetCurrentTime();
				return;
			}
		}
	}
	else
	{
		if ( (m_bInReload) && (m_flNextPrimaryAttack <= GetCurrentTime()))
		{
			FinishReload();
			m_flNextPrimaryAttack	= GetCurrentTime();
			m_flNextSecondaryAttack = GetCurrentTime();
			m_bInReload = false;
		}
	}
}

bool CWeaponSDKBase::ReloadOrSwitchWeapons( void )
{
	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < GetCurrentTime() && m_flNextSecondaryAttack < GetCurrentTime() )
	{
		// This block used to have an auto switch to a non-empty weapon, but no longer.
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 GetCurrentTime() > m_flNextPrimaryAttack + 1.5f &&
			 GetCurrentTime() > m_flNextSecondaryAttack + 1.5f )
		{
			// if we're successfully reloading, we're done
			if ( Reload() )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponSDKBase::Deploy( )
{
	MDLCACHE_CRITICAL_SECTION();

	//Tony; on deploy clear shots fired.
	if (GetPlayerOwner())
	{
		GetPlayerOwner()->ClearShotsFired();

		if (GetPlayerOwner()->GetCurrentTime() < GetPlayerOwner()->m_flDisarmRedraw)
			return false;
	}

	bool bDeploy = DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDeployActivity(), (char*)GetAnimPrefix() );

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());

	float flSpeedMultiplier = GetSDKWpnData().m_flDrawTimeMultiplier;

	if (pOwner && pOwner->IsStyleSkillActive() && pOwner->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
		flSpeedMultiplier *= 0.6f;

	m_flNextPrimaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;
	m_flNextSecondaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;

	if (pOwner)
	{
		CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );

		if (vm)
			vm->SetPlaybackRate( 1/flSpeedMultiplier );

		pOwner->SetNextAttack( GetCurrentTime() + SequenceDuration() * flSpeedMultiplier );

		pOwner->m_Shared.SetAimIn(0.0f);
	}

	if (bDeploy && pOwner)
		pOwner->ReadyWeapon();

	return bDeploy;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//Tony; use the same name as the base context one.
#define SDK_HIDEWEAPON_THINK_CONTEXT			"BaseCombatWeapon_HideThink"
bool CWeaponSDKBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	MDLCACHE_CRITICAL_SECTION();

	// cancel any reload in progress.
	m_bInReload = false; 

	// kill any think functions
	SetThink(NULL);

	// Send holster animation
	SendWeaponAnim( ACT_VM_HOLSTER );

	// Some weapon's don't have holster anims yet, so detect that
	float flSequenceDuration = 0;
	if ( GetActivity() == ACT_VM_HOLSTER )
	{
		flSequenceDuration = SequenceDuration();
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner)
	{
		pOwner->SetNextAttack( GetCurrentTime() + flSequenceDuration );
	}

	// If we don't have a holster anim, hide immediately to avoid timing issues
	if ( !flSequenceDuration )
	{
		SetWeaponVisible( false );
	}
	else
	{
		// Hide the weapon when the holster animation's finished
		SetContextThink( &CBaseCombatWeapon::HideThink, GetCurrentTime() + flSequenceDuration, SDK_HIDEWEAPON_THINK_CONTEXT );
	}

	return true;
}

bool CWeaponSDKBase::HasWeaponIdleTimeElapsed( void )
{
	if ( GetCurrentTime() > m_flTimeWeaponIdle )
		return true;

	return false;
}

float CWeaponSDKBase::GetCurrentTime() const
{
	if (!GetOwner())
		return gpGlobals->curtime;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());

	if (!pOwner)
		return gpGlobals->curtime;

	return pOwner->GetCurrentTime();
}

#ifdef GAME_DLL
void CWeaponSDKBase::SetDieThink( bool bDie )
{
	if( bDie )
		SetContextThink( &CWeaponSDKBase::Die, gpGlobals->curtime + 45.0f, "DieContext" );
	else
		SetContextThink( NULL, gpGlobals->curtime, "DieContext" );
}
void CWeaponSDKBase::Die( void )
{
	UTIL_Remove( this );
}
#endif

bool CWeaponSDKBase::CanBeSelected()
{
	if ( !VisibleInWeaponSelection() )
		return false;

	return true;
}

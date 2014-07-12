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
#include "weapon_akimbobase.h"
#include "datacache/imdlcache.h"

#include "sdk_fx_shared.h"
#include "sdk_gamerules.h"
#include "da_viewmodel.h"

#if defined( CLIENT_DLL )

	#include "c_sdk_player.h"
	#include "prediction.h"
	#include "hud/sdk_hud_ammo.h"
	#include "view.h"
	#include "sourcevr/isourcevirtualreality.h"
	#include "weapon_selection.h"

#else

	#include "sdk_player.h"
	#include "te_effect_dispatch.h"
	#include "weapon_grenade.h"
	#include "ilagcompensationmanager.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ----------------------------------------------------------------------------- //
// CWeaponSDKBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBase, DT_WeaponSDKBase )

BEGIN_NETWORK_TABLE( CWeaponSDKBase, DT_WeaponSDKBase )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_flReloadEndTime ) ),
  	RecvPropFloat( RECVINFO( m_flDecreaseShotsFired ) ),
  	RecvPropFloat( RECVINFO( m_flAccuracyDecay ) ),
  	RecvPropFloat( RECVINFO( m_flSwingTime ) ),
  	RecvPropFloat( RECVINFO( m_flUnpauseFromSwingTime ) ),
  	RecvPropFloat( RECVINFO( m_flNextBrawlTime ) ),
	RecvPropFloat( RECVINFO( m_flCycleTime ) ),
	RecvPropFloat( RECVINFO( m_flViewPunchMultiplier ) ),
	RecvPropFloat( RECVINFO( m_flRecoil ) ),
	RecvPropFloat( RECVINFO( m_flSpread ) ),
  	RecvPropBool( RECVINFO( m_bSwingSecondary ) ),

	RecvPropInt(RECVINFO(leftclip)),
	RecvPropInt(RECVINFO(rightclip)),
	RecvPropBool(RECVINFO(shootright)),

	RecvPropFloat( RECVINFO( m_flGrenadeThrowStart ) ),
#else
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropTime( SENDINFO( m_flReloadEndTime ) ),
	SendPropFloat( SENDINFO( m_flDecreaseShotsFired ) ),
	SendPropFloat( SENDINFO( m_flAccuracyDecay ) ),
	SendPropFloat( SENDINFO( m_flCycleTime ) ),
	SendPropFloat( SENDINFO( m_flViewPunchMultiplier ) ),
	SendPropFloat( SENDINFO( m_flRecoil ) ),
	SendPropFloat( SENDINFO( m_flSpread ) ),
	SendPropFloat( SENDINFO( m_flSwingTime ) ),
	SendPropFloat( SENDINFO( m_flUnpauseFromSwingTime ) ),
	SendPropFloat( SENDINFO( m_flNextBrawlTime ) ),
	SendPropBool( SENDINFO( m_bSwingSecondary ) ),

	SendPropInt(SENDINFO(leftclip)),
	SendPropInt(SENDINFO(rightclip)),
	SendPropBool(SENDINFO(shootright)),

	SendPropFloat( SENDINFO( m_flGrenadeThrowStart ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSDKBase )
	DEFINE_PRED_FIELD_TOL( m_flReloadEndTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	

	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flAccuracyDecay, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flSwingTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flUnpauseFromSwingTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flNextBrawlTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bSwingSecondary, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD(leftclip, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),			
	DEFINE_PRED_FIELD(rightclip, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(shootright, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),	
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sdk_base, CWeaponSDKBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponSDKBase )

		// New weapon Think and Touch Functions go here..
		DEFINE_FIELD(leftclip, FIELD_INTEGER),
		DEFINE_FIELD(rightclip, FIELD_INTEGER),
		DEFINE_FIELD(shootright, FIELD_BOOLEAN),
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
	m_flSwingTime = 0;
	m_flNextBrawlTime = 0;
	m_flUnpauseFromSwingTime = 0;

#ifdef CLIENT_DLL
	m_flArrowGoalSize = 0;
	m_flArrowCurSize = 0;
	m_flArrowSpinOffset = RandomFloat(0, 10);

	m_flMarksmanGold = 0;
#endif

	m_flGrenadeThrowStart = -1;
}

void CWeaponSDKBase::Precache()
{
	BaseClass::Precache();

#ifdef GAME_DLL
	// server must enforce these values
	m_flCycleTime = GetSDKWpnData().m_flCycleTime;
	m_flViewPunchMultiplier = GetSDKWpnData().m_flViewPunchMultiplier;
	m_flRecoil = GetSDKWpnData().m_flRecoil;
	m_flSpread = GetSDKWpnData().m_flSpread;
#endif

	CBaseEntity::PrecacheModel( "particle/weaponarrow.vmt" );
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

bool CWeaponSDKBase::IsAkimbo() const
{
	return !FStrEq(GetSDKWpnData().m_szSingle, "");
}

bool CWeaponSDKBase::HasAkimbo() const
{
	return !FStrEq(GetSDKWpnData().m_szAkimbo, "");
}

bool CWeaponSDKBase::IsGrenade() const
{
	return GetSDKWpnData().m_eWeaponType == WT_GRENADE;
}

const char* CWeaponSDKBase::GetViewModel( int viewmodelindex ) const
{
	if (IsThrowingGrenade() && GetCurrentTime() > GetGrenadeThrowWeaponHolsterTime() && GetCurrentTime() < GetGrenadeThrowWeaponDeployTime())
		return CSDKWeaponInfo::GetWeaponInfo(SDK_WEAPON_GRENADE)->szViewModel;

	return BaseClass::GetViewModel(viewmodelindex);
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

ConVar da_fulldecay( "da_fulldecay", "0.5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The maximum accuracy decay." );
ConVar da_coldaccuracymultiplier( "da_coldaccuracymultiplier", "0.25", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The accuracy of a cold barrel as a multiplier of the original accuracy." );
ConVar da_decayrate( "da_decayrate", "2", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "A multiplier for the accuracy decay rate of weapons as they fire." );

//Tony; added as a default primary attack if it doesn't get overridden, ie: by CSDKWeaponMelee
void CWeaponSDKBase::FinishAttack (CSDKPlayer *pPlayer)
{
#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif
	pPlayer->IncreaseShotsFired();

	float flSpread = GetWeaponSpread();

	bool bGetAimInBonus = false;

	bGetAimInBonus = pPlayer->m_Shared.IsAimedIn() || pPlayer->m_Shared.IsSuperFalling();

	if (WeaponSpreadFixed())
		bGetAimInBonus = false;

	if (bGetAimInBonus)
	{
		float flAimIn = pPlayer->m_Shared.GetAimIn();

		if (pPlayer->m_Shared.IsSuperFalling())
			flAimIn = 1;

		if (GetSDKWpnData().m_bAimInSpreadBonus)
			flSpread *= RemapVal(flAimIn, 0, 1, 1, 0.3f);
		else
			flSpread *= RemapVal(flAimIn, 0, 1, 1, 0.8f);
	}

	flSpread = pPlayer->m_Shared.ModifySkillValue(flSpread, -0.25f, SKILL_MARKSMAN);

	if (!WeaponSpreadFixed())
		flSpread *= RemapValClamped(m_flAccuracyDecay, 0, da_fulldecay.GetFloat(), da_coldaccuracymultiplier.GetFloat(), 1);

	QAngle angShoot = pPlayer->EyeAngles() + pPlayer->GetPunchAngle();

	if (pPlayer->IsInThirdPerson())
	{
		// First find where the camera should be.
		Vector vecCamera = pPlayer->GetThirdPersonCameraPosition();

		Vector vecShoot;
		AngleVectors(angShoot, &vecShoot);

		int iMask = MASK_VISIBLE|CONTENTS_MONSTER|CONTENTS_DEBRIS|CONTENTS_HITBOX;

		// There's nothing complicated like glass if you're superfalling, so use a more reliable mask.
		if (pPlayer->m_Shared.IsSuperFalling())
			iMask = MASK_SOLID;

		trace_t tr;
		UTIL_TraceLine( vecCamera, vecCamera + vecShoot * 99999, iMask, pPlayer, COLLISION_GROUP_NONE, &tr );

		// If the player backs up against something, this will prevent their bullets from hitting the object behind.
		if (tr.fraction == 0.0)
			UTIL_TraceLine( vecCamera + vecShoot * 10, vecCamera + vecShoot * 99999, iMask, pPlayer, COLLISION_GROUP_NONE, &tr );

		Vector vecBulletDirection = (tr.endpos - pPlayer->Weapon_ShootPosition()).Normalized();

		VectorAngles(vecBulletDirection, angShoot);
	}

	FX_FireBullets(
		pPlayer->entindex(),
		GetShootPosition(pPlayer),
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
	m_flAccuracyDecay += (flFireRate * da_decayrate.GetFloat());

	if (m_flAccuracyDecay > da_fulldecay.GetFloat())
		m_flAccuracyDecay = da_fulldecay.GetFloat();

	m_flNextPrimaryAttack = GetCurrentTime() + flFireRate;
	m_flNextSecondaryAttack = GetCurrentTime() + min(flFireRate, 0.2f);

#ifdef CLIENT_DLL
	if (!prediction->InPrediction() || prediction->IsFirstTimePredicted())
	{
		CHudElement* pElement = gHUD.FindElement("CHudAmmo");
		if (pElement)
		{
			CHudAmmo* pHudAmmo = dynamic_cast<CHudAmmo*>(pElement);
			if (pHudAmmo)
			{
				if (dynamic_cast<CAkimboBase*>(this))
					pHudAmmo->ShotFired(this, true, shootright);
				else
					pHudAmmo->ShotFired(this, false, true);
			}
		}
	}
#endif
}

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

	if (m_iClip1 == 1)
	{
		if (!SendWeaponAnim( ACT_DA_VM_FIRELAST ))
			SendWeaponAnim( GetPrimaryAttackActivity() );
	}
	else
		SendWeaponAnim( GetPrimaryAttackActivity() );

	/*if (pPlayer->IsStyleSkillActive(SKILL_MARKSMAN))
	{
		// Marksmen don't consume ammo while their skill is active.
	}
	else*/
	{
		// Make sure we don't fire more than the amount in the clip
		if ( UsesClipsForAmmo1() )
			m_iClip1 --;
		else
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType );
	}

	/*Chopped this in half here for akimbos*/
	FinishAttack (pPlayer);
}

void CWeaponSDKBase::SecondaryAttack()
{
	StartSwing(true, true);
}

#define MELEE_HULL_DIM		16

static const Vector g_meleeMins(-MELEE_HULL_DIM,-MELEE_HULL_DIM,-MELEE_HULL_DIM);
static const Vector g_meleeMaxs(MELEE_HULL_DIM,MELEE_HULL_DIM,MELEE_HULL_DIM);

void CWeaponSDKBase::StartSwing(bool bIsSecondary, bool bIsStockAttack)
{
	// Try a ray
	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return;

	if (!bIsStockAttack && bIsSecondary && pOwner->m_Shared.IsDiving())
		return;

	if (pOwner->GetCurrentTime() < m_flNextBrawlTime)
		return;

	bool bNeedsUnpause = false;
	if (m_bInReload)
	{
		CDAViewModel* vm = dynamic_cast<CDAViewModel*>(pOwner->GetViewModel( m_nViewModelIndex ));
		if (vm)
		{
			vm->PauseAnimation();
			bNeedsUnpause = true;
		}
	}

	pOwner->Instructor_LessonLearned("brawl");

	pOwner->ReadyWeapon();

	// Send the anim
	int iAnim = ACT_DA_VM_BRAWL;
	if (m_iClip1 == 0)
		iAnim = ACT_DA_VM_BRAWL_EMPTY;
	if (!bIsSecondary)
		iAnim = ACT_VM_PRIMARYATTACK;
	if (!SendWeaponAnim( iAnim ))
		SendWeaponAnim( ACT_VM_DRAW ); // If the animation is missing, play the draw animation instead as a placeholder.

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

	flFireRate = pOwner->m_Shared.ModifySkillValue(flFireRate, -0.2f, SKILL_BOUNCER);
	float flTimeToSwing = flFireRate * 0.3f;

	if (m_bInReload)
	{
		// Brawl interrupts the reload and tacks the extra time onto it.
		m_flReloadEndTime += flTimeToSwing;
		m_flNextPrimaryAttack = m_flReloadEndTime;

		if (bNeedsUnpause)
			m_flUnpauseFromSwingTime = GetCurrentTime() + flFireRate * 0.7f;
	}

	//Setup our next attack times
	m_flNextSecondaryAttack = GetCurrentTime() + flFireRate;

	m_flSwingTime = GetCurrentTime() + flTimeToSwing;
	m_flNextBrawlTime = GetCurrentTime() + flFireRate;
	m_bSwingSecondary = bIsSecondary;

	pOwner->FreezePlayer(0.6f, flFireRate*3/2);
}

void CWeaponSDKBase::Swing()
{
	// Try a ray
	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return;

	m_flSwingTime = 0;

	trace_t traceHit;

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	pOwner->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetMeleeRange();

#ifndef CLIENT_DLL
	lagcompensation->StartLagCompensation( pOwner, pOwner->GetCurrentCommand() );
#endif

	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );

#ifndef CLIENT_DLL
	lagcompensation->FinishLagCompensation( pOwner );

	// Like bullets, melee traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetMeleeDamage( m_bSwingSecondary, ToSDKPlayer(traceHit.m_pEnt) ), DMG_CLUB );
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

	if (m_bSwingSecondary)
		pOwner->UseStyleCharge(SKILL_BOUNCER, 5);
	else
		pOwner->UseStyleCharge(SKILL_BOUNCER, 2.5f);

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
	}
	else
	{
		Hit( traceHit, m_bSwingSecondary );
	}
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
		float flDamage = GetMeleeDamage( bIsSecondary, ToSDKPlayer(pHitEntity) );

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
		CSoundParameters params;

		if (traceHit.m_pEnt && traceHit.m_pEnt->IsPlayer() && GetParametersForSound( "Weapon_Brawl.PunchHit", params, NULL ) )
		{
			CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
			filter.MakeReliable();

			CDisablePredictionFiltering disabler;

			EmitSound( filter, traceHit.m_pEnt->entindex(), params );
		}
		else
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

float CWeaponSDKBase::GetMeleeDamage( bool bIsSecondary, CSDKPlayer* pVictim ) const
{
	CSDKPlayer *pPlayer = ToSDKPlayer( GetOwner() );

	// The heavier the damage the more it hurts.
	float flDamage = RemapVal(GetSDKWpnData().iWeight, 7, 20, 45, 80);

	bool bIsStockAttack = pPlayer && pPlayer->GetActiveSDKWeapon() && !pPlayer->GetActiveSDKWeapon()->IsMeleeWeapon();

	if (pVictim)
	{
		// If the victim is low on health then finish them off.
		if (pVictim->GetHealth() <= 55)
			return pVictim->GetHealth() * 1.2f;

		if (pPlayer->IsStyleSkillActive(SKILL_BOUNCER))
		{
			if (bIsSecondary && !bIsStockAttack)
				return pVictim->GetHealth() * 1.5f;
			else
				flDamage += 20;
		}

		Vector vecForward;
		AngleVectors(pVictim->EyeAngles(), &vecForward, NULL, NULL);

		// Attacks to the rear do double damage.
		if ((pPlayer->GetAbsOrigin() - pVictim->GetAbsOrigin()).Normalized().Dot(vecForward) < -0.3f)
			flDamage *= 2;
	}

	// DIVEPUNCH!
	if (pPlayer->m_Shared.IsDiving() || pPlayer->m_Shared.IsRolling() || pPlayer->m_Shared.IsSliding() || pPlayer->m_Shared.IsWallFlipping(true))
		flDamage *= 1.8f;
	else if (!pPlayer->GetGroundEntity())
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

	float flTime = RemapValClamped(flWeight, 5, 20, 0.25f, 0.6f);

	return flTime;
}

ConVar da_grenade_throw_time( "da_grenade_throw_time", "1.0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long does it take to throw a grenade?" );

void CWeaponSDKBase::StartGrenadeToss()
{
	m_flGrenadeThrowStart = GetCurrentTime();

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());

	if (m_bInReload && pOwner)
	{
		CDAViewModel* vm = dynamic_cast<CDAViewModel*>(pOwner->GetViewModel( m_nViewModelIndex ));
		if (vm)
			vm->PauseAnimation();
	}

	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( GetCurrentTime() + da_grenade_throw_time.GetFloat() );

	if (m_bInReload)
	{
		m_flReloadEndTime += da_grenade_throw_time.GetFloat() * 0.7f;
		m_flNextPrimaryAttack = m_flReloadEndTime;
	}
	else 
		m_flNextPrimaryAttack = GetCurrentTime() + da_grenade_throw_time.GetFloat();

	m_flNextSecondaryAttack	= GetCurrentTime() + da_grenade_throw_time.GetFloat();

	m_bGrenadeThrown = false;

	pOwner->DoAnimationEvent( PLAYERANIMEVENT_GRENADE1_THROW );
}

ConVar da_grenade_weaponlerp_time( "da_grenade_weaponlerp_time", "0.25", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long does it take for the weapon to appear and disappear while throwing a grenade?" );

bool CWeaponSDKBase::MaintainGrenadeToss()
{
	if (GetCurrentTime() >= GetGrenadeThrowEnd())
	{
		m_flGrenadeThrowStart = -1;
		return false;
	}

	CSDKPlayer* pPlayer = ToSDKPlayer(GetOwner());
	if (!pPlayer)
		return false;

	SetViewModel();
	SetModel(GetViewModel());

	if (pPlayer && GetCurrentTime() > GetGrenadeThrowWeaponDeployTime())
	{
		CDAViewModel* vm = dynamic_cast<CDAViewModel*>(pPlayer->GetViewModel( m_nViewModelIndex ));
		if (vm && vm->IsAnimationPaused())
			// Technically unpausing it here instead of after the throw means we'll lose a tad of it,
			// so we rewind it a bit.
			vm->UnpauseAnimation(da_grenade_weaponlerp_time.GetFloat());
	}

	if (!m_bGrenadeThrown && GetCurrentTime() > GetGrenadeThrowWeaponHolsterTime())
	{
		m_bGrenadeThrown = true;

		pPlayer->Instructor_LessonLearned("grenade");

		SendWeaponAnim( ACT_VM_THROW );

#ifdef GAME_DLL
		Vector vecSrc, vecThrow;
		QAngle angThrow;
		GetGrenadeThrowVectors(vecSrc, vecThrow, angThrow);

		CGrenadeProjectile::Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, this, GRENADE_TIMER );

		if( pPlayer )
			pPlayer->RemoveAmmo( 1, GetAmmoDef()->Index("grenades") );
#endif
	}

	return true;
}

float CWeaponSDKBase::GetGrenadeThrowWeaponHolsterTime() const
{
	Assert(IsThrowingGrenade());
	return m_flGrenadeThrowStart + da_grenade_weaponlerp_time.GetFloat();
}

float CWeaponSDKBase::GetGrenadeThrowWeaponDeployTime() const
{
	Assert(IsThrowingGrenade());
	return m_flGrenadeThrowStart + da_grenade_throw_time.GetFloat() - da_grenade_weaponlerp_time.GetFloat();
}

float CWeaponSDKBase::GetGrenadeThrowEnd() const
{
	Assert(IsThrowingGrenade());
	return m_flGrenadeThrowStart + da_grenade_throw_time.GetFloat();
}

void CWeaponSDKBase::GetGrenadeThrowVectors(Vector& vecSrc, Vector& vecThrow, QAngle& angThrow)
{
	CSDKPlayer* pPlayer = ToSDKPlayer(GetOwner());
	Assert(pPlayer);
	if (!pPlayer)
		return;

	angThrow = pPlayer->LocalEyeAngles();

	Vector vForward, vRight, vUp;

	if (angThrow.x < 90 )
		angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
	else
	{
		angThrow.x = 360.0f - angThrow.x;
		angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
	}

	float flVel = (90 - angThrow.x) * 8;

	if (flVel > 850)
		flVel = 850;

	AngleVectors( angThrow, &vForward, &vRight, &vUp );

	vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

	vecSrc += vForward * 16;

	vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();
}

void CWeaponSDKBase::AddViewKick()
{
#ifdef CLIENT_DLL
	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		return;
#endif
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if ( pPlayer )
	{
		// Update punch angles.
		QAngle angle = pPlayer->GetPunchAngle();

		angle.x -= SharedRandomInt( "PunchAngle", 2, 3 );

		float flPunchBonus = 1;
		float flRecoilBonus = 1;
		if (GetPlayerOwner()->m_Shared.IsAimedIn() || pPlayer->m_Shared.IsSuperFalling())
		{
			float flAimIn = GetPlayerOwner()->m_Shared.GetAimIn();

			if (pPlayer->m_Shared.IsSuperFalling())
				flAimIn = 1;

			if (HasAimInRecoilBonus())
			{
				flPunchBonus = RemapValClamped(flAimIn, 0, 1, 1, 0.2f);
				flRecoilBonus = RemapValClamped(flAimIn, 0, 0.8f, 1, 0.2f);
			}
			else
			{
				flPunchBonus = RemapValClamped(flAimIn, 0, 1, 1, 0.8f);
				flRecoilBonus = RemapValClamped(flAimIn, 0, 1, 1, 0.6f);
			}
		}

		flPunchBonus = pPlayer->m_Shared.ModifySkillValue(flPunchBonus, -0.25f, SKILL_MARKSMAN);
		flRecoilBonus = pPlayer->m_Shared.ModifySkillValue(flRecoilBonus, -0.25f, SKILL_MARKSMAN);

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
	return m_flSpread;
}

#ifdef CLIENT_DLL
bool CWeaponSDKBase::ShouldDrawCrosshair()
{
	return UseVR();
}

void CWeaponSDKBase::CreateMove(float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles)
{	
	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		return;

	BaseClass::CreateMove(flInputSampleTime, pCmd, vecOldViewAngles);

	if (!GetPlayerOwner())
		return;

	Vector vecRecoil = GetPlayerOwner()->m_Shared.GetRecoil(flInputSampleTime);
	pCmd->viewangles[PITCH] -= vecRecoil.y;
	pCmd->viewangles[YAW] += vecRecoil.x;
}

void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha)
{
	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m );
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,0, 0 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * -flSize) + (vecUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,1, 0 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * flSize) + (vecUp * flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,1, 1 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * flSize) + (vecUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,0, 1 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * -flSize) + (vecUp * -flSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.End(false, true);
}

CMaterialReference g_hWeaponArrow;
CMaterialReference g_hGrenadeIcon;
int CWeaponSDKBase::DrawModel(int flags)
{
	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if (pLocalPlayer && pLocalPlayer->UseVRHUD() && pLocalPlayer == GetOwnerEntity())
	{
		Vector vecAmmo1, vecAmmo2;

		if (GetAttachment("ammo_1", vecAmmo1) && GetAttachment("ammo_2", vecAmmo2))
			DrawVRBullets(vecAmmo1, vecAmmo2, m_iClip1, GetMaxClip1(), true);

		if (GetAttachment("ammo_1_r", vecAmmo1) && GetAttachment("ammo_2_r", vecAmmo2))
			DrawVRBullets(vecAmmo1, vecAmmo2, rightclip, GetMaxClip1()/2, true);

		if (GetAttachment("ammo_1_l", vecAmmo1) && GetAttachment("ammo_2_l", vecAmmo2))
			DrawVRBullets(vecAmmo1, vecAmmo2, leftclip, GetMaxClip1()/2, false);
	}
	else
		s_iMaxClip = GetMaxClip1();

	if (!g_hWeaponArrow.IsValid())
		g_hWeaponArrow.Init( "particle/weaponarrow.vmt", TEXTURE_GROUP_OTHER );
	if (!g_hGrenadeIcon.IsValid())
		g_hGrenadeIcon.Init( "particle/grenadeicon.vmt", TEXTURE_GROUP_OTHER );

	int iReturn = BaseClass::DrawModel(flags);

	if (GetOwnerEntity())
	{
		m_flArrowCurSize = m_flArrowGoalSize = 0;

		return iReturn;
	}

	if (!pLocalPlayer)
		return iReturn;

	float flAppearDistance = 250;
	float flAppearDistanceSqr = flAppearDistance*flAppearDistance;

	if ((pLocalPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flAppearDistanceSqr)
		m_flArrowGoalSize = 5;
	else
		m_flArrowGoalSize = 0;

	float flTime = C_SDKPlayer::GetLocalOrSpectatedPlayer()->GetCurrentTime() + m_flArrowSpinOffset;
	float flFrameTime = gpGlobals->frametime * C_SDKPlayer::GetLocalOrSpectatedPlayer()->GetSlowMoMultiplier();

	m_flArrowCurSize = Approach(m_flArrowGoalSize, m_flArrowCurSize, flFrameTime*20);

	if (m_flArrowCurSize == 0)
		return iReturn;

	Vector vecOrigin = GetAbsOrigin() + Vector(0, 0, 10);
	Vector vecRight = Vector(sin(flTime*4), cos(flTime*4), 0);
	Vector vecUp = Vector(0, 0, 1);

	float flSize = m_flArrowCurSize;

	DrawIconQuad(g_hWeaponArrow, vecOrigin, vecRight, vecUp, flSize, 1);

	if (GetWeaponID() == SDK_WEAPON_GRENADE)
		DrawIconQuad(g_hGrenadeIcon, vecOrigin + Vector(0, 0, 10), vecRight, vecUp, flSize, 1);

	return iReturn;
}

static void RotateVertex( const Vector& x, const Vector& y, float flAngle, Vector& x_r, Vector& y_r )
{
	float flRadians = DEG2RAD( flAngle );

	x_r = x * cos(flRadians) - y * sin(flRadians);
	y_r = x * sin(flRadians) + y * cos(flRadians);
}

int C_WeaponSDKBase::s_iMaxClip = 0;
Vector C_WeaponSDKBase::s_vecAmmo1L;
Vector C_WeaponSDKBase::s_vecAmmo2L;
Vector C_WeaponSDKBase::s_vecAmmo1R;
Vector C_WeaponSDKBase::s_vecAmmo2R;
CUtlVector<C_WeaponSDKBase::CFlyingRound> C_WeaponSDKBase::s_aRounds;

CMaterialReference g_hBulletIcon;
void CWeaponSDKBase::DrawVRBullets(const Vector& vecAmmo1, const Vector& vecAmmo2, int iClip, int iMaxClip, bool bRight)
{
	if (!g_hBulletIcon.IsValid())
		g_hBulletIcon.Init( "da/bullet.vmt", TEXTURE_GROUP_OTHER );

	// This will get out of date since the processing is done before the drawing,
	// but not enough to notice.
	if (bRight)
	{
		s_vecAmmo1R = vecAmmo1;
		s_vecAmmo2R = vecAmmo2;
	}
	else
	{
		s_vecAmmo1L = vecAmmo1;
		s_vecAmmo2L = vecAmmo2;
	}
	s_iMaxClip = iMaxClip;

	for (int i = 0; i < iClip; i++)
		DrawIconQuad(g_hBulletIcon, GetVRRoundPosition(i, bRight), CurrentViewRight(), CurrentViewUp(), 0.7f, 1);

	float flFrameTime = gpGlobals->frametime;
	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if (pLocalPlayer)
		flFrameTime *= pLocalPlayer->GetSlowMoMultiplier();

	for (int i = 0; i < s_aRounds.Count(); i++)
	{
		CFlyingRound& oRound = s_aRounds[i];

		if (!oRound.bActive)
			continue;

		oRound.flAlpha -= flFrameTime * 0.7f;

		if (oRound.flAlpha < 0)
		{
			oRound.bActive = false;
			continue;
		}

		oRound.vecPosition += flFrameTime * oRound.vecVelocity;
		oRound.vecVelocity.z -= flFrameTime * 130;
		oRound.flAngle += flFrameTime * oRound.flAngularVelocity;

		Vector vecRight;
		Vector vecUp;

		RotateVertex(CurrentViewRight(), CurrentViewUp(), oRound.flAngle, vecRight, vecUp);

		DrawIconQuad(g_hBulletIcon, oRound.vecPosition, vecRight, vecUp, 0.7f, oRound.flAlpha);
	}
}

Vector CWeaponSDKBase::GetVRRoundPosition(int iRound, bool bRight)
{
	if (!s_iMaxClip)
		s_iMaxClip = 10;

	float f = (float)iRound/(float)s_iMaxClip;

	if (bRight)
		return Lerp(1-f, s_vecAmmo1R, s_vecAmmo2R);
	else
		return Lerp(1-f, s_vecAmmo1L, s_vecAmmo2L);
}

void CWeaponSDKBase::VRBulletFired(int iRound, bool bRight)
{
	int iSpot = -1;

	// Find a spot to put it.
	for (int j = 0; j < s_aRounds.Count(); j++)
	{
		if (!s_aRounds[j].bActive)
		{
			iSpot = j;
			break;
		}
	}

	if (iSpot == -1)
		iSpot = s_aRounds.AddToTail();

	Vector vecRight(0, 0, 0);
	Vector vecForward(0, 0, 0);
	Vector vecVelocity(0, 0, 0);

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if (pLocalPlayer)
	{
		AngleVectors(pLocalPlayer->EyeAngles(), NULL, &vecRight, NULL);
		vecVelocity = pLocalPlayer->GetAbsVelocity();
	}

	s_aRounds[iSpot].bActive = true;
	s_aRounds[iSpot].flAngle = 0;
	s_aRounds[iSpot].flAngularVelocity = RandomFloat(10, 1000);
	s_aRounds[iSpot].vecVelocity = vecVelocity + vecRight * RandomFloat(10, 20) + vecForward * RandomFloat(30, 50) + Vector(0, 0, 1) * RandomFloat(30, 50);
	s_aRounds[iSpot].vecPosition = GetVRRoundPosition(iRound, bRight);
	s_aRounds[iSpot].flAlpha = 1;
}

void CWeaponSDKBase::ClientThink()
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	BaseClass::ClientThink();

	float flFrameTime = gpGlobals->frametime;
	if (C_SDKPlayer::GetLocalOrSpectatedPlayer())
		flFrameTime *= C_SDKPlayer::GetLocalSDKPlayer()->GetSlowMoMultiplier();

	float flMarksmanGoldGoal = 0;
	if (GetPlayerOwner())
		flMarksmanGoldGoal = GetPlayerOwner()->IsStyleSkillActive(SKILL_MARKSMAN);

	if (flMarksmanGoldGoal > m_flMarksmanGold)
		flFrameTime *= 2;

	m_flMarksmanGold = Approach(flMarksmanGoldGoal, m_flMarksmanGold, flFrameTime);
}

void C_WeaponSDKBase::OnDataChanged( DataUpdateType_t type )
{
	// Skip C_BaseCombatWeapon, that code is replaced here.
	C_BaseAnimating::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
		SetNextClientThink( CLIENT_THINK_ALWAYS );

	CHandle< C_BaseCombatWeapon > handle = this;

	// If it's being carried by the *local* player, on the first update,
	// find the registered weapon for this ID

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatCharacter *pOwner = GetOwner();

	if (m_hLastOwner != pOwner)
	{
		if ( m_hLastOwner == pPlayer || pOwner == pPlayer )
		{
			if ( ShouldDrawPickup() )
			{
				CBaseHudWeaponSelection *pHudSelection = GetHudWeaponSelection();
				if ( pHudSelection )
					pHudSelection->OnWeaponPickup( this );

				pPlayer->EmitSound( "Player.PickupWeapon" );
			}
		}
	}

	bool bIsLocalPlayer = pPlayer && pPlayer == pOwner;
	if ( !bIsLocalPlayer )
	{
		int overrideModelIndex = CalcOverrideModelIndex();
		if( overrideModelIndex != -1 && overrideModelIndex != GetModelIndex() )
			SetModelIndex( overrideModelIndex );
	}

	UpdateVisibility();

	m_hLastOwner = pOwner;
}

float C_WeaponSDKBase::GetMarksmanGold()
{
	return m_flMarksmanGold;
}
#endif

float CWeaponSDKBase::GetViewPunchMultiplier()
{
	return m_flViewPunchMultiplier;
}

float CWeaponSDKBase::GetRecoil()
{
	return m_flRecoil;
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

ConVar da_decaymultiplier( "da_decaymultiplier", "0.7", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The multiplier for the recoil decay rate." );

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
	m_flAccuracyDecay -= (gpGlobals->frametime * da_decaymultiplier.GetFloat() * pPlayer->GetSlowMoMultiplier());

	if (m_flAccuracyDecay < 0)
		m_flAccuracyDecay = 0;

	bool bFired = false;

	if (IsThrowingGrenade())
	{
		if (MaintainGrenadeToss())
			return;
	}
	else if ((pPlayer->m_nButtons & IN_ALT2) && !IsThrowingGrenade() && pPlayer->GetAmmoCount(GetAmmoDef()->Index("grenades")) && pPlayer->CanAttack())
	{
		bool bAllow = (m_flNextPrimaryAttack < GetCurrentTime());
		if (m_bInReload)
			bAllow = true;

		if (bAllow)
		{
			StartGrenadeToss();
			return;
		}
	}

	// Secondary attack has priority
	if (m_flSwingTime > 0 && m_flSwingTime <= GetCurrentTime())
	{
		m_flSwingTime = 0;
		Swing();
	}
	else if ((pPlayer->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
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

	if (m_flUnpauseFromSwingTime > 0 && m_flUnpauseFromSwingTime <= GetCurrentTime())
	{
		CDAViewModel* vm = dynamic_cast<CDAViewModel*>(pPlayer->GetViewModel( m_nViewModelIndex ));
		if (vm && vm->IsAnimationPaused())
			vm->UnpauseAnimation();

		m_flUnpauseFromSwingTime = 0;
	}

	if ( !bFired && (pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() && (( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				pPlayer->ReadyWeapon();
				SendWeaponAnim( ACT_VM_DRYFIRE );
				m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
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

	int iWeight = GetWeight();

	if (iWeight + pPlayer->FindCurrentWeaponsWeight() > MAX_LOADOUT_WEIGHT)
		pPlayer->DropWeaponsToPickUp(this);

	if (iWeight + pPlayer->FindCurrentWeaponsWeight() > MAX_LOADOUT_WEIGHT)
		return;

	// ----------------------------------------
	// If I already have it just take the ammo
	// ----------------------------------------
	if (pPlayer->Weapon_OwnsThisType( GetClassname(), GetSubType())) 
	{
		if (HasAkimbo())
		{
			// I already have one of these, and there's an akimbo version. Do we have it?
			if (!pPlayer->FindWeapon(AliasToWeaponID(GetSDKWpnData().m_szAkimbo)))
			{
				// We don't have the akimbo version. Give it.
				CAkimboBase* pAkimbo = (CAkimboBase*)pPlayer->GiveNamedItem(UTIL_VarArgs("weapon_%s", GetSDKWpnData().m_szAkimbo));
				if (pAkimbo)
				{
					// Second pistol is always the left one
					pAkimbo->leftclip = m_iClip1;
					pAkimbo->SetOwner(pPlayer);

					// Player already has a weapon like this one, so just remove this one.
					UTIL_Remove(this);

					pPlayer->Weapon_Switch(pAkimbo);

					pPlayer->Instructor_LessonLearned("pickupweapon");
					return;
				}
			}

			// We already have the akimbo version. Don't pick it up.
			return;
		}

		if ( pPlayer->Weapon_EquipAmmoOnly( this ) )
		{
			// Only remove me if I have no ammo left
			if ( HasPrimaryAmmo() )
				return;

			if (GetWeaponID() == SDK_WEAPON_GRENADE && pPlayer->GetActiveSDKWeapon() && pPlayer->GetActiveSDKWeapon()->GetWeaponID() != SDK_WEAPON_BRAWL)
			{
				// We can throw it without switching to it and it'll appear on the HUD. Don't switch.
			}
			else
				pPlayer->Weapon_Switch(pPlayer->FindWeapon(GetWeaponID()));

			UTIL_Remove( this );
			OnPickedUp( pPlayer );

			pPlayer->Instructor_LessonLearned("pickupweapon");
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
			Holster(NULL);
		else
		{
			if (GetWeaponID() == SDK_WEAPON_GRENADE && pPlayer->GetActiveSDKWeapon() && pPlayer->GetActiveSDKWeapon()->GetWeaponID() != SDK_WEAPON_BRAWL)
			{
				// We can throw it without switching to it and it'll appear on the HUD. Don't switch.
			}
			else
				pPlayer->Weapon_Switch( this );
		}

		OnPickedUp( pPlayer );

		pPlayer->Instructor_LessonLearned("pickupweapon");
	}
#endif
}

extern void FX_TracerSound( const Vector &start, const Vector &end, int iTracerType );
void CWeaponSDKBase::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, bool bUseTracerAttachment )
{
#ifdef CLIENT_DLL
	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		return;

	CNewParticleEffect *pTracer = NULL;
	C_SDKPlayer *pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (pLocalPlayer)
	{
		int iObsMode = pLocalPlayer->GetObserverMode();
		bool bPovObs = iObsMode == OBS_MODE_IN_EYE && pLocalPlayer->GetObserverTarget() == GetOwner();

		if (bUseTracerAttachment)
		{
			if( pLocalPlayer == GetOwner() && !pLocalPlayer->IsInThirdPerson() || bPovObs )
			{
				for ( int i = 0; i < MAX_VIEWMODELS; i++ )
				{
					CBaseViewModel *vm = pLocalPlayer->GetViewModel( i );
					if ( !vm )
						continue;

					pTracer = vm->ParticleProp()->Create( "tracer_bullet", PATTACH_POINT, GetTracerAttachment());
				}
			}
			else
				pTracer = ParticleProp()->Create( "tracer_bullet", PATTACH_POINT, GetTracerAttachment());
		}
		else
		{
			pTracer = ParticleProp()->Create( "tracer_bullet", PATTACH_CUSTOMORIGIN );
			pTracer->SetControlPoint(0, vecTracerSrc);
		}

		// just in case we couldn't get a view model
		if( pTracer == NULL )
			return;

		// Set the particle effect's destination to our bullet's termination point
		pTracer->SetControlPoint( 1, tr.endpos );
		pTracer->SetSortOrigin( vecTracerSrc );
		
		//whiz (but don't whiz yourself) (also don't whiz in obs mode unless we're in POV)
		if( pLocalPlayer != GetOwner() && !bPovObs && !(iObsMode > 0 && iObsMode != OBS_MODE_IN_EYE) )
			FX_TracerSound( vecTracerSrc, tr.endpos, iTracerType );
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
		ToSDKPlayer(GetOwner())->UseStyleCharge(SKILL_MARKSMAN, 5);

		CSDKPlayer* pSDKOwner = ToSDKPlayer(GetOwner());

		if (pSDKOwner)
			pSDKOwner->ReadyWeapon();

		float flSpeedMultiplier = GetSDKWpnData().m_flReloadTimeMultiplier;

		flSpeedMultiplier = GetPlayerOwner()->m_Shared.ModifySkillValue(flSpeedMultiplier, -0.2f, SKILL_MARKSMAN);

		if (GetPlayerOwner()->m_Shared.IsSuperFalling())
			flSpeedMultiplier *= 0.5f;

		float flReloadSequenceDuration = SequenceDuration();

		// The client uses the w model and the server uses the v model so
		// client will return sequence duration 0 and server will return the
		// actual. This causes prediction problems, so instead of mucking
		// around with the SDK base code I just make sure there's some
		// substantive value larger than ping.
		if (flReloadSequenceDuration < 0.5f)
			flReloadSequenceDuration = 0.5f;

		float flSequenceEndTime = GetCurrentTime() + flReloadSequenceDuration * flSpeedMultiplier;

		if (pSDKOwner)
		{
			CBaseViewModel *vm = pSDKOwner->GetViewModel( m_nViewModelIndex );

			if (vm)
				vm->SetPlaybackRate( 1/flSpeedMultiplier );

			pSDKOwner->Instructor_LessonLearned("reload");
		}

		MDLCACHE_CRITICAL_SECTION();
		// Don't SetNextAttack since that forces ItemBusyFrame
		// meaning we won't be able to throw grenades during reload.
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;
		m_flReloadEndTime = flSequenceEndTime;

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
	CSDKPlayer *pOwner = GetPlayerOwner();

	if (pOwner)
	{
		bool bConsume = true;
		if (pOwner->IsStyleSkillActive(SKILL_MARKSMAN))
			bConsume = false;

		// If I use primary clips, reload primary
		if ( UsesClipsForAmmo1() )
		{
			int primary	= min( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));	
			m_iClip1 += primary;

			if (bConsume)
				pOwner->RemoveAmmo( primary, m_iPrimaryAmmoType);
		}

		// If I use secondary clips, reload secondary
		if ( UsesClipsForAmmo2() )
		{
			int secondary = min( GetMaxClip2() - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
			m_iClip2 += secondary;

			if (bConsume)
				pOwner->RemoveAmmo( secondary, m_iSecondaryAmmoType );
		}

		if ( m_bReloadsSingly )
		{
			m_bInReload = false;
		}

		pOwner->ReadyWeapon();
	}

#ifdef CLIENT_DLL
	if (!prediction->InPrediction() || prediction->IsFirstTimePredicted())
	{
		CHudElement* pElement = gHUD.FindElement("CHudAmmo");
		if (pElement)
		{
			CHudAmmo* pHudAmmo = dynamic_cast<CHudAmmo*>(pElement);
			if (pHudAmmo)
				pHudAmmo->Reload(this);
		}
	}
#endif
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

		if ((m_bInReload) && (m_flReloadEndTime <= GetCurrentTime()))
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
		if ( (m_bInReload) && (m_flReloadEndTime <= GetCurrentTime()))
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
			 GetCurrentTime() > m_flNextPrimaryAttack + 1.0f &&
			 GetCurrentTime() > m_flNextSecondaryAttack + 1.0f )
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

	flSpeedMultiplier = pOwner->m_Shared.ModifySkillValue(flSpeedMultiplier, -0.3f, SKILL_MARKSMAN);

	m_flNextPrimaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;
	m_flNextSecondaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;
	m_flNextBrawlTime = GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;

	if (pOwner)
	{
		CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );

		if (vm)
			vm->SetPlaybackRate( 1/flSpeedMultiplier );

		pOwner->DoAnimationEvent( PLAYERANIMEVENT_DRAW );

		pOwner->SetNextAttack( GetCurrentTime() + SequenceDuration() * flSpeedMultiplier );

		pOwner->m_Shared.SetAimIn(0.0f);
	}

	if (bDeploy && pOwner)
		pOwner->ReadyWeapon();

	m_flSwingTime = 0;
	m_flUnpauseFromSwingTime = 0;

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

	m_flSwingTime = 0;
	m_flUnpauseFromSwingTime = 0;
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

const Vector CWeaponSDKBase::GetShootPosition(CSDKPlayer* pShooter)
{
	Assert(pShooter);
	if (!pShooter)
		return Vector(0, 0, 0);

	return pShooter->EyePosition();
}

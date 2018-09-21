//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include "tier0/vprof.h"

#ifdef CLIENT_DLL
	#include "input.h"
#endif

#include "sdk_gamerules.h"

#include "takedamageinfo.h"

#include "effect_dispatch_data.h"
#include "weapon_sdkbase.h"
#include "movevars_shared.h"
#include "gamevars_shared.h"
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "engine/ivdebugoverlay.h"
#include "obstacle_pushaway.h"
#include "props_shared.h"
#include "ammodef.h"
#include "in_buttons.h"

#include "decals.h"
#include "util_shared.h"

#ifdef CLIENT_DLL
	
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"
	#include "prediction.h"
	#include "clientmode_sdk.h"
	#include "vgui_controls/AnimationController.h"
	#include "c_sdk_player_resource.h"

	#define CRecipientFilter C_RecipientFilter
#else
	#include "variant_t.h"

	#include "sdk_player.h"
	#include "sdk_team.h"
	#include "dove.h"
#endif

#include "da.h"

#include "da_bulletmanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Have to override all of ItemPostFrame to s/gpGlobals->curtime/GetCurrentTime()/
void CSDKPlayer::ItemPostFrame()
{
	VPROF( "CSDKPlayer::ItemPostFrame" );

	// Put viewmodels into basically correct place based on new player origin
	CalcViewModelView( EyePosition(), EyeAngles() );

	Assert(!GetVehicle());
	// If we ever add vehicles, add the vehicle bits from CBasePlayer::ItemPostFrame()

	// check if the player is using something
	if ( GetUseEntity() != NULL )
	{
#if !defined( CLIENT_DLL )
		Assert( !IsInAVehicle() );
		ImpulseCommands();// this will call playerUse
#endif
		return;
	}

	if ( GetCurrentTime() < m_flNextAttack )
	{
		if ( GetActiveWeapon() )
			GetActiveWeapon()->ItemBusyFrame();
	}
	else
	{
		if ( GetActiveWeapon() && (!IsInAVehicle() || UsingStandardWeaponsInVehicle()) )
		{
#if defined( CLIENT_DLL )
			// Not predicting this weapon
			if ( GetActiveWeapon()->IsPredicted() )
#endif

				GetActiveWeapon()->ItemPostFrame( );
		}
	}

#if !defined( CLIENT_DLL )
	ImpulseCommands();
#else
	// NOTE: If we ever support full impulse commands on the client,
	// remove this line and call ImpulseCommands instead.
	ResetImpulse();
#endif
}

ConVar da_stylemeteractivationcost( "da_stylemeteractivationcost", "100", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much (out of 100) does it cost to activate your style meter?" );

void CSDKPlayer::FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   SDKWeaponID eWeaponID,	// weapon that fired this shot
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   )
{
	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

	if ( !pevAttacker )
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize( vecDir );

	CSDKPlayer* pSDKAttacker = ToSDKPlayer(pevAttacker);
	Assert(pSDKAttacker);
	Assert(pSDKAttacker == this);

	CBulletManager::CBullet oBullet = BulletManager().MakeBullet(pSDKAttacker, vecSrc, vecDir, eWeaponID, GetActiveSDKWeapon(), iDamage, iBulletType, bDoEffects);

	if (pSDKAttacker && pSDKAttacker->GetSlowMoMultiplier() < 1 && pSDKAttacker->GetSlowMoType() != SLOWMO_SUPERFALL && !pSDKAttacker->m_Shared.IsSuperFalling())
	{
		BulletManager().AddBullet(oBullet);
		return;
	}

	BulletManager().SimulateBullet(oBullet, -1);

	Assert(!oBullet.m_bActive);
}

void CSDKPlayer::DoMuzzleFlash(int iAkimbo)
{
#ifdef CLIENT_DLL
	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		return;

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	C_WeaponSDKBase* pActiveWeapon = GetActiveSDKWeapon();

	if (pLocalPlayer)
	{
		if (pLocalPlayer == this && !::input->CAM_IsThirdPerson() || pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pLocalPlayer->GetObserverTarget() == this)
		{
			for ( int i = 0; i < MAX_VIEWMODELS; i++ )
			{
				CBaseViewModel *vm = GetViewModel( i );
				if ( !vm )
					continue;

				vm->DoMuzzleFlash();
			}
		}
		else if (pActiveWeapon)
		{
			const char* pszMuzzleAttachment = "muzzle";
			if (iAkimbo == 1)
				pszMuzzleAttachment = "muzzle1";
			else if (iAkimbo == 2)
				pszMuzzleAttachment = "muzzle2";

			// Force world model so the attachments work.
			pActiveWeapon->SetModelIndex( pActiveWeapon->GetWorldModelIndex() );

			switch (pActiveWeapon->GetWeaponType())
			{
			case WT_PISTOL:
			default:
				pActiveWeapon->ParticleProp()->Create( "muzzleflash_pistol", PATTACH_POINT_FOLLOW, pszMuzzleAttachment );
				break;

			case WT_SMG:
				pActiveWeapon->ParticleProp()->Create( "muzzleflash_smg", PATTACH_POINT_FOLLOW, pszMuzzleAttachment );
				break;

			case WT_RIFLE:
				pActiveWeapon->ParticleProp()->Create( "muzzleflash_rifle", PATTACH_POINT_FOLLOW, pszMuzzleAttachment );
				break;

			case WT_SHOTGUN:
				pActiveWeapon->ParticleProp()->Create( "muzzleflash_shotgun", PATTACH_POINT_FOLLOW, pszMuzzleAttachment );
				break;
			}
		}
	}
#endif
}

void CSDKPlayer::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, bool bUseTracerAttachment )
{
#ifdef CLIENT_DLL
	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
	if (pWeapon)
		pWeapon->MakeTracer( vecTracerSrc, tr, iTracerType, bUseTracerAttachment );
#endif
}

bool CSDKPlayer::CanMove( void ) const
{
	bool bValidMoveState = (State_Get() == STATE_ACTIVE || State_Get() == STATE_OBSERVER_MODE);
			
	if ( !bValidMoveState )
	{
		return false;
	}

	return true;
}

// BUG! This is not called on the client at respawn, only first spawn!
void CSDKPlayer::SharedSpawn()
{	
	BaseClass::SharedSpawn();

	// Reset the animation state or we will animate to standing
	// when we spawn

	SetGravity(1);
#ifdef CLIENT_DLL
	m_flCurrentAlphaVal = 255.0f;
#endif
	m_flReadyWeaponUntil = -1;
	m_bThirdPersonCamSide = true;
	m_flSideLerp = m_bThirdPersonCamSide?1:-1;

	m_Shared.SetJumping( false );

	m_Shared.m_flViewTilt = 0;
	m_Shared.m_flViewBobRamp = 0;
	m_Shared.m_flLastDuckPress = -1;
	m_Shared.m_bDiving = false;
	m_Shared.m_bRolling = false;
	m_Shared.m_bSliding = false;
	m_Shared.m_bDiveSliding = false;
	m_Shared.m_flDiveToProneLandTime = -1;
	m_Shared.m_bProne = false;
	m_Shared.m_bAimedIn = false;
	m_Shared.m_bIsTryingUnprone = false;
	m_Shared.m_bIsTryingUnduck = false;
	m_Shared.m_iWallFlipCount = 0;
	m_Shared.m_bIsWallFlipping = false;
	m_Shared.m_flWallFlipEndTime = 0;
	m_Shared.m_bIsManteling = false;
	m_Shared.m_flSuperFallOthersNextCheck = 0;
	m_Shared.m_bSuperFalling = false;
	m_Shared.m_bSuperSkill = false;

	//Tony; todo; fix

//	m_flMinNextStepSoundTime = gpGlobals->curtime;
#if defined ( SDK_USE_PRONE )
//	m_bPlayingProneMoveSound = false;
#endif // SDK_USE_PRONE
}

void CSDKPlayer::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch(pOther);

	CGameTrace tr;
	tr = GetTouchTrace();

	// If I'm diving and I hit a wall at a blunt angle, don't roll afterwards.
	if (m_Shared.IsDiving() && tr.plane.normal.Dot(m_Shared.m_vecDiveDirection) < -0.95f)
		m_Shared.m_bRollAfterDive = false;
}

#define GRENADE_PICKUP_RADIUS 100.f

bool CSDKPlayer::PlayerUse()
{
#ifdef GAME_DLL
	if ((m_afButtonPressed & IN_USE) && m_Shared.CanSuperFallRespawn())
		CommitSuicide(false, true);

	// Was use pressed or released?
	if ( ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) && !IsObserver() )
	{
		Vector forward, up;
		EyeVectors( &forward, NULL, &up );

		Vector vecSearchCenter = EyePosition();
		CBaseEntity *pObject = NULL;
		CBaseEntity *pNearest = NULL;
		float flNearest = FLT_MAX;

		// Look for grenades so we can prioritize picking them up first.
		for ( CEntitySphereQuery sphere( vecSearchCenter, GRENADE_PICKUP_RADIUS ); ( pObject = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if ( !pObject )
				continue;

			if ( !IsUseableEntity( pObject, FCAP_USE_IN_RADIUS ) )
				continue;

			CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(pObject);
			if (!pWeapon)
				continue;

			if (pWeapon->GetWeaponID() != SDK_WEAPON_GRENADE)
				continue;

			// If we're full up on grenades, pass over to whatever other weapons are lying around.
			if (!g_pGameRules->CanHavePlayerItem(this, pWeapon))
				continue;

			// see if it's more roughly in front of the player than previous guess
			Vector point;
			pObject->CollisionProp()->CalcNearestPoint( vecSearchCenter, &point );

			Vector dir = point - vecSearchCenter;
			VectorNormalize(dir);
			float dot = DotProduct( dir, forward );

			// Need to be looking at the object more or less
			if ( dot < 0.8 )
				continue;

			float dist = CalcDistanceToLine( point, vecSearchCenter, forward );

			ConVarRef sv_debug_player_use("sv_debug_player_use");
			if ( sv_debug_player_use.GetBool() )
			{
				Msg("Radius found %s, dist %.2f\n", pObject->GetClassname(), dist );
			}

			// Not worried about shit being behind a wall at this point.
			// Just greedily gobble up all nearby grenades since there's
			// no penalty to the player for doing so.

			if ( dist < flNearest )
			{
				pNearest = pObject;
				flNearest = dist;
			}
		}

		if (pNearest)
		{
			// This is a grenade. Use it to pick it up.
			variant_t emptyVariant;
			pNearest->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );
			return true;
		}
	}
#endif

	bool bUsed = BaseClass::PlayerUse();

	if (bUsed)
		return bUsed;

	if (!(m_afButtonPressed & IN_USE))
		return false;

	if (!IsAlive())
		return false;

	return false;
}

#if defined ( SDK_USE_SPRINTING )
void CSDKPlayer::SetSprinting( bool bIsSprinting )
{
	m_Shared.SetSprinting( bIsSprinting );
}

bool CSDKPlayer::IsSprinting( void )
{
	float flVelSqr = GetAbsVelocity().LengthSqr();

	return m_Shared.m_bIsSprinting && ( flVelSqr > 0.5f );
}
#endif // SDK_USE_SPRINTING

bool CSDKPlayer::CanAttack( void )
{
#if defined ( SDK_USE_SPRINTING )
	#if !defined ( SDK_SHOOT_WHILE_SPRINTING )
		if ( IsSprinting() ) 
			return false;
	#endif // SDK_SHOOT_WHILE_SPRINTING
#endif // SDK_USE_SPRINTING

#if !defined ( SDK_SHOOT_WHILE_JUMPING )
	if ( m_Shared.IsJumping() )
		return false;
#endif  //SDK_SHOOT_WHILE_JUMPING

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the players mins - overridden for prone
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CSDKPlayer::GetPlayerMins( void ) const
{
	if ( IsObserver() )
	{
		return VEC_OBS_HULL_MIN;	
	}
	else
	{
		if ( GetFlags() & FL_DUCKING )
			return VEC_DUCK_HULL_MIN;
		else if ( m_Shared.IsDiving() )
			return VEC_DIVE_HULL_MIN;
#if defined ( SDK_USE_PRONE )
		else if ( m_Shared.IsProne() )
			return VEC_PRONE_HULL_MIN;
#endif // SDK_USE_PRONE
		else if ( m_Shared.IsSliding() )
			return VEC_SLIDE_HULL_MIN;
		else if ( m_Shared.IsRolling() )
			return VEC_DUCK_HULL_MIN;
		else
			return VEC_HULL_MIN;
	}
}

//-----------------------------------------------------------------------------
// Purpose:  Returns the players Maxs - overridden for prone
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CSDKPlayer::GetPlayerMaxs( void ) const
{	
	if ( IsObserver() )
	{
		return VEC_OBS_HULL_MAX;	
	}
	else
	{
		if ( GetFlags() & FL_DUCKING )
			return VEC_DUCK_HULL_MAX;
		else if ( m_Shared.IsDiving() )
			return VEC_DIVE_HULL_MAX;
#if defined ( SDK_USE_PRONE )
		else if ( m_Shared.IsProne() )
			return VEC_PRONE_HULL_MAX;
#endif // SDK_USE_PRONE
		else if ( m_Shared.IsSliding() )
			return VEC_SLIDE_HULL_MAX;
		else if ( m_Shared.IsRolling() )
			return VEC_DUCK_HULL_MAX;
		else
			return VEC_HULL_MAX;
	}
}

float CSDKPlayer::GetStylePoints()
{
	if (SDKGameRules()->GetBountyPlayer() == this)
		return da_stylemeteractivationcost.GetFloat();

	return m_flStylePoints;
}

ConVar da_styletime( "da_styletime", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Turns on the player's style skill all the time." );
bool CSDKPlayer::IsStyleSkillActive(SkillID eSkill) const
{
	if (da_styletime.GetBool())
	{
		if (eSkill == SKILL_NONE)
			return true;
		else if (m_Shared.m_bSuperSkill || SDKGameRules()->GetBountyPlayer() == this)
			return true;
		else
			return m_Shared.m_iStyleSkill == eSkill;
	}

	if (m_flStyleSkillCharge == 0)
		return false;

	if (eSkill == SKILL_NONE)
		return true;
	else if (m_Shared.m_bSuperSkill || SDKGameRules()->GetBountyPlayer() == this)
		return true;
	else
		return m_Shared.m_iStyleSkill == eSkill;
}

static ConVar da_style_decay_max("da_style_decay_max", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar da_style_decay_min("da_style_decay_min", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CSDKPlayer::DecayStyle()
{
	if (IsStyleSkillActive())
		return;

	float flDecayPerSecond = RemapValClamped(m_flStylePoints, 0, da_stylemeteractivationcost.GetFloat(), da_style_decay_min.GetFloat(), da_style_decay_max.GetFloat());
	float flDecayThisFrame = flDecayPerSecond * gpGlobals->frametime * GetSlowMoMultiplier();
	m_flStylePoints = Approach(0, m_flStylePoints, flDecayThisFrame);
}

void CSDKPlayer::UseStyleCharge(SkillID eSkill, float flCharge)
{
	if (SDKGameRules()->GetBountyPlayer() == this)
		return;

	if (!IsStyleSkillActive(eSkill))
		return;

	if (m_Shared.m_bSuperSkill)
		flCharge *= 0.5f; // They earned it, let them keep it a while longer.

	m_flStyleSkillCharge = max(m_flStyleSkillCharge - flCharge, 0);

	if (!m_flStyleSkillCharge)
	{
		m_Shared.m_bSuperSkill = false;
		m_flStylePoints = 0;

		if (m_flSlowMoSeconds == 0)
			m_bHasSuperSlowMo = m_Shared.m_bSuperSkill;
	}
}

int CSDKPlayer::GetStyleStars(float flPoints)
{
	float flActivation = da_stylemeteractivationcost.GetFloat();

	return flPoints / (flActivation/100);
}

int CSDKPlayer::GetStyleStars()
{
	float flTotalStyle;

#ifdef GAME_DLL
	Assert(false);
	flTotalStyle = 0;
#else
	C_SDK_PlayerResource *sdkPR = SDKGameResources();

	flTotalStyle = sdkPR->GetStyle(entindex());
#endif

	return GetStyleStars(flTotalStyle);
}

void CSDKPlayer::FreezePlayer(float flAmount, float flTime)
{
	m_flFreezeAmount = flAmount;

	if (m_Shared.m_iStyleSkill == SKILL_BOUNCER || m_Shared.m_bSuperSkill || SDKGameRules()->GetBountyPlayer() == this)
		m_flFreezeAmount = RemapVal(m_flFreezeAmount, 0, 1, m_Shared.ModifySkillValue(1, -0.25f, SKILL_BOUNCER), 1);

	if (flAmount == 1.0f)
		m_flFreezeUntil = m_flCurrentTime;
	else if (flTime < 0)
		m_flFreezeUntil = 0;
	else
		m_flFreezeUntil = m_flCurrentTime + flTime;
}

void CSDKPlayer::ReadyWeapon()
{
	m_flReadyWeaponUntil = GetCurrentTime() + 3.5f;
}

bool CSDKPlayer::IsWeaponReady()
{
	if (m_Shared.IsAimedIn())
		return true;

#ifdef CLIENT_DLL
	// If using the VR hud, always use the weapon ready animation,
	// since the bullets are more clear
	if (UseVRHUD())
		return true;
#endif

	return GetCurrentTime() < m_flReadyWeaponUntil;
}

bool CSDKPlayer::PlayerFrozen()
{
	// m_flFreezeUntil == 0 means to freeze for an indefinite amount of time.
	// Otherwise it means freeze until curtime >= m_flFreezeUntil.
	return (m_flFreezeUntil == 0) || (m_flCurrentTime < m_flFreezeUntil);
}

// --------------------------------------------------------------------------------------------------- //
// CSDKPlayerShared implementation.
// --------------------------------------------------------------------------------------------------- //
CSDKPlayerShared::CSDKPlayerShared()
{
#if defined( SDK_USE_PRONE )
	m_bProne = false;
	m_flNextProneCheck = 0;
	m_flUnProneTime = 0;
	m_flGoProneTime = 0;
#endif

#if defined ( SDK_USE_PLAYERCLASSES )
	SetDesiredPlayerClass( PLAYERCLASS_UNDEFINED );
#endif

	m_bSliding = false;
	m_bDiveSliding = false;
	m_bRolling = false;
	m_flSlideStartTime = 0;
	m_flSlideAutoEndTime = 0;
}

CSDKPlayerShared::~CSDKPlayerShared()
{
}

void CSDKPlayerShared::Init( CSDKPlayer *pPlayer )
{
	m_pOuter = pPlayer;
}

bool CSDKPlayerShared::IsDucking( void ) const
{
	return ( m_pOuter->GetFlags() & FL_DUCKING ) ? true : false;
}

#if defined ( SDK_USE_PRONE )
bool CSDKPlayerShared::IsProne() const
{
	return m_bProne;
}

bool CSDKPlayerShared::IsGettingUpFromProne() const
{
	return ( m_flUnProneTime > 0 );
}

bool CSDKPlayerShared::IsGoingProne() const
{
	return ( m_flGoProneTime > 0 );
}

void CSDKPlayerShared::SetProne( bool bProne, bool bNoAnimation /* = false */ )
{
	m_bProne = bProne;
	m_bProneSliding = false;
	m_flDisallowUnProneTime = -1;

	if (bNoAnimation)
	{
		m_flGoProneTime = 0;
		m_flUnProneTime = 0;
	}

	if (!bProne)	// forceunzoom for going prone is in StartGoingProne
	{
		ForceUnzoom();
		m_pOuter->ReadyWeapon();
	}		
}

void CSDKPlayerShared::StartGoingProne( void )
{
	// make the prone sound
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoProne" );

	// slow to prone speed
	m_flGoProneTime = m_pOuter->GetCurrentTime() + TIME_TO_PRONE;

	m_flUnProneTime = 0.0f;	//reset

	if ( IsSniperZoomed() )
		ForceUnzoom();
}

void CSDKPlayerShared::StandUpFromProne( void )
{	
	// make the prone sound
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.UnProne" );

	// speed up to target speed
	m_flUnProneTime = m_pOuter->GetCurrentTime() + TIME_TO_PRONE;

	m_flGoProneTime = 0.0f;	//reset 
}

bool CSDKPlayerShared::CanChangePosition( void ) const
{
	if ( IsGettingUpFromProne() )
		return false;

	if ( IsGoingProne() )
		return false;

	return true;
}

#endif

bool CSDKPlayerShared::IsGettingUpFromSlide() const
{
	return ( m_flLastUnSlideTime > 0 );
}

bool CSDKPlayerShared::IsSliding() const
{
	return m_bSliding && m_pOuter->m_lifeState != LIFE_DEAD;
}

bool CSDKPlayerShared::IsDiveSliding() const
{
	return IsSliding() && m_bDiveSliding;
}

extern ConVar da_d2p_stunt_forgiveness;

bool CSDKPlayerShared::CanSlide() const
{
	if (m_pOuter->GetLocalVelocity().Length2D() < 10)
		return false;

	if (IsProne() && m_pOuter->GetCurrentTime() - m_flDiveToProneLandTime > da_d2p_stunt_forgiveness.GetFloat())
		return false;

	if (IsSliding())
		return false;

	if (IsRolling())
		return false;

	if (IsDucking())
		return false;

	if (IsDiving())
		return false;

	if (!CanChangePosition())
		return false;

	return true;
}

void CSDKPlayerShared::PlayStartSlideSound()
{
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoSlide" );
}

void CSDKPlayerShared::PlayEndSlideSound()
{
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.UnSlide" );
}

void CSDKPlayerShared::StartSliding(bool bDiveSliding)
{
	if (!CanSlide())
		return;

	m_pOuter->UseStyleCharge(SKILL_ATHLETIC, 10);

	PlayStartSlideSound();

	m_bSliding = true;
	m_bDiveSliding = bDiveSliding;
	SetAirSliding(false);

	ForceUnzoom();

	m_vecSlideDirection = m_pOuter->GetAbsVelocity();
	m_vecSlideDirection.GetForModify().NormalizeInPlace();

	ConVarRef sdk_slidetime("sdk_slidetime");

	m_flSlideStartTime = m_pOuter->GetCurrentTime();
	m_flSlideAutoEndTime = m_flSlideStartTime + sdk_slidetime.GetFloat();
	m_flLastUnSlideTime = 0;
}

void CSDKPlayerShared::EndSlide()
{
	// If it was long enough to notice what it was, then train the slide.
	if (m_pOuter->GetCurrentTime() - m_flSlideStartTime > 1)
	{
		if (m_bDiveSliding)
			m_pOuter->Instructor_LessonLearned("slideafterdive");
		else
			m_pOuter->Instructor_LessonLearned("slide");
	}

	m_bSliding = false;
	m_bDiveSliding = false;
	
	m_pOuter->ReadyWeapon();
}

void CSDKPlayerShared::StandUpFromSlide( bool bJumpUp )
{	
	// If it was long enough to notice what it was, then train the slide.
	if (m_pOuter->GetCurrentTime() - m_flSlideStartTime > 1)
	{
		if (m_bDiveSliding)
			m_pOuter->Instructor_LessonLearned("slideafterdive");
		else
			m_pOuter->Instructor_LessonLearned("slide");
	}

	// if we're in the air we've already played the end slide sound
	if ( !IsAirSliding() )
	{
		PlayEndSlideSound();

		// if we're going into a jump: block unwanted slide behavior
		if (bJumpUp)
		{
			m_bSliding = false;
			// and trigger brief slide cooldown
			m_flSlideStartTime = m_pOuter->GetCurrentTime();
		}
		else
			m_pOuter->FreezePlayer(0.4f, 0.3f);
	}
	else
	{
		// we're in the air so just end the slide
		m_bSliding = false;
		SetAirSliding(false);
	}

	m_flLastUnSlideTime = m_pOuter->GetCurrentTime() + TIME_TO_UNSLIDE;

	m_vecUnSlideEyeStartOffset = m_pOuter->GetViewOffset();
}

float CSDKPlayerShared::GetSlideFriction() const
{
	if (!m_bSliding)
		return 1;

	// While there are more than 0.2 seconds until the slide automatically ends...
	if (m_flSlideAutoEndTime - m_pOuter->GetCurrentTime() > 0.2f)
		return 0.05f; // ...Use a low friction coefficient

	return 1;
}

void CSDKPlayerShared::SetDuckPress(bool bReset)
{
	if (bReset)
		m_flLastDuckPress = -1;
	else
		m_flLastDuckPress = gpGlobals->curtime;
}

bool CSDKPlayerShared::IsRolling() const
{
	return m_bRolling && m_pOuter->m_lifeState != LIFE_DEAD;
}

bool CSDKPlayerShared::CanRoll() const
{
	if (m_pOuter->GetLocalVelocity().Length2D() < 10)
		return false;

	if (IsProne() && m_pOuter->GetCurrentTime() - m_flDiveToProneLandTime > da_d2p_stunt_forgiveness.GetFloat())
		return false;

	if (IsSliding())
		return false;

	if (IsRolling())
		return false;

	if (IsDiving())
		return false;

	if (!CanChangePosition())
		return false;

	if (!(m_pOuter->GetFlags() & FL_ONGROUND))
		return false;

	return true;
}

void CSDKPlayerShared::StartRolling(bool bFromDive)
{
	if (!CanRoll())
		return;

	if (bFromDive)
		m_pOuter->Instructor_LessonLearned("rollafterdive");

	m_bRolling = true;
	m_bRollingFromDive = bFromDive;

	ForceUnzoom();

	m_vecRollDirection = m_pOuter->GetAbsVelocity();
	m_vecRollDirection.GetForModify().NormalizeInPlace();

	m_flRollTime = m_pOuter->GetCurrentTime();
}

void CSDKPlayerShared::EndRoll()
{
	m_bRolling = false;
	m_flRollTime = 0;

	m_pOuter->ReadyWeapon();
}

bool CSDKPlayerShared::IsDiving() const
{
	return m_bDiving && m_pOuter->m_lifeState != LIFE_DEAD;
}

bool CSDKPlayerShared::CanDive() const
{
	if (m_pOuter->GetLocalVelocity().Length2D() < 10)
		return false;

	if (IsProne())
		return false;

	if (IsSliding())
		return false;

	if (IsRolling())
		return false;

	if (IsDucking())
		return false;

	if (IsDiving())
		return false;

	if (!CanChangePosition())
		return false;

	if (m_pOuter->GetCurrentTime() - m_flTimeLeftGround > 2)
		return false;

	return true;
}

ConVar  sdk_dive_height( "sdk_dive_height", "150", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar  sdk_dive_gravity("sdk_dive_gravity", "0.6", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

ConVar  sdk_dive_height_high("sdk_dive_height_high", "200", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

ConVar  sdk_dive_speed_adrenaline("sdk_dive_speed_adrenaline", "500", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
ConVar  sdk_dive_height_adrenaline("sdk_dive_height_adrenaline", "150", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
ConVar  sdk_dive_gravity_adrenaline("sdk_dive_gravity_adrenaline", "0.6", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

ConVar  da_acro_dive_arc ("da_acro_dive_arc", "90", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

Vector CSDKPlayerShared::StartDiving()
{
	if (!CanDive())
		return m_pOuter->GetAbsVelocity();

	m_flDiveTime = m_pOuter->GetCurrentTime();
	m_flDiveLerped = 0;
	m_flDiveToProneLandTime = -1;

	m_pOuter->UseStyleCharge(SKILL_ATHLETIC, 10);

	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoDive" );

	m_bDiving = true;
	m_bRollAfterDive = true;

#ifdef GAME_DLL
	m_pOuter->OnDive();
#endif

	ForceUnzoom();

	m_vecDiveDirection = m_pOuter->GetAbsVelocity();
	m_vecDiveDirection.GetForModify().z = 0;
	m_vecDiveDirection.GetForModify().NormalizeInPlace();

	m_pOuter->DoAnimationEvent(PLAYERANIMEVENT_DIVE);

	bool bWasOnGround = m_pOuter->GetFlags() & FL_ONGROUND;

	m_pOuter->SetGroundEntity(NULL);

	m_pOuter->Instructor_LessonLearned("dive");

	Assert (m_pOuter->m_Shared.m_flRunSpeed);
	float flSpeedFraction = RemapValClamped(m_pOuter->GetAbsVelocity().Length()/m_pOuter->m_Shared.m_flRunSpeed, 0, 1, 0.2f, 1);

	float flDiveHeight = sdk_dive_height.GetFloat();
	float y = m_pOuter->EyeAngles ().x;
	float arc = da_acro_dive_arc.GetFloat ();
	if (y > arc) flDiveHeight *= 0.33;
	else if (y < -arc) flDiveHeight *= 1.66;

	if (!bWasOnGround)
		flDiveHeight = sdk_dive_height_high.GetFloat();

	float flRatio = sdk_dive_height_adrenaline.GetFloat() / flDiveHeight;
	float flModifier = (flRatio - 1)/2;

	flDiveHeight = ModifySkillValue (flDiveHeight, flModifier, SKILL_ATHLETIC);

	flRatio = sdk_dive_gravity_adrenaline.GetFloat() / sdk_dive_gravity.GetFloat();
	flModifier = (flRatio - 1)/2;

	m_pOuter->SetGravity(ModifySkillValue(sdk_dive_gravity.GetFloat(), flModifier, SKILL_ATHLETIC));

	ConVarRef sdk_dive_speed("sdk_dive_speed");
	flRatio = sdk_dive_speed_adrenaline.GetFloat() / sdk_dive_speed.GetFloat();
	flModifier = (flRatio - 1)/2;

	return m_vecDiveDirection.Get() * (ModifySkillValue(sdk_dive_speed.GetFloat(), flModifier, SKILL_ATHLETIC) * flSpeedFraction) + Vector(0, 0, flDiveHeight);
}

void CSDKPlayerShared::StartSuperfallDiving()
{
	m_flDiveTime = m_pOuter->GetCurrentTime();
	m_flDiveLerped = 0;
	m_flDiveToProneLandTime = -1;

	m_bDiving = true;
	m_bRollAfterDive = true;

#ifdef GAME_DLL
	m_pOuter->OnDive();
#endif

	ForceUnzoom();

	m_vecDiveDirection = m_pOuter->GetAbsVelocity();
	m_vecDiveDirection.GetForModify().z = 0;
	m_vecDiveDirection.GetForModify().NormalizeInPlace();

	m_pOuter->DoAnimationEvent(PLAYERANIMEVENT_DIVE);

	m_pOuter->SetGroundEntity(NULL);
}

void CSDKPlayerShared::EndDive()
{
	m_flDiveTime = 0;

	m_pOuter->SetGravity(1);
	m_bDiving = false;

	m_pOuter->ReadyWeapon();
}

void CSDKPlayerShared::DiveLandedProne()
{
	SetProne(true, true);
	m_bProneSliding = true;
	m_flDisallowUnProneTime = m_pOuter->GetCurrentTime() + 0.4f;
	m_flDiveToProneLandTime = m_pOuter->GetCurrentTime();
}

void CSDKPlayerShared::StartManteling(const Vector& vecWallNormal)
{
	EndDive();
	EndWallFlip();

	m_pOuter->DoAnimationEvent (PLAYERANIMEVENT_WALLCLIMB);

	m_bIsManteling = true;
	m_vecMantelWallNormal = vecWallNormal;
}

void CSDKPlayerShared::ResetManteling()
{
	m_bIsManteling = false;
}

#if defined ( SDK_USE_SPRINTING )
void CSDKPlayerShared::SetSprinting( bool bSprinting )
{
	if ( bSprinting && !m_bIsSprinting )
	{
		StartSprinting();

		// only one penalty per key press
		if ( m_bGaveSprintPenalty == false )
		{
			m_flStamina -= INITIAL_SPRINT_STAMINA_PENALTY;
			m_bGaveSprintPenalty = true;
		}
	}
	else if ( !bSprinting && m_bIsSprinting )
	{
		StopSprinting();
	}
}

// this is reset when we let go of the sprint key
void CSDKPlayerShared::ResetSprintPenalty( void )
{
	m_bGaveSprintPenalty = false;
}

void CSDKPlayerShared::StartSprinting( void )
{
	m_bIsSprinting = true;
}

void CSDKPlayerShared::StopSprinting( void )
{
	m_bIsSprinting = false;
}
#endif

void CSDKPlayerShared::SetJumping( bool bJumping )
{
	m_bJumping = bJumping;
	
	if ( IsSniperZoomed() )
	{
		ForceUnzoom();
	}
}

ConVar da_acro_wallflip_delay ("da_acro_wallflip_delay", ".3", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

bool CSDKPlayerShared::IsWallFlipping(bool bExtend) const
{
	if (m_bIsWallFlipping)
		return m_bIsWallFlipping;

	if (!bExtend)
		return false;

	// The wall flip technically ends very quickly before the animation is done playing, so we have the option of extending it out a bit for some purposes.
	return m_pOuter->GetCurrentTime() < GetWallFlipEndTime() + 0.5f;
}

void CSDKPlayerShared::StartWallFlip(const Vector& vecWallNormal)
{
	m_bIsWallFlipping = true;
	m_flWallFlipEndTime = m_pOuter->GetCurrentTime() + da_acro_wallflip_delay.GetFloat();
	m_iWallFlipCount++;
	m_vecMantelWallNormal = vecWallNormal;

	m_pOuter->SetGravity(0.8f);

	m_pOuter->Instructor_LessonLearned("wallflip");

	m_pOuter->UseStyleCharge(SKILL_ATHLETIC, 10);
}

void CSDKPlayerShared::EndWallFlip()
{
	m_bIsWallFlipping = false;

	m_pOuter->SetGravity(1.0f);
}

ConVar da_superfall_time("da_superfall_time", "2.5", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

bool CSDKPlayerShared::IsSuperFalling()
{
	// Once he's determined in one life to be super-falling, he's super-falling until he super-falls to death.
	if (m_bSuperFalling)
		return true;

	if (m_pOuter->GetLocalVelocity().z > -100)
		return false;

	if (m_pOuter->GetCurrentTime() - m_flTimeLeftGround < da_superfall_time.GetFloat())
		return false;

	trace_t tr;
	UTIL_TraceLine(m_pOuter->GetAbsOrigin(), m_pOuter->GetAbsOrigin() + Vector(0, 0, -10000), MASK_PLAYERSOLID_BRUSHONLY, m_pOuter, COLLISION_GROUP_NONE, &tr);

	if ((tr.endpos - m_pOuter->GetAbsOrigin()).LengthSqr() < 2000*2000)
		return false;

	if (m_pOuter->GetAbsOrigin().z > SDKGameRules()->GetLowestSpawnPoint().z - 400)
		return false;

	m_bSuperFalling = true;
	return true;
}

ConVar da_superfall_calculate_time("da_superfall_calculate_time", "1", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

bool CSDKPlayerShared::CanSuperFallRespawn()
{
	if (!IsSuperFalling())
		return false;

	if (!m_pOuter->IsAlive())
		return false;

#ifndef CLIENT_DLL
	if (gpGlobals->curtime < m_flSuperFallOthersNextCheck)
		return !m_bSuperFallOthersVisible;

	m_bSuperFallOthersVisible = false;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer == m_pOuter)
			continue;

		if (m_pOuter->IsVisible(pPlayer))
		{
			m_bSuperFallOthersVisible = true;
			break;
		}
	}

	m_flSuperFallOthersNextCheck = gpGlobals->curtime + da_superfall_calculate_time.GetFloat();
#endif

	if (m_bSuperFallOthersVisible)
		return false;

	return true;
}

bool CSDKPlayerShared::IsAimedIn() const
{
	if (IsDiving() || IsRolling())
		return false;

	if (GetActiveSDKWeapon() && GetActiveSDKWeapon()->IsThrowingGrenade())
		return false;

	return m_bAimedIn;
}

void CSDKPlayerShared::SetAimIn(bool bAimIn)
{
	// If we're aimed in and aimin is being turned off and we're aimed in enough to have noticed a change, train the aimin lesson.
	if (m_bAimedIn && !bAimIn && m_flAimIn > 0.5f)
	{
		CWeaponSDKBase* pWeapon = m_pOuter->GetActiveSDKWeapon();

		// Also must be holding an aimin weapon.
		if (pWeapon && (pWeapon->HasAimInFireRateBonus() || pWeapon->HasAimInRecoilBonus() || pWeapon->HasAimInSpeedPenalty()))
			m_pOuter->Instructor_LessonLearned("aimin");
	}

	m_bAimedIn = bAimIn;
}

ConVar da_slowaimin_speedin("da_slowaimin_speedin", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
ConVar da_slowaimin_speedout("da_slowaimin_speedout", "10", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

void CSDKPlayerShared::RampSlowAimIn(float flGoal)
{
	if (flGoal > 0)
		m_flSlowAimIn = Approach(flGoal, m_flSlowAimIn, da_slowaimin_speedin.GetFloat()*gpGlobals->frametime);
	else
		m_flSlowAimIn = Approach(flGoal, m_flSlowAimIn, da_slowaimin_speedout.GetFloat()*gpGlobals->frametime);
}

ConVar da_recoildecay("da_recoildecay", "250", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

Vector CSDKPlayerShared::GetRecoil(float flFrameTime)
{
	if (m_flRecoilAccumulator <= 0)
		return Vector(0, 0, 0);

	float flRecoil = m_flRecoilAccumulator*flFrameTime;
	m_flRecoilAccumulator = Approach(0, m_flRecoilAccumulator, flFrameTime * da_recoildecay.GetFloat());
	return m_vecRecoilDirection * flRecoil;
}

ConVar da_recoilmultiplier("da_recoilmultiplier", "3", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

void CSDKPlayerShared::SetRecoil(float flRecoil)
{
	m_flRecoilAccumulator = flRecoil * da_recoilmultiplier.GetFloat();
	m_vecRecoilDirection.y = 1;
	m_vecRecoilDirection.x = SharedRandomFloat( "Recoil", -0.5f, 0.5f );
}

void CSDKPlayerShared::PlayerOnGround( void )
{
	m_flTimeLeftGround = m_pOuter->GetCurrentTime();
	m_iWallFlipCount = 0;
	m_flSuperFallOthersNextCheck = 0;
	m_pOuter->DeactivateSuperfall();
}

void CSDKPlayerShared::ForceUnzoom( void )
{
//	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
//	if( pWeapon && ( pWeapon->GetSDKWpnData().m_WeaponType & WPN_MASK_GUN ) )
//	{
//		CSDKSniperWeapon *pSniper = dynamic_cast<CSDKSniperWeapon *>(pWeapon);
//
//		if ( pSniper )
//		{
//			pSniper->ZoomOut();
//		}
//	}
}

bool CSDKPlayerShared::IsSniperZoomed( void ) const
{
//	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
//	if( pWeapon && ( pWeapon->GetSDKWpnData().m_WeaponType & WPN_MASK_GUN ) )
//	{
//		CWeaponSDKBaseGun *pGun = (CWeaponSDKBaseGun *)pWeapon;
//		Assert( pGun );
//		return pGun->IsSniperZoomed();
//	}

	return false;
}

#if defined ( SDK_USE_PLAYERCLASSES )
void CSDKPlayerShared::SetDesiredPlayerClass( int playerclass )
{
	m_iDesiredPlayerClass = playerclass;
}

int CSDKPlayerShared::DesiredPlayerClass( void )
{
	return m_iDesiredPlayerClass;
}

void CSDKPlayerShared::SetPlayerClass( int playerclass )
{
	m_iPlayerClass = playerclass;
}

int CSDKPlayerShared::PlayerClass( void )
{
	return m_iPlayerClass;
}
#endif

#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
void CSDKPlayerShared::SetStamina( float flStamina )
{
	m_flStamina = clamp( flStamina, 0, 100 );
}
#endif
CWeaponSDKBase* CSDKPlayerShared::GetActiveSDKWeapon() const
{
	CBaseCombatWeapon *pWeapon = m_pOuter->GetActiveWeapon();
	if ( pWeapon )
	{
		Assert( dynamic_cast< CWeaponSDKBase* >( pWeapon ) == static_cast< CWeaponSDKBase* >( pWeapon ) );
		return static_cast< CWeaponSDKBase* >( pWeapon );
	}
	else
	{
		return NULL;
	}
}

void CSDKPlayerShared::ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	Vector org = m_pOuter->GetAbsOrigin();

#ifdef SDK_USE_PRONE
	if ( IsProne() )
	{
		static Vector vecProneMin(-44, -44, 0 );
		static Vector vecProneMax(44, 44, 24 );

		VectorAdd( vecProneMin, org, *pVecWorldMins );
		VectorAdd( vecProneMax, org, *pVecWorldMaxs );
	}
	else
#endif
	{
		static Vector vecMin(-32, -32, 0 );
		static Vector vecMax(32, 32, 72 );

		VectorAdd( vecMin, org, *pVecWorldMins );
		VectorAdd( vecMax, org, *pVecWorldMaxs );
	}
}

ConVar da_speed_run("da_speed_run", "210", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar da_speed_prone("da_speed_prone", "100", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar da_speed_slide("da_speed_slide", "320", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar da_speed_roll("da_speed_roll", "250", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar da_speed_aimin("da_speed_aimin", "100", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CSDKPlayer::InitSpeeds()
{
	m_Shared.m_flRunSpeed = da_speed_run.GetFloat();
	m_Shared.m_flSprintSpeed = 330; // Unused
	m_Shared.m_flProneSpeed = da_speed_prone.GetFloat();
	m_Shared.m_flSlideSpeed = da_speed_slide.GetFloat();
	m_Shared.m_flRollSpeed = da_speed_roll.GetFloat();
	m_Shared.m_flAimInSpeed = da_speed_aimin.GetFloat();

	// Set the absolute max to sprint speed (but we don't use sprint so now it's runspeed)
	SetMaxSpeed( GetPlayerMaxSpeed(false) );
}

float CSDKPlayer::GetPlayerMaxSpeed(bool bDucking)
{
	float flMaxSpeed;

	// This check is now simplified, just use CanChangePosition because it checks the two things we need to check anyway.
	if ( m_Shared.IsProne() && m_Shared.CanChangePosition() && GetGroundEntity() != NULL )
	{
		if (m_Shared.m_bProneSliding)
			flMaxSpeed = m_Shared.ModifySkillValue(m_Shared.m_flSlideSpeed, 0.5f, SKILL_ATHLETIC);
		else
			flMaxSpeed = m_Shared.ModifySkillValue(m_Shared.m_flProneSpeed, 0.5f, SKILL_ATHLETIC);
	}
	//not prone - standing or crouching and possibly moving
	else if ( (m_Shared.IsSliding() && !m_Shared.IsGettingUpFromSlide()) && GetGroundEntity() )
		flMaxSpeed = GetMaxSlideSpeed();
	else if ( m_Shared.IsRolling() && GetGroundEntity() )
		flMaxSpeed = m_Shared.ModifySkillValue(m_Shared.m_flRollSpeed, 0.25f, SKILL_ATHLETIC);
	else if ( m_Shared.IsDiving() && !GetGroundEntity() )
	{
		ConVarRef sdk_dive_speed_adrenaline("sdk_dive_speed_adrenaline");
		ConVarRef sdk_dive_speed("sdk_dive_speed");

		float flSpeedRatio = sdk_dive_speed_adrenaline.GetFloat() / sdk_dive_speed.GetFloat();
		flSpeedRatio -= 1; // 0 means unchanged.
		flSpeedRatio /= 2; // It gets doubled when the skill is on.

		flMaxSpeed = m_Shared.ModifySkillValue(sdk_dive_speed.GetFloat(), flSpeedRatio, SKILL_ATHLETIC);
	}
	else
	{
		if ( bDucking )
			flMaxSpeed = m_Shared.m_flRunSpeed;	//gets cut in fraction later
		else
		{
			if ( m_Shared.IsAimedIn() )
				flMaxSpeed = RemapValClamped(m_Shared.GetAimIn(), 0, 1, m_Shared.m_flRunSpeed, m_Shared.m_flAimInSpeed);
			else
				flMaxSpeed = m_Shared.m_flRunSpeed;
		}

		flMaxSpeed = m_Shared.ModifySkillValue(flMaxSpeed, 0.2f, SKILL_ATHLETIC);
	}

	return flMaxSpeed;
}

ConVar da_athletic_slide_boost("da_athletic_slide_boost", ".25", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

float CSDKPlayer::GetMaxSlideSpeed()
{
	return m_Shared.ModifySkillValue(m_Shared.m_flSlideSpeed, da_athletic_slide_boost.GetFloat(), SKILL_ATHLETIC);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayer::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	// Only check these when using teams, otherwise it's normal!
#if defined ( SDK_USE_TEAMS )
	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup == COLLISION_GROUP_PROJECTILE )
	{
		switch( GetTeamNumber() )
		{
		case SDK_TEAM_BLUE:
			if ( !( contentsMask & CONTENTS_TEAM2 ) )
				return false;
			break;

		case SDK_TEAM_RED:
			if ( !( contentsMask & CONTENTS_TEAM1 ) )
				return false;
			break;
		}
	}
#endif
	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

void CSDKPlayer::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	if (m_Shared.IsRolling() || m_Shared.IsSliding() || m_Shared.IsDucking() || m_Shared.IsProne())
		return;

	BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
}

bool CSDKPlayer::CanAddToLoadout(SDKWeaponID eWeapon)
{
	if (eWeapon <= WEAPON_NONE)
		return false;

	if (eWeapon >= WEAPON_MAX)
		return false;

	CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eWeapon);

	// Don't allow buying the akimbo version. Must buy the single version twice.
	if (*pWeaponInfo->m_szSingle)
		return false;

	if (pWeaponInfo->iWeight + m_iLoadoutWeight > MAX_LOADOUT_WEIGHT)
		return false;

	if (pWeaponInfo->iMaxClip1 > 0)
	{
		if (*pWeaponInfo->m_szAkimbo)
		{
			// If this weapon has an akimbo version, allow buying two of them.
			if (m_aLoadout[eWeapon].m_iCount >= 2)
				return false;
		}
		else if (m_aLoadout[eWeapon].m_iCount)
			return false;
	}

	if (pWeaponInfo->m_eWeaponType == WT_GRENADE && m_aLoadout[eWeapon].m_iCount)
		return false;

	return true;
}

int CSDKPlayer::GetLoadoutWeaponCount(SDKWeaponID eWeapon)
{
	return m_aLoadout[eWeapon].m_iCount;
}

void CSDKPlayer::GetStepSoundVelocities( float *velwalk, float *velrun )
{
	BaseClass::GetStepSoundVelocities(velwalk, velrun);

	if (!( ( GetFlags() & FL_DUCKING) || ( GetMoveType() == MOVETYPE_LADDER ) ))
		*velwalk = 110;
}

float CSDKPlayer::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	return BaseClass::GetSequenceCycleRate( pStudioHdr, iSequence ) * GetSlowMoMultiplier();
}

void CSDKPlayer::ActivateSlowMo()
{
	if (!m_flSlowMoSeconds)
		return;

#ifdef GAME_DLL
	if (m_bHasSuperSlowMo || m_flSlowMoSeconds >= 3)
		CDove::SpawnDoves(this);
#endif

	m_flSlowMoTime = gpGlobals->curtime + m_flSlowMoSeconds + 0.5f;    // 1 second becomes 1.5 seconds, 2 becomes 2.5, etc
	m_flSlowMoSeconds = 0;
	m_iSlowMoType = m_bHasSuperSlowMo?SLOWMO_STYLESKILL:SLOWMO_ACTIVATED;

	if (m_Shared.m_bSuperSkill)
		m_iSlowMoType = SLOWMO_STYLESKILL;

#ifdef GAME_DLL
	SDKGameRules()->PlayerSlowMoUpdate(this);
#endif

	ReadyWeapon();
}

void CSDKPlayer::ActivateSuperfallSlowMo()
{
	if (!m_Shared.IsSuperFalling())
		return;

	m_flSlowMoTime = gpGlobals->curtime + 15;
	m_iSlowMoType = SLOWMO_SUPERFALL;

	ReadyWeapon();
}

void CSDKPlayer::DeactivateSlowMo()
{
	if (m_flSlowMoTime == 0)
		return;

	if (m_iSlowMoType != SLOWMO_SUPERFALL)
	{
		m_flSlowMoSeconds += (int)(m_flSlowMoTime - gpGlobals->curtime - 0.5f);
		if (m_flSlowMoSeconds < 0)
			m_flSlowMoSeconds = 0;
	}

	if (m_flSlowMoSeconds == 0)
		m_bHasSuperSlowMo = m_Shared.m_bSuperSkill;

	m_flSlowMoTime = 0;
	m_iSlowMoType = SLOWMO_NONE;

#ifdef GAME_DLL
	SDKGameRules()->PlayerSlowMoUpdate(this);
#endif
}

void CSDKPlayer::DeactivateSuperfall()
{
	m_Shared.m_bSuperFalling = false;

	if (GetSlowMoType() == SLOWMO_SUPERFALL) {
		DeactivateSlowMo();
	}
}

float CSDKPlayer::GetSlowMoMultiplier() const
{
	return m_flSlowMoMultiplier * da_globalslow.GetFloat();
}

float CSDKPlayer::GetSlowMoGoal() const
{
	if (m_iSlowMoType == SLOWMO_STYLESKILL)
		return 0.7f;
	else if (m_iSlowMoType == SLOWMO_ACTIVATED)
		return 0.65f;
	else if (m_iSlowMoType == SLOWMO_SUPERFALL)
		return 0.65f;
	else if (m_iSlowMoType == SLOWMO_PASSIVE)
		return 0.45f;
	else if (m_iSlowMoType == SLOWMO_PASSIVE_SUPER)
		return 0.35f;
	else //if (m_iSlowMoType == SLOWMO_NONE)
		return 1;
}

void CSDKPlayer::UpdateCurrentTime()
{
	m_flCurrentTime += gpGlobals->frametime * GetSlowMoMultiplier();

	m_flSlowMoMultiplier = Approach(GetSlowMoGoal(), m_flSlowMoMultiplier, gpGlobals->frametime/0.5f);

	if (m_flSlowMoTime > 0 && gpGlobals->curtime > m_flSlowMoTime)
	{
		m_flSlowMoTime = 0;
		m_iSlowMoType = SLOWMO_NONE;

#ifdef GAME_DLL
		SDKGameRules()->PlayerSlowMoUpdate(this);
#endif

		m_bHasSuperSlowMo = false;
	}

	UpdateViewBobRamp();
}

void CSDKPlayer::UpdateViewBobRamp()
{
#ifdef CLIENT_DLL
	// It's not filled in for remote clients, so force the local one since it's the same.
	float flMaxBobSpeed = C_SDKPlayer::GetLocalSDKPlayer()->m_Shared.m_flRunSpeed*0.7f;
#else
	float flMaxBobSpeed = m_Shared.m_flRunSpeed*0.7f;
#endif

	float flBobRampGoal = RemapValClamped(GetLocalVelocity().LengthSqr(), 0, flMaxBobSpeed*flMaxBobSpeed, 0, 1);
	if (!(GetFlags() & FL_ONGROUND) || m_Shared.IsRolling() || m_Shared.IsSliding())
		flBobRampGoal = 0;

	m_Shared.m_flViewBobRamp = Approach(flBobRampGoal, m_Shared.m_flViewBobRamp, gpGlobals->frametime*m_flSlowMoMultiplier*4);
}

ConVar  da_cam_fade_distance("sdk_cam_fade_distance", "30", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
ConVar  da_cam_fade_distance_aimin("da_cam_fade_distance_aimin", "25", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
ConVar	da_cam_fade_alpha_val("sdk_cam_fade_alpha_val", "100", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);

void CSDKPlayer::UpdateThirdCamera(const Vector& vecEye, const QAngle& angEye)
{
	if (!IsInThirdPerson())
		return;

	CWeaponSDKBase * pWeapon = NULL;
	if (GetActiveWeapon() != NULL){
		pWeapon = GetActiveSDKWeapon();
	}

	Assert(pWeapon);

#ifdef CLIENT_DLL

	float flFadeDistance = RemapVal(m_Shared.GetAimIn(), 0, 1, da_cam_fade_distance.GetFloat(), da_cam_fade_distance_aimin.GetFloat());

	if (m_vecThirdCamera.DistTo(vecEye) < flFadeDistance)
	{
		m_flCurrentAlphaVal = Approach(da_cam_fade_alpha_val.GetFloat(), m_flCurrentAlphaVal, 500.0f * gpGlobals->frametime);

		if (GetRenderMode() != kRenderTransTexture){
			SetRenderMode(kRenderTransTexture);
		}

		SetRenderColorA(m_flCurrentAlphaVal);

		if (pWeapon){
			if (pWeapon->GetRenderMode() != kRenderTransTexture){
				pWeapon->SetRenderMode(kRenderTransTexture);
			}
			pWeapon->SetRenderColorA(m_flCurrentAlphaVal);
		}
	}
	else
	{
		m_flCurrentAlphaVal = Approach(255.0f, m_flCurrentAlphaVal, 500.0f * gpGlobals->frametime);

		if (GetRenderMode() != kRenderNormal){
			SetRenderMode(kRenderNormal);
		}

		SetRenderColorA(m_flCurrentAlphaVal);

		if (pWeapon){
			if (pWeapon->GetRenderMode() != kRenderNormal){
				pWeapon->SetRenderMode(kRenderNormal);
			}
			pWeapon->SetRenderColorA(m_flCurrentAlphaVal);
		}
	}

#endif

	m_vecThirdCamera = CalculateThirdPersonCameraPosition(vecEye, angEye);

	Vector vecShoot;
	AngleVectors(angEye, &vecShoot);

	// Trace to see where the camera is pointing
	trace_t tr;
	UTIL_TraceLine( m_vecThirdCamera, m_vecThirdCamera + vecShoot * 99999, MASK_SHOT & ~(CONTENTS_WINDOW), this, COLLISION_GROUP_NONE, &tr );

	m_vecThirdTarget = tr.endpos;
}

ConVar da_viewbob( "da_viewbob", "2.5", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "View bob magnitude." );

Vector CSDKPlayer::EyePosition()
{
	Vector vecPosition = BaseClass::EyePosition();

	Assert(vecPosition.IsValid());

	bool bIsInThird = false;

#ifdef CLIENT_DLL
	bIsInThird = ::input->CAM_IsThirdPerson();

	if (C_SDKPlayer::GetLocalOrSpectatedPlayer() ==  this && C_SDKPlayer::GetLocalSDKPlayer() != C_SDKPlayer::GetLocalOrSpectatedPlayer())
		bIsInThird = false;
#endif

	if (m_Shared.m_flViewBobRamp && m_Shared.m_flRunSpeed && !bIsInThird)
	{
		Vector vecRight, vecUp;
		AngleVectors(EyeAngles(), NULL, &vecRight, &vecUp);

		float flViewBobMagnitude = m_Shared.m_flViewBobRamp * da_viewbob.GetFloat();

		float flRunPeriod = M_PI * 3;
		float flRunUpBob = sin(GetCurrentTime() * flRunPeriod * 2) * (flViewBobMagnitude / 2);
		float flRunRightBob = sin(GetCurrentTime() * flRunPeriod) * flViewBobMagnitude;

		float flWalkPeriod = M_PI * 1.5f;
		float flWalkUpBob = sin(GetCurrentTime() * flWalkPeriod * 2) * (flViewBobMagnitude / 2);
		float flWalkRightBob = sin(GetCurrentTime() * flWalkPeriod) * flViewBobMagnitude;

#ifdef CLIENT_DLL
		// It's not filled in for remote clients, so force the local one since it's the same.
		float flSpeedRatio = C_SDKPlayer::GetLocalSDKPlayer()->m_Shared.m_flAimInSpeed/C_SDKPlayer::GetLocalSDKPlayer()->m_Shared.m_flRunSpeed;
#else
		float flSpeedRatio = m_Shared.m_flAimInSpeed/m_Shared.m_flRunSpeed;
#endif

		// 0 is walk, 1 is run.
		float flRunRamp = RemapValClamped(m_Shared.m_flViewBobRamp, flSpeedRatio, 1.0f, 0.0f, 1.0f);

		float flRightBob = RemapValClamped(flRunRamp, 0, 1, flWalkRightBob, flRunRightBob);
		float flUpBob = RemapValClamped(flRunRamp, 0, 1, flWalkUpBob, flRunUpBob);

		vecPosition += vecRight * flRightBob + vecUp * flUpBob;
	}

	Assert(vecPosition.IsValid());

	return vecPosition;
}

bool CSDKPlayer::Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon )
{
	if (IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)this;
#if !defined( CLIENT_DLL )
		IServerVehicle *pVehicle = pPlayer->GetVehicle();
#else
		IClientVehicle *pVehicle = pPlayer->GetVehicle();
#endif
		if (pVehicle && !pPlayer->UsingStandardWeaponsInVehicle())
			return false;
	}

	if ( !pWeapon->CanDeploy() )
		return false;
	
	if ( GetActiveWeapon() )
	{
		if ( !GetActiveWeapon()->CanHolster() )
			return false;
	}

	return true;
}

CBaseCombatWeapon* CSDKPlayer::GetLastWeapon()
{
	// This is pretty silly, but I'd rather mess around with stock Valve code as little as possible.
#ifdef CLIENT_DLL
	CBaseCombatWeapon* pLastWeapon = BaseClass::GetLastWeapon();
#else
	CBaseCombatWeapon* pLastWeapon = BaseClass::Weapon_GetLast();
#endif

	if (pLastWeapon && pLastWeapon != GetActiveWeapon())
		return pLastWeapon;

	CWeaponSDKBase* pHeaviest = NULL;
	CWeaponSDKBase* pBrawl = NULL;
	for (int i = 0; i < WeaponCount(); i++)
	{
		if (!GetWeapon(i))
			continue;

		if (GetWeapon(i) == GetActiveWeapon())
			continue;

		CWeaponSDKBase* pSDKWeapon = dynamic_cast<CWeaponSDKBase*>(GetWeapon(i));
		if (!pSDKWeapon)
			continue;

		if (pSDKWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
		{
			pBrawl = pSDKWeapon;
			continue;
		}

		if (!pHeaviest)
		{
			pHeaviest = pSDKWeapon;
			continue;
		}

		if (pHeaviest->GetWeight() < pSDKWeapon->GetWeight())
			pHeaviest = pSDKWeapon;
	}

	if (!pHeaviest)
		pHeaviest = pBrawl;

	return pHeaviest;
}

#define CAM_HULL_OFFSET 9.0    // the size of the bounding hull used for collision checking

void CSDKPlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	BaseClass::CalcView(eyeOrigin, eyeAngles, zNear, zFar, fov);

	if (m_Shared.IsWallFlipping(true))
	{
		// If the player is wall flipping fix up their eye origin to make sure it never clips into the ceiling.
		trace_t trace;

		CTraceFilterSimple traceFilter( this, COLLISION_GROUP_NONE );
		UTIL_TraceHull( GetAbsOrigin(), EyePosition(),
			Vector(-CAM_HULL_OFFSET, -CAM_HULL_OFFSET, -CAM_HULL_OFFSET), Vector(CAM_HULL_OFFSET, CAM_HULL_OFFSET, CAM_HULL_OFFSET),
			CONTENTS_SOLID, &traceFilter, &trace );

		eyeOrigin = trace.endpos;
	}
}

bool CSDKPlayer::IsInThirdPerson() const
{
	if (!IsAlive())
		return false;

	return m_bThirdPerson;
}

ConVar da_cam_forward_lerp( "da_cam_forward_lerp", "4", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Speed of camera when lerping backwards." );
ConVar da_cam_back_lerp( "da_cam_back_lerp", "0.2", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Speed of camera when lerping forward (blocked by something)" );
ConVar da_cam_max_fastlerp_distance( "da_cam_max_fastlerp_distance", "30", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Use the slow lerp when farther than this distance." );

ConVar da_cam_stunt_up( "da_cam_stunt_up", "20", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Height to raise the camera during stunts." );

ConVar da_cam_lag_velocity( "da_cam_lag_velocity", ".8", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much player velocity will affect the camera lag." );
ConVar da_cam_lag_drag( "da_cam_lag_drag", "12", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How fast the lagged part of the camera will return to center." );
ConVar da_cam_lag_max( "da_cam_lag_max", "17", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Maximum camera lag distance." );

const Vector CSDKPlayer::CalculateThirdPersonCameraPosition(const Vector& vecEye, const QAngle& angCamera)
{
	float flCamBackIdle = GetUserInfoFloat("da_cam_back");
	float flCamUpIdle = GetUserInfoFloat("da_cam_up");
	float flCamRightIdle = GetUserInfoFloat("da_cam_right");

	float flCamBackAim = GetUserInfoFloat("da_cam_back_aim");
	float flCamUpAim = GetUserInfoFloat("da_cam_up_aim");
	float flCamRightAim = GetUserInfoFloat("da_cam_right_aim");

	if (m_bUsingVR)
	{
		flCamBackIdle = GetUserInfoFloat("da_cam_back_vr");
		flCamUpIdle = GetUserInfoFloat("da_cam_up_vr");
		flCamRightIdle = GetUserInfoFloat("da_cam_right_vr");

		flCamBackAim = GetUserInfoFloat("da_cam_back_aim_vr");
		flCamUpAim = GetUserInfoFloat("da_cam_up_aim_vr");
		flCamRightAim = GetUserInfoFloat("da_cam_right_aim_vr");
	}

	float flCamStandingScale = GetUserInfoFloat("da_cam_standing_back_mult");

	float flCamBack = flCamBackIdle;
	float flCamUp = RemapValClamped(Gain(m_Shared.GetAimIn(), 0.8f), 0, 1, flCamUpIdle, flCamUpAim);
	float flCamRight = RemapValClamped(Gain(m_Shared.GetAimIn(), 0.8f), 0, 1, flCamRightIdle, flCamRightAim);

	float flCamLagVelocity = da_cam_lag_velocity.GetFloat();
	float flCamLagDrag = da_cam_lag_drag.GetFloat();
	float flCamLagMax = da_cam_lag_max.GetFloat();

	if (m_bUsingVR)
	{
		flCamStandingScale = 1;
		flCamLagVelocity = 0;
		flCamLagDrag = 0;
		flCamLagMax = 0;
	}

	m_vecCameraLag -= GetAbsVelocity() * (GetSlowMoMultiplier() * gpGlobals->frametime * flCamLagVelocity);

	m_vecCameraLag *= (1 - GetSlowMoMultiplier() * gpGlobals->frametime * flCamLagDrag);

	if (m_vecCameraLag.Length() > flCamLagMax)
		m_vecCameraLag = m_vecCameraLag.Normalized() * flCamLagMax;

	m_flSideLerp = Approach(m_bThirdPersonCamSide?1:-1, m_flSideLerp, gpGlobals->frametime*15);
	flCamRight *= m_flSideLerp;

	QAngle angFixedCamera = angCamera;

	if (m_bUsingVR)
		// Slam pitch to 0.
		angFixedCamera.x = 0;

	Vector camForward, camRight, camUp;
	AngleVectors( angFixedCamera, &camForward, &camRight, &camUp );

	Vector vecCameraOffset = -camForward*flCamBack + camRight*flCamRight + camUp*flCamUp;

	vecCameraOffset += m_vecCameraLag * Gain(1-m_Shared.GetAimIn(), 0.8f);

	m_flStuntLerp = Approach((m_Shared.IsDiving()||m_Shared.IsRolling())?1:0, m_flStuntLerp, gpGlobals->frametime*2);

	if (m_flStuntLerp)
		vecCameraOffset += camUp * (m_flStuntLerp * da_cam_stunt_up.GetFloat());

	Vector vecNewOrigin = vecEye + vecCameraOffset;

	// Start the trace here instead of at the eye so that if something blocks it
	// the camera doesn't rush up to the back of the player's head.
	Vector vecCameraOffsetStart = vecEye + camRight*(m_bThirdPersonCamSide?1:-1)*5 + camUp*5;

	trace_t trace;

	CTraceFilterSimple traceFilter( this, COLLISION_GROUP_NONE );
	UTIL_TraceHull( vecEye, vecCameraOffsetStart,
		Vector(-CAM_HULL_OFFSET, -CAM_HULL_OFFSET, -CAM_HULL_OFFSET), Vector(CAM_HULL_OFFSET, CAM_HULL_OFFSET, CAM_HULL_OFFSET),
		MASK_VISIBLE|CONTENTS_GRATE, &traceFilter, &trace );

	if (trace.fraction < 1)
		return vecEye * trace.fraction + vecCameraOffsetStart * (1 - trace.fraction);

	UTIL_TraceHull( vecCameraOffsetStart, vecNewOrigin,
		Vector(-CAM_HULL_OFFSET, -CAM_HULL_OFFSET, -CAM_HULL_OFFSET), Vector(CAM_HULL_OFFSET, CAM_HULL_OFFSET, CAM_HULL_OFFSET),
		MASK_VISIBLE|CONTENTS_GRATE, &traceFilter, &trace );

	float flFraction = trace.fraction;
	if (GetLocalVelocity().Length2DSqr() < 10*10)
		flFraction = min(trace.fraction, flCamStandingScale);

	float flLerpScale = (fabs(flFraction - m_flCameraLerp) + 0.1f)*5.0f;
	if (vecCameraOffset.Length() * m_flCameraLerp < da_cam_max_fastlerp_distance.GetFloat() || trace.fraction < m_flCameraLerp)
		m_flCameraLerp = Approach(flFraction, m_flCameraLerp, da_cam_forward_lerp.GetFloat()*gpGlobals->frametime);
	else
		m_flCameraLerp = Approach(flFraction, m_flCameraLerp, da_cam_back_lerp.GetFloat()*gpGlobals->frametime*flLerpScale);

	if (m_Shared.GetAimIn() > 0)
	{
		float flCap = RemapValClamped(m_Shared.GetAimIn(), 0, 1, 1, flCamBackAim/flCamBackIdle);
		if (flCap < m_flCameraLerp)
			m_flCameraLerp = flCap;
	}

	return vecCameraOffsetStart + vecCameraOffset * m_flCameraLerp;
}

const Vector CSDKPlayer::GetThirdPersonCameraPosition()
{
	return m_vecThirdCamera;
}

const Vector CSDKPlayer::GetThirdPersonCameraTarget()
{
	return m_vecThirdTarget;
}

float CSDKPlayerShared::ModifySkillValue(float flValue, float flModify, SkillID eSkill) const
{
	if (!m_bSuperSkill && SDKGameRules()->GetBountyPlayer() != m_pOuter)
	{
		if (m_iStyleSkill != eSkill)
			return flValue;
	}

	ConVarRef da_stylemeteractivationcost("da_stylemeteractivationcost");
	float flMultiplier = RemapValClamped(m_pOuter->GetStylePoints(), 0, da_stylemeteractivationcost.GetFloat(), 1, 2);

	if (m_pOuter->IsStyleSkillActive())
		flMultiplier = 2;

	flModify *= flMultiplier;

	return flValue * (flModify+1);
}

CWeaponSDKBase* CSDKPlayer::FindWeapon (SDKWeaponID id)
{
	int i;
	for (i = 0; i < WeaponCount(); i++)
	{
		CWeaponSDKBase *wpn = (CWeaponSDKBase *)GetWeapon (i);
		if (wpn)
		{
			if (wpn->GetWeaponID () == id)
				return wpn;
		}
	}
	return NULL;
}

bool CSDKPlayer::HasBriefcase() const
{
	return !!m_hBriefcase;
}

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

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

#include "decals.h"
#include "util_shared.h"

#ifdef CLIENT_DLL
	
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"
	#include "prediction.h"
	#include "clientmode_sdk.h"
	#include "vgui_controls/AnimationController.h"

	#define CRecipientFilter C_RecipientFilter
#else
	#include "sdk_player.h"
	#include "sdk_team.h"
#endif

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client (red) and server (blue) bullet impact point" );

void DispatchEffect( const char *pName, const CEffectData &data );

void CSDKPlayer::FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   )
{
	float fCurrentDamage = iDamage;   // damage of the bullet at it's current trajectory
	float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far

	if (IsActionAbilityActive())
		fCurrentDamage *= 1.4f;

	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

	if ( !pevAttacker )
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize( vecDir );

	float flMaxRange = 8000;

	Vector vecEnd = vecSrc + vecDir * flMaxRange; // max bullet range is 10000 units

	trace_t tr; // main enter bullet trace

	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction == 1.0f )
		return; // we didn't hit anything, stop tracing shoot

	if ( sv_showimpacts.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawClientHitboxes( 4, true );
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawServerHitboxes( 4, true );
		}
#endif
	}

		//calculate the damage based on the distance the bullet travelled.
		flCurrentDistance += tr.fraction * flMaxRange;

		// damage get weaker of distance
		fCurrentDamage *= pow ( 0.85f, (flCurrentDistance / 500));

		int iDamageType = DMG_BULLET | DMG_NEVERGIB | GetAmmoDef()->DamageType(iBulletType);

		if (iDamageType & DMG_BUCKSHOT)
			fCurrentDamage *= RemapValClamped(flCurrentDistance, 500, 1000, 1.0f, 0.2f);

		if( bDoEffects )
		{
			// See if the bullet ended up underwater + started out of the water
			if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
			{	
				trace_t waterTrace;
				UTIL_TraceLine( vecSrc, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace );

				if( waterTrace.allsolid != 1 )
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat( 8, 12 );

					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}

					DispatchEffect( "gunshotsplash", data );
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					//Tony; only while using teams do we check for friendly fire.
					if ( pEntity && pEntity->IsPlayer() && (pEntity->GetBaseAnimating() && !pEntity->GetBaseAnimating()->IsRagdoll()) )
					{
#if defined ( SDK_USE_TEAMS )
						if ( pEntity->GetTeamNumber() == GetTeamNumber() )
						{
							if ( !friendlyfire.GetBool() )
								UTIL_ImpactTrace( &tr, iDamageType );
						}
#else
						UTIL_ImpactTrace( &tr, iDamageType );
#endif
					}
					//Tony; non player, just go nuts,
					else
					{
						UTIL_ImpactTrace( &tr, iDamageType );
					}
				}
			}
		} // bDoEffects

		// add damage to entity that we hit

#ifdef GAME_DLL
		ClearMultiDamage();

		CTakeDamageInfo info( pevAttacker, pevAttacker, fCurrentDamage, iDamageType );
		CalculateBulletDamageForce( &info, iBulletType, vecDir, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( info, vecDir, &tr );

		TraceAttackToTriggers( info, tr.startpos, tr.endpos, vecDir );

		ApplyMultiDamage();
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

	m_Shared.SetJumping( false );

	m_Shared.m_flViewTilt = 0;
	m_Shared.m_flLastDuckPress = -1;
	m_Shared.m_bDiving = false;
	m_Shared.m_bRolling = false;
	m_Shared.m_bSliding = false;
	m_Shared.m_bProne = false;
	m_Shared.m_bAimedIn = false;

	//Tony; todo; fix

//	m_flMinNextStepSoundTime = gpGlobals->curtime;
#if defined ( SDK_USE_PRONE )
//	m_bPlayingProneMoveSound = false;
#endif // SDK_USE_PRONE
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

#if !defined ( SDK_SHOOT_ON_LADDERS )
	if ( GetMoveType() == MOVETYPE_LADDER )
		return false;
#endif //SDK_SHOOT_ON_LADDERS

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
			return VEC_SLIDE_HULL_MIN;
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
			return VEC_SLIDE_HULL_MAX;
		else
			return VEC_HULL_MAX;
	}
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
	m_bRolling = false;
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

	if ( bNoAnimation )
	{
		m_flGoProneTime = 0;
		m_flUnProneTime = 0;
	}

	if ( !bProne /*&& IsSniperZoomed()*/ )	// forceunzoom for going prone is in StartGoingProne
	{
		ForceUnzoom();
	}
}

void CSDKPlayerShared::StartGoingProne( void )
{
	// make the prone sound
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoProne" );

	// slow to prone speed
	m_flGoProneTime = gpGlobals->curtime + TIME_TO_PRONE;

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
	m_flUnProneTime = gpGlobals->curtime + TIME_TO_PRONE;

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
	return ( m_flUnSlideTime > 0 );
}

bool CSDKPlayerShared::IsSliding() const
{
	return m_bSliding;
}

bool CSDKPlayerShared::CanSlide() const
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

	return true;
}

void CSDKPlayerShared::StartSliding()
{
	if (!CanSlide())
		return;

	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoSlide" );

	m_bSliding = true;

	ForceUnzoom();

	m_vecSlideDirection = m_pOuter->GetAbsVelocity();
	m_vecSlideDirection.GetForModify().NormalizeInPlace();

	m_flSlideTime = gpGlobals->curtime;
	m_flUnSlideTime = 0;
}

void CSDKPlayerShared::EndSlide()
{
	m_bSliding = false;
	m_flSlideTime = 0;
}

void CSDKPlayerShared::StandUpFromSlide( void )
{	
	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.UnSlide" );

	m_flUnSlideTime = gpGlobals->curtime + TIME_TO_UNSLIDE;

	m_vecUnSlideEyeStartOffset = m_pOuter->GetViewOffset();
}

ConVar sdk_slidetime("sdk_slidetime", "1", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

float CSDKPlayerShared::GetSlideFriction() const
{
	if (!m_bSliding)
		return 1;

	if (gpGlobals->curtime - m_flSlideTime < sdk_slidetime.GetFloat())
		return 0.1f;

	return RemapValClamped(gpGlobals->curtime, m_flSlideTime + sdk_slidetime.GetFloat(), m_flSlideTime + sdk_slidetime.GetFloat()+1, 0.1f, 1.0f);
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
	return m_bRolling;
}

bool CSDKPlayerShared::CanRoll() const
{
	if (m_pOuter->GetLocalVelocity().Length2D() < 10)
		return false;

	if (IsProne())
		return false;

	if (IsSliding())
		return false;

	if (IsRolling())
		return false;

	if (IsDiving())
		return false;

	if (!CanChangePosition())
		return false;

	return true;
}

void CSDKPlayerShared::StartRolling(bool bFromDive)
{
	if (!CanRoll())
		return;

	m_bRolling = true;
	m_bRollingFromDive = bFromDive;

	ForceUnzoom();

	m_vecRollDirection = m_pOuter->GetAbsVelocity();
	m_vecRollDirection.GetForModify().NormalizeInPlace();

	m_flRollTime = gpGlobals->curtime;
}

void CSDKPlayerShared::EndRoll()
{
	m_bRolling = false;
	m_flRollTime = 0;
}

bool CSDKPlayerShared::IsDiving() const
{
	return m_bDiving && m_pOuter->IsAlive();
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

	return true;
}

ConVar  sdk_dive_height( "sdk_dive_height", "150", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar  sdk_dive_gravity( "sdk_dive_gravity", "0.7", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

Vector CSDKPlayerShared::StartDiving()
{
	if (!CanDive())
		return m_pOuter->GetAbsVelocity();

	CPASFilter filter( m_pOuter->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pOuter->EmitSound( filter, m_pOuter->entindex(), "Player.GoDive" );

	m_bDiving = true;

	ForceUnzoom();

	m_vecDiveDirection = m_pOuter->GetAbsVelocity();
	m_vecDiveDirection.GetForModify().z = 0;
	m_vecDiveDirection.GetForModify().NormalizeInPlace();

	// We need to turn off interp since the dive changes our origin abruptly.
	m_pOuter->AddEffects( EF_NOINTERP );
	m_pOuter->DoAnimationEvent(PLAYERANIMEVENT_DIVE);

	m_pOuter->SetGroundEntity(NULL);

	m_pOuter->SetGravity(sdk_dive_gravity.GetFloat());

	ConVarRef sdk_dive_speed("sdk_dive_speed");
	return m_vecDiveDirection.Get() * sdk_dive_speed.GetFloat() + Vector(0, 0, sdk_dive_height.GetFloat());
}

void CSDKPlayerShared::EndDive()
{
	m_pOuter->SetGravity(1);
	m_bDiving = false;
	m_pOuter->RemoveEffects( EF_NOINTERP );
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

bool CSDKPlayerShared::IsAimedIn() const
{
	if (IsDiving() || IsRolling())
		return false;

	return m_bAimedIn;
}

void CSDKPlayerShared::SetAimIn(bool bAimIn)
{
	m_bAimedIn = bAimIn;
}

ConVar dab_recoildecay("dab_recoildecay", "100", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

Vector CSDKPlayerShared::GetRecoil(float flFrameTime)
{
	if (m_flRecoilAccumulator <= 0)
		return Vector(0, 0, 0);

	float flRecoil = m_flRecoilAccumulator*flFrameTime;
	m_flRecoilAccumulator = Approach(0, m_flRecoilAccumulator, flFrameTime * dab_recoildecay.GetFloat());
	return m_vecRecoilDirection * flRecoil;
}

ConVar dab_recoilmultiplier("dab_recoilmultiplier", "2", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

void CSDKPlayerShared::SetRecoil(float flRecoil)
{
	m_flRecoilAccumulator = flRecoil * dab_recoilmultiplier.GetFloat();
	m_vecRecoilDirection.y = 1;
	m_vecRecoilDirection.x = SharedRandomFloat( "Recoil", -0.5f, 0.5f );
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

void CSDKPlayer::InitSpeeds()
{
	m_Shared.m_flRunSpeed = SDK_DEFAULT_PLAYER_RUNSPEED;
	m_Shared.m_flSprintSpeed = SDK_DEFAULT_PLAYER_SPRINTSPEED;
	m_Shared.m_flProneSpeed = SDK_DEFAULT_PLAYER_PRONESPEED;
	m_Shared.m_flSlideSpeed = SDK_DEFAULT_PLAYER_SLIDESPEED;
	m_Shared.m_flRollSpeed = SDK_DEFAULT_PLAYER_ROLLSPEED;
	m_Shared.m_flAimInSpeed = 100;

	// Set the absolute max to sprint speed
	SetMaxSpeed( m_Shared.m_flSprintSpeed ); 
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
	CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eWeapon);

	if (pWeaponInfo->iWeight + m_iLoadoutWeight > MAX_LOADOUT_WEIGHT)
		return false;

	// When multiple weapon support goes in just erase this.
	if (m_aLoadout[eWeapon].m_iCount)
		return false;

	return true;
}

int CSDKPlayer::GetLoadoutWeaponCount(SDKWeaponID eWeapon)
{
	return m_aLoadout[eWeapon].m_iCount;
}

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"

#include "sdk_shareddefs.h"
#include "sdk_playeranimstate.h"
#include "sdk_gamerules.h"
#include "base_playeranimstate.h"
#include "datacache/imdlcache.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#endif

#define SDK_RUN_SPEED				320.0f
#define SDK_WALK_SPEED				120.0f
#define SDK_CROUCHWALK_SPEED		110.0f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : CMultiPlayerAnimState*
//-----------------------------------------------------------------------------
CSDKPlayerAnimState* CreateSDKPlayerAnimState( CSDKPlayer *pPlayer )
{
	MDLCACHE_CRITICAL_SECTION();

	// Setup the movement data.
	MultiPlayerMovementData_t movementData;
	movementData.m_flBodyYawRate = 720.0f;
	movementData.m_flRunSpeed = SDK_RUN_SPEED;
	movementData.m_flWalkSpeed = SDK_WALK_SPEED;
	movementData.m_flSprintSpeed = -1.0f;

	// Create animation state for this player.
	CSDKPlayerAnimState *pRet = new CSDKPlayerAnimState( pPlayer, movementData );

	// Specific SDK player initialization.
	pRet->InitSDKAnimState( pPlayer );

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CSDKPlayerAnimState::CSDKPlayerAnimState()
{
	m_pSDKPlayer = NULL;

	// Don't initialize SDK specific variables here. Init them in InitSDKAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			&movementData - 
//-----------------------------------------------------------------------------
CSDKPlayerAnimState::CSDKPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData )
: CMultiPlayerAnimState( pPlayer, movementData )
{
	m_pSDKPlayer = NULL;

	// Don't initialize SDK specific variables here. Init them in InitSDKAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CSDKPlayerAnimState::~CSDKPlayerAnimState()
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialize Team Fortress specific animation state.
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CSDKPlayerAnimState::InitSDKAnimState( CSDKPlayer *pPlayer )
{
	m_pSDKPlayer = pPlayer;

	m_flCharacterEyeYaw = 0;
	m_flCharacterEyePitch = 0;

#if defined ( SDK_USE_PRONE )
	m_iProneActivity = ACT_DA_STAND_IDLE;
	m_bProneTransition = false;
	m_bProneTransitionFirstFrame = false;
#endif

	m_bFacingForward = true;

	m_bDiveStart = false;
	m_bDiveStartFirstFrame = false;

	m_iSlideActivity = ACT_DA_SLIDESTART;
	m_bSlideTransition = false;
	m_bSlideTransitionFirstFrame = false;

	m_iRollActivity = ACT_DA_ROLL;
	m_bRollTransition = false;
	m_bRollTransitionFirstFrame = false;

	m_bFlipping = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayerAnimState::ClearAnimationState( void )
{
#if defined ( SDK_USE_PRONE )
	m_bProneTransition = false;
	m_bProneTransitionFirstFrame = false;
#endif

	m_bFacingForward = true;

	m_bDiveStart = false;
	m_bDiveStartFirstFrame = false;

	m_bSlideTransition = false;
	m_bSlideTransitionFirstFrame = false;

	m_bRollTransition = false;
	m_bRollTransitionFirstFrame = false;

	BaseClass::ClearAnimationState();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : actDesired - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CSDKPlayerAnimState::TranslateActivity( Activity actDesired )
{
	Activity translateActivity = BaseClass::TranslateActivity( actDesired );

	if ( GetSDKPlayer()->GetActiveWeapon() )
	{
		translateActivity = GetSDKPlayer()->GetActiveWeapon()->ActivityOverride( translateActivity, false );
	}

	return translateActivity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayerAnimState::Update( float eyeYaw, float eyePitch, float flCharacterYaw, float flCharacterPitch )
{
	// Profile the animation update.
	VPROF( "CMultiPlayerAnimState::Update" );

	// Get the SDK player.
	CSDKPlayer *pSDKPlayer = GetSDKPlayer();
	if ( !pSDKPlayer )
		return;

	// Get the studio header for the player.
	CStudioHdr *pStudioHdr = pSDKPlayer->GetModelPtr();
	if ( !pStudioHdr )
		return;

	// Check to see if we should be updating the animation state - dead, ragdolled?
	if ( !ShouldUpdateAnimState() )
	{
		ClearAnimationState();
		return;
	}

	// Store the eye angles.
	m_flEyeYaw = AngleNormalize( eyeYaw );
	m_flEyePitch = AngleNormalize( eyePitch );

	float flRampSpeed = 20;
	if (pSDKPlayer->GetActiveSDKWeapon() && pSDKPlayer->GetActiveSDKWeapon()->HasAimInSpeedPenalty())
		flRampSpeed = 50;

	float flApproachSpeed = gpGlobals->frametime * pSDKPlayer->GetSlowMoMultiplier() * RemapVal(m_pSDKPlayer->m_Shared.GetAimIn(), 0, 1, 10, flRampSpeed);

	float flYawDifference = AngleNormalize( flCharacterYaw - m_flCharacterEyeYaw );
	float flYawApproachSpeed = flYawDifference * flApproachSpeed;
	if (fabs(flYawApproachSpeed) < fabs(flYawDifference))
		m_flCharacterEyeYaw += flYawApproachSpeed;
	else
		m_flCharacterEyeYaw = flCharacterYaw;

	float flPitchDifference = AngleNormalize( flCharacterPitch - m_flCharacterEyePitch );
	float flPitchApproachSpeed = flPitchDifference * flApproachSpeed;
	if (fabs(flPitchApproachSpeed) < fabs(flPitchDifference))
		m_flCharacterEyePitch += flPitchApproachSpeed;
	else
		m_flCharacterEyePitch = flCharacterPitch;

	// Compute the player sequences.
	ComputeSequences( pStudioHdr );

	if ( SetupPoseParameters( pStudioHdr ) )
	{
		// Pose parameter - what direction are the player's legs running in.
		ComputePoseParam_MoveYaw( pStudioHdr );

		ComputePoseParam_StuntYaw( pStudioHdr );

		// Pose parameter - Torso aiming (up/down).
		ComputePoseParam_AimPitch( pStudioHdr );

		// Pose parameter - Torso aiming (rotation).
		ComputePoseParam_AimYaw( pStudioHdr );
	}

#ifdef CLIENT_DLL 
	if ( C_BasePlayer::ShouldDrawLocalPlayer() )
	{
		m_pSDKPlayer->SetPlaybackRate( 1.0f );
	}
#endif
}

bool CSDKPlayerAnimState::SetupPoseParameters( CStudioHdr *pStudioHdr )
{
	// Check to see if this has already been done.
	if ( m_bPoseParameterInit )
		return true;

	// Save off the pose parameter indices.
	if ( !pStudioHdr )
		return false;

	m_iStuntYawPose = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "stunt_yaw" );
	if (m_iStuntYawPose < 0)
		return false;

	// Look for the movement blenders.
	m_PoseParameterData.m_iMoveX = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "move_x" );
	m_PoseParameterData.m_iMoveY = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "move_y" );
	if ( ( m_PoseParameterData.m_iMoveX < 0 ) || ( m_PoseParameterData.m_iMoveY < 0 ) )
		return false;

	// Look for the aim pitch blender.
	m_PoseParameterData.m_iAimPitch = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "body_pitch" );
	if ( m_PoseParameterData.m_iAimPitch < 0 )
		return false;

	// Look for aim yaw blender.
	m_PoseParameterData.m_iAimYaw = GetBasePlayer()->LookupPoseParameter( pStudioHdr, "body_yaw" );
	if ( m_PoseParameterData.m_iAimYaw < 0 )
		return false;

	m_bPoseParameterInit = true;

	return true;
}

void CSDKPlayerAnimState::GetOuterAbsVelocity(Vector& vel)
{
	BaseClass::GetOuterAbsVelocity(vel);

#ifdef CLIENT_DLL
	// For non-local players this is an estimation based on interp data.
	// Compensate for slow motion accordingly.
	if (m_pSDKPlayer != C_SDKPlayer::GetLocalSDKPlayer())
		vel /= m_pSDKPlayer->GetSlowMoMultiplier();
#endif
}

void CSDKPlayerAnimState::ComputePoseParam_AimPitch( CStudioHdr *pStudioHdr )
{
	if (m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsDiveSliding())
	{
		Vector vecVelocity;
		GetOuterAbsVelocity( vecVelocity );

		QAngle angSlide;
		VectorAngles(vecVelocity, angSlide);
		m_angRender[YAW] = angSlide.y;

		Vector vecForward;
		m_pSDKPlayer->GetVectors(&vecForward, NULL, NULL);

		float flAimPitch = DotProduct(Vector(0, 0, 1), vecForward) * 90;

		// Set the aim yaw and save.
		GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimPitch, flAimPitch );
		m_DebugAnimData.m_flAimPitch = flAimPitch;

		return;
	}

	if (m_pSDKPlayer->IsInThirdPerson())
	{
		// Use the character's eye direction instead of the actual.
		float flAimPitch = m_flCharacterEyePitch;

		// Set the aim pitch pose parameter and save.
		GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimPitch, -flAimPitch );
		m_DebugAnimData.m_flAimPitch = flAimPitch;

		return;
	}

	BaseClass::ComputePoseParam_AimPitch(pStudioHdr);
}

void CSDKPlayerAnimState::ComputePoseParam_AimYaw( CStudioHdr *pStudioHdr )
{
	if (m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsDiveSliding())
	{
		Vector vecVelocity;
		GetOuterAbsVelocity( vecVelocity );

		QAngle angDir;
		VectorAngles(vecVelocity, angDir);

		//if (m_bFacingForward)
		m_flGoalFeetYaw = angDir[YAW];
		//else
		//	m_flGoalFeetYaw = AngleNormalize(angDir[YAW] + 180);

		m_flGoalFeetYaw = AngleNormalize( m_flGoalFeetYaw );
		if ( m_flGoalFeetYaw != m_flCurrentFeetYaw )
		{
			ConvergeYawAngles( m_flGoalFeetYaw, 720.0f, gpGlobals->frametime * m_pSDKPlayer->GetSlowMoMultiplier(), m_flCurrentFeetYaw );
			m_flLastAimTurnTime = m_pSDKPlayer->GetCurrentTime();
		}

		m_angRender[YAW] = angDir.y;

		// Get the view yaw.
		float flAngle = AngleNormalize( m_flEyeYaw );

		// Calc side to side turning - the view vs. movement yaw.
		float flAimYaw = angDir.y - flAngle;
		flAimYaw = AngleNormalize( flAimYaw );

		// Set the aim yaw and save.
		GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimYaw, flAimYaw );
		m_DebugAnimData.m_flAimYaw	= flAimYaw;

		// Turn off a force aim yaw - either we have already updated or we don't need to.
		m_bForceAimYaw = false;

#ifndef CLIENT_DLL
		QAngle angle = GetBasePlayer()->GetAbsAngles();
		angle[YAW] = m_flCurrentFeetYaw;

		GetBasePlayer()->SetAbsAngles( angle );
#endif

		return;
	}
	else if (m_bFlipping)
	{
		m_angRender[YAW] = m_flEyeYaw;
#ifndef CLIENT_DLL
		QAngle angle = GetBasePlayer()->GetAbsAngles();
		angle[YAW] = m_flCurrentFeetYaw;

		GetBasePlayer()->SetAbsAngles( angle );
#endif
		return;
	}
	else if (m_pSDKPlayer->m_Shared.IsManteling())
	{
		m_angRender[YAW] = m_PoseParameterData.m_flEstimateYaw;
#ifndef CLIENT_DLL
		QAngle angle = GetBasePlayer()->GetAbsAngles();
		angle[YAW] = m_PoseParameterData.m_flEstimateYaw;

		GetBasePlayer()->SetAbsAngles( angle );
#endif
		return;
	}

	// Get the movement velocity.
	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	// Check to see if we are moving.
	bool bMoving = ( vecVelocity.Length() > 1.0f ) ? true : false;

	if ( m_pSDKPlayer->m_Shared.IsProne() )
	{
		m_flGoalFeetYaw = m_flCurrentFeetYaw = m_flEyeYaw;
	}
	else if ( bMoving || m_bForceAimYaw )
	{
		if (m_pSDKPlayer->m_Shared.IsAimedIn() || m_pSDKPlayer->m_Shared.IsDiving() || m_pSDKPlayer->m_Shared.IsRolling() || m_pSDKPlayer->m_Shared.IsSliding())
		{
			// The feet match the eye direction when moving - the move yaw takes care of the rest.
			m_flGoalFeetYaw = m_flEyeYaw;
		}
		else
		{
			QAngle angDir;
			VectorAngles(vecVelocity, angDir);

			if (m_bFacingForward)
			{
				m_flGoalFeetYaw = angDir[YAW];
			}
			else
			{
				m_flGoalFeetYaw = AngleNormalize(angDir[YAW] + 180);
			}
		}
	}
	// Else if we are not moving.
	else
	{
		// Initialize the feet.
		if ( m_PoseParameterData.m_flLastAimTurnTime <= 0.0f )
		{
			m_flGoalFeetYaw	= m_flEyeYaw;
			m_flCurrentFeetYaw = m_flEyeYaw;
			m_PoseParameterData.m_flLastAimTurnTime = m_pSDKPlayer->GetCurrentTime();
		}
		// Make sure the feet yaw isn't too far out of sync with the eye yaw.
		// TODO: Do something better here!
		else
		{
			float flYawDelta = AngleNormalize(  m_flGoalFeetYaw - m_flEyeYaw );

			if ( fabs( flYawDelta ) > 75.0f )
			{
				float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
				m_flGoalFeetYaw += ( 75.0f * flSide );
			}
		}
	}

	// Fix up the feet yaw.
	m_flGoalFeetYaw = AngleNormalize( m_flGoalFeetYaw );
	if ( m_flGoalFeetYaw != m_flCurrentFeetYaw )
	{
		if ( m_bForceAimYaw )
		{
			m_flCurrentFeetYaw = m_flGoalFeetYaw;
		}
		else
		{
			ConvergeYawAnglesThroughZero( m_flGoalFeetYaw, 720.0f, gpGlobals->frametime * m_pSDKPlayer->GetSlowMoMultiplier(), m_flCurrentFeetYaw );
			m_flLastAimTurnTime = m_pSDKPlayer->GetCurrentTime();
		}
	}

	if (m_pSDKPlayer->m_Shared.IsDiving())
		m_flCurrentFeetYaw = m_flGoalFeetYaw;

	// Rotate the body into position.
	m_angRender[YAW] = m_flCurrentFeetYaw;

	// Find the aim(torso) yaw base on the eye and feet yaws.
	float flAimYaw = m_flEyeYaw - m_flCurrentFeetYaw;

	if (m_pSDKPlayer->IsInThirdPerson())
		flAimYaw = m_flCharacterEyeYaw - m_flCurrentFeetYaw;

	flAimYaw = AngleNormalize( flAimYaw );

	// Set the aim yaw and save.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimYaw, -flAimYaw );
	m_DebugAnimData.m_flAimYaw	= flAimYaw;

	// Turn off a force aim yaw - either we have already updated or we don't need to.
	m_bForceAimYaw = false;

#ifndef CLIENT_DLL
	QAngle angle = GetBasePlayer()->GetAbsAngles();
	angle[YAW] = m_flCurrentFeetYaw;

	GetBasePlayer()->SetAbsAngles( angle );
#endif
}

extern ConVar mp_slammoveyaw;
float SnapYawTo( float flValue );
void CSDKPlayerAnimState::ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr )
{
	// Get the estimated movement yaw.
	EstimateYaw();

	// Get the view yaw.
	float flAngle = AngleNormalize( m_flEyeYaw );

	// Calc side to side turning - the view vs. movement yaw.
	float flYaw = flAngle - m_PoseParameterData.m_flEstimateYaw;
	flYaw = AngleNormalize( -flYaw );

	// Get the current speed the character is running.
	bool bIsMoving;
	float flPlaybackRate = CalcMovementPlaybackRate( &bIsMoving );

	// Setup the 9-way blend parameters based on our speed and direction.
	Vector2D vecCurrentMoveYaw( 0.0f, 0.0f );
	if ( bIsMoving )
	{
		if (m_pSDKPlayer->m_Shared.IsAimedIn() || m_pSDKPlayer->m_Shared.IsDiving() || m_pSDKPlayer->m_Shared.IsRolling() || m_pSDKPlayer->m_Shared.IsSliding())
		{
			vecCurrentMoveYaw.x = cos( DEG2RAD( flYaw ) ) * flPlaybackRate;
			vecCurrentMoveYaw.y = -sin( DEG2RAD( flYaw ) ) * flPlaybackRate;
		}
		else
		{
			vecCurrentMoveYaw.x = cos( DEG2RAD( m_PoseParameterData.m_flEstimateYaw ) ) * flPlaybackRate;
			vecCurrentMoveYaw.y = -sin( DEG2RAD( m_PoseParameterData.m_flEstimateYaw ) ) * flPlaybackRate;
		}
	}

	// Set the 9-way blend movement pose parameters.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveX, vecCurrentMoveYaw.x );
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, vecCurrentMoveYaw.y );

	m_DebugAnimData.m_vecMoveYaw = vecCurrentMoveYaw;
}

void CSDKPlayerAnimState::ComputePoseParam_StuntYaw( CStudioHdr *pStudioHdr )
{
	// Get the view yaw.
	float flAngle = AngleNormalize( m_flEyeYaw );

	// Calc side to side turning - the view vs. movement yaw.
	float flYaw = flAngle - m_PoseParameterData.m_flEstimateYaw;
	flYaw = AngleNormalize( flYaw );

	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_iStuntYawPose, flYaw );
}

void CSDKPlayerAnimState::EstimateYaw( void )
{
	// Get the frame time.
	float flDeltaTime = gpGlobals->frametime * m_pSDKPlayer->GetSlowMoMultiplier();
	if ( flDeltaTime == 0.0f )
		return;

	// Get the player's velocity and angles.
	Vector vecEstVelocity;
	GetOuterAbsVelocity( vecEstVelocity );
	QAngle angles = GetBasePlayer()->GetLocalAngles();

	// If we are not moving, sync up the feet and eyes slowly.
	if (m_pSDKPlayer->m_Shared.IsProne())
	{
		// Don't touch it
	}
	else if (m_pSDKPlayer->m_Shared.IsManteling())
	{
		Vector vecWallNormal = m_pSDKPlayer->m_Shared.GetMantelWallNormal();
		m_PoseParameterData.m_flEstimateYaw = ( atan2( -vecWallNormal.y, -vecWallNormal.x ) * 180.0f / M_PI );
	}
	else if ( vecEstVelocity.x == 0.0f && vecEstVelocity.y == 0.0f )
	{
		float flYawDelta = angles[YAW] - m_PoseParameterData.m_flEstimateYaw;
		flYawDelta = AngleNormalize( flYawDelta );

		if ( flDeltaTime < 0.25f )
		{
			flYawDelta *= ( flDeltaTime * 4.0f );
		}
		else
		{
			flYawDelta *= flDeltaTime;
		}

		m_PoseParameterData.m_flEstimateYaw += flYawDelta;
		m_PoseParameterData.m_flEstimateYaw = AngleNormalize( m_PoseParameterData.m_flEstimateYaw );
	}
	else if (m_pSDKPlayer->m_Shared.IsAimedIn() || m_pSDKPlayer->m_Shared.IsDiving() || m_pSDKPlayer->m_Shared.IsRolling() || m_pSDKPlayer->m_Shared.IsSliding())
	{
		m_PoseParameterData.m_flEstimateYaw = ( atan2( vecEstVelocity.y, vecEstVelocity.x ) * 180.0f / M_PI );
		m_PoseParameterData.m_flEstimateYaw = clamp( m_PoseParameterData.m_flEstimateYaw, -180.0f, 180.0f );
	}
	else
	{
		QAngle angDir;
		VectorAngles(vecEstVelocity, angDir);

		if (fabs(AngleNormalize(angDir[YAW] - m_flEyeYaw)) <= 90)
			m_bFacingForward = true;
		else if (fabs(AngleNormalize(angDir[YAW] - m_flEyeYaw)) >= 91)
			m_bFacingForward = false;

		float flYawDelta = AngleNormalize(m_flGoalFeetYaw - m_flCurrentFeetYaw);

		if (m_bFacingForward)
			m_PoseParameterData.m_flEstimateYaw = flYawDelta;
		else
			m_PoseParameterData.m_flEstimateYaw = 180-flYawDelta;
	}
}

void CSDKPlayerAnimState::ConvergeYawAnglesThroughZero( float flGoalYaw, float flYawRate, float flDeltaTime, float &flCurrentYaw )
{
	float flFadeTurnDegrees = 60;

	float flEyeGoalYaw = AngleDiff(flGoalYaw, m_flEyeYaw);
	float flEyeCurrentYaw = AngleDiff(flCurrentYaw, m_flEyeYaw);

	// Find the yaw delta.
	float flDeltaYaw = flEyeGoalYaw - flEyeCurrentYaw;
	float flDeltaYawAbs = fabs( flDeltaYaw );

	// Always do at least a bit of the turn (1%).
	float flScale = 1.0f;
	flScale = flDeltaYawAbs / flFadeTurnDegrees;
	flScale = clamp( flScale, 0.01f, 1.0f );

	float flYaw = flYawRate * flDeltaTime * flScale;
	if ( flDeltaYawAbs < flYaw )
	{
		flCurrentYaw = flGoalYaw;
	}
	else
	{
		float flSide = ( flDeltaYaw < 0.0f ) ? -1.0f : 1.0f;
		flCurrentYaw += ( flYaw * flSide );
	}

	flCurrentYaw = AngleNormalize( flCurrentYaw );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : event - 
//-----------------------------------------------------------------------------
void CSDKPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	Activity iGestureActivity = ACT_INVALID;

	switch( event )
	{
	case PLAYERANIMEVENT_ATTACK_PRIMARY:
		{
			// Weapon primary fire.
			if ( m_pSDKPlayer->m_Shared.IsProne() || m_pSDKPlayer->m_Shared.IsDiveSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK_PRONE );
			else if ( m_pSDKPlayer->m_Shared.IsSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK_SLIDE );
			else if ( m_pSDKPlayer->m_Shared.IsRolling() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK_ROLL );
			else if ( m_pSDKPlayer->m_Shared.IsDiving() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK_DIVE );
			else if ( m_pSDKPlayer->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK_CROUCH );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_PRIMARYATTACK );

			iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}

	case PLAYERANIMEVENT_VOICE_COMMAND_GESTURE:
		{
			if ( !IsGestureSlotActive( GESTURE_SLOT_ATTACK_AND_RELOAD ) )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, (Activity)nData );
			}
			iGestureActivity = ACT_VM_IDLE; //TODO?
			break;
		}
	case PLAYERANIMEVENT_ATTACK_SECONDARY:
		{
			// Weapon secondary fire.
			if ( m_pSDKPlayer->m_Shared.IsProne() || m_pSDKPlayer->m_Shared.IsDiveSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL_PRONE );
			else if ( m_pSDKPlayer->m_Shared.IsSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL_SLIDE );
			else if ( m_pSDKPlayer->m_Shared.IsRolling() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL_ROLL );
			else if ( m_pSDKPlayer->m_Shared.IsDiving() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL_DIVE );
			else if ( m_pSDKPlayer->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL_CROUCH );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_BRAWL );

			iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}
	case PLAYERANIMEVENT_ATTACK_PRE:
		{
			if ( m_pSDKPlayer->GetFlags() & FL_DUCKING ) 
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc in crouch.
				iGestureActivity = ACT_MP_ATTACK_CROUCH_PREFIRE;
			}
			else
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc.
				iGestureActivity = ACT_MP_ATTACK_STAND_PREFIRE;
			}

			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, iGestureActivity, false );
			iGestureActivity = ACT_VM_IDLE; //TODO?

			break;
		}
	case PLAYERANIMEVENT_ATTACK_POST:
		{
			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_POSTFIRE );
			iGestureActivity = ACT_VM_IDLE; //TODO?
			break;
		}

	case PLAYERANIMEVENT_RELOAD:
		{
			// Weapon reload.
			/*//We only have standing reload-- it blends into other states
			if ( m_pSDKPlayer->m_Shared.IsProne() || m_pSDKPlayer->m_Shared.IsDiveSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_PRONE );
			else if ( m_pSDKPlayer->m_Shared.IsSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_SLIDE );
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_CROUCH );
			else*/
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD );

			iGestureActivity = ACT_VM_RELOAD; //Make view reload if it isn't already
			break;
		}
	case PLAYERANIMEVENT_RELOAD_LOOP:
		{
			// Weapon reload.
			/*//We only have standing reload-- it blends into other states
			if ( m_pSDKPlayer->m_Shared.IsProne() || m_pSDKPlayer->m_Shared.IsDiveSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_LOOP_PRONE );
			else if ( m_pSDKPlayer->m_Shared.IsSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_LOOP_SLIDE );
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_LOOP_CROUCH );
			else*/
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_LOOP );

			iGestureActivity = ACT_INVALID; //TODO: fix
			break;
		}
	case PLAYERANIMEVENT_RELOAD_END:
		{
			// Weapon reload.
			if ( m_pSDKPlayer->m_Shared.IsProne() || m_pSDKPlayer->m_Shared.IsDiveSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_END_PRONE );
			else if ( m_pSDKPlayer->m_Shared.IsSliding() )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_END_SLIDE );
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_END_CROUCH );
			else
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_RELOAD_END );

			iGestureActivity = ACT_INVALID; //TODO: fix
			break;
		}
#if defined ( SDK_USE_PRONE )
	case PLAYERANIMEVENT_STAND_TO_PRONE:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			//m_iProneActivity = ACT_MP_STAND_TO_PRONE;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no stand->prone so just idle.
		}
		break;
	case PLAYERANIMEVENT_CROUCH_TO_PRONE:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			//m_iProneActivity = ACT_MP_CROUCH_TO_PRONE;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no crouch->prone so just idle.
		}
		break;
	case PLAYERANIMEVENT_PRONE_TO_STAND:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			m_iProneActivity = ACT_DA_PRONE_TO_STAND;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no prone->stand so just idle.
		}
		break;
	case PLAYERANIMEVENT_PRONE_TO_CROUCH:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			//m_iProneActivity = ACT_MP_PRONE_TO_CROUCH;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no prone->crouch so just idle.
		}
		break;
#endif

	case PLAYERANIMEVENT_DRAW:
		{
			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_DA_DRAW );
			iGestureActivity = ACT_VM_DRAW;
			break;
		}

	case PLAYERANIMEVENT_DIVE:
		{
			m_bDiveStart = true;
			m_bDiveStartFirstFrame = true;
			m_iDiveActivity = ACT_DA_DIVE;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE;
		}
		break;

	case PLAYERANIMEVENT_DIVE_TO_ROLL:
		{
			m_bRollTransition = true;
			m_bRollTransitionFirstFrame = true;
			m_iRollActivity = ACT_DA_DIVEROLL;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no stand->slide so just idle.
		}
		break;

	case PLAYERANIMEVENT_STAND_TO_SLIDE:
		{
			m_bSlideTransition = true;
			m_bSlideTransitionFirstFrame = true;
			m_iSlideActivity = ACT_DA_SLIDESTART;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no stand->slide so just idle.
		}
		break;

	case PLAYERANIMEVENT_STAND_TO_ROLL:
		{
			m_bRollTransition = true;
			m_bRollTransitionFirstFrame = true;
			m_iRollActivity = ACT_DA_ROLL;
			RestartMainSequence();
			iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no stand->slide so just idle.
		}
		break;

	case PLAYERANIMEVENT_STAND_TO_VAULT:
		m_bRollTransition = true;
		m_bRollTransitionFirstFrame = true;
		m_iRollActivity = ACT_DA_ROLL;
		RestartMainSequence();
		iGestureActivity = ACT_VM_IDLE;
		break;
	case PLAYERANIMEVENT_STAND_TO_WALLRUN:
		m_bRollTransition = true;
		m_bRollTransitionFirstFrame = true;
		m_iRollActivity = ACT_DA_ROLL;
		RestartMainSequence();
		iGestureActivity = ACT_VM_IDLE;
		break;
	case PLAYERANIMEVENT_WALLFLIP:
		RestartMainSequence();
		iGestureActivity = ACT_VM_IDLE;
		break;
	case PLAYERANIMEVENT_WALLCLIMB:
		RestartMainSequence();
		iGestureActivity = ACT_VM_IDLE;
		break;
	case PLAYERANIMEVENT_JUMP:
		{
			// Jump.
			m_bJumping = true;
			m_bFirstJumpFrame = true;
			m_flJumpStartTime = m_pSDKPlayer->GetCurrentTime();

			RestartMainSequence();

			break;
		}

	case PLAYERANIMEVENT_CANCEL:
		{
			ResetGestureSlot( GESTURE_SLOT_ATTACK_AND_RELOAD );
			break;
		}

	default:
		{
			BaseClass::DoAnimationEvent( event, nData );
			break;
		}
	}

#ifdef CLIENT_DLL
	// Make the weapon play the animation as well
	if ( iGestureActivity != ACT_INVALID && GetSDKPlayer() != CSDKPlayer::GetLocalSDKPlayer())
	{
		CBaseCombatWeapon *pWeapon = GetSDKPlayer()->GetActiveWeapon();
		if ( pWeapon )
		{
			//pWeapon->EnsureCorrectRenderingModel();
			pWeapon->SendWeaponAnim( iGestureActivity );
			// Force animation events!
			//pWeapon->ResetEventsParity();		// reset event parity so the animation events will occur on the weapon. 
			pWeapon->DoAnimationEvents( pWeapon->GetModelPtr() );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleSwimming( Activity &idealActivity )
{
	// for now return false to just use the standard movement animstate
	// if we get swim anims this will need updating
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleMoving( Activity &idealActivity )
{	
	if (!(m_pSDKPlayer->GetFlags() & FL_ONGROUND) &&
		 m_pSDKPlayer->GetAbsVelocity ().z < -270)
	{
		idealActivity = ACT_DA_JUMP_FLOAT;
		return true;
	}

	float flSpeed = GetOuterXYSpeed();

	if ( flSpeed > 150 )
	{
		if (ShouldUseAimInAnims())
			idealActivity = ACT_DA_RUN_AIM;
		else if (m_pSDKPlayer->IsWeaponReady())
			idealActivity = ACT_DA_RUN_READY;
		else
			idealActivity = ACT_DA_RUN_IDLE;
	}
	else if ( flSpeed > MOVING_MINIMUM_SPEED )
	{
		if (ShouldUseAimInAnims())
			idealActivity = ACT_DA_WALK_AIM;
		else if (m_pSDKPlayer->IsWeaponReady())
			idealActivity = ACT_DA_WALK_READY;
		else
			idealActivity = ACT_DA_WALK_IDLE;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleDucking( Activity &idealActivity )
{
	if ( m_pSDKPlayer->GetFlags() & FL_DUCKING )
	{
		// falling crouch anims go here
		if ( !(m_pSDKPlayer->GetFlags() & FL_ONGROUND) )
		{
			if (ShouldUseAimInAnims())
				idealActivity = ACT_DA_CROUCH_AIM;
			else if (m_pSDKPlayer->IsWeaponReady())
				idealActivity = ACT_DA_CROUCH_READY;
			else
				idealActivity = ACT_DA_CROUCH_IDLE;		
		}
		else if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
			if (ShouldUseAimInAnims())
				idealActivity = ACT_DA_CROUCH_AIM;
			else if (m_pSDKPlayer->IsWeaponReady())
				idealActivity = ACT_DA_CROUCH_READY;
			else
				idealActivity = ACT_DA_CROUCH_IDLE;		
		}
		else
		{
			if (ShouldUseAimInAnims())
				idealActivity = ACT_DA_CROUCHWALK_AIM;
			else if (m_pSDKPlayer->IsWeaponReady())
				idealActivity = ACT_DA_CROUCHWALK_READY;
			else
				idealActivity = ACT_DA_CROUCHWALK_IDLE;		
		}

		return true;
	}

	return false;
}
#if defined ( SDK_USE_PRONE )
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleProne( Activity &idealActivity )
{
	if ( m_pSDKPlayer->m_Shared.IsDiveSliding() )
	{
		idealActivity = ACT_DA_DIVESLIDE;

		return true;
	}
	else if ( m_pSDKPlayer->m_Shared.IsProne() )
	{
		if (ShouldUseAimInAnims())
			idealActivity = ACT_DA_PRONECHEST_AIM;
		else
			idealActivity = ACT_DA_PRONECHEST_IDLE;

		return true;
	}
	
	return false;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleProneTransition( Activity &idealActivity )
{
	if ( m_bProneTransition )
	{
		if (m_bProneTransitionFirstFrame)
		{
			m_bProneTransitionFirstFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		if (!m_pSDKPlayer->m_Shared.IsProne() && !m_pSDKPlayer->m_Shared.IsGettingUpFromProne())
			m_bProneTransition = false;

		//Tony; check the cycle, and then stop overriding
		else if ( GetBasePlayer()->GetCycle() >= 0.99 )
			m_bProneTransition = false;
		else
			idealActivity = m_iProneActivity;
	}

	return m_bProneTransition;
}
#endif // SDK_USE_PRONE

bool CSDKPlayerAnimState::HandleDiving( Activity &idealActivity )
{
	if (!m_pSDKPlayer->m_Shared.IsDiving())
		m_bDiveStart = false;

	if ( m_bDiveStart )
	{
		if (m_bDiveStartFirstFrame)
		{
			m_bDiveStartFirstFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		//Tony; check the cycle, and then stop overriding
		if ( GetBasePlayer()->GetCycle() >= 0.99 )
			m_bDiveStart = false;
		else
			idealActivity = m_iDiveActivity;
	}

	if ( !m_bDiveStart && m_pSDKPlayer->m_Shared.IsDiving() )
	{
		idealActivity = ACT_DA_DIVEFALL;		

		return true;
	}
	
	return m_bDiveStart;
}

bool CSDKPlayerAnimState::HandleSliding( Activity &idealActivity )
{
	if ( m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsDiveSliding() )
	{
		idealActivity = ACT_DA_SLIDE;		

		return true;
	}
	
	return false;
}

bool CSDKPlayerAnimState::HandleSlideTransition( Activity &idealActivity )
{
	if (!m_pSDKPlayer->m_Shared.IsSliding())
		m_bSlideTransition = false;

	if ( m_bSlideTransition )
	{
		if (m_bSlideTransitionFirstFrame)
		{
			m_bSlideTransitionFirstFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		//Tony; check the cycle, and then stop overriding
		if ( GetBasePlayer()->GetCycle() >= 0.99 )
			m_bSlideTransition = false;
		else
			idealActivity = m_iSlideActivity;
	}

	return m_bSlideTransition;
}

bool CSDKPlayerAnimState::HandleRollTransition( Activity &idealActivity )
{
	if (!m_pSDKPlayer->m_Shared.IsRolling())
		m_bRollTransition = false;

	if ( m_bRollTransition )
	{
		if (m_bRollTransitionFirstFrame)
		{
			m_bRollTransitionFirstFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		//Tony; check the cycle, and then stop overriding
		if ( GetBasePlayer()->GetCycle() >= 0.99 )
			m_bRollTransition = false;
		else
			idealActivity = m_iRollActivity;
	}
	return m_bRollTransition;
}

#if defined ( SDK_USE_SPRINTING )
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayerAnimState::HandleSprinting( Activity &idealActivity )
{
	if ( m_pSDKPlayer->m_Shared.IsSprinting() )
	{
		idealActivity = ACT_SPRINT;		

		return true;
	}
	
	return false;
}
#endif // SDK_USE_SPRINTING
//-----------------------------------------------------------------------------
// Purpose: 
bool CSDKPlayerAnimState::HandleJumping( Activity &idealActivity )
{
	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	if ( m_bJumping )
	{
		if ( m_bFirstJumpFrame )
		{
			m_bFirstJumpFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		// Reset if we hit water and start swimming.
		if ( m_pSDKPlayer->GetWaterLevel() >= WL_Waist )
		{
			m_bJumping = false;
			RestartMainSequence();
		}
		// Don't check if he's on the ground for a sec.. sometimes the client still has the
		// on-ground flag set right when the message comes in.
		else if ( m_pSDKPlayer->GetCurrentTime() - m_flJumpStartTime > 0.2f )
		{
			if ( m_pSDKPlayer->GetFlags() & FL_ONGROUND )
			{
				m_bJumping = false;
				RestartMainSequence();

				RestartGesture( GESTURE_SLOT_JUMP, ACT_DA_JUMP_LAND );					
			}
		}

		// if we're still jumping
		if ( m_bJumping )
		{
			if ( m_pSDKPlayer->GetCurrentTime() - m_flJumpStartTime > 0.5 )
				idealActivity = ACT_DA_JUMP_FLOAT;
			else
				idealActivity = ACT_DA_JUMP_START;
		}
	}	

	if (!m_bJumping && 
		!(m_pSDKPlayer->GetFlags() & FL_ONGROUND) &&
		!m_pSDKPlayer->m_Shared.IsSliding () &&
		!m_pSDKPlayer->m_Shared.IsRolling () &&
		m_pSDKPlayer->GetAbsVelocity ().z < -270)
	{
		idealActivity = ACT_DA_JUMP_FLOAT;
		return true;
	}

	if ( m_bJumping )
		return true;

	return false;
}

bool CSDKPlayerAnimState::HandleWallFlip (Activity &idealActivity)
{
	if (!m_bFlipping)
	{
		/*Flip only on activation*/
		if (!m_pSDKPlayer->m_Shared.IsWallFlipping())
			return false;
	}

	if (m_pSDKPlayer->GetCurrentTime () - 
		m_pSDKPlayer->m_Shared.GetDiveTime () < 1e-1) 
	{/*Not in a dive (exception: we want to dive out of a flip)*/
		m_bFlipping = false;
		return false;
	}
	if (m_pSDKPlayer->GetFlags()&FL_ONGROUND)
	{/*Exit if we land on something*/
		m_bFlipping = false;
		return false;
	}
	if (GetBasePlayer()->GetCycle () >= 0.99) 
	{/*Exit once the flip animation finished once*/
		m_bFlipping = false;
		return false;
	}
	idealActivity = ACT_DA_WALLFLIP;
	m_bFlipping = true;
	return true;
}

bool CSDKPlayerAnimState::HandleWallClimb (Activity &idealActivity)
{
	if (m_pSDKPlayer->m_Shared.IsManteling())
	{
		idealActivity = ACT_DA_WALLCLIMB;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Overriding CMultiplayerAnimState to add prone and sprinting checks as necessary.
// Input  :  - 
// Output : Activity
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
extern ConVar anim_showmainactivity;
#endif

Activity CSDKPlayerAnimState::CalcMainActivity()
{
	Activity idealActivity = ACT_DA_STAND_IDLE;

	if (ShouldUseAimInAnims())
		idealActivity = ACT_DA_STAND_AIM;
	else if (m_pSDKPlayer->IsWeaponReady())
		idealActivity = ACT_DA_STAND_READY;

	if (HandleWallClimb (idealActivity) ||
		HandleWallFlip (idealActivity) ||
		HandleDiving( idealActivity ) ||

		HandleJumping( idealActivity ) || 
#if defined ( SDK_USE_PRONE )
		//Tony; handle these before ducking !!
		HandleProneTransition( idealActivity ) ||
		HandleProne( idealActivity ) ||
#endif
		HandleRollTransition( idealActivity ) ||
		HandleSlideTransition( idealActivity ) ||
		HandleSliding( idealActivity ) ||
		HandleDucking( idealActivity ) || 
		HandleSwimming( idealActivity ) || 
		HandleDying( idealActivity ) 
#if defined ( SDK_USE_SPRINTING )
		|| HandleSprinting( idealActivity )
#endif
		)
	{
		// intentionally blank
	}
	else
	{
		HandleMoving( idealActivity );
	}

	ShowDebugInfo();

	// Client specific.
#ifdef CLIENT_DLL

	if ( anim_showmainactivity.GetBool() )
	{
		DebugShowActivity( idealActivity );
	}

#endif

	return idealActivity;
}

bool CSDKPlayerAnimState::ShouldUseAimInAnims()
{
	if (!m_pSDKPlayer->m_Shared.IsAimedIn())
		return false;

	if (!m_pSDKPlayer->GetActiveSDKWeapon())
		return false;

	if (!m_pSDKPlayer->GetActiveSDKWeapon()->HasAimInSpeedPenalty())
		return false;

	return true;
}

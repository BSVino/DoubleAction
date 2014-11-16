//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SDK_PLAYERANIMSTATE_H
#define SDK_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "Multiplayer/multiplayer_animstate.h"

#if defined( CLIENT_DLL )
class C_DAPlayer;
#define CDAPlayer C_DAPlayer
#else
class CDAPlayer;
#endif

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //
class CDAPlayerAnimState : public CMultiPlayerAnimState
{
public:
	
	DECLARE_CLASS( CDAPlayerAnimState, CMultiPlayerAnimState );

	CDAPlayerAnimState();
	CDAPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData );
	~CDAPlayerAnimState();

	void InitSDKAnimState( CDAPlayer *pPlayer );
	CDAPlayer *GetDAPlayer( void )							{ return m_pDAPlayer; }

	virtual void ClearAnimationState();
	virtual Activity TranslateActivity( Activity actDesired );
	virtual void Update( float eyeYaw, float eyePitch, float flCharacterYaw, float flCharacterPitch );
	bool	SetupPoseParameters( CStudioHdr *pStudioHdr );

	virtual void GetOuterAbsVelocity( Vector& vel );

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );
	bool	HandleSwimming( Activity &idealActivity );

	bool	HandleWallFlip (Activity &idealActivity);
	bool	HandleWallClimb (Activity &idealActivity);
#if defined ( SDK_USE_PRONE )
	bool	HandleProne( Activity &idealActivity );
	bool	HandleProneTransition( Activity &idealActivity );
#endif

	bool	HandleDiving( Activity &idealActivity );

	bool	HandleSliding( Activity &idealActivity );
	bool	HandleSlideTransition( Activity &idealActivity );

	bool	HandleRollTransition( Activity &idealActivity );

#if defined ( SDK_USE_SPRINTING )
	bool	HandleSprinting( Activity &idealActivity );
#endif

	virtual void		ComputePoseParam_AimPitch( CStudioHdr *pStudioHdr );
	virtual void		ComputePoseParam_AimYaw( CStudioHdr *pStudioHdr );
	virtual void		ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr );
	virtual void		ComputePoseParam_StuntYaw( CStudioHdr *pStudioHdr );

	virtual void		EstimateYaw( void );
	void                ConvergeYawAnglesThroughZero( float flGoalYaw, float flYawRate, float flDeltaTime, float &flCurrentYaw );

	virtual Activity CalcMainActivity();	

	virtual bool        ShouldUseAimInAnims();

private:
	
	CDAPlayer   *m_pDAPlayer;
	bool		m_bInAirWalk;
#if defined ( SDK_USE_PRONE )
	Activity	m_iProneActivity;
	bool		m_bProneTransition;
	bool		m_bProneTransitionFirstFrame;
#endif

	bool		m_bFacingForward;

	float       m_flCharacterEyeYaw;
	float       m_flCharacterEyePitch;

	Activity	m_iDiveActivity;
	bool		m_bDiveStart;
	bool		m_bDiveStartFirstFrame;

	Activity	m_iSlideActivity;
	bool		m_bSlideTransition;
	bool		m_bSlideTransitionFirstFrame;

	Activity	m_iRollActivity;
	bool		m_bRollTransition;
	bool		m_bRollTransitionFirstFrame;

	bool		m_bFlipping;

	float		m_flHoldDeployedPoseUntilTime;

	int			m_iStuntYawPose;

	Vector m_vecLastPlayerVelocity;
};

CDAPlayerAnimState *CreateDAPlayerAnimState( CDAPlayer *pPlayer );



#endif // SDK_PLAYERANIMSTATE_H

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
#include "multiplayer_animstate.h"

#if defined( CLIENT_DLL )
class C_SDKPlayer;
#define CSDKPlayer C_SDKPlayer
#else
class CSDKPlayer;
#endif

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //
class CSDKPlayerAnimState : public CMultiPlayerAnimState
{
public:
	
	DECLARE_CLASS( CSDKPlayerAnimState, CMultiPlayerAnimState );

	CSDKPlayerAnimState();
	CSDKPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData );
	~CSDKPlayerAnimState();

	void InitSDKAnimState( CSDKPlayer *pPlayer );
	CSDKPlayer *GetSDKPlayer( void )							{ return m_pSDKPlayer; }

	virtual void ClearAnimationState();
	virtual Activity TranslateActivity( Activity actDesired );
	virtual void Update( float eyeYaw, float eyePitch );
	bool	SetupPoseParameters( CStudioHdr *pStudioHdr );

	virtual void GetOuterAbsVelocity( Vector& vel );

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );
	bool	HandleSwimming( Activity &idealActivity );

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
	
	CSDKPlayer   *m_pSDKPlayer;
	bool		m_bInAirWalk;
#if defined ( SDK_USE_PRONE )
	Activity	m_iProneActivity;
	bool		m_bProneTransition;
	bool		m_bProneTransitionFirstFrame;
#endif

	bool		m_bFacingForward;

	Activity	m_iDiveActivity;
	bool		m_bDiveStart;
	bool		m_bDiveStartFirstFrame;

	Activity	m_iSlideActivity;
	bool		m_bSlideTransition;
	bool		m_bSlideTransitionFirstFrame;

	Activity	m_iRollActivity;
	bool		m_bRollTransition;
	bool		m_bRollTransitionFirstFrame;

	float		m_flHoldDeployedPoseUntilTime;

	int			m_iStuntYawPose;
};

CSDKPlayerAnimState *CreateSDKPlayerAnimState( CSDKPlayer *pPlayer );



#endif // SDK_PLAYERANIMSTATE_H

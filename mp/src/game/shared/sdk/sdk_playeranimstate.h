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

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );
	bool	HandleSwimming( Activity &idealActivity );

#if defined ( SDK_USE_PRONE )
	bool	HandleProne( Activity &idealActivity );
	bool	HandleProneTransition( Activity &idealActivity );
#endif

#if defined ( SDK_USE_SPRINTING )
	bool	HandleSprinting( Activity &idealActivity );
#endif

	//Tony; overriding because the SDK Player models pose parameter is flipped the opposite direction
	virtual void		ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr );

	virtual Activity CalcMainActivity();	

private:
	
	CSDKPlayer   *m_pSDKPlayer;
	bool		m_bInAirWalk;
#if defined ( SDK_USE_PRONE )
	Activity	m_iProneActivity;
	bool		m_bProneTransition;
	bool		m_bProneTransitionFirstFrame;
#endif

	float		m_flHoldDeployedPoseUntilTime;
};

CSDKPlayerAnimState *CreateSDKPlayerAnimState( CSDKPlayer *pPlayer );



#endif // SDK_PLAYERANIMSTATE_H

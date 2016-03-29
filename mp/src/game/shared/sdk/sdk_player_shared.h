//============ Copyright � 1996-2008, Valve Corporation, All rights reserved. ===============//
//
// Purpose: Shared Player Variables / Functions and variables that may or may not be networked
//
//===========================================================================================//

#ifndef SDK_PLAYER_SHARED_H
#define SDK_PLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"
#include "weapon_sdkbase.h"

#ifdef CLIENT_DLL
class C_SDKPlayer;
#else
class CSDKPlayer;
#endif

class CSDKPlayerShared
{
public:

#ifdef CLIENT_DLL
	friend class C_SDKPlayer;
	typedef C_SDKPlayer OuterClass;
	DECLARE_PREDICTABLE();
#else
	friend class CSDKPlayer;
	typedef CSDKPlayer OuterClass;
#endif

	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( CSDKPlayerShared );

	CSDKPlayerShared();
	~CSDKPlayerShared();

#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	void	SetStamina( float stamina );
	float	GetStamina( void ) { return m_flStamina; }
#endif // SDK_USE_STAMINA || SDK_USE_SPRINTING

	void	Init( OuterClass *pOuter );

	bool	IsSniperZoomed( void ) const;
	bool	IsDucking( void ) const; 

#if defined ( SDK_USE_PLAYERCLASSES )
	void	SetDesiredPlayerClass( int playerclass );
	int		DesiredPlayerClass( void );

	void	SetPlayerClass( int playerclass );
	int		PlayerClass( void );
#endif

	CWeaponSDKBase* GetActiveSDKWeapon() const;

#if defined ( SDK_USE_PRONE )
	void	StartGoingProne( void );
	void	StandUpFromProne( void );
	bool	IsProne() const;
	bool	IsGettingUpFromProne() const;	
	bool	IsGoingProne() const;
	void	SetProne( bool bProne, bool bNoAnimation = false );
	bool	CanChangePosition( void ) const;
#endif

	bool	IsSliding() const;
	bool	IsDiveSliding() const;
	bool	IsAirSliding() const { return m_bInAirSlide; }
	bool	CanSlide() const;
	bool	IsGettingUpFromSlide() const;	
	void	SetAirSliding( bool bInAir) { m_bInAirSlide = bInAir; }
	void	PlayStartSlideSound();
	void	PlayEndSlideSound();
	void	StartSliding(bool bDiveSliding = false);
	void	EndSlide();
	void	StandUpFromSlide(bool bJumpUp = false);
	float	GetSlideTime() const { return m_flSlideTime; };
	void    AddSlideTime(float flAdd) { m_flSlideTime += flAdd; }
	Vector	GetSlideDirection() const { return m_vecSlideDirection; };
	float	GetSlideFriction() const;

	void	SetDuckPress(bool bReset = false);
	float	GetLastDuckPress() const { return m_flLastDuckPress; };
	bool	IsRolling() const;

	/*This is not used?*/
	bool	IsRollingFromDive() const { return m_bRollingFromDive; };
	
	bool	CanRoll() const;
	void	StartRolling(bool bFromDive = false);
	void	EndRoll();
	float	GetRollTime() const { return m_flRollTime; };
	Vector	GetRollDirection() const { return m_vecRollDirection; };

	bool	IsDiving() const;
	bool	CanDive() const;
	Vector	StartDiving();
	void    StartSuperfallDiving();
	void	EndDive();
	Vector	GetDiveDirection() const { return m_vecDiveDirection; };
	bool    ShouldRollAfterDiving() const { return m_bRollAfterDive; }
	float   GetDiveTime() const { return m_flDiveTime; };
	float   GetDiveLerped() const { return m_flDiveLerped; };
	void    IncreaseDiveLerped(float flLerped) { m_flDiveLerped += flLerped; };
	float   GetTimeLeftGround() const { return m_flTimeLeftGround; };
	float   GetDiveToProneLandTime() const { return m_flDiveToProneLandTime; }
	void    DiveLandedProne();

	bool    IsManteling() const { return m_bIsManteling; }
	void    StartManteling(const Vector& vecWallNormal);
	void    ResetManteling();
	Vector  GetMantelWallNormal() const { return m_vecMantelWallNormal; }

	bool    IsWallFlipping(bool bExtend = false) const;
	void    StartWallFlip(const Vector& vecWallNormal);
	void    EndWallFlip();
	void    ResetWallFlipCount() { m_iWallFlipCount = 0; }
	int     GetWallFlipCount() const { return m_iWallFlipCount; }
	float   GetWallFlipEndTime() const { return m_flWallFlipEndTime; }

	bool	IsJumping( void ) { return m_bJumping; }
	void	SetJumping( bool bJumping );

	bool    IsSuperFalling();
	bool    CanSuperFallRespawn();

	bool	IsAimedIn() const;
	void	SetAimIn(bool bAimIn);
	float	GetAimIn() const { return m_flAimIn; }
	void	SetAimIn(float flAimIn) { m_flAimIn = flAimIn; }
	void    RampSlowAimIn(float flGoal);

	Vector	GetRecoil(float flFrameTime);
	void	SetRecoil(float flRecoil);

	float   GetViewBobRamp() const { return m_flViewBobRamp; }
	float   GetViewTilt() const { return m_flViewTilt; }

	float   GetRunSpeed() const { return m_flRunSpeed; }
	float   GetAimInSpeed() const { return m_flAimInSpeed; }

	void    PlayerOnGround();

	void	ForceUnzoom( void );

	float   ModifySkillValue(float flValue, float flModify, SkillID eSkill) const;

#ifdef SDK_USE_SPRINTING
	bool	IsSprinting( void ) { return m_bIsSprinting; }

	void	SetSprinting( bool bSprinting );
	void	StartSprinting( void );
	void	StopSprinting( void );

	void ResetSprintPenalty( void );
#endif

	void ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );

private:

#if defined ( SDK_USE_PRONE )
	CNetworkVar( bool, m_bProne );
#endif

#if defined ( SDK_USE_PLAYERCLASSES )
	CNetworkVar( int, m_iPlayerClass );
	CNetworkVar( int, m_iDesiredPlayerClass );
#endif

#if defined ( SDK_USE_SPRINTING )
	CNetworkVar( bool, m_bIsSprinting );
	bool m_bGaveSprintPenalty;
#endif

	CNetworkVar( bool, m_bSliding );
	CNetworkVar( bool, m_bInAirSlide );
	CNetworkVar( Vector, m_vecSlideDirection );
	CNetworkVar( float, m_flSlideTime );
	CNetworkVar( bool, m_bDiveSliding );

	CNetworkVar( float, m_flLastDuckPress );
	CNetworkVar( bool, m_bRolling );
	CNetworkVar( bool, m_bRollingFromDive );

	CNetworkVar( Vector, m_vecRollDirection );
	CNetworkVar( float, m_flRollTime );

	CNetworkVar( bool, m_bDiving );
	CNetworkVar( Vector, m_vecDiveDirection );
	CNetworkVar( bool, m_bRollAfterDive );
	CNetworkVar( float, m_flDiveTime );
	CNetworkVar( float, m_flTimeLeftGround );
	CNetworkVar( float, m_flDiveLerped );
	CNetworkVar( float, m_flDiveToProneLandTime );
	float m_flViewTilt;

	float m_flViewBobRamp;

	CNetworkVar( bool, m_bAimedIn );
	CNetworkVar( float, m_flAimIn );
	CNetworkVar( float, m_flSlowAimIn );

	Vector	m_vecRecoilDirection;
	float	m_flRecoilAccumulator;

#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	CNetworkVar( float, m_flStamina );
#endif // SDK_USE_STAMINA || SDK_USE_SPRINTING

	CNetworkVar(int,    m_iWallFlipCount);
	CNetworkVar(bool,   m_bIsWallFlipping);
	CNetworkVar(float,  m_flWallFlipEndTime);
	CNetworkVar(bool,   m_bIsManteling);
	CNetworkVar(Vector, m_vecMantelWallNormal);

public:

#ifdef SDK_USE_PRONE
	float m_flNextProneCheck; // Prevent it switching their prone state constantly.

	CNetworkVar( float, m_flUnProneTime );
	CNetworkVar( float, m_flGoProneTime );
	CNetworkVar( float, m_flDisallowUnProneTime );
	CNetworkVar( bool, m_bProneSliding );
	CNetworkVar( bool, m_bIsTryingUnprone );
#endif

	CNetworkVar( float, m_flUnSlideTime );
	CNetworkVar( Vector, m_vecUnSlideEyeStartOffset );
	CNetworkVar( bool, m_bIsTryingUnduck );

	CNetworkVar( bool, m_bSuperFalling );
	CNetworkVar( bool, m_bSuperFallOthersVisible );
	CNetworkVar( float, m_flSuperFallOthersNextCheck );

	bool m_bJumping;

	float m_flLastViewAnimationTime;

	//Tony; player speeds; at spawn server and client update both of these based on class (if any)
	float m_flRunSpeed;
	float m_flSprintSpeed;
	float m_flProneSpeed;
	float m_flSlideSpeed;
	float m_flRollSpeed;
	float m_flAimInSpeed;

	CNetworkVar( int, m_iStyleSkill );
	CNetworkVar( int, m_iStyleSkillAfterRespawn );
	CNetworkVar( bool, m_bSuperSkill );

private:

	OuterClass *m_pOuter;
};			   

class CArmament
{
	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( CArmament );

public:
	CNetworkVar(int,				m_iCount);
};

#define MAX_LOADOUT WEAPON_MAX
#define MAX_LOADOUT_WEIGHT 30

#endif //SDK_PLAYER_SHARED_H
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_SDK_PLAYER_H
#define C_SDK_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "sdk_shareddefs.h"
#include "sdk_playeranimstate.h"
#include "c_baseplayer.h"
#include "baseparticleentity.h"
#include "sdk_player_shared.h"

#include "hud/da_instructor.h"

class CProjectedLightEffect;

class C_SDKPlayer : public C_BasePlayer
{
public:
	class CLessonProgress
	{
	public:
		CLessonProgress()
		{
			m_flLastTimeShowed = 0;
			m_iTimesLearned = 0;
			m_iTimesShown = 0;
			m_flLastTimeLearned = 0;
		}

	public:
		CUtlString     m_sLessonName;
		double         m_flLastTimeShowed;
		int            m_iTimesLearned;
		int            m_iTimesShown;
		double         m_flLastTimeLearned;
	};

	class LessonPriorityLess
	{
	public:
		typedef C_SDKPlayer::CLessonProgress* LessonPointer;
		bool Less( const LessonPointer& lhs, const LessonPointer& rhs, void *pCtx );
	};

public:
	DECLARE_CLASS( C_SDKPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_SDKPlayer();
	~C_SDKPlayer();

	virtual void PreThink();
	virtual void UpdateCurrentTime();
	virtual void UpdateViewBobRamp();
	virtual void UpdateThirdCamera(const Vector& vecEye, const QAngle& angEye);

	virtual void StartTouch( CBaseEntity *pOther );

	static C_SDKPlayer* GetLocalSDKPlayer();
	static C_SDKPlayer* GetLocalOrSpectatedPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual const Vector& GetRenderOrigin();
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual bool			PlayerUse( void );

	virtual void	GetStepSoundVelocities( float *velwalk, float *velrun );

	virtual float	GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );

	virtual void CalcVehicleView(IClientVehicle *pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov );
	virtual void CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual void CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual bool CalcFreezeCamThreeQuartersView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov, float flSide, Vector& vecCamTarget, Vector& vecCamPlayer, Vector& vecCamLookAt );
	virtual bool CalcFreezeCamHalfHalfView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov, float flSide, Vector& vecCamTarget, Vector& vecCamPlayer, Vector& vecCamLookAt );
	virtual bool CalcFreezeCamPortraitView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual bool CalcFreezeCamKillerAimInView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual Vector GetFreezeCamOrigin(C_BaseEntity* pTarget);

	virtual Vector  EyePosition();

	// Player avoidance
	bool ShouldCollide( int collisionGroup, int contentsMask ) const;
	void AvoidPlayers( CUserCmd *pCmd );
	float m_fNextThinkPushAway;
	virtual bool CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	virtual void ClientThink();
	virtual const QAngle& EyeAngles( void );

	// Have this player play the sounds from his view model's reload animation.
	void PlayReloadEffect();

	virtual void	UpdateFlashlight( void );
	void	TurnOffFlashlight( void );
	virtual const char *GetFlashlightTextureName( void ) const { return NULL; }
	virtual float GetFlashlightFOV( void ) const { return 0.0f; }
	virtual float GetFlashlightFarZ( void ) const { return 0.0f; }
	virtual float GetFlashlightLinearAtten( void ) const { return 0.0f; }
	virtual Vector GetFlashlightOrigin() const { return m_vecFlashlightOrigin; }
	virtual bool CastsFlashlightShadows( void ) const { return true; }
	virtual void GetFlashlightOffset( const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, Vector *pVecOffset ) const;

	virtual bool				ShouldReceiveProjectedTextures( int flags )
	{
		return true;
	}

	// Called by shared code.
public:
	SDKPlayerState State_Get() const;
	
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual bool ShouldDraw();

	CWeaponSDKBase *GetActiveSDKWeapon() const;

	virtual	bool Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon);
	virtual C_BaseCombatWeapon* GetLastWeapon( void );

	virtual C_BaseAnimating * BecomeRagdollOnClient();
	virtual IRagdoll* GetRepresentativeRagdoll() const;

	virtual void ItemPostFrame( void );

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		SDKWeaponID eWeaponID,
		int iDamage, 
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y );

	void IncreaseShotsFired() { m_iShotsFired++; if (m_iShotsFired > 16) m_iShotsFired = 16; }
	void DecreaseShotsFired() { m_iShotsFired--; if (m_iShotsFired < 0) m_iShotsFired = 0; }
	void ClearShotsFired() { m_iShotsFired = 0; }
	int GetShotsFired() { return m_iShotsFired; }

	virtual void DoMuzzleFlash();
	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, bool bUseTracerAttachment );

	virtual void	FreezePlayer(float flAmount = 0, float flTime = -1);
	virtual bool	PlayerFrozen();

	virtual void    ReadyWeapon();
	virtual bool    IsWeaponReady();

	CWeaponSDKBase *FindWeapon (SDKWeaponID id);

	float GetStylePoints();
	float GetStyleSkillCharge() { return m_flStyleSkillCharge; }
	bool IsStyleSkillActive(SkillID eSkill = SKILL_NONE) const;
	void UseStyleCharge(SkillID eSkill, float flCharge);
	void DecayStyle();
	static void GetStyleStars(float flPoints, int& iGold, int& iSilver, int& iBronze);
	void GetStyleStars(int& iGold, int& iSilver, int& iBronze);

	virtual void SharedSpawn();
	
	void InitSpeeds( void ); //Tony; called EVERY spawn on server and client after class has been chosen (if any!)
	virtual float GetPlayerMaxSpeed(bool bDucking);
	virtual float GetPlayerMaxSpeed() { return GetPlayerMaxSpeed(false); }
	float GetMaxSlideSpeed();

//Tony; pronetodo!
//	void CheckProneMoveSound( int groundspeed, bool onground );

	// Returns true if the player is allowed to move.
	bool CanMove() const;

	// Returns true if the player is allowed to attack.
	bool CanAttack( void );

#if defined ( SDK_USE_SPRINTING )
	void SetSprinting( bool bIsSprinting );
	bool IsSprinting( void );
#endif

	void ActivateSlowMo();
	void ActivateSuperfallSlowMo();
	void DeactivateSlowMo();
	float GetSlowMoMultiplier() const;
	float GetSlowMoGoal() const;
	float GetSlowMoSeconds() const { return m_flSlowMoSeconds; }
	float GetSlowMoTime() const { return m_flSlowMoTime; }
	bool HasSuperSlowMo() const { return m_bHasSuperSlowMo; }
	int GetSlowMoType() const { return m_iSlowMoType; }

	bool HasPlayerDied() const { return m_bHasPlayerDied; }

	bool IsInThirdPerson() const;
	const Vector CalculateThirdPersonCameraPosition(const Vector& vecEye, const QAngle& angCamera);
	const Vector GetThirdPersonCameraPosition();
	const Vector GetThirdPersonCameraTarget();

	bool HasBriefcase() const;

	int GetRaceWaypoint() const { return m_iRaceWaypoint; }

	const char* GetCharacter() const { return m_iszCharacter; }
	bool HasCharacterBeenChosen() const { return m_bCharacterChosen; }
	void CharacterChosen() { m_bCharacterChosen = true; }
	bool HasSkillsTabBeenSeen() const { return m_bSkillsSeen; }
	void SkillsTabSeen() { m_bSkillsSeen = true; }

	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	CSDKPlayerShared m_Shared;
	
	virtual const Vector	GetPlayerMins( void ) const; // uses local player
	virtual const Vector	GetPlayerMaxs( void ) const; // uses local player

	float GetUserInfoFloat(const char* pszCVar, float flBotDefault = 0);
	int GetUserInfoInt(const char* pszCVar, int iBotDefault = 0);

// Not Shared, but public.
public:

#if defined ( SDK_USE_TEAMS )
	bool CanShowTeamMenu();
#endif // SDK_USE_TEAMS

#if defined ( SDK_USE_PLAYERCLASSES )
	bool CanShowClassMenu();
#endif // SDK_USE_PLAYERCLASSES

	bool CanShowBuyMenu();
	bool CanShowSkillMenu();

	bool CanAddToLoadout(SDKWeaponID eWeapon);
	int GetLoadoutWeaponCount(SDKWeaponID eWeapon);
	int GetLoadoutWeight() { return m_iLoadoutWeight; }

	virtual void	Instructor_Initialize();
	virtual void	Instructor_Think();
	virtual void	Instructor_Respawn();
	virtual void	Instructor_Reset();

	virtual void	Instructor_LessonLearned(const CUtlString& sLesson);
	virtual void	Instructor_LessonShowed(const CUtlString& sLesson);
	virtual bool	Instructor_IsLessonLearned(const CLessonProgress* pLessonProgress);
	virtual int     Instructor_GetLessonTrainings(const CUtlString& sLesson);
	virtual bool	Instructor_IsLessonValid(const CLessonProgress* pLessonProgress);
	virtual class CLessonProgress*  Instructor_GetBestLesson();
	CInstructor*    GetInstructor() { return m_pInstructor; }

	void LocalPlayerRespawn( void );

	//Tony; update lookat, if our model has moving eyes setup, they need to be updated.
	void			UpdateLookAt( void );
	int				GetIDTarget() const;
	void			UpdateIDTarget( void );

	//Tony; when model is changed, need to init some stuff.
	virtual CStudioHdr *OnNewModel( void );
	void InitializePoseParams( void );

	virtual void				OverrideView( CViewSetup *pSetup );

	virtual void	UpdateTeamMenu( void );

	float			GetCurrentTime() const { return m_flCurrentTime; }

	float           GetLastSpawnTime() const { return m_flLastSpawnTime; }

	CSDKPlayer*     GetKiller() const { return m_hKiller; }
	CBaseEntity*    GetInflictor() const { return m_hInflictor; }
	bool            WasKilledByExplosion() const { return m_bWasKilledByExplosion; }
	Vector          GetKillingExplosionPosition() const { return m_vecKillingExplosionPosition; }
	bool            WasKilledByGrenade() const { return m_bWasKilledByGrenade; }
	bool            WasKilledByBrawl() const { return m_bWasKilledByBrawl; }
	const char*     GetKillerString() const { return m_bWasKilledByString?m_szKillerString:NULL; }

	int             GetCoderHacksButtons() const { return m_nCoderHacksButtons; }

	bool UseVRHUD() const;

public: // Public Variables
	CSDKPlayerAnimState *m_PlayerAnimState;
#if defined ( SDK_USE_PRONE )
	bool m_bUnProneToDuck;		//Tony; GAMEMOVEMENT USED VARIABLE
#endif // SDK_USE_PRONE

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	CNetworkVar( float, m_flFreezeUntil );
	CNetworkVar( float, m_flFreezeAmount );

	CNetworkVar( float, m_flReadyWeaponUntil );

	CNetworkVar( float, m_flDisarmRedraw );
	
	EHANDLE	m_hRagdoll;

	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	Vector m_vLookAtTarget;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	int	  m_iIDEntIndex;
	int GetArmorValue() { return m_ArmorValue; }

	float m_flMuzzleFlashYaw;

private:
	void UpdateSoundEvents();

	CNetworkVar( bool, m_bSpawnInterpCounter );
	bool m_bSpawnInterpCounterCache;

	CNetworkVar( SDKPlayerState, m_iPlayerState );

	C_SDKPlayer( const C_SDKPlayer & );

	int m_ArmorValue;

	float m_flStylePoints;
	float m_flStyleSkillCharge;
	float m_currentAlphaVal;

	class CSDKSoundEvent
	{
	public:
		string_t m_SoundName;
		float m_flEventTime;	// Play the event when gpGlobals->curtime goes past this.
	};
	CUtlLinkedList<CSDKSoundEvent,int> m_SoundEvents;

	CArmament		m_aLoadout[MAX_LOADOUT];
	int				m_iLoadoutWeight;

	CNetworkVar( int, m_iSlowMoType );
	CNetworkVar( bool, m_bHasSuperSlowMo );
	CNetworkVar( float, m_flSlowMoSeconds );
	CNetworkVar( float, m_flSlowMoTime );
	CNetworkVar( float, m_flSlowMoMultiplier );
	float m_flLastSlowMoMultiplier;

	CNetworkVar( float, m_flCurrentTime );		// Accounts for slow motion

	CNetworkVar( float, m_flLastSpawnTime );

	CNetworkVar( bool, m_bHasPlayerDied );

	CNetworkVar( bool, m_bThirdPerson );
	CNetworkVar( bool, m_bThirdPersonCamSide );

	CNetworkVar( bool, m_bUsingVR );

	float m_flCurrentAlphaVal;

	Vector m_vecThirdCamera; // Where is the third person camera?
	Vector m_vecThirdTarget; // Where is the third person camera pointing?
	Vector m_vecCameraLag;
	float  m_flCameraLerp;
	float  m_flStuntLerp;
	float  m_flSideLerp;

	char m_iszCharacter[256];
	bool m_bCharacterChosen;
	bool m_bSkillsSeen;

	CNetworkHandle(CSDKPlayer, m_hKiller);
	CNetworkHandle(CBaseEntity, m_hInflictor);
	bool m_bWasKilledByExplosion;
	Vector m_vecKillingExplosionPosition;
	bool m_bWasKilledByGrenade;
	bool m_bWasKilledByBrawl;
	bool m_bWasKilledByString;
	char m_szKillerString[256];

	CProjectedLightEffect *m_pProjectedFlashlight;
	bool			m_bFlashlightEnabled;
	Vector	m_vecFlashlightOrigin;
	Vector	m_vecFlashlightForward;
	Vector	m_vecFlashlightUp;
	Vector	m_vecFlashlightRight;

	CNewParticleEffect* m_hLeftArmGlow;
	CNewParticleEffect* m_hRightArmGlow;
	CNewParticleEffect* m_hLeftFootGlow;
	CNewParticleEffect* m_hRightFootGlow;

	class CInstructor*                     m_pInstructor;
	CUtlMap<CUtlString, CLessonProgress>   m_apLessonProgress;
	CUtlSortVector<CLessonProgress*, LessonPriorityLess> m_apLessonPriorities;
	float                                  m_flLastLesson;

	CNetworkHandle(class C_Briefcase, m_hBriefcase);
	int     m_iRaceWaypoint;

	bool    m_bCoderHacks;
	int     m_nCoderHacksButtons;

public:
	CNetworkHandle(CWeaponSDKBase, m_hSwitchFrom);
};


inline C_SDKPlayer* ToSDKPlayer( CBaseEntity *pPlayer )
{
	if ( !pPlayer || !pPlayer->IsPlayer() )
		return NULL;

	return static_cast< C_SDKPlayer* >( pPlayer );
}

inline SDKPlayerState C_SDKPlayer::State_Get() const
{
	return m_iPlayerState;
}

#endif // C_SDK_PLAYER_H

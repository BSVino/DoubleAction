//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for SDK Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_PLAYER_H
#define SDK_PLAYER_H
#pragma once

#include "basemultiplayerplayer.h"
#include "server_class.h"
#include "sdk_playeranimstate.h"
#include "sdk_player_shared.h"

#include "da.h"

// Function table for each player state.
class CSDKPlayerStateInfo
{
public:
	SDKPlayerState m_iPlayerState;
	const char *m_pStateName;
	
	void (CSDKPlayer::*pfnEnterState)();	// Init and deinit the state.
	void (CSDKPlayer::*pfnLeaveState)();
	void (CSDKPlayer::*pfnPreThink)();	// Do a PreThink() in this state.
};

extern ConVar bot_mimic;

//=============================================================================
// >> SDK Game player
//=============================================================================
class CSDKPlayer : public CBaseMultiplayerPlayer
{
public:
	DECLARE_CLASS( CSDKPlayer, CBaseMultiplayerPlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	typedef enum
	{
		VIS_NONE		= 0x00,
		VIS_GUT			= 0x01,
		VIS_HEAD		= 0x02,
		VIS_LEFT_SIDE	= 0x04,			///< the left side of the object from our point of view (not their left side)
		VIS_RIGHT_SIDE	= 0x08,			///< the right side of the object from our point of view (not their right side)
		VIS_FEET		= 0x10
	} VisiblePartType;

	struct PartInfo
	{
		Vector m_headPos;											///< current head position
		Vector m_gutPos;											///< current gut position
		Vector m_feetPos;											///< current feet position
		Vector m_leftSidePos;										///< current left side position
		Vector m_rightSidePos;										///< current right side position
		int m_validFrame;											///< frame of last computation (for lazy evaluation)
	};

	CSDKPlayer();
	~CSDKPlayer();

	static CSDKPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CSDKPlayer* Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	virtual void FlashlightTurnOn( void );
	virtual void FlashlightTurnOff( void );
	virtual int FlashlightIsOn( void );

	virtual void PreThink();
	virtual void PostThink();
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void UpdateCurrentTime();
	virtual void UpdateViewBobRamp();
	virtual void UpdateThirdCamera(const Vector& vecEye, const QAngle& angEye);

	virtual void OnDive();

	virtual void StartTouch( CBaseEntity *pOther );

	virtual void GiveDefaultItems();

	virtual bool			PlayerUse( void );

	virtual void			GetStepSoundVelocities( float *velwalk, float *velrun );

	// Animstate handles this.
	void SetAnimation( PLAYER_ANIM playerAnim ) { return; }

	virtual bool		CanHearAndReadChatFrom( CBasePlayer *pPlayer );

	virtual void Precache();
	virtual int			OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr );
	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );
	virtual void        OnDamagedByExplosion( const CTakeDamageInfo &info );

	virtual bool        FVisible(CBaseEntity* pEntity, int iTraceMask = MASK_OPAQUE, CBaseEntity** ppBlocker = NULL);
	virtual bool        IsVisible(const Vector &pos, bool testFOV = false, const CBaseEntity *ignore = NULL) const;	///< return true if we can see the point
	virtual bool        IsVisible(CSDKPlayer* pPlayer, bool testFOV = false, unsigned char* visParts = NULL) const;
	virtual Vector      GetPartPosition(CSDKPlayer* player, VisiblePartType part) const;	///< return world space position of given part on player
	virtual void        ComputePartPositions(CSDKPlayer *player);					///< compute part positions from bone location
	virtual Vector      GetCentroid() const;
	virtual CSDKPlayer* FindClosestFriend(float flMaxDistance, bool bFOV = true);
	virtual bool        RunMimicCommand( CUserCmd& cmd );
	virtual bool        IsReloading( void ) const;

	void         AwardStylePoints(CSDKPlayer* pVictim, bool bKilledVictim, const CTakeDamageInfo &info);
	void         SendAnnouncement(announcement_t eAnnouncement, style_point_t ePointStyle);
	void         SendNotice(notice_t eNotice);

	virtual int		TakeHealth( float flHealth, int bitsDamageType );
	virtual int		GetMaxHealth()  const;
	
	CWeaponSDKBase* GetActiveSDKWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual	bool			Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 );		// Switch to given weapon if has ammo (false if failed)
	virtual void Weapon_Equip( CBaseCombatWeapon *pWeapon );		//Tony; override so diethink can be cleared
	virtual bool ThrowActiveWeapon( bool bAutoSwitch = true );
	virtual	bool Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon);
	virtual CBaseCombatWeapon* GetLastWeapon( void );

	virtual Vector  EyePosition();

	virtual void    ImpulseCommands( void );
	virtual void	CheatImpulseCommands( int iImpulse );
	
	virtual int		SpawnArmorValue( void ) const { return m_iSpawnArmorValue; }
	virtual void	SetSpawnArmorValue( int i ) { m_iSpawnArmorValue = i; }

	virtual void	PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	virtual float	GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );

	virtual void	Instructor_LessonLearned(const char* pszLesson);

	CWeaponSDKBase *findweapon (SDKWeaponID id);

	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 
#if defined ( SDK_USE_PLAYERCLASSES )
	int GetPlayerClassAsString( char *pDest, int iDestSize );
	void SetClassMenuOpen( bool bIsOpen );
	bool IsClassMenuOpen( void );
#endif

	void SetCharacterMenuOpen( bool bIsOpen );
	bool IsCharacterMenuOpen( void );
	void ShowCharacterMenu();

	void SetBuyMenuOpen( bool bIsOpen );
	bool IsBuyMenuOpen( void );
	void ShowBuyMenu();

	void ClearLoadout();
	bool CanAddToLoadout(SDKWeaponID eWeapon);
	void AddToLoadout(SDKWeaponID eWeapon);
	void RemoveFromLoadout(SDKWeaponID eWeapon);
	void CountLoadoutWeight();
	int GetLoadoutWeaponCount(SDKWeaponID eWeapon);
	void BuyRandom();

	bool PickRandomCharacter();
	void PickRandomSkill();

	void SelectItem( const char *pstr, int iSubType = 0 );

	void SetSkillMenuOpen( bool bIsOpen );
	bool IsSkillMenuOpen( void );
	void ShowSkillMenu();

	void SetStyleSkill(SkillID eSkill);

	/*These 3 were made virtual for Double Action.
	They enable us to have new shadow hulls.*/
	virtual void SetupVPhysicsShadow( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, CPhysCollide *pStandModel, const char *pStandHullName, CPhysCollide *pCrouchModel, const char *pCrouchHullName );
	virtual void SetVCollisionState( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, int collisionState );
	virtual void PostThinkVPhysics (void);
	virtual void VPhysicsDestroyObject (void); 
	IPhysicsObject *shadow_slide;
	IPhysicsObject *shadow_dive;


	void PhysObjectSleep();
	void PhysObjectWake();

	virtual void ChangeTeam( int iTeamNum );

	// Player avoidance
	virtual	bool		ShouldCollide( int collisionGroup, int contentsMask ) const;
	void SDKPushawayThink(void);

	virtual CBaseEntity*	GiveNamedItem( const char *szName, int iSubType = 0 );
	virtual bool			BumpWeapon( CBaseCombatWeapon *pWeapon );

	virtual void Disarm();

	virtual void ThirdPersonToggle();
	virtual void ThirdPersonSwitchSide();
	virtual bool IsInThirdPerson() const;
	const Vector CalculateThirdPersonCameraPosition(const Vector& vecEye, const QAngle& angCamera);
	const Vector GetThirdPersonCameraPosition();
	const Vector GetThirdPersonCameraTarget();

	bool InSameTeam( CBaseEntity *pEntity ) const;	// Returns true if the specified entity is on the same team as this one

// In shared code.
public:
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

	virtual void DoMuzzleFlash();
	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

	CNetworkVarEmbedded( CSDKPlayerShared, m_Shared );
	virtual void			PlayerDeathThink( void );
	virtual bool		ClientCommand( const CCommand &args );

	void IncreaseShotsFired() { m_iShotsFired++; if (m_iShotsFired > 16) m_iShotsFired = 16; }
	void DecreaseShotsFired() { m_iShotsFired--; if (m_iShotsFired < 0) m_iShotsFired = 0; }
	void ClearShotsFired() { m_iShotsFired = 0; }
	int GetShotsFired() { return m_iShotsFired; }

	virtual void	FreezePlayer(float flAmount = 0, float flTime = -1);
	virtual bool	PlayerFrozen();

	virtual void    ReadyWeapon();
	virtual bool    IsWeaponReady();

#if defined ( SDK_USE_SPRINTING )
	void SetSprinting( bool bIsSprinting );
#endif // SDK_USE_SPRINTING
	// Returns true if the player is allowed to attack.
	bool CanAttack( void );

	virtual int GetPlayerStance();

	// Called whenever this player fires a shot.
	void NoteWeaponFired();
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;

	float GetUserInfoFloat(const char* pszCVar, float flBotDefault = 0);
	int GetUserInfoInt(const char* pszCVar, int iBotDefault = 0);

// ------------------------------------------------------------------------------------------------ //
// Player state management.
// ------------------------------------------------------------------------------------------------ //
public:

	void State_Transition( SDKPlayerState newState );
	SDKPlayerState State_Get() const;				// Get the current state.

	virtual bool	ModeWantsSpectatorGUI( int iMode ) { return ( iMode != OBS_MODE_DEATHCAM && iMode != OBS_MODE_FREEZECAM ); }

	// Universal Meter
	float GetStylePoints() { return m_flStylePoints; }
	void AddStylePoints(float points, style_point_t eStyle);
	void SetStylePoints(float points);
	bool UseStylePoints();
	bool IsStyleSkillActive(SkillID eSkill = SKILL_NONE) const;
	void UseStyleCharge(SkillID eSkill, float flCharge);

	void ActivateMeter();

	bool SetCharacter(const char* pszCharacter);

	void ActivateSlowMo();
	float GetSlowMoMultiplier() const;
	float GetSlowMoGoal() const;
	int GetSlowMoType() const { return m_iSlowMoType; }
	void SetSlowMoType(int iType);
	void GiveSlowMo(float flSeconds);

	float GetCurrentTime() const { return m_flCurrentTime; }

private:
	bool SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot );

	void State_Enter( SDKPlayerState newState );	// Initialize the new state.
	void State_Leave();								// Cleanup the previous state.
	void State_PreThink();							// Update the current state.

	// Specific state handler functions.
	void State_Enter_WELCOME();
	void State_PreThink_WELCOME();

	void State_Enter_MAPINFO();
	void State_PreThink_MAPINFO();

	void State_Enter_PICKINGTEAM();
	void State_Enter_PICKINGCLASS();

	void State_Enter_PICKINGCHARACTER();

	void State_Enter_BUYINGWEAPONS();

	void State_Enter_PICKINGSKILL();

public: //Tony; I had this private but I need it public for initial spawns.
	void MoveToNextIntroCamera();
private:

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();

	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();

	void State_Enter_DEATH_ANIM();
	void State_PreThink_DEATH_ANIM();

	// Find the state info for the specified state.
	static CSDKPlayerStateInfo* State_LookupInfo( SDKPlayerState state );

	// This tells us which state the player is currently in (joining, observer, dying, etc).
	// Each state has a well-defined set of parameters that go with it (ie: observer is movetype_noclip, non-solid,
	// invisible, etc).
	CNetworkVar( SDKPlayerState, m_iPlayerState );

	// Universal Meter
	CNetworkVar(float, m_flStylePoints);
	CNetworkVar(float, m_flStyleSkillCharge);

	CSDKPlayerStateInfo *m_pCurStateInfo;			// This can be NULL if no state info is defined for m_iPlayerState.
	bool HandleCommand_JoinTeam( int iTeam );
#if defined ( SDK_USE_TEAMS )
	bool	m_bTeamChanged;		//have we changed teams this spawn? Used to enforce one team switch per death rule
#endif

#if defined ( SDK_USE_PLAYERCLASSES )
	bool HandleCommand_JoinClass( int iClass );
	void ShowClassSelectMenu();
	bool m_bIsClassMenuOpen;
#endif

	bool m_bIsCharacterMenuOpen;

	bool m_bIsBuyMenuOpen;
	bool m_bIsSkillMenuOpen;

#if defined ( SDK_USE_PRONE )
	void InitProne( void );
#endif // SDK_USE_PRONE

#if defined ( SDK_USE_SPRINTING )
	void InitSprinting( void );
	bool IsSprinting( void );
	bool CanSprint( void );
#endif // SDK_USE_SPRINTING

	void InitSpeeds( void ); //Tony; called EVERY spawn on server and client after class has been chosen (if any!)

	// from CBasePlayer
	void			SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );

	CBaseEntity*	EntSelectSpawnPoint();
	bool			CanMove( void ) const;

	virtual void	SharedSpawn();

	virtual const Vector	GetPlayerMins( void ) const; // uses local player
	virtual const Vector	GetPlayerMaxs( void ) const; // uses local player

public:
	virtual void		CommitSuicide( bool bExplode = false, bool bForce = false );

private:
	// Last usercmd we shot a bullet on.
	int m_iLastWeaponFireUsercmd;

	virtual void SDKThrowWeapon( CWeaponSDKBase *pWeapon, const Vector &vecForward, const QAngle &vecAngles, float flDiameter  );
	virtual void SDKThrowWeaponDir( CWeaponSDKBase *pWeapon, const Vector &vecForward, Vector *pVecThrowDir );
	// When the player joins, it cycles their view between trigger_camera entities.
	// This is the current camera, and the time that we'll switch to the next one.
	EHANDLE m_pIntroCamera;
	float m_fIntroCamTime;

	void CreateRagdollEntity();
	void DestroyRagdoll( void );


	CSDKPlayerAnimState *m_PlayerAnimState;

	CNetworkVar( bool, m_bSpawnInterpCounter );

	int m_iSpawnArmorValue;
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_ArmorValue );

	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

	bool   m_bDamagedEnemyDuringDive;

public:
#if defined ( SDK_USE_PRONE )
	bool m_bUnProneToDuck;		//Tony; GAMEMOVEMENT USED VARIABLE
#endif // SDK_USE_PRONE

	CNetworkArray( CArmament, m_aLoadout, MAX_LOADOUT );
	CNetworkVar( int, m_iLoadoutWeight );

	CNetworkVar( float, m_flFreezeUntil );
	CNetworkVar( float, m_flFreezeAmount );

	CNetworkVar( float, m_flReadyWeaponUntil );

	float       m_flNextRegen;
	float       m_flNextHealthDecay;

	CNetworkVar( float, m_flDisarmRedraw );

	CNetworkVar( int, m_iSlowMoType );
	CNetworkVar( bool, m_bHasSuperSlowMo );
	CNetworkVar( float, m_flSlowMoSeconds );
	CNetworkVar( float, m_flSlowMoTime );
	CNetworkVar( float, m_flSlowMoMultiplier );

	CNetworkVar( float, m_flCurrentTime );		// Accounts for slow motion

	CNetworkVar( float, m_flLastSpawnTime );

	CNetworkVar( bool, m_bHasPlayerDied );

	CNetworkVar( bool, m_bThirdPerson );
	CNetworkVar( bool, m_bThirdPersonCamSide );

#ifdef CLIENT_DLL
	float m_flCurrentAlphaVal; // keeps track of the current alpha value for the player model
#endif

	Vector m_vecThirdCamera; // Where is the third person camera?
	Vector m_vecThirdTarget; // Where is the third person camera pointing?
	float  m_flCameraLerp;
	float  m_flStuntLerp;
	float  m_flSideLerp;

	int    m_iStyleKillStreak;

	CNetworkVar( string_t, m_iszCharacter );

	CWeaponSDKBase *switchfrom;

protected:
	static PartInfo m_partInfo[ MAX_PLAYERS ];						///< part positions for each player
};


inline CSDKPlayer *ToSDKPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CSDKPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CSDKPlayer* >( pEntity );
}

inline SDKPlayerState CSDKPlayer::State_Get() const
{
	return m_iPlayerState;
}

#endif	// SDK_PLAYER_H

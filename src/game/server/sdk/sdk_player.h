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

	void         AwardStylePoints(CSDKPlayer* pVictim, bool bKilledVictim, const CTakeDamageInfo &info);
	void         SendAnnouncement(announcement_t eAnnouncement, style_point_t ePointStyle);

	virtual int		TakeHealth( float flHealth, int bitsDamageType );
	virtual int		GetMaxHealth()  const;
	
	CWeaponSDKBase* GetActiveSDKWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual	bool			Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 );		// Switch to given weapon if has ammo (false if failed)
	virtual void Weapon_Equip( CBaseCombatWeapon *pWeapon );		//Tony; override so diethink can be cleared
	virtual bool ThrowActiveWeapon( bool bAutoSwitch = true );
	virtual	bool Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon);

	virtual Vector  EyePosition();

	virtual void	CheatImpulseCommands( int iImpulse );
	
	virtual int		SpawnArmorValue( void ) const { return m_iSpawnArmorValue; }
	virtual void	SetSpawnArmorValue( int i ) { m_iSpawnArmorValue = i; }

	virtual void	PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	virtual float	GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );

	virtual void	Instructor_LessonLearned(const char* pszLesson);

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

	void SelectItem( const char *pstr, int iSubType = 0 );

	void SetSkillMenuOpen( bool bIsOpen );
	bool IsSkillMenuOpen( void );
	void ShowSkillMenu();

	void SetStyleSkill(SkillID eSkill);

	void PhysObjectSleep();
	void PhysObjectWake();

#if defined ( SDK_USE_TEAMS )
	virtual void ChangeTeam( int iTeamNum );
#endif
	// Player avoidance
	virtual	bool		ShouldCollide( int collisionGroup, int contentsMask ) const;
	void SDKPushawayThink(void);

	virtual CBaseEntity*	GiveNamedItem( const char *szName, int iSubType = 0 );
	virtual bool			BumpWeapon( CBaseCombatWeapon *pWeapon );

	virtual void Disarm();

	virtual void ThirdPersonToggle();
	virtual bool IsInThirdPerson() const { return m_bThirdPerson; }
	const Vector GetThirdPersonCameraPosition(const Vector& vecEye, const QAngle& angCamera);

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
	bool IsStyleSkillActive() const;
	void UseStyleCharge(float flCharge);

	void ActivateMeter();

	void SetCharacter(const char* pszCharacter) { m_pszCharacter = pszCharacter; }

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
	const char* m_pszCharacter;

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

public:
#if defined ( SDK_USE_PRONE )
	bool m_bUnProneToDuck;		//Tony; GAMEMOVEMENT USED VARIABLE
#endif // SDK_USE_PRONE

	CNetworkArray( CArmament, m_aLoadout, MAX_LOADOUT );
	CNetworkVar( int, m_iLoadoutWeight );

	CNetworkVar( float, m_flFreezeUntil );
	CNetworkVar( float, m_flFreezeAmount );

	CNetworkVar( float, m_flReadyWeaponUntil );

	float		m_flNextRegen;
	float		m_flNextHealthDecay;
	float		m_flNextSecondWindRegen;

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

	int    m_iStyleKillStreak;
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

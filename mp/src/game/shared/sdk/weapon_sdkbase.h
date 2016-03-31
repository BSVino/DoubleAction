//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_SDKBASE_H
#define WEAPON_SDKBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "sdk_shareddefs.h"
#include "sdk_playeranimstate.h"
#include "sdk_weapon_parse.h"

#if defined( CLIENT_DLL )
	#define CWeaponSDKBase C_WeaponSDKBase
#endif

class CSDKPlayer;

inline const char *GetTranslatedWeaponAlias(const char *alias) { return alias; };

// These are the names of the ammo types that the weapon script files reference.
class CWeaponSDKBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponSDKBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponSDKBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif
	#ifdef CLIENT_DLL
       virtual bool ShouldPredict();
	#endif
	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
	virtual void	Precache( void );
	
	// Get SDK weapon specific weapon data.
	CSDKWeaponInfo const	&GetSDKWpnData() const;

	bool IsAkimbo() const;
	bool HasAkimbo() const;
	bool IsGrenade() const;

	virtual const char* GetViewModel( int viewmodelindex = 0 ) const;

	// Get a pointer to the player that owns this weapon
	CSDKPlayer* GetPlayerOwner() const;
	CSDKPlayer*	GetPrevOwner( void ) const { return m_pPrevOwner; }
	void		SetPrevOwner( CSDKPlayer *pOwner ) { m_pPrevOwner = pOwner; }

	// override to play custom empty sounds
	virtual bool PlayEmptySound();

	//Tony; these five functions return the sequences the view model uses for a particular action. -- You can override any of these in a particular weapon if you want them to do
	//something different, ie: when a pistol is out of ammo, it would show a different sequence.
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_PRIMARYATTACK;	}
	virtual Activity	GetIdleActivity( void ) { return ACT_VM_IDLE; }
	virtual Activity	GetDeployActivity( void ) { return ACT_VM_DRAW; }
	virtual Activity	GetReloadActivity( void ) { return ACT_VM_RELOAD; }
	virtual Activity	GetHolsterActivity( void ) { return ACT_VM_HOLSTER; }

	void					Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int						ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };

	virtual void			MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, bool bUseTracerAttachment );
	virtual void			WeaponIdle( void );
	virtual bool			Reload( void );
	virtual void            FinishReload();
	virtual	void			CheckReload( void );
	virtual bool			ReloadOrSwitchWeapons( void );
	virtual bool			Deploy();
	virtual bool			Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void			SendReloadEvents();
	virtual bool			HasWeaponIdleTimeElapsed( void );

	//Tony; added so we can have base functionality without implementing it into every weapon.
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	void FinishAttack (CSDKPlayer *pPlayer);

	virtual void      StartSwing(bool bIsSecondary, bool bIsStockAttack = false);
	virtual void      Swing();
	virtual void      Hit(trace_t &traceHit, bool bIsSecondary);
	virtual Activity  ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CSDKPlayer *pOwner );
	virtual	void      ImpactEffect( trace_t &trace );
	virtual bool      ImpactWater( const Vector &start, const Vector &end );

	virtual float	GetMeleeRange( void )								{	return	80.0f;	}
	virtual float	GetMeleeDamage( bool bSecondary, CSDKPlayer* pVictim ) const;

	virtual bool      IsThrowingGrenade() const { return m_flGrenadeThrowStart > 0; }
	virtual void      StartGrenadeToss();
	virtual bool      MaintainGrenadeToss();
	virtual float     GetGrenadeThrowStart() const { return m_flGrenadeThrowStart; }
	virtual float     GetGrenadeThrowWeaponHolsterTime() const; // What time does the weapon finish holstering
	virtual float     GetGrenadeThrowWeaponDeployTime() const;  // What time does the weapon start re-deploying
	virtual float     GetGrenadeThrowEnd() const;               // What time does the weapon finish re-deploying
	virtual void      GetGrenadeThrowVectors(Vector& vecSrc, Vector& vecThrow, QAngle& angThrow);

	virtual void			AddViewKick( void );	// Add in the view kick for the weapon
	virtual void			AddMeleeViewKick();

	virtual bool			CanBeSelected( void );

#ifdef CLIENT_DLL
	virtual void			CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles );
#endif

#ifdef CLIENT_DLL
	virtual int     DrawModel( int flags );
	virtual bool    ShouldDrawCrosshair( void );

	float m_flUseHighlight;
	float m_flUseHighlightGoal;

	static void    DrawVRBullets(const Vector& vecAmmo1, const Vector& vecAmmo2, int iClip, int iMaxClip, bool bRight);
	static void    VRBulletFired(int iRound, bool bRight);
	static Vector  GetVRRoundPosition(int i, bool bRight);

	class CFlyingRound
	{
	public:
		bool         bActive;
		Vector       vecPosition;
		Vector       vecVelocity;
		float        flAngle;
		float        flAngularVelocity;
		float        flAlpha;
	};

	static Vector s_vecAmmo1L, s_vecAmmo2L, s_vecAmmo1R, s_vecAmmo2R;
	static int s_iMaxClip;
	static CUtlVector<CFlyingRound> s_aRounds;

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink();
	float GetMarksmanGold();

	EHANDLE m_hLastOwner;

#endif

	virtual float GetWeaponSpread();
	virtual bool WeaponSpreadFixed() const { return false; }

	//Tony; by default, all weapons are automatic.
	//If you add code to switch firemodes, this function would need to be overridden to return the correct current mode.
	virtual int GetFireMode() const { return FM_AUTOMATIC; }

	virtual float GetFireRate( void ) { return m_flCycleTime; };
	virtual float GetSecondaryFireRate( void ) { return GetSDKWpnData().m_flSecondaryCycleTime; };

	virtual float GetBrawlFireRate( void );
	virtual float GetBrawlSecondaryFireRate( void );

	virtual bool  IsFullAuto() const { return GetFireMode() == FM_AUTOMATIC; }

	//Tony; by default, burst fire weapons use a max of 3 shots (3 - 1)
	//weapons with more, ie: a 5 round burst, can override and determine which firemode it's in.
	virtual int MaxBurstShots() const { return 2; }
	
	float GetWeaponFOV()
	{
		return GetSDKWpnData().m_flWeaponFOV;
	}
#ifdef GAME_DLL
	void SetDieThink( bool bDie );
	void Die( void );
	void SetWeaponModelIndex( const char *pName )
	{
 		 m_iWorldModelIndex = modelinfo->GetModelIndex( pName );
	}
#endif

	virtual bool CanWeaponBeDropped() const { return true; }
	virtual bool AllowsAutoSwitchFrom() const { return false; }

	virtual float GetViewPunchMultiplier();
	virtual float GetRecoil();
	virtual bool HasAimInSpeedPenalty();
	virtual bool HasAimInFireRateBonus();
	virtual bool HasAimInRecoilBonus();

	virtual const Vector GetShootPosition(CSDKPlayer* pShooter);

	virtual weapontype_t GetWeaponType() const;
	static weapontype_t  GetWeaponType( SDKWeaponID eWeapon );

	virtual bool FullAimIn() { return false; }

	float	GetCurrentTime() const;

	float GetSwingTime() const { return m_flSwingTime; }

private:

	CNetworkVar(float, m_flDecreaseShotsFired);

	CWeaponSDKBase( const CWeaponSDKBase & );

	CNetworkVar(float, m_flAccuracyDecay);
	
	CNetworkVar(float, m_flReloadEndTime);

	// server must enforce these values
	CNetworkVar(float, m_flCycleTime);
	CNetworkVar(float, m_flViewPunchMultiplier);
	CNetworkVar(float, m_flRecoil);
	CNetworkVar(float, m_flSpread);

	CNetworkVar(float, m_flSwingTime);
	CNetworkVar(bool, m_bSwingSecondary);
	CNetworkVar(float, m_flUnpauseFromSwingTime);
	CNetworkVar(float, m_flNextBrawlTime);

	CNetworkVar(float, m_flGrenadeThrowStart);
	bool m_bGrenadeThrown;

	CSDKPlayer *m_pPrevOwner;

#ifdef CLIENT_DLL
	float       m_flArrowGoalSize;
	float       m_flArrowCurSize;
	float       m_flArrowSpinOffset;

	float m_flMarksmanGold;
#endif

public:
	/*TODO: These should be moved into the akimbo class, but for some reason
	I can't get them to synch properly there.*/
	CNetworkVar(int, m_iLeftClip);
	CNetworkVar(int, m_iRightClip);
	CNetworkVar(bool, m_bShootRight);
};


#endif // WEAPON_SDKBASE_H

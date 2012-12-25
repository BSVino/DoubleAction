//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
	
	// Get SDK weapon specific weapon data.
	CSDKWeaponInfo const	&GetSDKWpnData() const;

	// Get a pointer to the player that owns this weapon
	CSDKPlayer* GetPlayerOwner() const;

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

	virtual void			WeaponIdle( void );
	virtual bool			Reload( void );
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

	virtual void      Swing(bool bIsSecondary);
	virtual void      Hit(trace_t &traceHit, bool bIsSecondary);
	virtual Activity  ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CSDKPlayer *pOwner );
	virtual	void      ImpactEffect( trace_t &trace );
	virtual bool      ImpactWater( const Vector &start, const Vector &end );

	virtual float	GetMeleeRange( void )								{	return	60.0f;	}
	virtual float	GetMeleeDamage( bool bSecondary ) const;

	virtual void			AddViewKick( void );	// Add in the view kick for the weapon
	virtual void			AddMeleeViewKick();

	virtual bool			CanBeSelected( void );

#ifdef CLIENT_DLL
	virtual void			CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles );
#endif

	virtual float GetWeaponSpread();
	virtual bool WeaponSpreadFixed() const { return false; }

	//Tony; by default, all weapons are automatic.
	//If you add code to switch firemodes, this function would need to be overridden to return the correct current mode.
	virtual int GetFireMode() const { return FM_AUTOMATIC; }

	virtual float GetFireRate( void ) { return GetSDKWpnData().m_flCycleTime; };
	virtual float GetSecondaryFireRate( void ) { return GetSDKWpnData().m_flSecondaryCycleTime; };

	virtual float GetBrawlFireRate( void );
	virtual float GetBrawlSecondaryFireRate( void );

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

	virtual weapontype_t GetWeaponType() const;

	virtual bool FullAimIn() { return false; }

	float	GetCurrentTime() const;

private:

	CNetworkVar(float, m_flDecreaseShotsFired);

	CWeaponSDKBase( const CWeaponSDKBase & );

	CNetworkVar(float, m_flAccuracyDecay);
};


#endif // WEAPON_SDKBASE_H

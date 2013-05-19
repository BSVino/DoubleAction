#ifndef WEAPON_AKIMBOBASE_H
#define WEAPON_AKIMBOBASE_H
#ifdef _WIN32
#pragma once
#endif

/*Currently akimbos are seperate weapons from their single variants.
Each pistol has its own ammo counter, and when one becomes empty before the
other then only the one with ammo will be fired. The right pistol has its 
ammo synched with the single pistol, and because of this akimbos should 
always give the player an equivalent single pistol. When players throw
akimbos two unique single variants are created, each one has its ammo
set to the left/right counters of the akimbos. When in use, the combined 
total of the left and right counters are packed into m_iClip1 for display
purposes.

The current implementation is not optimal and is implemented in several 
different places. It would be better to have a single weapon that functions
logically as both single and akimbo and have all the code implemented on it,
but this would require a deal of extra work from what I know.

sdk_player.cpp for drop functional
dab_viewmodel.cpp for muzzle flashes
weapon_sdkbase.cpp for pick up behaviour
(probably another one or two as well)

It would be wise to work toward a more unified approach, and one that
supplants this baseclass entirely.*/
#include "weapon_sdkbase.h"
#include "ai_activity.h"
enum
{/*ActivityOverride expects these in this order*/
	ACT_DAB_AKIMBO_RIGHT = LAST_SHARED_ACTIVITY,
	ACT_DAB_AKIMBO_CROUCH_RIGHT,
	ACT_DAB_AKIMBO_PRONE_RIGHT,
	ACT_DAB_AKIMBO_SLIDE_RIGHT,
	ACT_DAB_AKIMBO_DIVE_RIGHT,
	ACT_DAB_AKIMBO_ROLL_RIGHT,
	ACT_DAB_AKIMBO_LEFT,
	ACT_DAB_AKIMBO_CROUCH_LEFT,
	ACT_DAB_AKIMBO_PRONE_LEFT,
	ACT_DAB_AKIMBO_SLIDE_LEFT,
	ACT_DAB_AKIMBO_DIVE_LEFT,
	ACT_DAB_AKIMBO_ROLL_LEFT,
	/*Anything after this is private*/
	LAST_SHARED_AKIMBO_ACTIVITY
};
#ifdef CLIENT_DLL
	#define CAkimbobase C_Akimbobase
#endif
class CSDKPlayer;
class CAkimbobase : public CWeaponSDKBase
{
public:
	DECLARE_CLASS(CAkimbobase, CWeaponSDKBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	CAkimbobase ();
	virtual bool IsPredicted() const { return true; }
	virtual Activity ActivityOverride (Activity baseAct, bool *pRequired);
	virtual int GetTracerAttachment (void);
	virtual Activity GetIdleActivity (void);
	virtual bool Deploy ();
	virtual bool Holster (CBaseCombatWeapon *pSwitchingTo);
	virtual void PrimaryAttack (void);
	virtual void FinishReload (void);
	virtual void GiveDefaultAmmo (void);
	virtual void OnPickedUp (CBaseCombatCharacter *pNewOwner);
	virtual	void CheckReload (void);

	virtual int GetMaxClip1( void ) const;
	virtual int GetWeight( void ) const;
public:
};
#endif
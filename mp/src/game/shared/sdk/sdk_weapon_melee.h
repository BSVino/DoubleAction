//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base code for any melee based weapon
//
//=====================================================================================//

#ifndef SDK_WEAPON_MELEE_H
#define SDK_WEAPON_MELEE_H

#ifdef _WIN32
#pragma once
#endif


#if defined( CLIENT_DLL )
#define CWeaponSDKMelee C_WeaponSDKMelee
#endif

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CWeaponSDKMelee : public CWeaponSDKBase
{
	DECLARE_CLASS( CWeaponSDKMelee, CWeaponSDKBase );
public:
	CWeaponSDKMelee();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual	void	Spawn( void );
	virtual	void	Precache( void );
	
	//Attack functions
	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	
	virtual void	ItemPostFrame( void );

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity	GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

	virtual	float	GetMeleeDamage( bool bSecondary ) const;

	virtual float GetBrawlFireRate( void ) { return GetFireRate(); }
	virtual float GetBrawlSecondaryFireRate( void ) { return GetSecondaryFireRate(); }

	CWeaponSDKMelee( const CWeaponSDKMelee & );
};


#endif // SDK_WEAPON_MELEE_H

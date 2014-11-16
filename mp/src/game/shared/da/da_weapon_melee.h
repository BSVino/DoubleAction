//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base code for any melee based weapon
//
//=====================================================================================//

#ifndef DA_WEAPON_MELEE_H
#define DA_WEAPON_MELEE_H

#ifdef _WIN32
#pragma once
#endif


#if defined( CLIENT_DLL )
#define CWeaponDAMelee C_WeaponDAMelee
#endif

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CWeaponDAMelee : public CWeaponDABase
{
	DECLARE_CLASS( CWeaponDAMelee, CWeaponDABase );
public:
	CWeaponDAMelee();

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

	CWeaponDAMelee( const CWeaponDAMelee & );
};


#endif // DA_WEAPON_MELEE_H

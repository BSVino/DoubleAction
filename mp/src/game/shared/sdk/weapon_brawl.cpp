//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )

	#define CWeaponBrawl C_WeaponBrawl
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

	#include "ilagcompensationmanager.h"

#endif


class CWeaponBrawl : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS( CWeaponBrawl, CWeaponSDKMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponBrawl();

	virtual SDKWeaponID GetWeaponID( void ) const		{	return SDK_WEAPON_BRAWL; }
	virtual float	GetRange( void )					{	return	64.0f;	}	//Tony; let the crowbar swing further.
	virtual bool CanWeaponBeDropped() const				{	return false; }

private:

	CWeaponBrawl( const CWeaponBrawl & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBrawl, DT_WeaponBrawl )

BEGIN_NETWORK_TABLE( CWeaponBrawl, DT_WeaponBrawl )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBrawl )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_brawl, CWeaponBrawl );
PRECACHE_WEAPON_REGISTER( weapon_brawl );



CWeaponBrawl::CWeaponBrawl()
{
	AddEffects( EF_NODRAW );
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponBrawl::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE,				false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE,			false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE,		false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_IDLE,             false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_IDLE,              false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_IDLE,               false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_IDLE,            false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_IDLE,        false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE,		false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE,			false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH,	false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE,	false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE,	false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH,			false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE,			false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE,			false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL,                  false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH,           false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE,            false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE,            false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE,             false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL,             false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE,					false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL,				false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL,				false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL,					false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE,              false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND,         false },
};

IMPLEMENT_ACTTABLE( CWeaponBrawl );


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
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE,				false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE,			false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE,		false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE,		false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH,	false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE,	false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE,	false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE,			false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE,			false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE,					false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL,				false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL,				false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL,					false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE,					false },
};

IMPLEMENT_ACTTABLE( CWeaponBrawl );


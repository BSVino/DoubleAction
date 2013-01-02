//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponMP5K C_WeaponMP5K
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponMP5K : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMP5K, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponMP5K();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_MP5K; }

private:

	CWeaponMP5K( const CWeaponMP5K & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5K, DT_WeaponMP5K )

BEGIN_NETWORK_TABLE( CWeaponMP5K, DT_WeaponMP5K )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5K )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5k, CWeaponMP5K );
PRECACHE_WEAPON_REGISTER( weapon_mp5k );



CWeaponMP5K::CWeaponMP5K()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponMP5K::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_MP5K,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_MP5K,					false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_MP5K,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_MP5K,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_MP5K,			false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_MP5K,               false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_MP5K,                false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_MP5K,                 false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_MP5K,              false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_MP5K,          false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_MP5K,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_MP5K,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_MP5K,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_MP5K,				false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_MP5K,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_MP5K,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_MP5K,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_MP5K,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_MP5K,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_MP5K,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_MP5K,				false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_MP5K,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_MP5K,				false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_MP5K,                     false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_MP5K,              false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_MP5K,               false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_MP5K,               false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_MP5K,                false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_MP5K,                false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_MP5K,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_MP5K,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_MP5K,					false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_MP5K,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_MP5K,					false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_MP5K,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_MP5K,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_MP5K,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_MP5K,						false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_MP5K,            false },
};

IMPLEMENT_ACTTABLE( CWeaponMP5K );


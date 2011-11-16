//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponMP5 C_WeaponMP5
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponMP5 : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMP5, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponMP5();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_MP5; }

private:

	CWeaponMP5( const CWeaponMP5 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5, DT_WeaponMP5 )

BEGIN_NETWORK_TABLE( CWeaponMP5, DT_WeaponMP5 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponMP5 );
PRECACHE_WEAPON_REGISTER( weapon_mp5 );



CWeaponMP5::CWeaponMP5()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponMP5::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_MP5K,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_MP5K,					false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_MP5K,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_MP5K,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_MP5K,			false },
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
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_MP5K,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_MP5K,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_MP5K,					false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_MP5K,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_MP5K,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_MP5K,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_MP5K,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_MP5K,						false },
};

IMPLEMENT_ACTTABLE( CWeaponMP5 );


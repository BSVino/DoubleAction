//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shotgun.h"
#include "sdk_fx_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponM3 C_WeaponM3
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"
	#include "te_firebullets.h"

#endif


class CWeaponM3 : public CWeaponShotgun
{
public:
	DECLARE_CLASS( CWeaponM3, CWeaponShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponM3();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_M3; }

private:

	CWeaponM3( const CWeaponM3 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM3, DT_WeaponM3 )

BEGIN_NETWORK_TABLE( CWeaponM3, DT_WeaponM3 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM3 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m3, CWeaponM3 );
PRECACHE_WEAPON_REGISTER( weapon_m3 );



CWeaponM3::CWeaponM3()
{
}

acttable_t CWeaponM3::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_M3,					false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_M3,					false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_M3,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_M3,					false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_M3,				false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_M3,				false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_M3,				false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_M3,					false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_M3,				false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_M3,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_M3,			false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_M3,			false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_M3,			false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_M3,			false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_M3,						false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_M3,				false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_M3,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_M3,				false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_M3,					false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_M3,					false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_M3,					false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_M3,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_M3,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_M3,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_M3,					false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_M3,						false },
};

IMPLEMENT_ACTTABLE( CWeaponM3 );

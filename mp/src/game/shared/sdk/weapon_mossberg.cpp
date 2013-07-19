//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shotgun.h"
#include "sdk_fx_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponMossberg C_WeaponMossberg
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"
	#include "te_firebullets.h"

#endif


class CWeaponMossberg : public CWeaponShotgun
{
public:
	DECLARE_CLASS( CWeaponMossberg, CWeaponShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponMossberg();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_MOSSBERG; }

private:

	CWeaponMossberg( const CWeaponMossberg & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMossberg, DT_WeaponMossberg )

BEGIN_NETWORK_TABLE( CWeaponMossberg, DT_WeaponMossberg )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMossberg )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mossberg, CWeaponMossberg );
PRECACHE_WEAPON_REGISTER( weapon_mossberg );



CWeaponMossberg::CWeaponMossberg()
{
}

acttable_t CWeaponMossberg::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_MOSSBERG,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_MOSSBERG,					false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_MOSSBERG,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_MOSSBERG,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_MOSSBERG,			false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_MOSSBERG,               false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_MOSSBERG,                false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_MOSSBERG,                 false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_MOSSBERG,              false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_MOSSBERG,          false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_MOSSBERG,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_MOSSBERG,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_MOSSBERG,				false },
	{ ACT_DAB_STAND_AIM,                ACT_DAB_STAND_AIM_MOSSBERG,                 false },
	{ ACT_DAB_WALK_AIM,                 ACT_DAB_WALK_AIM_MOSSBERG,                  false },
	{ ACT_DAB_RUN_AIM,                  ACT_DAB_RUN_AIM_MOSSBERG,                   false },
	{ ACT_DAB_CROUCH_AIM,               ACT_DAB_CROUCH_AIM_MOSSBERG,                false },
	{ ACT_DAB_CROUCHWALK_AIM,           ACT_DAB_CROUCHWALK_AIM_MOSSBERG,            false },
	{ ACT_DAB_PRONECHEST_AIM,           ACT_DAB_PRONECHEST_AIM_MOSSBERG,            false },
	{ ACT_DAB_PRONEBACK_AIM,            ACT_DAB_PRONEBACK_AIM_MOSSBERG,             false },
	{ ACT_DAB_DRAW,                     ACT_DAB_DRAW_MOSSBERG,                      false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_MOSSBERG,				false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_MOSSBERG,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_MOSSBERG,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_MOSSBERG,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_MOSSBERG,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_MOSSBERG,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_MOSSBERG,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_MOSSBERG,				false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_MOSSBERG,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_MOSSBERG,				false },
	{ ACT_DAB_RELOAD_LOOP,              ACT_DAB_RELOAD_LOOP_MOSSBERG,               false },
	{ ACT_DAB_RELOAD_LOOP_CROUCH,       ACT_DAB_RELOAD_LOOP_CROUCH_MOSSBERG,        false },
	{ ACT_DAB_RELOAD_LOOP_PRONE,        ACT_DAB_RELOAD_LOOP_PRONE_MOSSBERG,         false },
	{ ACT_DAB_RELOAD_LOOP_SLIDE,        ACT_DAB_RELOAD_LOOP_SLIDE_MOSSBERG,         false },
	{ ACT_DAB_RELOAD_END,               ACT_DAB_RELOAD_END_MOSSBERG,                false },
	{ ACT_DAB_RELOAD_END_CROUCH,        ACT_DAB_RELOAD_END_CROUCH_MOSSBERG,         false },
	{ ACT_DAB_RELOAD_END_PRONE,         ACT_DAB_RELOAD_END_PRONE_MOSSBERG,          false },
	{ ACT_DAB_RELOAD_END_SLIDE,         ACT_DAB_RELOAD_END_SLIDE_MOSSBERG,          false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_MOSSBERG,                     false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_MOSSBERG,              false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_MOSSBERG,               false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_MOSSBERG,               false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_MOSSBERG,                false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_MOSSBERG,                false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_MOSSBERG,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_MOSSBERG,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_MOSSBERG,					false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_MOSSBERG,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_MOSSBERG,					false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_MOSSBERG,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_MOSSBERG,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_MOSSBERG,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_MOSSBERG,						false },
	{ ACT_DAB_DIVESLIDE,                ACT_DAB_DIVESLIDE_MOSSBERG,                 false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_MOSSBERG,            false },
};

IMPLEMENT_ACTTABLE( CWeaponMossberg );

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shotgun.h"
#include "da_fx_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponMossberg C_WeaponMossberg
	#include "c_da_player.h"

#else

	#include "da_player.h"
	#include "te_firebullets.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMossberg : public CWeaponShotgun
{
public:
	DECLARE_CLASS( CWeaponMossberg, CWeaponShotgun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponMossberg();

	virtual DAWeaponID GetWeaponID( void ) const		{ return DA_WEAPON_MOSSBERG; }

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
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_MOSSBERG,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_MOSSBERG,					false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_MOSSBERG,					false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_MOSSBERG,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_MOSSBERG,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_MOSSBERG,               false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_MOSSBERG,                false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_MOSSBERG,                 false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_MOSSBERG,              false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_MOSSBERG,          false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_MOSSBERG,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_MOSSBERG,			false },
	{ ACT_DA_STAND_AIM,                ACT_DA_STAND_AIM_MOSSBERG,                 false },
	{ ACT_DA_WALK_AIM,                 ACT_DA_WALK_AIM_MOSSBERG,                  false },
	{ ACT_DA_RUN_AIM,                  ACT_DA_RUN_AIM_MOSSBERG,                   false },
	{ ACT_DA_CROUCH_AIM,               ACT_DA_CROUCH_AIM_MOSSBERG,                false },
	{ ACT_DA_CROUCHWALK_AIM,           ACT_DA_CROUCHWALK_AIM_MOSSBERG,            false },
	{ ACT_DA_PRONECHEST_AIM,           ACT_DA_PRONECHEST_AIM_MOSSBERG,            false },
	{ ACT_DA_PRONEBACK_AIM,            ACT_DA_PRONEBACK_AIM_MOSSBERG,             false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_MOSSBERG,                      false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_MOSSBERG,				false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_MOSSBERG,		false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_MOSSBERG,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_MOSSBERG,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_MOSSBERG,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_MOSSBERG,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_MOSSBERG,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_MOSSBERG,				false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_MOSSBERG,				false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_MOSSBERG,				false },
	{ ACT_DA_RELOAD_LOOP,              ACT_DA_RELOAD_LOOP_MOSSBERG,               false },
	{ ACT_DA_RELOAD_LOOP_CROUCH,       ACT_DA_RELOAD_LOOP_CROUCH_MOSSBERG,        false },
	{ ACT_DA_RELOAD_LOOP_PRONE,        ACT_DA_RELOAD_LOOP_PRONE_MOSSBERG,         false },
	{ ACT_DA_RELOAD_LOOP_SLIDE,        ACT_DA_RELOAD_LOOP_SLIDE_MOSSBERG,         false },
	{ ACT_DA_RELOAD_END,               ACT_DA_RELOAD_END_MOSSBERG,                false },
	{ ACT_DA_RELOAD_END_CROUCH,        ACT_DA_RELOAD_END_CROUCH_MOSSBERG,         false },
	{ ACT_DA_RELOAD_END_PRONE,         ACT_DA_RELOAD_END_PRONE_MOSSBERG,          false },
	{ ACT_DA_RELOAD_END_SLIDE,         ACT_DA_RELOAD_END_SLIDE_MOSSBERG,          false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_MOSSBERG,                     false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_MOSSBERG,              false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_MOSSBERG,               false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_MOSSBERG,               false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_MOSSBERG,                false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_MOSSBERG,                false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_MOSSBERG,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_MOSSBERG,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_MOSSBERG,					false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_MOSSBERG,						false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_MOSSBERG,					false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_MOSSBERG,					false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_MOSSBERG,						false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_MOSSBERG,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_MOSSBERG,						false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_MOSSBERG,                 false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_MOSSBERG,            false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_MOSSBERG,             false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_MOSSBERG,                  false },
};

IMPLEMENT_ACTTABLE( CWeaponMossberg );

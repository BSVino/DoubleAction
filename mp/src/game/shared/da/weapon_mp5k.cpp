//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_dabase.h"

#if defined( CLIENT_DLL )

	#define CWeaponMP5K C_WeaponMP5K
	#include "c_da_player.h"

#else

	#include "da_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMP5K : public CWeaponDABase
{
public:
	DECLARE_CLASS( CWeaponMP5K, CWeaponDABase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponMP5K();

	virtual DAWeaponID GetWeaponID( void ) const		{ return DA_WEAPON_MP5K; }

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
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_MP5K,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_MP5K,					false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_MP5K,					false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_MP5K,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_MP5K,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_MP5K,               false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_MP5K,                false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_MP5K,                 false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_MP5K,              false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_MP5K,          false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_MP5K,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_MP5K,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_MP5K,                      false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_MP5K,				false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_MP5K,		false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_MP5K,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_MP5K,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_MP5K,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_MP5K,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_MP5K,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_MP5K,				false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_MP5K,				false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_MP5K,				false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_MP5K,                     false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_MP5K,              false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_MP5K,               false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_MP5K,               false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_MP5K,                false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_MP5K,                false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_MP5K,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_MP5K,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_MP5K,					false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_MP5K,						false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_MP5K,					false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_MP5K,					false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_MP5K,						false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_MP5K,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_MP5K,						false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_MP5K,                 false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_MP5K,            false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_MP5K,             false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_MP5K,                  false },
};

IMPLEMENT_ACTTABLE( CWeaponMP5K );


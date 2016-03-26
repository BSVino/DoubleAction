//========= Copyright � 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_dabase.h"

#if defined( CLIENT_DLL )

	#define CWeaponFAL C_WeaponFAL
	#include "c_da_player.h"

#else

	#include "da_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponFAL : public CWeaponDABase
{
public:
	DECLARE_CLASS( CWeaponFAL, CWeaponDABase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponFAL();

	virtual DAWeaponID GetWeaponID( void ) const		{ return DA_WEAPON_FAL; }

	virtual bool FullAimIn() { return true; }

private:

	CWeaponFAL( const CWeaponFAL & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFAL, DT_WeaponFAL )

BEGIN_NETWORK_TABLE( CWeaponFAL, DT_WeaponFAL )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponFAL )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_fal, CWeaponFAL );
PRECACHE_WEAPON_REGISTER( weapon_fal );



CWeaponFAL::CWeaponFAL()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponFAL::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_FAL,					false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_FAL,					false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_FAL,					false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_FAL,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_FAL,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_FAL,                false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_FAL,                 false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_FAL,                  false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_FAL,               false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_FAL,           false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_FAL,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_FAL,				false },
	{ ACT_DA_STAND_AIM,                ACT_DA_STAND_AIM_FAL,                  false },
	{ ACT_DA_WALK_AIM,                 ACT_DA_WALK_AIM_FAL,                   false },
	{ ACT_DA_RUN_AIM,                  ACT_DA_RUN_AIM_FAL,                    false },
	{ ACT_DA_CROUCH_AIM,               ACT_DA_CROUCH_AIM_FAL,                 false },
	{ ACT_DA_CROUCHWALK_AIM,           ACT_DA_CROUCHWALK_AIM_FAL,             false },
	{ ACT_DA_PRONECHEST_AIM,           ACT_DA_PRONECHEST_AIM_FAL,             false },
	{ ACT_DA_PRONEBACK_AIM,            ACT_DA_PRONEBACK_AIM_FAL,              false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_FAL,                       false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_FAL,				false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_FAL,		false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_FAL,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_FAL,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_FAL,			false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_FAL,			false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_FAL,						false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_FAL,				false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_FAL,				false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_FAL,				false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_FAL,                      false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_FAL,               false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_FAL,                false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_FAL,                false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_FAL,                 false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_FAL,                 false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_FAL,					false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_FAL,					false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_FAL,					false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_FAL,						false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_FAL,					false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_FAL,					false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_FAL,						false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_FAL,					false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_FAL,						false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_FAL,                  false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_FAL,             false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_FAL,              false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_FAL,                   false },
};

IMPLEMENT_ACTTABLE( CWeaponFAL );


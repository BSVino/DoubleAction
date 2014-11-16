//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_dabase.h"

#if defined( CLIENT_DLL )

	#define CWeaponPlanD C_WeaponPlanD
	#include "c_da_player.h"

#else

	#include "da_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponPlanD : public CWeaponDABase
{
public:
	DECLARE_CLASS( CWeaponPlanD, CWeaponDABase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponPlanD();

	virtual DAWeaponID GetWeaponID( void ) const		{ return DA_WEAPON_PLAN_D; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

private:

	CWeaponPlanD(const CWeaponPlanD &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponPlanD, DT_WeaponPlanD)

BEGIN_NETWORK_TABLE(CWeaponPlanD, DT_WeaponPlanD)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponPlanD)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_plan_d, CWeaponPlanD);
PRECACHE_WEAPON_REGISTER( weapon_plan_d );



CWeaponPlanD::CWeaponPlanD()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponPlanD::m_acttable[] =
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_PLAN_D,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_PLAN_D,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_PLAN_D,				false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_PLAN_D,			false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_PLAN_D,		false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_PLAN_D,            false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_PLAN_D,             false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_PLAN_D,              false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_PLAN_D,           false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_PLAN_D,       false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_PLAN_D,		false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_PLAN_D,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_PLAN_D,                   false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_PLAN_D,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_PLAN_D,	false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_PLAN_D,	false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_PLAN_D,	false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_PLAN_D,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_PLAN_D,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_PLAN_D,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_PLAN_D,			false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_PLAN_D,			false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_PLAN_D,			false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_PLAN_D,                  false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_PLAN_D,           false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_PLAN_D,            false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_PLAN_D,            false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_PLAN_D,             false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_PLAN_D,             false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_PLAN_D,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_PLAN_D,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_PLAN_D,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_PLAN_D,					false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_PLAN_D,				false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_PLAN_D,				false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_PLAN_D,					false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_PLAN_D,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_PLAN_D,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_PLAN_D,              false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_PLAN_D,         false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_PLAN_D,          false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_PLAN_D,               false },
};

IMPLEMENT_ACTTABLE(CWeaponPlanD);


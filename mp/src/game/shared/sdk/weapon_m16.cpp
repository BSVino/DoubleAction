//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponM16 C_WeaponM16
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponM16 : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponM16, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponM16();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_M16; }

	virtual bool FullAimIn() { return true; }

	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

private:

	CWeaponM16( const CWeaponM16 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM16, DT_WeaponM16 )

BEGIN_NETWORK_TABLE( CWeaponM16, DT_WeaponM16 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM16 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m16, CWeaponM16 );
PRECACHE_WEAPON_REGISTER( weapon_m16 );



CWeaponM16::CWeaponM16()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponM16::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_M16,					false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_M16,					false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_M16,					false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_M16,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_M16,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_M16,                false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_M16,                 false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_M16,                  false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_M16,               false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_M16,           false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_M16,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_M16,				false },
	{ ACT_DA_STAND_AIM,                ACT_DA_STAND_AIM_M16,                  false },
	{ ACT_DA_WALK_AIM,                 ACT_DA_WALK_AIM_M16,                   false },
	{ ACT_DA_RUN_AIM,                  ACT_DA_RUN_AIM_M16,                    false },
	{ ACT_DA_CROUCH_AIM,               ACT_DA_CROUCH_AIM_M16,                 false },
	{ ACT_DA_CROUCHWALK_AIM,           ACT_DA_CROUCHWALK_AIM_M16,             false },
	{ ACT_DA_PRONECHEST_AIM,           ACT_DA_PRONECHEST_AIM_M16,             false },
	{ ACT_DA_PRONEBACK_AIM,            ACT_DA_PRONEBACK_AIM_M16,              false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_M16,                       false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_M16,				false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_M16,		false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_M16,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_M16,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_M16,			false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_M16,			false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_M16,						false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_M16,				false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_M16,				false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_M16,				false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_M16,                      false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_M16,               false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_M16,                false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_M16,                false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_M16,                 false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_M16,                 false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_M16,					false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_M16,					false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_M16,					false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_M16,						false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_M16,					false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_M16,					false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_M16,						false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_M16,					false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_M16,						false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_M16,                  false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_M16,             false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_M16,              false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_M16,                   false },
};

IMPLEMENT_ACTTABLE( CWeaponM16 );


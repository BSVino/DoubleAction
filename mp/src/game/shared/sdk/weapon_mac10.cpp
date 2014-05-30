//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponMAC10 C_WeaponMAC10
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMAC10 : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMAC10, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponMAC10();

	virtual SDKWeaponID GetWeaponID( void ) const { return SDK_WEAPON_MAC10; }

private:

	CWeaponMAC10( const CWeaponMAC10 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMAC10, DT_WeaponMAC10 )

BEGIN_NETWORK_TABLE( CWeaponMAC10, DT_WeaponMAC10 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMAC10 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mac10, CWeaponMAC10 );
PRECACHE_WEAPON_REGISTER( weapon_mac10 );



CWeaponMAC10::CWeaponMAC10()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponMAC10::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,               ACT_DA_STAND_IDLE_MAC10,              false },
	{ ACT_DA_WALK_IDLE,                ACT_DA_WALK_IDLE_MAC10,               false },
	{ ACT_DA_RUN_IDLE,                 ACT_DA_RUN_IDLE_MAC10,                false },
	{ ACT_DA_CROUCH_IDLE,              ACT_DA_CROUCH_IDLE_MAC10,             false },
	{ ACT_DA_CROUCHWALK_IDLE,          ACT_DA_CROUCHWALK_IDLE_MAC10,         false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_MAC10,             false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_MAC10,              false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_MAC10,               false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_MAC10,            false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_MAC10,        false },
	{ ACT_DA_PRONECHEST_IDLE,          ACT_DA_PRONECHEST_IDLE_MAC10,         false },
	{ ACT_DA_PRONEBACK_IDLE,           ACT_DA_PRONEBACK_IDLE_MAC10,          false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_MAC10,                    false },
	{ ACT_DA_PRIMARYATTACK,            ACT_DA_PRIMARYATTACK_MAC10,           false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,     ACT_DA_PRIMARYATTACK_CROUCH_MAC10,    false },
	{ ACT_DA_PRIMARYATTACK_PRONE,      ACT_DA_PRIMARYATTACK_PRONE_MAC10,     false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,      ACT_DA_PRIMARYATTACK_SLIDE_MAC10,     false },
	{ ACT_DA_PRIMARYATTACK_DIVE,       ACT_DA_PRIMARYATTACK_DIVE_MAC10,      false },
	{ ACT_DA_PRIMARYATTACK_ROLL,       ACT_DA_PRIMARYATTACK_ROLL_MAC10,      false },
	{ ACT_DA_RELOAD,                   ACT_DA_RELOAD_MAC10,                  false },
	{ ACT_DA_RELOAD_CROUCH,            ACT_DA_RELOAD_CROUCH_MAC10,           false },
	{ ACT_DA_RELOAD_PRONE,             ACT_DA_RELOAD_PRONE_MAC10,            false },
	{ ACT_DA_RELOAD_SLIDE,             ACT_DA_RELOAD_SLIDE_MAC10,            false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_MAC10,                   false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_MAC10,            false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_MAC10,             false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_MAC10,             false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_MAC10,              false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_MAC10,              false },
	{ ACT_DA_JUMP_START,               ACT_DA_JUMP_START_MAC10,              false },
	{ ACT_DA_JUMP_FLOAT,               ACT_DA_JUMP_FLOAT_MAC10,              false },
	{ ACT_DA_JUMP_LAND,                ACT_DA_JUMP_LAND_MAC10,               false },
	{ ACT_DA_DIVE,                     ACT_DA_DIVE_MAC10,                    false },
	{ ACT_DA_DIVEFALL,                 ACT_DA_DIVEFALL_MAC10,                false },
	{ ACT_DA_DIVEROLL,                 ACT_DA_DIVEROLL_MAC10,                false },
	{ ACT_DA_ROLL,                     ACT_DA_ROLL_MAC10,                    false },
	{ ACT_DA_SLIDESTART,               ACT_DA_SLIDESTART_MAC10,              false },
	{ ACT_DA_SLIDE,                    ACT_DA_SLIDE_MAC10,                   false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_MAC10,               false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_MAC10,          false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_MAC10,           false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_MAC10,                false },
};

IMPLEMENT_ACTTABLE( CWeaponMAC10 );


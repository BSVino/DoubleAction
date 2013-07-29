//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponM1911 C_WeaponM1911
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponM1911 : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponM1911, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponM1911();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_M1911; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

private:

	CWeaponM1911( const CWeaponM1911 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM1911, DT_WeaponM1911 )

BEGIN_NETWORK_TABLE( CWeaponM1911, DT_WeaponM1911 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM1911 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m1911, CWeaponM1911 );
PRECACHE_WEAPON_REGISTER( weapon_m1911 );



CWeaponM1911::CWeaponM1911()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponM1911::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_M1911,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_M1911,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_M1911,					false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_M1911,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_M1911,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_M1911,              false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_M1911,               false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_M1911,                false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_M1911,             false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_M1911,         false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_M1911,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_M1911,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_M1911,                     false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_M1911,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_M1911,		false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_M1911,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_M1911,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_M1911,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_M1911,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_M1911,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_M1911,			false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_M1911,				false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_M1911,				false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_M1911,                    false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_M1911,             false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_M1911,              false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_M1911,              false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_M1911,               false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_M1911,               false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_M1911,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_M1911,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_M1911,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_M1911,						false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_M1911,					false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_M1911,					false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_M1911,						false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_M1911,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_M1911,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_M1911,                false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_M1911,           false },
};

IMPLEMENT_ACTTABLE( CWeaponM1911 );


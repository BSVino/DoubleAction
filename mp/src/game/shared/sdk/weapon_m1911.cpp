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
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_M1911,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_M1911,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_M1911,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_M1911,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_M1911,			false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_M1911,              false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_M1911,               false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_M1911,                false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_M1911,             false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_M1911,         false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_M1911,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_M1911,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_M1911,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_M1911,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_M1911,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_M1911,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_M1911,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_M1911,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_M1911,				false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_M1911,                    false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_M1911,             false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_M1911,              false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_M1911,              false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_M1911,               false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_M1911,               false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_M1911,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_M1911,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_M1911,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_M1911,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_M1911,					false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_M1911,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_M1911,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_M1911,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_M1911,					false },
	{ ACT_DAB_DIVESLIDE,                ACT_DAB_DIVESLIDE_M1911,                false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_M1911,           false },
};

IMPLEMENT_ACTTABLE( CWeaponM1911 );


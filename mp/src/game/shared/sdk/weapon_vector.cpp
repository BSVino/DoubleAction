//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponVector C_WeaponVector
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponVector : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponVector, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponVector();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_VECTOR; }

private:

	CWeaponVector( const CWeaponVector & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponVector, DT_WeaponVector )

BEGIN_NETWORK_TABLE( CWeaponVector, DT_WeaponVector )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponVector )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_vector, CWeaponVector );
PRECACHE_WEAPON_REGISTER( weapon_vector );



CWeaponVector::CWeaponVector()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponVector::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_VECTOR,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_VECTOR,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_VECTOR,				false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_VECTOR,				false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_VECTOR,			false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_VECTOR,             false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_VECTOR,              false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_VECTOR,               false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_VECTOR,            false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_VECTOR,        false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_VECTOR,			false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_VECTOR,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_VECTOR,                    false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_VECTOR,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_VECTOR,	false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_VECTOR,		false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_VECTOR,		false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_VECTOR,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_VECTOR,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_VECTOR,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_VECTOR,			false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_VECTOR,			false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_VECTOR,			false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_VECTOR,                   false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_VECTOR,            false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_VECTOR,             false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_VECTOR,             false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_VECTOR,              false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_VECTOR,              false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_VECTOR,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_VECTOR,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_VECTOR,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_VECTOR,					false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_VECTOR,				false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_VECTOR,				false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_VECTOR,					false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_VECTOR,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_VECTOR,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_VECTOR,               false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_VECTOR,          false },
};

IMPLEMENT_ACTTABLE( CWeaponVector );


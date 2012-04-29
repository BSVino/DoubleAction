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
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_VECTOR,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_VECTOR,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_VECTOR,				false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_VECTOR,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_VECTOR,			false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_VECTOR,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_VECTOR,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_VECTOR,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_VECTOR,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_VECTOR,	false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_VECTOR,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_VECTOR,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_VECTOR,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_VECTOR,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_VECTOR,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_VECTOR,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_VECTOR,			false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_VECTOR,			false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_VECTOR,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_VECTOR,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_VECTOR,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_VECTOR,					false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_VECTOR,				false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_VECTOR,				false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_VECTOR,					false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_VECTOR,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_VECTOR,					false },
};

IMPLEMENT_ACTTABLE( CWeaponVector );


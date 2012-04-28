//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponFAL C_WeaponFAL
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponFAL : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponFAL, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponFAL();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_FAL; }

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
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_FAL,					false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_FAL,					false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_FAL,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_FAL,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_FAL,			false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_FAL,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_FAL,				false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_FAL,					false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_FAL,				false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_FAL,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_FAL,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_FAL,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_FAL,			false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_FAL,			false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_FAL,						false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_FAL,				false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_FAL,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_FAL,				false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_FAL,					false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_FAL,					false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_FAL,					false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_FAL,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_FAL,					false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_FAL,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_FAL,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_FAL,					false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_FAL,						false },
};

IMPLEMENT_ACTTABLE( CWeaponFAL );


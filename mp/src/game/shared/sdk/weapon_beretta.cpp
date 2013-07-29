//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponBeretta C_WeaponBeretta
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponBeretta : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponBeretta, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponBeretta();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return SDK_WEAPON_BERETTA; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

private:

	CWeaponBeretta( const CWeaponBeretta & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBeretta, DT_WeaponBeretta )

BEGIN_NETWORK_TABLE( CWeaponBeretta, DT_WeaponBeretta )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBeretta )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_beretta, CWeaponBeretta );
PRECACHE_WEAPON_REGISTER( weapon_beretta );



CWeaponBeretta::CWeaponBeretta()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponBeretta::m_acttable[] = 
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_BERETTA,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_BERETTA,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_BERETTA,				false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_BERETTA,			false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_BERETTA,		false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_READY_BERETTA,            false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_READY_BERETTA,             false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_READY_BERETTA,              false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_READY_BERETTA,           false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_READY_BERETTA,       false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_BERETTA,		false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_BERETTA,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_BERETTA,                   false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_BERETTA,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_BERETTA,	false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_BERETTA,	false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_BERETTA,	false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_BERETTA,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_BERETTA,		false },
	{ ACT_DA_RELOAD,					ACT_DA_RELOAD_BERETTA,					false },
	{ ACT_DA_RELOAD_CROUCH,			ACT_DA_RELOAD_CROUCH_BERETTA,			false },
	{ ACT_DA_RELOAD_PRONE,				ACT_DA_RELOAD_PRONE_BERETTA,			false },
	{ ACT_DA_RELOAD_SLIDE,				ACT_DA_RELOAD_SLIDE_BERETTA,			false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_BERETTA,                  false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_BERETTA,           false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_BERETTA,            false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_BERETTA,            false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_BERETTA,             false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_BERETTA,             false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_BERETTA,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_BERETTA,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_BERETTA,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_BERETTA,					false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_BERETTA,				false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_BERETTA,				false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_BERETTA,					false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_BERETTA,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_BERETTA,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_BERETTA,              false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_BERETTA,         false },
};

IMPLEMENT_ACTTABLE( CWeaponBeretta );


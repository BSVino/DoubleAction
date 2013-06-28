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
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_BERETTA,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_BERETTA,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_BERETTA,				false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_BERETTA,			false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_BERETTA,		false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_BERETTA,            false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_BERETTA,             false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_BERETTA,              false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_BERETTA,           false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_BERETTA,       false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_BERETTA,		false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_BERETTA,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_BERETTA,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_BERETTA,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_BERETTA,	false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_BERETTA,	false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_BERETTA,	false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_BERETTA,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_BERETTA,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_BERETTA,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_BERETTA,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_BERETTA,			false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_BERETTA,			false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_BERETTA,                  false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_BERETTA,           false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_BERETTA,            false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_BERETTA,            false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_BERETTA,             false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_BERETTA,             false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_BERETTA,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_BERETTA,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_BERETTA,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_BERETTA,					false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_BERETTA,				false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_BERETTA,				false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_BERETTA,					false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_BERETTA,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_BERETTA,					false },
	{ ACT_DAB_DIVESLIDE,                ACT_DAB_DIVESLIDE_BERETTA,              false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_BERETTA,         false },
};

IMPLEMENT_ACTTABLE( CWeaponBeretta );


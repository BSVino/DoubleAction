//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_drugs.h"


#ifdef CLIENT_DLL
	
#else

	#include "sdk_player.h"
	#include "items.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDrugs, DT_WeaponDrugs )

BEGIN_NETWORK_TABLE(CWeaponDrugs, DT_WeaponDrugs)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponDrugs)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_drugs, CWeaponDrugs);
PRECACHE_WEAPON_REGISTER( weapon_drugs );


#ifdef GAME_DLL

BEGIN_DATADESC(CWeaponDrugs)
END_DATADESC()

void CWeaponDrugs::EmitGrenade(Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon)
{
	// Drugs aren't really grenades, don't tell anyone.
	ToSDKPlayer(pPlayer)->BeginDrugs();
}
#endif

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponDrugs::m_acttable[] =
{
	{ ACT_DA_STAND_IDLE,				ACT_DA_STAND_IDLE_GRENADE,				false },
	{ ACT_DA_WALK_IDLE,				ACT_DA_WALK_IDLE_GRENADE,				false },
	{ ACT_DA_RUN_IDLE,					ACT_DA_RUN_IDLE_GRENADE,				false },
	{ ACT_DA_CROUCH_IDLE,				ACT_DA_CROUCH_IDLE_GRENADE,			false },
	{ ACT_DA_CROUCHWALK_IDLE,			ACT_DA_CROUCHWALK_IDLE_GRENADE,		false },
	{ ACT_DA_STAND_READY,              ACT_DA_STAND_IDLE_GRENADE,             false },
	{ ACT_DA_WALK_READY,               ACT_DA_WALK_IDLE_GRENADE,              false },
	{ ACT_DA_RUN_READY,                ACT_DA_RUN_IDLE_GRENADE,               false },
	{ ACT_DA_CROUCH_READY,             ACT_DA_CROUCH_IDLE_GRENADE,            false },
	{ ACT_DA_CROUCHWALK_READY,         ACT_DA_CROUCHWALK_IDLE_GRENADE,        false },
	{ ACT_DA_PRONECHEST_IDLE,			ACT_DA_PRONECHEST_IDLE_GRENADE,		false },
	{ ACT_DA_PRONEBACK_IDLE,			ACT_DA_PRONEBACK_IDLE_GRENADE,			false },
	{ ACT_DA_DRAW,                     ACT_DA_DRAW_GRENADE,                   false },
	{ ACT_DA_PRIMARYATTACK,			ACT_DA_PRIMARYATTACK_GRENADE,			false },
	{ ACT_DA_PRIMARYATTACK_CROUCH,		ACT_DA_PRIMARYATTACK_CROUCH_GRENADE,	false },
	{ ACT_DA_PRIMARYATTACK_PRONE,		ACT_DA_PRIMARYATTACK_PRONE_GRENADE,	false },
	{ ACT_DA_PRIMARYATTACK_SLIDE,		ACT_DA_PRIMARYATTACK_SLIDE_GRENADE,	false },
	{ ACT_DA_PRIMARYATTACK_DIVE,		ACT_DA_PRIMARYATTACK_DIVE_GRENADE,		false },
	{ ACT_DA_PRIMARYATTACK_ROLL,		ACT_DA_PRIMARYATTACK_ROLL_GRENADE,		false },
	{ ACT_DA_BRAWL,                    ACT_DA_BRAWL_GRENADE,                  false },
	{ ACT_DA_BRAWL_CROUCH,             ACT_DA_BRAWL_CROUCH_GRENADE,           false },
	{ ACT_DA_BRAWL_PRONE,              ACT_DA_BRAWL_PRONE_GRENADE,            false },
	{ ACT_DA_BRAWL_SLIDE,              ACT_DA_BRAWL_SLIDE_GRENADE,            false },
	{ ACT_DA_BRAWL_DIVE,               ACT_DA_BRAWL_DIVE_GRENADE,             false },
	{ ACT_DA_BRAWL_ROLL,               ACT_DA_BRAWL_ROLL_GRENADE,             false },
	{ ACT_DA_JUMP_START,				ACT_DA_JUMP_START_GRENADE,				false },
	{ ACT_DA_JUMP_FLOAT,				ACT_DA_JUMP_FLOAT_GRENADE,				false },
	{ ACT_DA_JUMP_LAND,				ACT_DA_JUMP_LAND_GRENADE,				false },
	{ ACT_DA_DIVE,						ACT_DA_DIVE_GRENADE,					false },
	{ ACT_DA_DIVEFALL,					ACT_DA_DIVEFALL_GRENADE,				false },
	{ ACT_DA_DIVEROLL,					ACT_DA_DIVEROLL_GRENADE,				false },
	{ ACT_DA_ROLL,						ACT_DA_ROLL_GRENADE,					false },
	{ ACT_DA_SLIDESTART,				ACT_DA_SLIDESTART_GRENADE,				false },
	{ ACT_DA_SLIDE,					ACT_DA_SLIDE_GRENADE,					false },
	{ ACT_DA_DIVESLIDE,                ACT_DA_DIVESLIDE_GRENADE,              false },
	{ ACT_DA_PRONE_TO_STAND,           ACT_DA_PRONE_TO_STAND_GRENADE,         false },
	{ ACT_DA_THROW_GRENADE,            ACT_DA_THROW_GRENADE_GRENADE,          false },
	{ ACT_DA_WALLFLIP,                 ACT_DA_WALLFLIP_GRENADE,               false },
};

IMPLEMENT_ACTTABLE(CWeaponDrugs);

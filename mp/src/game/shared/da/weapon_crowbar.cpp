//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_dabase.h"
#include "da_weapon_melee.h"

#if defined( CLIENT_DLL )

	#define CWeaponCrowbar C_WeaponCrowbar
	#include "c_da_player.h"

#else

	#include "da_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponCrowbar : public CWeaponDAMelee
{
public:
	DECLARE_CLASS( CWeaponCrowbar, CWeaponDAMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponCrowbar();

	virtual DAWeaponID GetWeaponID( void ) const		{	return WEAPON_NONE; }
	virtual float	GetRange( void )					{	return	64.0f;	}	//Tony; let the crowbar swing further.
	virtual bool CanWeaponBeDropped() const				{	return false; }

private:

	CWeaponCrowbar( const CWeaponCrowbar & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCrowbar, DT_WeaponCrowbar )

BEGIN_NETWORK_TABLE( CWeaponCrowbar, DT_WeaponCrowbar )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponCrowbar )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );



CWeaponCrowbar::CWeaponCrowbar()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponCrowbar::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,					ACT_DOD_STAND_AIM_SPADE,				false },
	{ ACT_MP_CROUCH_IDLE,					ACT_DOD_CROUCH_AIM_SPADE,				false },

	{ ACT_MP_RUN,							ACT_DOD_RUN_AIM_SPADE,					false },
	{ ACT_MP_WALK,							ACT_DOD_WALK_AIM_SPADE,					false },
	{ ACT_MP_CROUCHWALK,					ACT_DOD_CROUCHWALK_AIM_SPADE,			false },
	{ ACT_SPRINT,							ACT_DOD_SPRINT_AIM_SPADE,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_SPADE,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_SPADE,			false },
};

IMPLEMENT_ACTTABLE( CWeaponCrowbar );


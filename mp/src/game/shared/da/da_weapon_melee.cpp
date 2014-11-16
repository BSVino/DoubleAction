//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base code for any melee based weapon
//
//=====================================================================================//

#include "cbase.h"

#include "weapon_dabase.h"
#include "da_weapon_melee.h"

#include "da_gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "animation.h"

#if defined( CLIENT_DLL )
	#include "c_da_player.h"
#else
	#include "da_player.h"
	#include "ndebugoverlay.h"
	#include "te_effect_dispatch.h"
	#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDAMelee, DT_WeaponDAMelee )

BEGIN_NETWORK_TABLE( CWeaponDAMelee, DT_WeaponDAMelee )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponDAMelee )
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponDAMelee::CWeaponDAMelee()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CWeaponDAMelee::Spawn( void )
{
	m_fMinRange1	= 0;
	m_fMinRange2	= 0;
	m_fMaxRange1	= 64;
	m_fMaxRange2	= 64;
	//Call base class first
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CWeaponDAMelee::Precache( void )
{
	//Call base class first
	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponDAMelee::ItemPostFrame( void )
{
	CDAPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer == NULL )
		return;

	if (IsThrowingGrenade())
	{
		if (MaintainGrenadeToss())
			return;
	}
	else if ((pPlayer->m_nButtons & IN_ALT2) && !IsThrowingGrenade() && pPlayer->GetAmmoCount(GetAmmoDef()->Index("grenades")) && pPlayer->CanAttack())
	{
		bool bAllow = (m_flNextPrimaryAttack < GetCurrentTime());
		if (m_bInReload)
			bAllow = true;

		if (bAllow)
		{
			StartGrenadeToss();
			return;
		}
	}

	if (GetSwingTime() > 0 && GetSwingTime() <= GetCurrentTime())
	{
		Swing();
	}
	else if ( (pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= GetCurrentTime()) && pPlayer->CanAttack() )
	{
		PrimaryAttack();
	} 
	else if ( (pPlayer->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= GetCurrentTime()) && pPlayer->CanAttack() )
	{
		SecondaryAttack();
	}
	else 
	{
		WeaponIdle();
	}
}

void CWeaponDAMelee::PrimaryAttack()
{
	
#ifndef CLIENT_DLL
	CDAPlayer *pPlayer = ToDAPlayer( GetPlayerOwner() );
	pPlayer->NoteWeaponFired();
#endif

	StartSwing( false );
}

void CWeaponDAMelee::SecondaryAttack()
{
	StartSwing( true );
}

float CWeaponDAMelee::GetMeleeDamage( bool bIsSecondary ) const
{
	CDAPlayer *pPlayer = ToDAPlayer( GetOwner() );

	float flDamage = GetSDKWpnData().m_iDamage;

	if (bIsSecondary)
		flDamage = GetSDKWpnData().m_iSecondaryDamage;

	flDamage = pPlayer->m_Shared.ModifySkillValue(flDamage, 0.2f, SKILL_BOUNCER);

	if (pPlayer->m_Shared.IsDiving())
		flDamage *= 1.5f;
	else if (!pPlayer->GetGroundEntity())
		flDamage *= 1.2f;

	return flDamage;
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base code for any melee based weapon
//
//=====================================================================================//

#include "cbase.h"

#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#include "sdk_gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "animation.h"

#if defined( CLIENT_DLL )
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "ndebugoverlay.h"
	#include "te_effect_dispatch.h"
	#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKMelee, DT_WeaponSDKMelee )

BEGIN_NETWORK_TABLE( CWeaponSDKMelee, DT_WeaponSDKMelee )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSDKMelee )
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponSDKMelee::CWeaponSDKMelee()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CWeaponSDKMelee::Spawn( void )
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
void CWeaponSDKMelee::Precache( void )
{
	//Call base class first
	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponSDKMelee::ItemPostFrame( void )
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
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

	if ( !( pPlayer->m_nButtons & (IN_ATTACK|IN_ATTACK2) ) && pPlayer->GetPunchesThrown() )
	{
		pPlayer->ClearPunchesThrown();
	}
}

void CWeaponSDKMelee::PrimaryAttack()
{
	StartSwing( false );
}

void CWeaponSDKMelee::SecondaryAttack()
{
	StartSwing( true );
}

float CWeaponSDKMelee::GetMeleeDamage( bool bIsSecondary ) const
{
	CSDKPlayer *pPlayer = ToSDKPlayer( GetOwner() );

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

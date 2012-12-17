//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shotgun.h"
#include "sdk_fx_shared.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponShotgun, DT_WeaponShotgun )

BEGIN_NETWORK_TABLE( CWeaponShotgun, DT_WeaponShotgun )

	#ifdef CLIENT_DLL
		RecvPropInt( RECVINFO( m_iInSpecialReload ) )
	#else
		SendPropInt( SENDINFO( m_iInSpecialReload ), 2, SPROP_UNSIGNED )
	#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponShotgun )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_iInSpecialReload, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_shotgun, CWeaponShotgun );
PRECACHE_WEAPON_REGISTER( weapon_shotgun );



CWeaponShotgun::CWeaponShotgun()
{
	m_flPumpTime = 0;
}

void CWeaponShotgun::PrimaryAttack()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = GetCurrentTime() + 0.15;
		return;
	}

	BaseClass::PrimaryAttack();

	if ( m_iClip1 > 0 )
		m_iInSpecialReload = 0;
}


bool CWeaponShotgun::Reload()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 || m_iClip1 == GetMaxClip1())
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > GetCurrentTime())
		return true;
		
	CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
	float flSpeedMultiplier = GetSDKWpnData().m_flReloadTimeMultiplier;

	// check to see if we're ready to reload
	if (m_iInSpecialReload == 0)
	{
		pPlayer->SetAnimation( PLAYER_RELOAD );

		float flStartTime = 0.5f * flSpeedMultiplier;

		SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );
		m_iInSpecialReload = 1;
		pPlayer->m_flNextAttack = GetCurrentTime() + flStartTime;
		m_flNextPrimaryAttack = GetCurrentTime() + flStartTime;
		m_flNextSecondaryAttack = GetCurrentTime() + flStartTime;
		SetWeaponIdleTime( GetCurrentTime() + flStartTime );

		if (vm)
			vm->SetPlaybackRate( 1/flSpeedMultiplier );

		pPlayer->Instructor_LessonLearned("reload");
		return true;
	}
	else if (m_iInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > GetCurrentTime())
			return true;
		// was waiting for gun to move to side
		m_iInSpecialReload = 2;

		float flReloadTime = 0.45 * flSpeedMultiplier;

		SendWeaponAnim( ACT_VM_RELOAD );
		SetWeaponIdleTime( GetCurrentTime() + flReloadTime );

		if (vm)
			vm->SetPlaybackRate( 1/flSpeedMultiplier );
	}
	else if ( m_iInSpecialReload == 2 ) // Sanity, make sure it's actually in the right state.
	{
		// Add them to the clip
		m_iClip1 += 1;
		
#ifdef GAME_DLL
		SendReloadEvents();
#endif
		CSDKPlayer *pPlayer = GetPlayerOwner();

		if ( pPlayer )
			 pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

		m_iInSpecialReload = 1;
	}

	return true;
}


void CWeaponShotgun::WeaponIdle()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (m_flPumpTime && m_flPumpTime < GetCurrentTime())
	{
		// play pumping sound
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle < GetCurrentTime())
	{
		if (m_iClip1 == 0 && m_iInSpecialReload == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ))
		{
			Reload( );
		}
		else if (m_iInSpecialReload != 0)
		{
			if (m_iClip1 != 8 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ))
			{
				Reload( );
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );
				
				// play cocking sound
				m_iInSpecialReload = 0;
				SetWeaponIdleTime( GetCurrentTime() + 1.5 );
				m_flNextPrimaryAttack = GetCurrentTime() + 0.15; // Add a small delay between finishing reload and firing again
			}
		}
		else
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

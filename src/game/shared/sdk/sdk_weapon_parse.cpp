//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "sdk_weapon_parse.h"


FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CSDKWeaponInfo;
}


CSDKWeaponInfo::CSDKWeaponInfo()
{
}


void CSDKWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_iDamage			= pKeyValuesData->GetInt( "Damage", 42 ); // Douglas Adams 1952 - 2001
	m_iBullets			= pKeyValuesData->GetInt( "Bullets", 1 );
	m_flCycleTime		= pKeyValuesData->GetFloat( "CycleTime", 0.15 );
	
	m_iDefaultAmmoClips = pKeyValuesData->GetInt( "NumClips", 2 );

	const char *pAnimEx = pKeyValuesData->GetString( "PlayerAnimationExtension", "mp5" );
	Q_strncpy( m_szAnimExtension, pAnimEx, sizeof( m_szAnimExtension ) );

	m_flWeaponFOV	= pKeyValuesData->GetFloat( "fov", 74.0f );

	m_flViewPunchMultiplier	= pKeyValuesData->GetFloat( "ViewPunchMultiplier", 1 );
	m_flRecoil				= pKeyValuesData->GetFloat( "Recoil", 10 );
	m_flSpread				= pKeyValuesData->GetFloat( "Spread", 0.01f );
	m_bAimInSpeedPenalty	= !!pKeyValuesData->GetInt( "AimInSpeedPenalty", 0 );
	m_bAimInFireRateBonus	= !!pKeyValuesData->GetInt( "AimInFireRateBonus", 0 );
	m_bAimInRecoilBonus		= !!pKeyValuesData->GetInt( "AimInRecoilBonus", 0 );
	m_bAimInSpreadBonus		= !!pKeyValuesData->GetInt( "AimInSpreadBonus", 0 );
}


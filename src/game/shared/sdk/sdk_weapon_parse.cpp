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
	m_iSecondaryDamage	= pKeyValuesData->GetInt( "SecondaryDamage", 35 );
	m_iBullets			= pKeyValuesData->GetInt( "Bullets", 1 );
	m_flCycleTime		= pKeyValuesData->GetFloat( "CycleTime", 0.15 );
	m_flSecondaryCycleTime	= pKeyValuesData->GetFloat( "SecondaryCycleTime", 0.35 );
	m_flReloadTimeMultiplier = pKeyValuesData->GetFloat( "ReloadTimeMultiplier", 1 );
	m_flDrawTimeMultiplier = pKeyValuesData->GetFloat( "DrawTimeMultiplier", 1 );

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

	const char* pszWeaponType = pKeyValuesData->GetString("WeaponType", "none");
	if (FStrEq(pszWeaponType, "rifle"))
		m_eWeaponType = WT_RIFLE;
	else if (FStrEq(pszWeaponType, "shotgun"))
		m_eWeaponType = WT_SHOTGUN;
	else if (FStrEq(pszWeaponType, "smg"))
		m_eWeaponType = WT_SMG;
	else if (FStrEq(pszWeaponType, "pistol"))
		m_eWeaponType = WT_PISTOL;
	else if (FStrEq(pszWeaponType, "melee"))
		m_eWeaponType = WT_MELEE;
	else
		m_eWeaponType = WT_NONE;
}

CSDKWeaponInfo* CSDKWeaponInfo::GetWeaponInfo(SDKWeaponID eWeapon)
{
	const char* pszAlias = WeaponIDToAlias( eWeapon );

	Assert(pszAlias);
	if (!pszAlias)
		return NULL;

	char szName[128];
	Q_snprintf( szName, sizeof( szName ), "weapon_%s", pszAlias );

	WEAPON_FILE_INFO_HANDLE hWeaponFile = LookupWeaponInfoSlot( szName );
	if (hWeaponFile == GetInvalidWeaponInfoHandle())
	{
		Assert(hWeaponFile != GetInvalidWeaponInfoHandle());
		return NULL;
	}

	return static_cast< CSDKWeaponInfo* >( GetFileWeaponInfoFromHandle( hWeaponFile ) );
}

static char* g_szWeaponTypes[] =
{
	"none",
	"melee",
	"rifle",
	"shotgun",
	"smg",
	"pistol",
};

weapontype_t CSDKWeaponInfo::StringToWeaponType( const char* szString )
{
	for (int i = 0; i < WT_MAX; i++)
	{
		if (Q_strcmp(szString, g_szWeaponTypes[i]) == 0)
			return (weapontype_t)i;
	}
	return WT_NONE;
}

const char* CSDKWeaponInfo::WeaponTypeToString( weapontype_t eWeapon )
{
	if (eWeapon == WT_NONE)
		return "none";

	if (eWeapon < 0 || eWeapon >= WT_MAX)
		return "none";

	return g_szWeaponTypes[eWeapon];
}


//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef DA_WEAPON_PARSE_H
#define DA_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"
#include "da_shareddefs.h"

//--------------------------------------------------------------------------------------------------------
class CSDKWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CSDKWeaponInfo, FileWeaponInfo_t );
	
	CSDKWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );

	char m_szAnimExtension[16];		// string used to generate player animations with this weapon

	int m_iDefaultAmmoClips;		//Tony; default number of clips the weapon comes with.
	// Parameters for FX_FireBullets:
	int		m_iDamage;
	int		m_iSecondaryDamage;
	int		m_iBullets;
	float	m_flCycleTime;
	float	m_flSecondaryCycleTime;
	float	m_flReloadTimeMultiplier;
	float	m_flDrawTimeMultiplier;

	float m_flWeaponFOV;		//Tony; added weapon fov, SDK uses models from a couple different games, so FOV is different.

	float	m_flViewPunchMultiplier;
	float	m_flRecoil;
	float	m_flSpread;
	bool	m_bAimInSpeedPenalty;
	bool	m_bAimInFireRateBonus;
	bool	m_bAimInRecoilBonus;
	bool	m_bAimInSpreadBonus;

	float   m_flStyleMultiplier;

	char    m_szAkimbo[32];
	char    m_szSingle[32];

	weapontype_t	m_eWeaponType;

	static CSDKWeaponInfo* GetWeaponInfo(DAWeaponID eWeapon);
	static weapontype_t    StringToWeaponType( const char* szString );
	static const char*     WeaponTypeToString( weapontype_t eWeapon );
};


#endif // DA_WEAPON_PARSE_H

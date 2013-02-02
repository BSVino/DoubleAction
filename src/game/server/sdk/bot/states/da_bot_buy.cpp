//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#include "cbase.h"
#include "sdk_gamerules.h"
#include "da_bot.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//--------------------------------------------------------------------------------------------------------------
ConVar bot_loadout( "bot_loadout", "", FCVAR_CHEAT, "bots are given these items at round start" );
ConVar bot_randombuy( "bot_randombuy", "0", FCVAR_CHEAT, "should bots ignore their prefered weapons and just buy weapons at random?" );

//--------------------------------------------------------------------------------------------------------------
/**
 *  Debug command to give a named weapon
 */
void CDABot::GiveWeapon( const char *weaponAlias )
{
	const char *translatedAlias = GetTranslatedWeaponAlias( weaponAlias );

	char wpnName[128];
	Q_snprintf( wpnName, sizeof( wpnName ), "weapon_%s", translatedAlias );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( wpnName );
	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		return;
	}

	CSDKWeaponInfo *pWeaponInfo = dynamic_cast< CSDKWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );
	if ( !pWeaponInfo )
	{
		return;
	}

	GiveNamedItem( wpnName );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Buy weapons, armor, etc.
 */
void BuyState::OnEnter( CDABot *me )
{
	m_retries = 0;
	m_prefRetries = 0;
	m_prefIndex = 0;

	m_doneBuying = false;

	m_isInitialDelay = true;

	m_buyPistol = false;

	me->PickRandomCharacter();
	me->BuyRandom();
	me->PickRandomSkill();

	me->State_Transition( STATE_ACTIVE );
}

struct BuyInfo
{
	weapontype_t type;
	char*        buyAlias;
	SDKWeaponID  id;
};

#define PRIMARY_WEAPON_BUY_COUNT 6

static BuyInfo primaryWeaponBuyInfo[ PRIMARY_WEAPON_BUY_COUNT ] =
{
	{ WT_SHOTGUN,   "mossberg", SDK_WEAPON_MOSSBERG },
	{ WT_RIFLE,     "fal",      SDK_WEAPON_FAL },
	{ WT_PISTOL,    "m1911",    SDK_WEAPON_M1911 },
	{ WT_PISTOL,    "p99",      SDK_WEAPON_P99 },
	{ WT_SMG,       "mp5k",     SDK_WEAPON_MP5K },
	{ WT_SMG,       "vector",   SDK_WEAPON_VECTOR },
};

/**
 * Given a weapon alias, return the kind of weapon it is
 */
inline weapontype_t GetWeaponType( const char *alias )
{
	int i;
	
	for( i=0; i<PRIMARY_WEAPON_BUY_COUNT; ++i )
	{
		if (!stricmp( alias, primaryWeaponBuyInfo[i].buyAlias ))
			return primaryWeaponBuyInfo[i].type;
	}

	return WT_MAX;
}




//--------------------------------------------------------------------------------------------------------------
void BuyState::OnUpdate( CDABot *me )
{
	// wait for a Navigation Mesh
	if (!TheNavMesh->IsLoaded())
		return;

	// if we're done buying and still in the freeze period, wait
	if (m_doneBuying)
		return;

	// If we're supposed to buy a specific weapon for debugging, do so and then bail
	const char *cheatWeaponString = bot_loadout.GetString();
	if ( cheatWeaponString && *cheatWeaponString )
	{
		CUtlVector<char*, CUtlMemory<char*> > loadout;
		Q_SplitString( cheatWeaponString, " ", loadout );
		for ( int i=0; i<loadout.Count(); ++i )
		{
			const char *item = loadout[i];
			for (int j = 0; j < PRIMARY_WEAPON_BUY_COUNT; j++)
			{
				if (FStrEq(primaryWeaponBuyInfo[j].buyAlias, item))
				{
					me->AddToLoadout(primaryWeaponBuyInfo[j].id);
					break;
				}
			}
		}
		m_doneBuying = true;
		return;
	}

	m_doneBuying = true;
}

//--------------------------------------------------------------------------------------------------------------
void BuyState::OnExit( CDABot *me )
{
	me->ResetStuckMonitor();
	me->EquipBestWeapon();
}


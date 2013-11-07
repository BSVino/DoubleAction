//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK CPlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "sdk_player.h"
#include "player_resource.h"
#include "sdk_player_resource.h"
#include <coordsize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Datatable
IMPLEMENT_SERVERCLASS_ST(CSDKPlayerResource, DT_SDKPlayerResource)
#if defined ( SDK_USE_PLAYERCLASSES )
	SendPropArray3( SENDINFO_ARRAY3(m_iPlayerClass), SendPropInt( SENDINFO_ARRAY(m_iPlayerClass), 4 ) ),
#endif
	SendPropArray3( SENDINFO_ARRAY3( m_iMaxHealth ), SendPropInt( SENDINFO_ARRAY( m_iMaxHealth ), 11, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iStyle ), SendPropInt( SENDINFO_ARRAY( m_iStyle ), 32, SPROP_UNSIGNED ) ),

	SendPropInt (SENDINFO (m_iHighestStyle)),
	SendPropInt (SENDINFO (m_iHighestStuntKills)),
	SendPropInt (SENDINFO (m_iHighestGrenadeKills)),
	SendPropInt (SENDINFO (m_iHighestBrawlKills)),
	SendPropInt (SENDINFO (m_iHighestKillStreak)),
	SendPropInt (SENDINFO (m_iHighestStylePlayer)),
	SendPropInt (SENDINFO (m_iHighestStuntKillPlayer)),
	SendPropInt (SENDINFO (m_iHighestGrenadeKillPlayer)),
	SendPropInt (SENDINFO (m_iHighestBrawlKillPlayer)),
	SendPropInt (SENDINFO (m_iHighestKillStreakPlayer)),
END_SEND_TABLE()

BEGIN_DATADESC( CSDKPlayerResource )
#if defined ( SDK_USE_PLAYERCLASSES )
	// DEFINE_ARRAY( m_iPlayerClass, FIELD_INTEGER, MAX_PLAYERS+1 ),
#endif
END_DATADESC()

LINK_ENTITY_TO_CLASS( sdk_player_manager, CSDKPlayerResource );

CSDKPlayerResource::CSDKPlayerResource( void )
{
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayerResource::UpdatePlayerData( void )
{
	int i;

	int iHighestStyle = 0;
	int iHighestStuntKills = 0;
	int iHighestGrenadeKills = 0;
	int iHighestBrawlKills = 0;
	int iHighestKillStreak = 0;

	int iHighestStylePlayer = 0;
	int iHighestStuntKillPlayer = 0;
	int iHighestGrenadeKillPlayer = 0;
	int iHighestBrawlKillPlayer = 0;
	int iHighestKillStreakPlayer = 0;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = (CSDKPlayer*)UTIL_PlayerByIndex( i );
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
#if defined ( SDK_USE_PLAYERCLASSES )
			m_iPlayerClass.Set( i, pPlayer->m_Shared.PlayerClass() );
#endif
			m_iMaxHealth.Set( i, pPlayer->GetMaxHealth() );
			m_iStyle.Set( i, pPlayer->GetTotalStyle() );

			if (pPlayer->GetTotalStyle() > iHighestStyle)
			{
				iHighestStyle = pPlayer->GetTotalStyle();
				iHighestStylePlayer = i;
			}

			if (pPlayer->m_iStuntKills > iHighestStuntKills)
			{
				iHighestStuntKills = pPlayer->m_iStuntKills;
				iHighestStuntKillPlayer = i;
			}

			if (pPlayer->m_iGrenadeKills > iHighestGrenadeKills)
			{
				iHighestGrenadeKills = pPlayer->m_iGrenadeKills;
				iHighestGrenadeKillPlayer = i;
			}

			if (pPlayer->m_iBrawlKills > iHighestBrawlKills)
			{
				iHighestBrawlKills = pPlayer->m_iBrawlKills;
				iHighestBrawlKillPlayer = i;
			}

			if (pPlayer->m_iStreakKills > iHighestKillStreak)
			{
				iHighestKillStreak = pPlayer->m_iStreakKills;
				iHighestKillStreakPlayer = i;
			}
		}
	}

	// Only overwrite the current high scorer if the new guy has passed him.
	// The first to get there should always keep it.
	if (iHighestStyle > m_iHighestStyle)
	{
		m_iHighestStyle = iHighestStyle;
		m_iHighestStylePlayer = iHighestStylePlayer;
	}

	if (iHighestStuntKills > m_iHighestStuntKills)
	{
		m_iHighestStuntKills = iHighestStuntKills;
		m_iHighestStuntKillPlayer = iHighestStuntKillPlayer;
	}

	if (iHighestGrenadeKills > m_iHighestGrenadeKills)
	{
		m_iHighestGrenadeKills = iHighestGrenadeKills;
		m_iHighestGrenadeKillPlayer = iHighestGrenadeKillPlayer;
	}

	if (iHighestBrawlKills > m_iHighestBrawlKills)
	{
		m_iHighestBrawlKills = iHighestBrawlKills;
		m_iHighestBrawlKillPlayer = iHighestBrawlKillPlayer;
	}

	if (iHighestKillStreak > m_iHighestKillStreak)
	{
		m_iHighestKillStreak = iHighestKillStreak;
		m_iHighestKillStreakPlayer = iHighestKillStreakPlayer;
	}

	BaseClass::UpdatePlayerData();
}

void CSDKPlayerResource::Spawn( void )
{
	int i;

	for ( i=0; i < MAX_PLAYERS+1; i++ )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		m_iPlayerClass.Set( i, PLAYERCLASS_UNDEFINED );
#endif
		m_iMaxHealth.Set( i, 1 );
		m_iStyle.Set( i, 0 );
	}

	m_iHighestStuntKills = -1;
	m_iHighestGrenadeKills = -1;
	m_iHighestBrawlKills = -1;
	m_iHighestKillStreak = -1;
	m_iHighestStuntKillPlayer = -1;
	m_iHighestGrenadeKillPlayer = -1;
	m_iHighestBrawlKillPlayer = -1;
	m_iHighestKillStreakPlayer = -1;

	BaseClass::Spawn();
}

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_SDK_PLAYER_RESOURCE_H
#define C_SDK_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_playerresource.h"

class C_DA_PlayerResource : public C_PlayerResource
{
	DECLARE_CLASS( C_DA_PlayerResource, C_PlayerResource );
public:
	DECLARE_CLIENTCLASS();

					C_DA_PlayerResource();
	virtual			~C_DA_PlayerResource();

#if defined ( SDK_USE_PLAYERCLASSES )
	int GetPlayerClass( int iIndex );
#endif
	
	int GetMaxHealth( int iIndex );
	int GetStyle( int iIndex );

	int GetHighestStyle() { return m_iHighestStyle; }
	int GetHighestStuntKills() { return m_iHighestStuntKills; }
	int GetHighestGrenadeKills() { return m_iHighestGrenadeKills; }
	int GetHighestBrawlKills() { return m_iHighestBrawlKills; }
	int GetHighestKillStreak() { return m_iHighestKillStreak; }

	int GetHighestStylePlayer() { return m_iHighestStylePlayer; }
	int GetHighestStuntKillPlayer() { return m_iHighestStuntKillPlayer; }
	int GetHighestGrenadeKillPlayer() { return m_iHighestGrenadeKillPlayer; }
	int GetHighestBrawlKillPlayer() { return m_iHighestBrawlKillPlayer; }
	int GetHighestKillStreakPlayer() { return m_iHighestKillStreakPlayer; }

protected:

#if defined ( SDK_USE_PLAYERCLASSES )
	int		m_iPlayerClass[MAX_PLAYERS+1];
#endif

	int		m_iMaxHealth[MAX_PLAYERS+1];
	int		m_iStyle[MAX_PLAYERS+1];

	CNetworkVar( int, m_iHighestStyle );
	CNetworkVar( int, m_iHighestStuntKills );
	CNetworkVar( int, m_iHighestGrenadeKills );
	CNetworkVar( int, m_iHighestBrawlKills );
	CNetworkVar( int, m_iHighestKillStreak );

	CNetworkVar( int, m_iHighestStylePlayer );
	CNetworkVar( int, m_iHighestStuntKillPlayer );
	CNetworkVar( int, m_iHighestGrenadeKillPlayer );
	CNetworkVar( int, m_iHighestBrawlKillPlayer );
	CNetworkVar( int, m_iHighestKillStreakPlayer );
};

C_DA_PlayerResource * DAGameResources( void );

#endif // C_SDK_PLAYERRESOURCE_H

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK CPlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_PLAYER_RESOURCE_H
#define SDK_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

class CSDKPlayerResource : public CPlayerResource
{
	DECLARE_CLASS( CSDKPlayerResource, CPlayerResource );
	
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CSDKPlayerResource();

	virtual void UpdatePlayerData( void );
	virtual void Spawn( void );

protected:
//	CNetworkArray( int, m_iObjScore, MAX_PLAYERS+1 );
#if defined ( SDK_USE_PLAYERCLASSES )
	CNetworkArray( int, m_iPlayerClass, MAX_PLAYERS+1 );
#endif

	CNetworkArray( int, m_iMaxHealth, MAX_PLAYERS+1 );

	CNetworkArray( int, m_iStyle, MAX_PLAYERS+1 );

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

#endif // SDK_PLAYER_RESOURCE_H

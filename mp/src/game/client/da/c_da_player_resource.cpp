//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_da_player_resource.h"
#include "c_da_player.h"
#include "da_gamerules.h"
#include <shareddefs.h>
#include "hud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_DA_PlayerResource, DT_DAPlayerResource, CDAPlayerResource)
#if defined ( SDK_USE_PLAYERCLASSES )
	RecvPropArray3( RECVINFO_ARRAY(m_iPlayerClass), RecvPropInt( RECVINFO(m_iPlayerClass[0]))),
#endif
	RecvPropArray3( RECVINFO_ARRAY( m_iMaxHealth ), RecvPropInt( RECVINFO( m_iMaxHealth[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iStyle ), RecvPropInt( RECVINFO( m_iStyle[0] ) ) ),

	RecvPropInt (RECVINFO (m_iHighestStyle)),
	RecvPropInt (RECVINFO (m_iHighestStuntKills)),
	RecvPropInt (RECVINFO (m_iHighestGrenadeKills)),
	RecvPropInt (RECVINFO (m_iHighestBrawlKills)),
	RecvPropInt (RECVINFO (m_iHighestKillStreak)),
	RecvPropInt (RECVINFO (m_iHighestStylePlayer)),
	RecvPropInt (RECVINFO (m_iHighestStuntKillPlayer)),
	RecvPropInt (RECVINFO (m_iHighestGrenadeKillPlayer)),
	RecvPropInt (RECVINFO (m_iHighestBrawlKillPlayer)),
	RecvPropInt (RECVINFO (m_iHighestKillStreakPlayer)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_DA_PlayerResource::C_DA_PlayerResource()
{
#if defined ( SDK_USE_TEAMS )
	m_Colors[SDK_TEAM_BLUE] = COLOR_BLUE;
	m_Colors[SDK_TEAM_RED] = COLOR_RED;
#endif

	m_Colors[TEAM_UNASSIGNED] = COLOR_WHITE;
	m_Colors[TEAM_SPECTATOR] = COLOR_GREY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_DA_PlayerResource::~C_DA_PlayerResource()
{
}

#if defined ( SDK_USE_PLAYERCLASSES )
int C_DA_PlayerResource::GetPlayerClass( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return PLAYERCLASS_UNDEFINED;

	return m_iPlayerClass[iIndex];
}

#endif // SDK_USE_PLAYERCLASSES

int C_DA_PlayerResource::GetMaxHealth( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return 1;

	return m_iMaxHealth[iIndex];
}

int C_DA_PlayerResource::GetStyle( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return 1;

	return m_iStyle[iIndex];
}


C_DA_PlayerResource * DAGameResources( void )
{
	return dynamic_cast<C_DA_PlayerResource *>(GameResources());
}

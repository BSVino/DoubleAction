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

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = (CSDKPlayer*)UTIL_PlayerByIndex( i );
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
#if defined ( SDK_USE_PLAYERCLASSES )
			m_iPlayerClass.Set( i, pPlayer->m_Shared.PlayerClass() );
#endif
		}
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
	}

	BaseClass::Spawn();
}

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_SDKTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_sdk_team.h"
#include "c_sdk_player_resource.h"

#include <vgui/ILocalize.h>
#include <tier3/tier3.h>
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Tony; undefine what I did in the header so everything from this point forward functions correctly.
#undef CSDKTeam

IMPLEMENT_CLIENTCLASS_DT(C_SDKTeam, DT_SDKTeam, CSDKTeam)
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
C_SDKTeam *GetGlobalSDKTeam( int iIndex )
{
	return (C_SDKTeam*)GetGlobalTeam( iIndex );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKTeam::C_SDKTeam()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKTeam::~C_SDKTeam()
{
}
char *C_SDKTeam::Get_Name( void )
{
	wchar_t *teamname;
	if (m_szTeamname[0] == '#')
	{
		teamname = g_pVGuiLocalize->Find(m_szTeamname);

		char ansi[128];
		g_pVGuiLocalize->ConvertUnicodeToANSI( teamname, ansi, sizeof( ansi ) );

		return strdup(ansi);
	}
	else 
		return m_szTeamname;
}

#if defined ( SDK_USE_PLAYERCLASSES )
void C_SDKTeam::AddPlayerClass( const char *szClassName )
{
	PLAYERCLASS_FILE_INFO_HANDLE hPlayerClassInfo;

	if ( ReadPlayerClassDataFromFileForSlot( filesystem, szClassName, &hPlayerClassInfo, GetEncryptionKey() ) )
	{
		m_hPlayerClassInfoHandles.AddToTail( hPlayerClassInfo );
	}
	else
	{
		Assert( !"missing playerclass script file" );
		Msg( "Missing playerclass script file for class: %s\n", szClassName );
	}	
}

const CSDKPlayerClassInfo &C_SDKTeam::GetPlayerClassInfo( int iPlayerClass ) const
{
	Assert( iPlayerClass >= 0 && iPlayerClass < m_hPlayerClassInfoHandles.Count() );

	const FilePlayerClassInfo_t *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassInfoHandles[iPlayerClass] );
	const CSDKPlayerClassInfo *pSDKInfo;

#ifdef _DEBUG
	pSDKInfo = dynamic_cast< const CSDKPlayerClassInfo* >( pPlayerClassInfo );
	Assert( pSDKInfo );
#else
	pSDKInfo = static_cast< const CSDKPlayerClassInfo* >( pPlayerClassInfo );
#endif

	return *pSDKInfo;
}

bool C_SDKTeam::IsClassOnTeam( const char *pszClassName, int &iClassNum ) const
{
	iClassNum = PLAYERCLASS_UNDEFINED;

	// Random is always on every team
	if( FStrEq( pszClassName, "cls_random" ) )
	{
		iClassNum = PLAYERCLASS_RANDOM;
		return true;
	}
	
	for( int i=0;i<m_hPlayerClassInfoHandles.Count(); i++ )
	{
		FilePlayerClassInfo_t *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassInfoHandles[i] );

		if( stricmp( pszClassName, pPlayerClassInfo->m_szSelectCmd ) == 0 )
		{
			iClassNum = i;
			return true;
		}
	}

	return false;
}

bool C_SDKTeam::IsClassOnTeam( int iClassNum ) const
{
	return ( iClassNum >= 0 && iClassNum < m_hPlayerClassInfoHandles.Count() );
}

int C_SDKTeam::CountPlayersOfThisClass( int iPlayerClass )
{
	int count = 0;

	C_SDK_PlayerResource *sdk_PR = dynamic_cast<C_SDK_PlayerResource *>(g_PR);

	Assert( sdk_PR );

	for ( int i=0;i<Get_Number_Players();i++ )
	{
		if ( iPlayerClass == sdk_PR->GetPlayerClass(m_aPlayers[i]) )
			count++;
	}

	return count;
}
#endif // SDK_USE_PLAYERCLASSES


IMPLEMENT_CLIENTCLASS_DT(C_SDKTeam_Unassigned, DT_SDKTeam_Unassigned, CSDKTeam_Unassigned)
END_RECV_TABLE()

C_SDKTeam_Unassigned::C_SDKTeam_Unassigned()
{
//Tony; If we're NOT using teams, and are using playerclasses - load up the playerclasses
#if defined ( SDK_USE_PLAYERCLASSES ) && !defined ( SDK_USE_TEAMS )
	//parse our classes
	int i = 0;
	while( pszPlayerClasses[i] != NULL )
	{
		AddPlayerClass( pszPlayerClasses[i] );
		i++;
	}	
#endif
}

#if defined ( SDK_USE_TEAMS )
IMPLEMENT_CLIENTCLASS_DT(C_SDKTeam_Blue, DT_SDKTeam_Blue, CSDKTeam_Blue)
END_RECV_TABLE()

C_SDKTeam_Blue::C_SDKTeam_Blue()
{
#if defined ( SDK_USE_PLAYERCLASSES )
	//parse our classes
	int i = 0;
	while( pszTeamBlueClasses[i] != NULL )
	{
		AddPlayerClass( pszTeamBlueClasses[i] );
		i++;
	}	
#endif
}


IMPLEMENT_CLIENTCLASS_DT(C_SDKTeam_Red, DT_SDKTeam_Red, CSDKTeam_Red)
END_RECV_TABLE()

C_SDKTeam_Red::C_SDKTeam_Red()
{
#if defined ( SDK_USE_PLAYERCLASSES )
	//parse our classes
	int i = 0;
	while( pszTeamRedClasses[i] != NULL )
	{
		AddPlayerClass( pszTeamRedClasses[i] );
		i++;
	}	
#endif
}
#endif // SDK_USE_TEAMS

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "da_team.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// Datatable
IMPLEMENT_SERVERCLASS_ST(CDATeam, DT_DATeam)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( da_team_manager, CDATeam );

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
CDATeam *GetGlobalDATeam( int iIndex )
{
	return (CDATeam*)GetGlobalTeam( iIndex );
}


//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
void CDATeam::Init( const char *pName, int iNumber )
{
	BaseClass::Init( pName, iNumber );

	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );
}

#if defined ( SDK_USE_PLAYERCLASSES )
void CDATeam::AddPlayerClass( const char *szClassName )
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

const CDAPlayerClassInfo &CDATeam::GetPlayerClassInfo( int iPlayerClass ) const
{
	Assert( iPlayerClass >= 0 && iPlayerClass < m_hPlayerClassInfoHandles.Count() );

	const FilePlayerClassInfo_t *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassInfoHandles[iPlayerClass] );
	const CDAPlayerClassInfo *pSDKInfo;

	#ifdef _DEBUG
		pSDKInfo = dynamic_cast< const CDAPlayerClassInfo* >( pPlayerClassInfo );
		Assert( pSDKInfo );
	#else
		pSDKInfo = static_cast< const CDAPlayerClassInfo* >( pPlayerClassInfo );
	#endif

	return *pSDKInfo;
}

bool CDATeam::IsClassOnTeam( const char *pszClassName, int &iClassNum ) const
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
#endif // SDK_USE_PLAYERCLASSES
void CDATeam::ResetScores( void )
{
	SetRoundsWon(0);
	SetScore(0);
}

//
// TEAMS
//

#if defined ( SDK_USE_TEAMS )

// BLUE TEAM
//==================

class CDATeam_Blue : public CDATeam
{
	DECLARE_CLASS( CDATeam_Blue, CDATeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

#if defined ( SDK_USE_PLAYERCLASSES )
		int i = 0;
		while( pszTeamBlueClasses[i] != NULL )
		{
			AddPlayerClass( pszTeamBlueClasses[i] );
			i++;
		}	
#endif 
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Blue"; }
};

IMPLEMENT_SERVERCLASS_ST(CDATeam_Blue, DT_DATeam_Blue)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( da_team_blue, CDATeam_Blue );


// REDTEAM
//==================

class CDATeam_Red : public CDATeam
{
	DECLARE_CLASS( CDATeam_Red, CDATeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

#if defined ( SDK_USE_PLAYERCLASSES )
		int i = 0;
		while( pszTeamRedClasses[i] != NULL )
		{
			AddPlayerClass( pszTeamRedClasses[i] );
			i++;
		}	
#endif
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Red"; }
};

IMPLEMENT_SERVERCLASS_ST(CDATeam_Red, DT_DATeam_Red)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( da_team_red, CDATeam_Red );
#endif

// UNASSIGNED
//==================

class CDATeam_Unassigned : public CDATeam
{
	DECLARE_CLASS( CDATeam_Unassigned, CDATeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );
//Tony; If we're NOT using teams, and are using playerclasses - load up the playerclasses
#if defined ( SDK_USE_PLAYERCLASSES ) && !defined ( SDK_USE_TEAMS )
		int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}
#endif
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Unassigned"; }
};

IMPLEMENT_SERVERCLASS_ST(CDATeam_Unassigned, DT_DATeam_Unassigned)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( da_team_unassigned, CDATeam_Unassigned );


class CDATeam_Deathmatch : public CDATeam
{
	DECLARE_CLASS( CDATeam_Deathmatch, CDATeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Deathmatch"; }
};

IMPLEMENT_SERVERCLASS_ST(CDATeam_Deathmatch, DT_DATeam_Deathmatch)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( da_team_deathmatch, CDATeam_Deathmatch );


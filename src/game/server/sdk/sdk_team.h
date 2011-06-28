//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_TEAM_H
#define SDK_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "team.h"
#include "playerclass_info_parse.h"
#include "sdk_playerclass_info_parse.h"

#if defined ( SDK_USE_PLAYERCLASSES )
typedef CUtlLinkedList< PLAYERCLASS_FILE_INFO_HANDLE, int > PlayerClassInfoList;
#endif

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CSDKTeam : public CTeam
{
	DECLARE_CLASS( CSDKTeam, CTeam );
	DECLARE_SERVERCLASS();

public:

	// Initialization
	virtual void Init( const char *pName, int iNumber );
	const unsigned char *GetEncryptionKey( void ) { return g_pGameRules->GetEncryptionKey(); }

#if defined ( SDK_USE_PLAYERCLASSES )
	CSDKPlayerClassInfo const &GetPlayerClassInfo( int iPlayerClass ) const;

	virtual void AddPlayerClass( const char *pszClassName );

	bool IsClassOnTeam( const char *pszClassName, int &iClassNum ) const;
	int GetNumPlayerClasses( void ) { return m_hPlayerClassInfoHandles.Count(); }
#endif  // SDK_USE_PLAYERCLASSES

	virtual const char *GetTeamName( void ) { return "#Teamname_Spectators"; }

	void ResetScores( void );

private:
#if defined ( SDK_USE_PLAYERCLASSES )
	CUtlVector < PLAYERCLASS_FILE_INFO_HANDLE >		m_hPlayerClassInfoHandles;
#endif  // SDK_USE_PLAYERCLASSES

};


extern CSDKTeam *GetGlobalSDKTeam( int iIndex );


#endif // TF_TEAM_H

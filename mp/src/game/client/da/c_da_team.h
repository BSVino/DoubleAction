//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_SDK_TEAM_H
#define C_SDK_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"
#include "da_shareddefs.h"
#include "da_playerclass_info_parse.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//Tony; so we can call this from shared code!
#define CDATeam C_DATeam

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_DATeam : public C_Team
{
	DECLARE_CLASS( C_DATeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_DATeam();
	virtual			~C_DATeam();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );
	virtual char	*Get_Name( void );

#if defined ( SDK_USE_PLAYERCLASSES )
	CDAPlayerClassInfo const &GetPlayerClassInfo( int iPlayerClass ) const;
	const unsigned char *GetEncryptionKey( void ) { return g_pGameRules->GetEncryptionKey(); }

	virtual void AddPlayerClass( const char *pszClassName );

	bool IsClassOnTeam( const char *pszClassName, int &iClassNum ) const;
	bool IsClassOnTeam( int iClassNum ) const;
	int GetNumPlayerClasses( void ) { return m_hPlayerClassInfoHandles.Count(); }

	int CountPlayersOfThisClass( int iPlayerClass );
#endif // SDK_USE_PLAYERCLASSES

private:
#if defined ( SDK_USE_PLAYERCLASSES )
	CUtlVector < PLAYERCLASS_FILE_INFO_HANDLE >		m_hPlayerClassInfoHandles;
#endif
};

class C_DATeam_Unassigned : public C_DATeam
{
	DECLARE_CLASS( C_DATeam_Unassigned, C_DATeam );
public:
	DECLARE_CLIENTCLASS();

				     C_DATeam_Unassigned();
	 virtual		~C_DATeam_Unassigned() {}
};

#if defined ( SDK_USE_TEAMS )
class C_DATeam_Blue : public C_DATeam
{
	DECLARE_CLASS( C_DATeam_Blue, C_DATeam );
public:
	DECLARE_CLIENTCLASS();

				     C_DATeam_Blue();
	 virtual		~C_DATeam_Blue() {}
};

class C_DATeam_Red : public C_DATeam
{
	DECLARE_CLASS( C_DATeam_Red, C_DATeam );
public:
	DECLARE_CLIENTCLASS();

					 C_DATeam_Red();
	virtual			~C_DATeam_Red() {}
};
#endif // SDK_USE_TEAMS

class C_DATeam_Deathmatch : public C_DATeam
{
	DECLARE_CLASS( C_DATeam_Deathmatch, C_DATeam );
public:
	DECLARE_CLIENTCLASS();

				     C_DATeam_Deathmatch();
	 virtual		~C_DATeam_Deathmatch() {}
};

extern C_DATeam *GetGlobalDATeam( int iIndex );

#endif // C_SDK_TEAM_H

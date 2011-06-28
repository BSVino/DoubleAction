//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "../EventLog.h"
#include "KeyValues.h"

#include "sdk_player.h"
#include "sdk_team.h"

class CSDKEventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CSDKEventLog() {};

public:
	bool PrintEvent( IGameEvent * event )	// override virtual function
	{
		if ( !PrintSDKEvent( event ) )
		{
			return BaseClass::PrintEvent( event );
		}
		else
		{
			return true;
		}

		return false;
	}
	bool Init()
	{
		BaseClass::Init();

		ListenForGameEvent( "player_death" );
		ListenForGameEvent( "player_hurt" );
#if defined ( SDK_USE_PLAYERCLASSES )
		ListenForGameEvent( "player_changeclass" );
#endif // SDK_USE_PLAYERCLASSES

		return true;
	}
protected:

	bool PrintSDKEvent( IGameEvent * event )	// print Mod specific logs
	{
		const char *eventName = event->GetName();
	
		if ( !Q_strncmp( eventName, "server_", strlen("server_")) )
		{
			return false; // ignore server_ messages, always.
		}

#if defined ( SDK_USE_PLAYERCLASSES )
		if ( FStrEq( eventName, "player_changeclass" ) )
		{
			const int userid = event->GetInt( "userid" );
			CBasePlayer *pPlayer = UTIL_PlayerByUserId( userid );

			if ( !pPlayer )
			{
				return false;
			}

			int iClass = event->GetInt("class");
			int iTeam = pPlayer->GetTeamNumber();
//Tony; when using teams, the player can only select classes on blue or red.
#if defined ( SDK_USE_TEAMS )
			if ( iTeam != SDK_TEAM_BLUE && iTeam != SDK_TEAM_RED )
#else
			if ( iTeam != TEAM_UNASSIGNED )
#endif
				return true;

			CSDKTeam *pTeam = GetGlobalSDKTeam( iTeam );

			if ( iClass == PLAYERCLASS_RANDOM )
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed role to \"Random\"\n",  
					pPlayer->GetPlayerName(),
					userid,
					pPlayer->GetNetworkIDString(),
					pTeam->GetName()
					);
			}
			else if ( iClass < GetGlobalSDKTeam(iTeam)->GetNumPlayerClasses() )
			{
				const CSDKPlayerClassInfo &pInfo = GetGlobalSDKTeam(iTeam)->GetPlayerClassInfo( iClass );

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed role to \"%s\"\n",  
					pPlayer->GetPlayerName(),
					userid,
					pPlayer->GetNetworkIDString(),
					pTeam->GetName(),
					pInfo.m_szPrintName
					);
			}
			return true;
		}
#endif // SDK_USE_PLAYERCLASSES
		return false;
	}

};

CSDKEventLog g_SDKEventLog;

//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IGameSystem* GameLogSystem()
{
	return &g_SDKEventLog;
}


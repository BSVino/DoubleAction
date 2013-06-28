//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client DLL VGUI2 Viewport
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#pragma warning( disable : 4800  )  // disable forcing int to bool performance warning

// VGUI panel includes
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/Cursor.h>
#include <vgui/IScheme.h>
#include <vgui/IVGUI.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>

// client dll/engine defines
#include "hud.h"
#include <voice_status.h>

// viewport definitions
#include <baseviewport.h>
#include "SDKViewport.h"

#include "vguicenterprint.h"
#include "text_message.h"
#include "c_sdk_player.h"
#include "sdk_scoreboard.h"
#include "sdk_textwindow.h"
#include "sdk_spectatorgui.h"
#if defined ( SDK_USE_PLAYERCLASSES )
#include "sdk_classmenu.h"
#endif
#if defined ( SDK_USE_TEAMS )
#include "sdk_teammenu.h"
#endif

#if defined ( SDK_USE_TEAMS )
CON_COMMAND_F( changeteam, "Choose a new team", FCVAR_SERVER_CAN_EXECUTE|FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( pPlayer && pPlayer->CanShowTeamMenu() )
	{
		gViewPortInterface->ShowPanel( PANEL_TEAM, true );
	}
}
#endif // SDK_USE_TEAMS

#if defined ( SDK_USE_PLAYERCLASSES )
CON_COMMAND_F( changeclass, "Choose a new class", FCVAR_SERVER_CAN_EXECUTE|FCVAR_CLIENTCMD_CAN_EXECUTE )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( pPlayer && pPlayer->CanShowClassMenu())
	{
		switch( pPlayer->GetTeamNumber() )
		{
#if defined ( SDK_USE_TEAMS )
		case SDK_TEAM_BLUE:
			gViewPortInterface->ShowPanel( PANEL_CLASS_BLUE, true );
			break;
		case SDK_TEAM_RED:
			gViewPortInterface->ShowPanel( PANEL_CLASS_RED, true );
			break;
#else
		case TEAM_UNASSIGNED:
			gViewPortInterface->ShowPanel( PANEL_CLASS, true );
			break;
#endif
		default:
			break;
		}
	}
}
#endif // SDK_USE_PLAYERCLASSES


CON_COMMAND_F( spec_help, "Show spectator help screen", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
	if ( gViewPortInterface )
		gViewPortInterface->ShowPanel( PANEL_INFO, true );
}

CON_COMMAND_F( spec_menu, "Activates spectator menu", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
	bool bShowIt = true;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( pPlayer && !pPlayer->IsObserver() )
		return;

	if ( args.ArgC() == 2 )
	{
		 bShowIt = atoi( args[ 1 ] ) == 1;
	}
	
	if ( gViewPortInterface )
		gViewPortInterface->ShowPanel( PANEL_SPECMENU, bShowIt );
}

CON_COMMAND_F( togglescores, "Toggles score panel", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
	if ( !gViewPortInterface )
		return;
	
	IViewPortPanel *scoreboard = gViewPortInterface->FindPanelByName( PANEL_SCOREBOARD );

	if ( !scoreboard )
		return;

	if ( scoreboard->IsVisible() )
	{
		gViewPortInterface->ShowPanel( scoreboard, false );
		GetClientVoiceMgr()->StopSquelchMode();
	}
	else
	{
		gViewPortInterface->ShowPanel( scoreboard, true );
	}
}

void SDKViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}


IViewPortPanel* SDKViewport::CreatePanelByName(const char *szPanelName)
{
	IViewPortPanel* newpanel = NULL;

	if ( Q_strcmp( PANEL_SCOREBOARD, szPanelName) == 0 )
	{
		newpanel = new CSDKScoreboard( this );
	}
	else if ( Q_strcmp( PANEL_INFO, szPanelName) == 0 )
	{
		newpanel = new CSDKTextWindow( this );
	}
	else if ( Q_strcmp(PANEL_SPECGUI, szPanelName) == 0 )
	{
		newpanel = new CSDKSpectatorGUI( this );	
	}

#if defined ( SDK_USE_PLAYERCLASSES )
	#if !defined ( SDK_USE_TEAMS )
		else if ( Q_strcmp( PANEL_CLASS_NOTEAMS, szPanelName) == 0 )
		{
			newpanel = new CSDKClassMenu_NoTeams( this );
		}
	#else
		else if ( Q_strcmp( PANEL_CLASS_BLUE, szPanelName) == 0 )
		{
			newpanel = new CSDKClassMenu_Blue( this );
		}
		else if ( Q_strcmp( PANEL_CLASS_RED, szPanelName) == 0 )
		{
			newpanel = new CSDKClassMenu_Red( this );
		}
	#endif
#endif
#if defined ( SDK_USE_TEAMS )
	else if ( Q_strcmp( PANEL_TEAM, szPanelName) == 0 )
	{
		newpanel = new CSDKTeamMenu( this );
	}
#endif
	else
	{
		// create a generic base panel, don't add twice
		newpanel = BaseClass::CreatePanelByName( szPanelName );
	}

	return newpanel; 
}

void SDKViewport::CreateDefaultPanels( void )
{
#if defined ( SDK_USE_PLAYERCLASSES )
	#if !defined ( SDK_USE_TEAMS )
		AddNewPanel( CreatePanelByName( PANEL_CLASS_NOTEAMS ), "PANEL_CLASS_NOTEAMS" );
	#else
		AddNewPanel( CreatePanelByName( PANEL_CLASS_BLUE ), "PANEL_CLASS_BLUE" );
		AddNewPanel( CreatePanelByName( PANEL_CLASS_RED ), "PANEL_CLASS_RED" );
	#endif
#endif
#if defined ( SDK_USE_TEAMS )
	AddNewPanel( CreatePanelByName( PANEL_TEAM ), "PANEL_TEAM" );
#endif
	BaseClass::CreateDefaultPanels();
}

int SDKViewport::GetDeathMessageStartHeight( void )
{
	int x = YRES(2);

	IViewPortPanel *spectator = gViewPortInterface->FindPanelByName( PANEL_SPECGUI );

	//TODO: Link to actual height of spectator bar
	if ( spectator && spectator->IsVisible() )
	{
		x += YRES(52);
	}

	return x;
}


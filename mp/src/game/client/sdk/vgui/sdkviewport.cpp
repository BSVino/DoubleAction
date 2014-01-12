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
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>

// client dll/engine defines
#include "hud.h"
#include <voice_status.h>

// viewport definitions
#include <baseviewport.h>
#include "sdkviewport.h"

#include "vguicenterprint.h"
#include "text_message.h"
#include "c_sdk_player.h"
#include "hud/sdk_scoreboard.h"
#include "sdk_textwindow.h"
#include "sdk_spectatorgui.h"
#if defined ( SDK_USE_PLAYERCLASSES )
#include "sdk_classmenu.h"
#endif
#if defined ( SDK_USE_TEAMS )
#include "sdk_teammenu.h"
#endif
#include "da_buymenu.h"
#include "da_skillmenu.h"
#include "da_charactermenu.h"
#include "sdk_mapinfo.h"
#include "da_menu_background.h"
#include "folder_gui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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

SDKViewport::SDKViewport()
{
	m_pMainMenuPanel = NULL;
}

SDKViewport::~SDKViewport()
{
	if ( !m_bHasParent && m_pMainMenuPanel )
	{
		m_pMainMenuPanel->MarkForDeletion();
	}
	m_pMainMenuPanel = NULL;
}

void SDKViewport::Start(IGameUIFuncs *pGameUIFuncs, IGameEventManager2 *pGameEventManager)
{
/*	m_pMainMenuPanel = new CDAMainMenu( NULL, NULL );
	m_pMainMenuPanel->SetZPos( 500 );
	m_pMainMenuPanel->SetVisible( false );
	m_pMainMenuPanel->StartVideo();*/

	BaseClass::Start(pGameUIFuncs, pGameEventManager);
}

void SDKViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}

void SDKViewport::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	bool bRestartMainMenuVideo = false;
	if (m_pMainMenuPanel)
		bRestartMainMenuVideo = m_pMainMenuPanel->IsVideoPlaying();

	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

/*	m_pMainMenuPanel = new CDAMainMenu( NULL, NULL );
	m_pMainMenuPanel->SetZPos( 500 );
	m_pMainMenuPanel->SetVisible( false );
	if (bRestartMainMenuVideo)
		m_pMainMenuPanel->StartVideo();*/
}

void SDKViewport::RemoveAllPanels( void)
{
	if (m_pMainMenuPanel)
	{
		m_pMainMenuPanel->MarkForDeletion();
		m_pMainMenuPanel = NULL;
	}

	BaseClass::RemoveAllPanels();
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
	else if ( Q_strcmp( PANEL_INTRO, szPanelName) == 0 )
	{
		newpanel = new CSDKMapInfo( this );
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
	else if ( Q_strcmp( PANEL_FOLDER, szPanelName) == 0 )
	{
		newpanel = new CFolderMenu( this );
	}
	else
	{
		// create a generic base panel, don't add twice
		newpanel = BaseClass::CreatePanelByName( szPanelName );
	}

	return newpanel; 
}

void SDKViewport::CreateDefaultPanels( void )
{
	AddNewPanel( CreatePanelByName( PANEL_INTRO ), "PANEL_INTRO" );
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

	AddNewPanel( CreatePanelByName( PANEL_FOLDER ), "PANEL_FOLDER" );

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

void SDKViewport::StopMainMenuVideo()
{
	if (m_pMainMenuPanel)
		m_pMainMenuPanel->StopVideo();
}

void SDKViewport::StartMainMenuVideo()
{
	if (m_pMainMenuPanel)
		m_pMainMenuPanel->StartVideo();
}

#define XPROJECT(x)	( (1.0f+(x))*ScreenWidth()*0.5f )
#define YPROJECT(y) ( (1.0f-(y))*ScreenHeight()*0.5f )

int ScreenTransform( const Vector& point, Vector& screen );

static void RotateVertex( float *x, float *y, float flAngle )
{
	float flRadians = DEG2RAD( flAngle );
	float x1, y1;

	x1 = *x * cos(flRadians) - *y * sin(flRadians);
	y1 = *x * sin(flRadians) + *y * cos(flRadians);

	*x = x1;
	*y = y1;
}

// GOOSEMAN : Renders a textured polygon onto the screen
bool SDKViewport::DrawPolygon( CHudTexture* pTexture, Vector vecWorldPosition, float flWidth, float flHeight, float flRotation, const Color& c )
{
	int x, y;
	Vector screenOrigin;
	ScreenTransform( vecWorldPosition, screenOrigin );
	x = XPROJECT(screenOrigin.x);
	y = YPROJECT(screenOrigin.y);

	return DrawPolygon( pTexture, XPROJECT(screenOrigin.x), YPROJECT(screenOrigin.y), flWidth, flHeight, flRotation, c );
}

// This is a heavily modified version of a function Minh wrote for TacInt
bool SDKViewport::DrawPolygon( CHudTexture* pTexture, float x, float y, float flWidth, float flHeight, float flRotation, const Color& c )
{
	if ( !pTexture )
		return false;

	float flOffsetX, flOffsetY;
	flOffsetX = flWidth/2; flOffsetY = flHeight/2;

	float vertex1[2], vertex2[2], vertex3[2], vertex4[2];

	vertex1[0] = -flOffsetX;	vertex1[1] = flOffsetY;
	vertex2[0] = flOffsetX;		vertex2[1] = flOffsetY;
	vertex3[0] = flOffsetX;		vertex3[1] = -flOffsetY;
	vertex4[0] = -flOffsetX;	vertex4[1] = -flOffsetY;

	if ( flRotation )
	{
		RotateVertex( &vertex1[0], &vertex1[1], flRotation );
		RotateVertex( &vertex2[0], &vertex2[1], flRotation );
		RotateVertex( &vertex3[0], &vertex3[1], flRotation );
		RotateVertex( &vertex4[0], &vertex4[1], flRotation );
	}

	// Adjust vertex co-ord so it's relative to the center of the icon
	vertex1[0] += x + flOffsetX;	vertex1[1] += y + flOffsetY;
	vertex2[0] += x + flOffsetX;	vertex2[1] += y + flOffsetY;
	vertex3[0] += x + flOffsetX;	vertex3[1] += y + flOffsetY;
	vertex4[0] += x + flOffsetX;	vertex4[1] += y + flOffsetY;

	Vector2D uv11( pTexture->texCoords[0], pTexture->texCoords[1] );
	Vector2D uv12( pTexture->texCoords[0], pTexture->texCoords[3] );
	Vector2D uv21( pTexture->texCoords[2], pTexture->texCoords[1] );
	Vector2D uv22( pTexture->texCoords[2], pTexture->texCoords[3] );

	// Draw the main icon
	vgui::Vertex_t vert[4];	
	vgui::surface()->DrawSetColor( c );
	vgui::surface()->DrawSetTexture( pTexture->textureId );

	vert[0].Init( Vector2D( vertex4[0], vertex4[1] ), uv11 );
	vert[1].Init( Vector2D( vertex3[0], vertex3[1] ), uv21 );
	vert[2].Init( Vector2D( vertex2[0], vertex2[1] ), uv22 );
	vert[3].Init( Vector2D( vertex1[0], vertex1[1] ), uv12 );

	vgui::surface()->DrawTexturedPolygon(4, vert);

	return true;
}

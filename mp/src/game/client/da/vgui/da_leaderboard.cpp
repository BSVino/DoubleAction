//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"

#include "da_leaderboard.h"

#include "steam/isteamfriends.h"
#include "steam/steam_api.h"

#include <cdll_client_int.h>
#include <ienginevgui.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

#include <filesystem.h>
#include <convar.h>

#include "da.h"

// Don't care me none about VCR mode.
#undef time
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Don't judge me.
static CLeaderboard* g_pLeaderboard = NULL;

using namespace vgui;

CLeaderboard* Leaderboard()
{
	return g_pLeaderboard;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLeaderboard::CLeaderboard() : Frame(NULL, "leaderboard")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/MenuScheme.res", "MenuScheme"));

	SetParent(enginevgui->GetPanel( PANEL_GAMEUIDLL ));

	SetProportional(true);

	LoadControlSettings( "Resource/UI/leaderboard.res" );
	InvalidateLayout();

	SetSizeable(false);

	SetTitle("#DA_Leaderboard_Title", true);

	m_pLeaderboard = new HTML(this, "LeaderboardHTML");

	m_pMore = new Button(this, "MoreButton", "#DA_View_Leaderboards");

	MakeReadyForUse();

	Update();

	g_pLeaderboard = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLeaderboard::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/leaderboard.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLeaderboard::~CLeaderboard()
{
}

void CLeaderboard::Update( void )
{
	m_pLeaderboard->SetVisible( true );

	m_pLeaderboard->OpenURL( "http://data.doubleactiongame.com/leaderboard/?ingame", NULL );

	m_pMore->SetVisible(true);
}

void CLeaderboard::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
		m_pLeaderboard->OpenURL( "http://data.doubleactiongame.com/leaderboard/?ingame", NULL );
	}
	else
		SetVisible( false );
}

void CLeaderboard::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

void CLeaderboard::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}
	else if (!Q_strcmp(command, "leaderboards"))
	{
		if ( steamapicontext && steamapicontext->SteamFriends() )
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( "http://data.doubleactiongame.com/leaderboard" );
		return;
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(gui_reload_leaderboard, "Reload resource for leaderboard frame.")
{
	CLeaderboard *pFrame = g_pLeaderboard;
	if (!pFrame)
		return;

	pFrame->LoadControlSettings( "Resource/UI/leaderboard.res" );
	pFrame->InvalidateLayout();
	pFrame->Update();
}

CON_COMMAND(da_leaderboard, "Show leaderboard frame.")
{
	CLeaderboard *pFrame = g_pLeaderboard;
	if (!pFrame)
		return;

	pFrame->LoadControlSettings( "Resource/UI/leaderboard.res" );
	pFrame->InvalidateLayout();
	pFrame->Activate();
	pFrame->Update();
}

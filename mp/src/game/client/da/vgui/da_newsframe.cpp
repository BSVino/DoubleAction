//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"

#include "da_newsframe.h"

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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Don't judge me.
static CNewsFrame* g_pNewsFrame = NULL;

using namespace vgui;

CNewsFrame* NewsFrame()
{
	return g_pNewsFrame;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CNewsFrame::CNewsFrame() : Frame(NULL, "news")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/MenuScheme.res", "MenuScheme"));

	SetParent(enginevgui->GetPanel( PANEL_GAMEUIDLL ));

	SetProportional(true);

	LoadControlSettings( "Resource/UI/news.res" );
	InvalidateLayout();

	SetSizeable(false);

	SetTitle("#DA_News_Title", true);

	m_pNewsMessage = new HTML(this, "NewsMessage");

	m_pWebsiteButton = new Button(this, "WebsiteButton", "#DA_Visit_Website");
	m_pForumsButton = new Button(this, "ForumsButton", "#DA_Visit_Forums");

	MakeReadyForUse();

	Update();

	g_pNewsFrame = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNewsFrame::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/news.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CNewsFrame::~CNewsFrame()
{
}

void CNewsFrame::Update( void )
{
	m_pNewsMessage->SetVisible( true );

	m_pNewsMessage->OpenURL( "http://forums.doubleactiongame.com/news.php?version=" DA_VERSION, NULL );

	m_pWebsiteButton->SetVisible(true);
	m_pForumsButton->SetVisible(true);
}

void CNewsFrame::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
		Activate();
	else
		SetVisible( false );
}

void CNewsFrame::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

void CNewsFrame::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}
	else if (!Q_strcmp(command, "website"))
	{
		if ( steamapicontext && steamapicontext->SteamFriends() )
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( "http://doubleactiongame.com" );
		return;
	}
	else if (!Q_strcmp(command, "forums"))
	{
		if ( steamapicontext && steamapicontext->SteamFriends() )
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( "http://forums.doubleactiongame.com" );
		return;
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(gui_reload_news, "Reload resource for news frame.")
{
	CNewsFrame *pNews = NewsFrame();
	if (!pNews)
		return;

	pNews->LoadControlSettings( "Resource/UI/news.res" );
	pNews->InvalidateLayout();
	pNews->Update();
}

CON_COMMAND(da_news, "Show news frame.")
{
	CNewsFrame *pNews = NewsFrame();
	if (!pNews)
		return;

	pNews->LoadControlSettings( "Resource/UI/news.res" );
	pNews->InvalidateLayout();
	pNews->Activate();
	pNews->Update();
}

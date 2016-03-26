//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"

#include "da_menubuttonpanel.h"

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
#include "da_leaderboard.h"
#include "da_newsframe.h"
#include "da_options.h"

// Don't care me none about VCR mode.
#undef time
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CButtonPanel::CButtonPanel() : EditablePanel(NULL, "button_panel")
{
	SetParent(enginevgui->GetPanel( PANEL_GAMEUIDLL ));

	SetProportional(true);

	LoadControlSettings( "Resource/UI/buttonpanel.res" );
	InvalidateLayout();

	m_pNews = new ImageButton(this, "NewsButton", "news", NULL, NULL, "opennews");
	m_pLeaderboard = new ImageButton(this, "LeaderboardButton", "leaderboard", NULL, NULL, "openleaderboard");
	m_pOptions = new ImageButton(this, "OptionsButton", "options", NULL, NULL, "openoptions");

	MakeReadyForUse();

	Update();

	KeyValues *manifest = new KeyValues( "latest_news" );
	if ( manifest->LoadFromFile( filesystem, "latest_news.txt" ) )
		m_iLatestNews = atol(manifest->GetFirstValue()->GetString());
	else
		m_iLatestNews = 0;

	// Start us out with the leaderboard.
	CloseAll();
	Leaderboard()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CButtonPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/buttonpanel.res");

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CButtonPanel::~CButtonPanel()
{
}

extern bool DAMostRecentNewsReady(int& most_recent_news, int& most_recent_version);

void CButtonPanel::OnThink()
{
	BaseClass::OnThink();

	int most_recent_news, most_recent_version;
	if (DAMostRecentNewsReady(most_recent_news, most_recent_version))
	{
		if (most_recent_news > m_iLatestNews && FStrEq(m_pNews->GetNormalImage(), "news"))
			m_pNews->SetNormalImage("news_new");

		// If there's a new version force it to open so players see the message.
		if (!NewsFrame()->IsVisible() && most_recent_version > atoi(DA_VERSION))
			OnCommand("opennews");
	}
}

void CButtonPanel::Update( void )
{
	m_pNews->SetVisible(true);
}

void CButtonPanel::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	SetVisible( bShow );
}

void CButtonPanel::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "opennews"))
	{
		CloseAll();

		m_pNews->SetNormalImage("news");

		m_iLatestNews = time(NULL);

		KeyValues *manifest = new KeyValues( "latest_news" );
		manifest->SetInt("time", (long)m_iLatestNews);
		manifest->SaveToFile( filesystem, "latest_news.txt", "MOD" );

		NewsFrame()->ShowPanel(true);
		return;
	}

	if (!Q_strcmp(command, "openleaderboard"))
	{
		CloseAll();

		Leaderboard()->ShowPanel(true);
		return;
	}

	if (!Q_strcmp(command, "openoptions"))
	{
		CloseAll();

		Options()->ShowPanel(true);
		return;
	}

	BaseClass::OnCommand(command);
}

void CButtonPanel::CloseAll()
{
	NewsFrame()->SetVisible(false);
	Leaderboard()->SetVisible(false);
}

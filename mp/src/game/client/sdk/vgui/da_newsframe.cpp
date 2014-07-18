//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "da_newsframe.h"
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
static CNewsFrame* g_pNewsFrame = NULL;

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

	MakeReadyForUse();

	Update();

	KeyValues *manifest = new KeyValues( "latest_news" );
	if ( manifest->LoadFromFile( filesystem, "latest_news.txt" ) )
		m_iLatestNews = atol(manifest->GetFirstValue()->GetString());
	else
		m_iLatestNews = 0;
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
		if (!g_pNewsFrame->IsVisible() && most_recent_version > atoi(DA_VERSION))
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
		m_pNews->SetNormalImage("news");

		m_iLatestNews = time(NULL);

		KeyValues *manifest = new KeyValues( "latest_news" );
		manifest->SetInt("time", (long)m_iLatestNews);
		manifest->SaveToFile( filesystem, "latest_news.txt", "MOD" );

		g_pNewsFrame->SetVisible(true);
		return;
	}

	BaseClass::OnCommand(command);
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CNewsFrame::CNewsFrame() : Frame(NULL, "news")
{
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

#ifdef _WIN32
	m_pWebsiteButton->SetVisible(true);
	m_pForumsButton->SetVisible(true);
#else
	m_pWebsiteButton->SetVisible(false);
	m_pForumsButton->SetVisible(false);
#endif
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

#ifdef _WIN32
//#include <Windows.h>

extern "C" {
#define FAKE_DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
FAKE_DECLARE_HANDLE(HINSTANCE);
FAKE_DECLARE_HANDLE(HWND);

// This phenomenally ugly hack brought to you by yours truly so that I wouldn't have to include windows.h
// I'm not worried about anything going horribly wrong since any problems will just throw a linker error.
extern __declspec(dllimport) HINSTANCE __stdcall ShellExecuteA(
  _In_opt_  HWND hwnd,
  _In_opt_  const char* lpOperation,
  _In_      const char* lpFile,
  _In_opt_  const char* lpParameters,
  _In_opt_  const char* lpDirectory,
  _In_      int nShowCmd
);
}

void OpenWebsite (char * cpURL)
{
	ShellExecuteA (NULL, "open", cpURL, NULL, NULL, 1/*SW_SHOWNORMAL*/);
}
#else
void OpenWebsite (char * cpURL)
{
}
#endif

void CNewsFrame::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}
	else if (!Q_strcmp(command, "website"))
	{
		OpenWebsite("http://doubleactiongame.com");
		return;
	}
	else if (!Q_strcmp(command, "forums"))
	{
		OpenWebsite("http://forums.doubleactiongame.com");
		return;
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(gui_reload_news, "Reload resource for news frame.")
{
	CNewsFrame *pNews = g_pNewsFrame;
	if (!pNews)
		return;

	pNews->LoadControlSettings( "Resource/UI/news.res" );
	pNews->InvalidateLayout();
	pNews->Update();
}

CON_COMMAND(da_news, "Show news frame.")
{
	CNewsFrame *pNews = g_pNewsFrame;
	if (!pNews)
		return;

	pNews->LoadControlSettings( "Resource/UI/news.res" );
	pNews->InvalidateLayout();
	pNews->Activate();
	pNews->Update();
}

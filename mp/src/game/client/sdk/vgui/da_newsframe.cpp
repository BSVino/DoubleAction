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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
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
	Activate();

	Update();
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

static ConVar da_first_launch("da_first_launch", "1", FCVAR_ARCHIVE);

void CNewsFrame::Update( void )
{
	m_pNewsMessage->SetVisible( true );

	if (da_first_launch.GetBool())
	{
		m_pNewsMessage->OpenURL( "http://forums.doubleactiongame.com/news.php?firstlaunch&version=" DA_VERSION, NULL );
		da_first_launch.SetValue(false);
	}
	else
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
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( "news" );
	CNewsFrame *pNews = dynamic_cast<CNewsFrame*>(pPanel);
	if (!pNews)
		return;

	pNews->LoadControlSettings( "Resource/UI/news.res" );
	pNews->InvalidateLayout();
	pNews->Update();
}

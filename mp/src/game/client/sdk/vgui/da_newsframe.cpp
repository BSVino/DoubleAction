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

void CNewsFrame::Update( void )
{
	m_pNewsMessage->SetVisible( true );
	m_pNewsMessage->OpenURL( "http://forums.doubleactiongame.com/news.php?version=" DA_VERSION, NULL );
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

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "da_credits.h"
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
static CDACredits* g_pCredits = NULL;

using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDACredits::CDACredits() : Frame(NULL, "credits")
{
	SetParent(enginevgui->GetPanel( PANEL_GAMEUIDLL ));

	SetProportional(true);

	LoadControlSettings( "Resource/UI/credits.res" );
	InvalidateLayout();

	SetSizeable(false);

	SetTitle("#DA_Credits_Title", true);

	m_pCredits = new HTML(this, "CreditsHTML");

	MakeReadyForUse();

	g_pCredits = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDACredits::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/credits.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDACredits::~CDACredits()
{
}

void CDACredits::Update( void )
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);

	m_pCredits->SetVisible( true );

	char szLocalURL[ _MAX_PATH + 7 ];
	Q_strncpy( szLocalURL, "file://", sizeof( szLocalURL ) );
		
	char szPathData[ _MAX_PATH ];
	g_pFullFileSystem->GetLocalPath( "credits.html", szPathData, sizeof(szPathData) );
	Q_strncat( szLocalURL, szPathData, sizeof( szLocalURL ), COPY_ALL_CHARACTERS );

	m_pCredits->OpenURL( szLocalURL, NULL );
}

void CDACredits::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
		Activate();
	else
		SetVisible( false );
}

void CDACredits::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

void CDACredits::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(gui_reload_credits, "Reload resource for credits frame.")
{
	CDACredits *pCredits = g_pCredits;
	if (!pCredits)
		return;

	pCredits->LoadControlSettings( "Resource/UI/credits.res" );
	pCredits->InvalidateLayout();
	pCredits->Update();
}

CON_COMMAND(da_credits, "Show credits frame.")
{
	CDACredits *pCredits = g_pCredits;
	if (!pCredits)
		return;

	pCredits->LoadControlSettings( "Resource/UI/credits.res" );
	pCredits->InvalidateLayout();
	pCredits->Activate();
	pCredits->Update();
}

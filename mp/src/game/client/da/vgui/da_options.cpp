//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"

#include "da_options.h"

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
static COptions* g_pOptions = NULL;

using namespace vgui;

COptions* Options()
{
	return g_pOptions;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COptions::COptions() : Frame(NULL, "options")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/MenuScheme.res", "MenuScheme"));

	SetParent(enginevgui->GetPanel( PANEL_GAMEUIDLL ));

	SetProportional(true);

	LoadControlSettings( "Resource/UI/options.res" );
	InvalidateLayout();

	SetSizeable(false);

	SetTitle("#GameUI_GameMenu_Options", true);

	MakeReadyForUse();

	Update();

	g_pOptions = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptions::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/options.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COptions::~COptions()
{
}

void COptions::Update(void)
{
}

void COptions::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
	}
	else
		SetVisible( false );
}

void COptions::OnKeyCodePressed(KeyCode code)
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

void COptions::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(gui_reload_options, "Reload resource for options frame.")
{
	COptions *pFrame = g_pOptions;
	if (!pFrame)
		return;

	pFrame->LoadControlSettings( "Resource/UI/options.res" );
	pFrame->InvalidateLayout();
	pFrame->Update();
}

CON_COMMAND(da_options, "Show options frame.")
{
	COptions *pFrame = g_pOptions;
	if (!pFrame)
		return;

	pFrame->LoadControlSettings( "Resource/UI/options.res" );
	pFrame->InvalidateLayout();
	pFrame->Activate();
	pFrame->Update();
}

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "sdk_mapinfo.h"
#include <cdll_client_int.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

#include <filesystem.h>
#include <convar.h>

#include "folder_gui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKMapInfo::CSDKMapInfo(IViewPort *pViewPort) : Frame(NULL, PANEL_INTRO)
{
	// load the new scheme early!!
	SetScheme(scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible( false );

	LoadControlSettings( "Resource/UI/MapInfo.res" );
	InvalidateLayout();

	m_pMapMessage = new HTML(this, "MapMessage");;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKMapInfo::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/MapInfo.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSDKMapInfo::~CSDKMapInfo()
{
}

//-----------------------------------------------------------------------------
// Purpose: Center the dialog on the screen.  (vgui has this method on
//			Frame, but we're an EditablePanel, need to roll our own.)
//-----------------------------------------------------------------------------
void CSDKMapInfo::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CSDKMapInfo::Update( void )
{
	m_pMapMessage->SetVisible( false );

	if (engine->GetLevelName() && *engine->GetLevelName())
	{
		char szMapName[MAX_MAP_NAME];
		Q_FileBase( engine->GetLevelName(), szMapName, sizeof(szMapName) );
		Q_strlower( szMapName );

		char szLocalURL[ _MAX_PATH + 7 ];
		Q_strncpy( szLocalURL, "file://", sizeof( szLocalURL ) );
		
		char szPathData[ _MAX_PATH ];
		g_pFullFileSystem->RelativePathToFullPath( VarArgs("maps/%s.htm", szMapName), "MOD", szPathData, sizeof(szPathData) );

		if (g_pFullFileSystem->IsFileImmediatelyAvailable(szPathData))
		{
			Q_strncat( szLocalURL, szPathData, sizeof( szLocalURL ), COPY_ALL_CHARACTERS );

			m_pMapMessage->SetVisible( true );
			m_pMapMessage->OpenURL( szLocalURL, NULL );

			CFolderLabel* pMapName = dynamic_cast<CFolderLabel *>(FindChildByName("MapName"));
			pMapName->SetText(szMapName);
		}
		else
		{
			// No cigar? Skip automatically.
			OnCommand("okay");
		}
	}
	else
	{
		// No cigar? Skip automatically.
		OnCommand("okay");
	}

	MoveToCenterOfScreen();
}

void CSDKMapInfo::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
		Activate();
	else
		SetVisible( false );
}

void CSDKMapInfo::PaintBackground()
{
	// Don't
}

void CSDKMapInfo::PaintBorder()
{
	// Don't
}

Panel *CSDKMapInfo::CreateControlByName( const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( this, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( this, NULL );

	return BaseClass::CreateControlByName(controlName);
}

void CSDKMapInfo::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

void CSDKMapInfo::OnCommand( const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		engine->ClientCmd( "joingame" );
		SetVisible(false);
	}

	BaseClass::OnCommand(command);
}

CON_COMMAND(hud_reload_mapinfo, "Reload resource for map info menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( "intro" );
	CSDKMapInfo *pInfo = dynamic_cast<CSDKMapInfo*>(pPanel);
	if (!pInfo)
		return;

	pInfo->LoadControlSettings( "Resource/UI/MapInfo.res" );
	pInfo->InvalidateLayout();
	pInfo->Update();
}

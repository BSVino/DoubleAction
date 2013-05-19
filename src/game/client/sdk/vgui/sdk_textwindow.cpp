//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "sdk_textwindow.h"
#include <cdll_client_int.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>

#include <FileSystem.h>
#include <convar.h>

#include "sdk_backgroundpanel.h"
#include "folder_gui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKTextWindow::CSDKTextWindow(IViewPort *pViewPort) : CTextWindow( pViewPort )
{
	// load the new scheme early!!
	SetScheme("FolderScheme");

	m_pMapMessage = new HTML(this,"MapMessage");;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKTextWindow::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings("Resource/UI/TextWindow.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSDKTextWindow::~CSDKTextWindow()
{
}

//-----------------------------------------------------------------------------
// Purpose: Center the dialog on the screen.  (vgui has this method on
//			Frame, but we're an EditablePanel, need to roll our own.)
//-----------------------------------------------------------------------------
void CSDKTextWindow::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CSDKTextWindow::Update( void )
{
	BaseClass::Update();

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
			m_pMapMessage->OpenURL( szLocalURL );
		}
	}

	MoveToCenterOfScreen();
}

void CSDKTextWindow::PaintBackground()
{
	// Don't
}

void CSDKTextWindow::PaintBorder()
{
	// Don't
}

Panel *CSDKTextWindow::CreateControlByName( const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( this, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( this, NULL );

	return BaseClass::CreateControlByName(controlName);
}

void CSDKTextWindow::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed( code );
}

CON_COMMAND(hud_reload_motd, "Reload resource for motd menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( "info" );
	CSDKTextWindow *pBuy = dynamic_cast<CSDKTextWindow*>(pPanel);
	if (!pBuy)
		return;

	pBuy->LoadControlSettings( "Resource/UI/TextWindow.res" );
	pBuy->InvalidateLayout();
	pBuy->Update();
}

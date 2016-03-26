//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <filesystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui/IVGui.h>

#include <vgui_controls/Panel.h>

#include "cdll_util.h"

#include <game/client/iviewport.h>

#include "basemodelpanel.h"

#include "da_gamerules.h"
#include "c_da_player.h"
#include "c_da_team.h"

#include "da_charactermenu.h"
#include "da_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

#undef min
#undef max

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CCharacterButton::CCharacterButton(vgui::Panel *parent, const char *panelName)
	: Button( parent, panelName, "CharacterButton")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	InvalidateLayout(true, true);
}

void CCharacterButton::ApplySettings( KeyValues *resourceData )
{
	BaseClass::ApplySettings( resourceData );

	strcpy(m_szCharacter, resourceData->GetString("character"));

	const char* pszSequence = resourceData->GetString("sequence", "");
	strcpy(m_szSequence, pszSequence);

	const char* pszWeaponModel = resourceData->GetString("weaponmodel", "");
	strcpy(m_szWeaponModel, pszWeaponModel);

	m_flBodyYaw = resourceData->GetFloat("body_yaw");
	m_flBodyPitch = resourceData->GetFloat("body_pitch");
}

void CCharacterButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	CDACharacterMenu* pParent = dynamic_cast<CDACharacterMenu*>(GetParent());
	if (!pParent)
		return;

	pParent->SetCharacterPreview(m_szCharacter, m_szSequence, m_szWeaponModel, m_flBodyYaw, m_flBodyPitch);
}

void CCharacterButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	CDACharacterMenu* pParent = dynamic_cast<CDACharacterMenu*>(GetParent());
	if (!pParent)
		return;

	pParent->SetCharacterPreview(NULL, NULL, NULL, 0, 0);
}

CDACharacterMenu::CDACharacterMenu(Panel *parent) : CFolderMenuPanel( parent, PANEL_CLASS )
{
	m_pCharacterImage = new CModelPanel(this, "CharacterImage");

	m_iCharacterMenuKey = BUTTON_CODE_INVALID;

	m_pszControlSettingsFile = "Resource/UI/CharacterMenu.res";

	SetVisible(true);

	LoadControlSettings( m_pszControlSettingsFile );
	InvalidateLayout();
	Update();
}

void CDACharacterMenu::Reset()
{
}

void CDACharacterMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iCharacterMenuKey = gameuifuncs->GetButtonCodeForBind( "character" );

	m_pCharacterImage->SwapModel("");

	if ( bShow )
	{
		//Activate();
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
}

void CDACharacterMenu::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
	{
		if (C_DAPlayer::GetLocalDAPlayer())
			C_DAPlayer::GetLocalDAPlayer()->CharacterChosen();

		engine->ServerCmd("character random");
		GetFolderMenu()->ShowPage(PANEL_BUY);
	}
	else if ( m_iCharacterMenuKey != BUTTON_CODE_INVALID && m_iCharacterMenuKey == code )
	{
		ShowPanel( false );
	}
}

void CDACharacterMenu::Update()
{
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();

	if (!pPlayer)
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDACharacterMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "CharacterButton", controlName ) )
		return new CCharacterButton( this, NULL );
	else if ( !Q_stricmp( "CIconPanel", controlName ) )
		return new CIconPanel(this, "icon_panel");
	else
	{
		Panel* pPanel = CFolderMenu::CreateControlByNameStatic(this, controlName);

		if (pPanel)
			return pPanel;

		return BaseClass::CreateControlByName(controlName);
	}
}

void CDACharacterMenu::OnCommand( const char *command )
{
	if ( Q_strncmp( command, "character ", 10 ) == 0 )
	{
		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		engine->ServerCmd( command );

		if (C_DAPlayer::GetLocalDAPlayer())
			C_DAPlayer::GetLocalDAPlayer()->CharacterChosen();

		GetFolderMenu()->ShowPage( PANEL_BUY );
	}
	else
		BaseClass::OnCommand(command);
}

void CDACharacterMenu::SetCharacterPreview(const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch)
{
	GetFolderMenu()->SetCharacterPreview(pszPreview, pszSequence, pszWeaponModel, flYaw, flPitch);
}

void CDACharacterMenu::PaintBackground()
{
	// Don't
}

void CDACharacterMenu::PaintBorder()
{
	// Don't
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDACharacterMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

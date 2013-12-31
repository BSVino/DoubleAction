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

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "da_charactermenu.h"
#include "da_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

#undef min
#undef max

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_charactermenuopen( "_cl_charactermenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when character selection menu is open" );

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

	vgui::Label* pInfoLabel = pParent->GetCharacterInfo();
	if (pInfoLabel)
	{
		if (m_szCharacter[0])
			pInfoLabel->SetText((std::string("#characterinfo_") + m_szCharacter).c_str());
		else
			pInfoLabel->SetText(m_szCharacter);
	}
}

CDACharacterMenu::CDACharacterMenu(Panel *parent) : CFolderMenuPanel( parent, PANEL_CLASS )
{
	m_pszCharacterModel = "";

	m_pCharacterInfo = new CFolderLabel(this, "CharacterInfo");
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

	m_pCharacterInfo->SetText("");
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
		engine->ClientCmd("character random");
		OnCommand("close");
	}
	else if ( m_iCharacterMenuKey != BUTTON_CODE_INVALID && m_iCharacterMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

static ConVar hud_characterpreview_x("hud_characterpreview_x", "220", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_characterpreview_y("hud_characterpreview_y", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_characterpreview_z("hud_characterpreview_z", "-40", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CDACharacterMenu::Update()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	const char szPlayerPreviewTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/player/frank.mdl\"\n"
		"		\"origin_z\"	\"-35\"\n"
		"		\"origin_y\"	\"0\"\n"
		"		\"origin_x\"	\"130\"\n"
		"		\"angles_y\"	\"180\"\n"

		"		\"animation\"\n"
		"		{\n"
		"			\"sequence\"		\"m1911_idle\"\n"
		"			\"pose_parameters\"\n"
		"			{\n"
		"				\"body_yaw\" \"25.0\"\n"
		"				\"body_pitch\" \"-30.0\"\n"
		"			}\n"
		"		}\n"
			
		"		\"attached_model\"\n"
		"		{\n"
		"			\"modelname\" \"models/weapons/m1911.mdl\"\n"
		"		}\n"
		"	}";

	if (strcmp(m_pszCharacterModel, "random") == 0)
		m_pCharacterImage->SwapModel("");
	else if (m_pszCharacterModel[0])
	{
		KeyValues* pValues = new KeyValues("preview");
		pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

		std::string sCharacter = std::string("models/player/") + m_pszCharacterModel + ".mdl";
		pValues->SetString("modelname", sCharacter.c_str());

		pValues->SetFloat("origin_x", hud_characterpreview_x.GetFloat());
		pValues->SetFloat("origin_y", hud_characterpreview_y.GetFloat());
		pValues->SetFloat("origin_z", hud_characterpreview_z.GetFloat());

		KeyValues* pAnimation = pValues->FindKey("animation");
		if (pAnimation)
		{
			pAnimation->SetString("sequence", m_pszCharacterSequence);

			KeyValues* pPoseParameters = pAnimation->FindKey("pose_parameters");
			if (pPoseParameters)
			{
				pPoseParameters->SetFloat("body_pitch", m_flBodyPitch);
				pPoseParameters->SetFloat("body_yaw", m_flBodyYaw);
			}
		}

		KeyValues* pWeapon = pValues->FindKey("attached_model");
		if (pWeapon)
			pWeapon->SetString("modelname", m_pszCharacterWeaponModel);

		m_pCharacterImage->ParseModelInfo(pValues);

		pValues->deleteThis();
	}
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

void CDACharacterMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ServerCmd( "charmenuopen" );			// to the server
		engine->ClientCmd( "_cl_charactermenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "charmenuclosed" );	
		engine->ClientCmd( "_cl_charactermenuopen 0" );
	}
}

void CDACharacterMenu::OnCommand( const char *command )
{
	if ( Q_strncmp( command, "character ", 10 ) == 0 )
	{
		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		engine->ServerCmd( command );

		GetFolderMenu()->ShowPage( PANEL_BUY );
	}
	else
		BaseClass::OnCommand(command);
}

void CDACharacterMenu::SetCharacterPreview(const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch)
{
	m_pszCharacterModel = pszPreview;
	m_pszCharacterSequence = pszSequence;
	m_pszCharacterWeaponModel = pszWeaponModel;
	m_flBodyPitch = flPitch;
	m_flBodyYaw = flYaw;

	Update();
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

Label* CDACharacterMenu::GetCharacterInfo()
{
	return m_pCharacterInfo;
}

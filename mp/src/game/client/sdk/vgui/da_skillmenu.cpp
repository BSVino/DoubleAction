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

#include "da_skillmenu.h"


#include "IGameUIFuncs.h" // for key bindings

#undef min
#undef max

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CSkillButton::CSkillButton(vgui::Panel *parent, const char *panelName )
	: Button( parent, panelName, "WeaponButton")
{
	m_szSkillName[0] = '\0';

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	InvalidateLayout(true, true);
}

void CSkillButton::ApplySettings( KeyValues *resourceData )
{
	BaseClass::ApplySettings( resourceData );

	strcpy(m_szSkillName, resourceData->GetString("skill"));
}

void CSkillButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	InvalidateLayout();

	CDASkillMenu* pParent = dynamic_cast<CDASkillMenu*>(GetParent());
	if (!pParent)
		return;

	vgui::Label* pInfoLabel = pParent->GetSkillInfo();
	if (pInfoLabel && m_szSkillName[0])
		pInfoLabel->SetText((std::string("#skillinfo_") + m_szSkillName).c_str());
	else
		pInfoLabel->SetText("");

	CPanelTexture* pInfoTexture = pParent->GetSkillIcon();
	if (pInfoTexture)
		pInfoTexture->SetImage(m_szSkillName);
}

void CSkillButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	InvalidateLayout();
}

void CSkillButton::Paint()
{
	surface()->DrawSetColor(255, 255, 255, 100);
	surface()->DrawFilledRect(0, 0, GetWide(), GetTall());

	BaseClass::Paint();
}

SkillID CSkillButton::GetSkill()
{
	return AliasToSkillID(m_szSkillName);
}

CDASkillMenu::CDASkillMenu(Panel *parent) : CFolderMenuPanel( parent, PANEL_BUY_EQUIP_CT )
{
	m_pSkillInfo = new CFolderLabel(this, "SkillInfo");
	m_pSkillIcon = new CPanelTexture(this, "SkillIcon");

	m_iSkillMenuKey = BUTTON_CODE_INVALID;

	m_pszControlSettingsFile = "Resource/UI/SkillMenu.res";

	if (C_SDKPlayer::GetLocalSDKPlayer())
		C_SDKPlayer::GetLocalSDKPlayer()->SkillsTabSeen();

	SetVisible(true);

	LoadControlSettings( m_pszControlSettingsFile );
	InvalidateLayout();
	Update();
}

void CDASkillMenu::Update()
{
}

void CDASkillMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iSkillMenuKey = gameuifuncs->GetButtonCodeForBind( "skill" );

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

void CDASkillMenu::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
	{
		engine->ServerCmd("setskill random");
		engine->ServerCmd("respawn");
		GetFolderMenu()->Close();
	}
	else if ( m_iSkillMenuKey != BUTTON_CODE_INVALID && m_iSkillMenuKey == code )
	{
		ShowPanel( false );
	}
}

Panel *CDASkillMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "SkillButton", controlName ) )
		return new CSkillButton( this, NULL );
	else
	{
		Panel* pPanel = CFolderMenu::CreateControlByNameStatic(this, controlName);

		if (pPanel)
			return pPanel;

		return BaseClass::CreateControlByName(controlName);
	}
}

void CDASkillMenu::OnCommand( const char *command )
{
	if (Q_strncasecmp(command, "setskill ", 9) == 0)
	{
		engine->ClientCmd( command );

		gViewPortInterface->ShowBackGround( false );
	}
	else
		BaseClass::OnCommand(command);
}

void CDASkillMenu::PaintBackground()
{
	// Don't
}

void CDASkillMenu::PaintBorder()
{
	// Don't
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDASkillMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

CFolderLabel* CDASkillMenu::GetSkillInfo()
{
	return m_pSkillInfo;
}

CPanelTexture* CDASkillMenu::GetSkillIcon()
{
	return m_pSkillIcon;
}

CON_COMMAND(hud_reload_skill, "Reload resource for skill menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_BUY_EQUIP_CT );
	CDASkillMenu *pSkill = dynamic_cast<CDASkillMenu*>(pPanel);
	if (!pSkill)
		return;
	
	pSkill->LoadControlSettings( "Resource/UI/SkillMenu.res" );
	pSkill->InvalidateLayout();
	pSkill->Update();
}

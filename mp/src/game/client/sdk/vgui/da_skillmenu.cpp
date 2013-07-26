//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#include "cbase.h"
#include <stdio.h>
#include <string>

#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui/IVGUI.h>

#include <vgui_controls/Panel.h>

#include "cdll_util.h"

#include <game/client/iviewport.h>

#include "basemodelpanel.h"

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "da_skillmenu.h"


#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_skillmenuopen( "_cl_skillmenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when skill menu is open" );

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

SkillID CSkillButton::GetSkill()
{
	return AliasToSkillID(m_szSkillName);
}

CDASkillMenu::CDASkillMenu(IViewPort* pViewPort) : CFolderMenu( PANEL_BUY_EQUIP_CT )
{
	m_pViewPort = pViewPort;

	m_iSkillMenuKey = BUTTON_CODE_INVALID;

	m_pSkillInfo = new CFolderLabel(this, "SkillInfo");
	m_pSkillIcon = new CPanelTexture(this, "SkillIcon");

	LoadControlSettings( "Resource/UI/SkillMenu.res" );
	InvalidateLayout();
}

//Destructor
CDASkillMenu::~CDASkillMenu()
{
}

void CDASkillMenu::Update()
{
	BaseClass::Update();

	CSkillButton* pCancel = dynamic_cast<CSkillButton*>(FindChildByName("skill_cancel"));
	if (pCancel)
	{
		if (C_SDKPlayer::GetLocalSDKPlayer() && C_SDKPlayer::GetLocalSDKPlayer()->m_Shared.m_iStyleSkill != SKILL_NONE)
			pCancel->SetVisible(true);
		else
			pCancel->SetVisible(false);
	}
}

void CDASkillMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iSkillMenuKey = gameuifuncs->GetButtonCodeForBind( "skill" );

	if ( bShow )
	{
		Activate();
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
		engine->ClientCmd("setskill random");
		ShowPanel(false);
	}
	else if ( m_iSkillMenuKey != BUTTON_CODE_INVALID && m_iSkillMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

Panel *CDASkillMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "SkillButton", controlName ) )
		return new CSkillButton( this, NULL );
	else
		return BaseClass::CreateControlByName( controlName );
}

void CDASkillMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ServerCmd( "skillmenuopen" );			// to the server
		engine->ClientCmd( "_cl_skillmenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "skillmenuclosed" );	
		engine->ClientCmd( "_cl_skillmenuopen 0" );
	}
}

void CDASkillMenu::OnCommand( const char *command )
{
	if (Q_strncasecmp(command, "setskill ", 9) == 0)
	{
		engine->ClientCmd( command );

		Close();

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

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
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

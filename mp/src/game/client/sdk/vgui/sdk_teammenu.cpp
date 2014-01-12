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

#include "sdk_teammenu.h"
#include "da_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

#undef min
#undef max

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#if defined ( SDK_USE_TEAMS )

ConVar _cl_teammenuopen( "_cl_teammenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when team selection menu is open" );

CTeamButton::CTeamButton(vgui::Panel *parent, const char *panelName)
	: Button( parent, panelName, "TeamButton")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	InvalidateLayout(true, true);
}

void CTeamButton::ApplySettings( KeyValues *resourceData )
{
	BaseClass::ApplySettings( resourceData );

	m_iSkin = resourceData->GetFloat("skin");
}

void CTeamButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

}

CSDKTeamMenu::CSDKTeamMenu(Panel *parent) : CFolderMenuPanel( parent, PANEL_TEAM )
{
	m_pCharacterImageRed = new CModelPanel(this, "CharacterImageRed");
	m_pCharacterImageBlue = new CModelPanel(this, "CharacterImageBlue");

	m_pszControlSettingsFile = "Resource/UI/TeamMenu.res";

	SetVisible(true);

	LoadControlSettings( m_pszControlSettingsFile );
	InvalidateLayout();
	Update();
}

//Destructor
CSDKTeamMenu::~CSDKTeamMenu()
{
}

void CSDKTeamMenu::ApplySettings( KeyValues *resourceData )
{
	BaseClass::ApplySettings( resourceData );
}

void CSDKTeamMenu::Reset()
{
}

void CSDKTeamMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iCharacterMenuKey = gameuifuncs->GetButtonCodeForBind( "character" );

	if ( bShow )
	{
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
}

void CSDKTeamMenu::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
	{
		engine->ClientCmd("jointeam 0");
		OnCommand("close");
	}
	else if ( m_iCharacterMenuKey != BUTTON_CODE_INVALID && m_iCharacterMenuKey == code )
	{
		ShowPanel( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: called to update the menu with new information
//-----------------------------------------------------------------------------
void CSDKTeamMenu::Update( void )
{
	BaseClass::Update();

	// The character was changed. Reload control settings so that animation is updated.
	LoadControlSettings( "Resource/UI/TeamMenu.res" );

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	
	if ( !pPlayer )
		return;

	Label *pStatusLabel = dynamic_cast<Label *>(FindChildByName("blueteaminfo"));
	C_Team *team = GetGlobalTeam(SDK_TEAM_BLUE);
	if (pStatusLabel && team)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DA_TeamMenu_Info" );
		if ( pchFmt && pchFmt[0] )
		{
			wchar_t szText[512]=L"";
			wchar_t szNumPlayersOnTeam[ 10 ];
			
			Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );
			_snwprintf( szNumPlayersOnTeam, ARRAYSIZE(szNumPlayersOnTeam) - 1, L"%d",  team->Get_Number_Players() );
			g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 1, szNumPlayersOnTeam );

			pStatusLabel->SetText(szText);
		}
	}

	pStatusLabel = dynamic_cast<Label *>(FindChildByName("redteaminfo"));
	team = GetGlobalTeam(SDK_TEAM_RED);
	if (pStatusLabel && team)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DA_TeamMenu_Info" );
		if ( pchFmt && pchFmt[0] )
		{
			wchar_t szText[512]=L"";
			wchar_t szNumPlayersOnTeam[ 10 ];
			
			Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );
			_snwprintf( szNumPlayersOnTeam, ARRAYSIZE(szNumPlayersOnTeam) - 1, L"%d",  team->Get_Number_Players() );
			g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 1, szNumPlayersOnTeam );

			pStatusLabel->SetText(szText);
		}
	}

	UpdateCharacter(m_pCharacterImageRed);
	UpdateCharacter(m_pCharacterImageBlue);
}

void CSDKTeamMenu::UpdateCharacter(CModelPanel* pPanel)
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	
	if ( !pPlayer )
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

	KeyValues* pValues = new KeyValues("preview");
	pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

	pValues->SetString("modelname", "models/player/wish.mdl");

	pValues->SetFloat("origin_x", 200);
	pValues->SetFloat("origin_y", 0);
	pValues->SetFloat("origin_z", -35);

	KeyValues* pAnimation = pValues->FindKey("animation");
	if (pAnimation)
	{
		pAnimation->SetString("sequence", "wish_pose");

		KeyValues* pPoseParameters = pAnimation->FindKey("pose_parameters");
		if (pPoseParameters)
		{
			pPoseParameters->SetFloat("body_pitch", 15);
			pPoseParameters->SetFloat("body_yaw", -40);
		}
	}

	KeyValues* pWeapon = pValues->FindKey("attached_model");
	if (pWeapon)
		pWeapon->SetString("modelname", "models/weapons/m1911.mdl");

	pValues->SetInt("skin", (pPanel==m_pCharacterImageRed)?2:1);

	pPanel->ParseModelInfo(pValues);

	pValues->deleteThis();
}

Panel *CSDKTeamMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "TeamButton", controlName ) )
		return new CTeamButton( this, NULL );
	else if ( !Q_stricmp( "CIconPanel", controlName ) )
		return new CIconPanel(this, "icon_panel");
	else
		return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKTeamMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	/*if ( state )
	{
		Panel *pAutoButton = FindChildByName( "autobutton" );
		if ( pAutoButton )
		{
			pAutoButton->RequestFocus();
		}
	}*/

	if ( state )
	{
		engine->ServerCmd( "teammenuopen" );			// to the server
		engine->ClientCmd( "_cl_teammenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "teammenuclosed" );	
		engine->ClientCmd( "_cl_teammenuopen 0" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: When a team button is pressed it triggers this function to 
//			cause the player to join a team
//-----------------------------------------------------------------------------
void CSDKTeamMenu::OnCommand( const char *command )
{
	if ( Q_strncmp( command, "jointeam ", 9 ) == 0 )
	{
		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		engine->ServerCmd( command );

		// Until the message goes to the server and comes back we stuff teamnum
		// so that it has the proper value and we don't get circular menus.
		// A hack, but I want to ship.
		if (C_SDKPlayer::GetLocalSDKPlayer())
			C_SDKPlayer::GetLocalSDKPlayer()->m_iTeamNum = atoi(command+9);

		GetFolderMenu()->ShowPage( PANEL_CLASS );
	}
	else
		BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CSDKTeamMenu::PaintBackground()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CSDKTeamMenu::PaintBorder()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CSDKTeamMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

#endif // SDK_USE_TEAMS

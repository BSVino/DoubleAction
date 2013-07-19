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

#include "sdk_teammenu.h"
#include "dab_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

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

	strcpy(m_szCharacter, resourceData->GetString("character"));

	const char* pszSequence = resourceData->GetString("sequence", "");
	strcpy(m_szSequence, pszSequence);

	const char* pszWeaponModel = resourceData->GetString("weaponmodel", "");
	strcpy(m_szWeaponModel, pszWeaponModel);

	m_flBodyYaw = resourceData->GetFloat("body_yaw");
	m_flBodyPitch = resourceData->GetFloat("body_pitch");
}

void CTeamButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	CSDKTeamMenu* pParent = dynamic_cast<CSDKTeamMenu*>(GetParent());
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

CSDKTeamMenu::CSDKTeamMenu(IViewPort *pViewPort) : CFolderMenu( PANEL_TEAM )
{
	m_pViewPort = pViewPort;

	m_pszCharacterModel = "";

	m_pCharacterInfo = new CFolderLabel(this, "CharacterInfo");
	m_pCharacterImage = new CModelPanel(this, "CharacterImage");

	m_iCharacterMenuKey = BUTTON_CODE_INVALID;

	LoadControlSettings( "Resource/UI/TeamMenu.res" );
	InvalidateLayout();
}

//Destructor
CSDKTeamMenu::~CSDKTeamMenu()
{
}

void CSDKTeamMenu::Reset()
{
}

void CSDKTeamMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iCharacterMenuKey = gameuifuncs->GetButtonCodeForBind( "character" );

	m_pCharacterInfo->SetText("");
	m_pCharacterImage->SwapModel("");

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
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

//-----------------------------------------------------------------------------
// Purpose: called to update the menu with new information
//-----------------------------------------------------------------------------
void CSDKTeamMenu::Update( void )
{
	//BaseClass::Update();

	//const ConVar *allowspecs =  cvar->FindVar( "mp_allowspectators" );

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	
	if ( !pPlayer )//|| !SDKGameRules() )
		return;

	/*if ( allowspecs && allowspecs->GetBool() )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_UNASSIGNED || ( pPlayer && pPlayer->IsPlayerDead() ) )
		{
			SetVisibleButton("specbutton", true);
		}
		else
		{
			SetVisibleButton("specbutton", false);
		}
	}
	else
	{
		SetVisibleButton("specbutton", false );
	}

	if( pPlayer->GetTeamNumber() == TEAM_UNASSIGNED ) // we aren't on a team yet
	{
		SetVisibleButton("CancelButton", false); 
	}
	else
	{
		SetVisibleButton("CancelButton", true); 
	}

	MoveToCenterOfScreen();*/

	Label *pStatusLabel = dynamic_cast<Label *>(FindChildByName("frankteaminfo"));
	C_Team *team = GetGlobalTeam(SDK_TEAM_BLUE);
	if (pStatusLabel && team)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DAB_TeamMenu_Info" );
		if ( pchFmt && pchFmt[0] )
		{
			wchar_t szText[512]=L"";
			wchar_t szNumPlayersOnTeam[ 10 ];
			wchar_t szTeamScore[ 10 ];
			
			Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );
			_snwprintf( szNumPlayersOnTeam, ARRAYSIZE(szNumPlayersOnTeam) - 1, L"%d",  team->Get_Number_Players() );
			_snwprintf( szTeamScore, ARRAYSIZE(szTeamScore) - 1, L"%d",  team->Get_Score() );
			g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 2, szNumPlayersOnTeam, szTeamScore );

			pStatusLabel->SetText(szText);
		}
	}

	pStatusLabel = dynamic_cast<Label *>(FindChildByName("wishteaminfo"));
	team = GetGlobalTeam(SDK_TEAM_RED);
	if (pStatusLabel && team)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DAB_TeamMenu_Info" );
		if ( pchFmt && pchFmt[0] )
		{
			wchar_t szText[512]=L"";
			wchar_t szNumPlayersOnTeam[ 10 ];
			wchar_t szTeamScore[ 10 ];
			
			Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );
			_snwprintf( szNumPlayersOnTeam, ARRAYSIZE(szNumPlayersOnTeam) - 1, L"%d",  team->Get_Number_Players() );
			_snwprintf( szTeamScore, ARRAYSIZE(szTeamScore) - 1, L"%d",  team->Get_Score() );
			g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 2, szNumPlayersOnTeam, szTeamScore );

			pStatusLabel->SetText(szText);
		}
	}

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

		ConVarRef hud_characterpreview_x( "hud_characterpreview_x" );
		ConVarRef hud_characterpreview_y( "hud_characterpreview_y" );
		ConVarRef hud_characterpreview_z( "hud_characterpreview_z" );

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

	BaseClass::Update();
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
		Close();

		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		//engine->ClientCmd( command );
	}
	else
		BaseClass::OnCommand(command);

	/*if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( command );
	}
	
	
	BaseClass::OnCommand(command);

	gViewPortInterface->ShowBackGround( false );
	OnClose();*/
}

//-----------------------------------------------------------------------------
// Purpose: Sets the visibility of a button by name
//-----------------------------------------------------------------------------
/*void CSDKTeamMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetVisible(state);
	}
}*/

void CSDKTeamMenu::SetCharacterPreview(const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch)
{
	m_pszCharacterModel = pszPreview;
	m_pszCharacterSequence = pszSequence;
	m_pszCharacterWeaponModel = pszWeaponModel;
	m_flBodyPitch = flPitch;
	m_flBodyYaw = flYaw;

	Update();
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CSDKTeamMenu::PaintBackground()
{
	/*int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );*/
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CSDKTeamMenu::PaintBorder()
{
	/*int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );*/
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CSDKTeamMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	/*m_bgColor = GetSchemeColor("BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );*/

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

}

Label* CSDKTeamMenu::GetCharacterInfo()
{
	return m_pCharacterInfo;
}
#endif // SDK_USE_TEAMS

CON_COMMAND(hud_reload_team, "Reload resource for team menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_TEAM );
	CSDKTeamMenu *pTeam = dynamic_cast<CSDKTeamMenu*>(pPanel);
	if (!pTeam)
		return;
	
	pTeam->LoadControlSettings( "Resource/UI/TeamMenu.res" );
	pTeam->InvalidateLayout();
	pTeam->Update();
}

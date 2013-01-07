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
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui/IVGUI.h>

#include <vgui_controls/Panel.h>

#include "cdll_util.h"

#include <game/client/iviewport.h>

#include "basemodelpanel.h"

#include "sdk_backgroundpanel.h"

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "dab_charactermenu.h"
#include "dab_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_charactermenuopen( "_cl_charactermenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when character selection menu is open" );
//ConVar hud_buyautokill("hud_buyautokill", "0");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABCharacterInfoPanel::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "CIconPanel", controlName ) )
	{
		return new CIconPanel(this, "icon_panel");
	}
	else
	{
		return BaseClass::CreateControlByName( controlName );
	}
}

void CDABCharacterInfoPanel::ApplySchemeSettings( IScheme *pScheme )
{
	RichText *pCharacterInfo = dynamic_cast<RichText*>(FindChildByName("characterInfo"));

	if ( pCharacterInfo )
	{
		pCharacterInfo->SetBorder(pScheme->GetBorder("NoBorder"));
		pCharacterInfo->SetBgColor(pScheme->GetColor("Blank", Color(0,0,0,0)));
	}

	BaseClass::ApplySchemeSettings( pScheme );
}

void CCharacterButton::ShowPage()
{
	if( m_pPanel )
	{
		m_pPanel->SetVisible( true );
		m_pPanel->MoveToFront();
		g_lastPanel = m_pPanel;
	}

	if (!FStrEq(GetName(), "character_random"))
	{
		static_cast<CDABCharacterMenu*>(GetParent())->SetCharacterPreview(GetName(), m_pszSequence, m_pszWeaponModel, m_flBodyYaw, m_flBodyPitch);
		static_cast<CDABCharacterMenu*>(GetParent())->Update();
	}
}

void CCharacterButton::ApplySettings( KeyValues *resourceData ) 
{
	BaseClass::ApplySettings( resourceData );

	if (m_pszSequence)
		delete [] m_pszSequence;

	const char* pszSequence = resourceData->GetString("sequence");
	if (strlen(pszSequence))
	{
		m_pszSequence = new char[strlen(pszSequence)];
		strcpy(m_pszSequence, pszSequence);
	}

	if (m_pszWeaponModel)
		delete [] m_pszWeaponModel;

	const char* pszWeaponModel = resourceData->GetString("weaponmodel");
	if (strlen(pszWeaponModel))
	{
		m_pszWeaponModel = new char[strlen(pszWeaponModel)];
		strcpy(m_pszWeaponModel, pszWeaponModel);
	}

	m_flBodyYaw = resourceData->GetFloat("body_yaw");
	m_flBodyPitch = resourceData->GetFloat("body_pitch");

	// name, position etc of button is set, now load matching
	// resource file for associated info panel:
	m_pPanel->LoadControlSettings( GetCharacterPage( GetName() ) );
}		

CDABCharacterMenu::CDABCharacterMenu(IViewPort* pViewPort) : vgui::Frame( NULL, PANEL_CLASS )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("SourceScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	m_pszCharacterPreview = "playermale";

	// info window about this class
	m_pPanel = new EditablePanel( this, "CharacterInfo" );

	m_iCharacterMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	m_pCharacterInfoPanel = new CDABCharacterInfoPanel( this, "CharacterInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

//	m_pSuicideOption = new CheckButton( this, "suicide_option", "Sky is blue?" );

	LoadControlSettings( "Resource/UI/CharacterMenu.res" );
	InvalidateLayout();
}

//Destructor
CDABCharacterMenu::~CDABCharacterMenu()
{
}

void CDABCharacterMenu::Reset()
{
	for ( int i = 0 ; i < GetChildCount() ; ++i )
	{
		// Hide the subpanel for the CCharacterButtons
		CCharacterButton *pPanel = dynamic_cast<CCharacterButton *>( GetChild( i ) );

		if ( pPanel )
			pPanel->HidePage();
	}

	if (m_pInitialButton)
		m_pInitialButton->ShowPage();
}

void CDABCharacterMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		engine->CheckPoint( "CharacterMenu" );

		m_iCharacterMenuKey = gameuifuncs->GetButtonCodeForBind( "character" );

		//m_pSuicideOption->SetSelected( hud_buyautokill.GetBool() );
	}

	for( int i = 0; i< GetChildCount(); i++ ) 
	{
		//Tony; using mouse over button for now, later we'll use CModelButton when I get it implemented!!
		CCharacterButton *button = dynamic_cast<CCharacterButton *>(GetChild(i));

		if ( button )
		{
			if( button == m_pInitialButton && bShow == true )
				button->ShowPage();
			else
				button->HidePage();
		}
	}

	CCharacterButton *pRandom = dynamic_cast<CCharacterButton *>( FindChildByName("random") );

	if ( pRandom )
		pRandom->HidePage();

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
	
	m_pViewPort->ShowBackGround( bShow );
}

void CDABCharacterMenu::OnKeyCodePressed( KeyCode code )
{
	if ( m_iCharacterMenuKey != BUTTON_CODE_INVALID && m_iCharacterMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CDABCharacterMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CDABCharacterMenu::Update()
{
	MoveToCenterOfScreen();

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	const char szPlayerPreviewTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/player/playermale.mdl\"\n"
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

	CModelPanel *pPlayerPreview = dynamic_cast<CModelPanel *>(FindChildByName("player_preview"));

	if (pPlayerPreview)
	{
		KeyValues* pValues = new KeyValues("preview");
		pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

		KeyValues* pModel = pValues->FindKey("modelname");
		if (pModel)
			pModel->SetStringValue(VarArgs("models/player/%s.mdl", m_pszCharacterPreview));

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

		pPlayerPreview->ParseModelInfo(pValues);

		pValues->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABCharacterMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "CharacterButton", controlName ) )
	{
		CCharacterButton *newButton = new CCharacterButton( this, NULL, m_pCharacterInfoPanel );

		if( !m_pInitialButton )
		{
			m_pInitialButton = newButton;
		}

		return newButton;
	}
	else if ( !Q_stricmp( "CIconPanel", controlName ) )
	{
		return new CIconPanel(this, "icon_panel");
	}
	else
	{
		return BaseClass::CreateControlByName( controlName );
	}
}

//-----------------------------------------------------------------------------
// Catch the mouseover event and set the active class
//-----------------------------------------------------------------------------
void CDABCharacterMenu::OnShowPage( const char *pagename )
{
	m_pszCharacterPreview = "playermale";
}

/*void CDABBuyMenu::OnSuicideOptionChanged( vgui::Panel *Panel )
{
	hud_buyautokill.SetValue( m_pSuicideOption->IsSelected() );
}*/

//-----------------------------------------------------------------------------
// Do things that should be done often, eg number of players in the 
// selected class
//-----------------------------------------------------------------------------
void CDABCharacterMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == PLAYERCLASS_UNASSIGNED and if so
	//hide the cancel button

	if ( !IsVisible() )
		return;

	BaseClass::OnTick();
}

void CDABCharacterMenu::SetVisible( bool state )
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

void CDABCharacterMenu::OnCommand( const char *command )
{
	if ( Q_strncmp( command, "character", 9 ) == 0 )
	{
		Close();

		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		engine->ClientCmd( command );
	}
	else
		engine->ClientCmd( command );
}

void CDABCharacterMenu::SetCharacterPreview(const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch)
{
	m_pszCharacterPreview = pszPreview;
	m_pszCharacterSequence = pszSequence;
	m_pszCharacterWeaponModel = pszWeaponModel;
	m_flBodyPitch = flPitch;
	m_flBodyYaw = flYaw;
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CDABCharacterMenu::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CDABCharacterMenu::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDABCharacterMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
}

CON_COMMAND(hud_reload_character, "Reload resource for character menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_CLASS );
	CDABCharacterMenu *pCharacter = dynamic_cast<CDABCharacterMenu*>(pPanel);
	if (!pCharacter)
		return;
	
	pCharacter->LoadControlSettings( "Resource/UI/CharacterMenu.res" );
	pCharacter->InvalidateLayout();
	pCharacter->Update();
}

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

#include "dab_skillmenu.h"


#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_skillmenuopen( "_cl_skillmenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when skill menu is open" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABSkillInfoPanel::CreateControlByName( const char *controlName )
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

void CDABSkillInfoPanel::ApplySchemeSettings( IScheme *pScheme )
{
	RichText *pSkillInfo = dynamic_cast<RichText*>(FindChildByName("skillInfo"));

	if ( pSkillInfo )
	{
		pSkillInfo->SetBorder(pScheme->GetBorder("NoBorder"));
		pSkillInfo->SetBgColor(pScheme->GetColor("Blank", Color(0,0,0,0)));
	}

	BaseClass::ApplySchemeSettings( pScheme );
}

CDABSkillMenu::CDABSkillMenu(IViewPort* pViewPort) : vgui::Frame( NULL, PANEL_BUY_EQUIP_CT )
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

	// info window about this class
	m_pPanel = new EditablePanel( this, "SkillInfo" );

	m_iSkillMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	m_pSkillInfoPanel = new CDABSkillInfoPanel( this, "SkillInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	LoadControlSettings( "Resource/UI/SkillMenu.res" );
	InvalidateLayout();
}

//Destructor
CDABSkillMenu::~CDABSkillMenu()
{
}

void CDABSkillMenu::Reset()
{
	for ( int i = 0 ; i < GetChildCount() ; ++i )
	{
		// Hide the subpanel for the CSkillButtons
		CSkillButton *pPanel = dynamic_cast<CSkillButton *>( GetChild( i ) );

		if ( pPanel )
			pPanel->HidePage();
	}

	if (m_pInitialButton)
		m_pInitialButton->ShowPage();
}

void CDABSkillMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iSkillMenuKey = gameuifuncs->GetButtonCodeForBind( "skill" );

	for( int i = 0; i< GetChildCount(); i++ ) 
	{
		//Tony; using mouse over button for now, later we'll use CModelButton when I get it implemented!!
		CSkillButton *button = dynamic_cast<CSkillButton *>(GetChild(i));

		if ( button )
		{
			if( button == m_pInitialButton && bShow == true )
				button->ShowPage();
			else
				button->HidePage();
		}
	}

	CSkillButton *pRandom = dynamic_cast<CSkillButton *>( FindChildByName("random") );

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

void CDABSkillMenu::OnKeyCodePressed( KeyCode code )
{
	if ( m_iSkillMenuKey != BUTTON_CODE_INVALID && m_iSkillMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CDABSkillMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CDABSkillMenu::Update()
{
	Button *entry = dynamic_cast<Button *>(FindChildByName("CancelButton"));
	if (entry)
		entry->SetVisible(true);

	MoveToCenterOfScreen();

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABSkillMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "SkillButton", controlName ) )
	{
		CSkillButton *newButton = new CSkillButton( this, NULL, m_pSkillInfoPanel );

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
void CDABSkillMenu::OnShowPage( const char *pagename )
{
}

//-----------------------------------------------------------------------------
// Do things that should be done often, eg number of players in the 
// selected class
//-----------------------------------------------------------------------------
void CDABSkillMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == PLAYERCLASS_UNASSIGNED and if so
	//hide the cancel button

	if ( !IsVisible() )
		return;

	BaseClass::OnTick();
}

void CDABSkillMenu::SetVisible( bool state )
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

void CDABSkillMenu::OnCommand( const char *command )
{
	engine->ClientCmd( command );

	Close();

	gViewPortInterface->ShowBackGround( false );
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CDABSkillMenu::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CDABSkillMenu::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDABSkillMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
}

void CDABSkillMenu::SetCharacterPreview(const char* pszCharacter)
{
	for (int i = 0; ; i++)
	{
		if (pszPossiblePlayerModels[i] == nullptr)
			break;

		if (FStrEq(VarArgs("models/player/%s.mdl", pszCharacter), pszPossiblePlayerModels[i]))
		{
			m_pszCharacterPreview = pszPossiblePlayerModels[i];
			return;
		}
	}
}

CON_COMMAND(hud_reload_skill, "Reload resource for skill menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_BUY_EQUIP_CT );
	CDABSkillMenu *pSkill = dynamic_cast<CDABSkillMenu*>(pPanel);
	if (!pSkill)
		return;
	
	pSkill->LoadControlSettings( "Resource/UI/SkillMenu.res" );
	pSkill->InvalidateLayout();
	pSkill->Update();
}

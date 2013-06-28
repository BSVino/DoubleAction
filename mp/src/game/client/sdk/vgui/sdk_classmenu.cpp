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

#include "sdk_backgroundpanel.h"

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "sdk_classmenu.h"


#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#if defined ( SDK_USE_PLAYERCLASSES )

ConVar _cl_classmenuopen( "_cl_classmenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when class menu is open" );
extern ConVar hud_classautokill;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CSDKClassInfoPanel::CreateControlByName( const char *controlName )
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

void CSDKClassInfoPanel::ApplySchemeSettings( IScheme *pScheme )
{
	RichText *pClassInfo = dynamic_cast<RichText*>(FindChildByName("classInfo"));

	if ( pClassInfo )
	{
		pClassInfo->SetBorder(pScheme->GetBorder("NoBorder"));
		pClassInfo->SetBgColor(pScheme->GetColor("Blank", Color(0,0,0,0)));
	}

	BaseClass::ApplySchemeSettings( pScheme );
}
CSDKClassMenu::CSDKClassMenu(IViewPort *pViewPort) : CClassMenu( pViewPort )
{
	// load the new scheme early!!
	SetScheme("SourceScheme");

	m_mouseoverButtons.RemoveAll();
	m_iClassMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	m_pClassInfoPanel = new CSDKClassInfoPanel( this, "ClassInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_iActivePlayerClass = -1;
	m_iLastPlayerClassCount = -1;

	int i = 0;
	int j = SDK_NUM_PLAYERCLASSES;
	for (i = 0;i<j;i++)
	{
		m_pClassNumLabel[i] = new Label( this, VarArgs("class_%d_num", i+1), "" );
		m_pClassFullLabel[i] = new Label( this, VarArgs("class_%d_full", i+1), "" );
	}

	m_pSuicideOption = new CheckButton( this, "suicide_option", "Sky is blue?" );
}
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKClassMenu::CSDKClassMenu(IViewPort *pViewPort, const char *panelName) : CClassMenu(pViewPort, panelName)
{
	// load the new scheme early!!
	SetScheme("SourceScheme");

	m_mouseoverButtons.RemoveAll();
	m_iClassMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	m_pClassInfoPanel = new CSDKClassInfoPanel( this, "ClassInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_iActivePlayerClass = -1;
	m_iLastPlayerClassCount = -1;

	int i = 0;
	int j = SDK_NUM_PLAYERCLASSES;
	for (i = 0;i<j;i++)
	{
		m_pClassNumLabel[i] = new Label( this, VarArgs("class_%d_num", i+1), "" );
		m_pClassFullLabel[i] = new Label( this, VarArgs("class_%d_full", i+1), "" );
	}

	m_pSuicideOption = new CheckButton( this, "suicide_option", "Sky is blue?" );
}
//Destructor
CSDKClassMenu::~CSDKClassMenu()
{
}

void CSDKClassMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		engine->CheckPoint( "ClassMenu" );

		m_iClassMenuKey = gameuifuncs->GetButtonCodeForBind( "changeclass" );

		m_pSuicideOption->SetSelected( hud_classautokill.GetBool() );
	}

	for( int i = 0; i< GetChildCount(); i++ ) 
	{
		//Tony; using mouse over button for now, later we'll use CModelButton when I get it implemented!!
		MouseOverButton<CSDKClassInfoPanel> *button = dynamic_cast<MouseOverButton<CSDKClassInfoPanel> *>(GetChild(i));

		if ( button )
		{
			if( button == m_pInitialButton && bShow == true )
				button->ShowPage();
			else
				button->HidePage();
		}
	}

	MouseOverButton<CSDKClassInfoPanel> *pRandom =	dynamic_cast<MouseOverButton<CSDKClassInfoPanel> *>( FindChildByName("random") );

	if ( pRandom )
		pRandom->HidePage();

	// recalc position of checkbox, since it doesn't do right alignment
	m_pSuicideOption->SizeToContents();

	int x, y, wide, tall; 
	m_pSuicideOption->GetBounds( x, y, wide, tall );

	int parentW, parentH;
	GetSize( parentW, parentH );

	x = parentW / 2;	// - wide;
	m_pSuicideOption->SetPos( x, y );

	BaseClass::ShowPanel( bShow );
}

void CSDKClassMenu::OnKeyCodePressed( KeyCode code )
{
	if ( m_iClassMenuKey != BUTTON_CODE_INVALID && m_iClassMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}
void CSDKClassMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}
void CSDKClassMenu::Update()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( pPlayer && pPlayer->m_Shared.DesiredPlayerClass() == PLAYERCLASS_UNDEFINED )
	{
		SetVisibleButton( "CancelButton", false );
	}
	else
	{
		SetVisibleButton( "CancelButton", true ); 
	}

	MoveToCenterOfScreen();

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CSDKClassMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "SDKButton", controlName ) )
	{
		MouseOverButton<CSDKClassInfoPanel> *newButton = new MouseOverButton<CSDKClassInfoPanel>( this, NULL, m_pClassInfoPanel );

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
void CSDKClassMenu::OnShowPage( const char *pagename )
{
	// change which class we are counting based on class name

	// turn the button name into a classname

	char buf[64];

	Q_snprintf( buf, sizeof(buf), "cls_%s", pagename );

	C_SDKTeam *pTeam = dynamic_cast<C_SDKTeam *>( GetGlobalTeam(GetTeamNumber()) );

	if( !pTeam )
		return;

	// Pull the index of this class via IsClassOnTeam
	if ( !pTeam->IsClassOnTeam( buf, m_iActivePlayerClass ) )
	{
		Assert( !"bad class name on class button" );
	}

	UpdateNumClassLabel();
}
void CSDKClassMenu::OnSuicideOptionChanged( vgui::Panel *Panel )
{
	hud_classautokill.SetValue( m_pSuicideOption->IsSelected() );
}
//-----------------------------------------------------------------------------
// Do things that should be done often, eg number of players in the 
// selected class
//-----------------------------------------------------------------------------
void CSDKClassMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == PLAYERCLASS_UNASSIGNED and if so
	//hide the cancel button

	if ( !IsVisible() )
		return;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if( pPlayer && pPlayer->m_Shared.PlayerClass() == PLAYERCLASS_UNDEFINED )
	{
		SetVisibleButton("CancelButton", false);
	}

	UpdateNumClassLabel();

	BaseClass::OnTick();
}

void CSDKClassMenu::UpdateNumClassLabel( void )
{
	int iClassCount[SDK_NUM_PLAYERCLASSES];
	int iClassLimit[SDK_NUM_PLAYERCLASSES];

	// count how many of this class there are
	C_SDKTeam *pTeam = dynamic_cast<C_SDKTeam *>( GetGlobalTeam(GetTeamNumber()) );

	if ( !pTeam )
		return;

	char buf[16];

	for( int i=0;i<SDK_NUM_PLAYERCLASSES;i++ )
	{
		iClassCount[i] = pTeam->CountPlayersOfThisClass( i );

		if ( !m_pClassNumLabel[i] || !m_pClassFullLabel[i] )
			continue;

		if ( pTeam->IsClassOnTeam( i ) )
		{
			// FIXME - store pointers to these cvars
			const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( i );
			ConVar *pLimitCvar = ( ConVar * )cvar->FindVar( pClassInfo.m_szLimitCvar );

			if ( pLimitCvar )
				iClassLimit[i] = min( 32, pLimitCvar->GetInt() );
		}	

		if ( iClassLimit[i] < 0 || iClassCount[i] < iClassLimit[i] )
			m_pClassFullLabel[i]->SetVisible( false );
		else
			m_pClassFullLabel[i]->SetVisible( true );

		if ( iClassLimit[i] > -1 )
		{
			// draw "3 / 4"
			Q_snprintf( buf, sizeof(buf), "%d / %d", iClassCount[i], iClassLimit[i] );
		}
		else
		{
			// just "3"
			Q_snprintf( buf, sizeof(buf), "x %d", iClassCount[i] );
		}

		m_pClassNumLabel[i]->SetText( buf );
	}
}

void CSDKClassMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ServerCmd( "menuopen" );			// to the server
		engine->ClientCmd( "_cl_classmenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "menuclosed" );	
		engine->ClientCmd( "_cl_classmenuopen 0" );
	}
}
//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CSDKClassMenu::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CSDKClassMenu::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CSDKClassMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
}

#endif // SDK_USE_PLAYERCLASSES

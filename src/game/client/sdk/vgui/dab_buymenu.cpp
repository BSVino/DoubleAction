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

#include "dab_buymenu.h"


#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_buymenuopen( "_cl_buymenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when buy menu is open" );
ConVar hud_buyautokill("hud_buyautokill", "0");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABWeaponInfoPanel::CreateControlByName( const char *controlName )
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

void CDABWeaponInfoPanel::ApplySchemeSettings( IScheme *pScheme )
{
	RichText *pBuyInfo = dynamic_cast<RichText*>(FindChildByName("weaponInfo"));

	if ( pBuyInfo )
	{
		pBuyInfo->SetBorder(pScheme->GetBorder("NoBorder"));
		pBuyInfo->SetBgColor(pScheme->GetColor("Blank", Color(0,0,0,0)));
	}

	BaseClass::ApplySchemeSettings( pScheme );
}

CDABBuyMenu::CDABBuyMenu(IViewPort* pViewPort) : vgui::Frame( NULL, PANEL_BUY )
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
	m_pPanel = new EditablePanel( this, "WeaponInfo" );

	m_iBuyMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	m_pWeaponInfoPanel = new CDABWeaponInfoPanel( this, "WeaponInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pSuicideOption = new CheckButton( this, "suicide_option", "Sky is blue?" );

	LoadControlSettings( "Resource/UI/BuyMenu.res" );
	InvalidateLayout();
}

//Destructor
CDABBuyMenu::~CDABBuyMenu()
{
}

void CDABBuyMenu::Reset()
{
	for ( int i = 0 ; i < GetChildCount() ; ++i )
	{
		// Hide the subpanel for the CWeaponButtons
		CWeaponButton *pPanel = dynamic_cast<CWeaponButton *>( GetChild( i ) );

		if ( pPanel )
			pPanel->HidePage();
	}

	if (m_pInitialButton)
		m_pInitialButton->ShowPage();
}

void CDABBuyMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		engine->CheckPoint( "BuyMenu" );

		m_iBuyMenuKey = gameuifuncs->GetButtonCodeForBind( "buy" );

		m_pSuicideOption->SetSelected( hud_buyautokill.GetBool() );
	}

	for( int i = 0; i< GetChildCount(); i++ ) 
	{
		//Tony; using mouse over button for now, later we'll use CModelButton when I get it implemented!!
		CWeaponButton *button = dynamic_cast<CWeaponButton *>(GetChild(i));

		if ( button )
		{
			if( button == m_pInitialButton && bShow == true )
				button->ShowPage();
			else
				button->HidePage();
		}
	}

	CWeaponButton *pRandom = dynamic_cast<CWeaponButton *>( FindChildByName("random") );

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

void CDABBuyMenu::OnKeyCodePressed( KeyCode code )
{
	if ( m_iBuyMenuKey != BUTTON_CODE_INVALID && m_iBuyMenuKey == code )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CDABBuyMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CDABBuyMenu::Update()
{
	Button *entry = dynamic_cast<Button *>(FindChildByName("CancelButton"));
	if (entry)
		entry->SetVisible(true);

	MoveToCenterOfScreen();

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	Label *pSlotsLabel = dynamic_cast<Label *>(FindChildByName("SlotsRemaining"));
	if (pSlotsLabel)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DAB_BuyMenu_SlotsRemaining" );
		if ( pchFmt && pchFmt[0] )
		{
			wchar_t szText[512]=L"";
			wchar_t szLoadoutWeight[ 10 ];

			Q_wcsncpy( szFmt, pchFmt, sizeof( szFmt ) );
			_snwprintf( szLoadoutWeight, ARRAYSIZE(szLoadoutWeight) - 1, L"%d",  MAX_LOADOUT_WEIGHT-pPlayer->GetLoadoutWeight() );
			g_pVGuiLocalize->ConstructString( szText, sizeof( szText ), szFmt, 1, szLoadoutWeight );

			pSlotsLabel->SetText(szText);
		}
	}

	for ( int i = 0 ; i < GetChildCount() ; ++i )
	{
		// Hide the subpanel for the CWeaponButtons
		CWeaponButton *pPanel = dynamic_cast<CWeaponButton *>( GetChild( i ) );

		if (!pPanel)
			continue;

		pPanel->SetEnabled(true);

		if (!pPanel->GetName())
			continue;

		if (strlen(pPanel->GetName()) < 7)
			continue;

		SDKWeaponID eWeapon = AliasToWeaponID(pPanel->GetName() + 7);

		if (!eWeapon)
			continue;

		pPanel->SetEnabled(pPlayer->CanAddToLoadout(eWeapon));
	}

	CModelPanel* pPreviews[4];
	pPreviews[0] = dynamic_cast<CModelPanel *>(FindChildByName("loadout_preview_1"));
	pPreviews[1] = dynamic_cast<CModelPanel *>(FindChildByName("loadout_preview_2"));
	pPreviews[2] = dynamic_cast<CModelPanel *>(FindChildByName("loadout_preview_3"));
	pPreviews[3] = dynamic_cast<CModelPanel *>(FindChildByName("loadout_preview_4"));

	int iPreview = 0;

	const char szTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/weapons/mp5k.mdl\"\n"
		"		\"origin_z\"	\"0\"\n"
		"		\"origin_y\"	\"7\"\n"
		"		\"origin_x\"	\"50\"\n"
		"		\"angles_y\"	\"160\"\n"
		"	}";

	SDKWeaponID eFirst = WEAPON_NONE;
	for (int i = 0; i < MAX_LOADOUT; i++)
	{
		for (int j = 0; j < pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i); j++)
		{
			CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);
			if (!pWeaponInfo)
				continue;

			if (!eFirst)
				eFirst = (SDKWeaponID)i;

			KeyValues* pValues = new KeyValues("preview");
			pValues->LoadFromBuffer("model", szTemplate);

			pValues->SetString("modelname", pWeaponInfo->szWorldModel);

			pPreviews[iPreview]->ParseModelInfo(pValues);
			pPreviews[iPreview]->SetVisible(true);

			pValues->deleteThis();

			iPreview++;
		}
	}

	for (int i = iPreview; i < 4; i++)
	{
		pPreviews[i]->SetVisible(false);
	}

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
	CSDKWeaponInfo* pWeaponInfo = nullptr;
	if (eFirst)
		pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eFirst);

	if (pPlayerPreview)
	{
		KeyValues* pValues = new KeyValues("preview");
		pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

		pValues->SetString("modelname", m_pszCharacterPreview);

		if (pWeaponInfo)
		{
			KeyValues* pAnimation = pValues->FindKey("animation");
			if (pAnimation)
				pAnimation->SetString("sequence", VarArgs("%s_idle", WeaponIDToAlias(eFirst)));

			KeyValues* pWeapon = pValues->FindKey("attached_model");
			if (pWeapon)
				pWeapon->SetString("modelname", pWeaponInfo->szWorldModel);
		}
		else
		{
			KeyValues* pAnimation = pValues->FindKey("animation");
			if (pAnimation)
				pAnimation->SetString("sequence", "idle");

			KeyValues* pWeapon = pValues->FindKey("attached_model");
			if (pWeapon)
				pWeapon->SetString("modelname", "");
		}

		pPlayerPreview->ParseModelInfo(pValues);

		pValues->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CDABBuyMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "WeaponButton", controlName ) )
	{
		CWeaponButton *newButton = new CWeaponButton( this, NULL, m_pWeaponInfoPanel );

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
void CDABBuyMenu::OnShowPage( const char *pagename )
{
}

void CDABBuyMenu::OnSuicideOptionChanged( vgui::Panel *Panel )
{
	hud_buyautokill.SetValue( m_pSuicideOption->IsSelected() );
}
//-----------------------------------------------------------------------------
// Do things that should be done often, eg number of players in the 
// selected class
//-----------------------------------------------------------------------------
void CDABBuyMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == PLAYERCLASS_UNASSIGNED and if so
	//hide the cancel button

	if ( !IsVisible() )
		return;

	BaseClass::OnTick();
}

void CDABBuyMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ServerCmd( "menuopen" );			// to the server
		engine->ClientCmd( "_cl_buymenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "menuclosed" );	
		engine->ClientCmd( "_cl_buymenuopen 0" );
	}
}

void CDABBuyMenu::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "close" ) == 0 )
	{
		Close();

		gViewPortInterface->ShowBackGround( false );

		BaseClass::OnCommand( command );

		if ( hud_buyautokill.GetBool() )
			engine->ClientCmd( "kill" );
	}
	else
		engine->ClientCmd( command );
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CDABBuyMenu::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CDABBuyMenu::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDABBuyMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
}

void CDABBuyMenu::SetCharacterPreview(const char* pszCharacter)
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

CON_COMMAND(hud_reload_buy, "Reload resource for buy menu.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_BUY );
	CDABBuyMenu *pBuy = dynamic_cast<CDABBuyMenu*>(pPanel);
	if (!pBuy)
		return;
	
	pBuy->LoadControlSettings( "Resource/UI/BuyMenu.res" );
	pBuy->InvalidateLayout();
	pBuy->Update();
}

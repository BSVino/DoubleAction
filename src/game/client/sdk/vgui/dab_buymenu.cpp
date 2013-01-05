//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#include "cbase.h"
#include <stdio.h>
#include <string>
#include <sstream>

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
#include "IGameUIFuncs.h" // for key bindings

#include "basemodelpanel.h"

#include "ammodef.h"

#include "sdk_backgroundpanel.h"

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "dab_buymenu.h"
#include "folder_gui.h"
#include "da.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_buymenuopen( "_cl_buymenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when buy menu is open" );
ConVar hud_buyautokill("hud_buyautokill", "0");

CWeaponButton::CWeaponButton(vgui::Panel *parent, const char *panelName )
	: Button( parent, panelName, "WeaponButton")
{
	m_pArmedBorder = nullptr;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	InvalidateLayout(true, true);
}

void CWeaponButton::ApplySettings( KeyValues *resourceData )
{
	BaseClass::ApplySettings( resourceData );

	strcpy(m_szInfoString, resourceData->GetString("info_string"));
	strcpy(m_szInfoModel, resourceData->GetString("info_model"));
	strcpy(m_szWeaponID, resourceData->GetString("weaponid"));
}

void CWeaponButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pArmedBorder = pScheme->GetBorder("FolderButtonArmedBorder");
}

void CWeaponButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	InvalidateLayout();
	SetBorder(m_pArmedBorder);

	CDABBuyMenu* pParent = dynamic_cast<CDABBuyMenu*>(GetParent());
	if (!pParent)
		return;

	vgui::Label* pInfoLabel = pParent->GetWeaponInfo();
	if (pInfoLabel)
	{
		if (m_szWeaponID[0])
			pInfoLabel->SetText((std::string("#weaponinfo_") + m_szWeaponID).c_str());
		else
			pInfoLabel->SetText(m_szInfoString);
	}

	CModelPanel* pInfoModel = pParent->GetWeaponImage();
	if (pInfoModel)
	{
		if (!strlen(m_szInfoModel))
			pInfoModel->SwapModel("");
		else
			pInfoModel->SwapModel(m_szInfoModel);
	}
}

void CWeaponButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	InvalidateLayout();
}

SDKWeaponID CWeaponButton::GetWeaponID()
{
	return AliasToWeaponID(m_szWeaponID);
}

CDABBuyMenu::CDABBuyMenu(IViewPort* pViewPort) : vgui::Frame( NULL, PANEL_BUY )
{
	m_pszCharacterPreview = "models/player/playermale.mdl";
	m_pFolderBackground = nullptr;

	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	m_iBuyMenuKey = BUTTON_CODE_INVALID;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pSuicideOption = new CheckButton( this, "suicide_option", "Sky is blue?" );

	LoadControlSettings( "Resource/UI/BuyMenu.res" );
	InvalidateLayout();

	m_pWeaponInfo = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponInfo"));
	m_pWeaponImage = dynamic_cast<CModelPanel*>(FindChildByName("WeaponImage"));
}

//Destructor
CDABBuyMenu::~CDABBuyMenu()
{
}

void CDABBuyMenu::Reset()
{
	m_pWeaponInfo->SetText("");
	m_pWeaponImage->SwapModel("");
}

void CDABBuyMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		engine->CheckPoint( "BuyMenu" );

		m_iBuyMenuKey = gameuifuncs->GetButtonCodeForBind( "buy" );

		m_pSuicideOption->SetSelected( hud_buyautokill.GetBool() );
	}

	m_pWeaponInfo->SetText("");
	m_pWeaponImage->SwapModel("");

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
	m_pWeaponInfo = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponInfo"));
	m_pWeaponInfo->SetText("");

	m_pWeaponImage = dynamic_cast<CModelPanel*>(FindChildByName("WeaponImage"));
	m_pWeaponImage->SwapModel("");

	Button *entry = dynamic_cast<Button *>(FindChildByName("CancelButton"));
	if (entry)
		entry->SetVisible(true);

	CFolderLabel* pWeaponType = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponType"));
	int iWeaponTypeX, iWeaponTypeY;
	pWeaponType->GetPos(iWeaponTypeX, iWeaponTypeY);

	CFolderLabel* pWeaponAmmo = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponAmmo"));
	int iWeaponAmmoX, iWeaponAmmoY;
	pWeaponAmmo->GetPos(iWeaponAmmoX, iWeaponAmmoY);

	CFolderLabel* pWeaponWeight = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponWeight"));
	int iWeaponWeightX, iWeaponWeightY;
	pWeaponWeight->GetPos(iWeaponWeightX, iWeaponWeightY);

	CFolderLabel* pWeaponQuantity = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponQuantity"));
	int iWeaponQuantityX, iWeaponQuantityY;
	pWeaponQuantity->GetPos(iWeaponQuantityX, iWeaponQuantityY);

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

	for ( int i = 0; i < m_apTypes.Count(); i++)
	{
		m_apTypes[i]->DeletePanel();
		m_apTypes[i] = nullptr;
	}

	for ( int i = 0; i < m_apAmmos.Count(); i++)
	{
		m_apAmmos[i]->DeletePanel();
		m_apAmmos[i] = nullptr;
	}

	for ( int i = 0; i < m_apWeights.Count(); i++)
	{
		m_apWeights[i]->DeletePanel();
		m_apWeights[i] = nullptr;
	}

	for ( int i = 0; i < m_apQuantities.Count(); i++)
	{
		m_apQuantities[i]->DeletePanel();
		m_apQuantities[i] = nullptr;
	}

	m_apTypes.RemoveAll();
	m_apAmmos.RemoveAll();
	m_apWeights.RemoveAll();
	m_apQuantities.RemoveAll();

	CUtlVector<CWeaponButton*> apWeaponButtons;

	for ( int i = 0 ; i < GetChildCount() ; ++i )
	{
		// Hide the subpanel for the CWeaponButtons
		CWeaponButton *pPanel = dynamic_cast<CWeaponButton *>( GetChild( i ) );

		if (!pPanel)
			continue;

		if (pPanel->GetWeaponID() != WEAPON_NONE)
			apWeaponButtons.AddToTail(pPanel);

		pPanel->SetEnabled(true);

		if (!pPanel->GetName())
			continue;

		if (strlen(pPanel->GetName()) < 7)
			continue;

		SDKWeaponID eWeapon = AliasToWeaponID(pPanel->GetName() + 7);

		if (!eWeapon)
			continue;

		pPanel->SetEnabled(pPlayer->CanAddToLoadout(eWeapon));
		pPanel->InvalidateLayout(true);
	}

	for (int i = 0; i < apWeaponButtons.Count(); i++)
	{
		CWeaponButton* pPanel = apWeaponButtons[i];

		int iWeaponX, iWeaponY;
		pPanel->GetPos(iWeaponX, iWeaponY);

		CSDKWeaponInfo* pInfo = CSDKWeaponInfo::GetWeaponInfo(pPanel->GetWeaponID());

		m_apTypes.AddToTail(new CFolderLabel(this, nullptr));

		m_apTypes.Tail()->SetText((std::string("#DA_WeaponType_") + WeaponTypeToAlias(pInfo->m_eWeaponType)).c_str());
		m_apTypes.Tail()->SetPos(iWeaponTypeX, iWeaponY);
		m_apTypes.Tail()->SetZPos(-5);
		m_apTypes.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderSmall"));
		m_apTypes.Tail()->SetScheme("FolderScheme");

		m_apAmmos.AddToTail(new CFolderLabel(this, nullptr));

		m_apAmmos.Tail()->SetText((std::string("#DA_Ammo_") + pInfo->szAmmo1).c_str());
		m_apAmmos.Tail()->SetPos(iWeaponAmmoX, iWeaponY);
		m_apAmmos.Tail()->SetZPos(-5);
		m_apAmmos.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderSmall"));
		m_apAmmos.Tail()->SetScheme("FolderScheme");

		m_apWeights.AddToTail(new CFolderLabel(this, nullptr));

		std::ostringstream sWeight;
		sWeight << pInfo->iWeight;
		m_apWeights.Tail()->SetText(sWeight.str().c_str());
		m_apWeights.Tail()->SetPos(iWeaponWeightX, iWeaponY);
		m_apWeights.Tail()->SetZPos(-5);
		m_apWeights.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderMedium"));
		m_apWeights.Tail()->SetScheme("FolderScheme");

		if (pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID()))
		{
			m_apQuantities.AddToTail(new CFolderLabel(this, nullptr));

			std::ostringstream sCount;
			sCount << pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID());
			m_apQuantities.Tail()->SetText(sCount.str().c_str());
			m_apQuantities.Tail()->SetPos(iWeaponQuantityX, iWeaponY);
			m_apQuantities.Tail()->SetZPos(-5);
			m_apQuantities.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderMedium"));
			m_apQuantities.Tail()->SetScheme("FolderScheme");
		}
	}

	CFolderLabel* pLabels[2];
	pLabels[0] = dynamic_cast<CFolderLabel *>(FindChildByName("RequestedArmament1"));
	pLabels[1] = dynamic_cast<CFolderLabel *>(FindChildByName("RequestedArmament2"));

	int iArmamentsOn1 = 0;

	std::wostringstream sLabel1;
	std::wostringstream sLabel2;

	SDKWeaponID eFirst = WEAPON_NONE;
	for (int i = 0; i < MAX_LOADOUT; i++)
	{
		if (!pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i))
			continue;

		CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);
		if (!pWeaponInfo)
			continue;

		if (!eFirst)
			eFirst = (SDKWeaponID)i;

		std::wostringstream sLabel;

		const wchar_t *pchFmt = g_pVGuiLocalize->Find( pWeaponInfo->szPrintName );
		if ( pchFmt && pchFmt[0] )
			sLabel << pchFmt;
		else
			sLabel << pWeaponInfo->szPrintName;

		if (pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i) > 1)
			sLabel << " x" << pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i) << "\n";
		else
			sLabel << "\n";

		if (pWeaponInfo->szAmmo1[0] && !FStrEq(pWeaponInfo->szAmmo1, "grenades"))
		{
			int iAmmo = min(pWeaponInfo->iMaxClip1*pWeaponInfo->m_iDefaultAmmoClips, GetAmmoDef()->GetAmmoOfIndex(GetAmmoDef()->Index(pWeaponInfo->szAmmo1))->pMaxCarry);
			int iMags = iAmmo/pWeaponInfo->iMaxClip1;
			sLabel << "  " << pWeaponInfo->szAmmo1 << " x" << iAmmo << "\n";
			sLabel << "  " << pWeaponInfo->iMaxClip1 << " round mag x" << iMags << "\n";
		}

		sLabel << "\n";

		if (iArmamentsOn1 >= 2)
			sLabel2 << sLabel.str();
		else
			sLabel1 << sLabel.str();

		iArmamentsOn1++;
	}

	pLabels[0]->SetText(sLabel1.str().c_str());
	pLabels[1]->SetText(sLabel2.str().c_str());

	const char szPlayerPreviewTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/player/playermale.mdl\"\n"
		"		\"origin_z\"	\"-57\"\n"
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
	if (FStrEq(controlName, "WeaponButton"))
		return new CWeaponButton(this, nullptr);

	Panel* pResult = Folder_CreateControlByName( this, controlName );
	if (pResult)
		return pResult;

	return BaseClass::CreateControlByName( controlName );
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

	if (!m_pFolderBackground)
		m_pFolderBackground = gHUD.GetIcon("folder_background");

	m_pFolderBackground->DrawSelf(0, 0, wide, tall, Color(255, 255, 255, 255));
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

vgui::Label* CDABBuyMenu::GetWeaponInfo()
{
	return m_pWeaponInfo;
}

CModelPanel* CDABBuyMenu::GetWeaponImage()
{
	return m_pWeaponImage;
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

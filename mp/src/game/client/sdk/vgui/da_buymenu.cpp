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
#include "IGameUIFuncs.h" // for key bindings

#include "basemodelpanel.h"

#include "ammodef.h"

#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"

#include "da_buymenu.h"
#include "folder_gui.h"
#include "da.h"

#undef min
#undef max

#include <string>
#include <sstream>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_buymenuopen( "_cl_buymenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when buy menu is open" );

CWeaponButton::CWeaponButton(vgui::Panel *parent, const char *panelName )
	: Button( parent, panelName, "WeaponButton")
{
	m_pArmedBorder = NULL;

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

	CDABuyMenu* pParent = dynamic_cast<CDABuyMenu*>(GetParent());
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

CDABuyMenu::CDABuyMenu(Panel *parent) : CFolderMenuPanel( parent, PANEL_BUY )
{
	m_pWeaponInfo = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponInfo"));
	m_pWeaponImage = dynamic_cast<CModelPanel*>(FindChildByName("WeaponImage"));

	m_iBuyMenuKey = BUTTON_CODE_INVALID;

	m_pszControlSettingsFile = "Resource/UI/BuyMenu.res";

	SetVisible(true);

	LoadControlSettings( m_pszControlSettingsFile );
	InvalidateLayout();
	Update();
}

void CDABuyMenu::Reset()
{
	m_pWeaponInfo->SetText("");
	m_pWeaponImage->SwapModel("");
}

void CDABuyMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_iBuyMenuKey = gameuifuncs->GetButtonCodeForBind( "buy" );

	m_pWeaponInfo->SetText("");
	m_pWeaponImage->SwapModel("");

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

void CDABuyMenu::OnCommand( const char *command )
{
	BaseClass::OnCommand(command);

	if (V_strncasecmp("buy ", command, 4) == 0)
	{
		engine->ServerCmd(command);
		Update();
	}

	if (FStrEq(command, "close"))
	{
		// Automatically bring up the next menu if the player is dead.
		if (C_SDKPlayer::GetLocalSDKPlayer() && !C_SDKPlayer::GetLocalSDKPlayer()->IsAlive())
			GetFolderMenu()->ShowPage( PANEL_BUY_EQUIP_CT );
	}
}

void CDABuyMenu::OnKeyCodePressed( KeyCode code )
{
	if ( code == KEY_PAD_ENTER || code == KEY_ENTER )
	{
		engine->ServerCmd("buy random");

		GetFolderMenu()->ShowPage( PANEL_BUY_EQUIP_CT );
	}
	else if ( m_iBuyMenuKey != BUTTON_CODE_INVALID && m_iBuyMenuKey == code )
	{
		ShowPanel( false );
	}
}

static ConVar hud_playerpreview_x("hud_playerpreview_x", "120", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_y("hud_playerpreview_y", "-5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_z("hud_playerpreview_z", "-57", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CDABuyMenu::Update()
{
	m_pWeaponInfo = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponInfo"));
	m_pWeaponInfo->SetText("");

	m_pWeaponImage = dynamic_cast<CModelPanel*>(FindChildByName("WeaponImage"));
	m_pWeaponImage->SwapModel("");

	Button *entry = dynamic_cast<Button *>(FindChildByName("CancelButton"));
	if (entry)
		entry->SetVisible(true);

	CFolderLabel* pWeaponStyle = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponStyle"));
	int iWeaponStyleX, iWeaponStyleY;
	pWeaponStyle->GetPos(iWeaponStyleX, iWeaponStyleY);

	CFolderLabel* pWeaponWeight = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponWeight"));
	int iWeaponWeightX, iWeaponWeightY;
	pWeaponWeight->GetPos(iWeaponWeightX, iWeaponWeightY);

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	for ( int i = 0; i < m_apStyles.Count(); i++)
	{
		m_apStyles[i]->DeletePanel();
		m_apStyles[i] = NULL;
	}

	for ( int i = 0; i < m_apWeights.Count(); i++)
	{
		m_apWeights[i]->DeletePanel();
		m_apWeights[i] = NULL;
	}

	for ( int i = 0; i < m_apCheckMarks.Count(); i++)
	{
		m_apCheckMarks[i]->DeletePanel();
		m_apCheckMarks[i] = NULL;
	}

	m_apStyles.RemoveAll();
	m_apWeights.RemoveAll();
	m_apCheckMarks.RemoveAll();

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

		m_apStyles.AddToTail(new CFolderLabel(this, NULL));

		std::ostringstream sStyle;
		if (pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID()) == 2)
		{
			if (*pInfo->m_szAkimbo)
			{
				CSDKWeaponInfo* pAkimboInfo = CSDKWeaponInfo::GetWeaponInfo(AliasToWeaponID(pInfo->m_szAkimbo));
				if (pAkimboInfo && pAkimboInfo->m_flStyleMultiplier > 1)
					sStyle << pAkimboInfo->m_flStyleMultiplier << "x";
			}
			else if (pInfo->m_flStyleMultiplier > 1)
				sStyle << pInfo->m_flStyleMultiplier << "x";
		}
		else if (pInfo->m_flStyleMultiplier > 1)
			sStyle << pInfo->m_flStyleMultiplier << "x";

		m_apStyles.Tail()->SetText(sStyle.str().c_str());
		m_apStyles.Tail()->SetPos(iWeaponStyleX, iWeaponY);
		m_apStyles.Tail()->SetZPos(-5);
		m_apStyles.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderMedium"));
		m_apStyles.Tail()->SetScheme("FolderScheme");

		m_apWeights.AddToTail(new CFolderLabel(this, NULL));

		std::ostringstream sWeight;
		sWeight << pInfo->iWeight;
		m_apWeights.Tail()->SetText(sWeight.str().c_str());
		m_apWeights.Tail()->SetPos(iWeaponWeightX, iWeaponY);
		m_apWeights.Tail()->SetZPos(-5);
		m_apWeights.Tail()->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderMedium"));
		m_apWeights.Tail()->SetScheme("FolderScheme");

		int iWeaponInfoX, iWeaponInfoY;
		m_pWeaponInfo->GetPos(iWeaponInfoX, iWeaponInfoY);

		m_apCheckMarks.AddToTail(new CImageButton(this, "checkmark"));

		m_apCheckMarks.Tail()->SetDimensions(iWeaponInfoX, iWeaponY, pPanel->GetTall(), pPanel->GetTall());
		m_apCheckMarks.Tail()->SetZPos(5);
		m_apCheckMarks.Tail()->SetVisible(true);

		if (pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID()))
		{
			m_apCheckMarks.Tail()->SetImage("folder_check");
			m_apCheckMarks.Tail()->SetCommand(VarArgs("buy remove %s", pInfo->szClassName+7));
		}
		else if (pPlayer->CanAddToLoadout(pPanel->GetWeaponID()))
		{
			m_apCheckMarks.Tail()->SetImage("folder_nocheck");
			m_apCheckMarks.Tail()->SetCommand(VarArgs("buy %s", pInfo->szClassName+7));
		}
		else
		{
			m_apCheckMarks.Tail()->SetImage("folder_nocheck");
			m_apCheckMarks.Tail()->SetImageColor(Color(255, 255, 255, 100));
		}

		if (pInfo->m_szAkimbo[0])
		{
			m_apCheckMarks.AddToTail(new CImageButton(this, "checkmark"));

			m_apCheckMarks.Tail()->SetDimensions(iWeaponInfoX + pPanel->GetTall() + 5, iWeaponY, pPanel->GetTall(), pPanel->GetTall());
			m_apCheckMarks.Tail()->SetZPos(5);
			m_apCheckMarks.Tail()->SetVisible(true);

			if (pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID()) == 2)
			{
				m_apCheckMarks.Tail()->SetImage("folder_check");
				m_apCheckMarks.Tail()->SetCommand(VarArgs("buy remove %s", pInfo->szClassName+7));
			}
			else if (pPlayer->GetLoadoutWeaponCount(pPanel->GetWeaponID()) == 0)
			{
				m_apCheckMarks.Tail()->SetImage("folder_nocheck");
				m_apCheckMarks.Tail()->SetImageColor(Color(255, 255, 255, 100));
			}
			else if (pPlayer->CanAddToLoadout(pPanel->GetWeaponID()))
			{
				m_apCheckMarks.Tail()->SetImage("folder_nocheck");
				m_apCheckMarks.Tail()->SetCommand(VarArgs("buy %s", pInfo->szClassName+7));
			}
			else
			{
				m_apCheckMarks.Tail()->SetImage("folder_nocheck");
				m_apCheckMarks.Tail()->SetImageColor(Color(255, 255, 255, 100));
			}
		}
	}
}

Panel *CDABuyMenu::CreateControlByName( const char *controlName )
{
	if (FStrEq(controlName, "WeaponButton"))
		return new CWeaponButton(this, NULL);
	else
	{
		Panel* pPanel = CFolderMenu::CreateControlByNameStatic(this, controlName);

		if (pPanel)
			return pPanel;

		return BaseClass::CreateControlByName(controlName);
	}
}

void CDABuyMenu::PaintBackground()
{
	// Don't
}

void CDABuyMenu::PaintBorder()
{
	// Don't
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CDABuyMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

vgui::Label* CDABuyMenu::GetWeaponInfo()
{
	return m_pWeaponInfo;
}

CModelPanel* CDABuyMenu::GetWeaponImage()
{
	return m_pWeaponImage;
}

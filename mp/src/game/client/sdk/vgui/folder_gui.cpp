#include "cbase.h"

#include "folder_gui.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/CheckButton.h>

#include "viewport_panel_names.h"
#include "basemodelpanel.h"
#include "game/client/iviewport.h"
#include "clientmode_shared.h"

#include "ammodef.h"
#include "c_sdk_player.h"
#include "sdk_gamerules.h"

#include "da_buymenu.h"
#include "da_charactermenu.h"
#include "da_skillmenu.h"
#include "sdk_teammenu.h"

// Using std version
#undef min
#undef max

#include <sstream>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CFolderMenu* CFolderMenuPanel::GetFolderMenu()
{
	return dynamic_cast<CFolderMenu*>(GetParent());
}

ConVar hud_buyautokill("hud_buyautokill", "0");

CFolderMenu::CFolderMenu(IViewPort *pViewPort) : Frame( null, "folder" )
{
	m_pPage = NULL;

	m_bNeedsUpdate = false;

	m_szCharacter[0] = '\0';
	m_szPreviewCharacter[0] = '\0';
	m_szPreviewSequence[0] = '\0';
	m_szPreviewWeaponModel[0] = '\0';

	// initialize dialog
	SetTitle("", true);

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pSuicideOption = new CheckButton( this, "suicide_option", "" );

	m_pProfileInfo = new CFolderLabel( this, "ProfileInfo" );

	// load the new scheme early!!
	SetScheme(scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	ReloadControlSettings(false);
	InvalidateLayout();
}

//Destructor
CFolderMenu::~CFolderMenu()
{
	delete m_pPage;
}

void CFolderMenu::Reset()
{
}

void CFolderMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ClientCmd_Unrestricted( "gameui_preventescapetoshow\n" );
		engine->ServerCmd( "menuopen" );
	}
	else
	{
		engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );
		engine->ServerCmd( "menuclosed" );
	}
}

void CFolderMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_pSuicideOption->SetSelected( hud_buyautokill.GetBool() );

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

void CFolderMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

Panel *CFolderMenu::CreateControlByName( const char *controlName )
{
	Panel* pPanel = CreateControlByNameStatic(this, controlName);

	if (pPanel)
		return pPanel;

	return BaseClass::CreateControlByName(controlName);
}

Panel *CFolderMenu::CreateControlByNameStatic( vgui::Panel* pParent, const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( pParent, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( pParent, NULL );

	if (FStrEq(controlName, "ImageButton"))
		return new CImageButton( pParent, NULL );

	return NULL;
}

static ConVar hud_playerpreview_x("hud_playerpreview_x", "120", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_y("hud_playerpreview_y", "-5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_z("hud_playerpreview_z", "-57", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

static ConVar hud_characterpreview_x("hud_characterpreview_x", "300", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_characterpreview_y("hud_characterpreview_y", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_characterpreview_z("hud_characterpreview_z", "-35", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CFolderMenu::Update()
{
	ReloadControlSettings(false, false);

	MoveToCenterOfScreen();

	Button *entry = dynamic_cast<Button *>(FindChildByName("ApproveButton"));
	if (entry)
		entry->SetVisible(true);

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	if (pPlayer->HasCharacterBeenChosen())
		Q_strcpy(m_szCharacter, pPlayer->GetCharacter());
	else
		m_szCharacter[0] = '\0';

	if (ShouldShowCharacterOnly() && !ShouldShowTeams())
	{
		m_pProfileInfo->SetVisible(true);
		if (m_szPreviewCharacter[0])
			m_pProfileInfo->SetText((std::string("#DA_CharacterInfo_") + m_szPreviewCharacter).c_str());
		else if (m_szCharacter[0])
			m_pProfileInfo->SetText((std::string("#DA_CharacterInfo_") + m_szCharacter).c_str());
		else
			m_pProfileInfo->SetText("#DA_CharacterInfo_None");
	}
	else
		m_pProfileInfo->SetVisible(false);

	CFolderLabel *pCharacterName = dynamic_cast<CFolderLabel *>(FindChildByName("AgentName"));
	if (pCharacterName)
	{
		if (!ShouldShowTeams())
		{
			std::string sCharacter;
			if (m_szPreviewCharacter[0])
				sCharacter = std::string("#DA_Character_") + m_szPreviewCharacter;
			else if (m_szCharacter[0])
				sCharacter = std::string("#DA_Character_") + m_szCharacter;

			std::wstring sLocalized;
			wchar_t* pszLocalized = g_pVGuiLocalize->Find( sCharacter.c_str() );

			if (pszLocalized)
				sLocalized += pszLocalized;

			if (pPlayer->m_Shared.m_iStyleSkill)
			{
				std::string sSkill = std::string("#DA_Skill_") + SkillIDToAlias((SkillID)pPlayer->m_Shared.m_iStyleSkill.Get()) + "_Adjective";

				pszLocalized = g_pVGuiLocalize->Find( sSkill.c_str() );

				if (pszLocalized)
					sLocalized += pszLocalized;
			}

			pCharacterName->SetText(sLocalized.c_str());
		}
		else
			pCharacterName->SetText("");
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
		{
			eFirst = (SDKWeaponID)i;

			if (pPlayer->GetLoadoutWeaponCount(eFirst) > 1)
			{
				CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eFirst);
				if (pWeaponInfo && pWeaponInfo->m_szAkimbo[0])
				{
					// If we have two of this weapon and this weapon has an akimbo, use the akimbo instead.
					eFirst = AliasToWeaponID(pWeaponInfo->m_szAkimbo);
				}
			}
		}
		else
		{
			if (pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i) > 1)
			{
				CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);
				if (pWeaponInfo && pWeaponInfo->m_szAkimbo[0])
				{
					SDKWeaponID eAkimbo = AliasToWeaponID(pWeaponInfo->m_szAkimbo);
					if (eAkimbo < eFirst)
					{
						// If we have this akimbo and it's preferred to the current weapon, use it instead.
						// (Preferred means lower weapon ID.)
						eFirst = eAkimbo;
					}
				}
			}
		}

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

		if (iArmamentsOn1 >= 2)
			sLabel2 << sLabel.str();
		else
			sLabel1 << sLabel.str();

		iArmamentsOn1++;
	}

	if (pLabels[0])
		pLabels[0]->SetText(sLabel1.str().c_str());

	if (pLabels[1])
		pLabels[1]->SetText(sLabel2.str().c_str());

	const char szPlayerPreviewTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/player/frank.mdl\"\n"
		"		\"origin_z\"	\"-57\"\n"
		"		\"origin_y\"	\"10\"\n"
		"		\"origin_x\"	\"110\"\n"
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
	CSDKWeaponInfo* pWeaponInfo = NULL;
	if (eFirst)
		pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eFirst);

	if ((m_szCharacter[0] || m_szPreviewCharacter[0]) && pPlayerPreview && !ShouldShowTeams())
	{
		KeyValues* pValues = new KeyValues("preview");
		pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

		const char* pCharacter = m_szCharacter;
		if (m_szPreviewCharacter[0])
			pCharacter = m_szPreviewCharacter;

		pValues->SetString("modelname", VarArgs("models/player/%s.mdl", pCharacter));

		if (ShouldShowCharacterOnly() || ShouldShowCharacterAndWeapons())
		{
			pValues->SetFloat("origin_x", hud_characterpreview_x.GetFloat());
			pValues->SetFloat("origin_y", hud_characterpreview_y.GetFloat());
			pValues->SetFloat("origin_z", hud_characterpreview_z.GetFloat());
		}
		else
		{
			pValues->SetFloat("origin_x", hud_playerpreview_x.GetFloat());
			pValues->SetFloat("origin_y", hud_playerpreview_y.GetFloat());
			pValues->SetFloat("origin_z", hud_playerpreview_z.GetFloat());
		}

		if (m_pPage && FStrEq(m_pPage->GetName(), "class") && m_szPreviewSequence[0] && m_szPreviewWeaponModel[0] && !pPlayer->GetLoadoutWeight())
		{
			KeyValues* pAnimation = pValues->FindKey("animation");
			if (pAnimation)
				pAnimation->SetString("sequence", m_szPreviewSequence);

			KeyValues* pWeapon = pValues->FindKey("attached_model");
			if (pWeapon)
				pWeapon->SetString("modelname", m_szPreviewWeaponModel);
		}
		else if (pWeaponInfo)
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

		if (SDKGameRules()->IsTeamplay())
		{
			if (pPlayer->GetTeamNumber() == SDK_TEAM_BLUE)
				pValues->SetInt("skin", 1);
			else if (pPlayer->GetTeamNumber() == SDK_TEAM_RED)
				pValues->SetInt("skin", 2);
			else
				pValues->SetInt("skin", 0);
		}
		else
			pValues->SetInt("skin", 0);

		pPlayerPreview->ParseModelInfo(pValues);

		pValues->deleteThis();
	}
	else if (pPlayerPreview)
		pPlayerPreview->SwapModel("");

	for ( int i = 0; i < m_apWeaponIcons.Count(); i++)
	{
		if (m_apWeaponIcons[i].m_pWeaponName)
			m_apWeaponIcons[i].m_pWeaponName->DeletePanel();
	
		if (m_apWeaponIcons[i].m_pSlots)
			m_apWeaponIcons[i].m_pSlots->DeletePanel();

		if (m_apWeaponIcons[i].m_pImage)
			m_apWeaponIcons[i].m_pImage->DeletePanel();

		if (m_apWeaponIcons[i].m_pDelete)
			m_apWeaponIcons[i].m_pDelete->DeletePanel();
	}

	m_apWeaponIcons.RemoveAll();

	const char szWeaponPreviewTemplate[] =
		"	\"model\"\n"
		"	{\n"
		"		\"spotlight\"	\"1\"\n"
		"		\"modelname\"	\"models/weapons/beretta.mdl\"\n"
		"		\"origin_x\"	\"30\"\n"
		"		\"origin_y\"	\"3\"\n"
		"		\"origin_z\"	\"-3\"\n"
		"		\"angles_y\"	\"200\"\n"
		"	}";

	Panel *pWeaponIconArea = FindChildByName("WeaponIconArea");
	if ((ShouldShowCharacterAndWeapons() || ShouldShowEverything()) && pWeaponIconArea)
	{
		int iWeaponAreaX, iWeaponAreaY, iWeaponAreaW, iWeaponAreaH;
		pWeaponIconArea->GetPos(iWeaponAreaX, iWeaponAreaY);
		pWeaponIconArea->GetSize(iWeaponAreaW, iWeaponAreaH);

		int iMargin = 5;

		int iBoxSize = (iWeaponAreaW-5)/2;

		int iWeapon = 0;

		for (int i = 0; i < MAX_LOADOUT; i++)
		{
			if (!pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i))
				continue;

			CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);
			if (!pWeaponInfo)
				continue;

			for (int j = 0; j < pPlayer->GetLoadoutWeaponCount((SDKWeaponID)i); j++)
			{
				float flMoveRight = 0;
				if (iWeapon%2 == 1)
					flMoveRight = iBoxSize + iMargin;

				CWeaponIcon* pIcon = &m_apWeaponIcons[m_apWeaponIcons.AddToTail()];

				pIcon->m_pWeaponName = new CFolderLabel(this, NULL);
				pIcon->m_pWeaponName->SetText(VarArgs("#DA_Weapon_Obituary_%s", pWeaponInfo->szClassName+7)); // Use the obit version because it's shorter
				pIcon->m_pWeaponName->SetPos(iWeaponAreaX + flMoveRight, iWeaponAreaY + 10 + (iWeapon/2) * (iBoxSize+iMargin));
				pIcon->m_pWeaponName->SetSize(iBoxSize, 15);
				pIcon->m_pWeaponName->SetContentAlignment(Label::a_center);
				pIcon->m_pWeaponName->SetZPos(-5);
				pIcon->m_pWeaponName->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderLarge"));
				pIcon->m_pWeaponName->SetScheme("FolderScheme");

				std::wostringstream sSlotsLabel;

				if (pWeaponInfo->iWeight)
				{
					const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DA_BuyMenu_Weapon_Slots" );
					if ( pchFmt && pchFmt[0] )
						sSlotsLabel << pchFmt;
					else
						sSlotsLabel << "Slots: ";

					sSlotsLabel << pWeaponInfo->iWeight;

					pIcon->m_pSlots = new CFolderLabel(this, NULL);
					pIcon->m_pSlots->SetText(sSlotsLabel.str().c_str());
					pIcon->m_pSlots->SetPos(iWeaponAreaX + flMoveRight, iWeaponAreaY + iBoxSize - 10 + (iWeapon/2) * (iBoxSize+iMargin));
					pIcon->m_pSlots->SetSize(iBoxSize, 15);
					pIcon->m_pSlots->SetContentAlignment(Label::a_center);
					pIcon->m_pSlots->SetZPos(-5);
					pIcon->m_pSlots->SetFont(vgui::scheme()->GetIScheme(GetScheme())->GetFont("FolderSmall"));
					pIcon->m_pSlots->SetScheme("FolderScheme");
				}

				KeyValues* pValues = new KeyValues("preview");
				pValues->LoadFromBuffer("model", szWeaponPreviewTemplate);
				pValues->SetString("modelname", pWeaponInfo->szWorldModel);

				if (pWeaponInfo->m_eWeaponType == WT_PISTOL)
					pIcon->m_flDistance = 20;
				else if (pWeaponInfo->m_eWeaponType == WT_RIFLE)
					pIcon->m_flDistance = 50;
				else if (pWeaponInfo->m_eWeaponType == WT_SHOTGUN)
					pIcon->m_flDistance = 50;
				else if (pWeaponInfo->m_eWeaponType == WT_SMG)
					pIcon->m_flDistance = 30;
				else if (pWeaponInfo->m_eWeaponType == WT_GRENADE)
					pIcon->m_flDistance = 20;

				pIcon->m_pImage = new CModelPanel(this, NULL);
				pIcon->m_pImage->SetPos(iWeaponAreaX + flMoveRight, iWeaponAreaY + (iWeapon/2) * (iBoxSize+iMargin));
				pIcon->m_pImage->SetSize(iBoxSize, iBoxSize);
				pIcon->m_pImage->SetZPos(-15);
				pIcon->m_pImage->SetScheme("FolderScheme");
				pIcon->m_pImage->ParseModelInfo(pValues);

				pValues->deleteThis();

				pIcon->m_pDelete = new CImageButton(this, VarArgs("delete_%d", iWeapon));
				pIcon->m_pDelete->SetDimensions(iWeaponAreaX + iBoxSize - 8 + flMoveRight, iWeaponAreaY + 30 + (iWeapon/2) * (iBoxSize+iMargin), 12, 12);
				pIcon->m_pDelete->SetZPos(15);
				pIcon->m_pDelete->SetImage("folder_delete");
				pIcon->m_pDelete->SetPaintBorderEnabled(false);
				pIcon->m_pDelete->SetPaintBackgroundEnabled(true);
				pIcon->m_pDelete->SetCommand(VarArgs("buy remove %d", i));

				iWeapon++;
			}
		}
	}

	CFolderLabel* pWeaponTotalWeightNumber = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponTotalWeightNumber"));
	CFolderLabel* pWeaponTotalWeight = dynamic_cast<CFolderLabel*>(FindChildByName("WeaponTotalWeight"));
	if ((ShouldShowCharacterAndWeapons() || ShouldShowEverything()) && pPlayer->GetLoadoutWeight())
	{
		if (pWeaponTotalWeightNumber)
		{
			wchar_t szText[20];
			_snwprintf( szText, ARRAYSIZE(szText) - 1, L"%d/%d", pPlayer->GetLoadoutWeight(), MAX_LOADOUT_WEIGHT );
			pWeaponTotalWeightNumber->SetText(szText);
			pWeaponTotalWeightNumber->SetVisible(true);
		}

		if (pWeaponTotalWeight)
			pWeaponTotalWeight->SetVisible(true);
	}
	else
	{
		if (pWeaponTotalWeightNumber)
			pWeaponTotalWeightNumber->SetVisible(false);

		if (pWeaponTotalWeight)
			pWeaponTotalWeight->SetVisible(false);
	}

	if (ShouldShowEverything())
	{
		CFolderLabel *pSkillInfo = dynamic_cast<CFolderLabel *>(FindChildByName("SkillInfo"));
		CPanelTexture *pSkillIcon = dynamic_cast<CPanelTexture *>(FindChildByName("SkillIcon"));

		if (pSkillInfo && pSkillIcon)
		{
			if (pPlayer->m_Shared.m_iStyleSkill)
			{
				pSkillInfo->SetText((std::string("#DA_SkillInfo_") + SkillIDToAlias((SkillID)pPlayer->m_Shared.m_iStyleSkill.Get())).c_str());
				pSkillIcon->SetImage(SkillIDToAlias((SkillID)pPlayer->m_Shared.m_iStyleSkill.Get()));
			}
			else
			{
				pSkillInfo->SetText("");
				pSkillIcon->SetImage("");
			}
		}
	}

	m_pSuicideOption->SetVisible(pPlayer->IsAlive() && !ShouldShowTeams());

	Button *pProceedButton = dynamic_cast<Button *>(FindChildByName("ProceedButton"));
	if (pProceedButton)
		pProceedButton->SetVisible(m_pPage && FStrEq(m_pPage->GetName(), PANEL_BUY));

	Button *pApproveButton = dynamic_cast<Button *>(FindChildByName("ApproveButton"));
	if (pApproveButton)
		pApproveButton->SetVisible(IsLoadoutComplete());

	Button *pAgentsTab = dynamic_cast<Button *>(FindChildByName("AgentsTab"));
	if (pAgentsTab)
		pAgentsTab->SetVisible(pPlayer->HasCharacterBeenChosen());

	Button *pWeaponsTab = dynamic_cast<Button *>(FindChildByName("WeaponsTab"));
	if (pWeaponsTab)
		pWeaponsTab->SetVisible(pPlayer->HasCharacterBeenChosen());

	Button *pSkillsTab = dynamic_cast<Button *>(FindChildByName("SkillsTab"));
	if (pSkillsTab)
		pSkillsTab->SetVisible(pPlayer->HasSkillsTabBeenSeen());

	Button *pChangeTeams = dynamic_cast<Button *>(FindChildByName("ChangeTeamsButton"));
	if (pChangeTeams)
		pChangeTeams->SetVisible(SDKGameRules()->IsTeamplay());
}

void CFolderMenu::OnSuicideOptionChanged( Panel *Panel )
{
	hud_buyautokill.SetValue( m_pSuicideOption->IsSelected() );
}

void CFolderMenu::OnKeyCodeTyped( KeyCode code )
{
	BaseClass::OnKeyCodeTyped(code);

	if (code == KEY_ESCAPE)
	{
		Close();

		if (!IsLoadoutComplete())
			engine->ServerCmd("spectate");
	}
}

void CFolderMenu::OnKeyCodePressed( KeyCode code )
{
	if (m_pPage)
		m_pPage->OnKeyCodePressed(code);

	BaseClass::OnKeyCodePressed(code);
}

void CFolderMenu::OnCommand( const char *command )
{
	if ( Q_strncasecmp( command, "tab ", 4 ) == 0)
	{
		if (FStrEq(command+4, "characters"))
			ShowPage(PANEL_CLASS);
		else if (FStrEq(command+4, "weapons"))
			ShowPage(PANEL_BUY);
		else if (FStrEq(command+4, "skills"))
			ShowPage(PANEL_BUY_EQUIP_CT);
		else if (FStrEq(command+4, "team"))
			ShowPage(PANEL_TEAM);
	}
	else if ( Q_stricmp( command, "close" ) == 0 )
	{
		Close();

		BaseClass::OnCommand( command );

		if ( hud_buyautokill.GetBool() )
			engine->ClientCmd( "kill" );
	}
	else if ( Q_stricmp( command, "respawn" ) == 0 )
	{
		Close();

		BaseClass::OnCommand( command );

		engine->ServerCmd("jointeam 0");

		if ( hud_buyautokill.GetBool() )
			engine->ClientCmd( "kill" );

		engine->ServerCmd("respawn");
	}
	else if ( Q_stricmp( command, "spectate" ) == 0 )
	{
		engine->ServerCmd( "spectate" );	

		Close();

		BaseClass::OnCommand( command );
	}
	else
		engine->ClientCmd( command );	
}

void CFolderMenu::OnTick()
{
	if (m_bNeedsUpdate)
	{
		Update();

		if (m_pPage)
			m_pPage->Update();

		m_bNeedsUpdate = false;
	}

	for (int i = 0; i < m_apWeaponIcons.Count(); i++)
		PositionWeaponInModelPanel(m_apWeaponIcons[i].m_pImage, 90, m_apWeaponIcons[i].m_flDistance);

	BaseClass::OnTick();
}

void CFolderMenu::PaintBackground()
{
	// Don't
}

void CFolderMenu::PaintBorder()
{
	// Don't
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CFolderMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.
}

void CFolderMenu::ShowPage(const char* pszPage)
{
	delete m_pPage;

	if (FStrEq(pszPage, PANEL_CLASS))
		m_pPage = new CDACharacterMenu(this);
	else if (FStrEq(pszPage, PANEL_BUY))
		m_pPage = new CDABuyMenu(this);
	else if (FStrEq(pszPage, PANEL_BUY_EQUIP_CT))
		m_pPage = new CDASkillMenu(this);
	else if (FStrEq(pszPage, PANEL_TEAM))
		m_pPage = new CSDKTeamMenu(this);

	Update();
}

bool CFolderMenu::ShouldShowTeams()
{
	return m_pPage && FStrEq(m_pPage->GetName(), "team");
}

bool CFolderMenu::ShouldShowCharacterOnly()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return true;

	// If the player has anything, show the weapons version
	if (pPlayer->GetLoadoutWeight())
		return false;

	if (pPlayer->m_Shared.m_iStyleSkill != SKILL_NONE)
		return false;

	if (m_pPage)
	{
		if (FStrEq(m_pPage->GetName(), PANEL_BUY) || FStrEq(m_pPage->GetName(), PANEL_BUY_EQUIP_CT))
			return false;
	}

	return true;
}

bool CFolderMenu::ShouldShowCharacterAndWeapons()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return true;

	if (pPlayer->m_Shared.m_iStyleSkill != SKILL_NONE)
		return false;

	if (m_pPage)
	{
		if (FStrEq(m_pPage->GetName(), PANEL_BUY_EQUIP_CT))
			return false;
	}

	return true;
}

bool CFolderMenu::ShouldShowEverything()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return true;

	if (pPlayer->m_Shared.m_iStyleSkill != SKILL_NONE || m_pPage && FStrEq(m_pPage->GetName(), PANEL_BUY_EQUIP_CT))
		return true;

	return false;
}

bool CFolderMenu::IsLoadoutComplete()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return false;

	if (!pPlayer->HasCharacterBeenChosen())
		return false;

	if (pPlayer->m_Shared.m_iStyleSkill == SKILL_NONE)
		return false;

	return true;
}

void CFolderMenu::ReloadControlSettings(bool bUpdate, bool bReloadPage)
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	if (ShouldShowTeams())
		LoadControlSettings( "Resource/UI/Folder_Team.res" );
	else if (ShouldShowCharacterOnly())
		LoadControlSettings( "Resource/UI/Folder_NoWeapons.res" );
	else if (ShouldShowCharacterAndWeapons())
		LoadControlSettings( "Resource/UI/Folder_Weapons.res" );
	else
		LoadControlSettings( "Resource/UI/Folder_Complete.res" );

	InvalidateLayout();
	if (bUpdate)
		Update();

	if (m_pPage && bReloadPage)
	{
		m_pPage->LoadControlSettings(m_pPage->GetControlSettingsFile());
		m_pPage->InvalidateLayout();
		if (bUpdate)
			m_pPage->Update();
	}
}

void CFolderMenu::SetCharacterPreview(const char* pszCharacter, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch)
{
	if (pszCharacter)
		strcpy(m_szPreviewCharacter, pszCharacter);
	else
		m_szPreviewCharacter[0] = '\0';

	if (pszSequence)
		strcpy(m_szPreviewSequence, pszSequence);
	else
		m_szPreviewSequence[0] = '\0';

	if (pszWeaponModel)
		strcpy(m_szPreviewWeaponModel, pszWeaponModel);
	else
		m_szPreviewWeaponModel[0] = '\0';

	m_flBodyPitch = flPitch;
	m_flBodyYaw = flYaw;

	Update();
}

CFolderLabel::CFolderLabel(Panel *parent, const char *panelName)
	: Label( parent, panelName, "")
{
	m_bUnderline = false;
}

void CFolderLabel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_bUnderline = inResourceData->GetBool("underline", false);
}

void CFolderLabel::Paint()
{
	BaseClass::Paint();

	vgui::TextImage* pTextImage = GetTextImage();

	if (m_bUnderline && pTextImage)
	{
		int x, y;
		pTextImage->GetPos(x, y);

		int w, h;
		pTextImage->GetSize(w, h);

		surface()->DrawSetColor(0, 0, 0, 255);
		surface()->DrawFilledRect(x, y+h-2, x+w, y+h);
	}
}

CPanelTexture::CPanelTexture(Panel *parent, const char *panelName)
	: Panel(parent, panelName)
{
	m_pImage = NULL;

	m_iX = m_iY = 0;
	m_iWidth = m_iHeight = 100;
}

void CPanelTexture::SetImage(const char* pszName)
{
	V_strncpy(m_szImageName, pszName, ARRAYSIZE(m_szImageName));
	m_pImage = gHUD.GetIcon(pszName);
}

void CPanelTexture::SetDimensions(int x, int y, int w, int h)
{
	m_iX = x;
	m_iY = y;
	m_iWidth = w;
	m_iHeight = h;

	SetPos(x, y);
	SetSize(w, h);
}

void CPanelTexture::PaintBackground()
{
	if (m_pImage)
	{
		int x, y;
		GetPos(x, y);
		m_pImage->DrawSelf(m_iX, m_iY, m_iWidth, m_iHeight, Color(255, 255, 255, 255));
	}
}

void CPanelTexture::ApplySettings(KeyValues *inResourceData)
{
	Q_strncpy(m_szImageName, inResourceData->GetString("image", ""), sizeof(m_szImageName));

	BaseClass::ApplySettings(inResourceData);

	SetPos(0, 0);

	int wide, tall;

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	wide = screenWide; 
	tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	SetSize( wide, tall );

	const char *xstr = inResourceData->GetString( "xpos", NULL );
	const char *ystr = inResourceData->GetString( "ypos", NULL );
	if (xstr)
	{
		bool bRightAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (xstr[0] == 'r' || xstr[0] == 'R')
		{
			bRightAligned = true;
			xstr++;
		}
		else if (xstr[0] == 'c' || xstr[0] == 'C')
		{
			bCenterAligned = true;
			xstr++;
		}

		// get the value
		m_iX = atoi(xstr);

		// scale the x up to our screen co-ords
		if ( IsProportional() )
			m_iX = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iX);

		// now correct the alignment
		if (bRightAligned)
			m_iX = screenWide - m_iX; 
		else if (bCenterAligned)
			m_iX = (screenWide / 2) + m_iX;
	}

	if (ystr)
	{
		bool bBottomAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (ystr[0] == 'r' || ystr[0] == 'R')
		{
			bBottomAligned = true;
			ystr++;
		}
		else if (ystr[0] == 'c' || ystr[0] == 'C')
		{
			bCenterAligned = true;
			ystr++;
		}

		m_iY = atoi(ystr);
		if (IsProportional())
			// scale the y up to our screen co-ords
			m_iY = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iY);

		// now correct the alignment
		if (bBottomAligned)
			m_iY = screenTall - m_iY; 
		else if (bCenterAligned)
			m_iY = (screenTall / 2) + m_iY;
	}

	const char *wstr = inResourceData->GetString( "wide", NULL );
	if ( wstr )
	{
		bool bFull = false;
		if (wstr[0] == 'f' || wstr[0] == 'F')
		{
			bFull = true;
			wstr++;
		}

		m_iWidth = atof(wstr);
		if ( IsProportional() )
		{
			// scale the x and y up to our screen co-ords
			m_iWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iWidth);
		}

		// now correct the alignment
		if (bFull)
			m_iWidth = screenWide - m_iWidth; 
	}

	m_iHeight = inResourceData->GetInt( "tall", tall );
	if ( IsProportional() )
	{
		// scale the x and y up to our screen co-ords
		m_iHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iHeight);
	}
}

void CPanelTexture::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetImage( m_szImageName );
}

CImageButton::CImageButton(Panel *parent, const char *panelName)
	: Button(parent, panelName, "")
{
	m_pImage = NULL;
	m_clrImage = Color(255, 255, 255, 255);

	m_iX = m_iY = 0;
	m_iWidth = m_iHeight = 100;
	SetPaintBorderEnabled(false);
}

void CImageButton::SetImage(const char* pszName)
{
	V_strncpy(m_szImageName, pszName, ARRAYSIZE(m_szImageName));
	m_pImage = gHUD.GetIcon(pszName);
}

void CImageButton::SetImageColor(const Color& clrImage)
{
	m_clrImage = clrImage;
}

void CImageButton::SetDimensions(int x, int y, int w, int h)
{
	m_iX = x;
	m_iY = y;
	m_iWidth = w;
	m_iHeight = h;

	SetPos(x, y);
	SetSize(w, h);
}

void CImageButton::PaintBackground()
{
	if (m_pImage)
	{
		int x, y;
		GetPos(x, y);
		m_pImage->DrawSelf(0, 0, m_iWidth, m_iHeight, m_clrImage);
	}
}

void CImageButton::ApplySettings(KeyValues *inResourceData)
{
	Q_strncpy(m_szImageName, inResourceData->GetString("image", ""), sizeof(m_szImageName));

	BaseClass::ApplySettings(inResourceData);

	SetPos(0, 0);

	int wide, tall;

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	wide = screenWide; 
	tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	SetSize( wide, tall );

	const char *xstr = inResourceData->GetString( "xpos", NULL );
	const char *ystr = inResourceData->GetString( "ypos", NULL );
	if (xstr)
	{
		bool bRightAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (xstr[0] == 'r' || xstr[0] == 'R')
		{
			bRightAligned = true;
			xstr++;
		}
		else if (xstr[0] == 'c' || xstr[0] == 'C')
		{
			bCenterAligned = true;
			xstr++;
		}

		// get the value
		m_iX = atoi(xstr);

		// scale the x up to our screen co-ords
		if ( IsProportional() )
			m_iX = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iX);

		// now correct the alignment
		if (bRightAligned)
			m_iX = screenWide - m_iX; 
		else if (bCenterAligned)
			m_iX = (screenWide / 2) + m_iX;
	}

	if (ystr)
	{
		bool bBottomAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (ystr[0] == 'r' || ystr[0] == 'R')
		{
			bBottomAligned = true;
			ystr++;
		}
		else if (ystr[0] == 'c' || ystr[0] == 'C')
		{
			bCenterAligned = true;
			ystr++;
		}

		m_iY = atoi(ystr);
		if (IsProportional())
			// scale the y up to our screen co-ords
			m_iY = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iY);

		// now correct the alignment
		if (bBottomAligned)
			m_iY = screenTall - m_iY; 
		else if (bCenterAligned)
			m_iY = (screenTall / 2) + m_iY;
	}

	const char *wstr = inResourceData->GetString( "wide", NULL );
	if ( wstr )
	{
		bool bFull = false;
		if (wstr[0] == 'f' || wstr[0] == 'F')
		{
			bFull = true;
			wstr++;
		}

		m_iWidth = atof(wstr);
		if ( IsProportional() )
		{
			// scale the x and y up to our screen co-ords
			m_iWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iWidth);
		}

		// now correct the alignment
		if (bFull)
			m_iWidth = screenWide - m_iWidth; 
	}

	m_iHeight = inResourceData->GetInt( "tall", tall );
	if ( IsProportional() )
	{
		// scale the x and y up to our screen co-ords
		m_iHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iHeight);
	}

	SetPos(m_iX, m_iY);
	SetSize(m_iWidth, m_iHeight);
	SetPaintBorderEnabled(false);
}

void CImageButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetImage( m_szImageName );
	SetPaintBorderEnabled(false);
}

void __MsgFunc_FolderPanel( bf_read &msg )
{
	char panelname[2048]; 

	msg.ReadString( panelname, sizeof(panelname) );

	IViewPortPanel *viewport = gViewPortInterface->FindPanelByName( "folder" );

	CFolderMenu* pFolder = dynamic_cast<CFolderMenu*>(viewport);

	if ( !pFolder )
		return;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	gViewPortInterface->ShowPanel( (IViewPortPanel*)pFolder, true );

	if (pPlayer && SDKGameRules()->IsTeamplay() && (pPlayer->GetTeamNumber() != SDK_TEAM_RED && pPlayer->GetTeamNumber() != SDK_TEAM_BLUE))
		pFolder->ShowPage(PANEL_TEAM);
	else if (pPlayer && !pPlayer->HasCharacterBeenChosen())
		pFolder->ShowPage(PANEL_CLASS);
	else
		pFolder->ShowPage(panelname);
}

CON_COMMAND(hud_reload_folder, "Reload resource for folder menu.")
{
	IViewPortPanel *pViewportPanel = gViewPortInterface->FindPanelByName( PANEL_FOLDER );
	CFolderMenu *pPanel = dynamic_cast<CFolderMenu*>(pViewportPanel);
	if (!pPanel)
		return;

	pPanel->ReloadControlSettings();
}

void PositionWeaponInModelPanel(CModelPanel* pModelPanel, float flYaw, float flDistance)
{
	if (!pModelPanel)
		return;

	if (!pModelPanel->m_pModelInfo || !pModelPanel->m_hModel)
		return;

	if (pModelPanel->IsPanelDirty())
	{
		// Kind of a hack. Move it out of the way so it doesn't show until the loading is done.
		pModelPanel->m_pModelInfo->m_vecOriginOffset = Vector(0, 0, 10000);
		return;
	}

	VMatrix mCenterToWorld;

	pModelPanel->m_hModel->SetAbsOrigin(Vector(0, 0, 0));
	pModelPanel->m_hModel->SetAbsAngles(QAngle(0, 0, 0));

	int iCenter = pModelPanel->m_hModel->LookupAttachment("center");
	if (iCenter > 0)
	{
		// Use the center attachment if it's available.
		matrix3x4_t mAttachmentToWorld;
		pModelPanel->m_hModel->GetAttachment(iCenter, mAttachmentToWorld);
		mCenterToWorld.CopyFrom3x4(mAttachmentToWorld);
	}
	else
	{
		// Otherwise use the right hand weapon bone. Akimbos will never be shown in this panel so that's not an issue.
		int iBone = pModelPanel->m_hModel->LookupBone("DABBiped.RHandWeapon");
		if (iBone >= 0)
		{
			matrix3x4_t mBoneToWorld;
			pModelPanel->m_hModel->GetBoneTransform(iBone, mBoneToWorld);
			mCenterToWorld.CopyFrom3x4(mBoneToWorld);
		}
	}

	VMatrix mCenterToWorldInverse;
	MatrixInverseTR(mCenterToWorld, mCenterToWorldInverse);

	VMatrix mCamera;
	MatrixFromAngles( QAngle(0, flYaw, 90), mCamera );
	mCamera.SetTranslation( Vector(flDistance, 0, 0) );

	// First moving the model so that it's upright in the world and looking at the center bone,
	// then moving it so the camera can see it.
	VMatrix mResult = mCamera * mCenterToWorldInverse;

	QAngle angModel;
	MatrixAngles( mResult.As3x4(), angModel, pModelPanel->m_pModelInfo->m_vecOriginOffset );
	pModelPanel->m_pModelInfo->m_vecAbsAngles.x = angModel.x;
	pModelPanel->m_pModelInfo->m_vecAbsAngles.y = angModel.y;
	pModelPanel->m_pModelInfo->m_vecAbsAngles.z = angModel.z;
}

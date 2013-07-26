#include "cbase.h"

#include "folder_gui.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/CheckButton.h>

#include "viewport_panel_names.h"
#include "basemodelpanel.h"

#include "ammodef.h"
#include "c_sdk_player.h"
#include "sdk_gamerules.h"

// Using std version
#undef min
#undef max

#include <sstream>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar hud_buyautokill("hud_buyautokill", "0");

CFolderMenu::CFolderMenu(const char* pszName) : Frame( null, pszName )
{
	m_bNeedsUpdate = false;

	m_szCharacter[0] = '\0';

	// initialize dialog
	SetTitle("", true);

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pSuicideOption = new CheckButton( this, "suicide_option", "" );

	m_pProfileInfo = new CFolderLabel( this, "ProfileInfo" );
	m_pCharacteristicsInfo = new CFolderLabel( this, "CharacteristicsInfo" );

	// load the new scheme early!!
	SetScheme(scheme()->LoadSchemeFromFile("resource/FolderScheme.res", "FolderScheme"));
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);
}

//Destructor
CFolderMenu::~CFolderMenu()
{
}

void CFolderMenu::ShowPanel( bool bShow )
{
	if ( bShow )
		m_pSuicideOption->SetSelected( hud_buyautokill.GetBool() );
}

void CFolderMenu::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

Panel *CFolderMenu::CreateControlByName( const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( this, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( this, NULL );

	if (FStrEq(controlName, "ImageButton"))
		return new CImageButton( this, NULL );

	return BaseClass::CreateControlByName(controlName);
}

static ConVar hud_playerpreview_x("hud_playerpreview_x", "120", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_y("hud_playerpreview_y", "-5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar hud_playerpreview_z("hud_playerpreview_z", "-57", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CFolderMenu::Update()
{
	MoveToCenterOfScreen();

	Button *entry = dynamic_cast<Button *>(FindChildByName("ApproveButton"));
	if (entry)
		entry->SetVisible(true);

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	Q_strcpy(m_szCharacter, pPlayer->GetCharacter());

	if (m_szCharacter[0])
		m_pProfileInfo->SetText((std::string("#DA_CharacterInfo_") + m_szCharacter).c_str());
	else
		m_pProfileInfo->SetText("#DA_CharacterInfo_None");

	Label *pSlotsLabel = dynamic_cast<Label *>(FindChildByName("SlotsRemaining"));
	if (pSlotsLabel)
	{
		wchar_t szFmt[128]=L"";
		const wchar_t *pchFmt = g_pVGuiLocalize->Find( "#DA_BuyMenu_SlotsRemaining" );
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
			int iAmmo = std::min(pWeaponInfo->iMaxClip1*pWeaponInfo->m_iDefaultAmmoClips, GetAmmoDef()->GetAmmoOfIndex(GetAmmoDef()->Index(pWeaponInfo->szAmmo1))->pMaxCarry);
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

	if (m_szCharacter[0] && pPlayerPreview)
	{
		KeyValues* pValues = new KeyValues("preview");
		pValues->LoadFromBuffer("model", szPlayerPreviewTemplate);

		pValues->SetString("modelname", VarArgs("models/player/%s.mdl", m_szCharacter));

		pValues->SetFloat("origin_x", hud_playerpreview_x.GetFloat());
		pValues->SetFloat("origin_y", hud_playerpreview_y.GetFloat());
		pValues->SetFloat("origin_z", hud_playerpreview_z.GetFloat());

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

	if (pPlayer->m_Shared.m_iStyleSkill)
		m_pCharacteristicsInfo->SetText((std::string("#DA_SkillInfo_") + SkillIDToAlias((SkillID)pPlayer->m_Shared.m_iStyleSkill.Get())).c_str());
	else
		m_pCharacteristicsInfo->SetText("");
}

void CFolderMenu::OnSuicideOptionChanged( Panel *Panel )
{
	hud_buyautokill.SetValue( m_pSuicideOption->IsSelected() );
}

void CFolderMenu::OnCommand( const char *command )
{
	if ( Q_strncasecmp( command, "tab ", 4 ) == 0)
	{
		if (FStrEq(command+4, "characters"))
		{
			Close();
			engine->ServerCmd("character");
		}
		else if (FStrEq(command+4, "weapons"))
		{
			Close();
			engine->ServerCmd("buy");
		}
		else if (FStrEq(command+4, "skills"))
		{
			Close();
			engine->ServerCmd("setskill");
		}
	}
	else if ( Q_stricmp( command, "close" ) == 0 )
	{
		Close();

		BaseClass::OnCommand( command );

		if ( hud_buyautokill.GetBool() )
			engine->ClientCmd( "kill" );
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
		m_bNeedsUpdate = false;
	}

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

CFolderLabel::CFolderLabel(Panel *parent, const char *panelName)
	: Label( parent, panelName, "")
{
}

void CFolderLabel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
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
	m_pImage = gHUD.GetIcon(pszName);
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

	m_iX = m_iY = 0;
	m_iWidth = m_iHeight = 100;
	SetPaintBorderEnabled(false);
}

void CImageButton::SetImage(const char* pszName)
{
	m_pImage = gHUD.GetIcon(pszName);
}

void CImageButton::PaintBackground()
{
	if (m_pImage)
	{
		int x, y;
		GetPos(x, y);
		m_pImage->DrawSelf(0, 0, m_iWidth, m_iHeight, Color(255, 255, 255, 255));
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

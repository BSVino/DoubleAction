//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "vgui_rootpanel_sdk.h"
#include "vgui/IVGui.h"
#include <vgui/ISurface.h>
#include "c_sdk_player.h"
#include <vgui/ILocalize.h>

#undef min
#undef max

#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SDKRootPanel *g_pRootPanel = NULL;

using namespace vgui;

//-----------------------------------------------------------------------------
// Global functions.
//-----------------------------------------------------------------------------
void VGUI_CreateClientDLLRootPanel( void )
{
	g_pRootPanel = new C_SDKRootPanel( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
}

void VGUI_DestroyClientDLLRootPanel( void )
{
	delete g_pRootPanel;
	g_pRootPanel = NULL;
}

vgui::VPANEL VGui_GetClientDLLRootPanel( void )
{
	return g_pRootPanel->GetVPanel();
}


//-----------------------------------------------------------------------------
// C_SDKRootPanel implementation.
//-----------------------------------------------------------------------------
C_SDKRootPanel::C_SDKRootPanel( vgui::VPANEL parent )
	: BaseClass( NULL, "SDK Root Panel" )
{
	SetParent( parent );
	SetPaintEnabled( false );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( true );

	// Make it screen sized
	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );

	// Ask for OnTick messages
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_hDeathFrameLarge = vgui::INVALID_FONT;
	m_hDeathFrameMedium = vgui::INVALID_FONT;
	m_hDeathFrameSmall = vgui::INVALID_FONT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKRootPanel::~C_SDKRootPanel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::PaintBackground()
{
	BaseClass::PaintBackground();
	
	RenderLetterboxing();
}

//-----------------------------------------------------------------------------
// Purpose: For each panel effect, check if it wants to draw and draw it on
//  this panel/surface if so
//-----------------------------------------------------------------------------
void C_SDKRootPanel::RenderLetterboxing( void )
{
	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	C_WeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	ConVarRef spec_freeze_time("spec_freeze_time");
	if (pPlayer && !pPlayer->IsAlive() && pPlayer->GetDeathTime() && gpGlobals->curtime < pPlayer->GetDeathTime() + spec_freeze_time.GetFloat())
		RenderDeathFrame();
	else
		m_flKilledByStartTime = 0;

	float flLetterbox = 0.0;
	if (pPlayer)
	{
		if (pPlayer->m_Shared.GetAimIn() > 0 && pWeapon && (pWeapon->FullAimIn() || pWeapon->HasAimInFireRateBonus() || pWeapon->HasAimInRecoilBonus()))
		{
			flLetterbox = pPlayer->m_Shared.GetAimIn()*2;
			if (flLetterbox > 1)
				flLetterbox = 1;
		}

		if (pPlayer->GetSlowMoMultiplier() < 1)
			flLetterbox = std::max(flLetterbox, RemapValClamped(pPlayer->GetSlowMoMultiplier(), 1, 0.8f, 0, 1));
	}

	if (flLetterbox > 0)
	{
		int iWidth = ScreenWidth();
		int iHeight = ScreenHeight();

		int i169Height = iWidth*9/16;
		if (i169Height >= iHeight - 50)
			i169Height = iHeight - 50;
		int iBarHeight = ((iHeight - i169Height)/2)*flLetterbox;

		surface()->DrawSetColor(Color(0, 0, 0, 255*flLetterbox));
		surface()->DrawFilledRect( 0, 0, ScreenWidth(), iBarHeight );
		surface()->DrawFilledRect( 0, ScreenHeight()-iBarHeight, ScreenWidth(), ScreenHeight() );
	}
}

ConVar da_deathframe_lerp_time( "da_deathframe_lerp_time", "0.3", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY );

void C_SDKRootPanel::RenderDeathFrame( void )
{
	int iWidth = ScreenWidth();
	int iHeight = ScreenHeight();

	int iTopBarHeight = 150;
	int iBottomBarHeight = 75;

	surface()->DrawSetColor(Color(0, 0, 0, 255));
	surface()->DrawFilledRect( 0, 0, iWidth, iTopBarHeight );
	surface()->DrawFilledRect( 0, iHeight-iBottomBarHeight, iWidth, iHeight );

	if (m_hDeathFrameLarge == vgui::INVALID_FONT)
	{
		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( scheme );
		m_hDeathFrameLarge = pScheme->GetFont("DeathFrameLarge", false);
		m_hDeathFrameMedium = pScheme->GetFont("DeathFrameMedium", false);
		m_hDeathFrameSmall = pScheme->GetFont("DeathFrameSmall", false);
	}

	std::wstring sKilledBy;

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	C_SDKPlayer* pKiller = pPlayer->GetKiller();

	if (!m_flKilledByStartTime)
		m_flKilledByStartTime = gpGlobals->curtime;

	float flLerpTime = da_deathframe_lerp_time.GetFloat();

	wchar_t* pszScreenshot = g_pVGuiLocalize->Find("#DA_DeathFrame_Screenshot");

	if (pszScreenshot)
	{
		wchar_t szScreenshot[512];
		UTIL_ReplaceKeyBindings( pszScreenshot, wcslen(pszScreenshot)*sizeof(wchar_t), szScreenshot, sizeof( szScreenshot ) );

		float flXOffset = RemapValClamped(Bias(RemapValClamped(gpGlobals->curtime, m_flKilledByStartTime + 0.5f, m_flKilledByStartTime + 0.5f + flLerpTime, 0, 1), 0.8f), 0, 1, -iWidth*2/3, 0);

		int iWide, iTall;
		surface()->GetTextSize(m_hDeathFrameSmall, pszScreenshot, iWide, iTall);
		surface()->DrawSetTextFont(m_hDeathFrameSmall);
		surface()->DrawSetTextPos(10 + flXOffset, 10);
		surface()->DrawSetTextColor(Color(255, 255, 255, 255));
		surface()->DrawPrintText(szScreenshot, wcslen(szScreenshot));
	}

	if (pKiller != pPlayer)
	{
		wchar_t* pszKilledBy = g_pVGuiLocalize->Find("#DA_DeathFrame_KilledBy");

		float flKilledByXOffset = RemapValClamped(Bias(RemapValClamped(gpGlobals->curtime, m_flKilledByStartTime, m_flKilledByStartTime + flLerpTime, 0, 1), 0.8f), 0, 1, -iWidth*2/3, 0);

		if (pszKilledBy)
		{
			int iWide, iTall;
			surface()->GetTextSize(m_hDeathFrameLarge, pszKilledBy, iWide, iTall);
			surface()->DrawSetTextFont(m_hDeathFrameLarge);
			surface()->DrawSetTextPos(flKilledByXOffset + iWidth/2 - iWide/2, 10);
			surface()->DrawSetTextColor(Color(255, 255, 255, 255));
			surface()->DrawPrintText(pszKilledBy, wcslen(pszKilledBy));
		}

		wchar_t* pszDefaultKiller = g_pVGuiLocalize->Find("#DA_Trigger_Kill_Default");
		if (!pszDefaultKiller)
			pszDefaultKiller = L"Having A Great Big Brain";

		const char* pszKillerString = pPlayer->GetKillerString();

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];

		if (pszKillerString)
		{
			wchar_t* pszLocalized = g_pVGuiLocalize->Find(pszKillerString);
			if (pszLocalized)
			{
				V_wcsncpy(wszPlayerName, pszLocalized, sizeof(wszPlayerName));

				CUtlVector<wchar_t*> apwszLocalizations;
				apwszLocalizations.AddToTail(pszLocalized);

				if (pszKillerString[strlen(pszKillerString)-1] == '1')
				{
					char szKillerString[512];
					V_strcpy_safe(szKillerString, pszKillerString);
					char cNumber = '2';
					do
					{
						szKillerString[strlen(szKillerString)-1] = cNumber;
						pszLocalized = g_pVGuiLocalize->Find(szKillerString);
						if (pszLocalized)
							apwszLocalizations.AddToTail(pszLocalized);

						cNumber++;
					}
					while (pszLocalized);
				}

				wchar_t* pszKiller = apwszLocalizations[((int)m_flKilledByStartTime)%apwszLocalizations.Size()]; // Effectively a random number.

				V_wcsncpy(wszPlayerName, pszKiller, sizeof(wszPlayerName));
			}
			else
				V_wcsncpy(wszPlayerName, pszDefaultKiller, sizeof(wszPlayerName));
		}
		else if (pKiller)
			g_pVGuiLocalize->ConvertANSIToUnicode( pKiller->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
		else
			V_wcsncpy(wszPlayerName, pszDefaultKiller, sizeof(wszPlayerName));

		int iWide, iTall;
		surface()->GetTextSize(m_hDeathFrameLarge, wszPlayerName, iWide, iTall);
		surface()->DrawSetTextFont(m_hDeathFrameLarge);
		surface()->DrawSetTextPos(flKilledByXOffset + iWidth/2 - iWide/2, 20 + surface()->GetFontTall(m_hDeathFrameLarge));
		surface()->DrawSetTextColor(Color(255, 255, 255, 255));
		surface()->DrawPrintText(wszPlayerName, wcslen(wszPlayerName));
	}

	float flWeaponXOffset = RemapValClamped(Bias(RemapValClamped(gpGlobals->curtime, m_flKilledByStartTime + 0.2f, m_flKilledByStartTime + 0.2f + flLerpTime, 0, 1), 0.8f), 0, 1, iWidth*2/3, 0);

	wchar_t* pszWeaponOfChoice = g_pVGuiLocalize->Find("#DA_DeathFrame_WeaponOfChoice");

	if (pKiller && pszWeaponOfChoice && pKiller->GetActiveSDKWeapon())
	{
		wchar_t* pszWeaponName = g_pVGuiLocalize->Find(pKiller->GetActiveSDKWeapon()->GetPrintName());

		if (pPlayer->WasKilledByGrenade())
			pszWeaponName = g_pVGuiLocalize->Find("#DA_Weapon_Grenade");

		if (pPlayer->WasKilledByBrawl())
			pszWeaponName = g_pVGuiLocalize->Find("#DA_Weapon_Brawl");

		if (pszWeaponName)
		{
			std::wstring sMessage = std::wstring(pszWeaponOfChoice) + pszWeaponName;

			int iWide, iTall;
			surface()->GetTextSize(m_hDeathFrameMedium, sMessage.c_str(), iWide, iTall);
			surface()->DrawSetTextFont(m_hDeathFrameMedium);
			surface()->DrawSetTextPos(flWeaponXOffset + iWidth/2 - iWide/2, iHeight - iTall - 20);
			surface()->DrawSetTextColor(Color(255, 255, 255, 255));
			surface()->DrawPrintText(sMessage.c_str(), sMessage.length());
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::OnTick( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Reset effects on level load/shutdown
//-----------------------------------------------------------------------------
void C_SDKRootPanel::LevelInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::LevelShutdown( void )
{
}


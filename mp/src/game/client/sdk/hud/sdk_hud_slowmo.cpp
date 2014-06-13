//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include "ihudlcd.h"
#include "c_sdk_player.h"
#include "sdkviewport.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudSlowMo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudSlowMo, CHudNumericDisplay );

public:
	CHudSlowMo( const char *pElementName );
	virtual void ApplySchemeSettings( IScheme *scheme );
	void Init( void );
	void VidInit( void );
	void Reset();
	void Blink();

	void SetSlowMo(float flSlowMo);

	virtual void Paint();
	virtual void PaintBackground() {};

protected:
	virtual void OnThink();

	void UpdatePlayerSlowMo( C_BasePlayer *player );

	CPanelAnimationVarAliasType( float, watch_xpos, "watch_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, watch_ypos, "watch_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, watch_tall, "watch_tall", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, watch_wide, "watch_wide", "10", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hHintFont, "HintFont", "Default" );

private:
	CHudTexture* m_pBackground;
	CHudTexture* m_pBackgroundSuper;

	float m_flBlink;
};

DECLARE_HUDELEMENT( CHudSlowMo );

void __MsgFunc_BlinkTimer( bf_read &msg )
{
	CHudSlowMo* pSlowMo = (CHudSlowMo*)gHUD.FindElement("CHudSlowMo");

	pSlowMo->Blink();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSlowMo::CHudSlowMo( const char *pElementName ) : BaseClass(NULL, "HudSlowMo"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	m_pBackground = NULL;
	m_pBackgroundSuper = NULL;

	SetIsTime(true);

	m_flBlink = -1;
}

void CHudSlowMo::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pBackground = gHUD.GetIcon("slowmo_background");
	m_pBackgroundSuper = gHUD.GetIcon("slowmo_super_background");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::Init( void )
{
	HOOK_MESSAGE( BlinkTimer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::VidInit( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudSlowMo::Reset()
{
	BaseClass::Reset();

//	SetLabelText(g_pVGuiLocalize->Find("#DA_HUD_Slowmo"));

	UpdatePlayerSlowMo( C_BasePlayer::GetLocalPlayer() );

	m_flBlink = -1;
}

void CHudSlowMo::Blink()
{
	m_flBlink = gpGlobals->curtime;
}

void CHudSlowMo::UpdatePlayerSlowMo( C_BasePlayer *player )
{
	SetPaintEnabled(false);
	SetPaintBackgroundEnabled(false);

	C_SDKPlayer* pSDKPlayer = ToSDKPlayer(player);

	if (!pSDKPlayer)
		return;

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetSlowMo(pSDKPlayer->GetSlowMoSeconds());
}

ConVar hud_slowmo_needle_lerp("hud_slowmo_needle_lerp", "10", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CHudSlowMo::OnThink()
{
	if (m_flBlink > 0 && gpGlobals->curtime - m_flBlink < 2)
	{
		if (fmod(gpGlobals->curtime - m_flBlink, 0.5f) < 0.25f)
			m_bDisplayValue = false;
		else
			m_bDisplayValue = true;
	}
	else
		m_bDisplayValue = true;

	UpdatePlayerSlowMo( C_BasePlayer::GetLocalPlayer() );

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (pPlayer->GetSlowMoTime() > 0)
		SetDisplayValue((pPlayer->GetSlowMoTime() - gpGlobals->curtime)*60);
}

void CHudSlowMo::SetSlowMo(float flSlowMo)
{
	SetDisplayValue(flSlowMo*60);
}

extern float Oscillate(float flTime, float flLength);

void CHudSlowMo::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	if (pPlayer->HasSuperSlowMo() || pPlayer->m_Shared.m_bSuperSkill)
	{
		SetFgColor(Color(255, 190, 20, 255));

		if (m_pBackgroundSuper)
			m_pBackgroundSuper->DrawSelf( watch_xpos, watch_ypos, watch_wide, watch_tall, Color(255, 255, 255, 255) );
	}
	else
	{
		SetFgColor(Color(40, 200, 14, 255));

		if (m_pBackground)
			m_pBackground->DrawSelf( watch_xpos, watch_ypos, watch_wide, watch_tall, Color(255, 255, 255, 255) );
	}

	BaseClass::Paint();

	wchar_t* pszActivate = g_pVGuiLocalize->Find("#DA_HUD_Slowmo_Activate");

	if (pszActivate && pPlayer->GetSlowMoTime() == 0 && pPlayer->GetSlowMoSeconds() >= 2)
	{
#define WSTRLEN 512
		// replace any key references with bound keys
		wchar_t wszHintLabel[WSTRLEN];
		UTIL_ReplaceKeyBindings( pszActivate, 0, wszHintLabel, sizeof( wszHintLabel ) );

		int iTextWide, iTextTall;
		surface()->GetTextSize( m_hHintFont, wszHintLabel, iTextWide, iTextTall );

		surface()->DrawSetTextPos( GetWide()/2 - iTextWide/2, RemapVal(fabs(sin(gpGlobals->curtime*3)), 0, 1, watch_ypos - surface()->GetFontTall(m_hHintFont), 0) );
		surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
		surface()->DrawSetTextFont( m_hHintFont );
		surface()->DrawUnicodeString( wszHintLabel, vgui::FONT_DRAW_NONADDITIVE );
	}
}

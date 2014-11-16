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
#include "c_da_player.h"
#include "daviewport.h"

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
	CHudTexture* m_pClock;

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

	m_pClock = NULL;

	SetIsTime(true);

	m_flBlink = -1;
}

void CHudSlowMo::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pClock = gHUD.GetIcon("hud_clock");
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

	C_DAPlayer* pDAPlayer = ToDAPlayer(player);

	if (!pDAPlayer)
		return;

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetSlowMo(pDAPlayer->GetSlowMoSeconds());
}

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

	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
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
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	surface()->DrawSetColor( Color(0, 0, 0, 180) );
	surface()->DrawFilledRect( 0, 0, watch_xpos, watch_ypos );

	Color clrClock;
	if (pPlayer->HasSuperSlowMo() || pPlayer->m_Shared.m_bSuperSkill)
		clrClock = Color(255, 190, 20, 255);
	else if (pPlayer->GetSlowMoTime() > 0)
		clrClock = Color(107, 157, 244, 255);
	else
		clrClock = Color(255, 255, 255, 255);

	if (m_flBlink > 0 && gpGlobals->curtime - m_flBlink < 2)
		clrClock = Color(255, 0, 0, 255);

	SetFgColor(clrClock);

	float flMargin = 5;

	if (!m_pClock)
		return;

	float flClockHeight = watch_ypos - flMargin*2;
	float flClockWidth = flClockHeight * m_pClock->Width() / m_pClock->Height();

	m_pClock->DrawSelf(flMargin, flMargin, flClockWidth, flClockHeight, clrClock);

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

		surface()->DrawSetTextPos( RemapVal(fabs(sin(gpGlobals->curtime*3)), 0, 1, watch_xpos, watch_xpos + 10), watch_ypos/2 - surface()->GetFontTall(m_hHintFont)/2 );
		surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
		surface()->DrawSetTextFont( m_hHintFont );
		surface()->DrawUnicodeString( wszHintLabel, vgui::FONT_DRAW_NONADDITIVE );
	}

	/*wchar_t* pszSlowmo = g_pVGuiLocalize->Find("#DA_HUD_Slowmo");
	if (pszSlowmo)
	{
		int iTextWide, iTextTall;
		surface()->GetTextSize( m_hHintFont, pszSlowmo, iTextWide, iTextTall );

		surface()->DrawSetTextPos( watch_xpos/2 - iTextWide/2, watch_ypos );
		surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
		surface()->DrawSetTextFont( m_hHintFont );
		surface()->DrawUnicodeString( pszSlowmo, vgui::FONT_DRAW_NONADDITIVE );
	}*/
}

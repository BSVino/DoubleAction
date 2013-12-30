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
};

DECLARE_HUDELEMENT( CHudSlowMo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSlowMo::CHudSlowMo( const char *pElementName ) : BaseClass(NULL, "HudSlowMo"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	m_pBackground = NULL;

	SetIsTime(true);
}

void CHudSlowMo::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pBackground = gHUD.GetIcon("slowmo_background");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::Init( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::VidInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudSlowMo::Reset()
{
	BaseClass::Reset();

//	SetLabelText(g_pVGuiLocalize->Find("#DA_HUD_Slowmo"));

	UpdatePlayerSlowMo( C_BasePlayer::GetLocalPlayer() );
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

	if (m_pBackground)
		m_pBackground->DrawSelf( watch_xpos, watch_ypos, watch_wide, watch_tall, Color(255, 255, 255, 255) );

	BaseClass::Paint();

	wchar_t* pszActivate = g_pVGuiLocalize->Find("#DA_HUD_Slowmo_Activate");

	if (pszActivate && pPlayer->GetSlowMoTime() == 0 && pPlayer->GetSlowMoSeconds() >= 2)
	{
#define WSTRLEN 100
		wchar_t wszHintLabel[WSTRLEN];
		V_wcsncpy(wszHintLabel, pszActivate, WSTRLEN);

		int iTextWide, iTextTall;
		surface()->GetTextSize( m_hHintFont, wszHintLabel, iTextWide, iTextTall );

		surface()->DrawSetTextPos( GetWide()/2 - iTextWide/2, RemapVal(fabs(sin(gpGlobals->curtime*3)), 0, 1, watch_ypos - surface()->GetFontTall(m_hHintFont), 0) );
		surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
		surface()->DrawSetTextFont( m_hHintFont );
		surface()->DrawUnicodeString( wszHintLabel, vgui::FONT_DRAW_NONADDITIVE );
	}
}

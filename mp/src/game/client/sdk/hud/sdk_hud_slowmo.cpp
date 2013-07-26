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

private:
	float m_flSlowMo;
	float m_flSlowMoGoal;

	CHudTexture* m_pBackground;
	CHudTexture* m_pRed;
	CHudTexture* m_pNeedle;
};

DECLARE_HUDELEMENT( CHudSlowMo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSlowMo::CHudSlowMo( const char *pElementName ) : BaseClass(NULL, "HudSlowMo"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	m_pBackground = m_pRed = m_pNeedle = NULL;
}

void CHudSlowMo::ApplySchemeSettings( IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pBackground = gHUD.GetIcon("slowmo_background");
	m_pRed = gHUD.GetIcon("slowmo_red");
	m_pNeedle = gHUD.GetIcon("slowmo_needle");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::Init( void )
{
	m_flSlowMo = m_flSlowMoGoal = -1;
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

	m_flSlowMo = m_flSlowMoGoal = 0;

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

	float m_flSlowMoGoal = pPlayer->GetSlowMoSeconds();
	if (pPlayer->GetSlowMoTime() > 0)
		m_flSlowMoGoal = pPlayer->GetSlowMoTime() - gpGlobals->curtime;

	m_flSlowMo = Approach(m_flSlowMoGoal, m_flSlowMo, hud_slowmo_needle_lerp.GetFloat() * gpGlobals->frametime * fabs(m_flSlowMoGoal - m_flSlowMo));
}

void CHudSlowMo::SetSlowMo(float flSlowMo)
{
	if (flSlowMo != m_flSlowMoGoal)
	{
		m_flSlowMoGoal = flSlowMo;
	}

	SetDisplayValue(flSlowMo);
}

extern float Oscillate(float flTime, float flLength);

void CHudSlowMo::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	int iWide = GetWide();
	int iTall = GetTall();

	if (m_pBackground)
		m_pBackground->DrawSelf( 0, 0, iWide, iTall, Color(255, 255, 255, 255) );

	if (pPlayer->GetSlowMoTime() > 0 && m_pRed)
		m_pRed->DrawSelf( 0, 0, iWide, iTall, Color(255, 255, 255, 255 * Oscillate(gpGlobals->curtime, 0.5f)) );

	if (m_pNeedle)
		SDKViewport::DrawPolygon(m_pNeedle, 0, iTall-iWide, iWide, iWide, m_flSlowMo * 90);
}

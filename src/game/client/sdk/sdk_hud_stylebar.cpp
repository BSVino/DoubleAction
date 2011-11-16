//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "c_sdk_player.h"

#include "ConVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHudStyleBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudStyleBar, vgui::Panel );

public:
	CHudStyleBar( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint();

private:
	int		m_flStyle;

	CPanelAnimationVarAliasType( float, m_flGap, "Gap", "2", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudStyleBar );

CHudStyleBar::CHudStyleBar( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudStyleBar" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

void CHudStyleBar::Init()
{
	Reset();
}

void CHudStyleBar::Reset()
{
	m_flStyle = 0;
}

void CHudStyleBar::VidInit()
{
	Reset();
}

void CHudStyleBar::OnThink()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	float flStyle = pPlayer->GetActionPoints();

	if ( flStyle == m_flStyle )
		return;

	m_flStyle = flStyle;
}

inline float Oscillate(float flTime, float flLength)
{
	return fabs(RemapVal(fmod(flTime, flLength), 0, flLength, -1, 1));
}

void CHudStyleBar::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	Color clrBar;
	if (m_flStyle > 25)
		clrBar = gHUD.m_clrCaution;
	else
		clrBar = gHUD.m_clrNormal;

	if (pPlayer->IsActionAbilityActive())
		clrBar.SetColor(clrBar.r(), clrBar.g(), clrBar.b(), Oscillate(gpGlobals->curtime, 1)*255);

	surface()->DrawSetColor( clrBar );

	float flPercent = m_flStyle / 100.0f;
	int iWidth, iHeight;
	GetSize(iWidth, iHeight);
	float flBarHeight = iHeight - m_flGap*2;
	surface()->DrawFilledRect( m_flGap, m_flGap + flBarHeight*(1-flPercent), iWidth - m_flGap, flBarHeight );
}

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

#include "da.h"

class CHudNotices : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudNotices, vgui::Panel );

public:
	CHudNotices( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint();
	virtual void PaintBackground() {};

	void	MsgFunc_Notice( bf_read &msg );

private:
	int		m_flStyle;

	CHudTexture* m_apNotices[TOTAL_NOTICES];

	float    m_flStartTime;
	notice_t m_eNotice;
};

DECLARE_HUDELEMENT( CHudNotices );
DECLARE_HUD_MESSAGE( CHudNotices, Notice );

CHudNotices::CHudNotices( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudNotices" )
{
	memset(m_apNotices, 0, sizeof(m_apNotices));

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

void CHudNotices::Init()
{
	Reset();

	HOOK_HUD_MESSAGE( CHudNotices, Notice );
}

ConVar hud_noticetime("hud_noticetime", "3", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long notices stick around, in seconds.");

void CHudNotices::MsgFunc_Notice( bf_read &msg )
{
	notice_t eNotice = (notice_t)msg.ReadLong();

	// Don't overwrite a more important notice with a slowmo notice. NOBODY CARES, SLOWMO. GO AWAY.
	if (gpGlobals->curtime < m_flStartTime + hud_noticetime.GetFloat() && eNotice == NOTICE_SLOMO)
		return;

	m_flStartTime = gpGlobals->curtime;
	m_eNotice = eNotice;
}

void CHudNotices::Reset()
{
	m_flStyle = 0;
}

void CHudNotices::VidInit()
{
	Reset();
}

void CHudNotices::OnThink()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;
}

void CHudNotices::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	if (!m_apNotices[NOTICE_MARKSMAN])
	{
		m_apNotices[NOTICE_MARKSMAN] = gHUD.GetIcon("notice_marksman");
		m_apNotices[NOTICE_ADRENALINE] = gHUD.GetIcon("notice_adrenaline");
		m_apNotices[NOTICE_SUPERSLO] = gHUD.GetIcon("notice_superslo");
		m_apNotices[NOTICE_TOUGHTOKILL] = gHUD.GetIcon("notice_toughtokill");
		m_apNotices[NOTICE_SLOMO] = gHUD.GetIcon("notice_slowmo");
		m_apNotices[NOTICE_STYLESTREAK] = gHUD.GetIcon("notice_stylestreak");
	}

	if (m_eNotice < 0)
		return;

	if (m_eNotice >= TOTAL_NOTICES)
		return;

	if (!m_apNotices[m_eNotice])
		return;

	if (gpGlobals->curtime > m_flStartTime + hud_noticetime.GetFloat())
		return;

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	auto* pTexture = m_apNotices[m_eNotice];

	float flScale = 1;
	if (m_eNotice == NOTICE_SLOMO)
		flScale = 0.6f;

	float flSlideInTime = m_flStartTime + 0.3f;
	float flEndTime = m_flStartTime + hud_noticetime.GetFloat();
	float flSlideOutTime = flEndTime - 0.5f;

	float flSlideInXStart = 100;
	float flSlideInXEnd = 80;

	float flSlideIn;
	float flAlpha = 1;
	if (gpGlobals->curtime < flSlideInTime)
	{
		float flSlideInRamp = RemapValClamped(gpGlobals->curtime, m_flStartTime, flSlideInTime, 0, 1);
		flSlideIn = RemapVal(Bias(flSlideInRamp, 0.75), 0, 1, iWidth, flSlideInXStart);
		flAlpha = RemapValClamped(gpGlobals->curtime, m_flStartTime, flSlideInTime, 0, 1);
	}
	else if (gpGlobals->curtime < flSlideOutTime)
	{
		flSlideIn = RemapVal(gpGlobals->curtime, flSlideInTime, flSlideOutTime, flSlideInXStart, flSlideInXEnd);
		flAlpha = 1;
	}
	else
	{
		float flSlideInRamp = RemapValClamped(gpGlobals->curtime, flSlideOutTime, flEndTime, 0, 1);
		flSlideIn = RemapVal(Bias(flSlideInRamp, 0.25), 0, 1, flSlideInXEnd, -300);
		flAlpha = RemapValClamped(gpGlobals->curtime, flSlideOutTime, flEndTime, 1, 0);
	}

	pTexture->DrawSelf(
		flSlideIn, iHeight/2,
		pTexture->EffectiveWidth(flScale), pTexture->EffectiveHeight(flScale),
		Color(255, 255, 255, 255 * flAlpha)
		);
}

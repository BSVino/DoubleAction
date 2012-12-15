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

static const char* g_apszAnnouncementTextures[] = {
	"announcement_cool",
	"announcement_stylish",
	"announcement_dive",
	"announcement_dive_kill",
	"announcement_slide",
	"announcement_slide_kill",
	"announcement_stunt",
	"announcement_stunt_kill",
	"announcement_through_wall",
	"announcement_last_bullet",
	"announcement_long_range",
	"announcement_long_range_kill",
	"announcement_double_kill",
	"announcement_grenade",
	"announcement_grenade_kill",
	"announcement_slowmo_kill",
	"announcement_tacticool",
	"announcement_brawl",
	"announcement_brawl_kill",
};

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
	virtual void PaintBackground() {};

	void	MsgFunc_StyleAnnouncement( bf_read &msg );

private:
	int		m_flStyle;

	CPanelAnimationVarAliasType( float, m_flGap, "Gap", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "10", "proportional_float" );

	CHudTexture* m_apAnnouncements[TOTAL_ANNOUNCEMENTS];

	class CAnnouncement
	{
	public:
		float          m_flStartTime;
		announcement_t m_eAnnouncement;
		style_point_t  m_ePointStyle;
		float          m_flBarPosition;
	};
	CUtlLinkedList<CAnnouncement> m_aAnnouncements;
};

DECLARE_HUDELEMENT( CHudStyleBar );
DECLARE_HUD_MESSAGE( CHudStyleBar, StyleAnnouncement );

CHudStyleBar::CHudStyleBar( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudStyleBar" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

void CHudStyleBar::Init()
{
	for (int i = 0; i < TOTAL_ANNOUNCEMENTS; i++)
		m_apAnnouncements[i] = gHUD.GetIcon(g_apszAnnouncementTextures[i]);

	Reset();

	HOOK_HUD_MESSAGE( CHudStyleBar, StyleAnnouncement );
}

void CHudStyleBar::MsgFunc_StyleAnnouncement( bf_read &msg )
{
	announcement_t eAnnouncement = (announcement_t)msg.ReadLong();
	style_point_t ePointStyle = (style_point_t)msg.ReadByte();
	float flBar = msg.ReadFloat();

	CAnnouncement oAnnouncement;
	oAnnouncement.m_flStartTime = gpGlobals->curtime;
	oAnnouncement.m_eAnnouncement = eAnnouncement;
	oAnnouncement.m_ePointStyle = ePointStyle;
	oAnnouncement.m_flBarPosition = flBar;

	m_aAnnouncements.AddToTail(oAnnouncement);
}

void CHudStyleBar::Reset()
{
	m_flStyle = 0;
}

void CHudStyleBar::VidInit()
{
	Reset();
}

ConVar hud_announcementtime("hud_announcementtime", "3", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long announcements stick around, in seconds.");

void CHudStyleBar::OnThink()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	float flStyle = pPlayer->GetStylePoints();

	if ( flStyle == m_flStyle )
		return;

	m_flStyle = flStyle;

	int iNext;
	for (int i = m_aAnnouncements.Head(); i != m_aAnnouncements.InvalidIndex(); i = iNext)
	{
		iNext = m_aAnnouncements.Next( i );
		if (gpGlobals->curtime > m_aAnnouncements[i].m_flStartTime + hud_announcementtime.GetFloat())
			m_aAnnouncements.Remove(i);
	}
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

	if (!pPlayer->IsAlive())
		return;

	for (int i = 0; i < TOTAL_ANNOUNCEMENTS; i++)
	{
		if (!m_apAnnouncements[i])
			m_apAnnouncements[i] = gHUD.GetIcon(g_apszAnnouncementTextures[i]);
	}

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	surface()->DrawSetColor( Color(0, 0, 0, 100) );
	surface()->DrawFilledRect( iWidth - m_flBarWidth, 0, iWidth, iHeight );

	Color clrBar;
	if (pPlayer->IsStyleSkillActive())
	{
		clrBar = gHUD.m_clrCaution;
		clrBar.SetColor(clrBar.r(), clrBar.g(), clrBar.b(), Oscillate(gpGlobals->curtime, 1)*255);
	}
	else
		clrBar = gHUD.m_clrNormal;

	surface()->DrawSetColor( clrBar );

	float flPercent;
	if (pPlayer->IsStyleSkillActive())
		flPercent = min(pPlayer->GetStyleSkillCharge() / 100, 1);
	else
		flPercent = m_flStyle * 4 / 100.0f;

	float flBarHeight = iHeight - m_flGap*2;
	surface()->DrawFilledRect( iWidth - m_flBarWidth + m_flGap, m_flGap + flBarHeight*(1-flPercent), iWidth - m_flGap, flBarHeight );

	int iNext;
	for (int i = m_aAnnouncements.Head(); i != m_aAnnouncements.InvalidIndex(); i = iNext)
	{
		iNext = m_aAnnouncements.Next( i );

		CAnnouncement* pAnnouncement = &m_aAnnouncements[i];

		if (pAnnouncement->m_eAnnouncement < 0)
			continue;

		if (pAnnouncement->m_eAnnouncement >= TOTAL_ANNOUNCEMENTS)
			continue;

		if (!m_apAnnouncements[pAnnouncement->m_eAnnouncement])
			continue;

		auto* pTexture = m_apAnnouncements[pAnnouncement->m_eAnnouncement];

		float flScale = 1.2;
		if (pAnnouncement->m_ePointStyle == STYLE_POINT_LARGE)
			flScale = 0.8f;
		else if (pAnnouncement->m_ePointStyle == STYLE_POINT_SMALL)
			flScale = 0.6f;

		float flSlideIn = RemapValClamped(gpGlobals->curtime, pAnnouncement->m_flStartTime, pAnnouncement->m_flStartTime + 0.3f, 0, 1);
		flSlideIn = RemapVal(Bias(flSlideIn, 0.75), 0, 1, 0, iWidth - m_flBarWidth - pTexture->EffectiveWidth(flScale));

		float flEndTime = pAnnouncement->m_flStartTime + hud_announcementtime.GetFloat();

		float flAlpha = 1;
		if (gpGlobals->curtime < pAnnouncement->m_flStartTime + 0.3f)
			flAlpha = RemapValClamped(gpGlobals->curtime, pAnnouncement->m_flStartTime, pAnnouncement->m_flStartTime + 0.3f, 0, 1);
		else if (gpGlobals->curtime > flEndTime-0.5f)
			flAlpha = RemapValClamped(gpGlobals->curtime, flEndTime-0.5f, flEndTime, 1, 0);

		pTexture->DrawSelf(
			flSlideIn, RemapValClamped(pAnnouncement->m_flBarPosition, 0, 1, m_flGap + flBarHeight - pTexture->EffectiveHeight(flScale), m_flGap),
			pTexture->EffectiveWidth(flScale), pTexture->EffectiveHeight(flScale),
			Color(255, 255, 255, 255 * flAlpha)
		);
	}
}

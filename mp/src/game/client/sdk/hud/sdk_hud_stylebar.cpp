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

#include "sdk_hud_stylebar.h"

#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

#include "hud_numericdisplay.h"
#include "c_sdk_player.h"
#include "sdk_hud_ammo.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const char* g_apszAnnouncementTextures[] = {
	"announcement_none",
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
	"announcement_divepunch",
	"announcement_slidepunch",
	"announcement_headshot",
	"announcement_point_blank",
	"announcement_execution",
};

DECLARE_HUDELEMENT( CHudStyleBar );
DECLARE_HUD_MESSAGE( CHudStyleBar, StyleAnnouncement );

CHudStyleBar::CHudStyleBar( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudStyleBar" )
{
	memset(m_apActiveSkillIcons, 0, sizeof(m_apActiveSkillIcons));

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
	float flPoints = msg.ReadFloat();

	CAnnouncement oAnnouncement;
	oAnnouncement.m_flStartTime = gpGlobals->curtime;
	oAnnouncement.m_eAnnouncement = eAnnouncement;
	oAnnouncement.m_ePointStyle = ePointStyle;
	oAnnouncement.m_flBarPosition = flBar;

	if (m_aAnnouncements.Count())
	{
		// If a few at a time come in off the wire don't throw them all up at once. Subsequent ones should come in with a delay.

		float flDelay = 0.02f;
		if (gpGlobals->curtime < m_aAnnouncements[m_aAnnouncements.Tail()].m_flStartTime + flDelay)
			oAnnouncement.m_flStartTime = m_aAnnouncements[m_aAnnouncements.Tail()].m_flStartTime + flDelay;
	}

	oAnnouncement.m_flStylePoints = flPoints;

	m_aAnnouncements.AddToTail(oAnnouncement);
}

void CHudStyleBar::Notice(notice_t eNotice)
{
	if (eNotice == NOTICE_MARKSMAN || eNotice == NOTICE_TROLL || eNotice == NOTICE_BOUNCER
		|| eNotice == NOTICE_ATHLETIC || eNotice == NOTICE_SUPERSLO || eNotice == NOTICE_RESILIENT)
	{
		m_flStyleIconLerpStart = gpGlobals->curtime;
	}
}

float CHudStyleBar::GetIconX()
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), 350);
}

float CHudStyleBar::GetIconY()
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), 400);
}

float CHudStyleBar::GetIconW()
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), 60);
}

float CHudStyleBar::GetIconH()
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), 60);
}

void CHudStyleBar::Reset()
{
	m_flCurrentStyle = m_flGoalStyle = 0;
	m_aAnnouncements.RemoveAll();
	m_flStyleIconLerpStart = 0;
}

void CHudStyleBar::VidInit()
{
	Reset();
}

void CHudStyleBar::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	int wide, tall;

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	wide = screenWide; 
	tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	const char *xstr = inResourceData->GetString( "barxpos", NULL );
	const char *ystr = inResourceData->GetString( "barypos", NULL );
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
		m_flElementXPos = atoi(xstr);

		// scale the x up to our screen co-ords
		if ( IsProportional() )
			m_flElementXPos = scheme()->GetProportionalScaledValueEx(GetScheme(), m_flElementXPos);

		// now correct the alignment
		if (bRightAligned)
			m_flElementXPos = screenWide - m_flElementXPos; 
		else if (bCenterAligned)
			m_flElementXPos = (screenWide / 2) + m_flElementXPos;
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

		m_flElementYPos = atoi(ystr);
		if (IsProportional())
			// scale the y up to our screen co-ords
			m_flElementYPos = scheme()->GetProportionalScaledValueEx(GetScheme(), m_flElementYPos);

		// now correct the alignment
		if (bBottomAligned)
			m_flElementYPos = screenTall - m_flElementYPos; 
		else if (bCenterAligned)
			m_flElementYPos = (screenTall / 2) + m_flElementYPos;
	}

	const char *wstr = inResourceData->GetString( "barwide", NULL );
	if ( wstr )
	{
		bool bFull = false;
		if (wstr[0] == 'f' || wstr[0] == 'F')
		{
			bFull = true;
			wstr++;
		}

		m_flElementWide = atof(wstr);
		if ( IsProportional() )
		{
			// scale the x and y up to our screen co-ords
			m_flElementWide = scheme()->GetProportionalScaledValueEx(GetScheme(), m_flElementWide);
		}

		// now correct the alignment
		if (bFull)
			m_flElementWide = screenWide - m_flElementWide; 
	}

	m_flElementTall = inResourceData->GetInt( "bartall", tall );
	if ( IsProportional() )
	{
		// scale the x and y up to our screen co-ords
		m_flElementTall = scheme()->GetProportionalScaledValueEx(GetScheme(), m_flElementTall);
	}
}

ConVar hud_announcementtime("hud_announcementtime", "3", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long announcements stick around, in seconds.");

void CHudStyleBar::OnThink()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	float flStyle = pPlayer->GetStylePoints();

	m_flGoalStyle = flStyle;
	m_flCurrentStyle = Approach(m_flGoalStyle, m_flCurrentStyle, RemapValClamped(fabs((float)m_flGoalStyle-m_flCurrentStyle), 0, 10, gpGlobals->frametime*10, gpGlobals->frametime*100));

	int iNext;
	for (int i = m_aAnnouncements.Head(); i != m_aAnnouncements.InvalidIndex(); i = iNext)
	{
		iNext = m_aAnnouncements.Next( i );
		if (gpGlobals->curtime > m_aAnnouncements[i].m_flStartTime + hud_announcementtime.GetFloat())
			m_aAnnouncements.Remove(i);
	}
}

float Oscillate(float flTime, float flLength)
{
	return fabs(RemapVal(fmod(flTime, flLength), 0, flLength, -1, 1));
}

extern ConVar da_stylemeteractivationcost;

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

	if (!m_apActiveSkillIcons[SKILL_BOUNCER])
	{
		m_apActiveSkillIcons[SKILL_BOUNCER] = gHUD.GetIcon("bouncer");
		m_apActiveSkillIcons[SKILL_ATHLETIC] = gHUD.GetIcon("athletic");
		m_apActiveSkillIcons[SKILL_REFLEXES] = gHUD.GetIcon("reflexes");
		m_apActiveSkillIcons[SKILL_MARKSMAN] = gHUD.GetIcon("marksman");
		m_apActiveSkillIcons[SKILL_TROLL] = gHUD.GetIcon("troll");
		m_apActiveSkillIcons[SKILL_SUPER] = gHUD.GetIcon("super");
		//m_apActiveSkillIcons[SKILL_RESILIENT] = gHUD.GetIcon("resilient");

		m_pGoldStar = gHUD.GetIcon("star_gold");
		m_pSilverStar = gHUD.GetIcon("star_silver");
		m_pBronzeStar = gHUD.GetIcon("star_bronze");

		m_pBriefcase = gHUD.GetIcon("briefcase");
	}

	CHudTexture* pStyleTexture = m_apActiveSkillIcons[pPlayer->m_Shared.m_iStyleSkill];

	if (pPlayer->m_Shared.m_bSuperSkill)
		pStyleTexture = m_apActiveSkillIcons[SKILL_SUPER];

	float flStyleTextureWidth = 0;
	float flStyleTextureHeight = 0;

	if (pStyleTexture)
	{
		flStyleTextureWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), pStyleTexture->EffectiveWidth(0.5f)) * 0.8f;
		flStyleTextureHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), pStyleTexture->EffectiveHeight(0.5f)) * 0.8f;
	}

	int iWidth = m_flElementWide;
	int iHeight = m_flElementTall;

	int flScreenWide, flScreenTall;
	surface()->GetScreenSize(flScreenWide, flScreenTall);

	float flBarLeft = m_flElementXPos + iWidth - flStyleTextureWidth/2 - m_flBarWidth/2;
	surface()->DrawSetColor( Color(0, 0, 0, 100) );
	surface()->DrawFilledRect(
		flBarLeft,
		m_flElementYPos,
		m_flElementXPos + iWidth - flStyleTextureWidth/2 + m_flBarWidth/2,
		m_flElementYPos + iHeight - flStyleTextureHeight - m_flGap
		);

	Color clrBar;
	if (pPlayer->IsStyleSkillActive())
	{
		clrBar = gHUD.m_clrCaution;
		clrBar.SetColor(clrBar.r(), clrBar.g(), clrBar.b(), RemapValClamped(Gain(Oscillate(gpGlobals->curtime, 1), 0.7f), 0, 1, 0.1f, 1)*255);
	}
	else
	{
		int r = Lerp(m_flCurrentStyle/da_stylemeteractivationcost.GetFloat(), gHUD.m_clrNormal.r()/2, gHUD.m_clrNormal.r());
		int g = Lerp(m_flCurrentStyle/da_stylemeteractivationcost.GetFloat(), gHUD.m_clrNormal.g()/2, gHUD.m_clrNormal.g());
		int b = Lerp(m_flCurrentStyle/da_stylemeteractivationcost.GetFloat(), gHUD.m_clrNormal.b()/2, gHUD.m_clrNormal.b());
		clrBar.SetColor(r, g, b, gHUD.m_clrNormal.a());
	}

	surface()->DrawSetColor( clrBar );

	float flPercent;
	if (pPlayer->IsStyleSkillActive())
		flPercent = min(pPlayer->GetStyleSkillCharge() / 100, 1);
	else
		flPercent = m_flCurrentStyle / da_stylemeteractivationcost.GetFloat();

	float flBarHeight = iHeight - flStyleTextureHeight - m_flGap*2;

	int iBarLeft = m_flElementXPos + iWidth - flStyleTextureWidth/2 - m_flBarWidth/2 + m_flGap;
	int iBarRight = m_flElementXPos + iWidth - flStyleTextureWidth/2 + m_flBarWidth/2 - m_flGap;
	surface()->DrawFilledRect( iBarLeft, m_flElementYPos + m_flGap + flBarHeight*(1-flPercent), iBarRight, m_flElementYPos + flBarHeight );

	float flPulseTime = 0.6f;
	float flAlphaRamp = RemapValClamped(fmod(gpGlobals->curtime, flPulseTime), 0, flPulseTime, 0, 1);
	int iPulseAlpha = RemapValClamped(Bias(flAlphaRamp, 0.1f), 0, flPulseTime, 50, 10);
	surface()->DrawSetColor( Color(255, 255, 255, iPulseAlpha) );

	float flWidth = 0.1f;
	float flBottom = RemapValClamped(fmod(gpGlobals->curtime, flPulseTime), 0, flPulseTime, -flWidth, 1);
	float flTop = flBottom + flWidth;

	flBottom = clamp(flBottom, 0, flPercent);
	flTop = clamp(flTop, 0, flPercent);

	surface()->DrawFilledRect( iBarLeft + 2, m_flElementYPos + m_flGap + flBarHeight*(1-flTop), iBarRight - 2, m_flElementYPos + flBarHeight*(1-flBottom) );

	if (pStyleTexture)
	{
		float flAlpha = 1;
		float flNonRed = 0;
		float flRed = 0;

		if (pPlayer->IsStyleSkillActive())
		{
			flAlpha = RemapValClamped(Gain(Oscillate(gpGlobals->curtime, 1), 0.7f), 0, 1, 0.5f, 1);
			flNonRed = Gain(1-Oscillate(gpGlobals->curtime, 1), 0.7f);
			flRed = 1;
		}

		float flBarIconX = m_flElementXPos + iWidth - flStyleTextureWidth;
		float flBarIconY = m_flElementYPos + iHeight - flStyleTextureHeight;
		pStyleTexture->DrawSelf(
				flBarIconX, flBarIconY,
				flStyleTextureWidth, flStyleTextureHeight,
				Color(
					Lerp(flRed, 255, gHUD.m_clrCaution.r()),
					Lerp(flNonRed, 255, gHUD.m_clrCaution.g()),
					Lerp(flNonRed, 255, gHUD.m_clrCaution.b()),
					255*flAlpha
				)
			);

		float flIconXPos = GetIconX();
		float flIconYPos = GetIconY();
		float flIconWide = GetIconW();
		float flIconTall = GetIconH();

		if (pPlayer->IsStyleSkillActive())
		{
			pStyleTexture->DrawSelf(
					flIconXPos, flIconYPos,
					flIconWide, flIconTall,
					Color( 255, 255, 255, 255 )
				);
		}

		if (m_pBriefcase && pPlayer->HasBriefcase())
		{
			m_pBriefcase->DrawSelf(
					flIconXPos + flIconWide + 20, flIconYPos,
					flIconWide, flIconTall,
					Color( 255, 255, 255, 255 )
				);
		}

		float flLerpTime = 1.0f;
		float flFadeTime = 1.0f;
		if (m_flStyleIconLerpStart && gpGlobals->curtime < m_flStyleIconLerpStart + flLerpTime + flFadeTime)
		{
			float flRamp = Bias(RemapValClamped(gpGlobals->curtime, m_flStyleIconLerpStart, m_flStyleIconLerpStart + flLerpTime, 0, 1), 0.7f);

			if (pPlayer->m_Shared.m_iStyleSkill == SKILL_TROLL)
			{
				CHudAmmo* pElement = dynamic_cast<CHudAmmo*>(gHUD.FindElement("CHudAmmo"));
				if (pElement)
				{
					Vector4D vecGrenade = pElement->GetGrenadePosition(pPlayer->GetAmmoCount("grenades")-1);
					flIconXPos = vecGrenade.x;
					flIconYPos = vecGrenade.y;
					flIconWide = vecGrenade.z;
					flIconTall = vecGrenade.w;
				}
			}
			else if (pPlayer->m_Shared.m_iStyleSkill == SKILL_REFLEXES)
			{
				CHudNumericDisplay* pElement = dynamic_cast<CHudNumericDisplay*>(gHUD.FindElement("CHudSlowMo"));
				if (pElement)
				{
					int x, y;
					pElement->GetPos(x, y);
					flIconXPos = x;
					flIconYPos = y;

					int w, h;
					pElement->GetSize(w, h);
					flIconWide = flIconTall = (w + h)/2;
				}
			}

			float flAlpha = RemapValClamped(gpGlobals->curtime, m_flStyleIconLerpStart + flLerpTime, m_flStyleIconLerpStart + flLerpTime + flFadeTime, 1, 0);

			pStyleTexture->DrawSelf(
					RemapVal(flRamp, 0, 1, flBarIconX, flIconXPos), RemapVal(flRamp, 0, 1, flBarIconY, flIconYPos),
					RemapVal(flRamp, 0, 1, flStyleTextureWidth, flIconWide), RemapVal(flRamp, 0, 1, flStyleTextureHeight, flIconTall),
					Color( gHUD.m_clrCaution.r(), gHUD.m_clrCaution.g(), gHUD.m_clrCaution.b(), 255*flAlpha )
				);
		}
	}

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

		if (gpGlobals->curtime < pAnnouncement->m_flStartTime)
			continue;

		CHudTexture* pTexture = m_apAnnouncements[pAnnouncement->m_eAnnouncement];

		float flScale = 1.2;
		if (pAnnouncement->m_ePointStyle == STYLE_POINT_LARGE)
			flScale = 0.8f;
		else if (pAnnouncement->m_ePointStyle == STYLE_POINT_SMALL)
			flScale = 0.6f;

		float flStarWidth = pTexture->EffectiveHeight(flScale);

		float flSlideInLerp = RemapValClamped(gpGlobals->curtime, pAnnouncement->m_flStartTime, pAnnouncement->m_flStartTime + 0.3f, 0, 1);
		float flSlideIn = RemapVal(Bias(flSlideInLerp, 0.75), 0, 1, -1000, 0);

		float flEndTime = pAnnouncement->m_flStartTime + hud_announcementtime.GetFloat();

		float flAlpha = 1;
		if (gpGlobals->curtime < pAnnouncement->m_flStartTime + 0.3f)
			flAlpha = RemapValClamped(gpGlobals->curtime, pAnnouncement->m_flStartTime, pAnnouncement->m_flStartTime + 0.3f, 0, 1);
		else if (gpGlobals->curtime > flEndTime-0.5f)
			flAlpha = RemapValClamped(gpGlobals->curtime, flEndTime-0.5f, flEndTime, 1, 0);

		pTexture->DrawSelf(
			flBarLeft - pTexture->EffectiveWidth(flScale) - flStarWidth + flSlideIn, m_flElementYPos + RemapValClamped(pAnnouncement->m_flBarPosition, 0, 1, m_flGap + flBarHeight - pTexture->EffectiveHeight(flScale), m_flGap),
			pTexture->EffectiveWidth(flScale), pTexture->EffectiveHeight(flScale),
			Color(255, 255, 255, 255 * flAlpha)
		);

		int iGold, iSilver, iBronze;
		C_SDKPlayer::GetStyleStars(pAnnouncement->m_flStylePoints, iGold, iSilver, iBronze);

		CHudTexture* pStarTexture;
		int iStars;
		if (iGold)
		{
			pStarTexture = m_pGoldStar;
			iStars = iGold;
		}
		else if (iSilver)
		{
			pStarTexture = m_pSilverStar;
			iStars = iSilver;
		}
		else
		{
			pStarTexture = m_pBronzeStar;
			iStars = iBronze;
		}

		if (pStarTexture)
		{
			pStarTexture->DrawSelf(
				flBarLeft - flStarWidth + flSlideIn, m_flElementYPos + RemapValClamped(pAnnouncement->m_flBarPosition, 0, 1, m_flGap + flBarHeight - pTexture->EffectiveHeight(flScale), m_flGap),
				flStarWidth, flStarWidth,
				Color(255, 255, 255, 255 * flAlpha)
			);
		}
	}
}

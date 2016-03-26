//========= Copyright Valve Corporation, All rights reserved. ============//
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

#include "c_da_player.h"
#include "c_da_player_resource.h"

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	virtual void ApplySchemeSettings( IScheme *scheme );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
			void MsgFunc_Damage( bf_read &msg );

	virtual void Paint();
	virtual void PaintBackground() {};

	float GetLerpedHealth();

	CPanelAnimationVarAliasType( float, m_flHealthLerpTime, "HealthLerpTime", "0.25", "float" );

private:
	int     m_iOldHealth;
	int     m_iHealth;
	float   m_flLastHealthChange;

	int		m_bitsDamage;

	CHudTexture* m_pHeart;
};

DECLARE_HUDELEMENT( CHudHealth );
DECLARE_HUD_MESSAGE( CHudHealth, Damage );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudHealth")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudHealth::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	m_pHeart = gHUD.GetIcon("hud_heart");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	m_iHealth = INIT_HEALTH;
	m_iOldHealth = INIT_HEALTH;
	m_flLastHealthChange = -1;
	m_bitsDamage = 0;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_HEALTH");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"HEALTH");
	}
	SetDisplayValue(m_iHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		newHealth = MAX( local->GetHealth(), 0 );
	}

	// Only update the fade if we've changed health
	if ( newHealth == m_iHealth )
	{
		return;
	}

	m_iOldHealth = GetLerpedHealth();
	m_iHealth = newHealth;
	m_flLastHealthChange = gpGlobals->curtime;

	if ( m_iHealth >= 20 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if ( m_iHealth > 0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}

	SetDisplayValue(m_iHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage( bf_read &msg )
{
	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
	{
		if ( damageTaken > 0 )
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
		}
	}
}

void CHudHealth::Paint()
{
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	surface()->DrawSetColor( Color(0, 0, 0, 180) );
	surface()->DrawFilledRect( 0, 0, 1000, 1000 );

//	if (pPlayer->IsStyleSkillActive())
//		clrBar.SetColor(clrBar.r(), clrBar.g(), clrBar.b(), Oscillate(gpGlobals->curtime, 1)*255);

	int iElementBuffer = 12;	// The entire element gets a stencil crop, so leave some buffer room on the outsides so that the blood splatters can overflow.
	int iWidth, iHeight;
	GetSize(iWidth, iHeight);
	iWidth -= iElementBuffer*2;
	iHeight -= iElementBuffer*2;

	float flMargin = 5;

	float flHeartHeight = GetTall() - flMargin*2;

	if (m_pHeart)
		m_pHeart->DrawSelf(flMargin, flMargin, flHeartHeight, flHeartHeight, Color(255, 255, 255, 255));

	float flBarWidth = GetWide() - flMargin*3 - flHeartHeight;
	float flBarHeight = 4;

	float flHurtLerpTime = RemapValClamped(m_iOldHealth - m_iHealth, 10, 50, m_flHealthLerpTime, m_flHealthLerpTime*3);
	float flHurtAlpha = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + flHurtLerpTime, 1, 0);
	float flHurtPercent = Clamp((float)m_iOldHealth/100, 0.0f, 1.0f);

	float flHealthPercent = Clamp((float)GetLerpedHealth()/100, 0.0f, 1.0f);

	if (flHurtAlpha && flHealthPercent < flHurtPercent)
	{
		surface()->DrawSetColor( Color(255, 0, 0, flHurtAlpha*255) );
		surface()->DrawFilledRect( flMargin*2 + flHeartHeight + flHealthPercent * flBarWidth, GetTall()/2-flBarHeight/2, flMargin*2 + flHeartHeight + flHurtPercent * flBarWidth, GetTall()/2+flBarHeight/2 );
	}

	surface()->DrawSetColor( Color(255, 255, 255, 255) );
	surface()->DrawFilledRect( flMargin*2 + flHeartHeight, GetTall()/2-flBarHeight/2, flMargin*2 + flHeartHeight + flHealthPercent * flBarWidth, GetTall()/2+flBarHeight/2 );

	float flOverhealPercent = RemapValClamped((float)GetLerpedHealth(), 100, 150, 0.0f, 1.0f);
	if (flOverhealPercent)
	{
		surface()->DrawSetColor( Color(255, 190, 20, 128) );
		surface()->DrawFilledRect( flMargin*2 + flHeartHeight, flMargin, flMargin*2 + flHeartHeight + flOverhealPercent * flBarWidth, GetTall() - flMargin );
	}
}

float CHudHealth::GetLerpedHealth()
{
	float flHealthLerp = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + m_flHealthLerpTime, 0, 1);
	flHealthLerp = Bias(flHealthLerp, 0.8f);
	return RemapValClamped(flHealthLerp, 0, 1, m_iOldHealth, m_iHealth);
}

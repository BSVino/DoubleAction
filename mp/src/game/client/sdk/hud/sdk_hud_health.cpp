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

#include "c_sdk_player.h"
#include "c_sdk_player_resource.h"

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

#include "sdk_hud_health.h"

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

	float GetLerpedHealth() const;

	CPanelAnimationVarAliasType( float, m_flHealthLerpTime, "HealthLerpTime", "0.25", "float" );

private:
	int     m_iOldHealth;
	int     m_iHealth;
	float   m_flLastHealthChange;

	int		m_bitsDamage;

	CHudTexture* m_pHeart;

	CHealthWidget m_oHealthWidget;
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
	m_oHealthWidget.Reset();

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
	m_oHealthWidget.SetPlayer(ToSDKPlayer(C_BasePlayer::GetLocalPlayer()));
	m_oHealthWidget.Update();

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
	int damageTaken = msg.ReadByte();	// health

	Vector vecFrom;

	msg.ReadBitVec3Coord(vecFrom);

	Assert(msg.GetNumBytesLeft() == 0);

	// Actually took damage?
	if ( damageTaken > 0 )
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
	}
}

void CHudHealth::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	float flMargin = 5;
	float flHeartHeight = GetTall() - flMargin * 2;

	m_oHealthWidget.Paint(m_pHeart, flHeartHeight, 0, 0, iWidth, iHeight, 1.0f, Color(255, 255, 255, 255));
}

float CHudHealth::GetLerpedHealth() const
{
	float flHealthLerp = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + m_flHealthLerpTime, 0, 1);
	flHealthLerp = Bias(flHealthLerp, 0.8f);
	return RemapValClamped(flHealthLerp, 0, 1, m_iOldHealth, m_iHealth);
}

void CHealthWidget::SetPlayer(CSDKPlayer* pPlayer)
{
	m_hPlayer = pPlayer;
}

void CHealthWidget::Reset()
{
	m_iOldHealth = 0;
	m_iHealth = 0;
	m_flLastHealthChange = 0.0f;
}

void CHealthWidget::Update()
{
	int iNewHealth = 0;
	CSDKPlayer* pPlayer = m_hPlayer;
	if (pPlayer)
	{
		// Never below zero
		iNewHealth = max(pPlayer->GetHealth(), 0);
	}

	// Only update the fade if we've changed health
	if (iNewHealth == m_iHealth)
	{
		return;
	}

	m_iOldHealth = GetLerpedHealth();
	m_iHealth = iNewHealth;
	m_flLastHealthChange = gpGlobals->curtime;
}

void CHealthWidget::Paint(CHudTexture* pIconTexture, float flIconHeight, int x, int y, int iWidth, int iHeight, float flAlpha, const Color& color)
{
	surface()->DrawSetColor(Color(0, 0, 0, color.a() * 0.7f * flAlpha));
	surface()->DrawFilledRect(x, y, x + iWidth, y + iHeight);

	float flMargin = 5;
	float flLeftMargin = flMargin * (flIconHeight == 0 ? 1.5f : 2.0f);

	if (pIconTexture)
	{
		pIconTexture->DrawSelf(x + flMargin, y + flMargin, flIconHeight, flIconHeight, Color(color.r(), color.g(), color.b(), color.a() * flAlpha));
	}

	float flBarWidth = iWidth - flMargin * 3 - flIconHeight;
	float flBarHeight = 4;

	float flHurtLerpTime = RemapValClamped(m_iOldHealth - m_iHealth, 10, 50, m_flHealthLerpTime, m_flHealthLerpTime * 3);
	float flHurtAlpha = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + flHurtLerpTime, 1, 0);
	float flHurtPercent = Clamp((float)m_iOldHealth / 100, 0.0f, 1.0f);

	float flHealthPercent = Clamp((float)GetLerpedHealth() / 100, 0.0f, 1.0f);

	if (flHurtAlpha && flHealthPercent < flHurtPercent)
	{
		float flHurtBarHeight = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + flHurtLerpTime, iHeight, flBarHeight);

		surface()->DrawSetColor(Color(color.r(), 0, 0, flHurtAlpha * flAlpha * color.a()));
		surface()->DrawFilledRect(x + flLeftMargin + flIconHeight + flHealthPercent * flBarWidth, y + iHeight / 2 - flHurtBarHeight / 2, x + flLeftMargin + flIconHeight + flHurtPercent * flBarWidth, y + iHeight / 2 + flHurtBarHeight / 2);
	}

	surface()->DrawSetColor(Color(color.r(), color.g(), color.b(), color.a() * flAlpha));
	surface()->DrawFilledRect(x + flLeftMargin + flIconHeight, y + iHeight / 2 - flBarHeight / 2, x + flLeftMargin + flIconHeight + flHealthPercent * flBarWidth, y + iHeight / 2 + flBarHeight / 2);

	float flOverhealPercent = RemapValClamped((float)GetLerpedHealth(), 100, 150, 0.0f, 1.0f);
	if (flOverhealPercent)
	{
		surface()->DrawSetColor(Color(255, 190, 20, 128 * flAlpha));
		surface()->DrawFilledRect(x + flLeftMargin + flIconHeight, y + flMargin, x + flLeftMargin + flIconHeight + flOverhealPercent * flBarWidth, y + iHeight - flMargin);
	}
}

float CHealthWidget::GetLerpedHealth() const
{
	float flHealthLerp = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + m_flHealthLerpTime, 0, 1);
	flHealthLerp = Bias(flHealthLerp, 0.8f);
	return RemapValClamped(flHealthLerp, 0, 1, m_iOldHealth, m_iHealth);
}

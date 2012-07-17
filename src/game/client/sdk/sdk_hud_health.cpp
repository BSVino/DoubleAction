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

#include "c_sdk_player.h"
#include "c_sdk_player_resource.h"

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "ConVar.h"

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
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
			void MsgFunc_Damage( bf_read &msg );

	virtual void Paint();
	virtual void PaintBackground() {};

private:
	// old variables
	int		m_iHealth;
	
	int		m_bitsDamage;

	CHudTexture	*m_pBackground;
	CHudTexture	*m_pFill;
	CHudTexture	*m_pSplat;
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	m_pBackground = gHUD.GetIcon("health_background");
	m_pFill = gHUD.GetIcon("health_fill");
	m_pSplat = gHUD.GetIcon("health_splat");

	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	m_iHealth		= INIT_HEALTH;
	m_bitsDamage	= 0;

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
		newHealth = max( local->GetHealth(), 0 );
	}

	// Only update the fade if we've changed health
	if ( newHealth == m_iHealth )
	{
		return;
	}

	m_iHealth = newHealth;

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
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	if (!m_pBackground)
		m_pBackground = gHUD.GetIcon("health_background");
	if (!m_pFill)
		m_pFill = gHUD.GetIcon("health_fill");
	if (!m_pSplat)
		m_pSplat = gHUD.GetIcon("health_splat");

//	if (pPlayer->IsStyleSkillActive())
//		clrBar.SetColor(clrBar.r(), clrBar.g(), clrBar.b(), Oscillate(gpGlobals->curtime, 1)*255);

	int iElementBuffer = 12;	// The entire element gets a stencil crop, so leave some buffer room on the outsides so that the blood splatters can overflow.
	int iWidth, iHeight;
	GetSize(iWidth, iHeight);
	iWidth -= iElementBuffer*2;
	iHeight -= iElementBuffer*2;

	int	iFillWidth = m_pFill->Width();
	int	iFillHeight = m_pFill->Height();

	C_SDK_PlayerResource *pSDKPR = SDKGameResources();
	int iMaxHealth = pSDKPR->GetMaxHealth(pPlayer->entindex());

	int iSectionsPerRow = 4;
	float flBars = (float)m_iHealth*iSectionsPerRow/100;
	int iSections = max(flBars + (fmod(flBars, 1) == 0?0:1), iSectionsPerRow);

	int iBuffer = 5;
	int iSectionWidth = (iWidth - iBuffer*(iSectionsPerRow-1))/iSectionsPerRow;

	for (int i = 0; i < iSections; i++)
	{
		int iXOffset = 0;
		int iYOffset = 0;

		if (i < iSectionsPerRow)
		{
			iXOffset = iElementBuffer;
			iYOffset = iHeight/2 + iElementBuffer;
		}
		else
		{
			iXOffset = -iSectionsPerRow*(iSectionWidth+iBuffer) + iElementBuffer;
			iYOffset = iElementBuffer;
		}

		if (m_iHealth > i*100/iSectionsPerRow)
		{
			if (m_pBackground)
				m_pBackground->DrawSelf( iXOffset + i*(iSectionWidth+iBuffer), iYOffset, iSectionWidth, iHeight/2, Color(255, 255, 255, 255) );
			if (m_pFill)
			{
				float flFillWidth = RemapValClamped(m_iHealth, i*100/iSectionsPerRow, (i+1)*100/iSectionsPerRow, 0, 1);
				m_pFill->DrawSelfCropped( iXOffset + i*(iSectionWidth+iBuffer), iYOffset, 0, 0, iFillWidth*flFillWidth, iFillHeight, iSectionWidth*flFillWidth, iHeight/2, Color(255, 255, 255, 255) );
			}
		}
		else if (m_pSplat)
		{
			if (m_iHealth < iMaxHealth)
				m_pSplat->DrawSelf( 0 + i*(iSectionWidth+iBuffer), iHeight/2, iSectionWidth + iElementBuffer*2, iHeight/2 + iElementBuffer*2, Color(255, 255, 255, 255) );
		}
	}
}

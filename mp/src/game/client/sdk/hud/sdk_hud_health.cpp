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
	virtual void ApplySchemeSettings( IScheme *scheme );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
			void MsgFunc_Damage( bf_read &msg );

	virtual void Paint();
	virtual void PaintBackground() {};

	int GetLerpedHealth();

	CPanelAnimationVarAliasType( float, m_flHealthLerpTime, "HealthLerpTime", "0.25", "float" );

private:
	int     m_iOldHealth;
	int     m_iHealth;
	float   m_flLastHealthChange;

	int		m_bitsDamage;

	CHudTexture* m_pBackground;
	CHudTexture* m_pFill;
	CHudTexture* m_pSplat1;
	CHudTexture* m_pSplat2;
	CHudTexture* m_pCap;
	CHudTexture* m_pArmor;
	CHudTexture* m_pArmorGlow;
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

	m_pBackground = gHUD.GetIcon("health_background");
	m_pFill = gHUD.GetIcon("health_fill");
	m_pSplat1 = gHUD.GetIcon("health_splat1");
	m_pSplat2 = gHUD.GetIcon("health_splat2");
	m_pCap = gHUD.GetIcon("health_cap");
	m_pArmor = gHUD.GetIcon("health_armor");
	m_pArmorGlow = gHUD.GetIcon("health_armor_glow");
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
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

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

		int iLerpedHealth = GetLerpedHealth();

		if (iLerpedHealth > i*100/iSectionsPerRow)
		{
			float flFillWidth = RemapValClamped(iLerpedHealth, i*100/iSectionsPerRow, (i+1)*100/iSectionsPerRow, 0, 1);

			if (i < iSectionsPerRow)
			{
				if (m_pBackground)
					m_pBackground->DrawSelf( iXOffset + i*(iSectionWidth+iBuffer), iYOffset, iSectionWidth, iHeight/2, Color(255, 255, 255, 255) );

				if (m_pFill)
				{
					m_pFill->DrawSelfCropped( iXOffset + i*(iSectionWidth+iBuffer), iYOffset, 0, 0, iFillWidth*flFillWidth, iFillHeight, iSectionWidth*flFillWidth, iHeight/2, Color(255, 255, 255, 255) );

					if (flFillWidth < 0.95f)
						m_pCap->DrawSelf(iXOffset + i*(iSectionWidth+iBuffer) + iSectionWidth*flFillWidth - m_pCap->Width()/2, iYOffset, m_pCap->Width(), iHeight/2, Color(255, 255, 255, 255) );
				}
			}
			else
			{
				if (m_pArmor)
					m_pArmor->DrawSelfCropped( iXOffset + i*(iSectionWidth+iBuffer), iYOffset, 0, 0, iFillWidth*flFillWidth, iFillHeight, iSectionWidth*flFillWidth, iHeight/2, Color(255, 255, 255, 255) );
			}

			if (m_iOldHealth > m_iHealth + 5 && flFillWidth < 1 && gpGlobals->curtime < m_flLastHealthChange + m_flHealthLerpTime && m_pArmorGlow)
			{
				float flAlpha = Gain(RemapValClamped(gpGlobals->curtime, m_flLastHealthChange + m_flHealthLerpTime/2, m_flLastHealthChange + m_flHealthLerpTime, 1, 0), 0.7f);
				m_pArmorGlow->DrawSelf( iXOffset + i*(iSectionWidth+iBuffer) + iSectionWidth*flFillWidth - m_pArmorGlow->Width()/2, iYOffset, m_pArmorGlow->Width(), iHeight/2, Color(255, 255, 255, 255*flAlpha) );
			}
		}
		else if (m_pSplat1 && m_pSplat2)
		{
			if (m_iHealth < iMaxHealth)
			{
				CHudTexture* pSplat;
				switch (i)
				{
				default:
				case 0:
				case 2:
					pSplat = m_pSplat1;
					break;

				case 1:
				case 3:
					pSplat = m_pSplat2;
					break;
				}

				int iSplatWidth = iSectionWidth + iElementBuffer*2;
				int iSplatHeight = iHeight/2 + iElementBuffer*2;
				int iSplatSize = min(iSplatWidth, iSplatHeight);

				pSplat->DrawSelf( 0 + i*(iSectionWidth+iBuffer) + iSplatWidth/2 - iSplatSize/2, iHeight/2 + iSplatHeight/2 - iSplatSize/2, iSplatSize, iSplatSize, Color(255, 255, 255, 255) );
			}
		}
	}
}

int CHudHealth::GetLerpedHealth()
{
	float flHealthLerp = RemapValClamped(gpGlobals->curtime, m_flLastHealthChange, m_flLastHealthChange + m_flHealthLerpTime, 0, 1);
	flHealthLerp = Bias(flHealthLerp, 0.8f);
	return RemapValClamped(flHealthLerp, 0, 1, m_iOldHealth, m_iHealth);
}

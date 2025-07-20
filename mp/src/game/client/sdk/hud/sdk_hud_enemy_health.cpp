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
// implementation of CHudEnemyHealth class
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
class CHudEnemyHealth : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudEnemyHealth, vgui::Panel);

public:
	CHudEnemyHealth(const char *pElementName);
	virtual void ApplySchemeSettings( IScheme *scheme );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

	virtual void Paint();
	virtual void PaintBackground() {};

private:
	CHealthWidget m_aHealthWidgets[MAX_PLAYERS];
	float m_aflWidgetAlphas[MAX_PLAYERS];
	Vector m_avecLastKnownLocations[MAX_PLAYERS];

	float m_flPlayerMax = 0.0f;
};

DECLARE_HUDELEMENT(CHudEnemyHealth);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudEnemyHealth::CHudEnemyHealth(const char *pElementName) :
CHudElement(pElementName),
BaseClass(NULL, "HudEnemyHealth")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);

	for (int k = 0; k < MAX_PLAYERS; k++)
	{
		m_aflWidgetAlphas[k] = 0;
	}
}

void CHudEnemyHealth::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudEnemyHealth::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudEnemyHealth::Reset()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudEnemyHealth::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudEnemyHealth::OnThink()
{
}

static ConVar da_enemy_health_bar_width("da_enemy_health_bar_width", "100", 0, "Enemy health bar width");
static ConVar da_enemy_health_bar_height("da_enemy_health_bar_height", "10", 0, "Enemy health bar height");
static ConVar da_enemy_health_bar_max_distance("da_enemy_health_bar_max_distance", "1000", 0, "Enemy health bar max render distance");

void CHudEnemyHealth::Paint()
{
	for (int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(iClient));

		if (pPlayer == C_SDKPlayer::GetLocalSDKPlayer())
		{
			continue;
		}

		m_aHealthWidgets[iClient].SetPlayer(pPlayer);
		m_aHealthWidgets[iClient].Update();

		bool bHide = false;
		if (pPlayer)
		{
			if (m_flPlayerMax == 0.0f)
			{
				Vector mins, maxs;
				pPlayer->GetRenderBounds(mins, maxs);

				m_flPlayerMax = (maxs.z - mins.z) / 2;
			}

			Vector vecWorldSpaceCenter = pPlayer->WorldSpaceCenter();
			m_avecLastKnownLocations[iClient] = vecWorldSpaceCenter;

			trace_t tr;
			UTIL_TraceLine(CurrentViewOrigin(), m_avecLastKnownLocations[iClient], MASK_BLOCKLOS, pPlayer, COLLISION_GROUP_NONE, &tr);

			bHide = tr.fraction < 0.99f && tr.m_pEnt != C_SDKPlayer::GetLocalSDKPlayer();

			if (pPlayer->IsDormant())
			{
				bHide = true;
			}

			if (!pPlayer->IsAlive())
			{
				bHide = true;
			}

			if ((m_avecLastKnownLocations[iClient] - CSDKPlayer::GetLocalSDKPlayer()->WorldSpaceCenter()).Length() > da_enemy_health_bar_max_distance.GetFloat())
			{
				bHide = true;
			}
		}
		else
		{
			bHide = true;
		}

		m_aflWidgetAlphas[iClient] = Approach(bHide ? 0.0f : 1.0f, m_aflWidgetAlphas[iClient], gpGlobals->frametime * (bHide ? 5 : 2));

		if (m_aflWidgetAlphas[iClient] == 0.0f)
		{
			continue;
		}

		Vector vecAnchorPoint = m_avecLastKnownLocations[iClient];
		vecAnchorPoint.z += m_flPlayerMax;

		int x, y;
		GetVectorInHudSpace(vecAnchorPoint, x, y);

		m_aHealthWidgets[iClient].Paint(nullptr, 0, x - da_enemy_health_bar_width.GetInt() / 2, y - da_enemy_health_bar_height.GetInt(), da_enemy_health_bar_width.GetInt(), da_enemy_health_bar_height.GetInt(), m_aflWidgetAlphas[iClient], Color(255, 255, 255, 255));
	}
}

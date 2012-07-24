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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudSlowMo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudSlowMo, CHudNumericDisplay );

public:
	CHudSlowMo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset();

	void SetSlowMo(float flSlowMo);

protected:
	virtual void OnThink();

	void UpdatePlayerSlowMo( C_BasePlayer *player );

private:
	float	m_flSlowMo;
};

DECLARE_HUDELEMENT( CHudSlowMo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSlowMo::CHudSlowMo( const char *pElementName ) : BaseClass(NULL, "HudSlowMo"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSlowMo::Init( void )
{
	m_flSlowMo = -1;
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

	m_flSlowMo = 0;

	wchar_t *tempString = g_pVGuiLocalize->Find("#DAB_Hud_SlowMo");
	if (tempString)
	{
		wchar_t wzFinal[512] = L"";

		UTIL_ReplaceKeyBindings( tempString, 0, wzFinal, sizeof( wzFinal ) );

		SetLabelText(wzFinal);
	}
	else
	{
		SetLabelText(L"SLOWMO");
	}

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

void CHudSlowMo::OnThink()
{
	UpdatePlayerSlowMo( C_BasePlayer::GetLocalPlayer() );
}

void CHudSlowMo::SetSlowMo(float flSlowMo)
{
	if (flSlowMo != m_flSlowMo)
	{
		if (flSlowMo == 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SlowMoEmpty");
		else if (flSlowMo < m_flSlowMo)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SlowMoDecreased");
		else
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SlowMoIncreased");

		m_flSlowMo = flSlowMo;
	}

	SetDisplayValue(flSlowMo);
}

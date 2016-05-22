//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "hud.h"
#include "text_message.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"
#include "IEffects.h"
#include "hudelement.h"
#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "sdkviewport.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: HDU Damage indication
//-----------------------------------------------------------------------------
class CHudDamageIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDamageIndicator, vgui::Panel );

public:
	CHudDamageIndicator( const char *pElementName );

	void	Init( void );
	void	Reset( void );
	bool	ShouldDraw( void );
	virtual void OnThink();

	// Handler for our message
	void	MsgFunc_Damage( bf_read &msg );

private:
	void	Paint();
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	int     FindInactiveEvent();

private:
	Color	m_clrIndicator;

	CPanelAnimationVarAliasType( float, m_flDamageSize, "DamageSize", "20", "proportional_float" );

	CHudTexture	*icon_up;

	class CDamageEvent
	{
	public:
		CDamageEvent()
		{
			bActive = false;
		}

	public:
		bool   bActive;
		float  flDamage;
		float  flTimeDamaged;
		Vector vecSource;
	};

	CUtlVector<CDamageEvent> m_aDamageEvents;
};

DECLARE_HUDELEMENT( CHudDamageIndicator );
DECLARE_HUD_MESSAGE( CHudDamageIndicator, Damage );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudDamageIndicator::CHudDamageIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudDamageIndicator")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Reset( void )
{
	m_clrIndicator.SetColor( 250, 0, 0, 255 );
	m_aDamageEvents.RemoveAll();
}

void CHudDamageIndicator::Init( void )
{
	HOOK_HUD_MESSAGE( CHudDamageIndicator, Damage );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudDamageIndicator::ShouldDraw( void )
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	return true;
}

ConVar hud_damage_time("hud_damage_time", "1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long hud damage sticks around, in seconds.");

void CHudDamageIndicator::OnThink()
{
	BaseClass::OnThink();

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	for (int i = 0; i < m_aDamageEvents.Count(); i++)
	{
		if (!m_aDamageEvents[i].bActive)
			continue;

		if (pPlayer->GetCurrentTime() > m_aDamageEvents[i].flTimeDamaged + hud_damage_time.GetFloat())
			m_aDamageEvents[i].bActive = false;
	}
}

void CHudDamageIndicator::Paint()
{
	if ( !icon_up )
		icon_up = gHUD.GetIcon( "pain_up" );

	if ( !icon_up )
		return;

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (!pPlayer)
		return;

	matrix3x4_t mPlayer;
	AngleMatrix(MainViewAngles(), Vector(0, 0, 0), mPlayer);

	VMatrix mPlayerInvert;
	MatrixInverseTR(mPlayer, mPlayerInvert);

	VMatrix mToScreen(Vector(0, -1, 0), Vector(-1, 0, 0), Vector(0, 0, 1));

	VMatrix mPlayerToScreen = mToScreen * mPlayerInvert;

	int iWide = GetWide();
	int iTall = GetTall();

	float flRadius = (float)iTall/2 * 3/4;

	for (int i = 0; i < m_aDamageEvents.Count(); i++)
	{
		if (!m_aDamageEvents[i].bActive)
			continue;

		Vector vecDamageDirection = m_aDamageEvents[i].vecSource - pPlayer->GetAbsOrigin();

		Vector vecPlayerScreenDamageDirection = mPlayerToScreen.ApplyRotation(vecDamageDirection);
		Vector vecPlayerLocalDamageDirection = mPlayerInvert.ApplyRotation(vecDamageDirection);
		vecPlayerLocalDamageDirection.z = vecPlayerScreenDamageDirection.z = 0;
		vecPlayerScreenDamageDirection.NormalizeInPlace();
		vecPlayerLocalDamageDirection.NormalizeInPlace();

		Color clrIndicator = m_clrIndicator;
		float alphaIndicator = RemapVal(pPlayer->GetCurrentTime() - m_aDamageEvents[i].flTimeDamaged, 0, hud_damage_time.GetFloat(), 255, 0);
		clrIndicator.SetColor(clrIndicator.r(), clrIndicator.g(), clrIndicator.b(), alphaIndicator);

		float flSize = RemapVal(m_aDamageEvents[i].flDamage, 10, 60, m_flDamageSize, m_flDamageSize*3);
		flSize = RemapVal(flSize, 0, 768, 0, iTall);

		SDKViewport::DrawPolygon(icon_up,
			iWide/2 + vecPlayerScreenDamageDirection.x * flRadius - flSize/2, iTall/2 + vecPlayerScreenDamageDirection.y * flRadius - flSize/2,
			flSize, flSize, RAD2DEG(atan2(-vecPlayerLocalDamageDirection.y, vecPlayerLocalDamageDirection.x)), clrIndicator);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageIndicator::MsgFunc_Damage( bf_read &msg )
{
	int damageTaken	= msg.ReadByte();	

	Vector vecFrom;
	msg.ReadBitVec3Coord( vecFrom );

	if ( damageTaken <= 0 )
		return;

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pPlayer)
		return;

	int iEvent = FindInactiveEvent();
	m_aDamageEvents[iEvent].bActive = true;
	m_aDamageEvents[iEvent].flTimeDamaged = pPlayer->GetCurrentTime();
	m_aDamageEvents[iEvent].vecSource = vecFrom;
	m_aDamageEvents[iEvent].flDamage = damageTaken;
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudDamageIndicator::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	int wide, tall;
	GetHudSize(wide, tall);
	SetSize(wide, tall);
}


int CHudDamageIndicator::FindInactiveEvent()
{
	for (int i = 0; i < m_aDamageEvents.Count(); i++)
	{
		if (!m_aDamageEvents[i].bActive)
			return i;
	}

	return m_aDamageEvents.AddToTail();
}

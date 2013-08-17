//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "da_hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "ivrenderview.h"

#include "c_sdk_player.h"
#include "weapon_sdkbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar da_crosshair( "da_crosshair", "1", FCVAR_ARCHIVE );

using namespace vgui;

int ScreenTransform( const Vector& point, Vector& screen );

DECLARE_HUDELEMENT( CDAHudCrosshair );

CDAHudCrosshair::CDAHudCrosshair( const char *pElementName ) :
  CHudElement( pElementName ), BaseClass( NULL, "DAHudCrosshair" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_pCrosshair = 0;

	m_clrCrosshair = Color( 0, 0, 0, 0 );

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

void CDAHudCrosshair::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
	m_pObstructionCrosshair = gHUD.GetIcon("crosshair_obstruction");

	SetPaintBackgroundEnabled( false );

    SetSize( ScreenWidth(), ScreenHeight() );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CDAHudCrosshair::ShouldDraw( void )
{
	// OnThink isn't called when the thing isn't visible so force it to update.
	CalculateCrosshair();

	bool bNeedsDraw;

	if ( m_bHideCrosshair )
		return false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( pWeapon && !pWeapon->ShouldDrawCrosshair() )
		return false;

	ConVarRef cl_observercrosshair("cl_observercrosshair");

	// draw a crosshair only if alive or spectating in eye
	if ( IsX360() )
	{
		bNeedsDraw = m_pCrosshair && 
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			( !pPlayer->IsSuitEquipped() || g_pGameRules->IsMultiplayer() ) &&
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}
	else
	{
		bNeedsDraw = m_pCrosshair && 
			da_crosshair.GetInt() &&
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() && 
			g_pClientMode->ShouldDrawCrosshair() &&
			!( pPlayer->GetFlags() & FL_FROZEN ) &&
			( pPlayer->entindex() == render->GetViewEntity() ) &&
			!pPlayer->IsInVGuiInputMode() &&
			( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );
	}

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

void CDAHudCrosshair::CalculateCrosshair( void )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	CWeaponSDKBase *pWeapon = pPlayer->GetActiveSDKWeapon();

	if (!pWeapon)
	{
		ResetCrosshair();
		return;
	}

	if ( !pWeapon->ShouldDrawCrosshair() )
	{
		ResetCrosshair();
		return;
	}

	Color clrHUD = gHUD.m_clrNormal;
	clrHUD[3] = 255;
	Color white( 255, 255, 255, 255 );

	if ( pPlayer->m_Shared.GetAimIn() <= 0.5f || !pWeapon->HasAimInRecoilBonus() )
	{ 
		if ( pWeapon->GetWpnData().iconCrosshair )
			SetCrosshair( pWeapon->GetWpnData().iconCrosshair, clrHUD );
		else
			ResetCrosshair();
	}
	else
	{ 
		// zoomed crosshairs
		if ( pWeapon->GetWpnData().iconZoomedCrosshair )
			SetCrosshair( pWeapon->GetWpnData().iconZoomedCrosshair, white );
		else if ( pWeapon->GetWpnData().iconCrosshair )
			SetCrosshair( pWeapon->GetWpnData().iconCrosshair, clrHUD );
		else
			ResetCrosshair();
	}
}

void CDAHudCrosshair::Paint( void )
{
	if ( !m_pCrosshair )
		return;

	if ( !IsCurrentViewAccessAllowed() )
		return;

	m_curViewAngles = CurrentViewAngles();
	m_curViewOrigin = CurrentViewOrigin();

	float x, y;
	x = ScreenWidth()/2;
	y = ScreenHeight()/2;

	bool bObstruction = false;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (pPlayer && pPlayer->IsInThirdPerson() && m_pObstructionCrosshair)
	{
		Vector vecCamera = pPlayer->GetThirdPersonCameraPosition();

		Vector vecShootPosition = pPlayer->Weapon_ShootPosition();
		if (pPlayer->GetActiveSDKWeapon())
			vecShootPosition = pPlayer->GetActiveSDKWeapon()->GetShootPosition(pPlayer);

		Vector vecForward = pPlayer->GetThirdPersonCameraTarget() - vecShootPosition;

		// Now trace to see where the player will hit
		trace_t tr2;
		UTIL_TraceLine( vecShootPosition, vecShootPosition + vecForward * 1000, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr2 );

		//DebugDrawLine(vecCamera, tr.endpos, 255, 0, 0, true, 0.1f);
		//DebugDrawLine(vecShootPosition, tr2.endpos, 0, 0, 255, true, 0.1f);

		if ((tr2.endpos - pPlayer->GetThirdPersonCameraTarget()).LengthSqr() > 1)
		{
			bObstruction = true;

			Vector vecScreen;

			ScreenTransform( tr2.endpos, vecScreen );

			float x2, y2;

			x2 = ScreenWidth()/2 + 0.5 * vecScreen.x * ScreenWidth() + 0.5;
			y2 = ScreenHeight()/2 - 0.5 * vecScreen.y * ScreenHeight() + 0.5;

			m_pObstructionCrosshair->DrawSelf( 
					x2 - 0.5f * m_pObstructionCrosshair->Width(), 
					y2 - 0.5f * m_pObstructionCrosshair->Height(),
					m_clrCrosshair );
		}
	}

	m_pCrosshair->DrawSelf( 
			x - 0.5f * m_pCrosshair->Width(), 
			y - 0.5f * m_pCrosshair->Height(),
			bObstruction?Color(128, 128, 128, m_clrCrosshair.a()/2):m_clrCrosshair );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDAHudCrosshair::SetCrosshairAngle( const QAngle& angle )
{
	VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDAHudCrosshair::SetCrosshair( CHudTexture *texture, const Color& clr )
{
	m_pCrosshair = texture;
	m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CDAHudCrosshair::ResetCrosshair()
{
	SetCrosshair( m_pDefaultCrosshair, Color(255, 255, 255, 255) );
}

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
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include "IVRenderView.h"

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

	Color clr = gHUD.m_clrNormal;

	if ( pPlayer->GetFOV() >= 90 )
	{ 
		if ( pWeapon->GetWpnData().iconCrosshair )
		{
			clr[3] = 255;
			SetCrosshair( pWeapon->GetWpnData().iconCrosshair, clr );
		}
		else
		{
			ResetCrosshair();
		}
	}
	else
	{ 
		Color white( 255, 255, 255, 255 );

		// zoomed crosshairs
		if ( pWeapon->GetWpnData().iconZoomedCrosshair )
			SetCrosshair( pWeapon->GetWpnData().iconZoomedCrosshair, white );
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

	if (pPlayer && pPlayer->IsInThirdPerson())
	{
		Vector vecCamera = pPlayer->GetThirdPersonCameraPosition(pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles());

		Vector vecShoot;
		AngleVectors(pPlayer->EyeAngles(), &vecShoot);

		// Trace to see where the camera is pointing
		trace_t tr;
		UTIL_TraceLine( vecCamera, vecCamera + vecShoot * 99999, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr );

		Vector vecForward = tr.endpos - pPlayer->Weapon_ShootPosition();

		// Now trace to see where the player will hit
		trace_t tr2;
		UTIL_TraceLine( pPlayer->Weapon_ShootPosition(), pPlayer->Weapon_ShootPosition() + vecForward * 1000, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, pPlayer, COLLISION_GROUP_NONE, &tr2 );

		//DebugDrawLine(vecCamera, tr.endpos, 255, 0, 0, true, 0.1f);
		//DebugDrawLine(pPlayer->Weapon_ShootPosition(), tr2.endpos, 0, 0, 255, true, 0.1f);

		if ((tr2.endpos - tr.endpos).LengthSqr() > 1)
		{
			bObstruction = true;

			Vector vecScreen;

			ScreenTransform( tr2.endpos, vecScreen );

			float x2, y2;

			x2 = ScreenWidth()/2 + 0.5 * vecScreen.x * ScreenWidth() + 0.5;
			y2 = ScreenHeight()/2 - 0.5 * vecScreen.y * ScreenHeight() + 0.5;

			m_pCrosshair->DrawSelf( 
					x2 - 0.5f * m_pCrosshair->Width(), 
					y2 - 0.5f * m_pCrosshair->Height(),
					m_clrCrosshair );
		}
	}

	m_pCrosshair->DrawSelf( 
			x - 0.5f * m_pCrosshair->Width(), 
			y - 0.5f * m_pCrosshair->Height(),
			bObstruction?Color(m_clrCrosshair.r(), m_clrCrosshair.g()/2, m_clrCrosshair.b()/2, m_clrCrosshair.a()/2):m_clrCrosshair );
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
void CDAHudCrosshair::SetCrosshair( CHudTexture *texture, Color& clr )
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

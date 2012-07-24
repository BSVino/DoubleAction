//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "vgui_rootpanel_sdk.h"
#include "vgui/ivgui.h"
#include <vgui/ISurface.h>
#include "c_sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SDKRootPanel *g_pRootPanel = NULL;

using namespace vgui;

//-----------------------------------------------------------------------------
// Global functions.
//-----------------------------------------------------------------------------
void VGUI_CreateClientDLLRootPanel( void )
{
	g_pRootPanel = new C_SDKRootPanel( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
}

void VGUI_DestroyClientDLLRootPanel( void )
{
	delete g_pRootPanel;
	g_pRootPanel = NULL;
}

vgui::VPANEL VGui_GetClientDLLRootPanel( void )
{
	return g_pRootPanel->GetVPanel();
}


//-----------------------------------------------------------------------------
// C_SDKRootPanel implementation.
//-----------------------------------------------------------------------------
C_SDKRootPanel::C_SDKRootPanel( vgui::VPANEL parent )
	: BaseClass( NULL, "SDK Root Panel" )
{
	SetParent( parent );
	SetPaintEnabled( false );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( true );

	// Make it screen sized
	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );

	// Ask for OnTick messages
	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKRootPanel::~C_SDKRootPanel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::PaintBackground()
{
	BaseClass::PaintBackground();
	
	RenderLetterboxing();
}

//-----------------------------------------------------------------------------
// Purpose: For each panel effect, check if it wants to draw and draw it on
//  this panel/surface if so
//-----------------------------------------------------------------------------
void C_SDKRootPanel::RenderLetterboxing( void )
{
	C_SDKPlayer* pPlayer = ToSDKPlayer(C_BasePlayer::GetLocalPlayer());
	C_WeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	if (pPlayer && pPlayer->m_Shared.GetAimIn() > 0 && pWeapon && (pWeapon->FullAimIn() || pWeapon->HasAimInFireRateBonus() || pWeapon->HasAimInRecoilBonus()))
	{
		// Consider 50% fully letterboxed 
		float flRealAimIn = pPlayer->m_Shared.GetAimIn()*2;
		if (flRealAimIn > 1)
			flRealAimIn = 1;

		int iWidth = ScreenWidth();
		int iHeight = ScreenHeight();

		int i169Height = iWidth*9/16;
		if (i169Height >= iHeight - 50)
			i169Height = iHeight - 50;
		int iBarHeight = ((iHeight - i169Height)/2)*flRealAimIn;

		surface()->DrawSetColor(Color(0, 0, 0, 255*flRealAimIn));
		surface()->DrawFilledRect( 0, 0, ScreenWidth(), iBarHeight );
		surface()->DrawFilledRect( 0, ScreenHeight()-iBarHeight, ScreenWidth(), ScreenHeight() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::OnTick( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Reset effects on level load/shutdown
//-----------------------------------------------------------------------------
void C_SDKRootPanel::LevelInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKRootPanel::LevelShutdown( void )
{
}


//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include "hudelement.h"
#include <vgui_controls/Panel.h>

namespace vgui
{
	class IScheme;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CDAHudCrosshair : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CDAHudCrosshair, vgui::Panel );
public:
	CDAHudCrosshair( const char *pElementName );

	void			SetCrosshairAngle( const QAngle& angle );
	void			SetCrosshair( CHudTexture *texture, const Color& clr );
	void			ResetCrosshair();
	void			DrawCrosshair( void );
  	bool			HasCrosshair( void ) { return ( m_pCrosshair != NULL ); }
	bool			ShouldDraw();
	virtual void    CalculateCrosshair();

protected:
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint();

private:
	// Crosshair sprite and colors
	CHudTexture		*m_pCrosshair;
	CHudTexture		*m_pDefaultCrosshair;
	CHudTexture*    m_pObstructionCrosshair;
	Color			m_clrCrosshair;
	QAngle			m_vecCrossHairOffsetAngle;

	QAngle			m_curViewAngles;
	Vector			m_curViewOrigin;

	CPanelAnimationVar( bool, m_bHideCrosshair, "never_draw", "false" );

	float           m_flWatchAlpha;
	CPanelAnimationVar( vgui::HFont, m_hWatchFont, "WatchFont", "Default" );
};


// Enable/disable crosshair rendering.
extern ConVar da_crosshair;

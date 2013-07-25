//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef VGUI_ROOTPANEL_SDK_H
#define VGUI_ROOTPANEL_SDK_H
#ifdef _WIN32
#pragma once
#endif


#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>
#include "utlvector.h"


class CPanelEffect;


// Serial under of effect, for safe lookup
typedef unsigned int EFFECT_HANDLE;

//-----------------------------------------------------------------------------
// Purpose: Sits between engine and client .dll panels
//  Responsible for drawing screen overlays
//-----------------------------------------------------------------------------
class C_SDKRootPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
						C_SDKRootPanel( vgui::VPANEL parent );
	virtual				~C_SDKRootPanel( void );

	virtual void		PaintBackground();

	// Clear list of Panel Effects
	virtual void		LevelInit( void );
	virtual void		LevelShutdown( void );

	// Run effects and let them decide whether to remove themselves
	void				OnTick( void );

private:

	// Render all panel effects
	void		RenderLetterboxing( void );
	void		RenderDeathFrame( void );

	// List of current panel effects
	CUtlVector< CPanelEffect *> m_Effects;

	float       m_flKilledByStartTime;
	vgui::HFont m_hDeathFrameLarge;
	vgui::HFont m_hDeathFrameMedium;
	vgui::HFont m_hDeathFrameSmall;
};


#endif // VGUI_ROOTPANEL_SDK_H

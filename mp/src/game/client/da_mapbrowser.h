//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include <vguitextwindow.h>
#include "vgui_controls/ScrollableEditablePanel.h"
#include "vgui_controls/AnimationController.h"

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CMapBrowser : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CMapBrowser, vgui::Frame);

public:
	CMapBrowser();
	~CMapBrowser();

	virtual const char *GetName(void) { return "mapbrowser"; }

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};

	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }

	virtual void ShowPanel(bool bShow);

	virtual void Update();

	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	KeyValues* game_maps;
	vgui::HFont m_hSmallFont;
	vgui::AnimationController* vgui_animation_controller;

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnCommand(const char *command);

};

extern CMapBrowser* MapBrowser();

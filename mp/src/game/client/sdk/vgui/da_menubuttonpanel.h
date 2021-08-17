//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include <vguitextwindow.h>

#include "vgui_imagebutton.h"

class CButtonPanel : public vgui::EditablePanel, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CButtonPanel, vgui::EditablePanel);
	
public:
	CButtonPanel();
	~CButtonPanel();

	virtual const char *GetName( void ) { return "button_panel"; }

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }

	virtual void ShowPanel( bool bShow );

	virtual void OnThink();
	virtual void Update();

	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command);

	void CloseAll();

	vgui::ImageButton* m_pNews;
	vgui::ImageButton* m_pLeaderboard;
	vgui::ImageButton* m_pOptions;
	vgui::ImageButton* m_pMapBrowser;

	time_t m_iLatestNews;
};

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include <vguitextwindow.h>

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CSDKMapInfo : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CSDKMapInfo, vgui::Frame);
	
public:
	CSDKMapInfo(IViewPort *pViewPort);
	~CSDKMapInfo();

	virtual const char *GetName( void ) { return PANEL_INTRO; }

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }

	virtual void ShowPanel( bool bShow );

	virtual void MoveToCenterOfScreen();
	virtual void Update();

	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnCommand( const char *command);

	vgui::HTML* m_pMapMessage;
};

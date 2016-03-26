//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
class COptions : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(COptions, vgui::Frame);
	
public:
	COptions();
	~COptions();

	virtual const char *GetName( void ) { return "options"; }

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }

	virtual void ShowPanel( bool bShow );

	virtual void Update();

	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnCommand( const char *command);
};

extern COptions* Options();

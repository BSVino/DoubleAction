//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDKVIEWPORT_H
#define SDKVIEWPORT_H


#include "baseviewport.h"


using namespace vgui;

namespace vgui 
{
	class Panel;
}

class SDKViewport : public CBaseViewport
{

private:
	DECLARE_CLASS_SIMPLE( SDKViewport, CBaseViewport );

public:

	IViewPortPanel* CreatePanelByName(const char *szPanelName);
	void CreateDefaultPanels( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		
	int GetDeathMessageStartHeight( void );

	static bool DrawPolygon( CHudTexture* pTexture, Vector vecWorldPosition, float flWidth, float flHeight, float flRotation, const Color& c = Color(255, 255, 255, 255) );
	static bool DrawPolygon( CHudTexture* pTexture, float x, float y, float flWidth, float flHeight, float flRotation, const Color& c = Color(255, 255, 255, 255) );
};


#endif // SDKViewport_H

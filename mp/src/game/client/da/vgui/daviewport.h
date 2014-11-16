//========= Copyright Valve Corporation, All rights reserved. ============//
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
	SDKViewport();
	~SDKViewport();

public:
	IViewPortPanel* CreatePanelByName(const char *szPanelName);
	void CreateDefaultPanels( void );

	virtual void Start( IGameUIFuncs *pGameUIFuncs, IGameEventManager2 *pGameEventManager );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);

	virtual void RemoveAllPanels( void);

	int GetDeathMessageStartHeight( void );

	void StopMainMenuVideo();
	void StartMainMenuVideo();

	static bool DrawPolygon( CHudTexture* pTexture, Vector vecWorldPosition, float flWidth, float flHeight, float flRotation, const Color& c = Color(255, 255, 255, 255) );
	static bool DrawPolygon( CHudTexture* pTexture, float x, float y, float flWidth, float flHeight, float flRotation, const Color& c = Color(255, 255, 255, 255) );

private:
	class CDAMainMenu* m_pMainMenuPanel;
};


#endif // SDKViewport_H

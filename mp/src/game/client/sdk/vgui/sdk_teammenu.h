//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_TEAMMENU_H
#define SDK_TEAMMENU_H

#include "teammenu.h"

#if defined ( SDK_USE_TEAMS )
class CSDKTeamMenu : public CTeamMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKTeamMenu, CTeamMenu );
	// VGUI2 override
	void OnCommand( const char *command);
	// helper functions
	void SetVisibleButton(const char *textEntryName, bool state);

public:
	CSDKTeamMenu(IViewPort *pViewPort);
	virtual ~CSDKTeamMenu();

	void Update();
	virtual void SetVisible(bool state);
	void MoveToCenterOfScreen();

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	// rounded corners
	Color					 m_bgColor;
	Color					 m_borderColor;

};
#endif // SDK_USE_TEAMS

#endif //SDK_CLASSMENU_H
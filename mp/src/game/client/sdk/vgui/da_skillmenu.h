//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_SKILLMENU_H
#define SDK_SKILLMENU_H

#include <classmenu.h>
#include <filesystem.h>
#include <vgui/KeyCode.h>
#include "folder_gui.h"

class CSkillButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CSkillButton, vgui::Button );
	
public:
	CSkillButton(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings( KeyValues *resourceData );

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	SkillID GetSkill();

private:
	char m_szSkillName[100];
};

class CDASkillMenu : public CFolderMenuPanel
{
	DECLARE_CLASS_SIMPLE( CDASkillMenu, CFolderMenuPanel );

public:
	CDASkillMenu(Panel *parent);

public:
	virtual const char *GetName( void ) { return PANEL_BUY_EQUIP_CT; }

	virtual void Reset() {};
	virtual void Update( void );
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void SetVisible( bool state );
	virtual void ShowPanel(bool bShow);
	void OnCommand( const char *command );

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	CFolderLabel*      GetSkillInfo();
	CPanelTexture*     GetSkillIcon();

private:
	ButtonCode_t m_iSkillMenuKey;

	CFolderLabel*      m_pSkillInfo;
	CPanelTexture*     m_pSkillIcon;

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

#endif //SDK_SKILLMENU_H
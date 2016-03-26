//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_BUYMENU_H
#define SDK_BUYMENU_H

#include <classmenu.h>
#include <vgui_controls/EditablePanel.h>
#include <filesystem.h>
#include "IconPanel.h"
#include <vgui_controls/CheckButton.h>

#include "folder_gui.h"
#include "da.h"

class CWeaponButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CWeaponButton, vgui::Button );
	
public:
	CWeaponButton(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings( KeyValues *resourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	DAWeaponID GetWeaponID();

private:
	char m_szWeaponID[100];
	char m_szInfoString[100];
	char m_szInfoModel[100];

	IBorder* m_pArmedBorder;
};

class CDABuyMenu : public CFolderMenuPanel
{
	DECLARE_CLASS_SIMPLE( CDABuyMenu, CFolderMenuPanel );

public:
	CDABuyMenu(Panel *parent);

public:
	virtual const char *GetName( void ) { return PANEL_BUY; }

	virtual void Reset();
	virtual void Update( void );
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void ShowPanel(bool bShow);
	virtual void OnCommand( const char *command );
	virtual void OnThink();

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	vgui::Label*       GetWeaponInfo();
	class CModelPanel* GetWeaponImage();

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	class CFolderLabel* m_pWeaponInfo;
	class CModelPanel*  m_pWeaponImage;

	CUtlVector<CFolderLabel*> m_apStyles;
	CUtlVector<CFolderLabel*> m_apWeights;
	CUtlVector<CImageButton*> m_apCheckMarks;

	ButtonCode_t m_iBuyMenuKey;
};

#endif //SDK_BUYMENU_H
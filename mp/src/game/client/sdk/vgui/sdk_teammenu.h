//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#pragma once
#ifndef SDK_TEAMMENU_H
#define SDK_TEAMMENU_H

#include <string>

#include "teammenu.h"
#include <classmenu.h>
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "iconpanel.h"
#include <vgui_controls/CheckButton.h>
#include "folder_gui.h"

#if defined ( SDK_USE_TEAMS )
class CTeamButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CTeamButton, vgui::Button );
	
public:
	CTeamButton(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings( KeyValues *resourceData );

	virtual void OnCursorEntered();

private:
	int m_iSkin;
};

class CSDKTeamMenu : public CFolderMenu, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CSDKTeamMenu, CFolderMenu );
	// helper functions
	//void SetVisibleButton(const char *textEntryName, bool state);

public:
	CSDKTeamMenu(IViewPort *pViewPort);
	virtual ~CSDKTeamMenu();

	virtual void ApplySettings( KeyValues *resourceData );

	virtual const char *GetName( void ) { return PANEL_TEAM; }
	
	virtual void Reset();
	virtual void Update();
	virtual void UpdateCharacter();
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void SetVisible(bool state);
	virtual void ShowPanel(bool bShow);
	void OnCommand( const char *command);

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	void SetCharacterPreview( const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch );
	void SetCharacterSkin( int iSkin );

	vgui::Label*       GetCharacterInfo();
	class CModelPanel* GetCharacterImage();

private:
	IViewPort	*m_pViewPort;

	class CFolderLabel* m_pCharacterInfo;
	class CModelPanel*  m_pCharacterImage;

	ButtonCode_t m_iCharacterMenuKey;

	std::string m_sCharacterModel;
	std::string m_sCharacterSequence;
	std::string m_sCharacterWeaponModel;
	float       m_flBodyYaw;
	float       m_flBodyPitch;
	int         m_iCharacterSkin;

	// rounded corners
	//Color					 m_bgColor;
	//Color					 m_borderColor;

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

};
#endif // SDK_USE_TEAMS

#endif //SDK_CLASSMENU_H
//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#pragma once

#include <classmenu.h>
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "iconpanel.h"
#include <vgui_controls/CheckButton.h>
#include "folder_gui.h"

class CCharacterButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CCharacterButton, vgui::Button );
	
public:
	CCharacterButton(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings( KeyValues *resourceData );

	virtual void OnCursorEntered();

private:
	char m_szCharacter[100];
	char m_szSequence[100];
	char m_szWeaponModel[100];

	float m_flBodyPitch;
	float m_flBodyYaw;
};

class CDABCharacterMenu : public CFolderMenu, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CDABCharacterMenu, CFolderMenu );

public:
	CDABCharacterMenu(IViewPort *pViewPort);
	virtual ~CDABCharacterMenu();

	virtual const char *GetName( void ) { return PANEL_CLASS; }

	virtual void Reset();
	virtual void Update( void );
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void SetVisible( bool state );
	virtual void ShowPanel(bool bShow);
	void OnCommand( const char *command );

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	void SetCharacterPreview( const char* pszPreview, const char* pszSequence, const char* pszWeaponModel, float flYaw, float flPitch );

	vgui::Label*       GetCharacterInfo();
	class CModelPanel* GetCharacterImage();

private:
	IViewPort	*m_pViewPort;

	class CFolderLabel* m_pCharacterInfo;
	class CModelPanel*  m_pCharacterImage;

	ButtonCode_t m_iCharacterMenuKey;

	const char*	m_pszCharacterModel;
	const char*	m_pszCharacterSequence;
	const char*	m_pszCharacterWeaponModel;
	float		m_flBodyYaw;
	float		m_flBodyPitch;

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

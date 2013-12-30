#pragma once

#include <vgui_controls/Label.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <game/client/iviewport.h>

#include "da.h"

namespace vgui
{
	class CCheckButton;
}

class CFolderMenuPanel : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( CFolderMenuPanel, vgui::EditablePanel );

public:
	CFolderMenuPanel(Panel *parent, const char* pszName) : vgui::EditablePanel( parent, pszName ) {}

public:
	class CFolderMenu* GetFolderMenu();
};

class CFolderMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CFolderMenu, vgui::Frame );

public:
	CFolderMenu(IViewPort *pViewPort);
	virtual ~CFolderMenu();

public:
	virtual const char *GetName( void ) { return PANEL_FOLDER; }

	virtual void Reset();
	void MarkForUpdate() { m_bNeedsUpdate = true; }
	virtual void Update( void );
	void MoveToCenterOfScreen();
	virtual Panel *CreateControlByName( const char *controlName );
	static Panel* CreateControlByNameStatic( vgui::Panel* pParent, const char *controlName );
	virtual void ShowPanel(bool bShow);
	void OnCommand( const char *command );
	virtual void OnTick( void );

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	MESSAGE_FUNC_PTR( OnSuicideOptionChanged, "CheckButtonChecked", panel );

	void ShowPage(const char* pszPage);

protected:
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	bool                m_bNeedsUpdate;

	class CFolderLabel* m_pProfileInfo;
	class CFolderLabel* m_pCharacteristicsInfo;

	vgui::CheckButton*  m_pSuicideOption;

	char                m_szCharacter[100];

	CFolderMenuPanel*   m_pPage;
};

class CFolderLabel : public vgui::Label
{
private:
	DECLARE_CLASS_SIMPLE( CFolderLabel, vgui::Label );
	
public:
	CFolderLabel(vgui::Panel *parent, const char *panelName);

public:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

class CPanelTexture : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPanelTexture, vgui::Panel );

public:
	CPanelTexture(vgui::Panel *parent, const char *panelName);

	virtual void SetImage(const char *imageName);

	virtual void PaintBackground();
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	char         m_szImageName[100];
	CHudTexture* m_pImage;

	int          m_iX;
	int          m_iY;
	int          m_iWidth;
	int          m_iHeight;
};

class CImageButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CImageButton, vgui::Button );
	
public:
	CImageButton(vgui::Panel *parent, const char *panelName);

public:
	virtual void SetImage(const char *imageName);

	virtual void PaintBackground();
	virtual void PaintBorder() {};
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	char         m_szImageName[100];
	CHudTexture* m_pImage;

	int          m_iX;
	int          m_iY;
	int          m_iWidth;
	int          m_iHeight;
};

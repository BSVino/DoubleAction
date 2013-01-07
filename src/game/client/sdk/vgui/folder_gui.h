#pragma once

#include <vgui_controls/Label.h>

vgui::Panel* Folder_CreateControlByName( vgui::EditablePanel* pParent, const char *controlName );

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

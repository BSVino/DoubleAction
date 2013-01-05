#pragma once

#include <vgui_controls/Label.h>

class CFolderLabel : public vgui::Label
{
private:
	DECLARE_CLASS_SIMPLE( CFolderLabel, vgui::Label );
	
public:
	CFolderLabel(vgui::Panel *parent, const char *panelName );

public:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

vgui::Panel* Folder_CreateControlByName( vgui::EditablePanel* pParent, const char *controlName );

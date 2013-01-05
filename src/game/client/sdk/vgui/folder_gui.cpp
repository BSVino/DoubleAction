#include "cbase.h"

#include "folder_gui.h"

#include <vgui_controls/EditablePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CFolderLabel::CFolderLabel(vgui::Panel *parent, const char *panelName)
	: vgui::Label( parent, panelName, "FolderLabel")
{
}

void CFolderLabel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

vgui::Panel* Folder_CreateControlByName( vgui::EditablePanel* pParent, const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( pParent, NULL );

	return nullptr;
}

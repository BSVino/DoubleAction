//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"

#include "da_mapbrowser.h"

#include "steam/isteamfriends.h"
#include "steam/steam_api.h"

#include <cdll_client_int.h>
#include <ienginevgui.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

#include <vgui_controls/ImagePanel.h>

#include <filesystem.h>
#include <convar.h>

#include "da.h"

// Don't care me none about VCR mode.
#undef time
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Don't judge me.
static CMapBrowser* g_pMapBrowser = NULL;

using namespace vgui;

CMapBrowser* MapBrowser()
{
	return g_pMapBrowser;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapBrowser::CMapBrowser() : Frame(NULL, "mapbrowser")
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/MenuScheme.res", "MenuScheme"));

	SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));

	SetProportional(true);

	LoadControlSettings("Resource/UI/mapbrowser.res");
	InvalidateLayout();

	SetSizeable(false);

	SetTitle("Map Browser", true);

	game_maps = new KeyValues("game_maps");
	game_maps->LoadFromFile(filesystem, "resource/ui/game_maps.res");

	/*
		our keyvalues should look like this in game_maps.res:
		"game_maps"
		{
			"da_trainingday"
			{
				"map_name"      "da_trainingday"
				"map_image"     "resource/ui/maps/da_trainingday/da_trainingday.jpg"
			}
			"da_morgendorffer"
			{
				"map_name"      "da_morgendorffer"
				"map_image"     "resource/ui/maps/da_morgendorffer/da_morgendorffer.jpg"
			}
		}
	
	*/
	
	MakeReadyForUse();

	Update();

	g_pMapBrowser = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapBrowser::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("Resource/UI/mapbrowser.res");

	DisableFadeEffect(); //Tony; shut off the fade effect because we're using sourcesceheme.

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapBrowser::~CMapBrowser()
{
}

void CMapBrowser::Update(void)
{
	int countMaps = 0;
	for (KeyValues *map_data = game_maps->GetFirstSubKey(); map_data != NULL; map_data = map_data->GetNextKey())
	{
		// we are now looking at eg da_trainingday in the above keyvalues example
		// so map_data->GetString("map_name") == "da_trainingday"
		// DevMsg("\n\nloading keyvalue map name %s\n\n", map_data->GetString("map_name"));
		
		const char* mapName = map_data->GetString("map_name");
		const char* mapImage = map_data->GetString("map_image");

		int imageWidth = 96;
		int panelPaddingTop = 36;
		int panelPaddingLeft = 18;
		int imageMargin = 20;
		int colNum = countMaps % 3;
		int rowNum = (countMaps - colNum) / 3;
		int xPos = (imageWidth + imageMargin) * colNum + panelPaddingLeft;
		int yPos = (rowNum * imageWidth) + (rowNum * imageMargin) + panelPaddingTop;

		// create an image panel for each map in map_data.res
		ImagePanel* map_image = new ImagePanel(this, mapName);
		map_image->SetSize(imageWidth, imageWidth);
		map_image->SetPos(xPos, yPos);
		map_image->SetShouldScaleImage(true);
		map_image->SetImage(scheme()->GetImage(mapImage, false));

		countMaps++;
	}
}

void CMapBrowser::ShowPanel(bool bShow)
{

	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
	}
	else
		SetVisible(false);
}

void CMapBrowser::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_PAD_ENTER || code == KEY_ENTER)
		OnCommand("okay");
	else
		BaseClass::OnKeyCodePressed(code);
}

void CMapBrowser::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "okay"))
	{
		SetVisible(false);
		return;
	}

	BaseClass::OnCommand(command);
}
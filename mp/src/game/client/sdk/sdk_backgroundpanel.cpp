//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_backgroundpanel.h"

#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>
#include <vgui/ILocalize.h>
#include "vgui_controls/buildgroup.h"
#include "vgui_controls/BitmapImagePanel.h"

using namespace vgui;

//-----------------------------------------------------------------------------
const int NumSegments = 7;
static int coord[NumSegments+1] = {
	0,
	1,
	2,
	3,
	4,
	6,
	9,
	10
};

//-----------------------------------------------------------------------------
void DrawRoundedBackground( Color bgColor, int wide, int tall )
{
	int x1, x2, y1, y2;
	surface()->DrawSetColor(bgColor);
	surface()->DrawSetTextColor(bgColor);

	int i;

	// top-left corner --------------------------------------------------------
	int xDir = 1;
	int yDir = -1;
	int xIndex = 0;
	int yIndex = NumSegments - 1;
	int xMult = 1;
	int yMult = 1;
	int x = 0;
	int y = 0;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = y + coord[NumSegments];
		surface()->DrawFilledRect( x1, y1, x2, y2 );

		xIndex += xDir;
		yIndex += yDir;
	}

	// top-right corner -------------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = 0;
	xMult = -1;
	yMult = 1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = y + coord[NumSegments];
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-right corner ----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = tall;
	xMult = -1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = y - coord[NumSegments];
		y2 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-left corner -----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = 0;
	y = tall;
	xMult = 1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = y - coord[NumSegments];
		y2 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// paint between top left and bottom left ---------------------------------
	x1 = 0;
	x2 = coord[NumSegments];
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// paint between left and right -------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = 0;
	y2 = tall;
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// paint between top right and bottom right -------------------------------
	x1 = wide - coord[NumSegments];
	x2 = wide;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );
}

//-----------------------------------------------------------------------------
void DrawRoundedBorder( Color borderColor, int wide, int tall )
{
	int x1, x2, y1, y2;
	surface()->DrawSetColor(borderColor);
	surface()->DrawSetTextColor(borderColor);

	int i;

	// top-left corner --------------------------------------------------------
	int xDir = 1;
	int yDir = -1;
	int xIndex = 0;
	int yIndex = NumSegments - 1;
	int xMult = 1;
	int yMult = 1;
	int x = 0;
	int y = 0;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );

		xIndex += xDir;
		yIndex += yDir;
	}

	// top-right corner -------------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = 0;
	xMult = -1;
	yMult = 1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-right corner ----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = tall;
	xMult = -1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-left corner -----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = 0;
	y = tall;
	xMult = 1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// top --------------------------------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = 0;
	y2 = 1;
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// bottom -----------------------------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = tall - 1;
	y2 = tall;
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// left -------------------------------------------------------------------
	x1 = 0;
	x2 = 1;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// right ------------------------------------------------------------------
	x1 = wide - 1;
	x2 = wide;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );
}

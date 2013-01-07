#include "cbase.h"

#include "folder_gui.h"

#include <vgui/ISurface.h>

#include <vgui_controls/EditablePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

Panel* Folder_CreateControlByName( EditablePanel* pParent, const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( pParent, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( pParent, NULL );

	return nullptr;
}

CFolderLabel::CFolderLabel(Panel *parent, const char *panelName)
	: Label( parent, panelName, "FolderLabel")
{
}

void CFolderLabel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

CPanelTexture::CPanelTexture(Panel *parent, const char *panelName)
	: Panel(parent, panelName)
{
	m_pImage = nullptr;

	m_iX = m_iY = 0;
	m_iWidth = m_iHeight = 100;
}

void CPanelTexture::SetImage(const char* pszName)
{
	m_pImage = gHUD.GetIcon(pszName);
}

void CPanelTexture::PaintBackground()
{
	BaseClass::PaintBackground();

	if (m_pImage)
	{
		int x, y;
		GetPos(x, y);
		m_pImage->DrawSelf(m_iX, m_iY, m_iWidth, m_iHeight, Color(255, 255, 255, 255));
	}
}

void CPanelTexture::ApplySettings(KeyValues *inResourceData)
{
	Q_strncpy(m_szImageName, inResourceData->GetString("image", ""), sizeof(m_szImageName));

	BaseClass::ApplySettings(inResourceData);

	SetPos(0, 0);

	int wide, tall;

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	wide = screenWide; 
	tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	SetSize( wide, tall );

	const char *xstr = inResourceData->GetString( "xpos", NULL );
	const char *ystr = inResourceData->GetString( "ypos", NULL );
	if (xstr)
	{
		bool bRightAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (xstr[0] == 'r' || xstr[0] == 'R')
		{
			bRightAligned = true;
			xstr++;
		}
		else if (xstr[0] == 'c' || xstr[0] == 'C')
		{
			bCenterAligned = true;
			xstr++;
		}

		// get the value
		m_iX = atoi(xstr);

		// scale the x up to our screen co-ords
		if ( IsProportional() )
			m_iX = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iX);

		// now correct the alignment
		if (bRightAligned)
			m_iX = screenWide - m_iX; 
		else if (bCenterAligned)
			m_iX = (screenWide / 2) + m_iX;
	}

	if (ystr)
	{
		bool bBottomAligned = false;
		bool bCenterAligned = false;

		// look for alignment flags
		if (ystr[0] == 'r' || ystr[0] == 'R')
		{
			bBottomAligned = true;
			ystr++;
		}
		else if (ystr[0] == 'c' || ystr[0] == 'C')
		{
			bCenterAligned = true;
			ystr++;
		}

		m_iY = atoi(ystr);
		if (IsProportional())
			// scale the y up to our screen co-ords
			m_iY = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iY);

		// now correct the alignment
		if (bBottomAligned)
			m_iY = screenTall - m_iY; 
		else if (bCenterAligned)
			m_iY = (screenTall / 2) + m_iY;
	}

	const char *wstr = inResourceData->GetString( "wide", NULL );
	if ( wstr )
	{
		bool bFull = false;
		if (wstr[0] == 'f' || wstr[0] == 'F')
		{
			bFull = true;
			wstr++;
		}

		m_iWidth = atof(wstr);
		if ( IsProportional() )
		{
			// scale the x and y up to our screen co-ords
			m_iWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iWidth);
		}

		// now correct the alignment
		if (bFull)
			m_iWidth = screenWide - m_iWidth; 
	}

	m_iHeight = inResourceData->GetInt( "tall", tall );
	if ( IsProportional() )
	{
		// scale the x and y up to our screen co-ords
		m_iHeight = scheme()->GetProportionalScaledValueEx(GetScheme(), m_iHeight);
	}
}

void CPanelTexture::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetImage( m_szImageName );
}

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include <vguitextwindow.h>
#include "video/ivideoservices.h"

class CDAMainMenu : public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CDAMainMenu, vgui::Panel );
	CDAMainMenu::CDAMainMenu(vgui::Panel* parent, const char *pElementName );
	~CDAMainMenu();

public:
	bool IsVideoPlaying();
	void StartVideo();
	void StopVideo();
	void OnThink();
	void OnVideoOver();

	void SetNextThink( float flActionThink, int iAction );
	void GetPanelPos( int &xpos, int &ypos );

	void Paint( void );
	bool BeginPlayback( const char *pFilename );
	void ReleaseVideo();
	void SetBlackBackground( bool bBlack );
	void DoModal( void );

	MESSAGE_FUNC( OnDisconnectFromGame, "DisconnectedFromGame" );

private:
	bool			m_bToolsMode;
	bool			m_bPaintVideo;
	float			m_flActionThink;
	int				m_iAction;

	CMaterialReference m_MainMenuRef;

protected:
	IVideoMaterial *m_VideoMaterial;
	IMaterial		*m_pMaterial;
	int				m_nPlaybackHeight;			// Calculated to address ratio changes
	int				m_nPlaybackWidth;
	char			m_szExitCommand[MAX_PATH];	// This call is fired at the engine when the video finishes or is interrupted

	float			m_flU;	// U,V ranges for video on its sheet
	float			m_flV;

	bool			m_bBlackBackground;
	bool			m_bAllowAlternateMedia;

};

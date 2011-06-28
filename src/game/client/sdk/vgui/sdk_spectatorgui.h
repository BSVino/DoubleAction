//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSDKSPECTATORGUI_H
#define CSDKSPECTATORGUI_H
#ifdef _WIN32
#pragma once
#endif

#include "spectatorgui.h"
#include "mapoverview.h"

extern ConVar mp_forcecamera; // in gamevars_shared.h
extern ConVar mp_fadetoblack;


//-----------------------------------------------------------------------------
// Purpose: Cstrike Spectator UI
//-----------------------------------------------------------------------------
class CSDKSpectatorGUI : public CSpectatorGUI
{
private:
	DECLARE_CLASS_SIMPLE( CSDKSpectatorGUI, CSpectatorGUI );

public:
	CSDKSpectatorGUI( IViewPort *pViewPort );
		
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void UpdateSpectatorPlayerList( void );
	virtual void Update( void );
	virtual bool NeedsUpdate( void );

protected:

	void UpdateTimer();

	int		m_nLastTime;
	int		m_nLastSpecMode;
	CBaseEntity	*m_nLastSpecTarget;

	void StoreWidths( void );
	void ResizeControls( void );
	bool ControlsPresent( void ) const;

	vgui::Label *m_pBlueLabel;
	vgui::Label *m_pBlueScore;
	vgui::Label *m_pRedLabel;
	vgui::Label *m_pRedScore;

	vgui::Panel *m_pTimer;
	vgui::Label *m_pTimerLabel;
	vgui::Panel *m_pDivider;
	vgui::Label *m_pExtraInfo;

	bool m_modifiedWidths;

	int m_scoreWidth;
	int m_extraInfoWidth;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CSDKMapOverview : public CMapOverview
{
	DECLARE_CLASS_SIMPLE( CSDKMapOverview, CMapOverview );

public:	

	enum
	{
		MAP_ICON_PLAYER = 0,
#if defined ( SDK_USE_TEAMS )
		MAP_ICON_BLUE,
		MAP_ICON_RED,
#endif
		MAP_ICON_COUNT
	};

	CSDKMapOverview( const char *pElementName );
	virtual ~CSDKMapOverview();

	virtual bool ShouldDraw( void );
	vgui::Panel *GetAsPanel(){ return this; }
	virtual bool AllowConCommandsWhileAlive(){return false;}
	virtual void SetPlayerPreferredMode( int mode );
	virtual void SetPlayerPreferredViewSize( float viewSize );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

protected:	// private structures & types

	// list of game events the hLTV takes care of

	typedef struct {
		int		xpos;
		int		ypos;
	} FootStep_t;	

	// Extra stuff in a this-level parallel array
	typedef struct SDKMapPlayer_s {
		int		overrideIcon; // if not -1, the icon to use instead
		int		overrideIconOffscreen; // to use with overrideIcon
		float	overrideFadeTime; // Time to start fading the override icon
		float	overrideExpirationTime; // Time to not use the override icon any more
		Vector	overridePosition; // Where the overridden icon will draw
		QAngle	overrideAngle; // And at what angle
		bool	isDead;		// Death latch, since client can be behind the times on health messages.
		float	timeLastSeen; // curtime that we last saw this guy.
		float	timeFirstSeen; // curtime that we started seeing this guy
	} SDKMapPlayer_t;


public: // IViewPortPanel interface:

	virtual void Update();
	virtual void Init( void );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	// IGameEventListener

	virtual void FireGameEvent( IGameEvent *event);

	// VGUI overrides

	// Player settings:
	void SetPlayerSeen( int index );

	// general settings:
	virtual void SetMap(const char * map);
	virtual void SetMode( int mode );

	// rules that define if you can see a player on the overview or not
	virtual bool CanPlayerBeSeen(MapPlayer_t *player);

	virtual int GetIconNumberFromTeamNumber( int teamNumber );

protected:

	virtual void	DrawCamera();
	virtual void	DrawMapTexture();
	virtual void	DrawMapPlayers();
	virtual void	ResetRound();
	virtual void	InitTeamColorsAndIcons();
	virtual void	UpdateSizeAndPosition();
	Vector2D		PanelToMap( const Vector2D &panelPos );

	bool			AdjustPointToPanel(Vector2D *pos);
	MapPlayer_t*	GetPlayerByEntityID( int entityID );
	virtual void	UpdatePlayers();
	virtual bool	RunHudAnimations(){ return false; }

private:
	bool			DrawIconSDK(	int textureID,
		int offscreenTextureID,
		Vector pos,
		float scale,
		float angle,
		int alpha,
		bool allowRotation = true,
		const char *text = NULL,
		Color *textColor = NULL,
		float status = -1,
		Color *statusColor = NULL 
		);

	int GetMasterAlpha( void );// The main alpha that the map part should be, determined by using the mode to look at the right convar
	int GetBorderSize( void );// How far in from the edge of the panel we draw, based on mode.  Let's the background fancy corners show.
	SDKMapPlayer_t* GetSDKInfoForPlayerIndex( int index );
	SDKMapPlayer_t* GetSDKInfoForPlayer(MapPlayer_t *player);

	SDKMapPlayer_t	m_PlayersSDKInfo[MAX_PLAYERS];

	int		m_TeamIconsSelf[MAP_ICON_COUNT];
	int		m_TeamIconsDead[MAP_ICON_COUNT];
	int		m_TeamIconsOffscreen[MAP_ICON_COUNT];
	int		m_TeamIconsDeadOffscreen[MAP_ICON_COUNT];

	int		m_playerFacing;
	int		m_cameraIconFirst;
	int		m_cameraIconThird;
	int		m_cameraIconFree;

	int m_playerPreferredMode;
};

#endif // CSDKSPECTATORGUI_H

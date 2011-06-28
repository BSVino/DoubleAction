//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_spectatorgui.h"
#include "hud.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <filesystem.h>
#include "sdk_gamerules.h"
#include "c_sdk_team.h"
#include "c_sdk_player_resource.h"
#include "vtf/vtf.h"
#include "clientmode.h"
#include <vgui_controls/AnimationController.h>
#include "voice_status.h"
#include "c_sdk_player.h"

using namespace vgui;
DECLARE_HUDELEMENT( CSDKMapOverview )

extern ConVar overview_health;
extern ConVar overview_names;
extern ConVar overview_tracks;
extern ConVar overview_locked;
extern ConVar overview_alpha;

void PreferredOverviewModeChanged( IConVar *pConVar, const char *oldString, float flOldValue )
{
	ConVarRef var( pConVar );
	char cmd[32];
	V_snprintf( cmd, sizeof( cmd ), "overview_mode %d\n", var.GetInt() );
	engine->ClientCmd( cmd );
}
ConVar overview_preferred_mode( "overview_preferred_mode", "1", FCVAR_ARCHIVE, "Preferred overview mode", PreferredOverviewModeChanged );

ConVar overview_preferred_view_size( "overview_preferred_view_size", "600", FCVAR_ARCHIVE, "Preferred overview view size" );

#define DEATH_ICON_FADE (7.5f)
#define DEATH_ICON_DURATION (10.0f)
#define LAST_SEEN_ICON_DURATION (4.0f)
#define DIFFERENCE_THRESHOLD (100.0f)

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKSpectatorGUI::CSDKSpectatorGUI(IViewPort *pViewPort) : CSpectatorGUI(pViewPort)
{
	m_pBlueLabel =	NULL;
	m_pBlueScore =	NULL;
	m_pRedLabel =	NULL;
	m_pRedScore =	NULL;

	m_pTimer =		NULL;
	m_pTimerLabel =	NULL;

	m_pDivider =	NULL;

	m_pExtraInfo =	NULL;

	m_modifiedWidths = false;

	m_scoreWidth = 0;
	m_extraInfoWidth = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKSpectatorGUI::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Grab some control pointers
	m_pBlueLabel =	dynamic_cast<Label *>(FindChildByName("BlueScoreLabel"));
	m_pBlueScore =	dynamic_cast<Label *>(FindChildByName("BlueScoreValue"));
	m_pRedLabel =	dynamic_cast<Label *>(FindChildByName("RedScoreLabel"));
	m_pRedScore =	dynamic_cast<Label *>(FindChildByName("RedScoreValue"));

	m_pTimer =		dynamic_cast<Panel *>(FindChildByName("timerclock"));
	m_pTimerLabel =	dynamic_cast<Label *>(FindChildByName("timerlabel"));

	m_pDivider =	dynamic_cast<Panel *>(FindChildByName("DividerBar"));

	m_pExtraInfo =	dynamic_cast<Label *>(FindChildByName("extrainfo"));
}

//-----------------------------------------------------------------------------
// Purpose: Resets the list of players
//-----------------------------------------------------------------------------
void CSDKSpectatorGUI::UpdateSpectatorPlayerList()
{
#if defined ( SDK_USE_TEAMS )
	C_SDKTeam *blue = GetGlobalSDKTeam( SDK_TEAM_BLUE );
	if ( blue )
	{
		wchar_t frags[ 10 ];
		_snwprintf( frags, sizeof( frags ), L"%i",  blue->Get_Score()  );

		SetLabelText( "BlueScoreValue", frags );
	}

	C_SDKTeam *red = GetGlobalSDKTeam( SDK_TEAM_RED );
	if ( red )
	{
		wchar_t frags[ 10 ];
		_snwprintf( frags, sizeof( frags ), L"%i", red->Get_Score()  );
		
		SetLabelText( "RedScoreValue", frags );
	}
#else
	m_pBlueLabel->SetVisible( false );
	m_pBlueScore->SetVisible( false );
	m_pRedLabel->SetVisible( false );
	m_pRedScore->SetVisible( false );
#endif
}

bool CSDKSpectatorGUI::NeedsUpdate( void )
{
	C_SDKPlayer *player = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !player )
		return false;

	if ( m_nLastTime != (int)SDKGameRules()->GetMapRemainingTime() )
		return true;

	if ( m_nLastSpecMode != player->GetObserverMode() )
		return true;

	if ( m_nLastSpecTarget != player->GetObserverTarget() )
		return true;

	return BaseClass::NeedsUpdate();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the timer label if one exists
//-----------------------------------------------------------------------------
void CSDKSpectatorGUI::UpdateTimer()
{
	wchar_t szText[ 63 ];

	
	m_nLastTime = (int)( SDKGameRules()->GetMapRemainingTime() );

	if ( m_nLastTime < 0 )
		 m_nLastTime  = 0;

	_snwprintf ( szText, sizeof( szText ), L"%d:%02d", (m_nLastTime / 60), (m_nLastTime % 60) );
	szText[62] = 0;

	SetLabelText("timerlabel", szText );
}

void CSDKSpectatorGUI::Update()
{
	BaseClass::Update();
	
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if( pLocalPlayer )
	{
		m_nLastSpecMode = pLocalPlayer->GetObserverMode();
		m_nLastSpecTarget = pLocalPlayer->GetObserverTarget();
	}

	UpdateTimer();

	UpdateSpectatorPlayerList();

	if ( pLocalPlayer )
	{
		ResizeControls();
	}

}


//-----------------------------------------------------------------------------
// Purpose: Save off widths for sizing calculations
//-----------------------------------------------------------------------------
void CSDKSpectatorGUI::StoreWidths( void )
{
	if ( !ControlsPresent() )
		return;

	if ( !m_modifiedWidths )
	{
		m_scoreWidth = m_pBlueScore->GetWide();
		int terScoreWidth = m_pRedScore->GetWide();

		m_extraInfoWidth = m_pExtraInfo->GetWide();

		if ( m_scoreWidth != terScoreWidth )
		{
			m_pRedScore = NULL; // We're working with a modified res file.  Don't muck things up playing with positioning.
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resize controls so scores & map names are not cut off
//-----------------------------------------------------------------------------
void CSDKSpectatorGUI::ResizeControls( void )
{
	if ( !ControlsPresent() )
		return;

	int x1, y1, w1, t1;
	int x2, y2, w2, t2;

	StoreWidths();

	if ( !ControlsPresent() )
		return;

	// ensure scores are wide enough
	int wCT, hCT, wTer, hTer;
	m_pBlueScore->GetBounds( x1, y1, w1, t1 );
	m_pBlueScore->GetContentSize( wCT, hCT );
	m_pRedScore->GetBounds( x2, y2, w2, t2 );
	m_pRedScore->GetContentSize( wTer, hTer );
	
	int desiredScoreWidth = m_scoreWidth;
	desiredScoreWidth = max( desiredScoreWidth, wCT );
	desiredScoreWidth = max( desiredScoreWidth, wTer );

	int diff = desiredScoreWidth - w1;
	if ( diff != 0 )
	{
		m_pBlueScore->GetBounds( x1, y1, w1, t1 );
		m_pBlueScore->SetBounds( x1 - diff, y1, w1 + diff, t1 );

		m_pRedScore->GetBounds( x1, y1, w1, t1 );
		m_pRedScore->SetBounds( x1 - diff, y1, w1 + diff, t1 );

		m_pBlueLabel->GetPos( x1, y1 );
		m_pBlueLabel->SetPos( x1 - diff, y1 );

		m_pRedLabel->GetPos( x1, y1 );
		m_pRedLabel->SetPos( x1 - diff, y1 );

		m_modifiedWidths = true;
	}

	// ensure extra info is wide enough
	int wExtra, hExtra;
	m_pExtraInfo->GetBounds( x1, y1, w1, t1 );
	m_pExtraInfo->GetContentSize( wExtra, hExtra );

	int desiredExtraWidth = m_extraInfoWidth;
	desiredExtraWidth = max( desiredExtraWidth, wExtra );

	diff = desiredExtraWidth - w1;
	if ( diff != 0 )
	{
		m_pExtraInfo->GetBounds( x1, y1, w1, t1 );
		m_pExtraInfo->SetBounds( x1 - diff, y1, w1 + diff, t1 );

		m_pTimer->GetPos( x1, y1 );
		m_pTimer->SetPos( x1 - diff, y1 );

		m_pTimerLabel->GetPos( x1, y1 );
		m_pTimerLabel->SetPos( x1 - diff, y1 );

		m_pDivider->GetPos( x1, y1 );
		m_pDivider->SetPos( x1 - diff, y1 );

		m_pBlueScore->GetPos( x1, y1 );
		m_pBlueScore->SetPos( x1 - diff, y1 );

		m_pBlueLabel->GetPos( x1, y1 );
		m_pBlueLabel->SetPos( x1 - diff, y1 );

		m_pRedScore->GetPos( x1, y1 );
		m_pRedScore->SetPos( x1 - diff, y1 );

		m_pRedLabel->GetPos( x1, y1 );
		m_pRedLabel->SetPos( x1 - diff, y1 );

		m_modifiedWidths = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Verify controls are present to resize
//-----------------------------------------------------------------------------
bool CSDKSpectatorGUI::ControlsPresent( void ) const
{
	return ( m_pBlueLabel != NULL &&
		m_pBlueScore != NULL &&
		m_pRedLabel != NULL &&
		m_pRedScore != NULL &&
		m_pTimer != NULL &&
		m_pTimerLabel != NULL &&
		m_pDivider != NULL &&
		m_pExtraInfo != NULL );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int AdjustValue( int curValue, int targetValue, int amount )
{
	if ( curValue > targetValue )
	{
		curValue -= amount;

		if ( curValue < targetValue )
			curValue = targetValue;
	}
	else if ( curValue < targetValue )
	{
		curValue += amount;

		if ( curValue > targetValue )
			curValue = targetValue;
	}

	return curValue;
}

void CSDKMapOverview::InitTeamColorsAndIcons()
{
	BaseClass::InitTeamColorsAndIcons();

	Q_memset( m_TeamIconsSelf, 0, sizeof(m_TeamIconsSelf) );
	Q_memset( m_TeamIconsDead, 0, sizeof(m_TeamIconsDead) );
	Q_memset( m_TeamIconsOffscreen, 0, sizeof(m_TeamIconsOffscreen) );
	Q_memset( m_TeamIconsDeadOffscreen, 0, sizeof(m_TeamIconsDeadOffscreen) );

	m_playerFacing = -1;
	m_cameraIconFirst = -1;
	m_cameraIconThird = -1;
	m_cameraIconFree = -1;

	// setup normal player.
	m_TeamColors[MAP_ICON_PLAYER] = COLOR_YELLOW;
	m_TeamIcons[MAP_ICON_PLAYER] = AddIconTexture( "sprites/player_blue_small" );
	m_TeamIconsSelf[MAP_ICON_PLAYER] = AddIconTexture( "sprites/player_blue_self" );
	m_TeamIconsDead[MAP_ICON_PLAYER] = AddIconTexture( "sprites/player_blue_dead" );
	m_TeamIconsOffscreen[MAP_ICON_PLAYER] = AddIconTexture( "sprites/player_blue_offscreen" );
	m_TeamIconsDeadOffscreen[MAP_ICON_PLAYER] = AddIconTexture( "sprites/player_blue_dead_offscreen" );

#if defined ( SDK_USE_TEAMS )
	// setup team blue
	m_TeamColors[MAP_ICON_BLUE] = COLOR_BLUE;
	m_TeamIcons[MAP_ICON_BLUE] = AddIconTexture( "sprites/player_blue_small" );
	m_TeamIconsSelf[MAP_ICON_BLUE] = AddIconTexture( "sprites/player_blue_self" );
	m_TeamIconsDead[MAP_ICON_BLUE] = AddIconTexture( "sprites/player_blue_dead" );
	m_TeamIconsOffscreen[MAP_ICON_BLUE] = AddIconTexture( "sprites/player_blue_offscreen" );
	m_TeamIconsDeadOffscreen[MAP_ICON_BLUE] = AddIconTexture( "sprites/player_blue_dead_offscreen" );

	//setup team red
	m_TeamColors[MAP_ICON_RED] = COLOR_RED;
	m_TeamIcons[MAP_ICON_RED] = AddIconTexture( "sprites/player_red_small" );
	m_TeamIconsSelf[MAP_ICON_RED] = AddIconTexture( "sprites/player_red_self" );
	m_TeamIconsDead[MAP_ICON_RED] = AddIconTexture( "sprites/player_red_dead" );
	m_TeamIconsOffscreen[MAP_ICON_RED] = AddIconTexture( "sprites/player_red_offscreen" );
	m_TeamIconsDeadOffscreen[MAP_ICON_RED] = AddIconTexture( "sprites/player_red_dead_offscreen" );
#endif

	m_playerFacing = AddIconTexture( "sprites/player_tick" );
	m_cameraIconFirst = AddIconTexture( "sprites/spectator_eye" );
	m_cameraIconThird = AddIconTexture( "sprites/spectator_3rdcam" );
	m_cameraIconFree = AddIconTexture( "sprites/spectator_freecam" );

}

//-----------------------------------------------------------------------------
void CSDKMapOverview::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hIconFont = scheme->GetFont( "DefaultSmall", true );
}

//-----------------------------------------------------------------------------
void CSDKMapOverview::Update( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	int team = pPlayer->GetTeamNumber();

	// if dead with fadetoblack on, we can't show anything
	if ( mp_fadetoblack.GetBool() && team > TEAM_SPECTATOR && !pPlayer->IsAlive() )
	{
		SetMode( MAP_MODE_OFF );
		return;
	}

	BaseClass::Update();

	if ( GetSpectatorMode() == OBS_MODE_CHASE )
	{
		// Follow the local player in chase cam, so the map rotates using the local player's angles
		SetFollowEntity( pPlayer->entindex() );
	}
}

//-----------------------------------------------------------------------------
CSDKMapOverview::SDKMapPlayer_t* CSDKMapOverview::GetSDKInfoForPlayerIndex( int index )
{
	if ( index < 0 || index >= MAX_PLAYERS )
		return NULL;

	return &m_PlayersSDKInfo[ index ];
}

//-----------------------------------------------------------------------------
CSDKMapOverview::SDKMapPlayer_t* CSDKMapOverview::GetSDKInfoForPlayer(MapPlayer_t *player)
{
	if( player == NULL )
		return NULL;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( &m_Players[i] == player )
			return &m_PlayersSDKInfo[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
#define TIME_SPOTS_STAY_SEEN (0.5f)
#define TIME_UNTIL_ENEMY_SEEN (0.5f)
// rules that define if you can see a player on the overview or not
bool CSDKMapOverview::CanPlayerBeSeen( MapPlayer_t *player )
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if (!localPlayer || !player )
		return false;

	//Tony; if local player is spectating, just check base.
	if (localPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		return BaseClass::CanPlayerBeSeen(player);

	SDKMapPlayer_t *sdkPlayer = GetSDKInfoForPlayer(player);
		
	if ( !sdkPlayer )
		return false;

	if( player->health <= 0 )
	{
		// Have to be under the overriden icon time to draw when dead.
		if ( sdkPlayer->overrideExpirationTime == -1  ||  sdkPlayer->overrideExpirationTime <= gpGlobals->curtime )
			return false;
	}
	
	return BaseClass::CanPlayerBeSeen(player);
}

CSDKMapOverview::CSDKMapOverview( const char *pElementName ) : BaseClass( pElementName )
{
	g_pMapOverview = this;  // for cvars access etc

	switch ( overview_preferred_mode.GetInt() )
	{
	case MAP_MODE_INSET:
		m_playerPreferredMode = MAP_MODE_INSET;
		break;

	case MAP_MODE_FULL:
		m_playerPreferredMode = MAP_MODE_FULL;
		break;

	default:
		m_playerPreferredMode = MAP_MODE_OFF;
		break;
	}
}

void CSDKMapOverview::Init( void )
{
	BaseClass::Init();
}

CSDKMapOverview::~CSDKMapOverview()
{
	g_pMapOverview = NULL;

	//TODO release Textures ? clear lists
}

void CSDKMapOverview::UpdatePlayers()
{
	C_SDK_PlayerResource *pSDKPR = (C_SDK_PlayerResource*)GameResources();
	if ( !pSDKPR )
		return;

	CBasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
	if( localPlayer == NULL )
		return;

	MapPlayer_t *localMapPlayer = GetPlayerByUserID(localPlayer->GetUserID());
	if( localMapPlayer == NULL )
		return;

	for ( int i = 1; i<= gpGlobals->maxClients; i++)
	{
		MapPlayer_t *player = &m_Players[i-1];
		SDKMapPlayer_t *playerSDK = GetSDKInfoForPlayerIndex(i-1);

		if ( !playerSDK )
			continue;

		// update from global player resources
		if ( pSDKPR->IsConnected(i) )
		{
			player->health = pSDKPR->GetHealth( i );

			if ( !pSDKPR->IsAlive( i ) )
			{
				// Safety actually happens after a TKPunish.
				player->health = 0;
				playerSDK->isDead = true;
			}

			if ( player->team != pSDKPR->GetTeam( i ) )
			{
				player->team = pSDKPR->GetTeam( i );

				if( player == localMapPlayer )
					player->icon = m_TeamIconsSelf[ GetIconNumberFromTeamNumber(player->team) ];
				else
					player->icon = m_TeamIcons[ GetIconNumberFromTeamNumber(player->team) ];

				player->color = m_TeamColors[ GetIconNumberFromTeamNumber(player->team) ];
			}
		}

		Vector position = player->position;
		QAngle angles = player->angle;
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			// update position of active players in our PVS
			position = pPlayer->EyePosition();
			angles = pPlayer->EyeAngles();

			SetPlayerPositions( i-1, position, angles );
		}
	}

}

bool CSDKMapOverview::ShouldDraw( void )
{
	//Tony; don't draw the map unless it's a spectator - you could turn this into a radar if you wanted by using the other modes!
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (!pPlayer)
		return false;

	if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		return false;

	int alpha = GetMasterAlpha();
	if( alpha == 0 )
		return false;// we have been set to fully transparent

	return BaseClass::ShouldDraw();
}

CSDKMapOverview::MapPlayer_t* CSDKMapOverview::GetPlayerByEntityID( int entityID )
{
	C_BasePlayer *realPlayer = UTIL_PlayerByIndex(entityID);

	if( realPlayer == NULL )
		return NULL;

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *player = &m_Players[i];

		if ( player->userid == realPlayer->GetUserID() )
			return player;
	}

	return NULL;
}

#define BORDER_WIDTH 4
bool CSDKMapOverview::AdjustPointToPanel(Vector2D *pos)
{
	if( pos == NULL )
		return false;

	int mapInset = GetBorderSize();// This gives us the amount inside the panel that the background is drawing.
	if( mapInset != 0 )
		mapInset += BORDER_WIDTH; // And this gives us the border inside the map edge to give us room for offscreen icons.

	int x,y,w,t;

	//MapTpPanel has already offset the x and y.  That's why we use 0 for left and top.
	GetBounds( x,y,w,t );

	bool madeChange = false;
	if( pos->x < mapInset )
	{
		pos->x = mapInset;
		madeChange = true;
	}
	if( pos->x > w - mapInset )
	{
		pos->x = w - mapInset;
		madeChange = true;
	}
	if( pos->y < mapInset )
	{
		pos->y = mapInset;
		madeChange = true;
	}
	if( pos->y > t - mapInset )
	{
		pos->y = t - mapInset;
		madeChange = true;
	}

	return madeChange;
}

void CSDKMapOverview::DrawMapTexture()
{
	int alpha = GetMasterAlpha();

	if( GetMode() == MAP_MODE_FULL )
		SetBgColor( Color(0,0,0,0) );// no background in big mode
	else
		SetBgColor( Color(0,0,0,alpha * 0.5) );

	int textureIDToUse = m_nMapTextureID;

	int mapInset = GetBorderSize();
	int pwidth, pheight; 
	GetSize(pwidth, pheight);

	if ( textureIDToUse > 0 )
	{
		// We are drawing to the whole panel with a little border
		Vector2D panelTL = Vector2D( mapInset, mapInset );
		Vector2D panelTR = Vector2D( pwidth - mapInset, mapInset );
		Vector2D panelBR = Vector2D( pwidth - mapInset, pheight - mapInset );
		Vector2D panelBL = Vector2D( mapInset, pheight - mapInset );

		// So where are those four points on the great big map?
		Vector2D textureTL = PanelToMap( panelTL );// The top left corner of the display is where on the master map?
		textureTL /= OVERVIEW_MAP_SIZE;// Texture Vec2D is 0 to 1
		Vector2D textureTR = PanelToMap( panelTR );
		textureTR /= OVERVIEW_MAP_SIZE;
		Vector2D textureBR = PanelToMap( panelBR );
		textureBR /= OVERVIEW_MAP_SIZE;
		Vector2D textureBL = PanelToMap( panelBL );
		textureBL /= OVERVIEW_MAP_SIZE;

		Vertex_t points[4] =
		{
			// To draw a textured polygon, the first column is where you want to draw (to), and the second is what you want to draw (from).
			// We want to draw to the panel (pulled in for a border), and we want to draw the part of the map texture that should be seen.
			// First column is in panel coords, second column is in 0-1 texture coords
			Vertex_t( panelTL, textureTL ),
			Vertex_t( panelTR, textureTR ),
			Vertex_t( panelBR, textureBR ),
			Vertex_t( panelBL, textureBL )
		};

		surface()->DrawSetColor( 255, 255, 255, alpha );
		surface()->DrawSetTexture( textureIDToUse );
		surface()->DrawTexturedPolygon( 4, points );
	}
}

bool CSDKMapOverview::DrawIconSDK( int textureID, int offscreenTextureID, Vector pos, float scale, float angle, int alpha, bool allowRotation, const char *text, Color *textColor, float status, Color *statusColor )
{
	if( alpha <= 0 )
		return false;

	Vector2D pospanel = WorldToMap( pos );
	pospanel = MapToPanel( pospanel );

	int idToUse = textureID;
	float angleToUse = angle;

	Vector2D oldPos = pospanel;
	Vector2D adjustment(0,0);
	if( AdjustPointToPanel( &pospanel ) )
	{
		if( offscreenTextureID == -1 )
			return false; //Doesn't want to draw if off screen.

		// Move it in to on panel, and change the icon.
		idToUse = offscreenTextureID;
		// And point towards the original spot
		adjustment = Vector2D(pospanel.x - oldPos.x, pospanel.y - oldPos.y);
		QAngle adjustmentAngles;
		Vector adjustment3D(adjustment.x, -adjustment.y, 0); // Y gets flipped in WorldToMap
		VectorAngles(adjustment3D, adjustmentAngles) ;
		if( allowRotation )
		{
			angleToUse = adjustmentAngles[YAW];

			// And the angle needs to be in world space, not panel space.
			if( m_bFollowAngle )
			{
				angleToUse += m_fViewAngle;
			}
			else 
			{
				if ( m_bRotateMap )
					angleToUse += 180.0f;
				else
					angleToUse += 90.0f;
			}
		}

		// Don't draw names for icons that are offscreen (bunches up and looks bad)
		text = NULL;
	}

	int d = GetPixelOffset( scale );

	Vector offset;

	offset.x = -scale;	offset.y = scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos1 = WorldToMap( pos + offset );
	Vector2D pos1Panel = MapToPanel(pos1);
	pos1Panel.x += adjustment.x;
	pos1Panel.y += adjustment.y;

	offset.x = scale;	offset.y = scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos2 = WorldToMap( pos + offset );
	Vector2D pos2Panel = MapToPanel(pos2);
	pos2Panel.x += adjustment.x;
	pos2Panel.y += adjustment.y;

	offset.x = scale;	offset.y = -scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos3 = WorldToMap( pos + offset );
	Vector2D pos3Panel = MapToPanel(pos3);
	pos3Panel.x += adjustment.x;
	pos3Panel.y += adjustment.y;

	offset.x = -scale;	offset.y = -scale;
	VectorYawRotate( offset, angleToUse, offset );
	Vector2D pos4 = WorldToMap( pos + offset );
	Vector2D pos4Panel = MapToPanel(pos4);
	pos4Panel.x += adjustment.x;
	pos4Panel.y += adjustment.y;

	Vertex_t points[4] =
	{
		Vertex_t( pos1Panel, Vector2D(0,0) ),
			Vertex_t( pos2Panel, Vector2D(1,0) ),
			Vertex_t( pos3Panel, Vector2D(1,1) ),
			Vertex_t( pos4Panel, Vector2D(0,1) )
	};

	surface()->DrawSetColor( 255, 255, 255, alpha );
	surface()->DrawSetTexture( idToUse );
	surface()->DrawTexturedPolygon( 4, points );

	pospanel.y += d + 4;

	if ( status >=0.0f  && status <= 1.0f && statusColor )
	{
		// health bar is 50x3 pixels
		surface()->DrawSetColor( 0,0,0,255 );
		surface()->DrawFilledRect( pospanel.x-d, pospanel.y-1, pospanel.x+d, pospanel.y+1 );

		int length = (float)(d*2)*status;
		surface()->DrawSetColor( statusColor->r(), statusColor->g(), statusColor->b(), 255 );
		surface()->DrawFilledRect( pospanel.x-d, pospanel.y-1, pospanel.x-d+length, pospanel.y+1 );

		pospanel.y += 3;
	}

	if ( text && textColor )
	{
		wchar_t iconText[ MAX_PLAYER_NAME_LENGTH*2 ];

		g_pVGuiLocalize->ConvertANSIToUnicode( text, iconText, sizeof( iconText ) );

		int wide, tall;
		surface()->GetTextSize( m_hIconFont, iconText, wide, tall );

		int x = pospanel.x-(wide/2);
		int y = pospanel.y;

		// draw black shadow text
		surface()->DrawSetTextColor( 0, 0, 0, 255 );
		surface()->DrawSetTextPos( x+1, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );

		// draw name in color 
		surface()->DrawSetTextColor( textColor->r(), textColor->g(), textColor->b(), 255 );
		surface()->DrawSetTextPos( x, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );
	}

	return true;
}

void CSDKMapOverview::DrawMapPlayers()
{
	surface()->DrawSetTextFont( m_hIconFont );

	Color colorGreen( 0, 255, 0, 255 );	// health bar color
	CBasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	for (int i=0; i < MAX_PLAYERS; i++)
	{
		int alpha = 255;
		MapPlayer_t *player = &m_Players[i];
		SDKMapPlayer_t *playerSDK = GetSDKInfoForPlayerIndex(i);

		if ( !playerSDK )
			continue;

		if ( !CanPlayerBeSeen( player ) )
			continue;

		float status = -1;
		const char *name = NULL;

		if ( m_bShowNames && CanPlayerNameBeSeen( player ) )
			name = player->name;

		if ( m_bShowHealth && CanPlayerHealthBeSeen( player ) )
			status = player->health/100.0f;


		// Now draw them
		if( playerSDK->overrideExpirationTime > gpGlobals->curtime )// If dead, an X, if alive, an alpha'd normal icon
		{
			int alphaToUse = alpha;
			if( playerSDK->overrideFadeTime != -1 && playerSDK->overrideFadeTime <= gpGlobals->curtime )
			{
				// Fade linearly from fade start to disappear
				alphaToUse *= 1 - (float)(gpGlobals->curtime - playerSDK->overrideFadeTime) / (float)(playerSDK->overrideExpirationTime - playerSDK->overrideFadeTime);
			}

			DrawIconSDK( playerSDK->overrideIcon, playerSDK->overrideIconOffscreen, playerSDK->overridePosition, m_flIconSize * 1.1f, GetViewAngle(), player->health > 0 ? alphaToUse / 2 : alphaToUse, true, name, &player->color, -1, &colorGreen );
			if( player->health > 0 )
				DrawIconSDK( m_playerFacing, -1, playerSDK->overridePosition, m_flIconSize * 1.1f, playerSDK->overrideAngle[YAW], player->health > 0 ? alphaToUse / 2 : alphaToUse, true, name, &player->color, status, &colorGreen );
		}
		else
		{
			float zDifference = 0;
			if( localPlayer )
			{	
				if( (localPlayer->GetObserverMode() != OBS_MODE_NONE) && localPlayer->GetObserverTarget() )
					zDifference = player->position.z - localPlayer->GetObserverTarget()->GetAbsOrigin().z;
				else
					zDifference = player->position.z - localPlayer->GetAbsOrigin().z;
			}

			float sizeForRing = m_flIconSize * 1.4f;
			if( zDifference > DIFFERENCE_THRESHOLD )
			{
				// A dot above is bigger and a little fuzzy now.
				sizeForRing = m_flIconSize * 1.9f;
			}
			else if( zDifference < -DIFFERENCE_THRESHOLD )
			{
				// A dot below is smaller.
				sizeForRing = m_flIconSize * 1.0f;
			}

			bool showTalkRing = localPlayer ? (localPlayer->GetTeamNumber() == player->team) : false;
			if( localPlayer && localPlayer->GetTeamNumber() == TEAM_SPECTATOR )
				showTalkRing = true;

			float sizeForPlayer = m_flIconSize * 1.1f;// The 1.1 is because the player dots are shrunken a little, so their facing pip can have some space to live
			if( zDifference > DIFFERENCE_THRESHOLD )
			{
				// A dot above is bigger and a little fuzzy now.
				sizeForPlayer = m_flIconSize * 1.6f;
				alpha *= 0.5f;
			}
			else if( zDifference < -DIFFERENCE_THRESHOLD )
			{
				// A dot below is smaller.
				sizeForPlayer = m_flIconSize * 0.7f;
			}

			int normalIcon, offscreenIcon;
			normalIcon = player->icon;
			offscreenIcon = m_TeamIconsOffscreen[GetIconNumberFromTeamNumber(player->team)];

			bool doingLocalPlayer = false;
			if( GetPlayerByUserID(localPlayer->GetUserID()) == player )
				doingLocalPlayer = true;

			float angleForPlayer = GetViewAngle();

			if( doingLocalPlayer )
			{
				sizeForPlayer *= 4.0f; // The self icon is really big since it has a camera view cone attached.
				angleForPlayer = player->angle[YAW];// And, the self icon now rotates, natch.
			}

			DrawIconSDK( normalIcon, offscreenIcon, player->position, sizeForPlayer, angleForPlayer, alpha, true, name, &player->color, status, &colorGreen );
			if( !doingLocalPlayer )
			{
				// Draw the facing for everyone but the local player.
				if( player->health > 0 )
					DrawIconSDK( m_playerFacing, -1, player->position, sizeForPlayer, player->angle[YAW], alpha, true, name, &player->color, status, &colorGreen );
			}
		}
	}
}

void CSDKMapOverview::SetMap(const char * levelname)
{
	BaseClass::SetMap(levelname);

	int wide, tall;
	surface()->DrawGetTextureSize( m_nMapTextureID, wide, tall );
	if( wide == 0 && tall == 0 )
	{
		m_nMapTextureID = -1;
		return;
	}
}

void CSDKMapOverview::ResetRound()
{
	BaseClass::ResetRound();

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		SDKMapPlayer_t *p = &m_PlayersSDKInfo[i];

		p->isDead = false;

		p->overrideFadeTime = -1;
		p->overrideExpirationTime = -1;
		p->overrideIcon = -1;
		p->overrideIconOffscreen = -1;
		p->overridePosition = Vector( 0, 0, 0);
		p->overrideAngle = QAngle(0, 0, 0);

		p->timeLastSeen = -1;
		p->timeFirstSeen = -1;
	}
}

void CSDKMapOverview::DrawCamera()
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();

	if (!localPlayer)
		return;

	if( localPlayer->GetObserverMode() == OBS_MODE_ROAMING )
	{
		// Instead of the programmer-art red dot, we'll draw an icon for when our camera is roaming.
		int alpha = 255;
		DrawIconSDK(m_cameraIconFree, m_cameraIconFree, localPlayer->GetAbsOrigin(), m_flIconSize * 3.0f, localPlayer->EyeAngles()[YAW], alpha);
	}
	else if( localPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
	{
		if( localPlayer->GetObserverTarget() )
		{
			// Fade it if it is on top of a player dot.  And don't rotate it.
			int alpha = 255 * 0.5f;
			DrawIconSDK(m_cameraIconFirst, m_cameraIconFirst, localPlayer->GetObserverTarget()->GetAbsOrigin(), m_flIconSize * 1.5f, GetViewAngle(), alpha);
		}
	}
	else if( localPlayer->GetObserverMode() == OBS_MODE_CHASE )
	{
		if( localPlayer->GetObserverTarget() )
		{
			// Or Draw the third-camera a little bigger. (Needs room to be off the dot being followed)
			int alpha = 255;
			DrawIconSDK(m_cameraIconThird, m_cameraIconThird, localPlayer->GetObserverTarget()->GetAbsOrigin(), m_flIconSize * 3.0f, localPlayer->EyeAngles()[YAW], alpha);
		}
	}
}

void CSDKMapOverview::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type,"player_death") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->health = 0;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails

		SDKMapPlayer_t *playerSDK = GetSDKInfoForPlayer(player);

		if ( !playerSDK )
			return;

		playerSDK->isDead = true;
		playerSDK->overrideIcon = m_TeamIconsDead[GetIconNumberFromTeamNumber(player->team)];
		playerSDK->overrideIconOffscreen = playerSDK->overrideIcon;
		playerSDK->overridePosition = player->position;
		playerSDK->overrideAngle = player->angle;
		playerSDK->overrideFadeTime = gpGlobals->curtime + DEATH_ICON_FADE;
		playerSDK->overrideExpirationTime = gpGlobals->curtime + DEATH_ICON_DURATION;
	}
	else if ( Q_strcmp(type,"player_team") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		CBasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
		if( localPlayer == NULL )
			return;
		MapPlayer_t *localMapPlayer = GetPlayerByUserID(localPlayer->GetUserID());

		player->team = event->GetInt("team");

		if( player == localMapPlayer )
			player->icon = m_TeamIconsSelf[ GetIconNumberFromTeamNumber(player->team) ];
		else
			player->icon = m_TeamIcons[ GetIconNumberFromTeamNumber(player->team) ];

		player->color = m_TeamColors[ GetIconNumberFromTeamNumber(player->team) ];
	}
	else
	{
		BaseClass::FireGameEvent(event);
	}
}

void CSDKMapOverview::SetMode(int mode)
{
	if ( mode == MAP_MODE_INSET )
	{
		SetPaintBackgroundType( 2 );// rounded corners

		float desiredZoom = (overview_preferred_view_size.GetFloat() * m_fMapScale) / (OVERVIEW_MAP_SIZE * m_fFullZoom);

		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "zoom", desiredZoom, 0.0f, 0.2f, vgui::AnimationController::INTERPOLATOR_LINEAR );
	}
	else 
	{
		SetPaintBackgroundType( 0 );// square corners

		float desiredZoom = 1.0f;

		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "zoom", desiredZoom, 0.0f, 0.2f, vgui::AnimationController::INTERPOLATOR_LINEAR );
	}

	BaseClass::SetMode(mode);
}

void CSDKMapOverview::UpdateSizeAndPosition()
{
	int x,y,w,h;

	vgui::surface()->GetScreenSize( w, h );

	switch( m_nMode )
	{
	case MAP_MODE_INSET:
		{
			m_vPosition.x = XRES(16);
			m_vPosition.y = YRES(16);

			if ( g_pSpectatorGUI && g_pSpectatorGUI->IsVisible() )
			{
				m_vPosition.y += g_pSpectatorGUI->GetTopBarHeight();
			}

			m_vSize.x = w/4;
			m_vSize.y = m_vSize.x/1.333;
			break;
		}

	case MAP_MODE_FULL:
	default:
		{
			m_vSize.x = w;
			m_vSize.y = h;

			m_vPosition.x = 0;
			m_vPosition.y = 0;

			if ( g_pSpectatorGUI && g_pSpectatorGUI->IsVisible() )
			{
				m_vPosition.y += g_pSpectatorGUI->GetTopBarHeight();
				m_vSize.y -= g_pSpectatorGUI->GetTopBarHeight();
				m_vSize.y -= g_pSpectatorGUI->GetBottomBarHeight();
			}
			break;
		}
	}

	GetBounds( x,y,w,h );

	if ( m_flChangeSpeed > 0 )
	{
		// adjust slowly
		int pixels = m_flChangeSpeed * gpGlobals->frametime;
		x = AdjustValue( x, m_vPosition.x, pixels );
		y = AdjustValue( y, m_vPosition.y, pixels );
		w = AdjustValue( w, m_vSize.x, pixels );
		h = AdjustValue( h, m_vSize.y, pixels );
	}
	else
	{
		// set instantly
		x = m_vPosition.x;
		y = m_vPosition.y;
		w = m_vSize.x;
		h = m_vSize.y;
	}

	SetBounds( x,y,w,h );
}

void CSDKMapOverview::SetPlayerSeen( int index )
{
	SDKMapPlayer_t *pCS = GetSDKInfoForPlayerIndex(index);

	float now = gpGlobals->curtime;

	if( pCS )
	{
		if( pCS->timeLastSeen == -1 )
			pCS->timeFirstSeen = now;

		pCS->timeLastSeen = now;
	}
}

//-----------------------------------------------------------------------------
void CSDKMapOverview::SetPlayerPreferredMode( int mode )
{
	// A player has given an explicit overview_mode command
	m_playerPreferredMode = mode;

	switch ( mode )
	{
	case MAP_MODE_OFF:
		overview_preferred_mode.SetValue( MAP_MODE_OFF );
		break;

	case MAP_MODE_INSET:
		overview_preferred_mode.SetValue( MAP_MODE_INSET );
		break;

	case MAP_MODE_FULL:
		overview_preferred_mode.SetValue( MAP_MODE_FULL );
		break;
	}
}

//-----------------------------------------------------------------------------
void CSDKMapOverview::SetPlayerPreferredViewSize( float viewSize )
{
	overview_preferred_view_size.SetValue( viewSize );
}


//-----------------------------------------------------------------------------
int CSDKMapOverview::GetIconNumberFromTeamNumber( int teamNumber )
{
	switch(teamNumber) 
	{
#if defined ( SDK_USE_TEAMS )
	case SDK_TEAM_BLUE:
		return MAP_ICON_BLUE;

	case SDK_TEAM_RED:
		return MAP_ICON_RED;
#endif
	case 0:
	default:
		return MAP_ICON_PLAYER;
	}
}

//-----------------------------------------------------------------------------
int CSDKMapOverview::GetMasterAlpha( void )
{
	// The master alpha is the alpha that the map wants to draw at.  The background will be at half that, and the icons
	// will always be full.  (The icons fade themselves for functional reasons like seen-recently.)
	int alpha = clamp( overview_alpha.GetFloat() * 255, 0, 255 );

	return alpha;
}

//-----------------------------------------------------------------------------
int CSDKMapOverview::GetBorderSize( void )
{
	switch( GetMode() )
	{
		case MAP_MODE_INSET:
			return 4;
		case MAP_MODE_FULL:
		default:
			return 0;
	}
}

//-----------------------------------------------------------------------------
Vector2D CSDKMapOverview::PanelToMap( const Vector2D &panelPos )
{
	// This is the reversing of baseclass's MapToPanel
	int pwidth, pheight; 
	GetSize(pwidth, pheight);
	float viewAngle = GetViewAngle();
	float fScale = (m_fZoom * m_fFullZoom) / OVERVIEW_MAP_SIZE;

	Vector offset;
	offset.x = (panelPos.x - (pwidth * 0.5f)) / pheight;
	offset.y = (panelPos.y - (pheight * 0.5f)) / pheight;

	offset.x /= fScale;
	offset.y /= fScale;

	VectorYawRotate( offset, -viewAngle, offset );

	Vector2D mapPos;
	mapPos.x = offset.x + m_MapCenter.x;
	mapPos.y = offset.y + m_MapCenter.y;

	return mapPos;
}
//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hl2mpclientscoreboard.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_hl2mp_player.h"
#include "backgroundpanel.h"
#include "hl2mp_gamerules.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>
#include <vgui_controls/SectionedListPanel.h>

#include "voice_status.h"
#include "vgui_avatarimage.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::CHL2MPClientScoreBoardDialog( IViewPort *pViewPort ) : CClientScoreBoardDialog( pViewPort )
{
	m_pPlayerListDM = new SectionedListPanel( this, "PlayerListDM" );
	m_pPlayerCountLabel_DM = new Label( this, "DM_PlayerCount", "" );
	m_pPingLabel_DM = new Label( this, "DM_Latency", "" );

	m_pPlayerListR = new SectionedListPanel( this, "PlayerListR" );
	m_pPlayerCountLabel_R = new Label( this, "R_PlayerCount", "" );
	m_pScoreLabel_R = new Label( this, "R_Score", "" );
	m_pPingLabel_R = new Label( this, "R_Latency", "" );

	m_pPlayerListC = new SectionedListPanel( this, "PlayerListC" );
	m_pPlayerCountLabel_C = new Label( this, "C_PlayerCount", "" );
	m_pScoreLabel_C = new Label( this, "C_Score", "" );
	m_pPingLabel_C = new Label( this, "C_Latency", "" );

	m_pVertLine = new ImagePanel( this, "VerticalLine" );

	ListenForGameEvent( "server_spawn" );
	SetDialogVariable( "server", "" );
	SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::~CHL2MPClientScoreBoardDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint background for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBackground( m_bgColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBorder()
{
	int wide, tall;
	GetSize( wide, tall );

	DrawRoundedBorder( m_borderColor, wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/scoreboard.res" );

	m_bgColor = GetSchemeColor( "SectionedListPanel.BgColor", GetBgColor(), pScheme );
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );

	if ( m_pPlayerListDM )
	{
		m_pPlayerListDM->SetImageList( m_pImageList, false );
		m_pPlayerListDM->SetBgColor( Color( 0, 0, 0, 0 ) );
		m_pPlayerListDM->SetBorder( NULL );
		m_pPlayerListDM->SetVisible( false );
	}

	if ( m_pPlayerListR )
	{
		m_pPlayerListR->SetImageList( m_pImageList, false );
		m_pPlayerListR->SetBgColor( Color( 0, 0, 0, 0 ) );
		m_pPlayerListR->SetBorder( NULL );
		m_pPlayerListR->SetVisible( false );
	}

	if ( m_pPlayerListC )
	{
		m_pPlayerListC->SetImageList( m_pImageList, false );
		m_pPlayerListC->SetBgColor( Color( 0, 0, 0, 0 ) );
		m_pPlayerListC->SetBorder( NULL );
		m_pPlayerListC->SetVisible( false );
	}

	// turn off the default player list since we have our own
	if ( m_pPlayerList )
	{
		m_pPlayerList->SetVisible( false );
	}
	m_pScoreHeader_DM = (Label*)FindChildByName( "DM_ScoreHeader" );
	m_pDeathsHeader_DM = (Label*)FindChildByName( "DM_DeathsHeader" );
	m_pPingHeader_DM = (Label*)FindChildByName( "DM_PingHeader" );

	if ( m_pPlayerCountLabel_DM && m_pScoreHeader_DM && m_pDeathsHeader_DM && m_pPingHeader_DM && m_pPingLabel_DM )
	{
		m_pPlayerCountLabel_DM->SetFgColor( COLOR_YELLOW );
		m_pScoreHeader_DM->SetFgColor( COLOR_YELLOW );
		m_pDeathsHeader_DM->SetFgColor( COLOR_YELLOW );
		m_pPingHeader_DM->SetFgColor( COLOR_YELLOW );
		m_pPingLabel_DM->SetFgColor( COLOR_YELLOW );
	}
	m_pScoreHeader_R = (Label*)FindChildByName( "R_ScoreHeader" );
	m_pDeathsHeader_R = (Label*)FindChildByName( "R_DeathsHeader" );
	m_pPingHeader_R = (Label*)FindChildByName( "R_PingHeader" );

	if ( m_pPlayerCountLabel_R && m_pScoreHeader_R && m_pScoreLabel_R && m_pDeathsHeader_R && m_pPingHeader_R && m_pPingLabel_R )
	{
		m_pPlayerCountLabel_R->SetFgColor( COLOR_RED );
		m_pScoreHeader_R->SetFgColor( COLOR_RED );
		m_pScoreLabel_R->SetFgColor( COLOR_RED );
		m_pDeathsHeader_R->SetFgColor( COLOR_RED );
		m_pPingHeader_R->SetFgColor( COLOR_RED );
		m_pPingLabel_R->SetFgColor( COLOR_RED );
	}

	m_pScoreHeader_C = (Label*)FindChildByName( "C_ScoreHeader" );
	m_pDeathsHeader_C = (Label*)FindChildByName( "C_DeathsHeader" );
	m_pPingHeader_C = (Label*)FindChildByName( "C_PingHeader" );

	if ( m_pPlayerCountLabel_C && m_pScoreHeader_C && m_pScoreLabel_C && m_pDeathsHeader_C && m_pPingHeader_C && m_pPingLabel_C )
	{
		m_pPlayerCountLabel_C->SetFgColor( COLOR_BLUE );
		m_pScoreHeader_C->SetFgColor( COLOR_BLUE );
		m_pScoreLabel_C->SetFgColor( COLOR_BLUE );
		m_pDeathsHeader_C->SetFgColor( COLOR_BLUE );
		m_pPingHeader_C->SetFgColor( COLOR_BLUE );
		m_pPingLabel_C->SetFgColor( COLOR_BLUE );
	}

	// Store the scoreboard width, for Update();
	m_iStoredScoreboardWidth = GetWide();

	SetVisible( false );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Resets the scoreboard panel
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::Reset()
{
	InitPlayerList( m_pPlayerListDM, TEAM_UNASSIGNED );
	InitPlayerList( m_pPlayerListR, TEAM_REBELS );
	InitPlayerList( m_pPlayerListC, TEAM_COMBINE );
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::HL2MPPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1 = list->GetItemData( itemID1 );
	KeyValues *it2 = list->GetItemData( itemID2 );
	Assert( it1 && it2 );

	// first compare score
	int v1 = it1->GetInt( "frags" );
	int v2 = it2->GetInt( "frags" );
	if ( v1 > v2 )
		return true;
	else if ( v1 < v2 )
		return false;

	// second compare deaths
	v1 = it1->GetInt( "deaths" );
	v2 = it2->GetInt( "deaths" );
	if ( v1 > v2 )
		return false;
	else if ( v1 < v2 )
		return true;

	// if score and deaths are the same, use player index to get deterministic sort
	int iPlayerIndex1 = it1->GetInt( "playerIndex" );
	int iPlayerIndex2 = it2->GetInt( "playerIndex" );
	return ( iPlayerIndex1 > iPlayerIndex2 );
}

//-----------------------------------------------------------------------------
// Purpose: Inits the player list in a list panel
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::InitPlayerList( SectionedListPanel *pPlayerList, int teamNumber )
{
	pPlayerList->SetVerticalScrollbar( false );
	pPlayerList->RemoveAll();
	pPlayerList->RemoveAllSections();
	pPlayerList->AddSection( 0, "Players", HL2MPPlayerSortFunc );
	pPlayerList->SetSectionAlwaysVisible( 0, true );
	pPlayerList->SetSectionFgColor( 0, Color( 255, 255, 255, 255 ) );
	pPlayerList->SetBgColor( Color( 0, 0, 0, 0 ) );
	pPlayerList->SetBorder( NULL );

	// set the section to have the team color
	if ( teamNumber && GameResources() )
	{
		pPlayerList->SetSectionFgColor( 0, GameResources()->GetTeamColor( teamNumber ) );
	}

	if ( ShowAvatars() )
	{
		pPlayerList->AddColumnToSection( 0, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, m_iAvatarWidth );
	}

	pPlayerList->AddColumnToSection( 0, "name", "", 0, m_iNameWidth );
	pPlayerList->AddColumnToSection( 0, "class", "" , 0, m_iClassWidth );
	pPlayerList->AddColumnToSection( 0, "frags", "", SectionedListPanel::COLUMN_RIGHT, m_iScoreWidth );
	pPlayerList->AddColumnToSection( 0, "deaths", "", SectionedListPanel::COLUMN_RIGHT, m_iDeathWidth );
	pPlayerList->AddColumnToSection( 0, "ping", "", SectionedListPanel::COLUMN_RIGHT, m_iPingWidth );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::Update()
{

	UpdateItemVisibiity();
	UpdateTeamInfo();
	UpdatePlayerList();
	UpdateSpectatorList();
	MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: Updates information about teams
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdateTeamInfo()
{
	// update the team sections in the scoreboard
	int startTeam = TEAM_UNASSIGNED;

	if ( HL2MPRules()->IsTeamplay() )
		startTeam = TEAM_COMBINE;

	for ( int teamIndex = startTeam; teamIndex <= TEAM_REBELS; teamIndex++ )
	{
		// Make sure spectator is always skipped here.
		if ( teamIndex == TEAM_SPECTATOR )
			continue;

		wchar_t *teamName = NULL;
		C_Team *team = GetGlobalTeam( teamIndex );
		if ( team )
		{
			// choose dialog variables to set depending on team
			const char *pDialogVarTeamScore = NULL;
			const char *pDialogVarTeamPlayerCount = NULL;
			const char *pDialogVarTeamPing = NULL;
			switch ( teamIndex ) 
			{
				case TEAM_REBELS:
					teamName = g_pVGuiLocalize->Find( "#HL2MP_ScoreBoard_Rebels" );
					pDialogVarTeamScore = "r_teamscore";
					pDialogVarTeamPlayerCount = "r_teamplayercount";
					pDialogVarTeamPing = "r_teamping";
					break;
				case TEAM_COMBINE:
					teamName = g_pVGuiLocalize->Find( "#HL2MP_ScoreBoard_Combine" );
					pDialogVarTeamScore = "c_teamscore";
					pDialogVarTeamPlayerCount = "c_teamplayercount";
					pDialogVarTeamPing = "c_teamping";
					break;
				case TEAM_UNASSIGNED:
					teamName = g_pVGuiLocalize->Find( "#HL2MP_ScoreBoard_DM" );
					pDialogVarTeamPlayerCount = "dm_playercount";
					pDialogVarTeamPing = "dm_ping";
					break;
				default:
					Assert( false );
					break;
			}

			// update # of players on each team
			wchar_t name[64];
			wchar_t string1[1024];
			wchar_t wNumPlayers[6];
			_snwprintf( wNumPlayers, ARRAYSIZE( wNumPlayers ), L"%i", team->Get_Number_Players() );
			if ( !teamName && team )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( team->Get_Name(), name, sizeof( name ) );
				teamName = name;
			}
			if ( team->Get_Number_Players() == 1 )
			{
				g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find( "#ScoreBoard_Player" ), 2, teamName, wNumPlayers );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find( "#ScoreBoard_Players" ), 2, teamName, wNumPlayers );
			}

			// set # of players for team in dialog
			SetDialogVariable( pDialogVarTeamPlayerCount, string1 );

			// set team score in dialog
			if ( teamIndex != TEAM_UNASSIGNED )	// Don't accumulate deathmatch scores.
				SetDialogVariable( pDialogVarTeamScore, team->Get_Score() );			

			int pingsum = 0;
			int numcounted = 0;
			for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
			{
				if( g_PR->IsConnected( playerIndex ) && g_PR->GetTeam( playerIndex ) == teamIndex )
				{
					int ping = g_PR->GetPing( playerIndex );

					if ( ping >= 1 )
					{
						pingsum += ping;
						numcounted++;
					}
				}
			}

			if ( numcounted > 0 )
			{
				int ping = (int)( (float)pingsum / (float)numcounted );
				SetDialogVariable( pDialogVarTeamPing, ping );		
			}
			else
			{
				SetDialogVariable( pDialogVarTeamPing, "" );	
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the player list
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdatePlayerList()
{
	m_pPlayerListDM->RemoveAll();
	m_pPlayerListR->RemoveAll();
	m_pPlayerListC->RemoveAll();

	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pLocalPlayer )
		return;

	for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if( g_PR->IsConnected( playerIndex ) )
		{
			SectionedListPanel *pPlayerList = NULL;
			
			// Not teamplay, use the DM playerlist
			if ( !HL2MPRules()->IsTeamplay() )
				pPlayerList = m_pPlayerListDM;
			else
			{
				switch ( g_PR->GetTeam( playerIndex ) )
				{
				case TEAM_REBELS:
					pPlayerList = m_pPlayerListR;
					break;
				case TEAM_COMBINE:
					pPlayerList = m_pPlayerListC;
					break;
				}
			}

			if ( pPlayerList == NULL )
			{
				continue;			
			}

			KeyValues *pKeyValues = new KeyValues( "data" );
			GetPlayerScoreInfo( playerIndex, pKeyValues );

			int itemID = pPlayerList->AddItem( 0, pKeyValues );
			Color clr = g_PR->GetTeamColor( g_PR->GetTeam( playerIndex ) );
			pPlayerList->SetItemFgColor( itemID, clr );

			pKeyValues->deleteThis();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the spectator list
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdateSpectatorList()
{
	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pLocalPlayer )
		return;

	char szSpectatorList[512] = "" ;
	int nSpectators = 0;
	for( int playerIndex = 1 ; playerIndex <= MAX_PLAYERS; playerIndex++ )
	{
		if ( ShouldShowAsSpectator( playerIndex ) )
		{
			if ( nSpectators > 0 )
			{
				Q_strncat( szSpectatorList, ", ", ARRAYSIZE( szSpectatorList ) );
			}

			Q_strncat( szSpectatorList, g_PR->GetPlayerName( playerIndex ), ARRAYSIZE( szSpectatorList ) );
			nSpectators++;
		}
	}

	wchar_t wzSpectators[512] = L"";
	if ( nSpectators > 0 )
	{
		const char *pchFormat = ( 1 == nSpectators ? "#ScoreBoard_Spectator" : "#ScoreBoard_Spectators" );

		wchar_t wzSpectatorCount[16];
		wchar_t wzSpectatorList[1024];
		_snwprintf( wzSpectatorCount, ARRAYSIZE( wzSpectatorCount ), L"%i", nSpectators );
		g_pVGuiLocalize->ConvertANSIToUnicode( szSpectatorList, wzSpectatorList, sizeof( wzSpectatorList ) );
		g_pVGuiLocalize->ConstructString( wzSpectators, sizeof(wzSpectators), g_pVGuiLocalize->Find( pchFormat), 2, wzSpectatorCount, wzSpectatorList );
	}

	SetDialogVariable( "spectators", wzSpectators );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether the specified player index is a spectator
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::ShouldShowAsSpectator( int iPlayerIndex )
{
	// see if player is connected
	if ( g_PR->IsConnected( iPlayerIndex ) ) 
	{
		// spectators show in spectator list
		int iTeam = g_PR->GetTeam( iPlayerIndex );

		// In team play the DM playerlist is invisible, so show unassigned in the spectator list.
		if ( HL2MPRules()->IsTeamplay() && TEAM_UNASSIGNED == iTeam )
			return true;

		if ( TEAM_SPECTATOR == iTeam )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Event handler
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();

	if ( 0 == Q_strcmp( type, "server_spawn" ) )
	{		
		// set server name in scoreboard
		const char *hostname = event->GetString( "hostname" );
		wchar_t wzHostName[256];
		wchar_t wzServerLabel[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( hostname, wzHostName, sizeof( wzHostName ) );
		g_pVGuiLocalize->ConstructString( wzServerLabel, sizeof(wzServerLabel), g_pVGuiLocalize->Find( "#Scoreboard_Server" ), 1, wzHostName );
		SetDialogVariable( "server", wzServerLabel );
	}

	if( IsVisible() )
	{
		Update();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
{
	// Clean up the player name
	const char *oldName = g_PR->GetPlayerName( playerIndex );
	int bufsize = strlen( oldName ) * 2 + 1;
	char *newName = (char *)_alloca( bufsize );
	UTIL_MakeSafeName( oldName, newName, bufsize );
	kv->SetString( "name", newName );

	kv->SetInt( "playerIndex", playerIndex );
	kv->SetInt( "frags", g_PR->GetPlayerScore( playerIndex ) );
	kv->SetInt( "deaths", g_PR->GetDeaths( playerIndex ) );
	kv->SetString( "class", "" );

	UpdatePlayerAvatar( playerIndex, kv );

	if ( g_PR->GetPing( playerIndex ) < 1 )
	{
		if ( g_PR->IsFakePlayer( playerIndex ) )
		{
			kv->SetString( "ping", "BOT" );
		}
		else
		{
			kv->SetString( "ping", "" );
		}
	}
	else
	{
		kv->SetInt( "ping", g_PR->GetPing( playerIndex ) );
	}

	return true;
}

void CHL2MPClientScoreBoardDialog::UpdateItemVisibiity()
{
	// Need to do this in Update, ensure the correct player lists/headers are visible.
	if ( HL2MPRules()->IsTeamplay() )
	{
		// Rebel Labels _ON_
		m_pPlayerListR->SetVisible( true );
		m_pPlayerCountLabel_R->SetVisible( true );
		m_pScoreHeader_R->SetVisible( true );
		m_pScoreLabel_R->SetVisible( true );
		m_pDeathsHeader_R->SetVisible( true );
		m_pPingHeader_R->SetVisible( true );
		m_pPingLabel_R->SetVisible( true );

		// Combine Labels _ON_
		m_pPlayerListC->SetVisible( true );
		m_pPlayerCountLabel_C->SetVisible( true );
		m_pScoreHeader_C->SetVisible( true );
		m_pScoreLabel_C->SetVisible( true );
		m_pDeathsHeader_C->SetVisible( true );
		m_pPingHeader_C->SetVisible( true );
		m_pPingLabel_C->SetVisible( true );

		// Vertical Line _ON_
		m_pVertLine->SetVisible( true );

		// DM Labels _OFF_
		m_pPlayerListDM->SetVisible( false );
		m_pPlayerCountLabel_DM->SetVisible( false );
		m_pScoreHeader_DM->SetVisible( false );
		m_pDeathsHeader_DM->SetVisible( false );
		m_pPingHeader_DM->SetVisible( false );
		m_pPingLabel_DM->SetVisible( false );

		// Restore the size to the original incase we've switched from DM -> Teams and back.
		SetSize(m_iStoredScoreboardWidth, GetTall() );
	}
	else
	{
		// Rebel Labels _OFF_
		m_pPlayerListR->SetVisible( false );
		m_pPlayerCountLabel_R->SetVisible( false );
		m_pScoreHeader_R->SetVisible( false );
		m_pScoreLabel_R->SetVisible( false );
		m_pDeathsHeader_R->SetVisible( false );
		m_pPingHeader_R->SetVisible( false );
		m_pPingLabel_R->SetVisible( false );

		// Combine Labels _OFF_
		m_pPlayerListC->SetVisible( false );
		m_pPlayerCountLabel_C->SetVisible( false );
		m_pScoreHeader_C->SetVisible( false );
		m_pScoreLabel_C->SetVisible( false );
		m_pDeathsHeader_C->SetVisible( false );
		m_pPingHeader_C->SetVisible( false );
		m_pPingLabel_C->SetVisible( false );

		// Vertical Line _OFF_
		m_pVertLine->SetVisible( false );

		// DM Labels _ON_
		m_pPlayerListDM->SetVisible( true );
		m_pPlayerCountLabel_DM->SetVisible( true );
		m_pScoreHeader_DM->SetVisible( true );
		m_pDeathsHeader_DM->SetVisible( true );
		m_pPingHeader_DM->SetVisible( true );
		m_pPingLabel_DM->SetVisible( true );

		// Because we have a multi-pane player list, in deathmatch shrink the width of the scoreboard to match the one player list, so it looks nicer.
		int wide, tall;
		m_pPlayerListDM->GetContentSize(wide, tall);
		tall = GetTall();
		SetSize(wide+4, tall);
		m_pPlayerListDM->SetSize(wide, tall);
	}
}

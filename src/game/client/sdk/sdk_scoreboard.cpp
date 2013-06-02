//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "sdk_scoreboard.h"
#include "c_sdk_team.h"
#include "c_sdk_player_resource.h"
#include "sdk_gamerules.h"
#include "sdk_backgroundpanel.h"
#include "folder_gui.h"
#include "c_sdk_player.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>
#include <vgui_controls/SectionedListPanel.h>

using namespace vgui;

#define TEAM_MAXCOUNT			5

// id's of sections used in the scoreboard
enum EScoreboardSections
{
#if defined ( SDK_USE_TEAMS )
	SCORESECTION_TEAM1 = 1,
	SCORESECTION_TEAM2 = 2,
	SCORESECTION_FREEFORALL = 3,
	SCORESECTION_SPECTATOR = 4
#else
	SCORESECTION_FREEFORALL = 1, 
	SCORESECTION_SPECTATOR,
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKScoreboard::CSDKScoreboard(IViewPort *pViewPort):CClientScoreBoardDialog(pViewPort)
{
	SetScheme("FolderScheme");
	LoadControlSettings("Resource/UI/ScoreBoard.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSDKScoreboard::~CSDKScoreboard()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CSDKScoreboard::PaintBackground()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CSDKScoreboard::PaintBorder()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CSDKScoreboard::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void CSDKScoreboard::Update( void )
{
	m_pPlayerList->DeleteAllItems();

	UpdateTeamInfo();
	UpdatePlayerInfo();

	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos(0, (wt - GetTall()) / 2);

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: sets up base sections
//-----------------------------------------------------------------------------
void CSDKScoreboard::InitScoreboardSections()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);

	// fill out the structure of the scoreboard
	AddHeader();

#if defined ( SDK_USE_TEAMS )
	if ( SDKGameRules()->IsTeamplay() )
	{
		// add the team sections
		AddSection( TYPE_TEAM, SDK_TEAM_BLUE );
		AddSection( TYPE_TEAM, SDK_TEAM_RED );
	}
#endif
	AddSection( TYPE_TEAM, TEAM_UNASSIGNED );
	AddSection( TYPE_SPECTATORS, TEAM_SPECTATOR );
}

//-----------------------------------------------------------------------------
// Purpose: resets the scoreboard team info
//-----------------------------------------------------------------------------
void CSDKScoreboard::UpdateTeamInfo()
{
	if ( SDKGameResources() == NULL )
		return;

	int iNumPlayersInGame = 0;

	for ( int j = 1; j <= gpGlobals->maxClients; j++ )
	{	
		if ( SDKGameResources()->IsConnected( j ) )
		{
			iNumPlayersInGame++;
		}
	}

	// update the team sections in the scoreboard
	for ( int i = TEAM_SPECTATOR; i < TEAM_MAXCOUNT; i++ )
	{
		wchar_t *teamName = NULL;
		int sectionID = 0;
		C_Team *team = GetGlobalTeam(i);

		if ( team )
		{
			sectionID = GetSectionFromTeamNumber( i );
	
			// update team name
			wchar_t name[64];
			wchar_t string1[1024];
			wchar_t wNumPlayers[6];

			if ( i != TEAM_SPECTATOR && SDKGameRules()->IsTeamplay() == false )
			{
				// after sdk_team_deathmatch is used just get team->get_number_players
				C_Team *spec = GetGlobalTeam(TEAM_SPECTATOR);
				int iActivePlayers;

				if(spec)
					iActivePlayers = iNumPlayersInGame - spec->Get_Number_Players();
				else
					iActivePlayers = iNumPlayersInGame;

				_snwprintf(wNumPlayers, 6, L"%i", iActivePlayers );
				_snwprintf( name, sizeof(name), L"%s", g_pVGuiLocalize->Find("#SDK_ScoreBoard_Deathmatch") );
				
				teamName = name;

				if ( iActivePlayers == 1)
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#SDK_ScoreBoard_Player"), 2, teamName, wNumPlayers );
				}
				else
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#SDK_ScoreBoard_Players"), 2, teamName, wNumPlayers );
				}
			}
			else
			{
				_snwprintf(wNumPlayers, 6, L"%i", team->Get_Number_Players());

				if (!teamName && team)
				{
					g_pVGuiLocalize->ConvertANSIToUnicode(team->Get_Name(), name, sizeof(name));
					teamName = name;
				}

				if (team->Get_Number_Players() == 1)
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#SDK_ScoreBoard_Player"), 2, teamName, wNumPlayers );
				}
				else
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#SDK_ScoreBoard_Players"), 2, teamName, wNumPlayers );
				}

				// update stats
				wchar_t val[6];
				swprintf(val, L"%d", team->Get_Score());
				m_pPlayerList->ModifyColumn(sectionID, "frags", val);
				if (team->Get_Ping() < 1)
				{
					m_pPlayerList->ModifyColumn(sectionID, "ping", L"");
				}
				else
				{
					swprintf(val, L"%d", team->Get_Ping());
					m_pPlayerList->ModifyColumn(sectionID, "ping", val);
				}

			}
		
			m_pPlayerList->ModifyColumn(sectionID, "name", string1);
			m_pPlayerList->SetSectionFgColor(sectionID, Color(0, 0, 0, 255));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CSDKScoreboard::AddHeader()
{
	// add the top header
	m_pPlayerList->AddSection(0, "");
	m_pPlayerList->SetSectionAlwaysVisible(0);
	m_pPlayerList->SetSectionFgColor(0, Color(0, 0, 0, 255));

	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "FolderSmall", false );

	m_pPlayerList->SetFontSection(0, scheme()->GetIScheme( GetScheme() )->GetFont( "FolderTiny", false ));

	int iScoreWidth = scheme()->GetProportionalScaledValueEx( GetScheme(), m_iScoreWidth );
	int iPingWidth = scheme()->GetProportionalScaledValueEx( GetScheme(), m_iPingWidth );
	int iListWidth = m_pPlayerList->GetWide();

	m_pPlayerList->AddColumnToSection(0, "avatar", "", SectionedListPanel::COLUMN_IMAGE, m_iAvatarWidth );
	m_pPlayerList->AddColumnToSection(0, "name", "", 0, iListWidth - m_iAvatarWidth - iPingWidth - iScoreWidth*1.5f, hFallbackFont );
	m_pPlayerList->AddColumnToSection(0, "frags", "#DA_PlayerScore", 0 | SectionedListPanel::COLUMN_RIGHT, iScoreWidth );
	m_pPlayerList->AddColumnToSection(0, "ping", "#DA_PlayerPing", 0 | SectionedListPanel::COLUMN_RIGHT, iPingWidth );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
void CSDKScoreboard::AddSection(int teamType, int teamNumber)
{
	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "FolderSmall", false );

	int sectionID = GetSectionFromTeamNumber( teamNumber );
	if ( teamType == TYPE_TEAM )
	{
 		m_pPlayerList->AddSection(sectionID, "", StaticPlayerSortFunc);
		m_pPlayerList->SetSectionFgColor(sectionID, Color(0, 0, 0, 255));

		m_pPlayerList->SetFontSection(sectionID, scheme()->GetIScheme( GetScheme() )->GetFont( "FolderTiny", false ));

		int iScoreWidth = scheme()->GetProportionalScaledValueEx( GetScheme(), m_iScoreWidth );
		int iPingWidth = scheme()->GetProportionalScaledValueEx( GetScheme(), m_iPingWidth );
		int iListWidth = m_pPlayerList->GetWide();

		// setup the columns
		m_pPlayerList->AddColumnToSection(sectionID, "avatar", "", SectionedListPanel::COLUMN_IMAGE, m_iAvatarWidth );
		m_pPlayerList->AddColumnToSection(sectionID, "name", "", 0, iListWidth - m_iAvatarWidth - iPingWidth - iScoreWidth*1.5f, hFallbackFont );
		m_pPlayerList->AddColumnToSection(sectionID, "frags", "", SectionedListPanel::COLUMN_RIGHT, iScoreWidth );
		m_pPlayerList->AddColumnToSection(sectionID, "ping", "", SectionedListPanel::COLUMN_RIGHT, iPingWidth );

		//Tony; don't make unassigned always visible when using teams.
#if defined ( SDK_USE_TEAMS )
		if ( teamNumber != TEAM_UNASSIGNED )
			m_pPlayerList->SetSectionAlwaysVisible(sectionID);
#else
			m_pPlayerList->SetSectionAlwaysVisible(sectionID);
#endif
	}
	else if ( teamType == TYPE_SPECTATORS )
	{
		m_pPlayerList->AddSection(sectionID, "");
		m_pPlayerList->SetFontSection(sectionID, scheme()->GetIScheme( GetScheme() )->GetFont( "FolderTiny", false ));
		m_pPlayerList->AddColumnToSection(sectionID, "avatar", "", SectionedListPanel::COLUMN_IMAGE, m_iAvatarWidth );
		m_pPlayerList->AddColumnToSection(sectionID, "name", "#SDK_Team_Spectators", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), m_iNameWidth ), hFallbackFont );
		m_pPlayerList->SetSectionFgColor(sectionID, Color(0, 0, 0, 255));
	}
}

int CSDKScoreboard::GetSectionFromTeamNumber( int teamNumber )
{
	switch ( teamNumber )
	{
#if defined ( SDK_USE_TEAMS )
	case SDK_TEAM_BLUE:
		return SCORESECTION_TEAM1;
	case SDK_TEAM_RED:
		return SCORESECTION_TEAM2;
#endif
	case TEAM_SPECTATOR:
		return SCORESECTION_SPECTATOR;
	default:
		return SCORESECTION_FREEFORALL;
	}
	return SCORESECTION_FREEFORALL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKScoreboard::UpdatePlayerInfo()
{
	m_iSectionId = 0; // 0'th row is a header
	int selectedRow = -1;
	int i;

	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !SDKGameResources() )
		return;

	C_SDK_PlayerResource *sdkPR = SDKGameResources();

#if defined ( SDK_USE_PLAYERCLASSES )
	int localteam = pPlayer->GetTeamNumber();
	C_BasePlayer *pOther = NULL;
#endif

	// walk all the players and make sure they're in the scoreboard
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		bool shouldShow = sdkPR->IsConnected( i );
		if ( shouldShow )
		{
			// add the player to the list
			KeyValues *kv = new KeyValues("data");
			kv->SetInt("playerIndex", i);
			kv->SetInt("team", sdkPR->GetTeam( i ) );
			kv->SetString("name", sdkPR->GetPlayerName(i) );
			kv->SetInt("frags", sdkPR->GetStyle( i ));

#if defined ( SDK_USE_PLAYERCLASSES )
			//Tony; for player classname, just look up directly from the player.
			pOther = UTIL_PlayerByIndex(i);
			if (pOther )
			{
				int ot = pOther->GetTeamNumber();
				if ( ot == localteam || localteam == TEAM_UNASSIGNED)
					kv->SetString("class", SDKGameRules()->GetPlayerClassName( SDKGameResources()->GetPlayerClass(i), ot ) );
				else
					kv->SetString("class", "");
			}
			else
				kv->SetString("class","");
#endif

			UpdatePlayerAvatar( i, kv );

			if (sdkPR->GetPing( i ) < 1)
			{
				if ( sdkPR->IsFakePlayer( i ) )
				{
					kv->SetString("ping", "BOT");
				}
				else
				{
					kv->SetString("ping", "");
				}
			}
			else
			{
				kv->SetInt("ping", sdkPR->GetPing( i ));
			}

			int itemID = FindItemIDForPlayerIndex( i );
  			int sectionID = GetSectionFromTeamNumber( sdkPR->GetTeam( i ) );
						
			if (itemID == -1)
			{
				// add a new row
				itemID = m_pPlayerList->AddItem( sectionID, kv );

				HFont hItemFont = scheme()->GetIScheme( GetScheme() )->GetFont( "FolderSmall", false );
				m_pPlayerList->SetItemFont(itemID, hItemFont);
			}
			else
			{
				// modify the current row
				m_pPlayerList->ModifyItem( itemID, sectionID, kv );
			}

			if ( i == pPlayer->entindex() )
			{
				selectedRow = itemID;	// this is the local player, hilight this row
			}

			// set the row color based on the players team
			m_pPlayerList->SetItemFgColor( itemID, Color(0, 0, 0, 255) );

			kv->deleteThis();
		}
		else
		{
			// remove the player
			int itemID = FindItemIDForPlayerIndex( i );
			if (itemID != -1)
			{
				m_pPlayerList->RemoveItem(itemID);
			}
		}
	}

	if ( selectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem(selectedRow);
	}

	Label *pMostLabel;
	Label *pMostLabelPlayer;

	pMostLabel = dynamic_cast<Label *>(FindChildByName("MostStyle"));
	if (pMostLabel)
		pMostLabel->SetText("#DA_ScoreBoard_MostStyle");

	pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName("MostStylePlayer"));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(sdkPR->GetHighestStylePlayer()));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
	}

	pMostLabel = dynamic_cast<Label *>(FindChildByName("MostStunts"));
	if (pMostLabel)
	{
		pMostLabel->SetText("#DA_ScoreBoard_StuntKills");

		wchar_t wszLabel[100];
		pMostLabel->GetText(wszLabel, sizeof(wszLabel));

		wchar_t wszNewLabel[100];
		_snwprintf(wszNewLabel, 100, L"%s %i", wszLabel, sdkPR->GetHighestStuntKills());

		pMostLabel->SetText(wszNewLabel);
	}

	pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName("MostStuntsPlayer"));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(sdkPR->GetHighestStuntKillPlayer()));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
	}

	pMostLabel = dynamic_cast<Label *>(FindChildByName("MostBrawl"));
	if (pMostLabel)
	{
		pMostLabel->SetText("#DA_ScoreBoard_BrawlKills");

		wchar_t wszLabel[100];
		pMostLabel->GetText(wszLabel, sizeof(wszLabel));

		wchar_t wszNewLabel[100];
		_snwprintf(wszNewLabel, 100, L"%s %i", wszLabel, sdkPR->GetHighestBrawlKills());

		pMostLabel->SetText(wszNewLabel);
	}

	pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName("MostBrawlPlayer"));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(sdkPR->GetHighestBrawlKillPlayer()));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
	}

	pMostLabel = dynamic_cast<Label *>(FindChildByName("MostStreak"));
	if (pMostLabel)
	{
		pMostLabel->SetText("#DA_ScoreBoard_KillStreak");

		wchar_t wszLabel[100];
		pMostLabel->GetText(wszLabel, sizeof(wszLabel));

		wchar_t wszNewLabel[100];
		_snwprintf(wszNewLabel, 100, L"%s %i", wszLabel, sdkPR->GetHighestKillStreak());

		pMostLabel->SetText(wszNewLabel);
	}

	pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName("MostStreakPlayer"));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(sdkPR->GetHighestKillStreakPlayer()));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
	}

	pMostLabel = dynamic_cast<Label *>(FindChildByName("MostGrenade"));
	if (pMostLabel)
	{
		pMostLabel->SetText("#DA_ScoreBoard_GrenadeKills");

		wchar_t wszLabel[100];
		pMostLabel->GetText(wszLabel, sizeof(wszLabel));

		wchar_t wszNewLabel[100];
		_snwprintf(wszNewLabel, 100, L"%s %i", wszLabel, sdkPR->GetHighestGrenadeKills());

		pMostLabel->SetText(wszNewLabel);
	}

	pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName("MostGrenadePlayer"));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(sdkPR->GetHighestGrenadeKillPlayer()));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
	}
}

Panel *CSDKScoreboard::CreateControlByName( const char *controlName )
{
	if (FStrEq(controlName, "FolderLabel"))
		return new CFolderLabel( this, NULL );

	if (FStrEq(controlName, "PanelTexture"))
		return new CPanelTexture( this, NULL );

	return BaseClass::CreateControlByName(controlName);
}

CON_COMMAND(hud_reload_scoreboard, "Reload resource for scoreboard.")
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_SCOREBOARD );
	CSDKScoreboard *pScoreboard = dynamic_cast<CSDKScoreboard*>(pPanel);
	if (!pScoreboard)
		return;

	pScoreboard->LoadControlSettings( "Resource/UI/ScoreBoard.res" );
	pScoreboard->InvalidateLayout();
	pScoreboard->Update();
}

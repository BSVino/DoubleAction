//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "c_sdk_team.h"
#include "sdk_scoreboard.h"
#include "c_sdk_player_resource.h"
#include "sdk_gamerules.h"
#include "folder_gui.h"
#include "c_sdk_player.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
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
	SetScheme("ClientScheme");
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
	BaseClass::PaintBackground();

	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CSDKScoreboard::PaintBorder()
{
	BaseClass::PaintBorder();
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CSDKScoreboard::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("SectionedListPanel.BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );
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

	MoveToCenterOfScreen();
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

				V_swprintf_safe(wNumPlayers, L"%i", iActivePlayers);
				V_swprintf_safe( name, L"%s", g_pVGuiLocalize->Find("#SDK_ScoreBoard_Deathmatch") );
				
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
				V_swprintf_safe(wNumPlayers, L"%i", team->Get_Number_Players());

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
				V_swprintf_safe(val, L"%d", team->Get_Score());
				m_pPlayerList->ModifyColumn(sectionID, "frags", val);
				if (team->Get_Ping() < 1)
				{
					m_pPlayerList->ModifyColumn(sectionID, "ping", L"");
				}
				else
				{
					V_swprintf_safe(val, L"%d", team->Get_Ping());
					m_pPlayerList->ModifyColumn(sectionID, "ping", val);
				}

			}
		
			m_pPlayerList->ModifyColumn(sectionID, "name", string1);
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

	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardSmall", false );

	m_pPlayerList->SetFontSection(0, scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardTiny", false ));

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
	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardSmall", false );

	int sectionID = GetSectionFromTeamNumber( teamNumber );
	if ( teamType == TYPE_TEAM )
	{
 		m_pPlayerList->AddSection(sectionID, "", StaticPlayerSortFunc);

		m_pPlayerList->SetFontSection(sectionID, scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardTiny", false ));

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
		if ( !SDKGameRules()->IsTeamplay() || teamNumber != TEAM_UNASSIGNED )
			m_pPlayerList->SetSectionAlwaysVisible(sectionID);
#else
			m_pPlayerList->SetSectionAlwaysVisible(sectionID);
#endif
	}
	else if ( teamType == TYPE_SPECTATORS )
	{
		m_pPlayerList->AddSection(sectionID, "");
		m_pPlayerList->SetFontSection(sectionID, scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardTiny", false ));
		m_pPlayerList->AddColumnToSection(sectionID, "avatar", "", SectionedListPanel::COLUMN_IMAGE, m_iAvatarWidth );
		m_pPlayerList->AddColumnToSection(sectionID, "name", "#SDK_Team_Spectators", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), m_iNameWidth ), hFallbackFont );
	}
}

int CSDKScoreboard::GetSectionFromTeamNumber( int teamNumber )
{
	if (SDKGameRules()->IsTeamplay())
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
	}
	if (teamNumber == TEAM_SPECTATOR)
		return SCORESECTION_SPECTATOR;
	else
		return SCORESECTION_TEAM1;
		//return SCORESECTION_FREEFORALL;
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
		if (!shouldShow)
		{
			// remove the player
			int itemID = FindItemIDForPlayerIndex( i );
			if (itemID != -1)
			{
				m_pPlayerList->RemoveItem(itemID);
			}

			continue;
		}
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

			HFont hItemFont = scheme()->GetIScheme( GetScheme() )->GetFont( "ScoreboardSmall", false );
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
		m_pPlayerList->SetItemFgColor( itemID, sdkPR->GetTeamColor( sdkPR->GetTeam( i ) ) );

		kv->deleteThis();
	}

	if ( selectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem(selectedRow);
	}

	UpdateMostLabel(
		"MostStyle",
		"MostStylePlayer",
		"#DA_ScoreBoard_MostStyle",
		sdkPR->GetHighestStylePlayer(),
		-1//sdkPR->GetHighestStyle()
	);

	UpdateMostLabel(
		"MostStunts",
		"MostStuntsPlayer",
		"#DA_ScoreBoard_StuntKills",
		sdkPR->GetHighestStuntKillPlayer(),
		sdkPR->GetHighestStuntKills()
	);

	UpdateMostLabel(
		"MostBrawl",
		"MostBrawlPlayer",
		"#DA_ScoreBoard_BrawlKills",
		sdkPR->GetHighestBrawlKillPlayer(),
		sdkPR->GetHighestBrawlKills()
	);

	UpdateMostLabel(
		"MostStreak",
		"MostStreakPlayer",
		"#DA_ScoreBoard_KillStreak",
		sdkPR->GetHighestKillStreakPlayer(),
		sdkPR->GetHighestKillStreak()
	);

	UpdateMostLabel(
		"MostGrenade",
		"MostGrenadePlayer",
		"#DA_ScoreBoard_GrenadeKills",
		sdkPR->GetHighestGrenadeKillPlayer(),
		sdkPR->GetHighestGrenadeKills()
	);

	Label *pTimeLimitLabel = dynamic_cast<Label *>(FindChildByName("TimeLimit"));
	if (pTimeLimitLabel)
	{
		int iTimeElapsed = SDKGameRules()->GetMapElapsedTime() / 60;
		int iTimeLimit = mp_timelimit.GetInt();

		const char *printFormatString;
		if (iTimeLimit > 0)
			printFormatString = "#DA_ScoreBoard_TimeElapsedLimited";
		else
			printFormatString = "#DA_ScoreBoard_TimeElapsed";

		wchar_t wszTimeElapsed[20];
		V_swprintf_safe(wszTimeElapsed, L"%i", iTimeElapsed);

		wchar_t wszTimeLimit[20];
		V_swprintf_safe(wszTimeLimit, L"%i", iTimeLimit);

		wchar_t wszNewLabel[100];
		g_pVGuiLocalize->ConstructString(wszNewLabel, sizeof(wszNewLabel), g_pVGuiLocalize->Find(printFormatString), 2, wszTimeElapsed, wszTimeLimit);

		pTimeLimitLabel->SetText(wszNewLabel);
	}
}

void CSDKScoreboard::UpdateMostLabel(const char * pszMostLabelName, const char * pszMostLabelPlayerName, const char * pszMostLabelText, int iHighestPlayerIndex, int iHighestValue)
{
	Label *pMostLabel = dynamic_cast<Label *>(FindChildByName(pszMostLabelName));
	if (pMostLabel)
	{
		pMostLabel->SetText(pszMostLabelText);

		if (iHighestValue != -1)
		{
			wchar_t wszLabel[100];
			pMostLabel->GetText(wszLabel, sizeof(wszLabel));

			wchar_t wszNewLabel[100];
			V_swprintf_safe(wszNewLabel, L"%s %i", wszLabel, iHighestValue);

			pMostLabel->SetText(wszNewLabel);
		}
	}

	Label *pMostLabelPlayer = dynamic_cast<Label *>(FindChildByName(pszMostLabelPlayerName));
	if (pMostLabelPlayer)
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(iHighestPlayerIndex));
		if (pPlayer)
			pMostLabelPlayer->SetText(pPlayer->GetPlayerName());
		else
			pMostLabelPlayer->SetText("");
	}
}


Panel *CSDKScoreboard::CreateControlByName( const char *controlName )
{
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

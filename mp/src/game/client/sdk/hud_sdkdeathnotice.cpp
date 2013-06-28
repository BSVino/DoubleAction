//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//====================================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_sdk_player_resource.h"
#include "iclientmode.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>


#include "c_sdk_player.h"
#include "c_sdk_team.h"
#include "clientmode_sdk.h"

#include "hud_basedeathnotice.h"

#include "engine/ienginesound.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class SDKHudDeathNotice : public CHudBaseDeathNotice
{
	DECLARE_CLASS_SIMPLE( SDKHudDeathNotice, CHudBaseDeathNotice );
public:
	SDKHudDeathNotice( const char *pElementName ) : CHudBaseDeathNotice( pElementName ) {};
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual bool IsVisible( void );

protected:	
	virtual void OnGameEvent( IGameEvent *event, DeathNoticeItem &msg );
	virtual Color GetTeamColor( int iTeamNumber );

private:
	void AddAdditionalMsg( int iKillerID, int iVictimID, const char *pMsgKey );

	CHudTexture		*m_iconDomination;

#if defined ( SDK_USE_TEAMS )
	CPanelAnimationVar( Color, m_clrBlueText, "TeamBlue", "153 204 255 255" );
	CPanelAnimationVar( Color, m_clrRedText, "TeamRed", "255 64 64 255" );
#endif

};

DECLARE_HUDELEMENT( SDKHudDeathNotice );

void SDKHudDeathNotice::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_iconDomination = gHUD.GetIcon( "leaderboard_dominated" );
}

bool SDKHudDeathNotice::IsVisible( void )
{
	return BaseClass::IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: Called when a game event happens and a death notice is about to be 
//			displayed.  This method can examine the event and death notice and
//			make game-specific tweaks to it before it is displayed
//-----------------------------------------------------------------------------
void SDKHudDeathNotice::OnGameEvent( IGameEvent *event, DeathNoticeItem &msg )
{
	const char *pszEventName = event->GetName();

	if ( FStrEq( pszEventName, "player_death" ))
	{
		int iCustomDamage = event->GetInt( "customkill" );
		int iLocalPlayerIndex = GetLocalPlayerIndex();

		// if there was an assister, put both the killer's and assister's names in the death message
		int iAssisterID = engine->GetPlayerForUserID( event->GetInt( "assister" ) );
		const char *assister_name = ( iAssisterID > 0 ? g_PR->GetPlayerName( iAssisterID ) : NULL );
		if ( assister_name )
		{
			char szKillerBuf[MAX_PLAYER_NAME_LENGTH*2];
			Q_snprintf( szKillerBuf, ARRAYSIZE(szKillerBuf), "%s + %s", msg.Killer.szName, assister_name );
			Q_strncpy( msg.Killer.szName, szKillerBuf, ARRAYSIZE( msg.Killer.szName ) );
			if ( iLocalPlayerIndex == iAssisterID )
			{
				msg.bLocalPlayerInvolved = true;
			}
		}

		// if this death involved a player dominating another player or getting revenge on another player, add an additional message
		// mentioning that
		int iKillerID = engine->GetPlayerForUserID( event->GetInt( "attacker" ) );
		int iVictimID = engine->GetPlayerForUserID( event->GetInt( "userid" ) );

		if ( event->GetInt( "dominated" ) > 0 )
		{
			AddAdditionalMsg( iKillerID, iVictimID, "#Msg_Dominating" );
		}
		if ( event->GetInt( "assister_dominated" ) > 0 && ( iAssisterID > 0 ) )
		{
			AddAdditionalMsg( iAssisterID, iVictimID, "#Msg_Dominating" );
		}
		if ( event->GetInt( "revenge" ) > 0 ) 
		{
			AddAdditionalMsg( iKillerID, iVictimID, "#Msg_Revenge" );
		}
		if ( event->GetInt( "assister_revenge" ) > 0 && ( iAssisterID > 0 ) ) 
		{
			AddAdditionalMsg( iAssisterID, iVictimID, "#Msg_Revenge" );
		}

		const wchar_t *pMsg = NULL;
		switch ( iCustomDamage )
		{
		case SDK_DMG_CUSTOM_SUICIDE:
			{
				// display a different message if this was suicide, or assisted suicide (suicide w/recent damage, kill awarded to damager)
				bool bAssistedSuicide = event->GetInt( "userid" ) != event->GetInt( "attacker" );
				pMsg = g_pVGuiLocalize->Find( bAssistedSuicide ? "#DeathMsg_AssistedSuicide" : "#DeathMsg_Suicide" );
				if ( pMsg )
				{
					V_wcsncpy( msg.wzInfoText, pMsg, sizeof( msg.wzInfoText ) );
				}			
				break;
			}
		default:
			break;
		}
	} 
}

//-----------------------------------------------------------------------------
// Purpose: Adds an additional death message
//-----------------------------------------------------------------------------
void SDKHudDeathNotice::AddAdditionalMsg( int iKillerID, int iVictimID, const char *pMsgKey )
{
	DeathNoticeItem &msg2 = m_DeathNotices[AddDeathNoticeItem()];
	Q_strncpy( msg2.Killer.szName, g_PR->GetPlayerName( iKillerID ), ARRAYSIZE( msg2.Killer.szName ) );
	Q_strncpy( msg2.Victim.szName, g_PR->GetPlayerName( iVictimID ), ARRAYSIZE( msg2.Victim.szName ) );
	const wchar_t *wzMsg =  g_pVGuiLocalize->Find( pMsgKey );
	if ( wzMsg )
	{
		V_wcsncpy( msg2.wzInfoText, wzMsg, sizeof( msg2.wzInfoText ) );
	}
	msg2.iconDeath = m_iconDomination;
	int iLocalPlayerIndex = GetLocalPlayerIndex();
	if ( iLocalPlayerIndex == iVictimID || iLocalPlayerIndex == iKillerID )
	{
		msg2.bLocalPlayerInvolved = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the color to draw text in for this team.  
//-----------------------------------------------------------------------------
Color SDKHudDeathNotice::GetTeamColor( int iTeamNumber )
{
	switch ( iTeamNumber )
	{
#if defined ( SDK_USE_TEAMS )
	case SDK_TEAM_BLUE:
		return m_clrBlueText;
		break;
	case SDK_TEAM_RED:
		return m_clrRedText;
		break;
#endif
	case TEAM_UNASSIGNED:		
		return Color( 255, 255, 255, 255 );
		break;
	default:
		AssertOnce( false );	// invalid team
		return Color( 255, 255, 255, 255 );
		break;
	}
}

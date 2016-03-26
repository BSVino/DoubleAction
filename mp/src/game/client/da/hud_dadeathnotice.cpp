//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//====================================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_da_player_resource.h"
#include "iclientmode.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>


#include "c_da_player.h"
#include "c_da_team.h"
#include "clientmode_da.h"

#include "hud_basedeathnotice.h"

#include "engine/IEngineSound.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class SDKHudDeathNotice : public CHudBaseDeathNotice
{
	DECLARE_CLASS_SIMPLE( SDKHudDeathNotice, CHudBaseDeathNotice );
public:
	SDKHudDeathNotice( const char *pElementName ) : CHudBaseDeathNotice( pElementName ) {};
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual bool IsVisible( void );

	virtual void Paint( void );

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

void SDKHudDeathNotice::Paint()
{
	// Retire any death notices that have expired
	RetireExpiredDeathNotices();

	CBaseViewport *pViewport = dynamic_cast<CBaseViewport *>( GetClientModeNormal()->GetViewport() );
	int yStart = pViewport->GetDeathMessageStartHeight();

	surface()->DrawSetTextFont( m_hTextFont );

	int xMargin = XRES( 10 );
	int xSpacing = UTIL_ComputeStringWidth( m_hTextFont, L" " );

	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		DeathNoticeItem &msg = m_DeathNotices[i];
		
		CHudTexture *icon = msg.iconDeath;
						
		wchar_t victim[256]=L"";
		wchar_t killer[256]=L"";

		// TEMP - print the death icon name if we don't have a material for it

		g_pVGuiLocalize->ConvertANSIToUnicode( msg.Victim.szName, victim, sizeof( victim ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( msg.Killer.szName, killer, sizeof( killer ) );

		int iVictimTextWide = UTIL_ComputeStringWidth( m_hTextFont, victim ) + xSpacing;
		int iDeathInfoTextWide= msg.wzInfoText[0] ? UTIL_ComputeStringWidth( m_hTextFont, msg.wzInfoText ) + xSpacing : 0;
		int iKillerTextWide = killer[0] ? UTIL_ComputeStringWidth( m_hTextFont, killer ) + xSpacing : 0;
		int iLineTall = m_flLineHeight;
		int iTextTall = surface()->GetFontTall( m_hTextFont );
		int iconWide = 0, iconTall = 0, iVictimTextOffset = 0, iconActualWide = 0;

		wchar_t* pwszWeapon = NULL;

		if (msg.szIcon[0])
			pwszWeapon = g_pVGuiLocalize->Find( VarArgs("#DA_Weapon_Obituary_%s", msg.szIcon+2) );

		if (!pwszWeapon)
			pwszWeapon = L"x.x";

		wchar_t wszWeaponName[20];
		V_wcsncpy( wszWeaponName, pwszWeapon, sizeof( wszWeaponName ) );

		int iWeaponTextWide = msg.szIcon[0] ? UTIL_ComputeStringWidth( m_hTextFont, wszWeaponName ) + xSpacing : 0;

		// Get the local position for this notice
		if ( icon )
		{			
			iconActualWide = icon->EffectiveWidth( 1.0f );
			iconWide = iconActualWide + xSpacing;
			iconTall = icon->EffectiveHeight( 1.0f );
			
			int iconTallDesired = iLineTall-YRES(2);
			Assert( 0 != iconTallDesired );
			float flScale = (float) iconTallDesired / (float) iconTall;

			iconActualWide *= flScale;
			iconTall *= flScale;
			iconWide *= flScale;
		}
		int iTotalWide = iKillerTextWide + iWeaponTextWide + iVictimTextWide + iDeathInfoTextWide + ( xMargin * 2 );
		int y = yStart + ( ( iLineTall + m_flLineSpacing ) * i );				
		int yText = y + ( ( iLineTall - iTextTall ) / 2 );

		int x=0;
		if ( m_bRightJustify )
		{
			x =	GetWide() - iTotalWide;
		}

		// draw a background panel for the message
		Vertex_t vert[NUM_BACKGROUND_COORD];
		GetBackgroundPolygonVerts( x, y+1, x+iTotalWide, y+iLineTall-1, ARRAYSIZE( vert ), vert );		
		surface()->DrawSetTexture( -1 );
		surface()->DrawSetColor( msg.bLocalPlayerInvolved ? m_clrLocalBGColor : m_clrBaseBGColor );
		surface()->DrawTexturedPolygon( ARRAYSIZE( vert ), vert );

		x += xMargin;
			
		if ( killer[0] )
		{
			// Draw killer's name
			DrawText( x, yText, m_hTextFont, GetTeamColor( msg.Killer.iTeam ), killer );
			x += iKillerTextWide;
		}

		if ( wszWeaponName[0] )
		{
			DrawText( x, yText, m_hTextFont, Color( 255, 205, 55, 255 ), wszWeaponName );

			x += iWeaponTextWide;
		}

		// Draw victims name
		DrawText( x + iVictimTextOffset, yText, m_hTextFont, GetTeamColor( msg.Victim.iTeam ), victim );
		x += iVictimTextWide;
	}
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
		if (event->GetBool("grenade"))
			V_strcpy_safe( msg.szIcon, "d_grenade" );
		else if (event->GetBool("brawl"))
			V_strcpy_safe( msg.szIcon, "d_brawl" );

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
	case SDK_TEAM_DEATHMATCH:		
		return Color( 255, 255, 255, 255 );
		break;
	default:
		AssertOnce( false );	// invalid team
		return Color( 255, 255, 255, 255 );
		break;
	}
}

//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "sdk_hud_chat.h"
#include "c_sdk_player_resource.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include <vgui_controls/Controls.h>
#include <vgui/IVgui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include "engine/IEngineSound.h"
#include "c_sdk_team.h"
#include "c_playerresource.h"
#include "c_sdk_player.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudChat );

DECLARE_HUD_MESSAGE( CHudChat, SayText );
DECLARE_HUD_MESSAGE( CHudChat, SayText2 );
DECLARE_HUD_MESSAGE( CHudChat, TextMsg );

using namespace vgui;
//=====================
//CHudChatLine
//=====================

void CHudChatLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_hFont = pScheme->GetFont( "ChatFont" );
	SetBorder( NULL );
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );

	SetFont( m_hFont );
}

CHudChatLine::CHudChatLine( vgui::Panel *parent, const char *panelName ) : CBaseHudChatLine( parent, panelName )
{
	m_text = NULL;
}


//=====================
//CHudChatInputLine
//=====================

void CHudChatInputLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}



//=====================
//CHudChat
//=====================

CHudChat::CHudChat( const char *pElementName ) : BaseClass( pElementName )
{
	
}

void CHudChat::CreateChatInputLine( void )
{
	m_pChatInput = new CHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );
}

void CHudChat::CreateChatLines( void )
{
	m_ChatLine = new CHudChatLine( this, "ChatLine1" );
	m_ChatLine->SetVisible( false );		
}


void CHudChat::Init( void )
{
	BaseClass::Init();

	HOOK_HUD_MESSAGE( CHudChat, SayText );
	HOOK_HUD_MESSAGE( CHudChat, SayText2 );
	HOOK_HUD_MESSAGE( CHudChat, TextMsg );
}

//-----------------------------------------------------------------------------
// Purpose: Hides us when our render group is hidden
// move render group to base chat when its safer
//-----------------------------------------------------------------------------
bool CHudChat::ShouldDraw( void )
{
	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: Overrides base reset to not cancel chat at round restart
//-----------------------------------------------------------------------------
void CHudChat::Reset( void )
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CHudChat::GetChatInputOffset( void )
{
	if ( m_pChatInput->IsVisible() )
	{
		return m_iFontHeight;
	}
	else
	{
		return 0;
	}
}

int CHudChat::GetFilterForString( const char *pString )
{
	int iFilter = BaseClass::GetFilterForString( pString );

	if ( iFilter == CHAT_FILTER_NONE )
	{
		if ( !Q_stricmp( pString, "#HL_Name_Change" ) ) 
		{
			return CHAT_FILTER_NAMECHANGE;
		}
	}

	return iFilter;
}

//-----------------------------------------------------------------------------
Color CHudChat::GetClientColor( int clientIndex )
{
	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );

	if ( pScheme == NULL )
		return Color( 255, 255, 255, 255 );

	if ( clientIndex == 0 ) // console msg
	{
		return g_ColorGreen;
	}
	else if( g_PR )
	{
		int iTeam = g_PR->GetTeam( clientIndex );

		C_SDK_PlayerResource *sdk_PR = dynamic_cast<C_SDK_PlayerResource *>(g_PR);
		if ( !sdk_PR )
			return g_ColorGrey;

		return sdk_PR->GetTeamColor( iTeam );
	}

	return g_ColorYellow;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudChat::IsVisible( void )
{
	return BaseClass::IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color CHudChat::GetTextColorForClient( TextColor colorNum, int clientIndex )
{
	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );

	if ( pScheme == NULL )
		return Color( 255, 255, 255, 255 );

	Color c;
	switch ( colorNum )
	{
	case COLOR_PLAYERNAME:
		c = GetClientColor( clientIndex );
		break;

	case COLOR_LOCATION:
		c = g_ColorDarkGreen;
		break;

	case COLOR_ACHIEVEMENT:
		{
			IScheme *pSourceScheme = scheme()->GetIScheme( scheme()->GetScheme( "SourceScheme" ) ); 
			if ( pSourceScheme )
			{
				c = pSourceScheme->GetColor( "SteamLightGreen", GetBgColor() );
			}
			else
			{
				c = g_ColorYellow;
			}
		}
		break;
	default:
		c = g_ColorYellow;
	}

	return Color( c[0], c[1], c[2], 255 );
}



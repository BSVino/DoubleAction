//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_sdk_player.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "sdk_gamerules.h"
#include "../c_da_briefcase.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

static ConVar hud_centerid( "hud_centerid", "1" );
static ConVar hud_showtargetid( "hud_showtargetid", "1" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSDKTargetId : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CSDKTargetId, vgui::Panel );

public:
	CSDKTargetId( const char *pElementName );
	void Init( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void VidInit( void );

private:
	Color			GetColorForTargetTeam( int iTeamNumber );

	CPanelAnimationVar( vgui::HFont, m_hFont, "TargetIDFont", "Default" );

	int				m_iLastEntIndex;
	float			m_flLastChangeTime;

	CHudTexture*    m_pBriefcase;
	CHudTexture*    m_pCapturePoint;
	CHudTexture*    m_pBounty;

	typedef enum
	{
		TARGET_BRIEFCASE = 0,
		TARGET_CAPTURE,
		TARGET_BOUNTY,
		TARGET_WAYPOINT1,
		TARGET_WAYPOINT2,
		TARGET_WAYPOINT3,
		TARGET_LEADER,
		TARGET_FRONTRUNNER1,
		TARGET_FRONTRUNNER2,
		TARGET_TOTAL,
	} target_type_t;

	class CTarget
	{
	public:
		CTarget()
		{
			m_bHideIfVisible = true;
		}

	public:
		CHudTexture*    m_pTargetTexture;
		wchar_t*        m_pwszHint;
		float           m_flTargetAlpha;
		float           m_flScale;
		float           m_flMaxAlpha;
		Vector          m_vecLastKnownTarget;
		bool            m_bTargetOn;
		EHANDLE         m_hEntity;
		bool            m_bHideIfVisible;
	} m_Targets[TARGET_TOTAL];

	CPanelAnimationVar( vgui::HFont, m_hMiniObjectiveFont, "MiniObjectiveFont", "Default" );
};

DECLARE_HUDELEMENT( CSDKTargetId );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKTargetId::CSDKTargetId( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	m_pBriefcase = NULL;
	m_pCapturePoint = NULL;
	m_pBounty = NULL;

	for (int i = 0; i < TARGET_TOTAL; i++)
	{
		m_Targets[i].m_pTargetTexture = NULL;
		m_Targets[i].m_pwszHint = NULL;
		m_Targets[i].m_flTargetAlpha = 0;
		m_Targets[i].m_bTargetOn = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CSDKTargetId::Init( void )
{
};

void CSDKTargetId::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont( "TargetID", IsProportional() );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CSDKTargetId::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

Color CSDKTargetId::GetColorForTargetTeam( int iTeamNumber )
{
	return GameResources()->GetTeamColor( iTeamNumber );
} 

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CSDKTargetId::Paint()
{
	if (!C_SDKPlayer::GetLocalSDKPlayer())
		return;

	if (!m_pBriefcase)
	{
		m_pBriefcase = gHUD.GetIcon("briefcase");
		m_pCapturePoint = gHUD.GetIcon("capturezone");
		m_pBounty = gHUD.GetIcon("bounty");
	}

	if (SDKGameRules()->GetBriefcase() && SDKGameRules()->GetBriefcase()->GetOwnerEntity())
	{
		C_BriefcaseCaptureZone* pBriefcase = SDKGameRules()->GetCaptureZone();
		m_Targets[TARGET_CAPTURE].m_vecLastKnownTarget = pBriefcase->WorldSpaceCenter();
		m_Targets[TARGET_CAPTURE].m_hEntity = pBriefcase;

		m_Targets[TARGET_CAPTURE].m_pTargetTexture = m_pCapturePoint;

		m_Targets[TARGET_CAPTURE].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_Capture");

		m_Targets[TARGET_CAPTURE].m_flScale = 0.5f;
		m_Targets[TARGET_CAPTURE].m_flMaxAlpha = 0.5f;

		m_Targets[TARGET_CAPTURE].m_bTargetOn = true;
	}
	else
		m_Targets[TARGET_CAPTURE].m_bTargetOn = false;

	if (m_pBriefcase && SDKGameRules()->GetBriefcase())
	{
		C_Briefcase* pBriefcase = SDKGameRules()->GetBriefcase();
		m_Targets[TARGET_BRIEFCASE].m_vecLastKnownTarget = pBriefcase->WorldSpaceCenter();
		m_Targets[TARGET_BRIEFCASE].m_hEntity = pBriefcase;

		m_Targets[TARGET_BRIEFCASE].m_pTargetTexture = m_pBriefcase;

		m_Targets[TARGET_BRIEFCASE].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_Retrieve");

		m_Targets[TARGET_BRIEFCASE].m_flScale = 0.7f;
		m_Targets[TARGET_BRIEFCASE].m_flMaxAlpha = 0.7f;

		m_Targets[TARGET_BRIEFCASE].m_bTargetOn = true;
	}
	else
		m_Targets[TARGET_BRIEFCASE].m_bTargetOn = false;

	if (SDKGameRules()->GetBountyPlayer() && SDKGameRules()->GetBountyPlayer() != C_SDKPlayer::GetLocalSDKPlayer())
	{
		C_SDKPlayer* pBounty = SDKGameRules()->GetBountyPlayer();
		m_Targets[TARGET_BOUNTY].m_vecLastKnownTarget = pBounty->WorldSpaceCenter();
		m_Targets[TARGET_BOUNTY].m_hEntity = pBounty;

		m_Targets[TARGET_BOUNTY].m_pTargetTexture = m_pBounty;

		m_Targets[TARGET_BOUNTY].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_Bounty");

		m_Targets[TARGET_BOUNTY].m_flScale = 0.7f;
		m_Targets[TARGET_BOUNTY].m_flMaxAlpha = 0.5f;

		m_Targets[TARGET_BOUNTY].m_bTargetOn = true;
	}
	else
		m_Targets[TARGET_BOUNTY].m_bTargetOn = false;

	if (SDKGameRules()->GetWaypoint(0))
	{
		if (C_SDKPlayer::GetLocalSDKPlayer()->GetRaceWaypoint() == 0)
		{
			m_Targets[TARGET_WAYPOINT1].m_vecLastKnownTarget = SDKGameRules()->GetWaypoint(0)->WorldSpaceCenter();
			m_Targets[TARGET_WAYPOINT1].m_hEntity = SDKGameRules()->GetWaypoint(0);

			m_Targets[TARGET_WAYPOINT1].m_pTargetTexture = m_pCapturePoint;

			m_Targets[TARGET_WAYPOINT1].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Waypoint1");

			m_Targets[TARGET_WAYPOINT1].m_flScale = 0.5f;
			m_Targets[TARGET_WAYPOINT1].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_WAYPOINT1].m_bTargetOn = true;
			m_Targets[TARGET_WAYPOINT1].m_bHideIfVisible = false;
		}
		else
			m_Targets[TARGET_WAYPOINT1].m_bTargetOn = false;

		if (C_SDKPlayer::GetLocalSDKPlayer()->GetRaceWaypoint() == 1 || SDKGameRules()->GetLeader() && SDKGameRules()->GetLeader()->GetRaceWaypoint() == 1)
		{
			m_Targets[TARGET_WAYPOINT2].m_vecLastKnownTarget = SDKGameRules()->GetWaypoint(1)->WorldSpaceCenter();
			m_Targets[TARGET_WAYPOINT2].m_hEntity = SDKGameRules()->GetWaypoint(1);

			m_Targets[TARGET_WAYPOINT2].m_pTargetTexture = m_pCapturePoint;

			m_Targets[TARGET_WAYPOINT2].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Waypoint2");

			m_Targets[TARGET_WAYPOINT2].m_flScale = 0.5f;
			m_Targets[TARGET_WAYPOINT2].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_WAYPOINT2].m_bTargetOn = true;
			m_Targets[TARGET_WAYPOINT2].m_bHideIfVisible = false;
		}
		else
			m_Targets[TARGET_WAYPOINT2].m_bTargetOn = false;

		if (C_SDKPlayer::GetLocalSDKPlayer()->GetRaceWaypoint() == 2 || SDKGameRules()->GetLeader() && SDKGameRules()->GetLeader()->GetRaceWaypoint() == 2)
		{
			m_Targets[TARGET_WAYPOINT3].m_vecLastKnownTarget = SDKGameRules()->GetWaypoint(2)->WorldSpaceCenter();
			m_Targets[TARGET_WAYPOINT3].m_hEntity = SDKGameRules()->GetWaypoint(1);

			m_Targets[TARGET_WAYPOINT3].m_pTargetTexture = m_pCapturePoint;

			m_Targets[TARGET_WAYPOINT3].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Waypoint3");

			m_Targets[TARGET_WAYPOINT3].m_flScale = 0.5f;
			m_Targets[TARGET_WAYPOINT3].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_WAYPOINT3].m_bTargetOn = true;
			m_Targets[TARGET_WAYPOINT3].m_bHideIfVisible = false;
		}
		else
			m_Targets[TARGET_WAYPOINT3].m_bTargetOn = false;

		if (SDKGameRules()->GetLeader() && SDKGameRules()->GetLeader() != C_SDKPlayer::GetLocalSDKPlayer())
		{
			C_SDKPlayer* pLeader = SDKGameRules()->GetLeader();
			m_Targets[TARGET_LEADER].m_vecLastKnownTarget = pLeader->WorldSpaceCenter();
			m_Targets[TARGET_LEADER].m_hEntity = pLeader;

			m_Targets[TARGET_LEADER].m_pTargetTexture = m_pBounty;

			m_Targets[TARGET_LEADER].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Leader");

			m_Targets[TARGET_LEADER].m_flScale = 0.7f;
			m_Targets[TARGET_LEADER].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_LEADER].m_bTargetOn = true;
		}
		else
			m_Targets[TARGET_LEADER].m_bTargetOn = false;

		if (SDKGameRules()->GetFrontRunner1() && SDKGameRules()->GetFrontRunner1() != C_SDKPlayer::GetLocalSDKPlayer())
		{
			C_SDKPlayer* pLeader = SDKGameRules()->GetFrontRunner1();
			m_Targets[TARGET_FRONTRUNNER1].m_vecLastKnownTarget = pLeader->WorldSpaceCenter();
			m_Targets[TARGET_FRONTRUNNER1].m_hEntity = pLeader;

			m_Targets[TARGET_FRONTRUNNER1].m_pTargetTexture = m_pBounty;

			m_Targets[TARGET_FRONTRUNNER1].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Frontrunner1");

			m_Targets[TARGET_FRONTRUNNER1].m_flScale = 0.7f;
			m_Targets[TARGET_FRONTRUNNER1].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_FRONTRUNNER1].m_bTargetOn = true;
		}
		else
			m_Targets[TARGET_FRONTRUNNER1].m_bTargetOn = false;

		if (SDKGameRules()->GetFrontRunner2() && SDKGameRules()->GetFrontRunner2() != C_SDKPlayer::GetLocalSDKPlayer())
		{
			C_SDKPlayer* pLeader = SDKGameRules()->GetFrontRunner2();
			m_Targets[TARGET_FRONTRUNNER2].m_vecLastKnownTarget = pLeader->WorldSpaceCenter();
			m_Targets[TARGET_FRONTRUNNER2].m_hEntity = pLeader;

			m_Targets[TARGET_FRONTRUNNER2].m_pTargetTexture = m_pBounty;

			m_Targets[TARGET_FRONTRUNNER2].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Frontrunner2");

			m_Targets[TARGET_FRONTRUNNER2].m_flScale = 0.7f;
			m_Targets[TARGET_FRONTRUNNER2].m_flMaxAlpha = 0.5f;

			m_Targets[TARGET_FRONTRUNNER2].m_bTargetOn = true;
		}
		else
			m_Targets[TARGET_FRONTRUNNER2].m_bTargetOn = false;
	}
	else
	{
		m_Targets[TARGET_WAYPOINT1].m_bTargetOn = false;
		m_Targets[TARGET_WAYPOINT2].m_bTargetOn = false;
		m_Targets[TARGET_WAYPOINT3].m_bTargetOn = false;
		m_Targets[TARGET_LEADER].m_bTargetOn = false;
		m_Targets[TARGET_FRONTRUNNER1].m_bTargetOn = false;
		m_Targets[TARGET_FRONTRUNNER2].m_bTargetOn = false;
	}

	int iX, iY;

	for (int i = 0; i < TARGET_TOTAL; i++)
	{
		if (!m_Targets[i].m_bTargetOn && m_Targets[i].m_flTargetAlpha == 0)
			continue;

		float flAlphaGoal = 1;

		if (GetVectorInHudSpace(m_Targets[i].m_vecLastKnownTarget, iX, iY))
		{
			C_BaseEntity* pTarget = m_Targets[i].m_hEntity;

			bool bHide;
			if (!m_Targets[i].m_bTargetOn)
				bHide = true;
			else if (pTarget)
			{
				if (m_Targets[i].m_bHideIfVisible)
				{
					trace_t tr;
					UTIL_TraceLine(CurrentViewOrigin(), m_Targets[i].m_vecLastKnownTarget, MASK_BLOCKLOS, C_SDKPlayer::GetLocalSDKPlayer(), COLLISION_GROUP_NONE, &tr);

					bHide = tr.fraction >= 0.99f || tr.m_pEnt == pTarget;
					if ((CurrentViewOrigin() - pTarget->WorldSpaceCenter()).LengthSqr() > 1000*1000)
						bHide = false;
				}
				else
					bHide = false;
			}
			else
				bHide = true;

			if (!bHide)
				flAlphaGoal = m_Targets[i].m_flMaxAlpha;
			else
				flAlphaGoal = 0;
		}
		else
		{
			if (!m_Targets[i].m_bTargetOn)
				flAlphaGoal = 0;
		}

		m_Targets[i].m_flTargetAlpha = Approach(flAlphaGoal, m_Targets[i].m_flTargetAlpha, gpGlobals->frametime * 2);

		if (m_Targets[i].m_flTargetAlpha > 0)
		{
			int iWidth = m_Targets[i].m_pTargetTexture->EffectiveWidth(m_Targets[i].m_flScale);
			int iHeight = m_Targets[i].m_pTargetTexture->EffectiveHeight(m_Targets[i].m_flScale);
			m_Targets[i].m_pTargetTexture->DrawSelf(iX - iWidth/2, iY - iHeight/2, iWidth, iHeight, Color(255, 255, 255, 255 * m_Targets[i].m_flTargetAlpha));

			if (m_Targets[i].m_pwszHint)
			{
				int iHintWide, iHintTall;
				surface()->GetTextSize(m_hMiniObjectiveFont, m_Targets[i].m_pwszHint, iHintWide, iHintTall);

				vgui::surface()->DrawSetTextFont( m_hMiniObjectiveFont );
				vgui::surface()->DrawSetTextPos( iX - iHintWide/2, iY + iHeight/2 );
				vgui::surface()->DrawSetTextColor( Color(255, 255, 255, 255 * m_Targets[i].m_flTargetAlpha) );
				vgui::surface()->DrawPrintText( m_Targets[i].m_pwszHint, wcslen(m_Targets[i].m_pwszHint) );
			}
		}
	}

#define MAX_ID_STRING 256
	wchar_t sIDString[ MAX_ID_STRING ];
	sIDString[0] = 0;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();

	if ( !pPlayer )
		return;

	Color c;

	// Get our target's ent index
	int iEntIndex = pPlayer->GetIDTarget();
	// Didn't find one?
	if ( !iEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) )
		{
			m_flLastChangeTime = 0;
			sIDString[0] = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if ( iEntIndex )
	{
		C_BasePlayer *pPlayer = static_cast<C_BasePlayer*>(cl_entitylist->GetEnt( iEntIndex ));
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

		const char *printFormatString = NULL;
		wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
		wchar_t wszHealthText[ 10 ];
		bool bShowHealth = false;
		bool bShowPlayerName = false;

		// Some entities we always want to check, cause the text may change
		// even while we're looking at it
		// Is it a player?
		if ( IsPlayerIndex( iEntIndex ) )
		{
			c = GetColorForTargetTeam( pPlayer->GetTeamNumber() );

			bShowPlayerName = true;
			g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  wszPlayerName, sizeof(wszPlayerName) );
			
			if ( SDKGameRules()->IsTeamplay() == true && pPlayer->InSameTeam(pLocalPlayer) )
			{
				printFormatString = "#DA_Playerid_sameteam";
				bShowHealth = true;
			}
			else
			{
				printFormatString = "#DA_Playerid_diffteam";
			}
		

			if ( bShowHealth )
			{
				_snwprintf( wszHealthText, ARRAYSIZE(wszHealthText) - 1, L"%.0f%%",  ((float)pPlayer->GetHealth() / (float)pPlayer->GetMaxHealth() ) );
				wszHealthText[ ARRAYSIZE(wszHealthText)-1 ] = '\0';
			}
		}

		if ( printFormatString )
		{
			if ( bShowPlayerName && bShowHealth )
			{
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 2, wszPlayerName, wszHealthText );
			}
			else if ( bShowPlayerName )
			{
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 1, wszPlayerName );
			}
			else if ( bShowHealth )
			{
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 1, wszHealthText );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 0 );
			}
		}

		if ( sIDString[0] )
		{
			int wide, tall;
			int ypos = YRES(260);
			int xpos = XRES(10);

			vgui::surface()->GetTextSize( m_hFont, sIDString, wide, tall );

			if( hud_centerid.GetInt() == 0 )
			{
				ypos = YRES(420);
			}
			else
			{
				xpos = (ScreenWidth() - wide) / 2;
			}
			
			vgui::surface()->DrawSetTextFont( m_hFont );
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawSetTextColor( c );
			vgui::surface()->DrawPrintText( sIDString, wcslen(sIDString) );
		}
	}
}

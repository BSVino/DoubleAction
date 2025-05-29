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
#include "sdk_hud_targetid.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern float Oscillate(float flTime, float flLength);

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

static ConVar hud_centerid( "hud_centerid", "1" );
static ConVar hud_showtargetid( "hud_showtargetid", "1" );

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
	C_SDKPlayer* pSDKPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if (!pSDKPlayer)
		return;

	if (pSDKPlayer->GetObserverMode() == OBS_MODE_FREEZECAM)
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
		m_Targets[TARGET_CAPTURE].m_hFont = m_hMiniObjectiveFont;

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
		m_Targets[TARGET_BRIEFCASE].m_hFont = m_hMiniObjectiveFont;

		if (SDKGameRules()->IsTeamplay() && pBriefcase->GetOwnerEntity() && ToSDKPlayer(pBriefcase->GetOwnerEntity())
			&& ToSDKPlayer(pBriefcase->GetOwnerEntity())->GetTeamNumber() == C_SDKPlayer::GetLocalSDKPlayer()->GetTeamNumber())
			m_Targets[TARGET_BRIEFCASE].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_Protect");
		else
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
		m_Targets[TARGET_BOUNTY].m_hFont = m_hMiniObjectiveFont;

		if (SDKGameRules()->IsTeamplay() && pBounty->GetTeamNumber() == C_SDKPlayer::GetLocalSDKPlayer()->GetTeamNumber())
			m_Targets[TARGET_BOUNTY].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_Protect");
		else
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
			m_Targets[TARGET_WAYPOINT1].m_hFont = m_hMiniObjectiveFont;

			m_Targets[TARGET_WAYPOINT1].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Checkpoint");

			m_Targets[TARGET_WAYPOINT1].m_flScale = 0.5f;
			m_Targets[TARGET_WAYPOINT1].m_flMaxAlpha = 0.5f * Oscillate(gpGlobals->curtime, 1);

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

			if (C_SDKPlayer::GetLocalSDKPlayer()->GetRaceWaypoint() == 1)
			{
				m_Targets[TARGET_WAYPOINT2].m_flMaxAlpha = 0.5f * Oscillate(gpGlobals->curtime, 1);
				m_Targets[TARGET_WAYPOINT2].m_flScale = 0.5f;
				m_Targets[TARGET_WAYPOINT2].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Checkpoint");
				m_Targets[TARGET_WAYPOINT2].m_hFont = m_hMiniObjectiveFont;
			}
			else
			{
				m_Targets[TARGET_WAYPOINT2].m_flMaxAlpha = 0.3f;
				m_Targets[TARGET_WAYPOINT2].m_flScale = 0.3f;
				m_Targets[TARGET_WAYPOINT2].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Waypoint2");
				m_Targets[TARGET_WAYPOINT2].m_hFont = m_hMiniObjectiveFontSmall;
			}

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

			if (C_SDKPlayer::GetLocalSDKPlayer()->GetRaceWaypoint() == 2)
			{
				m_Targets[TARGET_WAYPOINT3].m_flMaxAlpha = 0.5f * Oscillate(gpGlobals->curtime, 1);
				m_Targets[TARGET_WAYPOINT3].m_flScale = 0.5f;
				m_Targets[TARGET_WAYPOINT3].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Checkpoint");
				m_Targets[TARGET_WAYPOINT3].m_hFont = m_hMiniObjectiveFont;
			}
			else
			{
				m_Targets[TARGET_WAYPOINT3].m_flMaxAlpha = 0.3f;
				m_Targets[TARGET_WAYPOINT3].m_flScale = 0.3f;
				m_Targets[TARGET_WAYPOINT3].m_pwszHint = g_pVGuiLocalize->Find("#DA_MiniObjective_RatRace_Waypoint3");
				m_Targets[TARGET_WAYPOINT3].m_hFont = m_hMiniObjectiveFontSmall;
			}

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
			m_Targets[TARGET_LEADER].m_hFont = m_hMiniObjectiveFont;

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
			m_Targets[TARGET_FRONTRUNNER1].m_hFont = m_hMiniObjectiveFont;

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
			m_Targets[TARGET_FRONTRUNNER2].m_hFont = m_hMiniObjectiveFont;

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

	PaintTargets(m_Targets, TARGET_TOTAL);

	for (int k = 0; k < pSDKPlayer->m_Shared.m_aRevealedEnemies.Count(); k++)
	{
		const CRevealedEnemy& oRevealedEnemy = pSDKPlayer->m_Shared.m_aRevealedEnemies[k];
		if (oRevealedEnemy.IsActive(pSDKPlayer->GetCurrentTime()))
		{
			m_RevealedEnemies[k].m_pTargetTexture = m_pBounty;
			m_RevealedEnemies[k].m_flTargetAlpha = 1.0f;
			m_RevealedEnemies[k].m_flScale = 0.4f;
			m_RevealedEnemies[k].m_flMaxAlpha = 1.0f;
			m_RevealedEnemies[k].m_bTargetOn = true;
			m_RevealedEnemies[k].m_flMaxAlpha = RemapVal(pSDKPlayer->GetCurrentTime(), oRevealedEnemy.m_flRevealTime, oRevealedEnemy.m_flRevealTime + oRevealedEnemy.m_flRevealDuration, 1, 0);

			if (C_SDKPlayer* pRevealedEnemy = ToSDKPlayer(ClientEntityList().GetBaseEntity(oRevealedEnemy.m_iEnemyClientIndex+1)))
			{
				m_RevealedEnemies[k].m_hEntity = pRevealedEnemy;
				m_RevealedEnemies[k].m_vecLastKnownTarget = pRevealedEnemy->WorldSpaceCenter();
			}
		}
		else
		{
			m_RevealedEnemies[k].m_bTargetOn = false;
		}
	}

	PaintTargets(m_RevealedEnemies, pSDKPlayer->m_Shared.m_aRevealedEnemies.Count());
}

void CSDKTargetId::PaintTargets(CTarget* pTargets, int iCount)
{
	int iX, iY;

	for (int i = 0; i < iCount; i++)
	{
		if (!pTargets[i].m_bTargetOn && pTargets[i].m_flTargetAlpha == 0)
			continue;

		float flAlphaGoal = 1;

		if (GetVectorInHudSpace(pTargets[i].m_vecLastKnownTarget, iX, iY))
		{
			C_BaseEntity* pTarget = pTargets[i].m_hEntity;

			bool bHide;
			if (!pTargets[i].m_bTargetOn)
				bHide = true;
			else if (pTarget)
			{
				if (pTargets[i].m_bHideIfVisible)
				{
					trace_t tr;
					UTIL_TraceLine(CurrentViewOrigin(), pTargets[i].m_vecLastKnownTarget, MASK_BLOCKLOS, C_SDKPlayer::GetLocalSDKPlayer(), COLLISION_GROUP_NONE, &tr);

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
				flAlphaGoal = pTargets[i].m_flMaxAlpha;
			else
				flAlphaGoal = 0;
		}
		else
		{
			if (!pTargets[i].m_bTargetOn)
				flAlphaGoal = 0;
		}

		pTargets[i].m_flTargetAlpha = Approach(flAlphaGoal, pTargets[i].m_flTargetAlpha, gpGlobals->frametime * 2);

		if (pTargets[i].m_flTargetAlpha > 0)
		{
			int iWidth = pTargets[i].m_pTargetTexture->EffectiveWidth(pTargets[i].m_flScale);
			int iHeight = pTargets[i].m_pTargetTexture->EffectiveHeight(pTargets[i].m_flScale);
			pTargets[i].m_pTargetTexture->DrawSelf(iX - iWidth / 2, iY - iHeight / 2, iWidth, iHeight, Color(255, 255, 255, 255 * pTargets[i].m_flTargetAlpha));

			if (pTargets[i].m_pwszHint)
			{
				int iHintWide, iHintTall;
				surface()->GetTextSize(pTargets[i].m_hFont, pTargets[i].m_pwszHint, iHintWide, iHintTall);

				vgui::surface()->DrawSetTextFont(pTargets[i].m_hFont);
				vgui::surface()->DrawSetTextPos( iX - iHintWide/2, iY + iHeight/2 );
				vgui::surface()->DrawSetTextColor(Color(255, 255, 255, 255 * pTargets[i].m_flTargetAlpha));
				vgui::surface()->DrawPrintText(pTargets[i].m_pwszHint, wcslen(pTargets[i].m_pwszHint));
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

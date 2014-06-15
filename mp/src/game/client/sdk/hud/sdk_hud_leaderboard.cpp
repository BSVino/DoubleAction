//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "convar.h"

#include "iclientmode.h"
#include "sourcevr/isourcevirtualreality.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/ImageList.h>

#include <vgui/ILocalize.h>

#include "vgui_avatarimage.h"

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "c_sdk_player.h"
#include "sdk_hud_stylebar.h"
#include "c_sdk_player_resource.h"
#include "sdk_gamerules.h"
#include "c_da_briefcase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#include "da.h"

#define TOP_RANKS 3

class CHudLeaderboard : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudLeaderboard, vgui::Panel );

public:
	CHudLeaderboard( const char *pElementName );
	~CHudLeaderboard();

public:
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PostApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Reset( void );
	virtual void OnThink();

	virtual bool ShouldDraw( void );

	void         ListPlayer(C_SDKPlayer *pPlayer, int iPosition);
	void         ListSpecialPlayer(C_SDKPlayer* pPlayer, CHudTexture* pTexture, int iPosition);
	void         PaintPlayerAvatar(C_SDKPlayer *pPlayer, float flX, float flY, float flW, float flH);
	virtual void Paint();
	virtual void PaintBackground() {};

protected:
	CHandle<C_SDKPlayer> m_ahPlayerRanks[TOP_RANKS];

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, m_flBorder, "Border", "20", "proportional_float" );

	CHudTexture* m_pGoldStar;
	CHudTexture* m_pSilverStar;
	CHudTexture* m_pBronzeStar;
	CHudTexture* m_pBriefcase;
	CHudTexture* m_pBounty;

	vgui::ImageList*      m_pImageList;
	CUtlMap<CSteamID,int> m_mAvatarsToImageList;
};

DECLARE_HUDELEMENT( CHudLeaderboard );

CHudLeaderboard::CHudLeaderboard( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudLeaderboard" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( 0 );

	m_pGoldStar = m_pSilverStar = m_pBronzeStar = NULL;

	m_pImageList = NULL;

	m_mAvatarsToImageList.SetLessFunc( DefLessFunc( CSteamID ) );
	m_mAvatarsToImageList.RemoveAll();
}

CHudLeaderboard::~CHudLeaderboard()
{
	if ( NULL != m_pImageList )
	{
		delete m_pImageList;
		m_pImageList = NULL;
	}
}

void CHudLeaderboard::Init()
{
	Reset();
}

void CHudLeaderboard::Reset()
{
}

void CHudLeaderboard::VidInit()
{
	Reset();
}

void CHudLeaderboard::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_pImageList )
		delete m_pImageList;
	m_pImageList = new ImageList( false );

	m_mAvatarsToImageList.RemoveAll();
}

void CHudLeaderboard::PostApplySchemeSettings( vgui::IScheme *pScheme )
{
	// resize the images to our resolution
	for (int i = 0; i < m_pImageList->GetImageCount(); i++ )
	{
		int wide, tall;
		m_pImageList->GetImage(i)->GetSize(wide, tall);
		m_pImageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValueEx( GetScheme(),wide), scheme()->GetProportionalScaledValueEx( GetScheme(),tall));
	}
}

void InsertPlayer(CHandle<C_SDKPlayer>* ahPlayerRanks, C_SDKPlayer* pPlayer, int i)
{
	// Move them all down.
	for (int j = TOP_RANKS-1; j >= i+1; j--)
		ahPlayerRanks[j] = ahPlayerRanks[j-1];

	ahPlayerRanks[i] = pPlayer;
}

void CHudLeaderboard::OnThink()
{
	C_SDK_PlayerResource *sdkPR = SDKGameResources();

	if (!sdkPR)
		return;

	CHandle<C_SDKPlayer> ahPlayerRanks[TOP_RANKS];
	ahPlayerRanks[0] = NULL;
	ahPlayerRanks[1] = NULL;
	ahPlayerRanks[2] = NULL;

	// Find the top three.
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		if (!sdkPR->IsConnected( i ))
			continue;

		int iStyle = sdkPR->GetStyle(i);
		if (!iStyle)
			continue;

		for (int j = 0; j < TOP_RANKS; j++)
		{
			if (!ahPlayerRanks[j] || iStyle > sdkPR->GetStyle(ahPlayerRanks[j]->index))
			{
				InsertPlayer(ahPlayerRanks, ToSDKPlayer(UTIL_PlayerByIndex(i)), j);
				break;
			}
		}
	}

	for (int i = 0; i < TOP_RANKS; i++)
		m_ahPlayerRanks[i] = ahPlayerRanks[i];
}

bool CHudLeaderboard::ShouldDraw()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return false;

	if (!pPlayer->IsAlive())
		return false;

	return true;
}

void CHudLeaderboard::ListPlayer(C_SDKPlayer *pPlayer, int iPosition)
{
	int iTextTall = surface()->GetFontTall( m_hTextFont );

	int iStars = pPlayer->GetStyleStars();

	CHudTexture* pStarTexture = m_pGoldStar;

	if (iStars == 0)
		iStars = 1;

	if (pStarTexture)
	{
		pStarTexture->DrawSelf(m_flBorder + iTextTall * iPosition, m_flBorder + iPosition*(iTextTall+5), iTextTall, iTextTall, Color(255, 255, 255, 255));

		wchar_t wszXLabel[100];
		V_swprintf_safe(wszXLabel, L"x%i", iStars);

		surface()->DrawSetTextPos( m_flBorder + iTextTall * iPosition + iTextTall, m_flBorder + iPosition*(iTextTall+5) );
		surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawUnicodeString( wszXLabel, vgui::FONT_DRAW_NONADDITIVE );

		int iTextWide;
		surface()->GetTextSize( m_hTextFont, wszXLabel, iTextWide, iTextTall );

		PaintPlayerAvatar(pPlayer, m_flBorder + iTextTall * iPosition + iTextTall + iTextWide, m_flBorder + iPosition*(iTextTall+5), iTextTall, iTextTall );
	}

	wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
	g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  wszPlayerName, sizeof(wszPlayerName) );

	surface()->DrawSetTextPos( m_flBorder + iTextTall * iPosition + iTextTall*3.5f, m_flBorder + iPosition*(iTextTall+5) );
	surface()->DrawSetTextColor( g_PR->GetTeamColor( pPlayer->GetTeamNumber() ) );
	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawUnicodeString( wszPlayerName, vgui::FONT_DRAW_NONADDITIVE );
}

void CHudLeaderboard::PaintPlayerAvatar(C_SDKPlayer *pPlayer, float flX, float flY, float flW, float flH)
{
	player_info_t pi;
	if ( engine->GetPlayerInfo( pPlayer->entindex(), &pi ) )
	{
		if ( pi.friendsID )
		{
			CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );

			// See if we already have that avatar in our list
			int iMapIndex = m_mAvatarsToImageList.Find( steamIDForPlayer );
			int iImageIndex;
			if ( iMapIndex == m_mAvatarsToImageList.InvalidIndex() )
			{
				CAvatarImage *pImage = new CAvatarImage();
				pImage->SetAvatarSteamID( steamIDForPlayer );
				pImage->SetAvatarSize(flW, flH);
				iImageIndex = m_pImageList->AddImage( pImage );

				m_mAvatarsToImageList.Insert( steamIDForPlayer, iImageIndex );
			}
			else
			{
				iImageIndex = m_mAvatarsToImageList[ iMapIndex ];
			}

			CAvatarImage *pAvIm = (CAvatarImage *)m_pImageList->GetImage( iImageIndex );
			pAvIm->UpdateFriendStatus();

			pAvIm->SetPos(flX, flY);
			pAvIm->Paint();
		}
	}
}

void CHudLeaderboard::ListSpecialPlayer(C_SDKPlayer* pPlayer, CHudTexture* pTexture, int iPosition)
{
	if (!pPlayer)
		return;

	int iTextTall = surface()->GetFontTall( m_hTextFont );

	if (pTexture)
		pTexture->DrawSelf(m_flBorder + iTextTall * iPosition, m_flBorder + iTextTall * iPosition, iTextTall, iTextTall, Color(255, 255, 255, 255));

	PaintPlayerAvatar(pPlayer, m_flBorder + iTextTall * (iPosition + 1), m_flBorder + iTextTall * iPosition, iTextTall, iTextTall);

	wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
	g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  wszPlayerName, sizeof(wszPlayerName) );

	surface()->DrawSetTextPos( m_flBorder + iTextTall + iTextTall*2 + iTextTall * iPosition, m_flBorder + iTextTall * iPosition );
	surface()->DrawSetTextColor( g_PR->GetTeamColor( pPlayer->GetTeamNumber() ) );
	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawUnicodeString( wszPlayerName, vgui::FONT_DRAW_NONADDITIVE );
}

void CHudLeaderboard::Paint()
{
	C_SDKPlayer *pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pLocalPlayer )
		return;

	if (!m_pGoldStar)
	{
		m_pGoldStar = gHUD.GetIcon("star_gold");
		m_pSilverStar = gHUD.GetIcon("star_silver");
		m_pBronzeStar = gHUD.GetIcon("star_bronze");
		m_pBriefcase = gHUD.GetIcon("briefcase");
		m_pBounty = gHUD.GetIcon("bounty");
	}

	int iTextTall = surface()->GetFontTall( m_hTextFont );
	surface()->DrawSetTextPos( m_flBorder + iTextTall + iTextTall*2, m_flBorder );
	surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
	surface()->DrawSetTextFont( m_hTextFont );

	if (SDKGameRules()->GetBountyPlayer())
	{
		wchar_t* pszGameMode = g_pVGuiLocalize->Find("#DA_LeaderBoard_Wanted");
		if (pszGameMode)
			surface()->DrawUnicodeString( pszGameMode, vgui::FONT_DRAW_NONADDITIVE );

		ListSpecialPlayer(SDKGameRules()->GetBountyPlayer(), m_pBounty, 1);
		return;
	}
	else if (SDKGameRules()->GetBriefcase() && ToSDKPlayer(SDKGameRules()->GetBriefcase()->GetOwnerEntity()))
	{
		wchar_t* pszGameMode = g_pVGuiLocalize->Find("#DA_LeaderBoard_CTB");
		if (pszGameMode)
			surface()->DrawUnicodeString( pszGameMode, vgui::FONT_DRAW_NONADDITIVE );

		ListSpecialPlayer(ToSDKPlayer(SDKGameRules()->GetBriefcase()->GetOwnerEntity()), m_pBriefcase, 1);
		return;
	}
	else if (SDKGameRules()->GetLeader())
	{
		wchar_t* pszGameMode = g_pVGuiLocalize->Find("#DA_LeaderBoard_RatRace");
		if (pszGameMode)
			surface()->DrawUnicodeString( pszGameMode, vgui::FONT_DRAW_NONADDITIVE );

		ListSpecialPlayer(SDKGameRules()->GetLeader(), m_pBounty, 1);
		ListSpecialPlayer(SDKGameRules()->GetFrontRunner1(), m_pBounty, 2);
		ListSpecialPlayer(SDKGameRules()->GetFrontRunner2(), m_pBounty, 3);
		return;
	}

	// Text is small and in a far corner, too much eye strain to read.
	if (UseVR())
		return;

	bool bLocalPlayerShown = false;

	for ( int i = 0; i < TOP_RANKS; i++ )
	{
		C_SDKPlayer* pRanked = m_ahPlayerRanks[i];
		if (!pRanked)
			continue;

		ListPlayer(pRanked, i);

		if (pRanked == pLocalPlayer)
			bLocalPlayerShown = true;
	}

	if (!bLocalPlayerShown)
	{
		C_SDK_PlayerResource *sdkPR = SDKGameResources();
		if (sdkPR && sdkPR->GetStyle(pLocalPlayer->entindex()))
			ListPlayer(pLocalPlayer, 3);
	}
}

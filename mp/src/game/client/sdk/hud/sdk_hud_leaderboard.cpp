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
	virtual void Paint();
	virtual void PaintBackground() {};

protected:
	CHandle<C_SDKPlayer> m_ahPlayerRanks[TOP_RANKS];

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, m_flBorder, "Border", "20", "proportional_float" );

	CHudTexture* m_pGoldStar;
	CHudTexture* m_pSilverStar;
	CHudTexture* m_pBronzeStar;

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

	int iGold, iSilver, iBronze;
	pPlayer->GetStyleStars(iGold, iSilver, iBronze);

	int iStars = iGold;
	CHudTexture* pStarTexture = m_pGoldStar;

	if (iStars == 0)
	{
		iStars = iSilver;
		pStarTexture = m_pSilverStar;

		if (iStars == 0)
		{
			iStars = iBronze;
			pStarTexture = m_pBronzeStar;
		}
	}

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
	}

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
				pImage->SetAvatarSize(iTextTall, iTextTall);
				iImageIndex = m_pImageList->AddImage( pImage );

				m_mAvatarsToImageList.Insert( steamIDForPlayer, iImageIndex );
			}
			else
			{
				iImageIndex = m_mAvatarsToImageList[ iMapIndex ];
			}

			CAvatarImage *pAvIm = (CAvatarImage *)m_pImageList->GetImage( iImageIndex );
			pAvIm->UpdateFriendStatus();

			pAvIm->SetPos(m_flBorder + iTextTall * iPosition + iTextTall*3.5f - pAvIm->GetWide(), m_flBorder + iPosition*(iTextTall+5) );
			pAvIm->Paint();
		}
	}

	wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
	g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  wszPlayerName, sizeof(wszPlayerName) );

	surface()->DrawSetTextPos( m_flBorder + iTextTall * iPosition + iTextTall*3.5f, m_flBorder + iPosition*(iTextTall+5) );
	surface()->DrawSetTextColor( Color(255, 255, 255, 255) );
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
	}

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
		if (sdkPR->GetStyle(pLocalPlayer->entindex()))
			ListPlayer(pLocalPlayer, 3);
	}
}

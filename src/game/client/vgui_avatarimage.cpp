//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "vgui_avatarimage.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#include "steam/steam_api.h"

DECLARE_BUILD_FACTORY( CAvatarImagePanel );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAvatarImage::CAvatarImage( void )
{
	ClearAvatarSteamID();
	m_pFriendIcon = NULL;
	m_nX = 0;
	m_nY = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::ClearAvatarSteamID( void ) 
{ 
	m_bValid = false; 
	m_bFriend = false;
	m_SteamID.Set( 0, k_EUniverseInvalid, k_EAccountTypeInvalid );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarSteamID( CSteamID steamIDUser )
{
	ClearAvatarSteamID();

	if ( steamapicontext->SteamFriends() && steamapicontext->SteamUtils() )
	{
		m_SteamID = steamIDUser;

		int iAvatar = steamapicontext->SteamFriends()->GetFriendAvatar( steamIDUser );

		/*
		// See if it's in our list already
		*/

		uint32 wide, tall;
		if ( steamapicontext->SteamUtils()->GetImageSize( iAvatar, &wide, &tall ) )
		{
			int cubImage = wide * tall * 4;
			byte *rgubDest = (byte*)_alloca( cubImage );
			steamapicontext->SteamUtils()->GetImageRGBA( iAvatar, rgubDest, cubImage );
			InitFromRGBA( rgubDest, wide, tall );

			/*
			// put it in the list
			RGBAImage *pRGBAImage = new RGBAImage( rgubDest, wide, tall );
			int iImageList = m_pImageList->AddImage( pRGBAImage );
			m_mapAvatarToIImageList.Insert( iAvatar, iImageList );
			*/
		}

		UpdateFriendStatus();
	}

	return m_bValid;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::UpdateFriendStatus( void )
{
	if ( !m_SteamID.IsValid() )
		return;

	if ( steamapicontext->SteamFriends() && steamapicontext->SteamUtils() )
	{
		m_bFriend = steamapicontext->SteamFriends()->HasFriend( m_SteamID, k_EFriendFlagImmediate );
		if ( m_bFriend && !m_pFriendIcon )
		{
			m_pFriendIcon = gHUD.GetIcon( "ico_friend_indicator_avatar" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::InitFromRGBA( const byte *rgba, int width, int height )
{
	m_iTextureID = vgui::surface()->CreateNewTextureID( true );
	vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, width, height, false, false );
	m_nWide = XRES(width);
	m_nTall = YRES(height);
	m_Color = Color( 255, 255, 255, 255 );

	m_bValid = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::Paint( void )
{
	if ( m_bValid )
	{
		if ( m_bFriend && m_pFriendIcon )
		{
			m_pFriendIcon->DrawSelf( m_nX, m_nY, m_nWide, m_nTall, m_Color );
		}

		vgui::surface()->DrawSetColor( m_Color );
		vgui::surface()->DrawSetTexture( m_iTextureID );
		vgui::surface()->DrawTexturedRect( m_nX + AVATAR_INDENT_X, m_nY + AVATAR_INDENT_Y, m_nX + AVATAR_INDENT_X + m_iAvatarWidth, m_nY + AVATAR_INDENT_Y + m_iAvatarHeight );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAvatarImagePanel::CAvatarImagePanel( vgui::Panel *parent, const char *name ) : vgui::ImagePanel( parent, name )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayer( C_BasePlayer *pPlayer )
{
	if ( GetImage() )
	{
		((CAvatarImage*)GetImage())->ClearAvatarSteamID();
	}

	if ( pPlayer && steamapicontext->SteamUtils() )
	{
		int iIndex = pPlayer->entindex();
		player_info_t pi;
		if ( engine->GetPlayerInfo(iIndex, &pi) )
		{
			if ( pi.friendsID )
			{
				CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );

				if ( !GetImage() )
				{
					CAvatarImage *pImage = new CAvatarImage();
					SetImage( pImage );
				}

				CAvatarImage *pAvImage = ((CAvatarImage*)GetImage());
				pAvImage->SetAvatarSteamID( steamIDForPlayer );

				// Indent the image. These are deliberately non-resolution-scaling.
				pAvImage->SetAvatarSize( 32, 32 );	// Deliberately non scaling

				SetSize( pAvImage->GetWide(), GetTall() );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImagePanel::PaintBackground( void )
{
	if ( GetImage() )
	{
		GetImage()->Paint();
	}
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SDK_CLIENTMODE_H
#define SDK_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "daviewport.h"
#include "GameUI/IGameUI.h"

class ClientModeSDKNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeSDKNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeSDKNormal();
	virtual			~ClientModeSDKNormal();

	virtual void	InitViewport();

	virtual float	GetViewModelFOV( void );

	int				GetDeathMessageStartHeight( void );

	virtual void	PostRenderVGui();

	virtual void	OverrideView( CViewSetup *pSetup );
	virtual void	OverrideMouseInput( float *x, float *y );

	virtual int     HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );
	virtual int		HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

private:
	
	//	void	UpdateSpectatorMode( void );

	IGameUI *m_pGameUI;
};


extern IClientMode *GetClientModeNormal();
extern ClientModeSDKNormal* GetClientModeSDKNormal();


#endif // SDK_CLIENTMODE_H

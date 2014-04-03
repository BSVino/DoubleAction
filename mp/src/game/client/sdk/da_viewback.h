#pragma once

#ifdef WITH_VIEWBACK

#include "sdk_shareddefs.h"

#include "../../../../viewback/server/viewback.h"

class CViewbackSystem : public CAutoGameSystemPerFrame, public IConsoleDisplayFunc
{
public:
	CViewbackSystem( char const *name );
	virtual ~CViewbackSystem();

public:
	virtual void LevelInitPostEntity();
	virtual void Update(float frametime);
	virtual void LevelShutdownPostEntity();

	// From IConsoleDisplayFunc
	virtual void ColorPrint( const Color& clr, const char *pMessage );
	virtual void Print( const char *pMessage );
	virtual void DPrint( const char *pMessage );

public:
	vb_channel_handle_t m_ePlayerStyle;
	vb_channel_handle_t m_eAimIn;
	vb_channel_handle_t m_eSlowMo;

	vb_channel_handle_t m_eAnimAim;
	vb_channel_handle_t m_eAnimMoveYaw;

	vb_channel_handle_t m_ePlayerOriginX;
	vb_channel_handle_t m_ePlayerOriginY;
	vb_channel_handle_t m_ePlayerOriginZ;
	vb_channel_handle_t m_ePlayerSpeed;
	vb_channel_handle_t m_ePlayerMaxSpeed;

	vb_channel_handle_t m_ePlayerRecoil;
	vb_channel_handle_t m_ePlayerRecoilFloat;
	vb_channel_handle_t m_ePlayerViewPunch;
};

CViewbackSystem& ViewbackSystem();

#endif

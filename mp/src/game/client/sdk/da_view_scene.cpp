//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iinput.h"
#include "model_types.h"
#include "clientsideeffects.h"
#include "particlemgr.h"
#include "viewrender.h"
#include "iclientmode.h"
#include "voice_status.h"
#include "glow_overlay.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "detailobjectsystem.h"
#include "tier0/vprof.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "view_scene.h"
#include "particles_ez.h"
#include "engine/IStaticPropMgr.h"
#include "engine/ivdebugoverlay.h"
#include "da_view_scene.h"
#include "c_sdk_player.h"
#include "sdk_gamerules.h"
#include "shake.h"
#include "clienteffectprecachesystem.h"
#include <vgui/ISurface.h>

CLIENTEFFECT_REGISTER_BEGIN( PrecacheDAViewScene )
CLIENTEFFECT_MATERIAL( "shaders/slowmo" )
CLIENTEFFECT_REGISTER_END()

static CDAViewRender g_ViewRender;

CDAViewRender::CDAViewRender()
{
	view = ( IViewRender * )&g_ViewRender;

	m_flStyleLerp = 0;
}

ConVar da_postprocess_compare( "da_postprocess_compare", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Only render to half of the screen for debug purposes" );
ConVar da_postprocess_deathcam_override( "da_postprocess_deathcam_override", "-1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Override the control for death came mode" );

ConVar da_postprocess_shaders( "da_postprocess_shaders", "1", FCVAR_USERINFO, "Use screen-space post-process shaders?" );

void CDAViewRender::PerformSlowMoEffect( const CViewSetup &view )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( !pPlayer )
		return;

	if (!da_postprocess_shaders.GetBool())
		return;

	if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
	{
		CBaseEntity *target = pPlayer->GetObserverTarget();
		if (target && target->IsPlayer())
			pPlayer = (C_SDKPlayer *)target;
	}

	if (!pPlayer->IsAlive())
		m_flStyleLerp = 0;

	ConVarRef da_postprocess_slowmo("da_postprocess_slowmo");
	ConVarRef da_postprocess_deathcam("da_postprocess_deathcam");
	ConVarRef da_postprocess_skill("da_postprocess_skill");

	if (pPlayer->IsStyleSkillActive())
		m_flStyleLerp = Approach(1, m_flStyleLerp, gpGlobals->frametime*2);
	else
		m_flStyleLerp = Approach(0, m_flStyleLerp, gpGlobals->frametime);

	bool bShowPostProcess = false;
	if (pPlayer->GetSlowMoMultiplier() < 1)
		bShowPostProcess = true;
	else if (m_flStyleLerp)
		bShowPostProcess = true;
	else if (!pPlayer->IsAlive() && pPlayer->GetObserverMode() == OBS_MODE_FREEZECAM)
		bShowPostProcess = true;
	else if (da_postprocess_compare.GetInt() || da_postprocess_slowmo.GetInt())
		bShowPostProcess = true;

	if ( bShowPostProcess )
	{
		IMaterial *pMaterial = materials->FindMaterial( "shaders/slowmo", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( !IsErrorMaterial(pMaterial) )
		{
			if (pPlayer->IsAlive())
				da_postprocess_deathcam.SetValue(false);
			else
				da_postprocess_deathcam.SetValue(true);

			da_postprocess_skill.SetValue(m_flStyleLerp);

			if (da_postprocess_compare.GetInt() == 1)
				DrawScreenEffectMaterial( pMaterial, 0, 0, XRES(320), ScreenHeight() );
			else
				DrawScreenEffectMaterial( pMaterial, 0, 0, ScreenWidth(), ScreenHeight() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------
void CDAViewRender::Render2DEffectsPreHUD( const CViewSetup &view )
{
	PerformSlowMoEffect( view );	// this needs to come before the HUD is drawn, or it will wash the HUD out
}

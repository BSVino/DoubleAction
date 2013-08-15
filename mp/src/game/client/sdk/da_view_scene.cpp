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
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "DetailObjectSystem.h"
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
#include "ClientEffectPrecacheSystem.h"
#include <vgui/isurface.h>

CLIENTEFFECT_REGISTER_BEGIN( PrecacheDAViewScene )
CLIENTEFFECT_MATERIAL( "shaders/slowmo" )
CLIENTEFFECT_REGISTER_END()

static CDAViewRender g_ViewRender;

CDAViewRender::CDAViewRender()
{
	view = ( IViewRender * )&g_ViewRender;
}

ConVar da_postprocess_compare( "da_postprocess_compare", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Only render to half of the screen for debug purposes" );
ConVar da_postprocess_deathcam_override( "da_postprocess_deathcam_override", "-1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Override the control for death came mode" );

void CDAViewRender::PerformSlowMoEffect( const CViewSetup &view )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( !pPlayer )
		return;

	if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
	{
		CBaseEntity *target = pPlayer->GetObserverTarget();
		if (target && target->IsPlayer())
			pPlayer = (C_SDKPlayer *)target;
	}

	ConVarRef da_postprocess_slowmo("da_postprocess_slowmo");
	ConVarRef da_postprocess_deathcam("da_postprocess_deathcam");

	if ( pPlayer->GetSlowMoMultiplier() < 1 || (!pPlayer->IsAlive() && pPlayer->GetObserverMode() == OBS_MODE_FREEZECAM) || da_postprocess_compare.GetInt() || da_postprocess_slowmo.GetInt() )
	{
		IMaterial *pMaterial = materials->FindMaterial( "shaders/slowmo", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( !IsErrorMaterial(pMaterial) )
		{
			if (pPlayer->IsAlive())
				da_postprocess_deathcam.SetValue(false);
			else
				da_postprocess_deathcam.SetValue(true);

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

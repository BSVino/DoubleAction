//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "hud.h"
#include "clientmode_sdk.h"
#include "cdll_client_int.h"
#include "iinput.h"
#include "vgui/ISurface.h"
#include "vgui/IPanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
#include "buymenu.h"
#include "filesystem.h"
#include "vgui/IVGui.h"
#include "hud_chat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include <KeyValues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "weapon_sdkbase.h"
#include "c_sdk_player.h"
#include "c_weapon__stubs.h"		//Tony; add stubs
#include "cam_thirdperson.h"
#include "sdk_in_main.h"
#include "da_newsframe.h"
#include "hud/da_hud_vote.h"
#include "da_credits.h"
#include "hud_macros.h"
#include "sourcevr/isourcevirtualreality.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHudChat;

ConVar default_fov( "default_fov", "90", FCVAR_CHEAT );

IClientMode *g_pClientMode = NULL;

//Tony; add stubs for cycler weapon and cubemap.
STUB_WEAPON_CLASS( cycler_weapon,   WeaponCycler,   C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap,  WeaponCubemap,  C_BaseCombatWeapon );

// --------------------------------------------------------------------------------- //
// CSDKModeManager.
// --------------------------------------------------------------------------------- //

class CSDKModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CSDKModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CSDKModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

extern void __MsgFunc_FolderPanel( bf_read &msg );

void CSDKModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();

	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );

	HOOK_MESSAGE( FolderPanel );
}

void CSDKModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );

	ConVarRef cl_detail_max_sway("cl_detail_max_sway");
	ConVarRef cl_detail_avoid_radius("cl_detail_avoid_radius");
	ConVarRef cl_detail_avoid_force("cl_detail_avoid_force");
	ConVarRef cl_detail_avoid_recover_speed("cl_detail_avoid_recover_speed");

	// HACK: the detail sway convars are archive, and default to 0.  Existing CS:S players thus have no detail
	// prop sway.  We'll force them to DoD's default values for now.
	if ( !cl_detail_max_sway.GetFloat() &&
		!cl_detail_avoid_radius.GetFloat() &&
		!cl_detail_avoid_force.GetFloat() &&
		!cl_detail_avoid_recover_speed.GetFloat() )
	{
		cl_detail_max_sway.SetValue( "5" );
		cl_detail_avoid_radius.SetValue( "64" );
		cl_detail_avoid_force.SetValue( "0.4" );
		cl_detail_avoid_recover_speed.SetValue( "0.25" );
	}

	if (UseVR())
	{
#ifdef _DEBUG
		ConVarRef("vr_force_windowed").SetValue(true);
#endif
		ConVarRef("vr_first_person_uses_world_model").SetValue(false);
		ConVarRef("vr_moveaim_reticle_yaw_limit").SetValue(15);
		ConVarRef("vr_moveaim_reticle_yaw_limit_zoom").SetValue(15);
		ConVarRef("vr_viewmodel_offset_forward").SetValue(5);
		ConVarRef("vr_zoom_multiplier").SetValue(1.4f);
	}

	AB_Input_LevelInit();
}

void CSDKModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeSDKNormal::ClientModeSDKNormal()
{
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeSDKNormal::~ClientModeSDKNormal()
{
}

void ClientModeSDKNormal::InitViewport()
{
	new CNewsFrame();
	new CDACredits();

	m_pViewport = new SDKViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

ClientModeSDKNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}


ClientModeSDKNormal* GetClientModeSDKNormal()
{
	Assert( dynamic_cast< ClientModeSDKNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeSDKNormal* >( GetClientModeNormal() );
}

float ClientModeSDKNormal::GetViewModelFOV( void )
{
	//Tony; retrieve the fov from the view model script, if it overrides it.
	float viewFov = 74.0;

	C_WeaponSDKBase *pWeapon = (C_WeaponSDKBase*)GetActiveWeapon();
	if ( pWeapon )
	{
		viewFov = pWeapon->GetWeaponFOV();
	}
	return viewFov;
}

int ClientModeSDKNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeSDKNormal::PostRenderVGui()
{
}

void ClientModeSDKNormal::OverrideView( CViewSetup *pSetup )
{
	QAngle camAngles;

	// Let the player override the view.
	C_BasePlayer *pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if(!pPlayer)
		return;

	pPlayer->OverrideView( pSetup );

	if( ::input->CAM_IsThirdPerson() && pPlayer->IsAlive() )
	{
		Vector cam_ofs = g_ThirdPersonManager.GetCameraOffsetAngles();

		camAngles[ PITCH ] = cam_ofs[ PITCH ];
		camAngles[ YAW ] = cam_ofs[ YAW ];
		camAngles[ ROLL ] = 0;

		Vector camForward, camRight, camUp;
		AngleVectors( camAngles, &camForward, &camRight, &camUp );

		VectorMA( pSetup->origin, -cam_ofs[ ROLL ], camForward, pSetup->origin );

		// Override angles from third person camera
		float flRoll = pSetup->angles.z;
		pSetup->angles = camAngles;
		pSetup->angles.z = flRoll;
	}
}

ConVar m_verticaldamping("m_verticaldamping", "0.85", FCVAR_CLIENTDLL|FCVAR_ARCHIVE, "Multiplier to dampen vertical component of mouse movement.", true, 0.1f, true, 1);
ConVar m_slowmodamping("m_slowmodamping", "0.7", FCVAR_CLIENTDLL|FCVAR_ARCHIVE, "Multiplier to dampen mouse movement during slow motion.", true, 0.1f, true, 1);
ConVar m_aimindamping("m_aimindamping", "0.5", FCVAR_CLIENTDLL|FCVAR_ARCHIVE, "Multiplier to dampen mouse movement during aim-in.", true, 0.1f, true, 1);
ConVar m_partialaimindamping("m_partialaimindamping", "0.8", FCVAR_CLIENTDLL|FCVAR_ARCHIVE, "Multiplier to dampen mouse movement during aim-in for non-rifles.", true, 0.1f, true, 1);

void ClientModeSDKNormal::OverrideMouseInput( float *x, float *y )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (!pPlayer)
		return;

	float flSlowMultiplier = RemapValClamped(pPlayer->GetSlowMoMultiplier(), 0.4f, 1, m_slowmodamping.GetFloat(), 1);

	*x *= flSlowMultiplier;
	*y *= flSlowMultiplier;

	*y *= m_verticaldamping.GetFloat();

	C_WeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	if (pWeapon)
	{
		float flAimInMultiplier;
		if (pWeapon->HasAimInSpeedPenalty())
			flAimInMultiplier = RemapValClamped(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, m_aimindamping.GetFloat());
		else
			flAimInMultiplier = RemapValClamped(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, m_partialaimindamping.GetFloat());

		*x *= flAimInMultiplier;
		*y *= flAimInMultiplier;
	}

	BaseClass::OverrideMouseInput(x, y);
}

int ClientModeSDKNormal::HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (pPlayer && pPlayer->GetTeamNumber() != TEAM_SPECTATOR)
	{
		// Override base class for +attack, it respawns us.
		if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+attack" ) == 0 )
			return 1;
	}

	// Default binding is +alt1 on the right mouse. Have this switch players.
	if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+alt1" ) == 0 )
	{
		if (pPlayer && pPlayer->GetTeamNumber() != TEAM_SPECTATOR)
			engine->ClientCmd( "spec_prev" );
		else
			engine->ClientCmd( "spec_next" );

		return 0;
	}

	// Can't figure out how to get +duck to send a close signal too, so let's just not use the spec menu.
	if ( down && pszCurrentBinding && Q_strcmp( pszCurrentBinding, "+duck" ) == 0 )
		return 0;

	return BaseClass::HandleSpectatorKeyInput(down, keynum, pszCurrentBinding);
}

int ClientModeSDKNormal::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	CHudVote* pElement = (CHudVote*)gHUD.FindElement("CHudVote");
	if (pElement)
		return pElement->KeyInput(down, keynum, pszCurrentBinding);
	else
		return BaseClass::HandleSpectatorKeyInput(down, keynum, pszCurrentBinding);
}

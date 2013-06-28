//===== Copyright 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "projectedlighteffect.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"
#include "c_sdk_player.h"

#if defined( _X360 )
extern ConVar r_flashlightdepthres;
#else
extern ConVar r_flashlightdepthres;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar r_flashlightdepthtexture;

static ConVar r_swingflashlight( "r_swingflashlight", "1", FCVAR_CHEAT );
static ConVar r_flashlightlockposition( "r_flashlightlockposition", "0", FCVAR_CHEAT );
static ConVar r_flashlightfov( "r_flashlightfov", "53.0", FCVAR_CHEAT );
ConVar r_flashlightoffsetx( "r_flashlightoffsetright", "5.0", FCVAR_CHEAT );
ConVar r_flashlightoffsety( "r_flashlightoffsetup", "-5.0", FCVAR_CHEAT );
ConVar r_flashlightoffsetz( "r_flashlightoffsetforward", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightnear( "r_flashlightnear", "4.0", FCVAR_CHEAT );
static ConVar r_flashlightfar( "r_flashlightfar", "2048.0", FCVAR_CHEAT );
static ConVar r_flashlightconstant( "r_flashlightconstant", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightlinear( "r_flashlightlinear", "100.0", FCVAR_CHEAT );
static ConVar r_flashlightquadratic( "r_flashlightquadratic", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightvisualizetrace( "r_flashlightvisualizetrace", "0", FCVAR_CHEAT );
static ConVar r_flashlightambient( "r_flashlightambient", "0.0", FCVAR_CHEAT );
static ConVar r_flashlightshadowatten( "r_flashlightshadowatten", "0.35", FCVAR_CHEAT );
static ConVar r_flashlightladderdist( "r_flashlightladderdist", "40.0", FCVAR_CHEAT );
static ConVar mat_slopescaledepthbias_shadowmap( "mat_slopescaledepthbias_shadowmap", "16", FCVAR_CHEAT );
static ConVar mat_depthbias_shadowmap(	"mat_depthbias_shadowmap", "0.0005", FCVAR_CHEAT  );

static ConVar r_flashlightnearoffsetscale( "r_flashlightnearoffsetscale", "1.0", FCVAR_CHEAT );
static ConVar r_flashlighttracedistcutoff( "r_flashlighttracedistcutoff", "128" );
static ConVar r_flashlightbacktraceoffset( "r_flashlightbacktraceoffset", "0.4", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
//Tony; modified so that we can have flashlights for everyone, AND have muzzleflashes for everyone too.
CProjectedLightEffectManager & ProjectedLightEffectManager( int32 nPlayerIndex )
{
	static CProjectedLightEffectManager s_flashlightEffectManagerArray[MAX_PLAYERS];
	return s_flashlightEffectManagerArray[ nPlayerIndex ];
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
CProjectedLightEffect::CProjectedLightEffect(int nEntIndex, const char *pszTextureName, float flFov, float flFarZ, float flLinearAtten )
{
	m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndex = nEntIndex;

	m_flCurrentPullBackDist = 1.0f;

	m_bMuzzleFlashEnabled = false;
	m_flMuzzleFlashBrightness = 1.0f;

	m_flFov = flFov;
	m_flFarZ = flFarZ;
	m_flLinearAtten = flLinearAtten;
	m_bCastsShadows = true;

	m_bIsOn = false;

	UpdateFlashlightTexture( pszTextureName );
	m_MuzzleFlashTexture.Init( "effects/muzzleflash_light", TEXTURE_GROUP_OTHER, true );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CProjectedLightEffect::~CProjectedLightEffect()
{
	LightOff();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectedLightEffect::TurnOn()
{
	m_bIsOn = true;
	m_flCurrentPullBackDist = 1.0f;
}


//-----------------------------------------------------------------------------
void CProjectedLightEffect::SetMuzzleFlashEnabled( bool bEnabled, float flBrightness )
{
	m_bMuzzleFlashEnabled = bEnabled;
	m_flMuzzleFlashBrightness = flBrightness;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectedLightEffect::TurnOff()
{
	if (m_bIsOn)
	{
		m_bIsOn = false;
		LightOff();
	}
}


void C_SDKPlayer::GetFlashlightOffset( const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, Vector *pVecOffset ) const
{
	*pVecOffset = r_flashlightoffsety.GetFloat() * vecUp + r_flashlightoffsetx.GetFloat() * vecRight + r_flashlightoffsetz.GetFloat() * vecForward;

	if (IsInThirdPerson())
		*pVecOffset += vecForward * 40;
}

ConVar r_flashlightmuzzleflashfov( "r_flashlightmuzzleflashfov", "120", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CProjectedLightEffect::UpdateLight(	int nEntIdx, const Vector &vecPos, const Vector &vecForward, const Vector &vecRight,
										const Vector &vecUp, float flFov, float flFarZ, float flLinearAtten, bool castsShadows, const char* pTextureName )
{
	VPROF_BUDGET( "CProjectedLightEffect::UpdateLightNew", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING );

	if ( !m_bIsOn )
	{
		//		return;
	}

	m_nEntIndex = nEntIdx;
	m_flFov = flFov;
	m_flFarZ = flFarZ;
	m_flLinearAtten = flLinearAtten;

	if ( m_bCastsShadows != castsShadows )
	{
		// requires recreation of the flashlight
		LightOff();
	}
	m_bCastsShadows = castsShadows;

	UpdateFlashlightTexture( pTextureName );

	FlashlightState_t state;

	if ( UpdateDefaultFlashlightState( state, vecPos, vecForward, vecRight, vecUp, castsShadows ) == false )
	{
		return;
	}

	if( m_FlashlightHandle == CLIENTSHADOW_INVALID_HANDLE )
	{
		m_FlashlightHandle = g_pClientShadowMgr->CreateFlashlight( state );
	}
	else
	{
		if( !r_flashlightlockposition.GetBool() )
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_FlashlightHandle, state );
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture( m_FlashlightHandle, true );

#ifndef NO_TOOLFRAMEWORK
	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "FlashlightState" );
		msg->SetFloat( "time", gpGlobals->curtime );
		msg->SetInt( "entindex", m_nEntIndex );
		msg->SetInt( "flashlightHandle", m_FlashlightHandle );
		msg->SetPtr( "flashlightState", &state );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
#endif
}

void CProjectedLightEffect::UpdateLight(	int nEntIdx, const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, 
										float flFov, bool castsShadows, ITexture *pFlashlightTexture, const Vector &vecBrightness,
										bool bTracePlayers )
{
	VPROF_BUDGET( "CProjectedLightEffect::UpdateLight", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING );

	m_nEntIndex = nEntIdx;
	if ( m_bCastsShadows != castsShadows )
	{
		// requires recreation of the flashlight
		LightOff();
	}
	m_bCastsShadows = castsShadows;

	FlashlightState_t state;

	if ( UpdateDefaultFlashlightState( state, vecPos, vecDir, vecRight, vecUp, castsShadows, bTracePlayers ) == false )
	{
		return;
	}

	state.m_fHorizontalFOVDegrees = flFov;
	state.m_fVerticalFOVDegrees = flFov;

	state.m_Color[0] = vecBrightness.x;
	state.m_Color[1] = vecBrightness.y;
	state.m_Color[2] = vecBrightness.z;

	if ( pFlashlightTexture )
	{
		state.m_pSpotlightTexture = pFlashlightTexture;
	}

	if( m_FlashlightHandle == CLIENTSHADOW_INVALID_HANDLE )
	{
		m_FlashlightHandle = g_pClientShadowMgr->CreateFlashlight( state );
	}
	else
	{
		if( !r_flashlightlockposition.GetBool() )
		{
			g_pClientShadowMgr->UpdateFlashlightState( m_FlashlightHandle, state );
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture( m_FlashlightHandle, true );

#ifndef NO_TOOLFRAMEWORK
	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "FlashlightState" );
		msg->SetFloat( "time", gpGlobals->curtime );
		msg->SetInt( "entindex", m_nEntIndex );
		msg->SetInt( "flashlightHandle", m_FlashlightHandle );
		msg->SetPtr( "flashlightState", &state );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
#endif
}

bool CProjectedLightEffect::UpdateDefaultFlashlightState( FlashlightState_t& state, const Vector &vecPos, const Vector &vecForward,
														 const Vector &vecRight, const Vector &vecUp, bool castsShadows, bool bTracePlayers )
{
	VPROF_BUDGET( "CProjectedLightEffect::UpdateDefaultFlashlightState", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING );

	if ( !m_bIsOn )
	{
		//		return;
	}

	if ( ComputeLightPosAndOrientation( vecPos, vecForward, vecRight, vecUp, state.m_vecLightOrigin, state.m_quatOrientation, bTracePlayers ) == false )
	{
		return false;
	}

	state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();

	bool bFlicker = false;

	if ( bFlicker == false )
	{
		if ( m_flLinearAtten > 0.0f )
		{
			state.m_fLinearAtten = m_flLinearAtten;
		}
		else
		{
			state.m_fLinearAtten = r_flashlightlinear.GetFloat();
		}

		if ( m_flFov > 0.0f )
		{
			state.m_fHorizontalFOVDegrees = m_flFov;
			state.m_fVerticalFOVDegrees = m_flFov;
		}
		else
		{
			state.m_fHorizontalFOVDegrees = r_flashlightfov.GetFloat();
			state.m_fVerticalFOVDegrees = r_flashlightfov.GetFloat();
		}

		if ( m_bMuzzleFlashEnabled )
		{
			state.m_fHorizontalFOVDegrees = state.m_fVerticalFOVDegrees = r_flashlightmuzzleflashfov.GetFloat();
		}
	}

	state.m_fConstantAtten = r_flashlightconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_flashlightambient.GetFloat();

	state.m_NearZ = r_flashlightnear.GetFloat() + r_flashlightnearoffsetscale.GetFloat() * m_flCurrentPullBackDist;		// Optionally push near plane out so that we don't clip the world when the flashlight pulls back 
	//OB
	//Tony; hackz0r.
	if ( m_bMuzzleFlashEnabled )
	{
//Now that i'm pointingt he light down i don't have to clamp the distance.. it's creating artifcats being clamped.
//		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( m_nEntIndex );
//		if ( pPlayer && pPlayer != C_BasePlayer::GetLocalPlayer() )
//		{
//			state.m_FarZ = 256.0f; //non local player flashes, clamp a lot.
//		}
//		else
			state.m_FarZ = 750.0f;//r_flashlightfar.GetFloat();
	}
	else
		state.m_FarZ = r_flashlightfar.GetFloat();

	//L4D
	/*
	if ( m_flFarZ > 0.0f )
	{
		state.m_FarZ = state.m_FarZAtten = m_flFarZ;	// Strictly speaking, these are different, but the game can treat them the same
	}
	else
	{
		state.m_FarZ = state.m_FarZAtten = r_flashlightfar.GetFloat();	// Strictly speaking, these are different, but the game can treat them the same
	}
	*/
	state.m_bEnableShadows = castsShadows && r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	if ( m_bMuzzleFlashEnabled )
	{
		state.m_pSpotlightTexture = m_MuzzleFlashTexture;
		state.m_Color[0] = m_flMuzzleFlashBrightness;
		state.m_Color[1] = m_flMuzzleFlashBrightness;
		state.m_Color[2] = m_flMuzzleFlashBrightness;
	}
	else
	{
		state.m_pSpotlightTexture = m_FlashlightTexture;
	}

	state.m_nSpotlightTextureFrame = 0;

	state.m_flShadowAtten = r_flashlightshadowatten.GetFloat();
	//OB
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();
	//L4D
//	state.m_flShadowSlopeScaleDepthBias = g_pMaterialSystemHardwareConfig->GetShadowSlopeScaleDepthBias();
//	state.m_flShadowDepthBias = g_pMaterialSystemHardwareConfig->GetShadowDepthBias();

	return true;
}

void CProjectedLightEffect::UpdateFlashlightTexture( const char* pTextureName )
{
	static const char *pEmptyString = "";

	if ( pTextureName == NULL )
	{
		pTextureName = pEmptyString;
	}

	if ( !m_FlashlightTexture.IsValid() ||
		V_stricmp( m_textureName, pTextureName ) != 0 )
	{
		if ( pTextureName == pEmptyString )
		{
			m_FlashlightTexture.Init( "effects/flashlight001", TEXTURE_GROUP_OTHER, true );
		}
		else
		{
			m_FlashlightTexture.Init( pTextureName, TEXTURE_GROUP_OTHER, true );
		}
		V_strncpy( m_textureName, pTextureName, sizeof( m_textureName ) );
	}
}

bool CProjectedLightEffect::ComputeLightPosAndOrientation( const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp,
														  Vector& vecFinalPos, Quaternion& quatOrientation, bool bTracePlayers )
{
	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	float flDistCutoff = r_flashlighttracedistcutoff.GetFloat();
	const float flDistDrag = 0.2;
	bool bDebugVis = r_flashlightvisualizetrace.GetBool();

	C_SDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex( m_nEntIndex ));
	if ( !pPlayer )
	{
		pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
		if ( !pPlayer )
		{
			Assert( false );
			return false;
		}
	}

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = ( pPlayer->GetMoveType() == MOVETYPE_LADDER );

	CTraceFilterSkipPlayerAndViewModelProjected traceFilter( pPlayer, bTracePlayers );

	//	Vector vOrigin = vecPos + r_flashlightoffsety.GetFloat() * vecUp;
	Vector vecOffset;
	pPlayer->GetFlashlightOffset( vecForward, vecRight, vecUp, &vecOffset );
	Vector vOrigin = vecPos + vecOffset;

	// Not on ladder...trace a hull
	if ( !bPlayerOnLadder ) 
	{
		Vector vecPlayerEyePos = pPlayer->GetRenderOrigin() + pPlayer->GetViewOffset();

		trace_t pmOriginTrace;
		UTIL_TraceHull( vecPlayerEyePos, vOrigin, Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SOLID & ~(CONTENTS_HITBOX) | CONTENTS_WINDOW | CONTENTS_GRATE, &traceFilter, &pmOriginTrace );//1

		if ( bDebugVis )
		{
			debugoverlay->AddBoxOverlay( pmOriginTrace.endpos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), 0, 255, 0, 16, 0 );
			if ( pmOriginTrace.DidHit() || pmOriginTrace.startsolid )
			{
				debugoverlay->AddLineOverlay( pmOriginTrace.startpos, pmOriginTrace.endpos, 255, 128, 128, true, 0 );
			}
			else
			{
				debugoverlay->AddLineOverlay( pmOriginTrace.startpos, pmOriginTrace.endpos, 255, 0, 0, true, 0 );
			}
		}

		if ( pmOriginTrace.DidHit() || pmOriginTrace.startsolid )
		{
			vOrigin = pmOriginTrace.endpos;
		}
		else
		{
			if ( pPlayer->GetFlashlightOrigin() != vecPlayerEyePos )
			{
				vOrigin = vecPos;
			}
		}
	}
	else // on ladder...skip the above hull trace
	{
		vOrigin = vecPos;
	}

	// Now do a trace along the flashlight direction to ensure there is nothing within range to pull back from
	int iMask = MASK_OPAQUE_AND_NPCS;
	iMask &= ~CONTENTS_HITBOX;
	iMask |= CONTENTS_WINDOW | CONTENTS_GRATE;

	Vector vTarget = vOrigin + vecForward * r_flashlightfar.GetFloat();

	// Work with these local copies of the basis for the rest of the function
	Vector vDir   = vTarget - vOrigin;
	Vector vRight = vecRight;
	Vector vUp    = vecUp;
	VectorNormalize( vDir   );
	VectorNormalize( vRight );
	VectorNormalize( vUp    );

	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct( vDir, vUp ) * vDir;
	VectorNormalize( vUp );
	vRight -= DotProduct( vDir, vRight ) * vDir;
	VectorNormalize( vRight );
	vRight -= DotProduct( vUp, vRight ) * vUp;
	VectorNormalize( vRight );

	AssertFloatEquals( DotProduct( vDir, vRight ), 0.0f, 1e-3 );
	AssertFloatEquals( DotProduct( vDir, vUp    ), 0.0f, 1e-3 );
	AssertFloatEquals( DotProduct( vRight, vUp  ), 0.0f, 1e-3 );

	trace_t pmDirectionTrace;
	UTIL_TraceHull( vOrigin, vTarget, Vector( -1.5, -1.5, -1.5 ), Vector( 1.5, 1.5, 1.5 ), iMask, &traceFilter, &pmDirectionTrace );//.5

	if ( bDebugVis )
	{
		debugoverlay->AddBoxOverlay( pmDirectionTrace.endpos, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ), QAngle( 0, 0, 0 ), 0, 0, 255, 16, 0 );
		debugoverlay->AddLineOverlay( vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0 );
	}

	float flTargetPullBackDist = 0.0f;
	float flDist = (pmDirectionTrace.endpos - vOrigin).Length();

	if ( flDist < flDistCutoff )
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_flashlightladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder

		flTargetPullBackDist = flPullBackDist;

		if ( !bPlayerOnLadder )
		{
			trace_t pmBackTrace;
			// start the trace away from the actual trace origin a bit, to avoid getting stuck on small, close "lips"
			UTIL_TraceHull( vOrigin - vDir * ( flDistCutoff * r_flashlightbacktraceoffset.GetFloat() ), vOrigin - vDir * ( flPullBackDist - flEpsilon ),
				Vector( -1.5f, -1.5f, -1.5f ), Vector( 1.5f, 1.5f, 1.5f ), iMask, &traceFilter, &pmBackTrace );

			if ( bDebugVis )
			{
				debugoverlay->AddLineOverlay( pmBackTrace.startpos, pmBackTrace.endpos, 255, 0, 255, true, 0 );
			}

			if( pmBackTrace.DidHit() )
			{
				// We have an intersection behind us as well, so limit our flTargetPullBackDist
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				flTargetPullBackDist = min( flMaxDist, flTargetPullBackDist );
				//m_flCurrentPullBackDist = MIN( flMaxDist, m_flCurrentPullBackDist );	// possible pop
			}
		}
	}

	if ( bDebugVis )
	{
		// visualize pullback
		debugoverlay->AddBoxOverlay( vOrigin - vDir * m_flCurrentPullBackDist, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), 255, 255, 0, 16, 0 );
		debugoverlay->AddBoxOverlay( vOrigin - vDir * flTargetPullBackDist, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 128, 128, 0, 16, 0 );
	}

	m_flCurrentPullBackDist = Lerp( flDistDrag, m_flCurrentPullBackDist, flTargetPullBackDist );
	m_flCurrentPullBackDist = min( m_flCurrentPullBackDist, flDistCutoff );	// clamp to max pullback dist
	vOrigin = vOrigin - vDir * m_flCurrentPullBackDist;

	vecFinalPos = vOrigin;
	BasisToQuaternion( vDir, vRight, vUp, quatOrientation );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectedLightEffect::LightOff()
{
#ifndef NO_TOOLFRAMEWORK
	if ( clienttools->IsInRecordingMode() )
	{
		KeyValues *msg = new KeyValues( "FlashlightState" );
		msg->SetFloat( "time", gpGlobals->curtime );
		msg->SetInt( "entindex", m_nEntIndex );
		msg->SetInt( "flashlightHandle", m_FlashlightHandle );
		msg->SetPtr( "flashlightState", NULL );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if( m_FlashlightHandle != CLIENTSHADOW_INVALID_HANDLE )
	{
		g_pClientShadowMgr->DestroyFlashlight( m_FlashlightHandle );
		m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

void CProjectedLightEffectManager::UpdateFlashlight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float flFov, bool castsShadows, float flFarZ, float flLinearAtten, const char* pTextureName)
{
	if ( m_bFlashlightOverride )
	{
		// don't mess with it while it's overridden
		return;
	}

	C_SDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex( m_nFlashlightEntIndex ));

	bool bMuzzleFlashActive = false;
	
	if (pPlayer)
		bMuzzleFlashActive = ( m_nMuzzleFlashFrameCountdown > 0 ) || (pPlayer->GetCurrentTime() < m_flMuzzleFlashStart + m_muzzleFlashTimer.GetCountdownDuration());

	// Don't let values from a previous level change trigger it.
	if (m_flMuzzleFlashStart - pPlayer->GetCurrentTime() > 5)
		bMuzzleFlashActive = false;

	if ( pPlayer && m_pFlashlightEffect )
	{
		m_flFov = flFov;
		m_flFarZ = flFarZ;
		m_flLinearAtten = flLinearAtten;
		m_pFlashlightEffect->UpdateLight( m_nFlashlightEntIndex, vecPos, vecDir, vecRight, vecUp, flFov, flFarZ, flLinearAtten, castsShadows, pTextureName );

		float flFadeOut = RemapValClamped(pPlayer->GetCurrentTime(), m_flMuzzleFlashStart, m_flMuzzleFlashStart+m_muzzleFlashTimer.GetCountdownDuration(), 1.0, 0.0f);
		m_flMuzzleFlashBrightness = m_flInitialMuzzleFlashBrightness * flFadeOut;

		m_pFlashlightEffect->SetMuzzleFlashEnabled( bMuzzleFlashActive, m_flMuzzleFlashBrightness );
	}

	if ( !bMuzzleFlashActive && !m_bFlashlightOn && m_pFlashlightEffect )
	{
		delete m_pFlashlightEffect;
		m_pFlashlightEffect = NULL;
	}

	if ( bMuzzleFlashActive && !m_bFlashlightOn && !m_pFlashlightEffect )
	{
		m_pFlashlightEffect = new CProjectedLightEffect( m_nFlashlightEntIndex );
		m_pFlashlightEffect->SetMuzzleFlashEnabled( bMuzzleFlashActive, m_flInitialMuzzleFlashBrightness );
	}

	if ( bMuzzleFlashActive && m_nFXComputeFrame != gpGlobals->framecount )
	{
		m_nFXComputeFrame = gpGlobals->framecount;
		m_nMuzzleFlashFrameCountdown--;
	}
}

void CProjectedLightEffectManager::TriggerMuzzleFlash()
{
	C_SDKPlayer *pPlayer = ToSDKPlayer(UTIL_PlayerByIndex( m_nFlashlightEntIndex ));

	if (!pPlayer)
		return;

	m_flMuzzleFlashStart = pPlayer->GetCurrentTime();
	m_nMuzzleFlashFrameCountdown = 2;
	m_muzzleFlashTimer.Start( 0.12f );		// show muzzleflash for 2 frames or 66ms, whichever is longer
	m_flInitialMuzzleFlashBrightness = random->RandomFloat( 0.4f, 2.0f );
}

bool CTraceFilterSkipPlayerAndViewModelProjected::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	// Test against the vehicle too?
	// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
	C_BaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
	if ( !pEntity )
		return true;

	if ( ( dynamic_cast<C_BaseViewModel*>( pEntity ) != NULL ) ||
		pEntity == m_pPlayer ||
		( m_bSkipPlayers && pEntity->IsPlayer() ) ||
		pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
		pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS )
	{
		return false;
	}

	return true;
}


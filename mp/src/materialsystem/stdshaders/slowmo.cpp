//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"

#include "slowmo_vs20.inc"
#include "slowmo_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar da_postprocess_skill_alpha( "da_postprocess_skill_alpha", "0.2", FCVAR_CHEAT, "Control for style skill mode. Set by code." );

ConVar da_postprocess_deathcam( "da_postprocess_deathcam", "0", FCVAR_CHEAT, "Control for death cam mode. Set by code." );
ConVar da_postprocess_skill( "da_postprocess_skill", "0", FCVAR_CHEAT, "Control for style skill mode. Set by code." );

ConVar da_postprocess_vr( "da_postprocess_vr", "0", FCVAR_CHEAT, "Set by code." );

BEGIN_VS_SHADER_FLAGS( slowmo, "Help for slowmo", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE,      SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer" )
		SHADER_PARAM( SLOWMOAMOUNT,     SHADER_PARAM_TYPE_FLOAT, "", "Strength of slow motion effect" )
		SHADER_PARAM( COMBINED,         SHADER_PARAM_TYPE_TEXTURE, "0", "Combined texture input. Red: overlay. Green: grain. Blue: Vignette alpha. Alpha: blur alpha." )
		SHADER_PARAM( GRAINOFFSET,      SHADER_PARAM_TYPE_FLOAT, "0", "Film grain texture offset" )
		SHADER_PARAM( BLUR,             SHADER_PARAM_TYPE_TEXTURE, "_rt_SmallFB1", "Blur texture" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
		LoadTexture( COMBINED );
		LoadTexture( BLUR );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( slowmo_vs20 );
			SET_STATIC_VERTEX_SHADER( slowmo_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( slowmo_ps20b );
			//SET_STATIC_PIXEL_SHADER_COMBO( WITH_BLUR, g_pHardwareConfig->GetDXSupportLevel() >= 90 );
			SET_STATIC_PIXEL_SHADER_COMBO( WITH_BLUR, 0 ); // Blur's not working right now for lack of a blurred buffer to render from.
			psh_forgot_to_set_static_WITH_BLUR = 0; // This is a dirty workaround to the shortcut [= 0] in the fxc
			SET_STATIC_PIXEL_SHADER( slowmo_ps20b );
		}

		DYNAMIC_STATE
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( slowmo_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO(WITH_VR, da_postprocess_vr.GetBool());
			SET_DYNAMIC_PIXEL_SHADER( slowmo_ps20b );

			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0 );

			float aflSlowMo[1] = { 0.0f };
			aflSlowMo[0] = params[SLOWMOAMOUNT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 0, aflSlowMo );

			float aflSkillAlpha[1] = { 0.0f };
			aflSkillAlpha[0] = da_postprocess_skill_alpha.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 1, aflSkillAlpha );

			float aflOffset[1] = { 0.0f };
			aflOffset[0] = params[GRAINOFFSET]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 5, aflOffset );

			float aflDeathCam[1] = { 0.0f };
			aflDeathCam[0] = da_postprocess_deathcam.GetBool()?1:0;
			pShaderAPI->SetPixelShaderConstant( 6, aflDeathCam );

			float aflStyleSkill[1] = { 0.0f };
			aflStyleSkill[0] = da_postprocess_skill.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 7, aflStyleSkill );

			BindTexture( SHADER_SAMPLER1, COMBINED, -1 );
			BindTexture( SHADER_SAMPLER2, BLUR, -1 );

			DECLARE_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
		}
		Draw();
	}
END_SHADER

//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "basevsshader.h"

#include "slowmo_vs20.inc"
#include "slowmo_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar da_postprocess_slowfilter( "da_postprocess_slowfilter", "0.09", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much does the blue filter show up during slowmo?" );
ConVar da_postprocess_vignette( "da_postprocess_vignette", "1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much to scale the vignette by?" );
ConVar da_postprocess_grain( "da_postprocess_grain", "0.05", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much to scale the grain by?" );
ConVar da_postprocess_bias( "da_postprocess_bias", "0.65", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much to bias the colors by?" );

BEGIN_VS_SHADER_FLAGS( slowmo, "Help for slowmo", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE,      SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer" )
		SHADER_PARAM( SLOWMOAMOUNT,     SHADER_PARAM_TYPE_FLOAT, "", "Strength of slow motion effect" )
		SHADER_PARAM( VIGNETTE,         SHADER_PARAM_TYPE_TEXTURE, "0", "Vignette texture" )
		SHADER_PARAM( GRAIN,            SHADER_PARAM_TYPE_TEXTURE, "0", "Film grain texture" )
		SHADER_PARAM( GRAINOFFSET,      SHADER_PARAM_TYPE_FLOAT, "0", "Film grain texture offset" )
		SHADER_PARAM( OVERLAY,          SHADER_PARAM_TYPE_TEXTURE, "0", "Overlay texture" )
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
		LoadTexture( VIGNETTE );
		LoadTexture( GRAIN );
		LoadTexture( OVERLAY );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( slowmo_vs20 );
			SET_STATIC_VERTEX_SHADER( slowmo_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( slowmo_ps20b );
			SET_STATIC_PIXEL_SHADER( slowmo_ps20b );
		}

		DYNAMIC_STATE
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0 );

			float aflSlowMo[1] = { 0.0f };
			aflSlowMo[0] = params[SLOWMOAMOUNT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 0, aflSlowMo );

			float aflSlowFilter[1] = { 0.0f };
			aflSlowFilter[0] = da_postprocess_slowfilter.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 1, aflSlowFilter );

			float aflVignette[1] = { 0.0f };
			aflVignette[0] = da_postprocess_vignette.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 2, aflVignette );

			float aflGrain[1] = { 0.0f };
			aflGrain[0] = da_postprocess_grain.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 3, aflGrain );

			float aflBias[1] = { 0.0f };
			aflBias[0] = da_postprocess_bias.GetFloat();
			pShaderAPI->SetPixelShaderConstant( 4, aflBias );

			float aflOffset[1] = { 0.0f };
			aflOffset[0] = params[GRAINOFFSET]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 5, aflOffset );

			BindTexture( SHADER_SAMPLER1, VIGNETTE, -1 );
			BindTexture( SHADER_SAMPLER2, GRAIN, -1 );
			BindTexture( SHADER_SAMPLER3, OVERLAY, -1 );

			DECLARE_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
		}
		Draw();
	}
END_SHADER

//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "basevsshader.h"

#include "slowmo_vs20.inc"
#include "slowmo_ps20.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar da_postprocess_slowfilter( "da_postprocess_slowfilter", "0.05", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much does the blue filter show up during slowmo?" );

BEGIN_VS_SHADER_FLAGS( slowmo, "Help for slowmo", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE,    SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer" )
		SHADER_PARAM( SLOWMOAMOUNT,   SHADER_PARAM_TYPE_FLOAT, "", "Strength of slow motion effect" )
		SHADER_PARAM( GRAIN_TEXTURE,  SHADER_PARAM_TYPE_TEXTURE, "0", "Film grain texture" )
		SHADER_PARAM( NOISESCALE,     SHADER_PARAM_TYPE_VEC4, "", "Strength of film grain" )
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
		LoadTexture( GRAIN_TEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( slowmo_vs20 );
			SET_STATIC_VERTEX_SHADER( slowmo_vs20 );

			DECLARE_STATIC_PIXEL_SHADER( slowmo_ps20 );
			SET_STATIC_PIXEL_SHADER( slowmo_ps20 );
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

			SetPixelShaderConstant( 2, NOISESCALE );

//			BindTexture( SHADER_SAMPLER1, GRAIN_TEXTURE, -1 );

			DECLARE_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( slowmo_vs20 );
		}
		Draw();
	}
END_SHADER

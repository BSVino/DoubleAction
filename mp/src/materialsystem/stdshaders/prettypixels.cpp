//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"

#include "prettypixels_vs30.inc"
#include "prettypixels_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS( prettypixels, "Help for pretty pixels", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE,      SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB_DA", "Framebuffer" )
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
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			DECLARE_STATIC_VERTEX_SHADER( prettypixels_vs30 );
			SET_STATIC_VERTEX_SHADER( prettypixels_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( prettypixels_ps30 );
			SET_STATIC_PIXEL_SHADER( prettypixels_ps30 );
		}

		DYNAMIC_STATE
		{
			DECLARE_DYNAMIC_PIXEL_SHADER( prettypixels_ps30 );
			SET_DYNAMIC_PIXEL_SHADER( prettypixels_ps30 );

			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0 );

			DECLARE_DYNAMIC_VERTEX_SHADER( prettypixels_vs30 );
			SET_DYNAMIC_VERTEX_SHADER( prettypixels_vs30 );
		}
		Draw();
	}
END_SHADER

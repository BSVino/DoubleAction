//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"

#include "model_types.h"

#include "c_da_briefcase.h"

#include "c_da_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#undef CBriefcase
#undef CBriefcaseCaptureZone
#undef CRatRaceWaypoint

IMPLEMENT_CLIENTCLASS_DT(C_Briefcase, DT_Briefcase, CBriefcase)
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_Briefcase )
END_PREDICTION_DATA();

C_Briefcase::C_Briefcase()
{
}

bool C_Briefcase::ShouldDraw()
{
	return true;
}

void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha);

CMaterialReference g_hBriefcaseArrow;
int C_Briefcase::DrawModel(int flags)
{
	if ( vcollide_wireframe.GetBool() )
	{
		if ( IsSolid() && CollisionProp()->GetSolid() == SOLID_VPHYSICS )
		{
			vcollide_t *pCollide = modelinfo->GetVCollide( GetModelIndex() );
			if (pCollide)
			{
				for (int i = 0; i < pCollide->solidCount; i++)
				{
					static color32 debugColor = {0,255,255,0};
					matrix3x4_t matrix;
					AngleMatrix( GetAbsAngles(), GetAbsOrigin(), matrix );
					engine->DebugDrawPhysCollide( pCollide->solids[i], NULL, matrix, debugColor );
					if ( VPhysicsGetObject() )
					{
						static color32 debugColorPhys = {255,0,0,0};
						matrix3x4_t matrix;
						VPhysicsGetObject()->GetPositionMatrix( &matrix );
						engine->DebugDrawPhysCollide( pCollide->solids[i], NULL, matrix, debugColorPhys );
					}
				}
			}
		}
	}

	if (!g_hBriefcaseArrow.IsValid())
		g_hBriefcaseArrow.Init( "particle/briefcase.vmt", TEXTURE_GROUP_OTHER );

	int iReturn = BaseClass::DrawModel(flags);

	if (flags & STUDIO_SSAODEPTHTEXTURE)
		return iReturn;

	C_DAPlayer* pLocalPlayer = C_DAPlayer::GetLocalOrSpectatedPlayer();
	if (!pLocalPlayer)
		return iReturn;

	float flTime = pLocalPlayer->GetCurrentTime() + m_flArrowSpinOffset;

	Vector vecOrigin = GetAbsOrigin() + Vector(0, 0, 40);
	Vector vecRight = Vector(sin(flTime*4), cos(flTime*4), 0);
	Vector vecUp = Vector(0, 0, 1);

	float flSize = 25;

	if (GetOwnerEntity())
	{
		vecOrigin = GetOwnerEntity()->GetAbsOrigin() + Vector(0, 0, GetOwnerEntity()->GetCollideable()->OBBMaxs().z + 20);
		flSize = 15;
	}

	DrawIconQuad(g_hBriefcaseArrow, vecOrigin, vecRight, vecUp, flSize, 1);

	return iReturn;
}

IMPLEMENT_CLIENTCLASS_DT(C_BriefcaseCaptureZone, DT_BriefcaseCaptureZone, CBriefcaseCaptureZone)
	RecvPropFloat( RECVINFO(m_flCaptureRadius) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BriefcaseCaptureZone )
END_PREDICTION_DATA();

C_BriefcaseCaptureZone::C_BriefcaseCaptureZone()
{
}

bool C_BriefcaseCaptureZone::ShouldDraw()
{
	return true;
}

void C_BriefcaseCaptureZone::GetRenderBounds( Vector& mins, Vector& maxs )
{
	// Expand render bounds so we don't get frustum culled.
	mins = Vector(-m_flCaptureRadius, -m_flCaptureRadius, -m_flCaptureRadius) * 1.1f;
	maxs = Vector(m_flCaptureRadius, m_flCaptureRadius, m_flCaptureRadius) * 1.1f;
}

CMaterialReference g_hCaptureZone;
int C_BriefcaseCaptureZone::DrawModel(int flags)
{
	if (flags & STUDIO_SSAODEPTHTEXTURE)
		return 0;

	if (!g_hCaptureZone.IsValid())
		g_hCaptureZone.Init( "da/capturezone.vmt", TEXTURE_GROUP_OTHER );
	if (!g_hBriefcaseArrow.IsValid())
		g_hBriefcaseArrow.Init( "particle/briefcase.vmt", TEXTURE_GROUP_OTHER );

	int iCylinderEdges = 40;
	Vector vecCenter = WorldSpaceCenter();

	if (!C_DAPlayer::GetLocalOrSpectatedPlayer())
		return 0;

	float flTime = C_DAPlayer::GetLocalOrSpectatedPlayer()->GetCurrentTime();

	Vector vecOrigin = vecCenter + Vector(0, 0, 20);
	Vector vecRight = Vector(sin(flTime*4), cos(flTime*4), 0);
	Vector vecUp = Vector(0, 0, 1);

	DrawIconQuad(g_hBriefcaseArrow, vecOrigin, vecRight, vecUp, 25, 1);

	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( g_hCaptureZone );
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	Vector vecX, vecY;

	vecX = Vector(m_flCaptureRadius, 0, 0);
	vecY = Vector(0, m_flCaptureRadius, 0);

	float flHeight = 50;

	for (int i = 0; i < iCylinderEdges; i++)
	{
		float flAngle1 = (float)i * 2 * M_PI / iCylinderEdges;
		float flAngle2 = (float)(i+1) * 2 * M_PI / iCylinderEdges;

		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flCaptureRadius * flAngle2 / flHeight, 0 );
		meshBuilder.Position3fv( (vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flCaptureRadius * flAngle1 / flHeight, 0 );
		meshBuilder.Position3fv( (vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flCaptureRadius * flAngle1 / flHeight, 1 );
		meshBuilder.Position3fv( (vecCenter + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flCaptureRadius * flAngle2 / flHeight, 1 );
		meshBuilder.Position3fv( (vecCenter + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.End(false, true);
	}

	return 1;
}

IMPLEMENT_CLIENTCLASS_DT(C_RatRaceWaypoint, DT_RatRaceWaypoint, CRatRaceWaypoint)
	RecvPropFloat( RECVINFO(m_flRadius) ),
	RecvPropInt( RECVINFO(m_iWaypoint) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_RatRaceWaypoint )
END_PREDICTION_DATA();

C_RatRaceWaypoint::C_RatRaceWaypoint()
{
}

bool C_RatRaceWaypoint::ShouldDraw()
{
	return true;
}

void C_RatRaceWaypoint::GetRenderBounds( Vector& mins, Vector& maxs )
{
	// Expand render bounds so we don't get frustum culled.
	mins = Vector(-m_flRadius, -m_flRadius, -m_flRadius) * 1.1f;
	maxs = Vector(m_flRadius, m_flRadius, m_flRadius) * 1.1f;
}

int C_RatRaceWaypoint::DrawModel(int flags)
{
	if (flags & STUDIO_SSAODEPTHTEXTURE)
		return 0;

	if (!g_hCaptureZone.IsValid())
		g_hCaptureZone.Init( "da/capturezone.vmt", TEXTURE_GROUP_OTHER );

	int iCylinderEdges = 40;
	Vector vecCenter = WorldSpaceCenter();

	if (!C_DAPlayer::GetLocalOrSpectatedPlayer())
		return 0;

	float flTime = C_DAPlayer::GetLocalOrSpectatedPlayer()->GetCurrentTime();

	Vector vecOrigin = vecCenter + Vector(0, 0, 20);
	Vector vecRight = Vector(sin(flTime*4), cos(flTime*4), 0);
	Vector vecUp = Vector(0, 0, 1);

	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( g_hCaptureZone );
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	Vector vecX, vecY;

	vecX = Vector(m_flRadius, 0, 0);
	vecY = Vector(0, m_flRadius, 0);

	float flHeight = 50;

	for (int i = 0; i < iCylinderEdges; i++)
	{
		float flAngle1 = (float)i * 2 * M_PI / iCylinderEdges;
		float flAngle2 = (float)(i+1) * 2 * M_PI / iCylinderEdges;

		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flRadius * flAngle2 / flHeight, 0 );
		meshBuilder.Position3fv( (vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flRadius * flAngle1 / flHeight, 0 );
		meshBuilder.Position3fv( (vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flRadius * flAngle1 / flHeight, 1 );
		meshBuilder.Position3fv( (vecCenter + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f( 1, 1, 1, 1 );
		meshBuilder.TexCoord2f( 0, m_flRadius * flAngle2 / flHeight, 1 );
		meshBuilder.Position3fv( (vecCenter + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base() );
		meshBuilder.AdvanceVertex();

		meshBuilder.End(false, true);
	}

	return 1;
}

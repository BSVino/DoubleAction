//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A floating health bar over the top of players in game
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"

#include "model_types.h"

#include "da_floating_healthbar.h"

#include "c_sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"







IMPLEMENT_CLIENTCLASS_DT(C_FloatingHealthbar, DT_FloatingHealthbar, CFloatingHealthbar)
RecvPropFloat(RECVINFO(m_flCaptureRadius)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_FloatingHealthbar)
END_PREDICTION_DATA();

C_FloatingHealthbar::C_FloatingHealthbar()
{
}

bool C_FloatingHealthbar::ShouldDraw()
{
	return true;
}

void C_FloatingHealthbar::GetRenderBounds(Vector& mins, Vector& maxs)
{
	// Expand render bounds so we don't get frustum culled.
	mins = Vector(-m_flCaptureRadius, -m_flCaptureRadius, -m_flCaptureRadius) * 1.1f;
	maxs = Vector(m_flCaptureRadius, m_flCaptureRadius, m_flCaptureRadius) * 1.1f;
}

void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha);


CMaterialReference g_hCaptureZonea;
CMaterialReference g_hBriefcaseArrowa;
int C_FloatingHealthbar::DrawModel(int flags)
{
	if (flags & STUDIO_SSAODEPTHTEXTURE)
		return 0;

	if (!g_hCaptureZonea.IsValid())
		g_hCaptureZonea.Init("da/capturezone.vmt", TEXTURE_GROUP_OTHER);
	if (!g_hBriefcaseArrowa.IsValid())
		g_hBriefcaseArrowa.Init("particle/briefcase.vmt", TEXTURE_GROUP_OTHER);

	int iCylinderEdges = 40;
	Vector vecCenter = WorldSpaceCenter();

	if (!C_SDKPlayer::GetLocalOrSpectatedPlayer()){
		DevMsg("\n\n no local player \n \n");
		return 0;
	}

	float flTime = C_SDKPlayer::GetLocalOrSpectatedPlayer()->GetCurrentTime();

	Vector vecOrigin = vecCenter + Vector(0, 0, 20);
	Vector vecRight = Vector(sin(flTime * 4), cos(flTime * 4), 0);
	Vector vecUp = Vector(0, 0, 1);

	DrawIconQuad(g_hBriefcaseArrowa, vecOrigin, vecRight, vecUp, 25, 1);

	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(g_hCaptureZonea);
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	Vector vecX, vecY;

	vecX = Vector(m_flCaptureRadius, 0, 0);
	vecY = Vector(0, m_flCaptureRadius, 0);

	float flHeight = 50;

	for (int i = 0; i < iCylinderEdges; i++)
	{
		float flAngle1 = (float)i * 2 * M_PI / iCylinderEdges;
		float flAngle2 = (float)(i + 1) * 2 * M_PI / iCylinderEdges;

		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		meshBuilder.Color4f(1, 1, 1, 1);
		meshBuilder.TexCoord2f(0, m_flCaptureRadius * flAngle2 / flHeight, 0);
		meshBuilder.Position3fv((vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f(1, 1, 1, 1);
		meshBuilder.TexCoord2f(0, m_flCaptureRadius * flAngle1 / flHeight, 0);
		meshBuilder.Position3fv((vecCenter + Vector(0, 0, flHeight) + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f(1, 1, 1, 1);
		meshBuilder.TexCoord2f(0, m_flCaptureRadius * flAngle1 / flHeight, 1);
		meshBuilder.Position3fv((vecCenter + vecX * cos(flAngle1) + vecY * sin(flAngle1)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4f(1, 1, 1, 1);
		meshBuilder.TexCoord2f(0, m_flCaptureRadius * flAngle2 / flHeight, 1);
		meshBuilder.Position3fv((vecCenter + vecX * cos(flAngle2) + vecY * sin(flAngle2)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.End(false, true);
	}

	return 1;
}







/*
C_FloatingHealthbar::C_FloatingHealthbar(C_SDKPlayer* pPlayer)
{
	DevMsg("\nsETTING oWNER \n\n");
	pOwner = pPlayer;
}


bool C_FloatingHealthbar::ShouldDraw(void){
	DevMsg("\nshouldDraw true \n\n");
	return true;
}

void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha);
/*
void C_FloatingHealthbar::DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha)
{
	CMeshBuilder meshBuilder;
	DevMsg("\nDrawIconQuad \n\n");

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m);
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.Position3fv((vecOrigin + (vecRight * -flSize) + (vecUp * flSize)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.Position3fv((vecOrigin + (vecRight * flSize) + (vecUp * flSize)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.Position3fv((vecOrigin + (vecRight * flSize) + (vecUp * -flSize)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.Position3fv((vecOrigin + (vecRight * -flSize) + (vecUp * -flSize)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.End(false, true);
}*/
/*
void DrawBarMesh(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flWidth, float flHeight, const float& flAlpha)
{
	CMeshBuilder meshBuilder;

	flHeight = flHeight / 2;
	flWidth = flWidth / 2;

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m);
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.Position3fv((vecOrigin + (vecRight * -flWidth) + (vecUp * flHeight)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.Position3fv((vecOrigin + (vecRight * flWidth) + (vecUp * flHeight)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.Position3fv((vecOrigin + (vecRight * flWidth) + (vecUp * -flHeight)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f(flAlpha, flAlpha, flAlpha, flAlpha);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.Position3fv((vecOrigin + (vecRight * -flWidth) + (vecUp * -flHeight)).Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.End(false, true);
}*/
/*

int C_FloatingHealthbar::DrawModel(int flags)
{
	DevMsg("\nDrawModel\n\n");


	if (!g_hHealthbarMaterial.IsValid())
		g_hHealthbarMaterial.Init("particle/briefcase.vmt", TEXTURE_GROUP_OTHER);

	int iReturn = BaseClass::DrawModel(flags);

	if (flags & STUDIO_SSAODEPTHTEXTURE)
		return iReturn;

//	int iCurHealth = pOwner->GetHealth();
//	int iMaxHealth = pOwner->GetMaxHealth();

	float fBarWidth = 100;
	//float fBarHeight = 20;
	float fBarAlpha = 1;

	Vector vecOrigin = pOwner->GetAbsOrigin() + Vector(0, 0, 16);
	Vector vecRight;
	Vector vecUp;

	AngleVectors(pOwner->GetLocalAngles(), NULL, &vecRight, &vecUp);

	DrawIconQuad(g_hHealthbarMaterial, vecOrigin, vecRight, vecUp, fBarWidth, fBarAlpha);
	
	return 1;
}
*/
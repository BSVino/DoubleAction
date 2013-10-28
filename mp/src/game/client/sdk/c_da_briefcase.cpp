//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"

#include "c_sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Briefcase : public C_BaseAnimating
{
	DECLARE_CLASS( C_Briefcase, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_Briefcase();

public:
	virtual bool    ShouldDraw( void );
	virtual int     DrawModel( int flags );

private:
	float       m_flArrowSpinOffset;
};

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

void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize);

CMaterialReference g_hBriefcaseArrow;
int C_Briefcase::DrawModel(int flags)
{
	if (!g_hBriefcaseArrow.IsValid())
		g_hBriefcaseArrow.Init( "particle/briefcase.vmt", TEXTURE_GROUP_OTHER );

	int iReturn = BaseClass::DrawModel(flags);

	if (GetOwnerEntity())
		return iReturn;

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (!pLocalPlayer)
		return iReturn;

	float flTime = C_SDKPlayer::GetLocalSDKPlayer()->GetCurrentTime() + m_flArrowSpinOffset;

	Vector vecOrigin = GetAbsOrigin() + Vector(0, 0, 20);
	Vector vecRight = Vector(sin(flTime*4), cos(flTime*4), 0);
	Vector vecUp = Vector(0, 0, 1);

	float flSize = 25;

	DrawIconQuad(g_hBriefcaseArrow, vecOrigin, vecRight, vecUp, flSize);

	return iReturn;
}

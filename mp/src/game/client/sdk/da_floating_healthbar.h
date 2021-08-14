#pragma once


#include "c_baseanimating.h"
#include "c_sdk_player.h"



/*
class C_FloatingHealthbar : public C_BaseAnimating
{
	DECLARE_CLASS(C_FloatingHealthbar, C_BaseAnimating);

public:
	C_FloatingHealthbar(C_SDKPlayer* pPlayer);
	//void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha);

public:
	virtual bool    ShouldDraw( void );
	virtual int     DrawModel( int flags );
	C_SDKPlayer* pOwner;

private:
	CMaterialReference g_hHealthbarMaterial;

};

#define CFloatingHealthbar C_FloatingHealthbar

*/


class C_FloatingHealthbar : public C_BaseEntity
{
	DECLARE_CLASS(C_FloatingHealthbar, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_FloatingHealthbar();

public:
	virtual bool    ShouldDraw(void);
	virtual void    GetRenderBounds(Vector& mins, Vector& maxs);
	virtual int     DrawModel(int flags);
	virtual bool    IsTransparent(void) { return true; }

private:
	float  m_flCaptureRadius;
};

#define CFloatingHealthbar C_FloatingHealthbar
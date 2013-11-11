#pragma once

#include "viewrender.h"

class CDAViewRender : public CViewRender
{
public:
	CDAViewRender();

	virtual void Render2DEffectsPreHUD( const CViewSetup &view );

private:
	void PerformSlowMoEffect( const CViewSetup &view );

	float m_flStyleLerp;
};

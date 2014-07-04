#pragma once

#include "viewrender.h"

class CDAViewRender : public CViewRender
{
	DECLARE_CLASS( CDAViewRender, CViewRender );

public:
	CDAViewRender();

	void Init();

	virtual void Render2DEffectsPreHUD( const CViewSetup &view );
	virtual void RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw );

private:
	void PerformSlowMoEffect( const CViewSetup &view );

	float m_flStyleLerp;
};

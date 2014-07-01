#pragma once

#include "viewrender.h"

class CDAViewRender : public CViewRender
{
	DECLARE_CLASS( CDAViewRender, CViewRender );

public:
	CDAViewRender();

	virtual void Init();

	virtual void DrawWorldAndEntities( bool drawSkybox, const CViewSetup &view, int nClearFlags, ViewCustomVisibility_t *pCustomVisibility = NULL );

	virtual void Render2DEffectsPreHUD( const CViewSetup &view );
	virtual void RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw );

private:
	void PerformSlowMoEffect( const CViewSetup &view );

	void DrawPrettyPixels( const CViewSetup &view );

	float m_flStyleLerp;
};

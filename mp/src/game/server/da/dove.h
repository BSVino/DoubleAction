#pragma once

#include "BaseAnimatingOverlay.h"

class CDove : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CDove, CBaseAnimatingOverlay );
	DECLARE_DATADESC();

public:
	void Precache();
	void Spawn();

	void FlyTo(const Vector& vecEnd);

	void FlyThink();

private:
	float  m_flStartTime;
	float  m_flEndTime;
	Vector m_vecStartPoint;
	Vector m_vecEndPoint;
	Vector m_vecDirection;
	float  m_flSlowMoMultiplier;
	float  m_flChangeAnimTime;

public:
	static void SpawnDoves(class CDAPlayer* pPlayer);
};

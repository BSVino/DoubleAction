#pragma once

#include "c_baseanimating.h"

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

#define CBriefcase C_Briefcase

class C_BriefcaseCaptureZone : public C_BaseEntity
{
	DECLARE_CLASS( C_BriefcaseCaptureZone, C_BaseEntity );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_BriefcaseCaptureZone();

public:
	virtual bool    ShouldDraw( void );
	virtual void    GetRenderBounds( Vector& mins, Vector& maxs );
	virtual int     DrawModel( int flags );
	virtual bool    IsTransparent( void ) { return true; }

private:
	float  m_flCaptureRadius;
};

#define CBriefcaseCaptureZone C_BriefcaseCaptureZone

class C_RatRaceWaypoint : public C_BaseEntity
{
	DECLARE_CLASS( C_RatRaceWaypoint, C_BaseEntity );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_RatRaceWaypoint();

public:
	virtual bool    ShouldDraw( void );
	virtual void    GetRenderBounds( Vector& mins, Vector& maxs );
	virtual int     DrawModel( int flags );
	virtual bool    IsTransparent( void ) { return true; }

	int  GetWaypoint() { return m_iWaypoint; }

private:
	float  m_flRadius;
	int    m_iWaypoint;
};

#define CRatRaceWaypoint C_RatRaceWaypoint

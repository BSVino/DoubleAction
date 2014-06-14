#pragma once

#include "predicted_viewmodel.h"

#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )
#define CDAViewModel C_DAViewModel
#endif

class CDAViewModel : public CPredictedViewModel
{
	DECLARE_CLASS( CDAViewModel, CPredictedViewModel );

	friend void RecvProxy_SequenceNum( const class CRecvProxyData *pData, void *pStruct, void *pOut );

public:
	CDAViewModel();

public:
	DECLARE_NETWORKCLASS();

	virtual float	GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );

	bool IsAnimationPaused() { return m_bPaused; }
	void PauseAnimation();
	void UnpauseAnimation(float flRewind = 0);

	virtual void DoMuzzleFlash();

#if defined( CLIENT_DLL )
	virtual int     DrawModel( int flags );
	virtual void FireObsoleteEvent( const Vector& origin, const QAngle& angles, int event, const char *options ) {};
#endif

	virtual void CalcViewModelLag( Vector& origin, QAngle& angles, QAngle& original_angles ) {};
	virtual void AddViewModelBob( CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles );

	CWeaponSDKBase* GetDAWeapon() { return static_cast<CWeaponSDKBase*>(GetOwningWeapon()); }

private:
	Vector m_vecPlayerVelocityLerp;
	QAngle m_angLastPlayerEyeAngles;

	bool   m_bPaused;
	float  m_flPauseCycle;
	float  m_flPauseAnimTime;
	int    m_nPauseSequence;
	float  m_flResumeCycle;
	float  m_flResumeAnimTime;
};

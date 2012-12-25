#pragma once

#include "predicted_viewmodel.h"

#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )
#define CDABViewModel C_DABViewModel
#endif

class CDABViewModel : public CPredictedViewModel
{
	DECLARE_CLASS( CDABViewModel, CPredictedViewModel );

public:
	DECLARE_NETWORKCLASS();

	virtual float	GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );

	virtual void DoMuzzleFlash();

	virtual void CalcViewModelLag( Vector& origin, QAngle& angles, QAngle& original_angles ) {};
	virtual void AddViewModelBob( CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles );

	CWeaponSDKBase* GetDAWeapon() { return static_cast<CWeaponSDKBase*>(GetOwningWeapon()); }
};

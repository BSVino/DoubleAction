#ifndef WEAPON_AKIMBOBASE_H
#define WEAPON_AKIMBOBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase.h"
#ifdef CLIENT_DLL
	#define CAkimbobase C_Akimbobase
#endif
class CAkimbobase : public CWeaponSDKBase
{
public:
	DECLARE_CLASS(CAkimbobase, CWeaponSDKBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	virtual int GetTracerAttachment (void);
	virtual Activity GetIdleActivity (void);
	virtual Activity GetPrimaryAttackActivity (void);
};
#endif
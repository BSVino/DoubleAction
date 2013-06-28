//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef VEHICLE_HL2BUGGY_H
#define VEHICLE_HL2BUGGY_H
#ifdef _WIN32
#pragma once
#endif

#include "vehicle_jeep.h"
#include "ai_basenpc.h"

class CParticleSystem;
class CSprite;

#define NUM_WHEEL_EFFECTS	2

//=============================================================================
// Episodic jeep

class CPropHL2Buggy : public CPropJeep
{
	DECLARE_CLASS( CPropHL2Buggy, CPropJeep );
//	DECLARE_SERVERCLASS();

public:
					CPropHL2Buggy( void );

	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	Think( void );
	virtual void	UpdateOnRemove( void );

	virtual void	NPC_FinishedEnterVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	virtual void	NPC_FinishedExitVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	
	virtual bool	NPC_CanEnterVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	virtual bool	NPC_CanExitVehicle( CAI_BaseNPC *pPassenger, bool bCompanion );
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	Precache( void );
	virtual void	EnterVehicle( CBaseCombatCharacter *pPassenger );
	virtual void	ExitVehicle( int nRole );
	virtual bool	AllowBlockedExit( CBaseCombatCharacter *pPassenger, int nRole );
	
	// Passengers take no damage except what we pass them
	virtual bool	PassengerShouldReceiveDamage( CTakeDamageInfo &info ) 
	{ 
		if ( GetServerVehicle() && GetServerVehicle()->IsPassengerExiting() )
			return false;

		return ( info.GetDamageType() & DMG_VEHICLE ) != 0; 
	}

	virtual int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | FCAP_NOTIFY_ON_TRANSITION); }


	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual int DrawDebugTextOverlays( void );

	DECLARE_DATADESC();

protected:
	virtual float			GetUprightTime( void ) { return 1.0f; }
	virtual float			GetUprightStrength( void );
	virtual bool			ShouldPuntUseLaunchForces( PhysGunForce_t reason ) { return ( reason == PHYSGUN_FORCE_PUNTED ); }
	virtual void			HandleWater( void );

	virtual AngularImpulse	PhysGunLaunchAngularImpulse( void );
	virtual Vector			PhysGunLaunchVelocity( const Vector &forward, float flMass );

private:
	
	void	UpdateWheelDust( void );

	void	InputLockEntrance( inputdata_t &data );
	void	InputUnlockEntrance( inputdata_t &data );
	void	InputLockExit( inputdata_t &data );
	void	InputUnlockExit( inputdata_t &data );
	void	InputDisablePhysGun( inputdata_t &data );
	void	InputEnablePhysGun( inputdata_t &data );
	void	InputCreateLinkController( inputdata_t &data );
	void	InputDestroyLinkController( inputdata_t &data );
	void	CreateAvoidanceZone( void );

	bool	m_bEntranceLocked;
	bool	m_bExitLocked;
	bool	m_bBlink;

	float	m_flNextAvoidBroadcastTime; // Next time we'll warn entity to move out of us

	COutputEvent	m_OnCompanionEnteredVehicle;	// Passenger has completed entering the vehicle
	COutputEvent	m_OnCompanionExitedVehicle;		// Passenger has completed exited the vehicle
	COutputEvent	m_OnHostileEnteredVehicle;	// Passenger has completed entering the vehicle
	COutputEvent	m_OnHostileExitedVehicle;		// Passenger has completed exited the vehicle

	CHandle< CParticleSystem >			m_hWheelDust[NUM_WHEEL_EFFECTS];
	CHandle< CParticleSystem >			m_hWheelWater[NUM_WHEEL_EFFECTS];

	float								m_flNextWaterSound;

	EHANDLE	m_hLinkControllerFront;
	EHANDLE m_hLinkControllerRear;
};

#endif // VEHICLE_HL2BUGGY_H

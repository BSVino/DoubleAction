//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef BASECSGRENADE_PROJECTILE_H
#define BASECSGRENADE_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"
#include "sdk_shareddefs.h"


#ifdef CLIENT_DLL
	#define CBaseGrenadeProjectile C_BaseGrenadeProjectile
#endif


class CBaseGrenadeProjectile : public CBaseGrenade
{
public:
	DECLARE_CLASS( CBaseGrenadeProjectile, CBaseGrenade );
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual void Spawn();

public:
	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{	return SDK_WEAPON_NONE; }

	// This gets sent to the client and placed in the client's interpolation history
	// so the projectile starts out moving right off the bat.
	CNetworkVector( m_vInitialVelocity );


#ifdef CLIENT_DLL
	CBaseGrenadeProjectile() {}
	CBaseGrenadeProjectile( const CBaseGrenadeProjectile& ) {}
	virtual int DrawModel( int flags );
	virtual void PostDataUpdate( DataUpdateType_t type );
	
	float m_flSpawnTime;
#else
	DECLARE_DATADESC();

	virtual void		Explode( trace_t *pTrace, int bitsDamageType );

	bool	CreateVPhysics( void );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	//Constants for all CS Grenades
	static inline float GetGrenadeGravity() { return 0.4f; }
	static inline const float GetGrenadeFriction() { return 0.2f; }
	static inline const float GetGrenadeElasticity() { return 0.45f; }

	//Think function to emit danger sounds for the AI
	void DangerSoundThink( void );
	
	virtual float GetShakeAmplitude( void ) { return 0.0f; }

	// Specify what velocity we want the grenade to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	void SetupInitialTransmittedGrenadeVelocity( const Vector &velocity );

protected:

	//Set the time to detonate ( now + timer )
	void SetDetonateTimerLength( float timer );

private:	
	
	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );
	
	float m_flDetonateTime;
	Vector		vecLastOrigin;

	bool	m_inSolid;

	float m_flNextBounceSound;
#endif
};


#endif // BASECSGRENADE_PROJECTILE_H

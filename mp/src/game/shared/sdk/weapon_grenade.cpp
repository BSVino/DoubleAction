//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_grenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"

#endif


#define GRENADE_TIMER	3.0f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenade, DT_WeaponGrenade )

BEGIN_NETWORK_TABLE(CWeaponGrenade, DT_WeaponGrenade)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenade )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade, CWeaponGrenade );
PRECACHE_WEAPON_REGISTER( weapon_grenade );


#ifdef GAME_DLL

#define GRENADE_MODEL "models/Weapons/w_eq_fraggrenade_thrown.mdl"

class CGrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CGrenadeProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{	return SDK_WEAPON_GRENADE; }

	// Overrides.
public:
	virtual void Spawn()
	{
		SetModel( GRENADE_MODEL );
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheModel( GRENADE_MODEL );
		BaseClass::Precache();
	}

	// Grenade stuff.
public:

	static CGrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CGrenadeProjectile *pGrenade = (CGrenadeProjectile*)CBaseEntity::Create( "grenade_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pGrenade->SetVelocity( velocity, angVelocity );

		pGrenade->SetDetonateTimerLength( timer );
		pGrenade->SetAbsVelocity( velocity );
		pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
		pGrenade->SetThrower( pOwner ); 

		pGrenade->SetGravity( BaseClass::GetGrenadeGravity() );
		pGrenade->SetFriction( BaseClass::GetGrenadeFriction() );
		pGrenade->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pGrenade->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pGrenade->m_DmgRadius = pGrenade->m_flDamage * 3.5f;
		pGrenade->ChangeTeam( pOwner->GetTeamNumber() );
		pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pGrenade->SetThink( &CGrenadeProjectile::DangerSoundThink );
		pGrenade->SetNextThink( gpGlobals->curtime );

		return pGrenade;
	}
};

LINK_ENTITY_TO_CLASS( grenade_projectile, CGrenadeProjectile );
PRECACHE_WEAPON_REGISTER( grenade_projectile );

BEGIN_DATADESC( CWeaponGrenade )
END_DATADESC()

void CWeaponGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CGrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, GRENADE_TIMER );
}
	
#endif

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponGrenade::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,					ACT_DOD_STAND_AIM_GREN_FRAG,				false },
	{ ACT_MP_CROUCH_IDLE,					ACT_DOD_CROUCH_AIM_GREN_FRAG,				false },
	{ ACT_MP_PRONE_IDLE,					ACT_DOD_PRONE_AIM_GREN_FRAG,				false },

	{ ACT_MP_RUN,							ACT_DOD_RUN_AIM_GREN_FRAG,					false },
	{ ACT_MP_WALK,							ACT_DOD_WALK_AIM_GREN_FRAG,					false },
	{ ACT_MP_CROUCHWALK,					ACT_DOD_CROUCHWALK_AIM_GREN_FRAG,			false },
	{ ACT_MP_PRONE_CRAWL,					ACT_DOD_PRONEWALK_AIM_GREN_FRAG,			false },
	{ ACT_SPRINT,							ACT_DOD_SPRINT_AIM_GREN_FRAG,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_GREN_FRAG,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_GREN_FRAG,			false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_PRONE_GREN_FRAG,		false },
};

IMPLEMENT_ACTTABLE( CWeaponGrenade );

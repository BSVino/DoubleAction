//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "sdk_basegrenade_projectile.h"

extern ConVar sv_gravity;


#ifdef CLIENT_DLL

	#include "model_types.h"

	#include "c_sdk_player.h"

#else

	#include "soundent.h"
	#include "particle_parse.h"

	#include "sdk_player.h"
	#include "sdk_gamerules.h"

#endif

#include "weapon_sdkbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#ifndef CLIENT_DLL
BEGIN_DATADESC( CBaseGrenadeProjectile )
	DEFINE_THINKFUNC( DangerSoundThink ),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( BaseGrenadeProjectile, DT_BaseGrenadeProjectile )

BEGIN_NETWORK_TABLE( CBaseGrenadeProjectile, DT_BaseGrenadeProjectile )
	#ifdef CLIENT_DLL
		RecvPropVector( RECVINFO( m_vInitialVelocity ) )
	#else
		SendPropVector( SENDINFO( m_vInitialVelocity ), 
			20,		// nbits
			0,		// flags
			-3000,	// low value
			3000	// high value
			)
	#endif
END_NETWORK_TABLE()

void CBaseGrenadeProjectile::Precache()
{
	PrecacheScriptSound( "BaseGrenade.Explode" );
	PrecacheScriptSound( "Grenade.Bounce" );
	PrecacheParticleSystem( "grenade_exp1" );
	PrecacheModel( "particle/grenadearrow.vmt" );
}

CSDKPlayer* CBaseGrenadeProjectile::GetSDKOwner()
{
	return ToSDKPlayer(GetOwnerEntity());
}

float CBaseGrenadeProjectile::GetCurrentTime()
{
	if (GetSDKOwner())
		return GetSDKOwner()->GetCurrentTime();

	return gpGlobals->curtime;
}

#ifdef CLIENT_DLL

	void CBaseGrenadeProjectile::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );

		if ( type == DATA_UPDATE_CREATED )
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

			// Add a sample 1 second back.
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

			// Add the current sample.
			vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead( changeTime, &vCurOrigin, false );
		}
	}

	CMaterialReference g_hGrenadeArrow;

	void DrawIconQuad(const CMaterialReference& m, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecUp, float flSize, const float& flAlpha);

	int CBaseGrenadeProjectile::DrawModel( int flags )
	{
		// During the first half-second of our life, don't draw ourselves if he's
		// still playing his throw animation.
		// (better yet, we could draw ourselves in his hand).
		if ( GetThrower() != C_BasePlayer::GetLocalPlayer() )
		{
			if ( GetCurrentTime() - m_flSpawnTime < 0.5 )
			{
//Tony; FIXME!
//				C_SDKPlayer *pPlayer = dynamic_cast<C_SDKPlayer*>( GetThrower() );
//				if ( pPlayer && pPlayer->m_PlayerAnimState->IsThrowingGrenade() )
//				{
//					return 0;
//				}
			}
		}

		if (!g_hGrenadeArrow.IsValid())
			g_hGrenadeArrow.Init( "particle/grenadearrow.vmt", TEXTURE_GROUP_OTHER );

		int iReturn = BaseClass::DrawModel(flags);

		if (flags & STUDIO_SSAODEPTHTEXTURE)
			return iReturn;

		C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
		if (!pLocalPlayer)
			return iReturn;

		float flAppearDistance = 500;
		float flAppearDistanceSqr = flAppearDistance*flAppearDistance;

		if ((pLocalPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() < flAppearDistanceSqr)
			m_flArrowGoalSize = 20;
		else
			m_flArrowGoalSize = 0;

		if (pLocalPlayer->GetActiveSDKWeapon() && pLocalPlayer->GetActiveSDKWeapon()->HasAimInSpeedPenalty())
			// Don't show grenades if we're aimed in.
			m_flArrowGoalSize *= 1-pLocalPlayer->m_Shared.GetAimIn();

		float flTime = C_SDKPlayer::GetLocalOrSpectatedPlayer()->GetCurrentTime() + m_flArrowSpinOffset;
		float flFrameTime = gpGlobals->frametime * C_SDKPlayer::GetLocalOrSpectatedPlayer()->GetSlowMoMultiplier();

		m_flArrowCurSize = Approach(m_flArrowGoalSize, m_flArrowCurSize, flFrameTime*100);

		if (m_flArrowCurSize == 0)
			return iReturn;

		Vector vecViewForward, vecViewRight, vecViewUp;
		pLocalPlayer->EyeVectors(&vecViewForward, &vecViewRight, &vecViewUp);

		float flSin = sin(flTime*4);
		float flCos = cos(flTime*4);

		Vector vecOrigin = GetAbsOrigin();
		Vector vecRight = vecViewRight * flSin + vecViewUp * flCos;
		Vector vecUp = vecViewRight * -flCos + vecViewUp * flSin;

		float flSize = m_flArrowCurSize;

		DrawIconQuad(g_hGrenadeArrow, vecOrigin, vecRight, vecUp, flSize, 1);

		return iReturn;
	}

	void CBaseGrenadeProjectile::Spawn()
	{
		m_flSpawnTime = GetCurrentTime();
		BaseClass::Spawn();

		m_flArrowGoalSize = 0;
		m_flArrowCurSize = 0;
		m_flArrowSpinOffset = RandomFloat(0, 10);
	}

#else

	void CBaseGrenadeProjectile::Spawn( void )
	{
		BaseClass::Spawn();

		SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		SetSolid( SOLID_BBOX );	// So it will collide with physics props!

		// smaller, cube bounding box so we rest on the ground
		SetSize( Vector ( -2, -2, -2 ), Vector ( 2, 2, 2 ) );
		SetCollisionGroup( COLLISION_GROUP_WEAPON );
		CreateVPhysics();

		//Tony; bit of a hack for the sdk, the CS grenade is really heavy for some reason.
		if ( VPhysicsGetObject() )
			VPhysicsGetObject()->SetMass( 5 );
	}

	void CBaseGrenadeProjectile::Explode( trace_t *pTrace, int bitsDamageType )
	{
		SetModelName( NULL_STRING );//invisible
		AddSolidFlags( FSOLID_NOT_SOLID );

		m_takedamage = DAMAGE_NO;

		// Pull out of the wall a bit
		if ( pTrace->fraction != 1.0 )
		{
			SetAbsOrigin( pTrace->endpos + (pTrace->plane.normal * 0.6) );
		}

		Vector vecAbsOrigin = GetAbsOrigin();

		// Since this code only runs on the server, make sure it shows the tempents it creates.
		// This solves a problem with remote detonating the pipebombs (client wasn't seeing the explosion effect)
		CDisablePredictionFiltering disabler;

		DispatchParticleEffect("grenade_exp1", vecAbsOrigin, QAngle(0, 0, 0));

		CSoundEnt::InsertSound ( SOUND_COMBAT, vecAbsOrigin, BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

		// Use the thrower's position as the reported position
		Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;
	
		CTakeDamageInfo info( this, GetThrower(), GetBlastForce(), vecAbsOrigin, m_flDamage, bitsDamageType, 0, &vecReported );

		int connected_players = 0;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			if (ToSDKPlayer(UTIL_PlayerByIndex(i)))
				connected_players++;
		}

		float radius = m_DmgRadius;
		radius = RemapValClamped(connected_players, 8, 16, radius, radius / 2);

		RadiusDamage(info, vecAbsOrigin, radius, CLASS_NONE, NULL);

		UTIL_DecalTrace( pTrace, "Scorch" );

		EmitSound( "BaseGrenade.Explode" );

		SetThink( &CBaseGrenade::SUB_Remove );
		SetTouch( NULL );
		SetSolid( SOLID_NONE );
	
		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );

		SetNextThink( gpGlobals->curtime );
	}

	void CBaseGrenadeProjectile::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
	{
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
		if ( pPhysicsObject )
		{
			pPhysicsObject->AddVelocity( &velocity, &angVelocity );
		}
	}
// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
	class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
	{
	public:
		// It does have a base, but we'll never network anything below here..
		DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );

		CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
			: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
		{
		}

		virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
		{
			if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
				return false;
			CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

			if ( pEntity )
			{
				if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
					return false;
				if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
					return true;
			}

			return false;
		}

	protected:
		const IHandleEntity *m_pPassEnt;
		int		m_collisionGroupAlreadyChecked;
		int		m_newCollisionGroup;
	};

	void CBaseGrenadeProjectile::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
	{
		if ( m_flNextBounceSound <= gpGlobals->curtime )
		{
			EmitSound( "Grenade.Bounce" );
			// default physics stuff will collide many times with the ground.. give this some randomness
			m_flNextBounceSound = gpGlobals->curtime + random->RandomFloat( 0.15f, 0.45f );
		}
		BaseClass::VPhysicsCollision( index, pEvent );
	}

	void CBaseGrenadeProjectile::VPhysicsUpdate( IPhysicsObject *pPhysics )
	{
		BaseClass::VPhysicsUpdate( pPhysics );
		Vector vel;
		AngularImpulse angVel;
		pPhysics->GetVelocity( &vel, &angVel );

		Vector start = GetAbsOrigin();
		// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
		CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
		trace_t tr;

		// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
		UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
		UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
		if ( tr.startsolid )
		{
			if ( !m_inSolid )
			{
				// UNDONE: Do a better contact solution that uses relative velocity?
				vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
				pPhysics->SetVelocity( &vel, NULL );
			}
			m_inSolid = true;
			return;
		}
		m_inSolid = false;
		if ( tr.DidHit() )
		{
			Vector dir = vel;
			VectorNormalize(dir);
			// send a tiny amount of damage so the character will react to getting bonked
			CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
			tr.m_pEnt->TakeDamage( info );

			// reflect velocity around normal
			vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;

			// absorb 80% in impact
			vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
			angVel *= -0.5f;
			pPhysics->SetVelocity( &vel, &angVel );
		}
	}
	bool CBaseGrenadeProjectile::CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_BBOX, 0, false );
		return true;
	}

	void CBaseGrenadeProjectile::DangerSoundThink( void )
	{
		if (!IsInWorld())
		{
			Remove( );
			return;
		}

		if ( GetCurrentTime() > m_flDetonateTime )
		{
			Detonate();
			return;
		}

		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, GetAbsVelocity().Length( ), 0.2 );

		SetNextThink( gpGlobals->curtime + 0.2 );

		if (GetWaterLevel() != 0)
		{
			SetAbsVelocity( GetAbsVelocity() * 0.5 );
		}
	}

	//Sets the time at which the grenade will explode
	void CBaseGrenadeProjectile::SetDetonateTimerLength( float timer )
	{
		m_flDetonateTime = GetCurrentTime() + timer;
	}

	void CBaseGrenadeProjectile::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
	{
		//Assume all surfaces have the same elasticity
		float flSurfaceElasticity = 1.0;

		//Don't bounce off of players with perfect elasticity
		if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
		{
			flSurfaceElasticity = 0.3;
		}

		// if its breakable glass and we kill it, don't bounce.
		// give some damage to the glass, and if it breaks, pass 
		// through it.
		bool breakthrough = false;

		if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
		{
			breakthrough = true;
		}

		if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
		{
			breakthrough = true;
		}

		if (breakthrough)
		{
			CTakeDamageInfo info( this, this, 10, DMG_CLUB );
			trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

			ApplyMultiDamage();

			if( trace.m_pEnt->m_iHealth <= 0 )
			{
				// slow our flight a little bit
				Vector vel = GetAbsVelocity();

				vel *= 0.4;

				SetAbsVelocity( vel );
				return;
			}
		}
		
		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
		vecAbsVelocity *= flTotalElasticity;

		// Get the total velocity (player + conveyors, etc.)
		VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
		float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

		// Stop if on ground.
		if ( trace.plane.normal.z > 0.7f )			// Floor
		{
			// Verify that we have an entity.
			CBaseEntity *pEntity = trace.m_pEnt;
			Assert( pEntity );

			SetAbsVelocity( vecAbsVelocity );

			if ( flSpeedSqr < ( 30 * 30 ) )
			{
				if ( pEntity->IsStandable() )
				{
					SetGroundEntity( pEntity );
				}

				// Reset velocities.
				SetAbsVelocity( vec3_origin );
				SetLocalAngularVelocity( vec3_angle );

				//align to the ground so we're not standing on end
				QAngle angle;
				VectorAngles( trace.plane.normal, angle );

				// rotate randomly in yaw
				angle[1] = random->RandomFloat( 0, 360 );

				// TODO: rotate around trace.plane.normal
				
				SetAbsAngles( angle );			
			}
			else
			{
				Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
				Vector vecBaseDir = GetBaseVelocity();
				VectorNormalize( vecBaseDir );
				float flScale = vecDelta.Dot( vecBaseDir );

				VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
				VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
				PhysicsPushEntity( vecVelocity, &trace );
			}
		}
		else
		{
			// If we get *too* slow, we'll stick without ever coming to rest because
			// we'll get pushed down by gravity faster than we can escape from the wall.
			if ( flSpeedSqr < ( 30 * 30 ) )
			{
				// Reset velocities.
				SetAbsVelocity( vec3_origin );
				SetLocalAngularVelocity( vec3_angle );
			}
			else
			{
				SetAbsVelocity( vecAbsVelocity );
			}
		}
		
		BounceSound();
	}

	void CBaseGrenadeProjectile::SetupInitialTransmittedGrenadeVelocity( const Vector &velocity )
	{
		m_vInitialVelocity = velocity;
	}

#endif

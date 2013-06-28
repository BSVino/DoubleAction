//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "sdk_gamerules.h"
#include "in_buttons.h"
#include "movevars_shared.h"
#include "coordsize.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sdk_clamp_back_speed( "sdk_clamp_back_speed", "0.9", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar  sdk_clamp_back_speed_min( "sdk_clamp_back_speed_min", "100", FCVAR_REPLICATED | FCVAR_CHEAT );

extern bool g_bMovementOptimizations;

class CSDKGameMovement : public CGameMovement
{
public:
	DECLARE_CLASS( CSDKGameMovement, CGameMovement );

	CSDKGameMovement();
	virtual ~CSDKGameMovement();

	void SetPlayerSpeed( void );
	virtual void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove );
	virtual bool CanAccelerate();
	virtual bool CheckJumpButton( void );
	virtual void ReduceTimers( void );
	virtual void WalkMove( void );
	virtual void CheckParameters( void );
	virtual void CheckFalling( void );
	virtual void CategorizePosition( void );

	// Ducking
	virtual void Duck( void );
	virtual void FinishUnDuck( void );
	virtual void FinishDuck( void );
	void SetSDKDuckedEyeOffset( float duckFraction );
#if defined ( SDK_USE_PRONE )
	// Prone
	void SetProneEyeOffset( float proneFraction );
	void FinishProne( void );
	void FinishUnProne( void );
	bool CanUnprone();
#endif // SDK_USE_PRONE

	virtual const Vector&	GetPlayerMins( void ) const; // uses local player
	virtual const Vector&	GetPlayerMaxs( void ) const; // uses local player

	// IGameMovement interface
	virtual const Vector&	GetPlayerMins( bool ducked ) const { return BaseClass::GetPlayerMins(ducked); }
	virtual const Vector&	GetPlayerMaxs( bool ducked ) const { return BaseClass::GetPlayerMaxs(ducked); }
	virtual const Vector&	GetPlayerViewOffset( bool ducked ) const { return BaseClass::GetPlayerViewOffset(ducked); }

	virtual unsigned int PlayerSolidMask( bool brushOnly = false );

protected:
	bool ResolveStanding( void );
	void TracePlayerBBoxWithStep( const Vector &vStart, const Vector &vEnd, unsigned int fMask, int collisionGroup, trace_t &trace );
public:
	CSDKPlayer *m_pSDKPlayer;
};


// Expose our interface.
static CSDKGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );


// ---------------------------------------------------------------------------------------- //
// CSDKGameMovement.
// ---------------------------------------------------------------------------------------- //

CSDKGameMovement::CSDKGameMovement()
{
}

CSDKGameMovement::~CSDKGameMovement()
{
}

void CSDKGameMovement::SetPlayerSpeed( void )
{
#if defined ( SDK_USE_PRONE )
	// This check is now simplified, just use CanChangePosition because it checks the two things we need to check anyway.
	if ( m_pSDKPlayer->m_Shared.IsProne() && m_pSDKPlayer->m_Shared.CanChangePosition() && m_pSDKPlayer->GetGroundEntity() != NULL )
	{
			mv->m_flClientMaxSpeed = m_pSDKPlayer->m_Shared.m_flProneSpeed;		//Base prone speed 
	}
	else	//not prone - standing or crouching and possibly moving
#endif // SDK_USE_PRONE
	{
		float stamina = 100.0f;
#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
		stamina = m_pSDKPlayer->m_Shared.GetStamina();
#endif
		if ( mv->m_nButtons & IN_DUCK )
		{
			mv->m_flClientMaxSpeed = m_pSDKPlayer->m_Shared.m_flRunSpeed;	//gets cut in fraction later
		}
		else
		{
			float flMaxSpeed;	
#if defined ( SDK_USE_SPRINTING )
			if ( ( mv->m_nButtons & IN_SPEED ) && ( stamina > 0 ) && ( mv->m_nButtons & IN_FORWARD ) )
			{
				flMaxSpeed = m_pSDKPlayer->m_Shared.m_flSprintSpeed;	//sprinting
			}
			else
#endif // SDK_USE_SPRINTING
			{
				flMaxSpeed = m_pSDKPlayer->m_Shared.m_flRunSpeed;	//jogging
			}

			mv->m_flClientMaxSpeed = flMaxSpeed - 100 + stamina;
		}
	}

#if defined ( SDK_USE_PRONE )
	if ( m_pSDKPlayer->GetGroundEntity() != NULL )
	{
		if( m_pSDKPlayer->m_Shared.IsGoingProne() )
		{
			float pronetime = m_pSDKPlayer->m_Shared.m_flGoProneTime - gpGlobals->curtime;

			//interp to prone speed
			float flProneFraction = SimpleSpline( pronetime / TIME_TO_PRONE );

			float maxSpeed = mv->m_flClientMaxSpeed;

			if ( m_pSDKPlayer->m_bUnProneToDuck )
				maxSpeed *= 0.33;
			
			mv->m_flClientMaxSpeed = ( ( 1 - flProneFraction ) * m_pSDKPlayer->m_Shared.m_flProneSpeed ) + ( flProneFraction * maxSpeed );
		}
		else if ( m_pSDKPlayer->m_Shared.IsGettingUpFromProne() )
		{
			float pronetime = m_pSDKPlayer->m_Shared.m_flUnProneTime - gpGlobals->curtime;

			//interp to regular speed speed
			float flProneFraction = SimpleSpline( pronetime / TIME_TO_PRONE );
			
			float maxSpeed = mv->m_flClientMaxSpeed;

			if ( m_pSDKPlayer->m_bUnProneToDuck )
				maxSpeed *= 0.33;

			mv->m_flClientMaxSpeed = ( flProneFraction * m_pSDKPlayer->m_Shared.m_flProneSpeed ) + ( ( 1 - flProneFraction ) * maxSpeed );
		}
	}	
#endif // SDK_USE_PRONE
}

ConVar cl_show_speed( "cl_show_speed", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "spam console with local player speed" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKGameMovement::CheckParameters( void )
{
	QAngle	v_angle;

	SetPlayerSpeed();

	if ( player->GetMoveType() != MOVETYPE_ISOMETRIC &&
		 player->GetMoveType() != MOVETYPE_NOCLIP &&
		 player->GetMoveType() != MOVETYPE_OBSERVER	)
	{
		float spd;
		float maxspeed;

		spd = ( mv->m_flForwardMove * mv->m_flForwardMove ) +
			  ( mv->m_flSideMove * mv->m_flSideMove ) +
			  ( mv->m_flUpMove * mv->m_flUpMove );

		maxspeed = mv->m_flClientMaxSpeed;
		if ( maxspeed != 0.0 )
		{
			mv->m_flMaxSpeed = min( maxspeed, mv->m_flMaxSpeed );
		}

		// Slow down by the speed factor
		float flSpeedFactor = 1.0f;
		if ( player->GetSurfaceData() )
		{
			flSpeedFactor = player->GetSurfaceData()->game.maxSpeedFactor;
		}

		// If we have a constraint, slow down because of that too.
		float flConstraintSpeedFactor = ComputeConstraintSpeedFactor();
		if (flConstraintSpeedFactor < flSpeedFactor)
			flSpeedFactor = flConstraintSpeedFactor;

		mv->m_flMaxSpeed *= flSpeedFactor;

		if ( g_bMovementOptimizations )
		{
			// Same thing but only do the sqrt if we have to.
			if ( ( spd != 0.0 ) && ( spd > mv->m_flMaxSpeed*mv->m_flMaxSpeed ) )
			{
				float fRatio = mv->m_flMaxSpeed / sqrt( spd );
				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove    *= fRatio;
				mv->m_flUpMove      *= fRatio;
			}
		}
		else
		{
			spd = sqrt( spd );
			if ( ( spd != 0.0 ) && ( spd > mv->m_flMaxSpeed ) )
			{
				float fRatio = mv->m_flMaxSpeed / spd;
				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove    *= fRatio;
				mv->m_flUpMove      *= fRatio;
			}
		}
	}


	if ( player->GetFlags() & FL_FROZEN ||
		 player->GetFlags() & FL_ONTRAIN || 
		 IsDead() )
	{
		mv->m_flForwardMove = 0;
		mv->m_flSideMove    = 0;
		mv->m_flUpMove      = 0;
	}

	DecayPunchAngle();

	// Take angles from command.
	if ( !IsDead() )
	{
		v_angle = mv->m_vecAngles;
		v_angle = v_angle + player->m_Local.m_vecPunchAngle;

		// Now adjust roll angle
		if ( player->GetMoveType() != MOVETYPE_ISOMETRIC  &&
			 player->GetMoveType() != MOVETYPE_NOCLIP )
		{
			mv->m_vecAngles[ROLL]  = CalcRoll( v_angle, mv->m_vecVelocity, sv_rollangle.GetFloat(), sv_rollspeed.GetFloat() );
		}
		else
		{
			mv->m_vecAngles[ROLL] = 0.0; // v_angle[ ROLL ];
		}
		mv->m_vecAngles[PITCH] = v_angle[PITCH];
		mv->m_vecAngles[YAW]   = v_angle[YAW];
	}
	else
	{
		mv->m_vecAngles = mv->m_vecOldAngles;
	}

	// Set dead player view_offset
	if ( IsDead() )
	{
		player->SetViewOffset( VEC_DEAD_VIEWHEIGHT );
	}

	// Adjust client view angles to match values used on server.
	if ( mv->m_vecAngles[YAW] > 180.0f )
	{
		mv->m_vecAngles[YAW] -= 360.0f;
	}

	if ( cl_show_speed.GetBool() )
	{
		Vector vel = m_pSDKPlayer->GetAbsVelocity();
		float actual_speed = sqrt( vel.x * vel.x + vel.y * vel.y );
		Msg( "player speed %.1f ( max: %f ) \n",actual_speed, mv->m_flClientMaxSpeed );
	}
}
void CSDKGameMovement::CheckFalling( void )
{
	// if we landed on the ground
	if ( player->GetGroundEntity() != NULL && !IsDead() )
	{
		if ( player->m_Local.m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHOLD )
		{
//Tony; left for playing a sound if you want.
//			CPASFilter filter( player->GetAbsOrigin() );
//			filter.UsePredictionRules();
//			player->EmitSound( filter, player->entindex(), "Player.JumpLanding" );
		}

		if ( m_pSDKPlayer->m_Shared.IsJumping() )
		{
			m_pSDKPlayer->m_Shared.SetJumping( false );
		}
	}

	BaseClass::CheckFalling();
}
void CSDKGameMovement::ProcessMovement( CBasePlayer *pBasePlayer, CMoveData *pMove )
{
	//Store the player pointer
	m_pSDKPlayer = ToSDKPlayer( pBasePlayer );
	Assert( m_pSDKPlayer );

	BaseClass::ProcessMovement( pBasePlayer, pMove );
}

bool CSDKGameMovement::CanAccelerate()
{
	// Only allow the player to accelerate when in certain states.
	SDKPlayerState curState = m_pSDKPlayer->State_Get();
	if ( curState == STATE_ACTIVE )
	{
		return player->GetWaterJumpTime() == 0;
	}
	else if ( player->IsObserver() )
	{
		return true;
	}
	else
	{	
		return false;
	}
}


void CSDKGameMovement::WalkMove( void )
{

#if defined ( SDK_USE_SPRINTING )
	float flSpeedCheck = m_pSDKPlayer->GetAbsVelocity().Length2D();

	bool bSprintButtonPressed = ( mv->m_nButtons & IN_SPEED ) > 0;

	if( bSprintButtonPressed && 
		( mv->m_nButtons & IN_FORWARD ) &&
#if defined ( SDK_USE_PRONE )
		!m_pSDKPlayer->m_Shared.IsProne() && 
#endif
		!m_pSDKPlayer->m_Shared.IsDucking() &&
		flSpeedCheck > 80 )
	{
		m_pSDKPlayer->SetSprinting( true );
	}
	else
	{
		m_pSDKPlayer->SetSprinting( false );
	}
#endif // SDK_USE_SPRINTING

	// Get the movement angles.
	Vector vecForward, vecRight, vecUp;
	AngleVectors( mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp );
	vecForward.z = 0.0f;
	vecRight.z = 0.0f;		
	VectorNormalize( vecForward );
	VectorNormalize( vecRight );

	// Copy movement amounts
	float flForwardMove = mv->m_flForwardMove;
	float flSideMove = mv->m_flSideMove;

	// Find the direction,velocity in the x,y plane.
	Vector vecWishDirection( ( ( vecForward.x * flForwardMove ) + ( vecRight.x * flSideMove ) ),
		( ( vecForward.y * flForwardMove ) + ( vecRight.y * flSideMove ) ), 
		0.0f );

	// Calculate the speed and direction of movement, then clamp the speed.
	float flWishSpeed = VectorNormalize( vecWishDirection );
	flWishSpeed = clamp( flWishSpeed, 0.0f, mv->m_flMaxSpeed );

	// Accelerate in the x,y plane.
	mv->m_vecVelocity.z = 0;
	Accelerate( vecWishDirection, flWishSpeed, sv_accelerate.GetFloat() );
	Assert( mv->m_vecVelocity.z == 0.0f );

	// Clamp the players speed in x,y.
	float flNewSpeed = VectorLength( mv->m_vecVelocity );
	if ( flNewSpeed > mv->m_flMaxSpeed )
	{
		float flScale = ( mv->m_flMaxSpeed / flNewSpeed );
		mv->m_vecVelocity.x *= flScale;
		mv->m_vecVelocity.y *= flScale;
	}

	
	// Now reduce their backwards speed to some percent of max, if they are travelling backwards unless they are under some minimum
	if ( sdk_clamp_back_speed.GetFloat() < 1.0 && VectorLength( mv->m_vecVelocity ) > sdk_clamp_back_speed_min.GetFloat() )
	{
		float flDot = DotProduct( vecForward, mv->m_vecVelocity );

		// are we moving backwards at all?
		if ( flDot < 0 )
		{
			Vector vecBackMove = vecForward * flDot;
			Vector vecRightMove = vecRight * DotProduct( vecRight, mv->m_vecVelocity );

			// clamp the back move vector if it is faster than max
			float flBackSpeed = VectorLength( vecBackMove );
			float flMaxBackSpeed = ( mv->m_flMaxSpeed * sdk_clamp_back_speed.GetFloat() );

			if ( flBackSpeed > flMaxBackSpeed )
			{
				vecBackMove *= flMaxBackSpeed / flBackSpeed;
			}

			// reassemble velocity	
			mv->m_vecVelocity = vecBackMove + vecRightMove;
		}
	}

	// Add base velocity to the player's current velocity - base velocity = velocity from conveyors, etc.
	VectorAdd( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	// Calculate the current speed and return if we are not really moving.
	float flSpeed = VectorLength( mv->m_vecVelocity );
	if ( flSpeed < 1.0f )
	{
		// I didn't remove the base velocity here since it wasn't moving us in the first place.
		mv->m_vecVelocity.Init();
		return;
	}

	// Calculate the destination.
	Vector vecDestination;
	vecDestination.x = mv->GetAbsOrigin().x + ( mv->m_vecVelocity.x * gpGlobals->frametime );
	vecDestination.y = mv->GetAbsOrigin().y + ( mv->m_vecVelocity.y * gpGlobals->frametime );	
	vecDestination.z = mv->GetAbsOrigin().z;

	// Try moving to the destination.
	trace_t trace;
	TracePlayerBBox( mv->GetAbsOrigin(), vecDestination, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	if ( trace.fraction == 1.0f )
	{
		// Made it to the destination (remove the base velocity).
		mv->SetAbsOrigin( trace.endpos );
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

		// Save the wish velocity.
		mv->m_outWishVel += ( vecWishDirection * flWishSpeed );

		// Try and keep the player on the ground.
		// NOTE YWB 7/5/07: Don't do this here, our version of CategorizePosition encompasses this test
		// StayOnGround();

		return;
	}

	// Now try and do a step move.
	StepMove( vecDestination, trace );

	// Remove base velocity.
	Vector baseVelocity = player->GetBaseVelocity();
	VectorSubtract( mv->m_vecVelocity, baseVelocity, mv->m_vecVelocity );

	// Save the wish velocity.
	mv->m_outWishVel += ( vecWishDirection * flWishSpeed );

	// Try and keep the player on the ground.
	// NOTE YWB 7/5/07: Don't do this here, our version of CategorizePosition encompasses this test
	// StayOnGround();
}

extern void TracePlayerBBoxForGround( const Vector& start, const Vector& end, const Vector& minsSrc,
									 const Vector& maxsSrc, IHandleEntity *player, unsigned int fMask,
									 int collisionGroup, trace_t& pm );

void CSDKGameMovement::CategorizePosition( void )
{
	// Observer.
	if ( player->IsObserver() )
		return;

	// Reset this each time we-recategorize, otherwise we have bogus friction when we jump into water and plunge downward really quickly
	player->m_surfaceFriction = 1.0f;

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	CheckWater();

	// If standing on a ladder we are not on ground.
	if ( player->GetMoveType() == MOVETYPE_LADDER )
	{
		SetGroundEntity( NULL );
		return;
	}

	// Check for a jump.
	if ( mv->m_vecVelocity.z > 250.0f )
	{
		SetGroundEntity( NULL );
		return;
	}

	// Calculate the start and end position.
	Vector vecStartPos = mv->GetAbsOrigin();
	Vector vecEndPos( mv->GetAbsOrigin().x, mv->GetAbsOrigin().y, ( mv->GetAbsOrigin().z - 2.0f ) );

	// NOTE YWB 7/5/07:  Since we're already doing a traceline here, we'll subsume the StayOnGround (stair debouncing) check into the main traceline we do here to see what we're standing on
	bool bUnderwater = ( player->GetWaterLevel() >= WL_Eyes );
	bool bMoveToEndPos = false;
	if ( player->GetMoveType() == MOVETYPE_WALK && 
		player->GetGroundEntity() != NULL && !bUnderwater )
	{
		// if walking and still think we're on ground, we'll extend trace down by stepsize so we don't bounce down slopes
		vecEndPos.z -= player->GetStepSize();
		bMoveToEndPos = true;
	}

	trace_t trace;
	TracePlayerBBox( vecStartPos, vecEndPos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );

	// Steep plane, not on ground.
	if ( trace.plane.normal.z < 0.7f )
	{
		// Test four sub-boxes, to see if any of them would have found shallower slope we could actually stand on.
		TracePlayerBBoxForGround( vecStartPos, vecEndPos, GetPlayerMins(), GetPlayerMaxs(), mv->m_nPlayerHandle.Get(), PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );
		if ( trace.plane.normal[2] < 0.7f )
		{
			// Too steep.
			SetGroundEntity( NULL );
			if ( ( mv->m_vecVelocity.z > 0.0f ) && 
				( player->GetMoveType() != MOVETYPE_NOCLIP ) )
			{
				player->m_surfaceFriction = 0.25f;
			}
		}
		else
		{
			SetGroundEntity( &trace );
		}
	}
	else
	{
		// YWB:  This logic block essentially lifted from StayOnGround implementation
		if ( bMoveToEndPos &&
			!trace.startsolid &&				// not sure we need this check as fraction would == 0.0f?
			trace.fraction > 0.0f &&			// must go somewhere
			trace.fraction < 1.0f ) 			// must hit something
		{
			float flDelta = fabs( mv->GetAbsOrigin().z - trace.endpos.z );
			// HACK HACK:  The real problem is that trace returning that strange value 
			//  we can't network over based on bit precision of networking origins
			if ( flDelta > 0.5f * COORD_RESOLUTION )
			{
				Vector org = mv->GetAbsOrigin();
				org.z = trace.endpos.z;
				mv->SetAbsOrigin( org );
			}
		}
		SetGroundEntity( &trace );
	}
}

inline void CSDKGameMovement::TracePlayerBBoxWithStep( const Vector &vStart, const Vector &vEnd, 
							unsigned int fMask, int collisionGroup, trace_t &trace )
{
	VPROF( "CSDKGameMovement::TracePlayerBBoxWithStep" );

	Vector vHullMin = GetPlayerMins( player->m_Local.m_bDucked );
	vHullMin.z += player->m_Local.m_flStepSize;
	Vector vHullMax = GetPlayerMaxs( player->m_Local.m_bDucked );

	Ray_t ray;
	ray.Init( vStart, vEnd, vHullMin, vHullMax );
	UTIL_TraceRay( ray, fMask, mv->m_nPlayerHandle.Get(), collisionGroup, &trace );
}

// Taken from TF2 to prevent bouncing down slopes
bool CSDKGameMovement::ResolveStanding( void )
{
	VPROF( "CSDKGameMovement::ResolveStanding" );

	//
	// Attempt to move down twice your step height.  Anything between 0.5 and 1.0
	// is a valid "stand" value.
	//

	// Matt - don't move twice your step height, only check a little bit down
	// this will keep us relatively glued to stairs without feeling too snappy
	float flMaxStepDrop = 8.0f;

	Vector vecStandPos( mv->GetAbsOrigin().x, mv->GetAbsOrigin().y, mv->GetAbsOrigin().z - ( flMaxStepDrop ) );

	trace_t trace;
	TracePlayerBBoxWithStep( mv->GetAbsOrigin(), vecStandPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );

	// Anything between 0.5 and 1.0 is a valid stand value
	if ( fabs( trace.fraction - 0.5 ) < 0.0004f )
	{
		return true;
	}

	if ( trace.fraction > 0.5f )
	{
		trace.fraction -= 0.5f;
		Vector vecNewOrigin;
		vecNewOrigin = mv->GetAbsOrigin() + trace.fraction * ( vecStandPos - mv->GetAbsOrigin() );
		mv->SetAbsOrigin( vecNewOrigin );
		return false;
	}

	// Less than 0.5 mean we need to attempt to push up the difference.
	vecStandPos.z = ( mv->GetAbsOrigin().z + ( ( 0.5f - trace.fraction ) * ( player->m_Local.m_flStepSize * 2.0f ) ) );
	TracePlayerBBoxWithStep( mv->GetAbsOrigin(), vecStandPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	
	// A fraction of 1.0 means we stood up fine - done.
	if ( trace.fraction == 1.0f )
	{
		mv->SetAbsOrigin( trace.endpos );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Recover stamina
//-----------------------------------------------------------------------------
void CSDKGameMovement::ReduceTimers( void )
{
	Vector vecPlayerVelocity = m_pSDKPlayer->GetAbsVelocity();

#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	float flStamina = m_pSDKPlayer->m_Shared.GetStamina();
#endif
	
#if defined ( SDK_USE_SPRINTING )

	float fl2DVelocitySquared = vecPlayerVelocity.x * vecPlayerVelocity.x + 
								vecPlayerVelocity.y * vecPlayerVelocity.y;	

	if ( !( mv->m_nButtons & IN_SPEED ) )
	{
		m_pSDKPlayer->m_Shared.ResetSprintPenalty();
	}

	// Can only sprint in forward direction.
	bool bSprinting = ( (mv->m_nButtons & IN_SPEED) && ( mv->m_nButtons & IN_FORWARD ) );

	// If we're holding the sprint key and also actually moving, remove some stamina
	Vector vel = m_pSDKPlayer->GetAbsVelocity();
	if ( bSprinting && fl2DVelocitySquared > 10000 ) //speed > 100
	{
		flStamina -= 20 * gpGlobals->frametime;

		m_pSDKPlayer->m_Shared.SetStamina( flStamina );
	}
	else
#endif // SDK_USE_SPRINTING

#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	{
		//gain some back		
		if ( fl2DVelocitySquared <= 0 )
		{
			flStamina += 60 * gpGlobals->frametime;
		}
		else if ( ( m_pSDKPlayer->GetFlags() & FL_ONGROUND ) && 
					( mv->m_nButtons & IN_DUCK ) &&
					( m_pSDKPlayer->GetFlags() & FL_DUCKING ) )
		{
			flStamina += 50 * gpGlobals->frametime;
		}
		else
		{
			flStamina += 10 * gpGlobals->frametime;
		}

		m_pSDKPlayer->m_Shared.SetStamina( flStamina );	
	}
#endif 
	BaseClass::ReduceTimers();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSDKGameMovement::CheckJumpButton( void )
{
	if (!m_pSDKPlayer->IsAlive())
	{
		mv->m_nOldButtons |= IN_JUMP ;	// don't jump again until released
		return false;
	}
#if defined ( SDK_USE_PRONE )
	if( m_pSDKPlayer->m_Shared.IsProne() ||
		m_pSDKPlayer->m_Shared.IsGettingUpFromProne() || 
		m_pSDKPlayer->m_Shared.IsGoingProne() )
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;
	}
#endif

	// See if we are waterjumping.  If so, decrement count and return.
	float flWaterJumpTime = player->GetWaterJumpTime();

	if ( flWaterJumpTime > 0 )
	{
		flWaterJumpTime -= gpGlobals->frametime;
		if (flWaterJumpTime < 0)
			flWaterJumpTime = 0;

		player->SetWaterJumpTime( flWaterJumpTime );
		
		return false;
	}

	// If we are in the water most of the way...
	if ( m_pSDKPlayer->GetWaterLevel() >= 2 )
	{	
		// swimming, not jumping
		SetGroundEntity( NULL );

		if(m_pSDKPlayer->GetWaterType() == CONTENTS_WATER)    // We move up a certain amount
			mv->m_vecVelocity[2] = 100;
		else if (m_pSDKPlayer->GetWaterType() == CONTENTS_SLIME)
			mv->m_vecVelocity[2] = 80;
		
		// play swiming sound
		if ( player->GetSwimSoundTime() <= 0 )
		{
			// Don't play sound again for 1 second
			player->SetSwimSoundTime( 1000 );
			PlaySwimSound();
		}

		return false;
	}

	// No more effect
 	if (m_pSDKPlayer->GetGroundEntity() == NULL)
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;		// in air, so no effect
	}

	if ( mv->m_nOldButtons & IN_JUMP )
		return false;		// don't pogo stick

	// In the air now.
	SetGroundEntity( NULL );
	
	m_pSDKPlayer->PlayStepSound( (Vector &)mv->GetAbsOrigin(), player->GetSurfaceData(), 1.0, true );
	m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_JUMP );

//Tony; liek the landing sound, leaving this here if as an example for playing a jump sound.
//	// make the jump sound
//	CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
//	filter.UsePredictionRules();
//	m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.Jump" );

	float flGroundFactor = 1.0f;
	if ( player->GetSurfaceData() )
	{
		flGroundFactor = player->GetSurfaceData()->game.jumpFactor; 
	}	


	Assert( sv_gravity.GetFloat() == 800.0f );

	float flJumpHeight = 268.3281572999747f;
	
	// Accelerate upward
	// If we are ducking...
	float startz = mv->m_vecVelocity[2];
	if ( (  m_pSDKPlayer->m_Local.m_bDucking ) || (  m_pSDKPlayer->GetFlags() & FL_DUCKING ) )
	{
		// d = 0.5 * g * t^2		- distance traveled with linear accel
		// t = sqrt(2.0 * 45 / g)	- how long to fall 45 units
		// v = g * t				- velocity at the end (just invert it to jump up that high)
		// v = g * sqrt(2.0 * 45 / g )
		// v^2 = g * g * 2.0 * 45 / g
		// v = sqrt( g * 2.0 * 45 )
							
		mv->m_vecVelocity[2] = flGroundFactor * flJumpHeight;		// flJumpSpeed of 45
	}
	else
	{
		mv->m_vecVelocity[2] += flGroundFactor * flJumpHeight;	// flJumpSpeed of 45
	}
	
	FinishGravity();

	mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.1f;

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released

	m_pSDKPlayer->m_Shared.SetJumping( true );

	return true;
}

#if defined ( SDK_USE_PRONE )
bool CSDKGameMovement::CanUnprone()
{
	int i;
	trace_t trace;
	Vector newOrigin;

	VectorCopy( mv->GetAbsOrigin(), newOrigin );

	Vector vecMins, vecMaxs;

	if ( mv->m_nButtons & IN_DUCK )
	{
		vecMins = VEC_DUCK_HULL_MIN;
		vecMaxs = VEC_DUCK_HULL_MAX;
	}
	else
	{
		vecMins = VEC_HULL_MIN;
		vecMaxs = VEC_HULL_MAX;
	}

	if ( player->GetGroundEntity() != NULL )
	{
		for ( i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( VEC_PRONE_HULL_MIN[i] - vecMins[i] );
		}
	}
	else
	{
		// If in air an letting go of crouch, make sure we can offset origin to make
		//  up for uncrouching

		Vector hullSizeNormal = vecMaxs - vecMins;
		Vector hullSizeProne = VEC_PRONE_HULL_MAX - VEC_PRONE_HULL_MIN;

		Vector viewDelta = -0.5f * ( hullSizeNormal - hullSizeProne );

		VectorAdd( newOrigin, viewDelta, newOrigin );
	}

	bool saveprone = m_pSDKPlayer->m_Shared.IsProne();
	bool saveducked = player->m_Local.m_bDucked;

	// pretend we're not prone
	m_pSDKPlayer->m_Shared.SetProne( false );
	if ( mv->m_nButtons & IN_DUCK )
		player->m_Local.m_bDucked = true;

	TracePlayerBBox( mv->GetAbsOrigin(), newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );

	// revert to reality
	m_pSDKPlayer->m_Shared.SetProne( saveprone );
	player->m_Local.m_bDucked = saveducked;

	if ( trace.startsolid || ( trace.fraction != 1.0f ) )
		return false;	

	return true;
}
#endif // SDK_USE_PRONE

//-----------------------------------------------------------------------------
// Purpose: Stop ducking
//-----------------------------------------------------------------------------
void CSDKGameMovement::FinishUnDuck( void )
{
	int i;
	trace_t trace;
	Vector newOrigin;

	VectorCopy( mv->GetAbsOrigin(), newOrigin );

	if ( player->GetGroundEntity() != NULL )
	{
		for ( i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i] );
		}
	}
	else
	{
		// If in air an letting go of crouch, make sure we can offset origin to make
		//  up for uncrouching
 		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

		Vector viewDelta = -0.5f * ( hullSizeNormal - hullSizeCrouch );

		VectorAdd( newOrigin, viewDelta, newOrigin );
	}

	player->m_Local.m_bDucked = false;
	player->RemoveFlag( FL_DUCKING );
	player->m_Local.m_bDucking  = false;
	player->SetViewOffset( GetPlayerViewOffset( false ) );
	player->m_Local.m_flDucktime = 0;
	
	mv->SetAbsOrigin( newOrigin );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CSDKGameMovement::FinishDuck( void )
{
	Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
	Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

	Vector viewDelta = 0.5f * ( hullSizeNormal - hullSizeCrouch );

	player->SetViewOffset( GetPlayerViewOffset( true ) );
	player->AddFlag( FL_DUCKING );
	player->m_Local.m_bDucking = false;

	if ( !player->m_Local.m_bDucked )
 	{
		Vector org = mv->GetAbsOrigin();

		if ( player->GetGroundEntity() != NULL )
		{
			org -= VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
		}
		else
		{
			VectorAdd( org, viewDelta, org );
		}
		mv->SetAbsOrigin( org );

		player->m_Local.m_bDucked = true;
	}

	// See if we are stuck?
	FixPlayerCrouchStuck( true );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : duckFraction - 
//-----------------------------------------------------------------------------
void CSDKGameMovement::SetSDKDuckedEyeOffset( float duckFraction )
{
	// Different from CGameMovement in that 
	Vector vDuckHullMin = GetPlayerMins( true );
	Vector vStandHullMin = GetPlayerMins( false );

	float fMore = ( vDuckHullMin.z - vStandHullMin.z );

	Vector vecStandViewOffset = GetPlayerViewOffset( false );

	Vector vecDuckViewOffset = GetPlayerViewOffset( true );
	Vector temp = player->GetViewOffset();
	temp.z = ( ( vecDuckViewOffset.z - fMore ) * duckFraction ) +
		( vecStandViewOffset.z * ( 1 - duckFraction ) );
	player->SetViewOffset( temp );
}

#if defined ( SDK_USE_PRONE )
void CSDKGameMovement::SetProneEyeOffset( float proneFraction )
{
	Vector vecPropViewOffset = VEC_PRONE_VIEW;
	Vector vecStandViewOffset = GetPlayerViewOffset( player->m_Local.m_bDucked || m_pSDKPlayer->m_bUnProneToDuck );

	Vector temp = player->GetViewOffset();
	temp.z = SimpleSplineRemapVal( proneFraction, 1.0, 0.0, vecPropViewOffset.z, vecStandViewOffset.z );

	player->SetViewOffset( temp );
}

void CSDKGameMovement::FinishUnProne( void )
{
	m_pSDKPlayer->m_Shared.m_flUnProneTime = 0.0f;
	
	SetProneEyeOffset( 0.0 );

	Vector vHullMin = GetPlayerMins( player->m_Local.m_bDucked );
	Vector vHullMax = GetPlayerMaxs( player->m_Local.m_bDucked );

	if ( m_pSDKPlayer->m_bUnProneToDuck )
	{
		FinishDuck();
	}
	else
	{
		CategorizePosition();

		if ( mv->m_nButtons & IN_DUCK && !( player->GetFlags() & FL_DUCKING ) )
		{
			// Use 1 second so super long jump will work
			player->m_Local.m_flDucktime = 1000;
			player->m_Local.m_bDucking    = true;
		}
	}
}

void CSDKGameMovement::FinishProne( void )
{	
	m_pSDKPlayer->m_Shared.SetProne( true );
	m_pSDKPlayer->m_Shared.m_flGoProneTime = 0.0f;

	FinishUnDuck();	// clear ducking

	SetProneEyeOffset( 1.0 );

	FixPlayerCrouchStuck(true);

	CategorizePosition();
}
#endif // SDK_USE_PRONE

//-----------------------------------------------------------------------------
// Purpose: See if duck button is pressed and do the appropriate things
//-----------------------------------------------------------------------------
void CSDKGameMovement::Duck( void )
{
	int buttonsChanged	= ( mv->m_nOldButtons ^ mv->m_nButtons );	// These buttons have changed this frame
	int buttonsPressed	=  buttonsChanged & mv->m_nButtons;			// The changed ones still down are "pressed"
	int buttonsReleased	=  buttonsChanged & mv->m_nOldButtons;		// The changed ones which were previously down are "released"

	if ( mv->m_nButtons & IN_DUCK )
	{
		mv->m_nOldButtons |= IN_DUCK;
	}
	else
	{
		mv->m_nOldButtons &= ~IN_DUCK;
	}

	if ( !player->IsAlive() )
	{
#if defined ( SDK_USE_PRONE )
		if( m_pSDKPlayer->m_Shared.IsProne() )
		{
			FinishUnProne();
		}
#endif // SDK_USE_PRONE

		// Unduck
		if ( player->m_Local.m_bDucking || player->m_Local.m_bDucked )
		{
			FinishUnDuck();
		}
		return;
	}

	static int iState = 0;

#if defined ( SDK_USE_PRONE )
	// Prone / UnProne - we don't duck if this is happening
	if( m_pSDKPlayer->m_Shared.IsGettingUpFromProne() == true )
	{
		float pronetime = m_pSDKPlayer->m_Shared.m_flUnProneTime - gpGlobals->curtime;

		if( pronetime < 0 )
		{
			FinishUnProne();

			if ( !m_pSDKPlayer->m_bUnProneToDuck && ( mv->m_nButtons & IN_DUCK ) )
			{
				buttonsPressed |= IN_DUCK;
				mv->m_nOldButtons &= ~IN_DUCK;
			}
		}
		else
		{
			// Calc parametric time
			float fraction = SimpleSpline( pronetime / TIME_TO_PRONE );
			SetProneEyeOffset( fraction );

		}

		// Set these, so that as soon as we stop unproning, we don't pop to standing
		// the information that we let go of the duck key has been lost by now.
		if ( m_pSDKPlayer->m_bUnProneToDuck )
		{
            player->m_Local.m_flDucktime = 1000;
			player->m_Local.m_bDucking    = true;
		}

		//don't deal with ducking while we're proning
		return;
	}
	else if ( m_pSDKPlayer->m_Shared.IsGoingProne() == true )
	{
		float pronetime = m_pSDKPlayer->m_Shared.m_flGoProneTime - gpGlobals->curtime;

		if( pronetime < 0 )
		{
			FinishProne();
		}
		else
		{
			// Calc parametric time
			float fraction = SimpleSpline( 1.0f - ( pronetime / TIME_TO_PRONE ) );
			SetProneEyeOffset( fraction );
		}

		//don't deal with ducking while we're proning
		return;
	}

	if ( gpGlobals->curtime > m_pSDKPlayer->m_Shared.m_flNextProneCheck )
	{
		if ( buttonsPressed & IN_ALT1 && m_pSDKPlayer->m_Shared.CanChangePosition() )
		{
			if( m_pSDKPlayer->m_Shared.IsProne() == false &&
				m_pSDKPlayer->m_Shared.IsGettingUpFromProne() == false )
			{
				m_pSDKPlayer->m_Shared.StartGoingProne();

				//Tony; here is where you'd want to do an animation for first person to give the effect of going prone.
				if ( m_pSDKPlayer->m_Shared.IsDucking() )
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_CROUCH_TO_PRONE );
				else
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_STAND_TO_PRONE );

			}
			else if ( CanUnprone() )
			{
				m_pSDKPlayer->m_Shared.SetProne( false );
				m_pSDKPlayer->m_Shared.StandUpFromProne();

				//
				//Tony; here is where you'd want to do an animation for first person to give the effect of getting up from prone.
				//

				m_pSDKPlayer->m_bUnProneToDuck = ( mv->m_nButtons & IN_DUCK ) > 0;

				if ( m_pSDKPlayer->m_bUnProneToDuck )
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_CROUCH );
				else
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_STAND );
			}
			
			m_pSDKPlayer->m_Shared.m_flNextProneCheck = gpGlobals->curtime + 1.0f;
			return;
		}
	}

	if ( m_pSDKPlayer->m_Shared.IsProne() &&
		m_pSDKPlayer->m_Shared.CanChangePosition() &&
		( buttonsPressed & IN_DUCK ) && 
		CanUnprone() )	
	{
		// If the player presses duck while prone,
		// unprone them to the duck position
		m_pSDKPlayer->m_Shared.SetProne( false );
		m_pSDKPlayer->m_Shared.StandUpFromProne();

		m_pSDKPlayer->m_bUnProneToDuck = true;

		//
		//Tony; here is where you'd want to do an animation for first person to give the effect of going to duck from prone.
		//

		m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_CROUCH );

		// simulate a duck that was pressed while we were prone
		player->AddFlag( FL_DUCKING );
		player->m_Local.m_bDucked = true;
		player->m_Local.m_flDucktime = 1000;
		player->m_Local.m_bDucking    = true;
	}

	// no ducking or unducking while deployed or prone
	if( m_pSDKPlayer->m_Shared.IsProne() ||
		m_pSDKPlayer->m_Shared.IsGettingUpFromProne() ||
		!m_pSDKPlayer->m_Shared.CanChangePosition() )
	{
		return;
	}
#endif // SDK_USE_PRONE
	HandleDuckingSpeedCrop();

	if ( !( player->GetFlags() & FL_DUCKING ) && ( player->m_Local.m_bDucked ) )
	{
		player->m_Local.m_bDucked = false;
	}

	// Holding duck, in process of ducking or fully ducked?
	if ( ( mv->m_nButtons & IN_DUCK ) || ( player->m_Local.m_bDucking ) || ( player->GetFlags() & FL_DUCKING ) )
	{
		if ( mv->m_nButtons & IN_DUCK )
		{
			bool alreadyDucked = ( player->GetFlags() & FL_DUCKING ) ? true : false;

			if ( (buttonsPressed & IN_DUCK ) && !( player->GetFlags() & FL_DUCKING ) )
			{
				// Use 1 second so super long jump will work
				player->m_Local.m_flDucktime = 1000;
				player->m_Local.m_bDucking    = true;
			}

			float duckmilliseconds = max( 0.0f, 1000.0f - (float)player->m_Local.m_flDucktime );
			float duckseconds = duckmilliseconds / 1000.0f;

			//time = max( 0.0, ( 1.0 - (float)player->m_Local.m_flDucktime / 1000.0 ) );
			
			if ( player->m_Local.m_bDucking )
			{
				// Finish ducking immediately if duck time is over or not on ground
				if ( ( duckseconds > TIME_TO_DUCK ) || 
					( player->GetGroundEntity() == NULL ) ||
					alreadyDucked)
				{
					FinishDuck();
				}
				else
				{
					// Calc parametric time
					float flDuckFraction = SimpleSpline( duckseconds / TIME_TO_DUCK );
					SetSDKDuckedEyeOffset( flDuckFraction );
				}
			}
		}
		else
		{
			// Try to unduck unless automovement is not allowed
			// NOTE: When not onground, you can always unduck
			if ( player->m_Local.m_bAllowAutoMovement || player->GetGroundEntity() == NULL )
			{
				if ( (buttonsReleased & IN_DUCK ) && ( player->GetFlags() & FL_DUCKING ) )
				{
					// Use 1 second so super long jump will work
					player->m_Local.m_flDucktime = 1000;
					player->m_Local.m_bDucking    = true;  // or unducking
				}

				float duckmilliseconds = max( 0.0f, 1000.0f - (float)player->m_Local.m_flDucktime );
				float duckseconds = duckmilliseconds / 1000.0f;

				if ( CanUnduck() )
				{
					if ( player->m_Local.m_bDucking || 
						 player->m_Local.m_bDucked ) // or unducking
					{
						// Finish ducking immediately if duck time is over or not on ground
						if ( ( duckseconds > TIME_TO_UNDUCK ) ||
							 ( player->GetGroundEntity() == NULL ) )
						{
							FinishUnDuck();
						}
						else
						{
							// Calc parametric time
							float duckFraction = SimpleSpline( 1.0f - ( duckseconds / TIME_TO_UNDUCK ) );
							SetSDKDuckedEyeOffset( duckFraction );
						}
					}
				}
				else
				{
					// Still under something where we can't unduck, so make sure we reset this timer so
					//  that we'll unduck once we exit the tunnel, etc.
					player->m_Local.m_flDucktime	= 1000;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: Allow bots etc to use slightly different solid masks
//-----------------------------------------------------------------------------
unsigned int CSDKGameMovement::PlayerSolidMask( bool brushOnly )
{
	int mask = 0;
#if defined ( SDK_USE_TEAMS )
	switch ( player->GetTeamNumber() )
	{
	case SDK_TEAM_BLUE:
		mask = CONTENTS_TEAM1;
		break;

	case SDK_TEAM_RED:
		mask = CONTENTS_TEAM2;
		break;
	}
#endif // SDK_USE_TEAMS
	return ( mask | BaseClass::PlayerSolidMask( brushOnly ) );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CSDKGameMovement::GetPlayerMins( void ) const
{
	if ( !player )
	{
		return vec3_origin;
	}

	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MIN;	
	}
	else
	{
		if ( player->m_Local.m_bDucked )
			return VEC_DUCK_HULL_MIN;
#if defined ( SDK_USE_PRONE )
		else if ( m_pSDKPlayer->m_Shared.IsProne() )
			return VEC_PRONE_HULL_MIN;
#endif // SDK_USE_PRONE
		else
			return VEC_HULL_MIN;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CSDKGameMovement::GetPlayerMaxs( void ) const
{	
	if ( !player )
	{
		return vec3_origin;
	}
	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MAX;	
	}
	else
	{
		if ( player->m_Local.m_bDucked )
			return VEC_DUCK_HULL_MAX;
#if defined ( SDK_USE_PRONE )
		else if ( m_pSDKPlayer->m_Shared.IsProne() )
            return VEC_PRONE_HULL_MAX;
#endif // SDK_USE_PRONE
		else
			return VEC_HULL_MAX;
	}
}


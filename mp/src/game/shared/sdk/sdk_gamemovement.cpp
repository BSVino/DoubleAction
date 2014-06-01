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
	#include "da_viewback.h"
#else
	#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sdk_clamp_back_speed( "sdk_clamp_back_speed", "0.9", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar  sdk_clamp_back_speed_min( "sdk_clamp_back_speed_min", "100", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar  sdk_dive_speed( "sdk_dive_speed", "330", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
ConVar sdk_slidetime("sdk_slidetime", "1.5", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);

#ifdef CLIENT_DLL
ConVar da_slide_unclick_time("da_slide_unclick_time", "0.25", FCVAR_ARCHIVE|FCVAR_USERINFO);
#endif

ConVar	da_d2p_stunt_forgiveness( "da_d2p_stunt_forgiveness", "0.4", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );



ConVar  da_acro_wallflip_dist ("da_acro_wallflip_dist", "8", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_wallflip_limit ("da_acro_wallflip_limit", "1", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_athletic_wallflip_limit ("da_athletic_wallflip_limit", "3", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_wallflip_speed ("da_acro_wallflip_speed", "180", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_wallflip_gain ("da_acro_wallflip_gain", "420", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

ConVar  da_acro_mantel_height ("da_acro_mantel_height", "92", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_mantel_min_height ("da_acro_mantel_min_height", "40", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_vault_height ("da_acro_vault_height", "45", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar  da_acro_roll_friction ("da_acro_roll_friction", "0.3", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

ConVar  da_physpush_base ("da_physpush_base", "0.2", FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

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
	virtual void AirAccelerate( Vector& wishdir, float wishspeed, float accel );
	virtual bool CheckJumpButton( void );
	virtual void ReduceTimers( void );
	virtual void WalkMove( void );
	virtual void PlayerMove( void );
	virtual void CheckParameters( void );
	virtual void CheckFalling( void );
	virtual void CategorizePosition( void );
	virtual void FixPlayerDiveStuck( bool upward );

	// Ducking
	virtual bool CanUnduck( void );
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

	void FinishUnSlide( void );

	virtual bool LadderMove( void );

	void SetSlideEyeOffset( float flFraction );
	void SetUnSlideEyeOffset( float flFraction );
	void SetRollEyeOffset( float flFraction );

	virtual Vector	GetPlayerMins( void ) const; // uses local player
	virtual Vector	GetPlayerMaxs( void ) const; // uses local player

	// IGameMovement interface
	virtual Vector	GetPlayerMins( bool ducked ) const { return BaseClass::GetPlayerMins(ducked); }
	virtual Vector	GetPlayerMaxs( bool ducked ) const { return BaseClass::GetPlayerMaxs(ducked); }
	virtual Vector	GetPlayerViewOffset( bool ducked ) const;

	virtual unsigned int PlayerSolidMask( bool brushOnly = false );

	void    AccumulateWallFlipTime();

	inline void TraceBBox (const Vector& start, const Vector& end, const Vector &mins, const Vector &maxs, trace_t &pm);
	virtual void FullWalkMove ();
	virtual void StepMove (Vector &vecDestination, trace_t &trace);

	bool CheckMantel();

protected:
	bool ResolveStanding( void );
	void TracePlayerBBoxWithStep( const Vector &vStart, const Vector &vEnd, unsigned int fMask, int collisionGroup, trace_t &trace );
public:
	CSDKPlayer *m_pSDKPlayer;
};

#define ROLL_TIME 0.65f
#define ROLLFINISH_TIME 0.2f
#define TIME_TO_SLIDE 0.2f
#define TIME_TO_RESLIDE 0.75f
#define SLIDE_TIME 6.0f
#define DIVE_RISE_TIME 0.4f

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
	mv->m_flClientMaxSpeed = m_pSDKPlayer->GetPlayerMaxSpeed(!!(mv->m_nButtons & IN_DUCK));

	if (m_pSDKPlayer->PlayerFrozen())
		mv->m_flClientMaxSpeed *= m_pSDKPlayer->m_flFreezeAmount;

#if defined ( SDK_USE_PRONE )
	if ( m_pSDKPlayer->GetGroundEntity() != NULL )
	{
		if( m_pSDKPlayer->m_Shared.IsGoingProne() )
		{
			float pronetime = m_pSDKPlayer->m_Shared.m_flGoProneTime - m_pSDKPlayer->GetCurrentTime();

			//interp to prone speed
			float flProneFraction = SimpleSpline( pronetime / TIME_TO_PRONE );

			float maxSpeed = mv->m_flClientMaxSpeed;

			if ( m_pSDKPlayer->m_bUnProneToDuck )
				maxSpeed *= 0.33;
			
			mv->m_flClientMaxSpeed = ( ( 1 - flProneFraction ) * m_pSDKPlayer->m_Shared.m_flProneSpeed ) + ( flProneFraction * maxSpeed );
		}
		else if ( m_pSDKPlayer->m_Shared.IsGettingUpFromProne() )
		{
			float pronetime = m_pSDKPlayer->m_Shared.m_flUnProneTime - m_pSDKPlayer->GetCurrentTime();

			//interp to regular speed speed
			float flProneFraction = SimpleSpline( pronetime / TIME_TO_PRONE );
			
			float maxSpeed = mv->m_flClientMaxSpeed;

			if ( m_pSDKPlayer->m_bUnProneToDuck )
				maxSpeed *= 0.33;

			mv->m_flClientMaxSpeed = ( flProneFraction * m_pSDKPlayer->m_Shared.m_flProneSpeed ) + ( ( 1 - flProneFraction ) * maxSpeed );
		}
	}	
#endif // SDK_USE_PRONE

	mv->m_flClientMaxSpeed = m_pSDKPlayer->m_Shared.ModifySkillValue(mv->m_flClientMaxSpeed, 0.2f, SKILL_ATHLETIC);

	Assert(IsFinite(mv->m_flClientMaxSpeed));
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

#ifdef CLIENT_DLL
	vb_data_send_float(ViewbackSystem().m_ePlayerSpeed, m_pSDKPlayer->GetAbsVelocity().Length2D());
	vb_data_send_float(ViewbackSystem().m_ePlayerMaxSpeed, mv->m_flClientMaxSpeed);
#endif

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
			CPASFilter filter( player->GetAbsOrigin() );
			filter.UsePredictionRules();
			player->EmitSound( filter, player->entindex(), "Player.JumpLanding" );
		}

		if ( m_pSDKPlayer->m_Shared.IsJumping() )
		{
			m_pSDKPlayer->m_Shared.SetJumping( false );
		}
	}

	if ( player->GetGroundEntity() != NULL && !IsDead() && player->m_Local.m_flFallVelocity >= 600 )
	{
		if ( player->GetWaterLevel() == 0 )
		{
			if (!m_pSDKPlayer->m_Shared.IsDiving() && !m_pSDKPlayer->m_Shared.IsSliding())
			{
				m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_STAND_TO_ROLL );
				m_pSDKPlayer->m_Shared.StartRolling();
				//m_pSDKPlayer->FreezePlayer(0.6f, RemapValClamped(m_pSDKPlayer->m_Local.m_flFallVelocity, 600, 1000, 0.4f, 0.8f));
			}
		}
	}

	BaseClass::CheckFalling();
}

void CSDKGameMovement::ProcessMovement( CBasePlayer *pBasePlayer, CMoveData *pMove )
{
	//Store the player pointer
	m_pSDKPlayer = ToSDKPlayer( pBasePlayer );
	Assert( m_pSDKPlayer );

	float flFrameTime = gpGlobals->frametime;

	gpGlobals->frametime *= m_pSDKPlayer->GetSlowMoMultiplier();

	BaseClass::ProcessMovement( pBasePlayer, pMove );

	gpGlobals->frametime = flFrameTime;
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

void CSDKGameMovement::AirAccelerate( Vector& wishdir, float wishspeed, float accel )
{
	float addspeed, accelspeed, currentspeed;
	float wishspd;

	if (m_pSDKPlayer->m_Shared.IsSuperFalling())
	{
		Vector vecNewVelocity = mv->m_vecVelocity.Normalized() * mv->m_vecVelocity.Length() * (1 - gpGlobals->frametime);
		vecNewVelocity.z = mv->m_vecVelocity.z;
		vecNewVelocity.x += wishdir.x * wishspeed * gpGlobals->frametime;
		vecNewVelocity.y += wishdir.y * wishspeed * gpGlobals->frametime;
		mv->m_vecVelocity = vecNewVelocity;
		return;
	}

	if (m_pSDKPlayer->m_Shared.IsDiving())
		return;
	
	if (player->pl.deadflag)
		return;
	
	if (player->GetWaterJumpTime())
		return;

	wishspd = wishspeed;
	if (wishspd > 30)
		wishspd = 30;

	// Determine veer amount
	currentspeed = mv->m_vecVelocity.Dot(wishdir);

	// See how much to add
	addspeed = wishspd - currentspeed;

	// If not adding any, done.
	if (addspeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	accelspeed = accel * wishspeed * gpGlobals->frametime * player->m_surfaceFriction;

	// Cap it
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	// Adjust pmove vel.
	mv->m_vecVelocity[0] += accelspeed * wishdir[0];
	mv->m_outWishVel[0] += accelspeed * wishdir[0];
	mv->m_vecVelocity[1] += accelspeed * wishdir[1];
	mv->m_outWishVel[1] += accelspeed * wishdir[1];
	//if (m_pSDKPlayer->m_Shared.runtime <= 0)
	{
		mv->m_vecVelocity[2] += accelspeed * wishdir[2];
		mv->m_outWishVel[2] += accelspeed * wishdir[2];
	}
}

void CSDKGameMovement::WalkMove( void )
{
	bool bSprintButtonPressed = ( mv->m_nButtons & IN_SPEED ) > 0;

	if( bSprintButtonPressed )
		m_pSDKPlayer->m_Shared.SetAimIn( true );
	else
		m_pSDKPlayer->m_Shared.SetAimIn( false );

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

	if (m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsGettingUpFromSlide())
		vecWishDirection = Vector(0,0,0);

	/*else if (m_pSDKPlayer->m_Shared.IsRolling())
	{
		if (vecWishDirection.LengthSqr() > 0)
			vecWishDirection = (m_pSDKPlayer->m_Shared.GetRollDirection() * mv->m_flMaxSpeed + vecWishDirection)/2;
		else
			vecWishDirection = m_pSDKPlayer->m_Shared.GetRollDirection() * mv->m_flMaxSpeed;
		vecWishDirection.z = 0;
	}*/

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

void CSDKGameMovement::PlayerMove (void)
{
	VPROF( "CSDKGameMovement::PlayerMove" );
	if (m_pSDKPlayer->PlayerFrozen())
	{
		mv->m_flForwardMove *= m_pSDKPlayer->m_flFreezeAmount;
		mv->m_flSideMove *= m_pSDKPlayer->m_flFreezeAmount;
		mv->m_flUpMove *= m_pSDKPlayer->m_flFreezeAmount;
		mv->m_nImpulseCommand = 0;
	}

	CheckParameters();
	
	// clear output applied velocity
	mv->m_outWishVel.Init();
	mv->m_outJumpVel.Init();

	MoveHelper( )->ResetTouchList();                    // Assume we don't touch anything
	ReduceTimers();

	AngleVectors (mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp );  // Determine movement angles

	// Always try and unstick us unless we are using a couple of the movement modes
	if ( player->GetMoveType() != MOVETYPE_NOCLIP && 
		 player->GetMoveType() != MOVETYPE_NONE && 		 
		 player->GetMoveType() != MOVETYPE_ISOMETRIC && 
		 player->GetMoveType() != MOVETYPE_OBSERVER && 
		 !player->pl.deadflag )
	{
		if ( CheckInterval( STUCK ) )
		{
			if ( CheckStuck() )
			{
				// Can't move, we're stuck
				return;  
			}
		}
	}

	// Now that we are "unstuck", see where we are (player->GetWaterLevel() and type, player->GetGroundEntity()).
	if (player->GetMoveType () != MOVETYPE_WALK || mv->m_bGameCodeMovedPlayer)
	{
		CategorizePosition();
	}
	else
	{
		if ( mv->m_vecVelocity.z > 250.0f )
		{
			SetGroundEntity( NULL );
		}
	}

	m_nOnLadder = 0;
	// Don't run ladder code if dead on on a train
	if ( !player->pl.deadflag && !(player->GetFlags() & FL_ONTRAIN) )
	{
		// If was not on a ladder now, but was on one before, 
		//  get off of the ladder
		
		// TODO: this causes lots of weirdness.
		//bool bCheckLadder = CheckInterval( LADDER );
		//if ( bCheckLadder || player->GetMoveType() == MOVETYPE_LADDER )
		{
			if ( !LadderMove() && 
				( player->GetMoveType() == MOVETYPE_LADDER ) )
			{
				// Clear ladder stuff unless player is dead or riding a train
				// It will be reset immediately again next frame if necessary
				player->SetMoveType( MOVETYPE_WALK );
				player->SetMoveCollide( MOVECOLLIDE_DEFAULT );
			}
		}
	}

	// Handle movement modes.
	switch (player->GetMoveType())
	{
		case MOVETYPE_NONE:
			break;
		case MOVETYPE_NOCLIP:
			FullNoClipMove( sv_noclipspeed.GetFloat(), sv_noclipaccelerate.GetFloat() );
			break;
		case MOVETYPE_FLY:
		case MOVETYPE_FLYGRAVITY:
			FullTossMove();
			break;
		case MOVETYPE_LADDER:
			FullLadderMove();
			break;
		case MOVETYPE_WALK:
		case MOVETYPE_ISOMETRIC:
			FullWalkMove();
			break;
		case MOVETYPE_OBSERVER:
			FullObserverMove(); // clips against world&players
			break;
		default:
			DevMsg( 1, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", player->GetMoveType(), player->IsServer());
			break;
	}
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
		if (trace.plane.normal[2] < 0.7f)
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
			SetGroundEntity (&trace);
		}
	}
	else if (m_pSDKPlayer->m_Shared.IsDiving() && m_pSDKPlayer->GetCurrentTime() < m_pSDKPlayer->m_Shared.GetDiveTime() + DIVE_RISE_TIME)
	{
		SetGroundEntity(NULL);
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

	if (m_pSDKPlayer->m_Shared.IsSliding())
		player->m_surfaceFriction *= m_pSDKPlayer->m_Shared.GetSlideFriction();
	else if (m_pSDKPlayer->m_Shared.IsProne() && m_pSDKPlayer->m_Shared.m_bProneSliding)
		player->m_surfaceFriction *= 0.2f;
	else if (m_pSDKPlayer->m_Shared.IsRolling ())
		player->m_surfaceFriction *= da_acro_roll_friction.GetFloat ();
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

	float flAimInSpeed = gpGlobals->frametime*3;

	float flAimGoal;
	if (m_pSDKPlayer->m_Shared.IsAimedIn())
		flAimGoal = 1;
	else
		flAimGoal = 0;

	if (mv->m_vecVelocity.Length2DSqr() > 10*10)
		m_pSDKPlayer->UseStyleCharge(SKILL_ATHLETIC, 2 * gpGlobals->frametime * m_pSDKPlayer->GetSlowMoMultiplier());

	m_pSDKPlayer->m_Shared.SetAimIn(Approach(flAimGoal, m_pSDKPlayer->m_Shared.GetAimIn(), flAimInSpeed));

	m_pSDKPlayer->m_Shared.RampSlowAimIn(flAimGoal);

	m_pSDKPlayer->DecayStyle();

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

#if 0
	if (m_pSDKPlayer->m_Shared.taptime > 0)
	{/*TODO: Don't use milliseconds?*/
		m_pSDKPlayer->m_Shared.taptime -= 1000*gpGlobals->frametime;
		if (m_pSDKPlayer->m_Shared.taptime <= 0)
		{
			m_pSDKPlayer->m_Shared.taptime = 0;
		}
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

	if (m_pSDKPlayer->m_Shared.IsRolling() || (m_pSDKPlayer->m_Shared.IsSliding() && m_pSDKPlayer->m_Shared.IsGettingUpFromSlide()))
		return false;

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
	
	//play end slide sound instead if we're jumping out of a slide
	if ( !m_pSDKPlayer->m_Shared.IsGettingUpFromSlide() )
		m_pSDKPlayer->PlayStepSound( (Vector &)mv->GetAbsOrigin(), player->GetSurfaceData(), 1.0, true );

	m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_JUMP );

//Tony; liek the landing sound, leaving this here if as an example for playing a jump sound.
	// make the jump sound
	CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
	filter.UsePredictionRules();
	m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.Jump" );

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

	m_pSDKPlayer->ReadyWeapon();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: See if we can get up from a crouching or rolling state
//-----------------------------------------------------------------------------
bool CSDKGameMovement::CanUnduck()
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
		// If in air and letting go of crouch, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );
		viewDelta.Negate();
		VectorAdd( newOrigin, viewDelta, newOrigin );
	}
	
	//temporarily set player bounds to represent a standing position
	m_pSDKPlayer->m_Shared.m_bIsTryingUnduck = true;
	TracePlayerBBox( mv->GetAbsOrigin(), newOrigin, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	m_pSDKPlayer->m_Shared.m_bIsTryingUnduck = false;
	if ( trace.startsolid || ( trace.fraction != 1.0f ) )
		return false;	

	return true;
}

#if defined ( SDK_USE_PRONE )
bool CSDKGameMovement::CanUnprone()
{
	if (m_pSDKPlayer->m_Shared.IsGoingProne())
		return false;

	if (m_pSDKPlayer->m_Shared.IsGettingUpFromProne())
		return false;

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
	
	//temporarily set player bounds to represent a crouching position
	m_pSDKPlayer->m_Shared.m_bIsTryingUnprone = true;

	TracePlayerBBox( mv->GetAbsOrigin(), newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );

	// revert to reality
	m_pSDKPlayer->m_Shared.m_bIsTryingUnprone = false;

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

void CSDKGameMovement::FinishUnSlide( void )
{
	m_pSDKPlayer->m_Shared.m_flUnSlideTime = 0.0f;
	
	SetUnSlideEyeOffset( 1.0 );
	m_pSDKPlayer->m_Shared.EndSlide();

	if ( m_pSDKPlayer->m_Shared.MustDuckFromSlide() )
	{
		if( CanUnprone() )
			FinishDuck();
		else
		{			
			m_pSDKPlayer->m_Shared.SetProne(true, true);
			SetProneEyeOffset( 1.0 );
		}
	}
	
	CategorizePosition();
}

bool CSDKGameMovement::LadderMove()
{
	trace_t pm;
	bool onFloor;
	Vector floor;
	Vector wishdir;
	Vector end;

	if ( player->GetMoveType() == MOVETYPE_NOCLIP )
		return false;

	if ( !GameHasLadders() )
		return false;

	// If I'm already moving on a ladder, use the previous ladder direction
	if ( player->GetMoveType() == MOVETYPE_LADDER )
	{
		wishdir = -player->m_vecLadderNormal;
	}
	else
	{
		// otherwise, use the direction player is attempting to move
		if ( mv->m_flForwardMove || mv->m_flSideMove )
		{
			for (int i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
				wishdir[i] = m_vecForward[i]*mv->m_flForwardMove + m_vecRight[i]*mv->m_flSideMove;

			VectorNormalize(wishdir);
		}
		else
		{
			// Player is not attempting to move, no ladder behavior
			return false;
		}
	}

	// wishdir points toward the ladder if any exists
	VectorMA( mv->GetAbsOrigin(), LadderDistance(), wishdir, end );
	TracePlayerBBox( mv->GetAbsOrigin(), end, LadderMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm );

	// no ladder in that direction, return
	if ( pm.fraction == 1.0f || !OnLadder( pm ) )
		return false;

	player->SetMoveType( MOVETYPE_LADDER );
	player->SetMoveCollide( MOVECOLLIDE_DEFAULT );

	player->m_vecLadderNormal = pm.plane.normal;

	// On ladder, convert movement to be relative to the ladder

	VectorCopy( mv->GetAbsOrigin(), floor );
	floor[2] += GetPlayerMins()[2] - 1;

	if( enginetrace->GetPointContents( floor ) == CONTENTS_SOLID || player->GetGroundEntity() != NULL )
	{
		onFloor = true;
	}
	else
	{
		onFloor = false;
	}

	player->SetGravity( 0 );

	float climbSpeed = ClimbSpeed();

	float forwardSpeed = 0, rightSpeed = 0;
	if ( mv->m_nButtons & IN_BACK )
		forwardSpeed -= climbSpeed;
	
	if ( mv->m_nButtons & IN_FORWARD )
		forwardSpeed += climbSpeed;
	
	if ( mv->m_nButtons & IN_MOVELEFT )
		rightSpeed -= climbSpeed;
	
	if ( mv->m_nButtons & IN_MOVERIGHT )
		rightSpeed += climbSpeed;

	if (((mv->m_nOldButtons^mv->m_nButtons)&mv->m_nButtons)&(IN_JUMP|IN_ALT1))
	{
		player->SetMoveType( MOVETYPE_WALK );
		player->SetMoveCollide( MOVECOLLIDE_DEFAULT );
		VectorScale( pm.plane.normal, 270, mv->m_vecVelocity );
	}
	else
	{
		if ( forwardSpeed != 0 || rightSpeed != 0 )
		{
			Vector velocity, perp, cross, lateral, tmp;

			//ALERT(at_console, "pev %.2f %.2f %.2f - ",
			//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
			// Calculate player's intended velocity
			//Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
			VectorScale( m_vecForward, forwardSpeed, velocity );
			VectorMA( velocity, rightSpeed, m_vecRight, velocity );

			// Perpendicular in the ladder plane
			VectorCopy( vec3_origin, tmp );
			tmp[2] = 1;
			CrossProduct( tmp, pm.plane.normal, perp );
			VectorNormalize( perp );

			// decompose velocity into ladder plane
			float normal = DotProduct( velocity, pm.plane.normal );

			// This is the velocity into the face of the ladder
			VectorScale( pm.plane.normal, normal, cross );

			// This is the player's additional velocity
			VectorSubtract( velocity, cross, lateral );

			// This turns the velocity into the face of the ladder into velocity that
			// is roughly vertically perpendicular to the face of the ladder.
			// NOTE: It IS possible to face up and move down or face down and move up
			// because the velocity is a sum of the directional velocity and the converted
			// velocity through the face of the ladder -- by design.
			CrossProduct( pm.plane.normal, perp, tmp );
			VectorMA( lateral, -normal, tmp, mv->m_vecVelocity );

			if ( onFloor && normal > 0 )	// On ground moving away from the ladder
			{
				VectorMA( mv->m_vecVelocity, MAX_CLIMB_SPEED, pm.plane.normal, mv->m_vecVelocity );
			}
			//pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
		}
		else
		{
			mv->m_vecVelocity.Init();
		}
	}
	m_pSDKPlayer->m_Shared.EndDive();
	m_pSDKPlayer->m_Shared.EndRoll();
	m_pSDKPlayer->m_Shared.EndSlide();
	m_pSDKPlayer->m_Shared.SetProne(false, true);
	m_pSDKPlayer->m_Shared.SetJumping(false);
	SetProneEyeOffset(1);
	SetRollEyeOffset(1);
	return true;
}

void CSDKGameMovement::SetSlideEyeOffset( float flFraction )
{
	Vector vecStandViewOffset = GetPlayerViewOffset( false );
	Vector vecSlideViewOffset = VEC_SLIDE_VIEW;

	Vector temp = player->GetViewOffset();

	temp.z = RemapValClamped( Bias(flFraction, 0.8f), 0.0f, 1.0f, vecStandViewOffset.z, vecSlideViewOffset.z );

	player->SetViewOffset( temp );
}

void CSDKGameMovement::SetUnSlideEyeOffset( float flFraction )
{
	Vector vecStartViewOffset = m_pSDKPlayer->m_Shared.m_vecUnSlideEyeStartOffset;
	Vector vecEndViewOffset;

	// transition to prone view if we have to
	if( !m_pSDKPlayer->m_Shared.IsGoingProne() )
		 vecEndViewOffset = GetPlayerViewOffset( m_pSDKPlayer->m_Shared.m_bMustDuckFromSlide );
	else
		vecEndViewOffset = VEC_PRONE_VIEW;

	Vector temp = player->GetViewOffset();

	temp.z = SimpleSplineRemapVal( flFraction, 0.0, 1.0, vecStartViewOffset.z, vecEndViewOffset.z );

	player->SetViewOffset( temp );
}

void CSDKGameMovement::SetRollEyeOffset( float flFraction )
{
	if (flFraction < 0.5f)
	{
		Vector vecStartViewOffset = GetPlayerViewOffset( false );
		if (m_pSDKPlayer->m_Shared.IsRolling() && m_pSDKPlayer->m_Shared.IsRollingFromDive())
			vecStartViewOffset = VEC_DIVE_VIEW;

		Vector vecSlideViewOffset = VEC_SLIDE_VIEW;

		Vector temp = player->GetViewOffset();
		flFraction = RemapVal( flFraction, 0, 0.5f, 0, 1 );

		temp.z = RemapVal( Bias(flFraction, 0.8f), 0.0f, 1.0f, vecStartViewOffset.z, vecSlideViewOffset.z );

		player->SetViewOffset( temp );
	}
	else
	{
		Vector vecSlideViewOffset = VEC_SLIDE_VIEW;
		Vector vecEndViewOffset = GetPlayerViewOffset( false );

		Vector temp = player->GetViewOffset();
		flFraction = RemapVal( flFraction, 0.5f, 1.0f, 0, 1 );

		temp.z = RemapVal( Bias(flFraction, 0.2f), 0.0f, 1.0f, vecSlideViewOffset.z, vecEndViewOffset.z );

		player->SetViewOffset( temp );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determine if diving caused player to get stuck in world
// Input  : direction to escape to (up is probably good!)
//-----------------------------------------------------------------------------
void CSDKGameMovement::FixPlayerDiveStuck( bool upward )
{
	EntityHandle_t hitent;
	int i;
	Vector test;
	trace_t dummy;

	int direction = upward ? 1 : 0;

	hitent = TestPlayerPosition( mv->GetAbsOrigin(), COLLISION_GROUP_PLAYER_MOVEMENT, dummy );
	if (hitent == INVALID_ENTITY_HANDLE )
		return;
	
	VectorCopy( mv->GetAbsOrigin(), test );	
	for ( i = 0; i < 36; i++ )
	{
		Vector org = mv->GetAbsOrigin();
		org.z += direction;
		hitent = TestPlayerPosition( org, COLLISION_GROUP_PLAYER_MOVEMENT, dummy );
		if (hitent == INVALID_ENTITY_HANDLE )
		{
			mv->SetAbsOrigin( org );
			return;
		}
	}

	// Failed
}

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
	if (m_pSDKPlayer->m_Shared.IsProne())
	{
		if (mv->m_nButtons & (IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT))
			m_pSDKPlayer->m_Shared.m_bProneSliding = false;

		if (gpGlobals->curtime - m_pSDKPlayer->m_Shared.GetDiveToProneLandTime() > da_d2p_stunt_forgiveness.GetFloat())
			m_pSDKPlayer->m_Shared.m_bProneSliding = false;
	}

	// Prone / UnProne - we don't duck if this is happening
	if( m_pSDKPlayer->m_Shared.IsGettingUpFromProne() == true )
	{
		float pronetime = m_pSDKPlayer->m_Shared.m_flUnProneTime - m_pSDKPlayer->GetCurrentTime();

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
			FinishProne();
            player->m_Local.m_flDucktime = 1000;
			player->m_Local.m_bDucking    = true;
		}

		//don't deal with ducking while we're proning
		return;
	}
	else if ( (m_pSDKPlayer->m_Shared.IsGoingProne() == true) && !m_pSDKPlayer->m_Shared.IsGettingUpFromSlide() )
	{
		float pronetime = m_pSDKPlayer->m_Shared.m_flGoProneTime - m_pSDKPlayer->GetCurrentTime();

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
	if( m_pSDKPlayer->m_Shared.IsGettingUpFromSlide() == true )
	{
		float slidetime = m_pSDKPlayer->m_Shared.m_flUnSlideTime - m_pSDKPlayer->GetCurrentTime();

		if( slidetime < 0 )
			FinishUnSlide();
		else
		{
			// Calc parametric time
			float fraction = slidetime / TIME_TO_UNSLIDE;
			SetUnSlideEyeOffset( 1-fraction );
		}

		//don't deal with ducking while we're sliding
		return;
	}
	else if( m_pSDKPlayer->m_Shared.IsSliding() )
	{
		if (!m_pSDKPlayer->m_Shared.IsGettingUpFromSlide() && !m_pSDKPlayer->m_Shared.IsAirSliding())
		{
			trace_t tr;
			Ray_t vecRay;
			vecRay.Init(mv->GetAbsOrigin(), mv->GetAbsOrigin() - Vector(0, 0, 10), Vector(0, 0, 0), Vector(0, 0, 0));
			UTIL_TraceRay(vecRay, PlayerSolidMask(), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &tr);

			if (tr.DidHitWorld())
			{
				Vector vecVelocityFlat = mv->m_vecVelocity;
				vecVelocityFlat.z = 0;
				vecVelocityFlat.NormalizeInPlace();

				// If we're sliding down a slope, add this slide time back in to pretend it never happened so we don't stop the slide as soon.
				if (tr.plane.normal.Dot(Vector(0, 0, 1)) < 0.99f && tr.plane.normal.Dot(vecVelocityFlat) > 0)
					m_pSDKPlayer->m_Shared.AddSlideTime(gpGlobals->frametime);
			}
		}

		/*if (!m_pSDKPlayer->GetGroundEntity())
		{
			// if we just left the ground
			if (!m_pSDKPlayer->m_Shared.IsAirSliding())
			{
				m_pSDKPlayer->m_Shared.PlayEndSlideSound();
				m_pSDKPlayer->m_Shared.SetAirSliding( true );
			}
		}
		else */if (m_pSDKPlayer->GetCurrentTime() - m_pSDKPlayer->m_Shared.GetSlideTime() > sdk_slidetime.GetFloat() && !m_pSDKPlayer->m_Shared.IsGettingUpFromSlide())
		{
			if(!CanUnduck())
				m_pSDKPlayer->m_Shared.m_bMustDuckFromSlide = true;
			
			// if there's no room to crouch indicate transition to prone
			if( !CanUnprone() )
				m_pSDKPlayer->m_Shared.m_flGoProneTime = m_pSDKPlayer->GetCurrentTime();
			
			m_pSDKPlayer->m_Shared.StandUpFromSlide();
			SetUnSlideEyeOffset( 0.0 );
		}
		else if (m_pSDKPlayer->m_Shared.IsAirSliding())
		{
			// if we hit the ground from the air while sliding start slide sound
			m_pSDKPlayer->m_Shared.PlayStartSlideSound();
			m_pSDKPlayer->m_Shared.SetAirSliding( false );
		}
		else if (m_pSDKPlayer->m_Shared.IsDiveSliding())
		{
			SetSlideEyeOffset(1);
		}
		else
		{
			float fraction = (m_pSDKPlayer->GetCurrentTime() - m_pSDKPlayer->m_Shared.GetSlideTime()) / TIME_TO_SLIDE;
			SetSlideEyeOffset( fraction );
		}
	}
	else if( m_pSDKPlayer->m_Shared.IsRolling() )
	{
		/*if (!m_pSDKPlayer->GetGroundEntity())
		{
			m_pSDKPlayer->m_Shared.EndRoll();
			SetRollEyeOffset( 0.0 );
		}
		else */if ( m_pSDKPlayer->GetCurrentTime() > m_pSDKPlayer->m_Shared.GetRollTime() + ROLL_TIME + ROLLFINISH_TIME )
		{
			m_pSDKPlayer->m_Shared.EndRoll();
			SetRollEyeOffset( 0.0 );

			if (!CanUnduck())
				FinishDuck();
		}
		// before we begin to stand up from the roll, let's make sure we don't want to go prone instead
		else if ( m_pSDKPlayer->GetCurrentTime() > m_pSDKPlayer->m_Shared.GetRollTime() + ROLL_TIME )
		{
			// force transition to duck if there won't be room to stand
			if ( !CanUnduck() )
			{
				SetRollEyeOffset( 0.0 );
				m_pSDKPlayer->m_Shared.EndRoll();
				FinishDuck();
			}
			else
			{
				float fraction = (m_pSDKPlayer->GetCurrentTime() - m_pSDKPlayer->m_Shared.GetRollTime()) / (ROLL_TIME + ROLLFINISH_TIME);
				SetRollEyeOffset( fraction );
			}
		}
		else
		{
			float fraction = (m_pSDKPlayer->GetCurrentTime() - m_pSDKPlayer->m_Shared.GetRollTime()) / (ROLL_TIME + ROLLFINISH_TIME);
			SetRollEyeOffset( fraction );
		}
	}
	else if( m_pSDKPlayer->m_Shared.IsDiving() )
	{
		trace_t	tr;
		Vector start, end;

		VectorCopy (mv->GetAbsOrigin (), start);
		VectorAdd (start, Vector (0, 0, -128), end);
		TraceBBox (start, end, GetPlayerMins (), GetPlayerMaxs (), tr);
		if (m_pSDKPlayer->GetGroundEntity() &&
			m_pSDKPlayer->GetCurrentTime() > m_pSDKPlayer->m_Shared.GetDiveTime() + DIVE_RISE_TIME)
		{
			m_pSDKPlayer->m_Shared.EndDive();
			m_pSDKPlayer->SetViewOffset( GetPlayerViewOffset( false ) );

			Vector vecForward, vecRight, vecUp;
			AngleVectors( mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp );
			vecForward.z = 0.0f;
			vecRight.z = 0.0f;		
			VectorNormalize( vecForward );
			VectorNormalize( vecRight );

			// Find the direction,velocity in the x,y plane.
			Vector vecWishDirection( vecForward.x*mv->m_flForwardMove + vecRight.x*mv->m_flSideMove, vecForward.y*mv->m_flForwardMove + vecRight.y*mv->m_flSideMove, 0.0f );
			vecWishDirection.NormalizeInPlace();

			Vector vecLocalVelocity = m_pSDKPlayer->GetLocalVelocity();
			vecLocalVelocity.NormalizeInPlace();

			float flWishDotLocal = vecWishDirection.Dot(vecLocalVelocity);

			bool bPlayerHoldingMoveKeys = !!(mv->m_nButtons & (IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT));

			bool bWantsProne, bWantsSlide, bWantsRoll;

			// If holding the stunt button, slide
			bWantsSlide = !!(mv->m_nButtons & IN_ALT1);
			// If holding the movement key forward, roll
			bWantsRoll = (bPlayerHoldingMoveKeys && flWishDotLocal > 0.5f) || (mv->m_nButtons & IN_DUCK);
			// Otherwise, prone
			bWantsProne = !bWantsRoll && !bWantsSlide;

			if (!CanUnprone() || bWantsProne)
			{
				m_pSDKPlayer->m_Shared.DiveLandedProne();
				SetProneEyeOffset( 1.0 );

				CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
				filter.UsePredictionRules();
				m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.DiveLand" );
			}
			else if (bWantsSlide && m_pSDKPlayer->m_Shared.CanRoll())
			{
				Vector vecAbsVelocity = m_pSDKPlayer->GetAbsVelocity();
				float flVelocity = vecAbsVelocity.Length();
				if (flVelocity >= 1e-5)
				{
					// Throw in some of the wish velocity into the slide.
					vecAbsVelocity = (vecAbsVelocity/flVelocity) + vecWishDirection;
					vecAbsVelocity.NormalizeInPlace();
					vecAbsVelocity *= flVelocity;

					m_pSDKPlayer->SetAbsVelocity(vecAbsVelocity);

					m_pSDKPlayer->m_Shared.StartSliding(true);

					float flSpeedFraction = RemapValClamped(flVelocity/sdk_dive_speed.GetFloat(), 0, 1, 0.2f, 1);

					mv->m_vecVelocity = m_pSDKPlayer->m_Shared.GetSlideDirection() * (m_pSDKPlayer->GetMaxSlideSpeed() * flSpeedFraction);
					mv->m_flClientMaxSpeed = m_pSDKPlayer->GetMaxSlideSpeed();
					mv->m_flMaxSpeed = m_pSDKPlayer->GetMaxSlideSpeed();
					player->m_surfaceFriction = m_pSDKPlayer->m_Shared.GetSlideFriction();

					SetSlideEyeOffset( 1.0 );
				}
			}
			else if (bWantsRoll && m_pSDKPlayer->m_Shared.ShouldRollAfterDiving() && m_pSDKPlayer->m_Shared.CanRoll())
			{
				m_pSDKPlayer->m_Shared.StartRolling(true);
				SetRollEyeOffset( 0.0 );
				m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_DIVE_TO_ROLL );

				CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
				filter.UsePredictionRules();
				m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.GoRoll" );
			}
			else
			{
				m_pSDKPlayer->m_Shared.SetProne(false, false);

				if (!CanUnduck())
				{
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_CROUCH );
					m_pSDKPlayer->m_bUnProneToDuck = true;

					//prepare for duck transition
					player->AddFlag( FL_DUCKING );
					player->m_Local.m_bDucked = true;
				}
				else
				{
					m_pSDKPlayer->m_Shared.StandUpFromProne();
					m_pSDKPlayer->m_bUnProneToDuck = false;
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_STAND );
				}

				CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
				filter.UsePredictionRules();
				m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.DiveLand" );
			}
		}
		//hey guys are we stuck
		FixPlayerDiveStuck( true );
	}

	if ( m_pSDKPlayer->m_Shared.CanChangePosition() )
	{
		bool bStunt = !!(buttonsPressed & IN_ALT1);

		bool bGoProne = false;
		bool bGetUp = !!(buttonsPressed & (IN_ALT1|IN_JUMP));
		bool bGetUpFromProne = (m_pSDKPlayer->GetCurrentTime() > m_pSDKPlayer->m_Shared.m_flDisallowUnProneTime) && (bGetUp || !!(mv->m_nButtons & (IN_BACK|IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT)));

		if (m_pSDKPlayer->m_Shared.IsSliding() && (buttonsReleased & IN_ALT1) && gpGlobals->curtime - m_pSDKPlayer->m_Shared.GetSlideTime() > m_pSDKPlayer->GetUserInfoFloat("da_slide_unclick_time", 0.25f))
			bGetUp = true;

		bool bSlide = false;
		bool bRoll = false;
		bool bFromDive = false;
		bool bDive = false;	

		if (buttonsPressed & (IN_DUCK))
		{
			if (m_pSDKPlayer->m_Shared.GetLastDuckPress() < 0)
				m_pSDKPlayer->m_Shared.SetDuckPress();
			else
			{
				float flTime = gpGlobals->curtime - m_pSDKPlayer->m_Shared.GetLastDuckPress();
				if (flTime < 0.4f)
				{
					bRoll = true;
					m_pSDKPlayer->m_Shared.SetDuckPress(true);
				}
				else
					m_pSDKPlayer->m_Shared.SetDuckPress();
			}
		}

		if (bStunt)
		{
			bSlide = (m_pSDKPlayer->GetAbsVelocity().Length() > 10.0f) && (mv->m_nButtons & (IN_BACK|IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT)) &&
				(m_pSDKPlayer->GetFlags() & FL_ONGROUND) && (mv->m_nButtons & IN_DUCK) &&
				(m_pSDKPlayer->m_Shared.GetSlideTime() < (m_pSDKPlayer->GetCurrentTime() - TIME_TO_RESLIDE));

			bDive = (m_pSDKPlayer->GetAbsVelocity().Length() > 10.0f);
		}

		if (!bRoll && m_pSDKPlayer->m_Shared.IsProne())
		{
			if (buttonsPressed&(IN_BACK|IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT|IN_ALT1))
			{
				if (m_pSDKPlayer->m_Shared.GetDiveToProneLandTime() > 0 && m_pSDKPlayer->GetCurrentTime() - m_pSDKPlayer->m_Shared.GetDiveToProneLandTime() < da_d2p_stunt_forgiveness.GetFloat())
				{
					// If the player presses a stunt button just after she lands from a dive,
					// give her the benefit of the doubt and transition to a roll as if the
					// button was down the whole time.
					Vector vecForward, vecRight, vecUp;
					AngleVectors( mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp );
					vecForward.z = 0.0f;
					vecRight.z = 0.0f;		
					VectorNormalize( vecForward );
					VectorNormalize( vecRight );

					Vector vecWishDirection( vecForward.x*mv->m_flForwardMove + vecRight.x*mv->m_flSideMove, vecForward.y*mv->m_flForwardMove + vecRight.y*mv->m_flSideMove, 0.0f );
					vecWishDirection.NormalizeInPlace();

					Vector vecLocalVelocity = m_pSDKPlayer->GetLocalVelocity();
					vecLocalVelocity.NormalizeInPlace();

					float flWishDotLocal = vecWishDirection.Dot(vecLocalVelocity);

					// Only if the direction of the movement keys is the same as the velocity.
					if (flWishDotLocal > 0.5f)
					{
						bRoll = true;
						bFromDive = true;
					}
				}
			}
		}

		if( bGoProne && m_pSDKPlayer->m_Shared.IsProne() == false && m_pSDKPlayer->m_Shared.IsGettingUpFromProne() == false &&
			!m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsRolling() )
		{
			m_pSDKPlayer->m_Shared.StartGoingProne();

			//Tony; here is where you'd want to do an animation for first person to give the effect of going prone.
			if ( m_pSDKPlayer->m_Shared.IsDucking() )
				m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_CROUCH_TO_PRONE );
			else
				m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_STAND_TO_PRONE );

			return;
		}
		else if ( bGetUpFromProne && m_pSDKPlayer->m_Shared.IsProne() && CanUnprone() )
		{
			m_pSDKPlayer->m_Shared.SetProne( false );
			m_pSDKPlayer->m_Shared.StandUpFromProne();

			//
			//Tony; here is where you'd want to do an animation for first person to give the effect of getting up from prone.
			//

			m_pSDKPlayer->m_bUnProneToDuck = !CanUnduck() || (( mv->m_nButtons & IN_DUCK ) > 0);

			if ( m_pSDKPlayer->m_bUnProneToDuck )
			{
				//m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_CROUCH );

				//prepare for duck transition
				player->AddFlag( FL_DUCKING );
				player->m_Local.m_bDucked = true;				
			}
			else
				m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONE_TO_STAND );

			return;
		}
		else if (bGetUp && m_pSDKPlayer->m_Shared.IsSliding() )
		{
			bool bStandingRoom = CanUnduck();
			bool bCanJumpUp = ( bStandingRoom && !m_pSDKPlayer->m_Shared.IsDiveSliding() );
			
			//if there's standing room and we're doing a standard slide we can chain into a jump
			if ( (mv->m_nButtons & IN_JUMP) > 0 )
			{
				//otherwise eat the jump input
				if ( !bCanJumpUp )
					mv->m_nOldButtons |= IN_JUMP;
			}
			else
				//if we aren't attempting to jump stand up normally
				bCanJumpUp = false;

			if ( !bStandingRoom )
			{
				if ( CanUnprone() )
					m_pSDKPlayer->m_Shared.m_bMustDuckFromSlide = true;
				else
					// indicate transition to prone
					m_pSDKPlayer->m_Shared.m_flGoProneTime = m_pSDKPlayer->GetCurrentTime();
			}

			m_pSDKPlayer->m_Shared.StandUpFromSlide( bCanJumpUp );
			SetUnSlideEyeOffset( 0.0 );

			return;
		}
		else if( bRoll && m_pSDKPlayer->m_Shared.CanRoll() )
		{
			m_pSDKPlayer->m_Shared.SetProne( false );
			m_pSDKPlayer->m_Shared.StartRolling(bFromDive);

			SetRollEyeOffset( 0.0 );

			m_pSDKPlayer->DoAnimationEvent( bFromDive?PLAYERANIMEVENT_DIVE_TO_ROLL:PLAYERANIMEVENT_STAND_TO_ROLL );

			CPASFilter filter( m_pSDKPlayer->GetAbsOrigin() );
			filter.UsePredictionRules();
			m_pSDKPlayer->EmitSound( filter, m_pSDKPlayer->entindex(), "Player.GoRoll" );
		}
		else if( bSlide && m_pSDKPlayer->m_Shared.CanSlide() )
		{
			m_pSDKPlayer->m_Shared.SetProne( false );

			Assert (m_pSDKPlayer->m_Shared.m_flRunSpeed);
			if (fabs (m_pSDKPlayer->m_Shared.m_flRunSpeed) > 1e-5)
			{
				float flSpeedFraction = RemapValClamped(m_pSDKPlayer->GetAbsVelocity().Length()/m_pSDKPlayer->m_Shared.m_flRunSpeed, 0, 1, 0.2f, 1);

				if (bFromDive)
				{
					flSpeedFraction = RemapValClamped(m_pSDKPlayer->GetAbsVelocity().Length()/sdk_dive_speed.GetFloat(), 0, 1, 0.2f, 1);

					// Give the player the benefit of the doubt that he probably just lost some velocity while not sliding.
					if (flSpeedFraction < 0.8f)
						flSpeedFraction = 0.8f;
				}

				m_pSDKPlayer->m_Shared.StartSliding(bFromDive);

				mv->m_vecVelocity = m_pSDKPlayer->m_Shared.GetSlideDirection() * (m_pSDKPlayer->GetMaxSlideSpeed() * flSpeedFraction);
				mv->m_flClientMaxSpeed = m_pSDKPlayer->GetMaxSlideSpeed();
				mv->m_flMaxSpeed = m_pSDKPlayer->GetMaxSlideSpeed();
				player->m_surfaceFriction = m_pSDKPlayer->m_Shared.GetSlideFriction();

				SetSlideEyeOffset( bFromDive?1.0:0.0 );

				if (!bFromDive)
					m_pSDKPlayer->DoAnimationEvent( PLAYERANIMEVENT_STAND_TO_SLIDE );
			}
		}
	}

	if ( m_pSDKPlayer->m_Shared.IsProne() &&
		m_pSDKPlayer->m_Shared.CanChangePosition() &&
		( buttonsPressed & IN_DUCK ) && 
		CanUnprone() )	
	{
		m_pSDKPlayer->m_Shared.SetProne( false );
		player->AddFlag( FL_DUCKING );
		player->m_Local.m_flDucktime = 1000;
		player->m_Local.m_bDucking    = true;
	}

#endif // SDK_USE_PRONE

	if (m_pSDKPlayer->m_Shared.IsRolling())
		return;

	if (m_pSDKPlayer->m_Shared.IsDiving())
		return;

	if (m_pSDKPlayer->m_Shared.IsSliding())
		return;

	if (m_pSDKPlayer->m_Shared.IsWallFlipping())
		return;

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
Vector CSDKGameMovement::GetPlayerMins( void ) const
{
	if ( !player )
	{
		return vec3_origin;
	}

	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MIN;	
	}
	// see if we're attempting a collision test
	else if ( m_pSDKPlayer->m_Shared.m_bIsTryingUnduck )
		return VEC_HULL_MIN;
	else if ( m_pSDKPlayer->m_Shared.m_bIsTryingUnprone )
		return VEC_DUCK_HULL_MIN;
	else
	{
		if ( player->m_Local.m_bDucked )
			return VEC_DUCK_HULL_MIN;
		else if ( m_pSDKPlayer->m_Shared.IsDiving() )
			return VEC_DIVE_HULL_MIN;
		else if (m_pSDKPlayer->m_Shared.IsWallFlipping())
			return VEC_HULL_MIN;
#if defined ( SDK_USE_PRONE )
		else if ( m_pSDKPlayer->m_Shared.IsProne() )
			return VEC_PRONE_HULL_MIN;
#endif // SDK_USE_PRONE
		else if ( m_pSDKPlayer->m_Shared.IsSliding() )
			return VEC_SLIDE_HULL_MIN;
		else if ( m_pSDKPlayer->m_Shared.IsRolling() )
			return VEC_DUCK_HULL_MIN;
		else
			return VEC_HULL_MIN;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
Vector CSDKGameMovement::GetPlayerMaxs( void ) const
{	
	if ( !player )
	{
		return vec3_origin;
	}
	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MAX;	
	}
	// see if we're attempting a collision test
	else if ( m_pSDKPlayer->m_Shared.m_bIsTryingUnduck )
		return VEC_HULL_MAX;
	else if ( m_pSDKPlayer->m_Shared.m_bIsTryingUnprone )
		return VEC_DUCK_HULL_MAX;
	else
	{
		if ( player->m_Local.m_bDucked )
			return VEC_DUCK_HULL_MAX;
		else if ( m_pSDKPlayer->m_Shared.IsDiving() )
			return VEC_DIVE_HULL_MAX;
		else if (m_pSDKPlayer->m_Shared.IsWallFlipping())
			return VEC_HULL_MAX;
#if defined ( SDK_USE_PRONE )
		else if ( m_pSDKPlayer->m_Shared.IsProne() )
            return VEC_PRONE_HULL_MAX;
#endif // SDK_USE_PRONE
		else if ( m_pSDKPlayer->m_Shared.IsSliding() )
			return VEC_SLIDE_HULL_MAX;
		else if ( m_pSDKPlayer->m_Shared.IsRolling() )
			return VEC_DUCK_HULL_MAX;
		else
			return VEC_HULL_MAX;
	}
}

Vector CSDKGameMovement::GetPlayerViewOffset( bool ducked ) const
{
	return BaseClass::GetPlayerViewOffset(ducked);
}

inline void CSDKGameMovement::TraceBBox (const Vector& start, const Vector& end, const Vector &mins, const Vector &maxs, trace_t &pm)
{
	Ray_t ray;
	ray.Init (start, end, mins, maxs);
	UTIL_TraceRay (ray, PlayerSolidMask (), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &pm);
}

bool CSDKGameMovement::CheckMantel()
{
	trace_t tr;
	Vector vecMins, vecMaxs;
	Vector pr1, pr2;
	Vector dir;
	float ofs;

	if (m_pSDKPlayer->m_Shared.IsManteling())
		return true;

	if (!(mv->m_nButtons&IN_JUMP))
		return false;

	vecMins = GetPlayerMins();
	vecMaxs = GetPlayerMaxs();

	dir[0] = m_vecForward[0];
	dir[1] = m_vecForward[1];
	dir[2] = 0;

	ofs = 0.5*vecMaxs[2];

	/*Probe up for free space*/
	pr1 = pr2 = mv->GetAbsOrigin();
	pr2[2] += da_acro_mantel_height.GetFloat ();
	TraceBBox (pr1, pr2, vecMins, vecMaxs, tr);

	if (tr.fraction < 1 || tr.allsolid || tr.startsolid)
		/*Blocked by ceiling*/
		return false;

	/*Then probe forward for free space*/
	pr1 = pr2;
	pr2 = pr1 + dir*4;
	TraceBBox (pr1, pr2, vecMins, vecMaxs, tr);
	if (tr.fraction < 1)
		/*Not enough space to stand on for potential ledge, or too far down wall*/
		return false;

	/*Find ground to stand on*/
	pr1 = pr2;
	pr2[2] -= da_acro_mantel_height.GetFloat ();
	TraceBBox (pr1, pr2, vecMins, vecMaxs, tr);
	if (tr.fraction == 1)
		return false;

	if (tr.endpos.z - mv->GetAbsOrigin().z < da_acro_mantel_min_height.GetFloat())
		return false;

	if (tr.m_pEnt && tr.m_pEnt->IsPlayer())
		return false;

	pr1 = mv->GetAbsOrigin();
	pr2 = pr1 + dir*4;
	TraceBBox (pr1, pr2, vecMins, vecMaxs, tr);

	// Only mantel with steep planes.
	if (tr.plane.normal.z > 0.3)
		return false;

	if (tr.plane.normal.z < 0)
		return false;

	m_pSDKPlayer->m_Shared.StartManteling(tr.plane.normal);
	m_pSDKPlayer->SetViewOffset( GetPlayerViewOffset( false ) );

	return true;
}

void CSDKGameMovement::AccumulateWallFlipTime()
{
	if (!m_pSDKPlayer->m_Shared.IsWallFlipping())
		return;

	if (m_pSDKPlayer->GetCurrentTime() < m_pSDKPlayer->m_Shared.GetWallFlipEndTime())
		return;

	trace_t traceresult;

	//temporarily set player bounds to represent a standing position
	m_pSDKPlayer->m_Shared.m_bIsTryingUnduck = true;
	bool bBlocked = (TestPlayerPosition( mv->GetAbsOrigin(), COLLISION_GROUP_PLAYER_MOVEMENT, traceresult ) != INVALID_ENTITY_HANDLE);
	m_pSDKPlayer->m_Shared.m_bIsTryingUnduck = false;
	if ( bBlocked )
		return;

	m_pSDKPlayer->m_Shared.EndWallFlip();
}

static ConVar da_terminal_velocity("da_terminal_velocity", "1600", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar da_terminal_velocity_delta("da_terminal_velocity_delta", "800", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CSDKGameMovement::FullWalkMove ()
{
	m_nOldWaterLevel = player->GetWaterLevel();
	if (!CheckWater ()) 
	{
		StartGravity();
	}

	if (m_pSDKPlayer->GetFlags() & FL_ONGROUND)
		m_pSDKPlayer->m_Shared.PlayerOnGround();

#if 0
	if (m_pSDKPlayer->m_Shared.runtime < 0)
	{
		if (!(m_pSDKPlayer->m_Shared.daflags&DA_WRLOCK))
		{
			m_pSDKPlayer->m_Shared.runtime += 1000*gpGlobals->frametime;
			if (m_pSDKPlayer->m_Shared.runtime > 0)
			{
				m_pSDKPlayer->m_Shared.runtime = 0;
			}
		}
	}
#endif

	if (m_pSDKPlayer->m_Shared.IsWallFlipping())
	{
		mv->m_flForwardMove = 0;
		mv->m_flSideMove = 0;

		AccumulateWallFlipTime();
	}

	if (player->GetWaterJumpTime())
	{// If we are leaping out of the water, just update the counters.
		WaterJump();
		TryPlayerMove();
		CheckWater();
		return;
	}
	if ( player->GetWaterLevel() >= WL_Waist ) 
	{/*Do swimming controls*/
		if ( player->GetWaterLevel() == WL_Waist )
		{
			CheckWaterJump();
		}
		if (mv->m_vecVelocity[2] < 0 && player->GetWaterJumpTime())
		{//If we are falling again, then we must not be trying to jump out of water any more.
			player->SetWaterJumpTime(0);
		}
		if (mv->m_nButtons & IN_JUMP)
		{
			CheckJumpButton();
		}
		else
		{
			mv->m_nOldButtons &= ~IN_JUMP;
		}
		WaterMove();
		CategorizePosition();
		if ( player->GetGroundEntity() != NULL )
		{
			mv->m_vecVelocity[2] = 0;			
		}
		return;
	}

	UpdateDuckJumpEyeOffset();
	Duck();

	if (m_pSDKPlayer->m_Shared.CanChangePosition ())
	{
		int pressed = (mv->m_nOldButtons^mv->m_nButtons)&mv->m_nButtons;
		if (pressed&IN_ALT1)
		{
			int iWallflips = da_acro_wallflip_limit.GetInt();
			if (m_pSDKPlayer->m_Shared.m_iStyleSkill == SKILL_ATHLETIC)
				iWallflips = da_athletic_wallflip_limit.GetInt();
			if (m_pSDKPlayer->IsStyleSkillActive(SKILL_ATHLETIC))
				iWallflips = 9999;

			if (!player->m_Local.m_bDucked && !m_pSDKPlayer->m_Shared.IsProne() && m_pSDKPlayer->m_Shared.GetWallFlipCount() < iWallflips)
			{
				trace_t tr;
				Vector org, mins, maxs;
				Vector dir;
				float dist;

				dist = da_acro_wallflip_dist.GetFloat ();
				VectorCopy (mv->GetAbsOrigin (), org);
				VectorCopy (GetPlayerMins (), mins);
				VectorCopy (GetPlayerMaxs (), maxs);
				mins[2] += player->m_Local.m_flStepSize + 1e-3;
				dir[0] = m_vecForward[0];
				dir[1] = m_vecForward[1];
				dir[2] = 0;
				TraceBBox (org, org + dist*dir, mins, maxs, tr);

				Vector vecForward;
				AngleVectors(mv->m_vecAbsViewAngles, &vecForward);

				// Don't flip if the surface is skybox.
				// Don't flip if the surface isn't a wall.
				// Don't flip if the player isn't at least sorta facing the wall.
				// Don't flip if the player is sliding or getting up from sliding.
				if (tr.fraction < 1 && !(tr.surface.flags&SURF_SKY) && fabs(tr.plane.normal[2]) < 0.7 && vecForward.Dot(tr.plane.normal) < -0.7f
					&& !m_pSDKPlayer->m_Shared.IsSliding() && !m_pSDKPlayer->m_Shared.IsGettingUpFromProne() && !m_pSDKPlayer->m_Shared.IsGettingUpFromSlide() )
				{
					m_pSDKPlayer->m_Shared.EndDive();
					m_pSDKPlayer->SetViewOffset( GetPlayerViewOffset( false ) );

					float speed = da_acro_wallflip_speed.GetFloat ();
					mv->m_vecVelocity[0] = speed*tr.plane.normal[0];
					mv->m_vecVelocity[1] = speed*tr.plane.normal[1];
					mv->m_vecVelocity[2] = da_acro_wallflip_gain.GetFloat ();
					SetGroundEntity (NULL);
					FinishGravity ();

					CPASFilter filter (org);
					filter.UsePredictionRules ();
					m_pSDKPlayer->EmitSound (filter, m_pSDKPlayer->entindex (), "Player.GoDive");

					m_pSDKPlayer->DoAnimationEvent (PLAYERANIMEVENT_WALLFLIP);
					m_pSDKPlayer->m_Shared.StartWallFlip(tr.plane.normal);
				}
			}
			if (m_pSDKPlayer->GetAbsVelocity().Length() > 10.0f &&
				!m_pSDKPlayer->m_Shared.IsWallFlipping() &&
				m_pSDKPlayer->m_Shared.CanDive ())
			{
#if 0
				if (m_pSDKPlayer->m_Shared.runtime > 0)
				{/*Project speed onto forward axis before dive, 
					else dive gets affected by the run velocity*/
					Vector velo;
					float speed;

					speed = player->GetAbsVelocity ().Length ();
					VectorScale (m_vecForward, speed, velo);
					player->SetAbsVelocity (velo);
					m_pSDKPlayer->m_Shared.runtime = -1;
				}
#endif
				mv->m_vecVelocity = m_pSDKPlayer->m_Shared.StartDiving();

				FinishGravity();
			}
		}
	}

#if 0
	if (checkrun ())
	{/*Wallrunning*/
		trace_t tr;
		Vector right;
		Vector mins, maxs, org;
		Vector dir;

		VectorCopy (GetPlayerMins (), mins);
		VectorCopy (GetPlayerMaxs (), maxs);
		VectorCopy (mv->GetAbsOrigin (), org);
		VectorCopy (m_pSDKPlayer->m_Shared.rundir, dir);
		TraceBBox (org, org + dir, mins, maxs, tr);
		if (tr.fraction == 1 || fabs (DotProduct (tr.plane.normal, dir)) < 1 - 1e-4)
		{/*Must have no obstructions*/
			int released;
			released = (mv->m_nOldButtons^mv->m_nButtons)&mv->m_nOldButtons;
			VectorScale (m_pSDKPlayer->m_Shared.wallnormal, 8, right);
			TraceBBox (org, org - right, mins, maxs, tr);
			if (tr.fraction >= 1 - 1e-2 || tr.surface.flags&SURF_SKY)
			{/*Wall no longer at side*/
				m_pSDKPlayer->m_Shared.runtime = -1;
			}
			else if (released&IN_JUMP)
			{/*Drop out of run if space is let go*/
				m_pSDKPlayer->m_Shared.runtime = -1;
			}
			else
			{/*Run right up along the wall*/
				Vector forward, up, run;
				float d, t, a, s;

				forward = m_pSDKPlayer->m_Shared.rundir;
				d = da_acro_wallrun_duration.GetFloat ();
				t = m_pSDKPlayer->m_Shared.runtime/d;
				a = da_acro_wallrun_thrust.GetFloat ();
				mv->m_vecVelocity[0] = a*forward[0];
				mv->m_vecVelocity[1] = a*forward[1];
				mv->m_vecVelocity[2] = (t*t/d*d)*a*forward[2];
				mv->m_flForwardMove = 0;
				mv->m_flSideMove = 0;
				mv->m_flUpMove = 0;
				SetGroundEntity (NULL);
				FinishGravity ();
				/*Resave wall normal since it may change*/
				Vector &n = m_pSDKPlayer->m_Shared.wallnormal.GetForModify ();
				VectorCopy (tr.plane.normal, n);
				/*Recompute run direction in case normal has changed*/
				up[0] = 0;
				up[1] = 0;
				up[2] = 1; /*This is approximate*/
				CrossProduct (tr.plane.normal, up, run);
				s = (DotProduct (forward, run) > 0) ? 1 : -1;
				Vector &nr = m_pSDKPlayer->m_Shared.rundir.GetForModify ();
				nr[0] = s*run[0];
				nr[1] = s*run[1];
				nr[2] = nr[2];

				m_pSDKPlayer->m_Shared.runtime -= 1000*gpGlobals->frametime;
				if (m_pSDKPlayer->m_Shared.runtime < 0)
				{/*Don't wallrun again until we've landed*/
					m_pSDKPlayer->m_Shared.runtime = -1;
				}
			}
		}
		else
		{
			m_pSDKPlayer->m_Shared.runtime = -1;
		}
	}
#endif
	if (CheckMantel())
	{
		/*Move up the height of the bbox over the time of the animation
		TODO: How get animation duration? Make this use the crouch bbox.*/
		trace_t tr;
		Vector mins, maxs;
		Vector pr1, pr2;
		const float sqt = 0.5;
		const float ph = 72;
		const float a = ph/sqt;

		if (!(mv->m_nButtons&IN_JUMP))
		{
			m_pSDKPlayer->m_Shared.ResetManteling();
			return;
		}
		VectorCopy (GetPlayerMins (), mins);
		VectorCopy (GetPlayerMaxs (), maxs);
		mv->m_flForwardMove = 0;
		mv->m_flSideMove = 0;
		mv->m_flUpMove = 0;

		VectorCopy (mv->GetAbsOrigin (), pr1);
		VectorCopy (pr1, pr2);
		pr2[2] += gpGlobals->frametime*a;

		TraceBBox (pr1, pr2, mins, maxs, tr);
		if (tr.fraction == 1 && !tr.startsolid && !tr.allsolid)
		{
			mv->SetAbsOrigin (tr.endpos);
			if (m_pSDKPlayer->m_Shared.IsManteling())
			{
				// Try to push the player onto the ledge.
				Vector vecOrigin = mv->GetAbsOrigin();
				Vector vecTarget = vecOrigin + Vector(m_vecForward.x, m_vecForward.y, 0).Normalized();

				TraceBBox(vecOrigin, vecTarget, mins, maxs, tr);

				if (!tr.DidHit ())
				{
					mv->SetAbsOrigin (tr.endpos);
					m_pSDKPlayer->m_Shared.ResetManteling();
					m_pSDKPlayer->Instructor_LessonLearned("mantel");
				}
				else
					SetGroundEntity(&tr);
			}
		}
		else
			m_pSDKPlayer->m_Shared.ResetManteling();

		return;
	}

	if (m_pSDKPlayer->m_Shared.IsDiving() && !m_pSDKPlayer->GetGroundEntity ())
	{/*Raise player off the ground*/
		if (m_pSDKPlayer->m_Shared.GetDiveLerped() < 1)
		{
			trace_t trace;
			float flDiveLerpTime = DIVE_RISE_TIME;
			float flBiasAmount = 0.7f;
			float flCurrentLerp = RemapValClamped(m_pSDKPlayer->GetCurrentTime(), m_pSDKPlayer->m_Shared.GetDiveTime(), m_pSDKPlayer->m_Shared.GetDiveTime() + flDiveLerpTime, 0, 1);
			float flCurrentBias = Bias(flCurrentLerp, flBiasAmount);
			float flLastTimeBias = Bias(m_pSDKPlayer->m_Shared.GetDiveLerped(), flBiasAmount);
			float flHullHeightNormal = VEC_HULL_MAX.z - VEC_HULL_MIN.z;
			float flHullHeightDive = VEC_DIVE_HULL_MAX.z - VEC_DIVE_HULL_MIN.z;
			float flMoveUpHeight = flHullHeightNormal-flHullHeightDive;
			float flCurrentHeight = flMoveUpHeight*flCurrentBias;
			float flLastTimeHeight = flMoveUpHeight*flLastTimeBias;
			float flHeightToMoveUp = flCurrentHeight - flLastTimeHeight;

			Vector vecNewPosition = mv->GetAbsOrigin() + Vector(0, 0, flHeightToMoveUp);
			TracePlayerBBoxWithStep (mv->GetAbsOrigin(), vecNewPosition, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
			mv->SetAbsOrigin (trace.endpos);

			m_pSDKPlayer->m_Shared.IncreaseDiveLerped(flCurrentLerp - m_pSDKPlayer->m_Shared.GetDiveLerped());
			Vector vecViewOffset = Lerp(flCurrentBias, GetPlayerViewOffset(false), VEC_DIVE_VIEW);
			m_pSDKPlayer->SetViewOffset( vecViewOffset );
		}
		else m_pSDKPlayer->SetViewOffset( VEC_DIVE_VIEW );
	}

	if (mv->m_nButtons&IN_JUMP) CheckJumpButton ();
	else mv->m_nOldButtons &= ~IN_JUMP;
	if (player->GetGroundEntity() != NULL)
	{
		mv->m_vecVelocity[2] = 0.0;
		Friction();
	}

	Vector vecForward;
	AngleVectors(mv->m_vecAngles, &vecForward);

	float flLookingUp = vecForward.Dot(Vector(0, 0, 1));

	float flTerminalVelocityLookingDown = da_terminal_velocity.GetFloat();
	float flTerminalVelocityLookingUp = da_terminal_velocity.GetFloat() - da_terminal_velocity_delta.GetFloat();
	float flTerminalVelocity = RemapValClamped(flLookingUp, -1, 1, flTerminalVelocityLookingDown, flTerminalVelocityLookingUp);

	if (mv->m_vecVelocity.z < -flTerminalVelocity)
		mv->m_vecVelocity.z = -flTerminalVelocity;

	CheckVelocity();
	if (player->GetGroundEntity() != NULL)
	{
		player->UpdateStepSound(player->m_pSurfaceData, mv->GetAbsOrigin (), mv->m_vecVelocity);
		WalkMove();
	}
	else
	{
		player->m_Local.m_flFallVelocity = -mv->m_vecVelocity[2];
		AirMove ();
	}

	CategorizePosition();
	CheckVelocity();
	if ( !CheckWater() )
	{
		FinishGravity();
	}
	if ( player->GetGroundEntity() != NULL )
	{
		mv->m_vecVelocity[2] = 0;
	}

	CheckFalling();
#ifdef GAME_DLL
	/*The great door hack*/
	{
		trace_t tr;
		float dt, slop;
		int msec, loss;
		
		UTIL_GetPlayerConnectionInfo (ENTINDEX (player), msec, loss);
		slop = msec/1000.0 + da_physpush_base.GetFloat ();
		dt = slop*m_pSDKPlayer->GetSlowMoMultiplier ();
		TraceBBox (mv->GetAbsOrigin (), 
				   mv->GetAbsOrigin () + dt*mv->m_vecVelocity, 
				   GetPlayerMins (), 
				   GetPlayerMaxs (), 
				   tr);
		if (tr.fraction < 1 && tr.DidHitNonWorldEntity ())
		{
			if (Q_strcmp ("func_physbox", tr.m_pEnt->GetClassname ()) == 0)
			{
				tr.m_pEnt->ApplyAbsVelocityImpulse (dt*mv->m_vecVelocity);
			}
		}
	}
#endif

	if ((m_nOldWaterLevel == WL_NotInWater && player->GetWaterLevel() != WL_NotInWater) ||
		( m_nOldWaterLevel != WL_NotInWater && player->GetWaterLevel() == WL_NotInWater))
	{/*Water enter/exit sound*/
#if !defined (CLIENT_DLL)
		player->Splash();
#endif
		PlaySwimSound();
	}
}

void CSDKGameMovement::StepMove( Vector &vecDestination, trace_t &trace )
{
	Vector vecEndPos;
	VectorCopy( vecDestination, vecEndPos );

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	Vector vecPos, vecVel;
	VectorCopy( mv->GetAbsOrigin(), vecPos );
	VectorCopy( mv->m_vecVelocity, vecVel );

	mv->SetAbsOrigin( vecPos );
	VectorCopy( vecVel, mv->m_vecVelocity );

	// Slide move down.
	TryPlayerMove( &vecEndPos, &trace );
	
	// Down results.
	Vector vecDownPos, vecDownVel;
	VectorCopy( mv->GetAbsOrigin(), vecDownPos );
	VectorCopy( mv->m_vecVelocity, vecDownVel );
	
	// Reset original values.
	mv->SetAbsOrigin( vecPos );
	VectorCopy( vecVel, mv->m_vecVelocity );
	
	// Move up a stair height.
	VectorCopy( mv->GetAbsOrigin(), vecEndPos );
	if ( player->m_Local.m_bAllowAutoMovement )
	{
		vecEndPos.z += player->m_Local.m_flStepSize + DIST_EPSILON;
	}
	
	TracePlayerBBox( mv->GetAbsOrigin(), vecEndPos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	if ( !trace.startsolid && !trace.allsolid )
	{
		mv->SetAbsOrigin( trace.endpos );
	}
	
	// Slide move up.
	TryPlayerMove();
	
	// Move down a stair (attempt to).
	VectorCopy( mv->GetAbsOrigin(), vecEndPos );
	if ( player->m_Local.m_bAllowAutoMovement )
	{
		vecEndPos.z -= player->m_Local.m_flStepSize + DIST_EPSILON;
	}
		
	TracePlayerBBox (mv->GetAbsOrigin (), vecEndPos, PlayerSolidMask (), COLLISION_GROUP_PLAYER_MOVEMENT, trace);
	if (trace.plane.normal[2] < 0.7)
	{
		mv->SetAbsOrigin( vecDownPos );
		VectorCopy( vecDownVel, mv->m_vecVelocity );
		float flStepDist = mv->GetAbsOrigin().z - vecPos.z;
		if ( flStepDist > 0.0f )
		{
			mv->m_outStepHeight += flStepDist;
		}
		return;
	}
	
	// If the trace ended up in empty space, copy the end over to the origin.
	if ( !trace.startsolid && !trace.allsolid )
	{
		mv->SetAbsOrigin( trace.endpos );
	}
	
	// Copy this origin to up.
	Vector vecUpPos;
	VectorCopy( mv->GetAbsOrigin(), vecUpPos );
	
	// decide which one went farther
	float flDownDist = ( vecDownPos.x - vecPos.x ) * ( vecDownPos.x - vecPos.x ) + ( vecDownPos.y - vecPos.y ) * ( vecDownPos.y - vecPos.y );
	float flUpDist = ( vecUpPos.x - vecPos.x ) * ( vecUpPos.x - vecPos.x ) + ( vecUpPos.y - vecPos.y ) * ( vecUpPos.y - vecPos.y );
	if ( flDownDist > flUpDist )
	{
		mv->SetAbsOrigin( vecDownPos );
		VectorCopy( vecDownVel, mv->m_vecVelocity );
	}
	else 
	{
		// copy z value from slide move
		mv->m_vecVelocity.z = vecDownVel.z;
	}
	
	float flStepDist = mv->GetAbsOrigin().z - vecPos.z;
	if ( flStepDist > 0 )
	{
		mv->m_outStepHeight += flStepDist;
	}
}

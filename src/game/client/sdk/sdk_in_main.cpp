//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: TF2 specific input handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "kbutton.h"
#include "input.h"
#include "sdk_in_main.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: TF Input interface
//-----------------------------------------------------------------------------
class CSDKInput : public CInput
{
public:
	void		GoFirstPerson();
	void		GoThirdPerson();
	void		ThirdPersonToggle();

	void		CAM_Think();
};

static CSDKInput g_Input;

// Expose this interface
IInput *input = ( IInput * )&g_Input;

CSDKInput* SDKInput()
{
	return &g_Input;
}

void CSDKInput::GoFirstPerson()
{
	input->CAM_ToFirstPerson();

	// Let the local player know
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
	if ( localPlayer )
		localPlayer->ThirdPersonSwitch( false );
}

void CSDKInput::GoThirdPerson()
{
	input->CAM_ToThirdPerson();

	// Let the local player know
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
	if ( localPlayer )
		localPlayer->ThirdPersonSwitch( true );
}

void CSDKInput::ThirdPersonToggle(void)
{
	if (input->CAM_IsThirdPerson())
		GoFirstPerson();
	else
		GoThirdPerson();
}

void AB_ThirdPersonToggle(void)
{
	SDKInput()->ThirdPersonToggle();
}

static ConCommand cam_thirdperson_toggle( "cam_thirdperson_toggle", ::AB_ThirdPersonToggle, "Toggle third person mode." );
static ConVar cl_thirdperson( "cl_thirdperson", "0", FCVAR_ARCHIVE, "Choose third person mode." );

void AB_Input_LevelInit()
{
	if (cl_thirdperson.GetBool())
		SDKInput()->GoThirdPerson();
	else
		SDKInput()->GoFirstPerson();
}

#define	DIST	 2
extern float MoveToward( float cur, float goal, float lag );
#define CAM_HULL_OFFSET		9.0    // the size of the bounding hull used for collision checking

void CSDKInput::CAM_Think( void )
{
	VPROF("CAM_Think");

	Vector idealAngles;
	Vector camOffset;
	QAngle viewangles;
	
	if( !CAM_IsThirdPerson() )
		return;

	ConVarRef cam_idealpitch( "cam_idealpitch" );
	ConVarRef cam_idealyaw( "cam_idealyaw" );
	ConVarRef cam_idealdist( "cam_idealdist" );

	idealAngles[ PITCH ] = cam_idealpitch.GetFloat();
	idealAngles[ YAW ]   = cam_idealyaw.GetFloat();
	idealAngles[ DIST ]  = cam_idealdist.GetFloat();

	// Obtain engine view angles and if they popped while the camera was static,
	// fix the camera angles as well
	engine->GetViewAngles( viewangles );

	// bring the pitch values back into a range that MoveToward can handle
	if ( idealAngles[ PITCH ] > 180 )
		idealAngles[ PITCH ] -= 360;
	else if ( idealAngles[ PITCH ] < -180 )
		idealAngles[ PITCH ] += 360;

	// bring the yaw values back into a range that MoveToward can handle
	// --
	// Vitaliy: going with >= 180 and <= -180.
	// This introduces a potential discontinuity when looking directly at model face
	// as camera yaw will be jumping from +180 to -180 and back, but when working with
	// the camera allows smooth rotational transitions from left to right and back.
	// Otherwise one of the transitions that has ">"-comparison will be locked.
	// --
	if ( idealAngles[ YAW ] >= 180 )
		idealAngles[ YAW ] -= 360;
	else if ( idealAngles[ YAW ] <= -180 )
		idealAngles[ YAW ] += 360;

	ConVarRef c_minpitch( "c_minpitch" );
	ConVarRef c_maxpitch( "c_maxpitch" );
	ConVarRef c_minyaw( "c_minyaw" );
	ConVarRef c_maxyaw( "c_maxyaw" );
	ConVarRef c_mindistance( "c_mindistance" );
	ConVarRef c_maxdistance( "c_maxdistance" );

	// clamp pitch, yaw and dist...
	idealAngles[ PITCH ] = clamp( idealAngles[ PITCH ], c_minpitch.GetFloat(), c_maxpitch.GetFloat() );
	idealAngles[ YAW ]   = clamp( idealAngles[ YAW ], c_minyaw.GetFloat(), c_maxyaw.GetFloat() );
	idealAngles[ DIST ]  = clamp( idealAngles[ DIST ], c_mindistance.GetFloat(), c_maxdistance.GetFloat() );

	// update ideal angles
	cam_idealpitch.SetValue( idealAngles[ PITCH ] );
	cam_idealyaw.SetValue( idealAngles[ YAW ] );
	cam_idealdist.SetValue( idealAngles[ DIST ] );
	
	// Move the CameraOffset "towards" the idealAngles
	// Note: CameraOffset = viewangle + idealAngle
	CAM_GetCameraOffset( camOffset );

	ConVarRef cam_snapto( "cam_snapto" );
	if( cam_snapto.GetInt() )
	{
		camOffset[ YAW ] = cam_idealyaw.GetFloat() + viewangles[ YAW ];
		camOffset[ PITCH ] = cam_idealpitch.GetFloat() + viewangles[ PITCH ];
		camOffset[ DIST ] = cam_idealdist.GetFloat();
	}
	else
	{
		ConVarRef cam_ideallag( "cam_ideallag" );
		float lag = max( 1, 1 + cam_ideallag.GetFloat() );

		if( camOffset[ YAW ] - viewangles[ YAW ] != cam_idealyaw.GetFloat() )
			camOffset[ YAW ] = MoveToward( camOffset[ YAW ], cam_idealyaw.GetFloat() + viewangles[ YAW ], lag );
		
		if( camOffset[ PITCH ] - viewangles[ PITCH ] != cam_idealpitch.GetFloat() )
			camOffset[ PITCH ] = MoveToward( camOffset[ PITCH ], cam_idealpitch.GetFloat() + viewangles[ PITCH ], lag );
		
		if( abs( camOffset[ DIST ] - cam_idealdist.GetFloat() ) < 2.0 )
			camOffset[ DIST ] = cam_idealdist.GetFloat();
		else
			camOffset[ DIST ] += ( cam_idealdist.GetFloat() - camOffset[ DIST ] ) / lag;
	}

	// move the camera closer to the player if it hit something
	trace_t trace;
	C_BasePlayer* localPlayer = C_BasePlayer::GetLocalPlayer();

	if ( localPlayer )
	{
		Vector camForward;

		// find our player's origin, and from there, the eye position
		Vector origin = localPlayer->GetLocalOrigin();
		origin += localPlayer->GetViewOffset();

		// get the forward vector
		AngleVectors( QAngle(camOffset[ PITCH ], camOffset[ YAW ], 0), &camForward, NULL, NULL );

		// use our previously #defined hull to collision trace
		CTraceFilterSimple traceFilter( localPlayer, COLLISION_GROUP_NONE );
		UTIL_TraceHull( origin, origin - (camForward * camOffset[ DIST ]),
			Vector(-CAM_HULL_OFFSET, -CAM_HULL_OFFSET, -CAM_HULL_OFFSET), Vector(CAM_HULL_OFFSET, CAM_HULL_OFFSET, CAM_HULL_OFFSET),
			MASK_SOLID, &traceFilter, &trace );

		// move the camera closer if it hit something
		if( trace.fraction < 1.0 )
		{
			camOffset[ DIST ] *= trace.fraction;
		}

		// For now, I'd rather see the insade of a player model than punch the camera through a wall
		// might try the fade out trick at some point
		//if( camOffset[ DIST ] < CAM_MIN_DIST )
		//    camOffset[ DIST ] = CAM_MIN_DIST; // clamp up to minimum
	}

	ConVarRef cam_showangles( "cam_showangles" );
	if ( cam_showangles.GetInt() )
	{
		engine->Con_NPrintf( 4, "Pitch: %6.1f   Yaw: %6.1f %38s", viewangles[ PITCH ], viewangles[ YAW ], "view angles" );
		engine->Con_NPrintf( 6, "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %19s", cam_idealpitch.GetFloat(), cam_idealyaw.GetFloat(), cam_idealdist.GetFloat(), "ideal angles" );
		engine->Con_NPrintf( 8, "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %16s", camOffset[ PITCH ], camOffset[ YAW ], camOffset[ DIST ], "camera offset" );
	}

	QAngle angOffset;
	angOffset.x = camOffset.x;
	angOffset.y = camOffset.y;
	angOffset.z = camOffset.z;

	CAM_SetCameraThirdData(NULL, angOffset);
}


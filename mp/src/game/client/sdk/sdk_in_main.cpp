//========= Copyright Valve Corporation, All rights reserved. ============//
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
#include "cam_thirdperson.h"

#include "c_sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: TF Input interface
//-----------------------------------------------------------------------------
class CSDKInput : public CInput
{
public:
	void		CAM_Think();
};

static CSDKInput g_Input;

// Expose this interface
IInput *input = ( IInput * )&g_Input;

CSDKInput* SDKInput()
{
	return &g_Input;
}

static ConVar da_thirdperson( "da_thirdperson", "1", FCVAR_USERINFO|FCVAR_ARCHIVE, "Choose third person mode." );

void AB_Input_LevelInit()
{
}

#define	DIST	 2
extern float MoveToward( float cur, float goal, float lag );

static ConVar da_cam_yaw("da_cam_yaw", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

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
	camOffset = g_ThirdPersonManager.GetCameraOffsetAngles();

	camOffset[ YAW ] = viewangles[ YAW ];
	camOffset[ PITCH ] = viewangles[ PITCH ];
	camOffset[ DIST ] = 0;

	QAngle angOffset;
	angOffset.x = camOffset.x;
	angOffset.y = camOffset.y + da_cam_yaw.GetFloat();
	angOffset.z = camOffset.z;

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();
	if (pPlayer)
		angOffset += pPlayer->GetPunchAngle();

	CAM_SetCameraThirdData(NULL, angOffset);
}


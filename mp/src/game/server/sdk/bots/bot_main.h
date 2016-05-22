//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#ifndef BOT_MAIN_H
#define BOT_MAIN_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player.h"

#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

// how often bot checks for obstacles (phys props, doors,etc) and deals with them
#define BOT_NEXT_OBSTACLE_CHECK 0.25f

// how often bot checks if got stuck
#define STUCK_SAMPLES 5.0f

// how quick bot turns 
#define SKILL_MIN_YAW_RATE 7.0f
#define SKILL_MAX_YAW_RATE 10.0f

// how fast bot runs
#define SKILL_MIN_SPEED 150.0f
#define SKILL_MAX_SPEED 250.0f

// how often bot strafes
#define SKILL_MIN_STRAFE 0.0f
#define SKILL_MAX_STRAFE 10.0f // set this to 0 to disable strafe skill at all

// spawn points are used as hide place references for bots
#define SPAWN_POINT_NAME "info_player_deathmatch" 

// ladder's top dismount point gets added some artificial distance to allow bot more room to complete the operation
#define LADDER_EXTRA_HEIGHT_VEC Vector(0,0,25)

enum BotGeneralStates_t
{
	BOT_NAVIG_IDLE, // bot just awaits
	BOT_NAVIG_PATH, // assigned a route to somewhere
	BOT_NAVIG_PATH_ENFORCED, // same but cannot be aborted by direct navigation
	BOT_NAVIG_DIRECT, // bot can see enemy and no obstacles in between detected, no need to ask for waypoints (less expensive)
	BOT_NAVIG_UNSTUCK, // bot is headed to a waypoint to get unstuck, cannot be interrupted no matter what
};

enum BotSchedules_t
{
	BOT_SCHED_COMBAT, 
	BOT_SCHED_HIDE,	
};

// these are the basic bot parameters, defined at creation time only
enum BotSkill_t
{
	BOT_SKILL_SPEED,
	BOT_SKILL_YAW_RATE,
	BOT_SKILL_STRAFE,

	BOT_SKILL_MAX_TOKENS,
};

// structure to store waypoints
struct NavAreaData_t
{
	Vector Center;
	NavTraverseType TransientType;
	int AttributeType;
	int Id;
};

// intermediate sized hull used when we want to test something smaller than the full bot hull, and bigger than a simple trace line
static Vector BotTestHull = Vector(5,5,5);

CBasePlayer *BotPutInServer( bool bFrozen, const char *name = NULL );

void Bot_RunAll();

#endif // BOT_MAIN_H

//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#include "bot_main.h"
#include "doors.h"
#include "BasePropDoor.h"

// support for nav mesh
#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

void DealWithObstacles( CSDKBot *pBot, CBaseEntity *pTouchEnt, CUserCmd &cmd  )
{
	if( !pTouchEnt  )
		return;

	pBot->m_flNextDealObstacles = gpGlobals->curtime + BOT_NEXT_OBSTACLE_CHECK;

	// fine navigation, some sort of
	// trace to anticipate possible obstacles, this deals with world brushes usually
	// let's say, if left trace detects an obstacle and right trace doesn't, then bot moves slightly into right direction
	Vector forward, right;
	pBot->EyeVectors( &forward, &right, NULL );
	Vector origin = pBot->GetLocalOrigin()+Vector(0,0,35);
	trace_t tr, tr2;

	UTIL_TraceLine( origin+right*17, (origin+right*17)+(forward*48), MASK_PLAYERSOLID, pBot, 0, &tr ); 
	UTIL_TraceLine( origin-right*17, (origin-right*17)+(forward*48), MASK_PLAYERSOLID, pBot, 0, &tr2 ); 

	if( tr.fraction < 1.0f && tr2.fraction == 1.0f )
	{
		pBot->m_flNextDealObstacles = 0;
		cmd.sidemove = -pBot->m_flSkill[BOT_SKILL_SPEED];
		cmd.forwardmove = 0;
	}
	else if( tr.fraction == 1.0f && tr2.fraction < 1.0f )
	{
		pBot->m_flNextDealObstacles = 0;
		cmd.sidemove = pBot->m_flSkill[BOT_SKILL_SPEED];
		cmd.forwardmove = 0;
	}

	// open doors 

	// model version
	if( FClassnameIs( pTouchEnt, "prop_door_rotating") )
	{
		CBasePropDoor *door = dynamic_cast<CBasePropDoor *>(pTouchEnt);

		if ( !door )
			return;

		if( door->IsDoorClosed() )
			((CBaseEntity*)door)->Use(pBot, pBot, USE_ON, 0);
	}

	// brush version
	if ( FClassnameIs( pTouchEnt, "func_door_rotating" ) )
	{
		CBaseDoor *door = dynamic_cast<CBaseDoor *>(pTouchEnt);
		if ( !door )
			return;

		if (door->m_toggle_state == TS_AT_BOTTOM )
			door->DoorGoUp();
	}

	// don't pay attention to undestructible objects
	// push objects too
	if ( FClassnameIs( pTouchEnt, "prop_physics") 
		|| FClassnameIs( pTouchEnt, "prop_physics_multiplayer") 
		|| FClassnameIs(pTouchEnt, "prop_physics_respawnable") 
		|| FClassnameIs(pTouchEnt, "func_breakable") )
	{
		CBreakableProp *pProp = dynamic_cast< CBreakableProp * >( pTouchEnt );
		if ( pProp )
		{
			// don't destroy explosive props
			if ( pProp->GetExplosiveDamage() > 0 ||  pProp->GetExplosiveRadius() > 0 )
				return;
		}

		if( pTouchEnt->HasSpawnFlags( SF_DYNAMICPROP_NO_VPHYSICS ) || pTouchEnt->HasSpawnFlags( SF_PHYSPROP_DEBRIS ) || pTouchEnt->HasSpawnFlags( SF_PHYSPROP_MOTIONDISABLED ) )
			return;

		bool hit = true;

		IPhysicsObject *pPhysicsObject = pTouchEnt->VPhysicsGetObject();

		if(pPhysicsObject != NULL )
		{			
			if( pPhysicsObject->IsAttachedToConstraint(false) ) // don't hit constrained items 
				hit = false;
			else // give some impulse to move the prop out of our way
			{
				Vector forward, up;
				pBot->GetVectors( &forward, NULL, &up );
				Vector vel = forward * 150 + up * 50;
				pPhysicsObject->AddVelocity( &vel, NULL );
			}
		}

		if( hit )			
			cmd.buttons |= IN_ATTACK;

		//pTouchEnt->TakeDamage( CTakeDamageInfo( pBot, pBot, 300, DMG_GENERIC ) ); // extra damage to it?
	}

}

void AddRandomPath( CSDKBot *pBot, float randomStartAngle = 0 )
{	
	if( pBot->m_flCreateRandomPathCoolDown > gpGlobals->curtime )
		return;

	pBot->ResetNavigationParams();

	QAngle fan;

	fan.x = 0;
	fan.z = 0;

	for( fan.y = randomStartAngle ; fan.y < 360 ; fan.y += 15.0 )
	{
		Vector vecDir;
		AngleVectors( fan, &vecDir );
		Vector endPos = pBot->GetLocalOrigin() + vecDir * 100.0f;

		trace_t tr, tr2;
		UTIL_TraceHull( pBot->GetLocalOrigin()+Vector(0,0,5), endPos+Vector(0,0,5), pBot->GetPlayerMins(), pBot->GetPlayerMaxs(), MASK_PLAYERSOLID_BRUSHONLY, pBot, COLLISION_GROUP_NONE, &tr ); 

		if(tr.fraction > 0.75f)
		{
			UTIL_TraceLine( tr.endpos, tr.endpos - Vector(0,0,100) , MASK_PLAYERSOLID_BRUSHONLY, pBot, 0, &tr2 ); // check the random waypoint is next to ground
				
			if( tr2.fraction != 1.0f )
			{
				pBot->AddWaypoint( tr2.endpos , GO_NORTH, 0, 0 ); // we actually use the near to ground position to avoid useless mid-air random waypoints
				pBot->m_nBotState = BOT_NAVIG_UNSTUCK;
				DevMsg("Random path created\n");
				return;
			}
		}
	}

	pBot->m_flCreateRandomPathCoolDown = gpGlobals->curtime+0.75f;

	DevMsg("Random path FAILED\n");

	
}

bool BotOnLadder( CSDKBot *pBot )
{
	if( pBot->m_Waypoints.Count() == 0 )
		return false;

	if( pBot->m_Waypoints[0].TransientType == GO_LADDER_UP || pBot->m_Waypoints[0].TransientType == GO_LADDER_DOWN )
		return true;

	return false;
}

bool ArrivedToWaypoint( CSDKBot *pBot )
{
	if( pBot->m_bIsOnLadder )
	{
		float dist = fabs(pBot->m_Waypoints[0].Center.z - pBot->GetLocalOrigin().z);
		//DevMsg(" L %f ", dist );

		if( pBot->m_Waypoints[0].TransientType == GO_LADDER_UP && dist < 10  )
			return true;
		else if( pBot->m_Waypoints[0].TransientType == GO_LADDER_DOWN && dist < 35 )
			return true;
	}
	else
	{	
		float dist = (pBot->GetLocalOrigin() - pBot->m_Waypoints[0].Center).Length();

		if( dist < ((pBot->m_nBotState == BOT_NAVIG_UNSTUCK) ? 15 : 35) ) // simple 3d distance check
			return true;

		float dist2d = (pBot->GetLocalOrigin() - pBot->m_Waypoints[0].Center).Length2D();

		trace_t tr;
		UTIL_TraceHull( pBot->GetLocalOrigin(), pBot->m_Waypoints[0].Center, BotTestHull, -BotTestHull, MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr ); 

		if( dist2d < 30 && tr.fraction == 1.0f ) // 2d check, in case bot flies over the checkpoint
		{
			return true;
		}
	}

	return false;
}

 //checks bot speed, if total amount each interval doesn't exceed a minimum, then bot is declared 'stuck'
void CheckStuck( CSDKBot *pBot, CUserCmd &cmd )
{
	if( pBot->m_nBotState == BOT_NAVIG_IDLE )
		return;

	//DevMsg( " %f ", pBot->m_flDistTraveled );

	if( pBot->m_flDistTraveled < 100.0f )
	{			
		CNavArea *area = TheNavMesh->GetNearestNavArea(  pBot->GetLocalOrigin() ); 
		if( area )
		{				
			area->IncreaseDanger(0, 1.0f); // let bots know this area is tricky to navigate so they avoid it eventually
		}			
		DevMsg("!STUCK, can't move at all\n");
		AddRandomPath(pBot);
		cmd.buttons |= IN_JUMP;
	}
	else if( pBot->m_Waypoints.Count() > 0 && (pBot->GetLocalOrigin()-pBot->m_Waypoints[0].Center).Length() < 50 && !pBot->m_bIsOnLadder && pBot->m_flDontUseDirectNav < gpGlobals->curtime) // check no obstacle in between bot and next waypoint
	{
		trace_t tr;
		UTIL_TraceHull( pBot->GetLocalOrigin()+Vector(0,0,15), pBot->m_Waypoints[0].Center+Vector(0,0,15), -BotTestHull, BotTestHull, MASK_SOLID_BRUSHONLY, pBot, COLLISION_GROUP_NONE, &tr ); 

		if( tr.fraction != 1.0 )
		{	
			NDebugOverlay::Line( pBot->GetLocalOrigin()+Vector(0,0,15), pBot->m_Waypoints[0].Center+Vector(0,0,15), 200, 0, 0, false, 5 );
			DevMsg("!STUCK can't see next waypoint\n");
			AddRandomPath(pBot);
		}
	}

	pBot->m_flDistTraveled = 0;
	pBot->m_flTimeToRecheckStuck = gpGlobals->curtime + (0.1f * STUCK_SAMPLES);
}

// nav mesh offers some important hints as crouch or jump, among others
void CheckNavMeshAttrib(  CSDKBot *pBot, trace_t *ptr, CUserCmd &cmd )
{
	if( pBot->m_Waypoints.Count() > 0 ) // waypoint based nav, hints are already stored
	{
		if( !pBot->m_bIsOnLadder && pBot->m_flNextJump < gpGlobals->curtime && ptr->fraction < 1.0f && (pBot->GetLocalOrigin().z - pBot->m_Waypoints[0].Center.z) > 50 ) // slope and there's obstacle, jump
		{
			cmd.buttons |= IN_JUMP;
			pBot->m_flNextJump = gpGlobals->curtime + 0.5f;
		}	

		if( pBot->m_Waypoints[0].AttributeType & NAV_MESH_CROUCH )
			cmd.buttons |= IN_DUCK;
	}
	else // direct nav, get hints from nearest nav area
	{
		CNavArea *nav = TheNavMesh->GetNearestNavArea( pBot->GetLocalOrigin(), false );
		if( nav )
		{
			if( nav->GetAttributes() & NAV_MESH_CROUCH )
				cmd.buttons |= IN_DUCK;

			if( nav->GetAttributes() & NAV_MESH_JUMP )
				cmd.buttons |= IN_JUMP;
		}
	}
}


bool CreatePath( CSDKBot *pBot, CBasePlayer *pPlayer, Vector OptionalOrg )
{
	if( !pBot || pBot == NULL )
		return false;

	pBot->m_flNextPathCheck = gpGlobals->curtime + 0.25f; // this function is expensive, make sure bot cannot use it every tick
	pBot->ResetWaypoints();

	Vector vEnd = (pPlayer != NULL) ? pPlayer->GetLocalOrigin() : OptionalOrg;

	CNavArea *closestArea = NULL;
	CNavArea *startArea = TheNavMesh->GetNearestNavArea(pBot->GetLocalOrigin());
	CNavArea *goalArea = TheNavMesh->GetNearestNavArea(vEnd);

	if( startArea == goalArea )
		return false;

	ShortestPathCost costfunc;

	// if you want to understand how waypoint gathering process work, read NavAreaBuildPath notes, very informative
	// nav areas are parented creating a path to goalArea. So we determine which ones are parented, and then they added to our waypoint list
	if( NavAreaBuildPath( startArea, goalArea, &vEnd, costfunc, &closestArea ) )
	{
		Vector center, center_portal, delta;
		float hwidth;
		NavDirType dir;

		goalArea = closestArea;

		Vector closestpoint;

		// waypopints are returned from last one (goal area) to closest one (start area)

		while( goalArea->GetParent() )
		{
			center = goalArea->GetParent()->GetCenter();
			dir = goalArea->ComputeDirection(&center);	
			goalArea->ComputePortal( goalArea->GetParent(), dir, &center_portal, &hwidth );
			goalArea->ComputeClosestPointInPortal( goalArea->GetParent(), dir, goalArea->GetParent()->GetCenter(), &closestpoint );

			closestpoint.z = goalArea->GetZ( closestpoint );

			NavTraverseType trav = goalArea->GetParentHow();

			// if previous waypoint is a ladder top dismount point, increase height a bit
			if( pBot->m_Waypoints.Count() > 0 && (pBot->m_Waypoints[0].Center.z - closestpoint.z ) > 64.0f && pBot->m_Waypoints[0].TransientType == GO_LADDER_UP )
			{
				FOR_EACH_VEC( TheNavMesh->GetLadders(), it )
				{
					CNavLadder *ladder = TheNavMesh->GetLadders()[ it ];

					// may be tricky to understand what I'm doing here, so here's some info:
					// if bot is meant to go up the ladder, some extra height is added to the ladder top end waypoint so the unmount operation can be completed successfully 
					// gets an extra impulse when leaving ladder, let's say
					if( goalArea->IsConnected( ladder, CNavLadder::LADDER_UP) )					
					{					
						pBot->AddWaypoint( ladder->m_top+LADDER_EXTRA_HEIGHT_VEC, GO_LADDER_UP, goalArea->GetAttributes(), ladder->GetID() ); // extra height to ladder top waypoint
						break;
					}					
				}
				trav = GO_NORTH; // since the new waypoint we just created is the reference for this ladder, set the original ladder waypoint to a non ladder traverse property
			}

			pBot->AddWaypoint( closestpoint, trav, goalArea->GetAttributes(), goalArea->GetID() );
			
			goalArea = goalArea->GetParent();
		}

		return true;
	}	

	DevMsg("!Can't create path from given start to goal areas\n");
	return false;
}

bool CreateHidePath( CSDKBot *pBot, Vector &HiDeSpot )
{
	DevMsg("Let's hide\n");
	pBot->m_flNextPathCheck = gpGlobals->curtime + 0.15f;

	CBaseEntity *pSpot = NULL;	
	while ( (pSpot = gEntList.FindEntityByClassname(pSpot, SPAWN_POINT_NAME) ) != NULL )
	{
		if ( pSpot )
		{
			float BotToHideSpotDist = (pSpot->GetLocalOrigin() - pBot->GetLocalOrigin()).Length();

			if( BotToHideSpotDist > 100 && BotToHideSpotDist < 1500 )
			{
				DevMsg("Found suitable spawn point\n");
				if( CreatePath( pBot, NULL, pSpot->GetLocalOrigin() ) ) // got a route to hide spot, now chech it for possible enemies near
				{
					DevMsg("Found hide spot\n");
					Vector prevWaypoint = vec3_origin;
					pBot->m_AlreadyCheckedHideSpots.AddToTail( pSpot->entindex() );

					for( int j=0; j<pBot->m_Waypoints.Count(); j++ )
					{
						float distToCarrier = (pBot->GetLocalOrigin() - pBot->m_Waypoints[j].Center).Length();

						if( distToCarrier < 350 )
							continue;

						if( prevWaypoint != vec3_origin )
						{
							float distToprevWaypoint = (prevWaypoint - pBot->m_Waypoints[j].Center).Length();

							if( distToprevWaypoint < 250 )
								continue;
						}

						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

							if ( pPlayer && pPlayer != NULL && pPlayer->IsAlive() && pPlayer != pBot )
							{
								float dist = (pBot->m_Waypoints[j].Center - pPlayer->GetLocalOrigin()).Length();

								if( dist > 750 )
									continue;

								trace_t tr;
								UTIL_TraceLine( pBot->m_Waypoints[j].Center, pPlayer->EyePosition(),MASK_SHOT, pPlayer, 0, &tr );

								if( tr.fraction == 1.0 ) // enemy can see this waypoint and is too close, so this route isn't valid
								{
									return false;
								}
							}
						}

						prevWaypoint = pBot->m_Waypoints[j].Center;
					}		

					HiDeSpot = pSpot->GetAbsOrigin();
					return true;					
				}
			}
		}			
	}

	pBot->ResetWaypoints();
	return false;
}

// decision making process, bot chooses a task to execute, pretty simplified here
void SelectSchedule( CSDKBot *pBot, bool forcePath = false )
{	
	Vector HideSpot;

	if( CreatePath( pBot, pBot->GetEnemy() ) ) // try to reach enemy
	{
		pBot->m_nBotState = forcePath ? BOT_NAVIG_PATH_ENFORCED : BOT_NAVIG_PATH; // the waypoint based route can be forced, it means that bot won't get distracted by any other thing
		pBot->m_nBotSchedule = BOT_SCHED_COMBAT;
	}
	else if ( CreateHidePath(pBot, HideSpot ) ) // if no route available then hide
	{
		if( CreatePath( pBot, NULL, HideSpot ) ) // create route to hide spot
		{
			pBot->m_nBotState = BOT_NAVIG_PATH_ENFORCED;
			pBot->m_nBotSchedule = BOT_SCHED_HIDE;
		}
		else
		{
			AddRandomPath(pBot);
			pBot->m_nBotState = BOT_NAVIG_PATH;
			pBot->m_nBotSchedule = BOT_SCHED_COMBAT;
		}
	}
	else // if everything else fails, just move randomly 
	{		
		AddRandomPath(pBot);
		pBot->m_nBotState = BOT_NAVIG_PATH;
		pBot->m_nBotSchedule = BOT_SCHED_COMBAT;
	}		
}	

// this function checks that non waypoint based navigation, including strafing, is safe enough (avoid deadly falls and such)
bool SafePathAhead( CSDKBot *pBot, Vector origin )
{
	if( pBot->m_bIsOnLadder || pBot->m_nBotState == BOT_NAVIG_PATH )
		return true;

	trace_t tr;
	UTIL_TraceHull( origin, origin - Vector(0,0,999999), -BotTestHull, BotTestHull, MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr ); 

	if( TheNavMesh->GetNearestNavArea( tr.endpos, false, 50 ) == NULL || ( (origin-tr.endpos).Length() > 50 && pBot->m_flHeightDifToEnemy == 0 ) ) // prevents the bot to walk straight into inappropiate places without nav mesh or high chances to fall
	{
		pBot->m_flDontUseDirectNav = gpGlobals->curtime + 1.0;

		if( pBot->m_nBotState == BOT_NAVIG_DIRECT )
			 SelectSchedule( pBot, true );

		return false;
	}

	return true;
}

void BotNavigation( CSDKBot *pBot, CUserCmd &cmd  )
{
	// teammate proximity check - this prevents two bots share same space, in case collisions among bots are disabled
	if( pBot->m_flNextProximityCheck < gpGlobals->curtime )
	{
		trace_t tr_teamm;
		Vector Forward;
		AngleVectors(pBot->GetLocalAngles(), &Forward);
		UTIL_TraceLine( pBot->GetLocalOrigin()+Vector(0,0,50), pBot->GetLocalOrigin() + Vector(0,0,50) + (Forward * 55), MASK_SHOT, pBot, COLLISION_GROUP_NONE, &tr_teamm );

		if( tr_teamm.m_pEnt && tr_teamm.m_pEnt->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( tr_teamm.m_pEnt );
			if( pPlayer && pPlayer->GetTeamNumber() == pBot->GetTeamNumber() && pPlayer->entindex() > pBot->entindex() ) // note how entity index is used to force only one of them to follow a random path
			{
				pBot->m_flNextProximityCheck = gpGlobals->curtime + 2.0f;
				random->SetSeed( gpGlobals->framecount * pBot->entindex() );
				AddRandomPath( pBot, (float)(random->RandomInt(0, random->RandomInt(0,80))+random->RandomInt(0, 60)+random->RandomInt(0, 60)) );
				return;
			}
		}
	}

	// Direct navigation, only one waypoint is set at enemy position, no extra WP are needed since there's a straight path with no obstacles in between. Shouldn't be used...
	if( pBot->m_flNextPathCheck < gpGlobals->curtime && // inmediately after some waypoints have been generated
		pBot->m_nBotState != BOT_NAVIG_UNSTUCK && // when bot is trying to get unstuck
		pBot->m_nBotSchedule == BOT_SCHED_COMBAT && // when bot is in the way to hide or something
		pBot->m_nBotState != BOT_NAVIG_PATH_ENFORCED && 
		pBot->m_bEnemyOnSights && // when bot/enemy have no direct vision over each other
		pBot->m_flDontUseDirectNav < gpGlobals->curtime ) // when told specifically for whatever reason, like using ladders
	{
		trace_t tr;
		UTIL_TraceHull( pBot->GetLocalOrigin()+Vector(0,0,5), pBot->GetEnemy()->GetLocalOrigin()+Vector(0,0,5), pBot->GetPlayerMins(), pBot->GetPlayerMaxs(), MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr ); 
					
		if( tr.m_pEnt == pBot->GetEnemy() ) // this checks if bot still can walk straight towards target, if not just reset waypoints so a route towards him is made
		{
			float BotToEnemyDist2d = (pBot->GetLocalOrigin() - pBot->GetEnemy()->GetLocalOrigin()).Length2D();
			//Msg(" %f ", tr.fraction );

			if( BotToEnemyDist2d < 16 ) // we are on top of enemy, forget about combat and get down there 
			{					
				pBot->m_flDontUseDirectNav = gpGlobals->curtime + 2.5f;
				DevMsg("!I'm on top of enemy, literally\n");
				if( pBot->GetLocalOrigin().z > pBot->GetEnemy()->GetLocalOrigin().z ) // let's only move if we are on top
					AddRandomPath( pBot );
				else
				{
					pBot->m_nBotState = BOT_NAVIG_UNSTUCK;
					pBot->ResetNavigationParams();		
				}
			}
			else
			{					
				if(  pBot->m_flHeightDifToEnemy > -50 ) 
					// there's a limit regarding enemy's height over chasing bot because direct navigation 
					// would glitch if a bot tries to get to an innacesible enemy
					// otherwise, if chasing bot is higher than its victim, this should work ok most of the time...
				{
					pBot->ResetNavigationParams();
					pBot->m_nBotState = BOT_NAVIG_DIRECT;
					pBot->AddWaypoint( pBot->GetEnemy()->GetLocalOrigin(), GO_NORTH, 0, 0 );
				}
			}
		}
	}
	
	// Waypoint navigation, if no direct navigation is possible (no waypoint was created), generate some waypoints
	if ( pBot->m_Waypoints.Count() == 0 && pBot->m_flNextPathCheck < gpGlobals->curtime )
		SelectSchedule( pBot );	

	// FORWARD MOVEMENT 
	// bot orientation (angles) towards next waypoint and speed is assigned here
	if( pBot->m_Waypoints.Count() > 0 )
	{
		Vector dir = vec3_origin;
		QAngle fwd;

		// get the desired angles bot should face to reach next waypoint
		fwd[YAW] = UTIL_VecToYaw ( pBot->m_Waypoints[0].Center - pBot->GetLocalOrigin() );
		fwd[PITCH] = fwd[ROLL] = 0;
		pBot->m_flDesiredYaw = fwd[YAW];
		
		//
		if( pBot->m_bIsOnLadder )
		{			
			pBot->m_flDontUseDirectNav = gpGlobals->curtime + 1.5f;				

			fwd[PITCH] = pBot->m_Waypoints[0].TransientType == GO_LADDER_UP ? -45 : 45;
		}

		// set desired view angles towards waypoint or enemy
		QAngle CurrentFwd = fwd;
		CurrentFwd[YAW] = pBot->EyeAngles()[YAW];
		
		float flYawDelta = AngleNormalize(  CurrentFwd[YAW] - pBot->m_flDesiredYaw );
		float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
		float rate = pBot->m_flSkill[BOT_SKILL_YAW_RATE];

		rate = clamp( rate, 0, fabs(flYawDelta) );
		CurrentFwd[YAW] += ( rate * flSide * gpGlobals->frametime * 30.0f );

		pBot->SnapEyeAngles(CurrentFwd);
		cmd.viewangles = pBot->EyeAngles();

		if( (!pBot->m_bEnemyOnSights || ( pBot->m_bEnemyOnSights && !pBot->m_bInRangeToAttack )) )
		{			
			Vector forward;
			pBot->EyeVectors( &forward, NULL, NULL );

			// gonna check the space in front of bot, just in case...
			Vector pos = (pBot->GetLocalOrigin()+Vector(0,0,50)) + (forward * 40);

			if( SafePathAhead( pBot, pos) )
			{
				cmd.buttons |= IN_FORWARD; // this actually only needed for ladders, or bot won't be able to unmount them
				cmd.forwardmove = pBot->m_flSkill[BOT_SKILL_SPEED];				

				if( fabs(flYawDelta) > 10 )  // decrease speed to avoid missing near waypoints
					cmd.forwardmove *= RemapValClamped(fabs(flYawDelta), 10.0f, 90.0f, 0.65f, 0.2f);
			}
		}

		if( ArrivedToWaypoint(pBot) )
		{
			if( pBot->m_flNextJump < gpGlobals->curtime && (pBot->m_Waypoints[0].TransientType == GO_JUMP || pBot->m_Waypoints[0].AttributeType & NAV_MESH_JUMP) )
			{
				pBot->m_flNextJump = gpGlobals->curtime + 0.5f; // bot jumps when reaching the waypoint that indicates it, I mean, it should happen right when reaching it, not before (when bot is moving towards it)
				cmd.buttons |= IN_JUMP;
			}

			pBot->m_Waypoints.Remove(0); // remove current waypoint
			pBot->m_bIsOnLadder = BotOnLadder( pBot );

			if ( pBot->m_Waypoints.Count() == 0 )
				pBot->ResetNavigationParams();
		}
	}

	// SIDEWAYS MOVEMENT 
	// bot shouldn't be an easy target unless you want it like that, so it strafes left and right randomly. Set m_flSkill[BOT_SKILL_STRAFE] = 0 to disable this behaviour
	if( pBot->m_flSkill[BOT_SKILL_STRAFE] > 0 && pBot->m_nBotState == BOT_NAVIG_DIRECT )
	{
		if( pBot->m_bEnemyOnSights 
			&& pBot->m_flNextStrafeTime < gpGlobals->curtime 
			&& pBot->m_flStrafeSkillRelatedTimer < gpGlobals->curtime 
			&& !pBot->m_bInRangeToAttack 
			&& pBot->m_flDontUseDirectNav < gpGlobals->curtime )
		{ 
			Vector right;
			pBot->EyeVectors( NULL, &right, NULL );
			Vector origin = pBot->GetLocalOrigin()+Vector(0,0,10);
			pBot->m_flSideMove = ( random->RandomInt( 0, 10 ) > 5 ? 1.0f : -1.0f ); // select left or right, randomly

			float amount = (pBot->m_flSkill[BOT_SKILL_SPEED]*0.5f) * pBot->m_flSideMove; // amount is relative to bot walk speed

			trace_t tr, trb;
		
			// checking there's actual room to walk sideways
			UTIL_TraceHull( origin, origin + right * amount, pBot->GetPlayerMins(), pBot->GetPlayerMaxs(), MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr ); 
			// checking bot walks sideways on solid ground
			UTIL_TraceLine( tr.endpos, tr.endpos - Vector(0,0,100) , MASK_PLAYERSOLID, pBot, 0, &trb );

			if( tr.fraction > 0.15f && trb.fraction < 0.25f )				
			{
				pBot->m_flNextStrafeTime = gpGlobals->curtime + random->RandomFloat(0.1f, tr.fraction*0.5f );
					
				// depending the bot skill, strafe again or wait some more time. The higher is strafe skill, the smaller are the pauses between each strafe movement
				pBot->m_flStrafeSkillRelatedTimer = gpGlobals->curtime + RemapValClamped( pBot->m_flSkill[BOT_SKILL_STRAFE], SKILL_MAX_STRAFE, SKILL_MIN_STRAFE, 0.15f, random->RandomFloat(0.5f, 0.85f) );			
			}			
		}
				
		if( pBot->m_flNextStrafeTime > gpGlobals->curtime )
		{
			Vector right;
			pBot->EyeVectors( NULL, &right, NULL );

			Vector pos = (pBot->GetLocalOrigin()+Vector(0,0,10)) + (right * (pBot->m_flSideMove * 60));

			if( SafePathAhead( pBot, pos) )	
			{
				//NDebugOverlay::Cross3DOriented( pBot->EyePosition(), QAngle(0,0,0), 20, 200, 0, 200, false, -1 );
				cmd.sidemove = pBot->m_flSkill[BOT_SKILL_SPEED] * pBot->m_flSideMove;
			}
		}
	}
}





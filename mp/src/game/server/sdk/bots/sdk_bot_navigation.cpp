#include "cbase.h"
#include "sdk_bot.h"

#include "BasePropDoor.h"
#include "in_buttons.h"

#include "sdk_gamerules.h"
#include "da_briefcase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CSDKBot::ResetNavigationParams()
{
	m_Waypoints.RemoveAll();
	m_flNextJump = 0;
	m_bIsOnLadder = false;
	m_flNextPathCheck = 0;
	m_flDontUseDirectNav = 0;
}

void CSDKBot::AddWaypoint( Vector center, NavTraverseType transient, int attribute, int id, bool AddToTail)
{
	NavAreaData_t data;
	VectorCopy( center, data.Center );
	data.TransientType = transient;
	data.AttributeType = attribute;
	data.Id = id;

	if( AddToTail )
		m_Waypoints.AddToTail(data);
	else
		m_Waypoints.AddToHead(data);
}

void CSDKBot::DealWithObstacles( CBaseEntity *pTouchEnt, CUserCmd &cmd )
{
	if( !pTouchEnt )
		return;

	m_flNextDealObstacles = gpGlobals->curtime + BOT_NEXT_OBSTACLE_CHECK;

	// fine navigation, some sort of
	// trace to anticipate possible obstacles, this deals with world brushes usually
	// let's say, if left trace detects an obstacle and right trace doesn't, then bot moves slightly into right direction
	Vector forward, right;
	EyeVectors( &forward, &right, NULL );
	Vector origin = GetLocalOrigin()+Vector(0,0,35);
	trace_t tr, tr2;

	UTIL_TraceLine( origin+right*17, (origin+right*17)+(forward*48), MASK_PLAYERSOLID, this, 0, &tr );
	UTIL_TraceLine( origin-right*17, (origin-right*17)+(forward*48), MASK_PLAYERSOLID, this, 0, &tr2 );

	if( tr.fraction < 1.0f && tr2.fraction == 1.0f )
	{
		m_flNextDealObstacles = 0;
		cmd.sidemove = -m_flSkill[BOT_SKILL_SPEED];
		cmd.forwardmove = 0;
	}
	else if( tr.fraction == 1.0f && tr2.fraction < 1.0f )
	{
		m_flNextDealObstacles = 0;
		cmd.sidemove = m_flSkill[BOT_SKILL_SPEED];
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
			((CBaseEntity*)door)->Use(this, this, USE_ON, 0);
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
			if ( pProp->GetExplosiveDamage() > 0 || pProp->GetExplosiveRadius() > 0 )
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
				GetVectors( &forward, NULL, &up );
				Vector vel = forward * 150 + up * 50;
				pPhysicsObject->AddVelocity( &vel, NULL );
			}
		}

		if( hit )
			cmd.buttons |= IN_ATTACK;

		//pTouchEnt->TakeDamage( CTakeDamageInfo( this, this, 300, DMG_GENERIC ) ); // extra damage to it?
	}
}

void CSDKBot::AddRandomPath( float randomStartAngle )
{
	if( m_flCreateRandomPathCoolDown > gpGlobals->curtime )
		return;

	ResetNavigationParams();

	QAngle fan;

	fan.x = 0;
	fan.z = 0;

	for( fan.y = randomStartAngle ; fan.y < 360 ; fan.y += 15.0 )
	{
		Vector vecDir;
		AngleVectors( fan, &vecDir );
		Vector endPos = GetLocalOrigin() + vecDir * 100.0f;

		trace_t tr, tr2;
		UTIL_TraceHull( GetLocalOrigin()+Vector(0,0,5), endPos+Vector(0,0,5), GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if(tr.fraction > 0.75f)
		{
			UTIL_TraceLine( tr.endpos, tr.endpos - Vector(0,0,100) , MASK_PLAYERSOLID_BRUSHONLY, this, 0, &tr2 ); // check the random waypoint is next to ground

			if( tr2.fraction != 1.0f )
			{
				AddWaypoint( tr2.endpos , GO_NORTH, 0, 0 ); // we actually use the near to ground position to avoid useless mid-air random waypoints
				m_nBotState = BOT_NAVIG_UNSTUCK;
				//DevMsg("Random path created\n");
				return;
			}
		}
	}

	m_flCreateRandomPathCoolDown = gpGlobals->curtime+0.75f;

	//DevMsg("Random path FAILED\n");
}

bool CSDKBot::BotOnLadder()
{
	if( m_Waypoints.Count() == 0 )
		return false;

	if( m_Waypoints[0].TransientType == GO_LADDER_UP || m_Waypoints[0].TransientType == GO_LADDER_DOWN )
		return true;

	return false;
}

bool CSDKBot::ArrivedToWaypoint()
{
	if( m_bIsOnLadder )
	{
		float dist = fabs(m_Waypoints[0].Center.z - GetLocalOrigin().z);
		//DevMsg(" L %f ", dist );

		if( m_Waypoints[0].TransientType == GO_LADDER_UP && dist < 10 )
			return true;
		else if( m_Waypoints[0].TransientType == GO_LADDER_DOWN && dist < 35 )
			return true;
	}
	else
	{
		float dist = (GetLocalOrigin() - m_Waypoints[0].Center).Length();

		if( dist < ((m_nBotState == BOT_NAVIG_UNSTUCK) ? 15 : 35) ) // simple 3d distance check
			return true;

		float dist2d = (GetLocalOrigin() - m_Waypoints[0].Center).Length2D();

		trace_t tr;
		UTIL_TraceHull( GetLocalOrigin(), m_Waypoints[0].Center, BotTestHull, -BotTestHull, MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

		if( dist2d < 30 && tr.fraction == 1.0f ) // 2d check, in case bot flies over the checkpoint
		{
			return true;
		}
	}

	return false;
}

//checks bot speed, if total amount each interval doesn't exceed a minimum, then bot is declared 'stuck'
void CSDKBot::CheckStuck( CUserCmd &cmd )
{
	if( m_nBotState == BOT_NAVIG_IDLE )
		return;

	//DevMsg( " %f ", m_flDistTraveled );

	if( m_flDistTraveled < 100.0f )
	{
		CNavArea *area = TheNavMesh->GetNearestNavArea( GetLocalOrigin() );
		if( area )
		{
			area->IncreaseDanger(0, 1.0f); // let bots know this area is tricky to navigate so they avoid it eventually
		}
		//DevMsg("!STUCK, can't move at all\n");
		AddRandomPath();
		cmd.buttons |= IN_JUMP;
	}
	else if( m_Waypoints.Count() > 0 && (GetLocalOrigin()-m_Waypoints[0].Center).Length() < 50 && !m_bIsOnLadder && m_flDontUseDirectNav < gpGlobals->curtime) // check no obstacle in between bot and next waypoint
	{
		trace_t tr;
		UTIL_TraceHull( GetLocalOrigin()+Vector(0,0,15), m_Waypoints[0].Center+Vector(0,0,15), -BotTestHull, BotTestHull, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if( tr.fraction != 1.0 )
		{
			//NDebugOverlay::Line( GetLocalOrigin()+Vector(0,0,15), m_Waypoints[0].Center+Vector(0,0,15), 200, 0, 0, false, 5 );
			//DevMsg("!STUCK can't see next waypoint\n");
			AddRandomPath();
		}
	}

	m_flDistTraveled = 0;
	m_flTimeToRecheckStuck = gpGlobals->curtime + (0.1f * STUCK_SAMPLES);
}

// nav mesh offers some important hints as crouch or jump, among others
void CSDKBot::CheckNavMeshAttrib( trace_t *ptr, CUserCmd &cmd )
{
	if( m_Waypoints.Count() > 0 ) // waypoint based nav, hints are already stored
	{
		if( !m_bIsOnLadder && m_flNextJump < gpGlobals->curtime && ptr->fraction < 1.0f && (GetLocalOrigin().z - m_Waypoints[0].Center.z) > 50 ) // slope and there's obstacle, jump
		{
			cmd.buttons |= IN_JUMP;
			m_flNextJump = gpGlobals->curtime + 0.5f;
		}

		if( m_Waypoints[0].AttributeType & NAV_MESH_CROUCH )
			cmd.buttons |= IN_DUCK;
	}
	else // direct nav, get hints from nearest nav area
	{
		CNavArea *nav = TheNavMesh->GetNearestNavArea( GetLocalOrigin(), false );
		if( nav )
		{
			if( nav->GetAttributes() & NAV_MESH_CROUCH )
				cmd.buttons |= IN_DUCK;

			if( nav->GetAttributes() & NAV_MESH_JUMP )
				cmd.buttons |= IN_JUMP;
		}
	}
}

bool CSDKBot::CreatePath( CBasePlayer *pPlayer, Vector OptionalOrg )
{
	m_flNextPathCheck = gpGlobals->curtime + 0.25f; // this function is expensive, make sure bot cannot use it every tick
	ResetWaypoints();

	Vector vEnd = (pPlayer != NULL) ? pPlayer->GetLocalOrigin() : OptionalOrg;

	CNavArea *closestArea = NULL;
	CNavArea *startArea = TheNavMesh->GetNearestNavArea(GetLocalOrigin());
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
			if( m_Waypoints.Count() > 0 && (m_Waypoints[0].Center.z - closestpoint.z ) > 64.0f && m_Waypoints[0].TransientType == GO_LADDER_UP )
			{
				FOR_EACH_VEC( TheNavMesh->GetLadders(), it )
				{
					CNavLadder *ladder = TheNavMesh->GetLadders()[ it ];

					// may be tricky to understand what I'm doing here, so here's some info:
					// if bot is meant to go up the ladder, some extra height is added to the ladder top end waypoint so the unmount operation can be completed successfully
					// gets an extra impulse when leaving ladder, let's say
					if( goalArea->IsConnected( ladder, CNavLadder::LADDER_UP) )
					{
						AddWaypoint( ladder->m_top+LADDER_EXTRA_HEIGHT_VEC, GO_LADDER_UP, goalArea->GetAttributes(), ladder->GetID() ); // extra height to ladder top waypoint
						break;
					}
				}
				trav = GO_NORTH; // since the new waypoint we just created is the reference for this ladder, set the original ladder waypoint to a non ladder traverse property
			}

			AddWaypoint( closestpoint, trav, goalArea->GetAttributes(), goalArea->GetID() );

			goalArea = goalArea->GetParent();
		}

		return true;
	}

	//DevMsg("!Can't create path from given start to goal areas\n");
	return false;
}

bool CSDKBot::CreateHidePath( Vector &HiDeSpot )
{
	//DevMsg("Let's hide\n");
	m_flNextPathCheck = gpGlobals->curtime + 0.15f;

	CBaseEntity *pSpot = NULL;
	while ( (pSpot = gEntList.FindEntityByClassname(pSpot, SPAWN_POINT_NAME) ) != NULL )
	{
		if ( pSpot )
		{
			float BotToHideSpotDist = (pSpot->GetLocalOrigin() - GetLocalOrigin()).Length();

			if( BotToHideSpotDist > 100 && BotToHideSpotDist < 1500 )
			{
				//DevMsg("Found suitable spawn point\n");
				if( CreatePath( NULL, pSpot->GetLocalOrigin() ) ) // got a route to hide spot, now chech it for possible enemies near
				{
					//DevMsg("Found hide spot\n");
					Vector prevWaypoint = vec3_origin;
					m_AlreadyCheckedHideSpots.AddToTail( pSpot->entindex() );

					for( int j=0; j<m_Waypoints.Count(); j++ )
					{
						float distToCarrier = (GetLocalOrigin() - m_Waypoints[j].Center).Length();

						if( distToCarrier < 350 )
							continue;

						if( prevWaypoint != vec3_origin )
						{
							float distToprevWaypoint = (prevWaypoint - m_Waypoints[j].Center).Length();

							if( distToprevWaypoint < 250 )
								continue;
						}

						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

							if ( pPlayer && pPlayer != NULL && pPlayer->IsAlive() && pPlayer != this )
							{
								float dist = (m_Waypoints[j].Center - pPlayer->GetLocalOrigin()).Length();

								if( dist > 750 )
									continue;

								trace_t tr;
								UTIL_TraceLine( m_Waypoints[j].Center, pPlayer->EyePosition(),MASK_SHOT, pPlayer, 0, &tr );

								if( tr.fraction == 1.0 ) // enemy can see this waypoint and is too close, so this route isn't valid
								{
									return false;
								}
							}
						}

						prevWaypoint = m_Waypoints[j].Center;
					}

					HiDeSpot = pSpot->GetAbsOrigin();
					return true;
				}
			}
		}
	}

	ResetWaypoints();
	return false;
}

// decision making process, bot chooses a task to execute, pretty simplified here
void CSDKBot::SelectSchedule( bool forcePath )
{
	Vector HideSpot;

	if (HasBriefcase() && CreatePath(NULL, SDKGameRules()->GetCaptureZone()->GetAbsOrigin()))
	{
		m_nBotState = BOT_NAVIG_PATH;
		m_nBotSchedule = BOT_SCHED_COMBAT;
	}
	else if (SDKGameRules()->GetBriefcase() && CreatePath(NULL, SDKGameRules()->GetBriefcase()->GetAbsOrigin()))
	{
		m_nBotState = BOT_NAVIG_PATH;
		m_nBotSchedule = BOT_SCHED_COMBAT;
	}
	else if( CreatePath( GetEnemy() ) ) // try to reach enemy
	{
		m_nBotState = forcePath ? BOT_NAVIG_PATH_ENFORCED : BOT_NAVIG_PATH; // the waypoint based route can be forced, it means that bot won't get distracted by any other thing
		m_nBotSchedule = BOT_SCHED_COMBAT;
	}
	else if ( CreateHidePath(HideSpot) ) // if no route available then hide
	{
		if( CreatePath( NULL, HideSpot ) ) // create route to hide spot
		{
			m_nBotState = BOT_NAVIG_PATH_ENFORCED;
			m_nBotSchedule = BOT_SCHED_HIDE;
		}
		else
		{
			AddRandomPath();
			m_nBotState = BOT_NAVIG_PATH;
			m_nBotSchedule = BOT_SCHED_COMBAT;
		}
	}
	else // if everything else fails, just move randomly
	{
		AddRandomPath();
		m_nBotState = BOT_NAVIG_PATH;
		m_nBotSchedule = BOT_SCHED_COMBAT;
	}
}

// this function checks that non waypoint based navigation, including strafing, is safe enough (avoid deadly falls and such)
bool CSDKBot::SafePathAhead( Vector origin )
{
	if( m_bIsOnLadder || m_nBotState == BOT_NAVIG_PATH )
		return true;

	trace_t tr;
	UTIL_TraceHull( origin, origin - Vector(0,0,999999), -BotTestHull, BotTestHull, MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

	if( TheNavMesh->GetNearestNavArea( tr.endpos, false, 50 ) == NULL || ( (origin-tr.endpos).Length() > 50 && m_flHeightDifToEnemy == 0 ) ) // prevents the bot to walk straight into inappropiate places without nav mesh or high chances to fall
	{
		m_flDontUseDirectNav = gpGlobals->curtime + 1.0;

		if( m_nBotState == BOT_NAVIG_DIRECT )
			SelectSchedule( true );

		return false;
	}

	return true;
}

void CSDKBot::Navigation( CUserCmd &cmd )
{
	int iLikelihood = 3;
	if (m_bEnemyOnSights)
		iLikelihood = 1;
	if (gpGlobals->curtime > m_flLastStuntTime + 4 && random->RandomInt(0, iLikelihood) == 0)
	{
		switch (random->RandomInt(0, 1))
		{
		default:
		case 0:
			// Dive
			cmd.buttons |= IN_ALT1;
			break;

		case 1:
			// Slide
			cmd.buttons |= IN_DUCK|IN_ALT1;
			break;
		}

		m_flLastStuntTime = gpGlobals->curtime;
	}

	// teammate proximity check - this prevents two bots share same space, in case collisions among bots are disabled
	if( m_flNextProximityCheck < gpGlobals->curtime )
	{
		trace_t tr_teamm;
		Vector Forward;
		AngleVectors(GetLocalAngles(), &Forward);
		UTIL_TraceLine( GetLocalOrigin()+Vector(0,0,50), GetLocalOrigin() + Vector(0,0,50) + (Forward * 55), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr_teamm );

		if( tr_teamm.m_pEnt && tr_teamm.m_pEnt->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer( tr_teamm.m_pEnt );
			if( pPlayer && pPlayer->GetTeamNumber() == GetTeamNumber() && pPlayer->entindex() > entindex() ) // note how entity index is used to force only one of them to follow a random path
			{
				m_flNextProximityCheck = gpGlobals->curtime + 2.0f;
				random->SetSeed( gpGlobals->framecount * entindex() );
				AddRandomPath( (float)(random->RandomInt(0, random->RandomInt(0,80))+random->RandomInt(0, 60)+random->RandomInt(0, 60)) );
				return;
			}
		}
	}

	// Direct navigation, only one waypoint is set at enemy position, no extra WP are needed since there's a straight path with no obstacles in between. Shouldn't be used...
	if( m_flNextPathCheck < gpGlobals->curtime && // inmediately after some waypoints have been generated
		m_nBotState != BOT_NAVIG_UNSTUCK && // when bot is trying to get unstuck
		m_nBotSchedule == BOT_SCHED_COMBAT && // when bot is in the way to hide or something
		m_nBotState != BOT_NAVIG_PATH_ENFORCED &&
		m_bEnemyOnSights && // when bot/enemy have no direct vision over each other
		m_flDontUseDirectNav < gpGlobals->curtime ) // when told specifically for whatever reason, like using ladders
	{
		trace_t tr;
		UTIL_TraceHull( GetLocalOrigin()+Vector(0,0,5), GetEnemy()->GetLocalOrigin()+Vector(0,0,5), GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

		if( tr.m_pEnt == GetEnemy() ) // this checks if bot still can walk straight towards target, if not just reset waypoints so a route towards him is made
		{
			float BotToEnemyDist2d = (GetLocalOrigin() - GetEnemy()->GetLocalOrigin()).Length2D();
			//Msg(" %f ", tr.fraction );

			if( BotToEnemyDist2d < 16 ) // we are on top of enemy, forget about combat and get down there
			{
				m_flDontUseDirectNav = gpGlobals->curtime + 2.5f;
				//DevMsg("!I'm on top of enemy, literally\n");
				if( GetLocalOrigin().z > GetEnemy()->GetLocalOrigin().z ) // let's only move if we are on top
					AddRandomPath();
				else
				{
					m_nBotState = BOT_NAVIG_UNSTUCK;
					ResetNavigationParams();
				}
			}
			else
			{
				if( m_flHeightDifToEnemy > -50 )
					// there's a limit regarding enemy's height over chasing bot because direct navigation
					// would glitch if a bot tries to get to an innacesible enemy
					// otherwise, if chasing bot is higher than its victim, this should work ok most of the time...
				{
					ResetNavigationParams();
					m_nBotState = BOT_NAVIG_DIRECT;
					AddWaypoint( GetEnemy()->GetLocalOrigin(), GO_NORTH, 0, 0 );
				}
			}
		}
	}

	bool bRunningFromGrenade = false;

	// Find average position of all nearby grenades
	Vector vecGrenade(0, 0, 0);
	int iGrenades = 0;
	CBaseEntity* pGrenade = NULL;
	while ((pGrenade = gEntList.FindEntityInSphere(pGrenade, WorldSpaceCenter(), 500)) != NULL)
	{
		if (!FStrEq(pGrenade->GetClassname(), "grenade_projectile"))
			continue;

		iGrenades++;
		vecGrenade += pGrenade->WorldSpaceCenter();
	}

	if (iGrenades)
	{
		vecGrenade /= iGrenades;

		ResetNavigationParams();
		m_nBotState = BOT_NAVIG_DIRECT;
		AddWaypoint( GetAbsOrigin() + (GetAbsOrigin() - vecGrenade).Normalized() * 500, GO_NORTH, 0, 0 );

		bRunningFromGrenade = true;
	}

	CBaseEntity* pWalkToTarget = NULL;
	if (SDKGameRules()->GetBriefcase() && !SDKGameRules()->GetBriefcase()->GetOwnerEntity())
		pWalkToTarget = SDKGameRules()->GetBriefcase();
	else if (HasBriefcase())
		pWalkToTarget = SDKGameRules()->GetCaptureZone();

	if (!bRunningFromGrenade && pWalkToTarget && gpGlobals->curtime > m_flDontUseDirectNav)
	{
		// If there's a briefcase and it's on the ground, check if I can see it.
		trace_t tr;
		UTIL_TraceLine( EyePosition(), pWalkToTarget->WorldSpaceCenter()+Vector(0,0,5), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

		if( tr.fraction >= 0.99f || tr.m_pEnt == pWalkToTarget )
		{
			float BotToEnemyDist2d = (GetAbsOrigin() - pWalkToTarget->GetAbsOrigin()).Length2D();
			//Msg(" %f ", tr.fraction );

			if( BotToEnemyDist2d < 16 ) // we are on top of enemy, forget about combat and get down there
			{
				m_flDontUseDirectNav = gpGlobals->curtime + 2.5f;
				//DevMsg("!I'm on top of enemy, literally\n");
				if( GetLocalOrigin().z > pWalkToTarget->GetAbsOrigin().z ) // let's only move if we are on top
					AddRandomPath();
				else
				{
					m_nBotState = BOT_NAVIG_UNSTUCK;
					ResetNavigationParams();
				}
			}
			else
			{
				if( m_flHeightDifToEnemy > -50 )
					// there's a limit regarding enemy's height over chasing bot because direct navigation
					// would glitch if a bot tries to get to an innacesible enemy
					// otherwise, if chasing bot is higher than its victim, this should work ok most of the time...
				{
					ResetNavigationParams();
					m_nBotState = BOT_NAVIG_DIRECT;
					AddWaypoint( pWalkToTarget->GetAbsOrigin(), GO_NORTH, 0, 0 );
				}
			}
		}
	}

	// Waypoint navigation, if no direct navigation is possible (no waypoint was created), generate some waypoints
	if ( m_Waypoints.Count() == 0 && m_flNextPathCheck < gpGlobals->curtime )
		SelectSchedule();

	// FORWARD MOVEMENT
	// bot orientation (angles) towards next waypoint and speed is assigned here
	if( m_Waypoints.Count() > 0 )
	{
		Vector dir = vec3_origin;
		QAngle fwd;

		// get the desired angles bot should face to reach next waypoint
		fwd[YAW] = UTIL_VecToYaw ( m_Waypoints[0].Center - GetLocalOrigin() );
		fwd[PITCH] = fwd[ROLL] = 0;
		m_flDesiredYaw = fwd[YAW];

		//
		if( m_bIsOnLadder )
		{
			m_flDontUseDirectNav = gpGlobals->curtime + 1.5f;

			fwd[PITCH] = m_Waypoints[0].TransientType == GO_LADDER_UP ? -45 : 45;
		}

		// set desired view angles towards waypoint or enemy
		QAngle CurrentFwd = fwd;
		CurrentFwd[YAW] = EyeAngles()[YAW];

		float flYawDelta = AngleNormalize( CurrentFwd[YAW] - m_flDesiredYaw );
		float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
		float rate = m_flSkill[BOT_SKILL_YAW_RATE];

		rate = clamp( rate, 0, fabs(flYawDelta) );
		CurrentFwd[YAW] += ( rate * flSide * gpGlobals->frametime * 30.0f );

		SnapEyeAngles(CurrentFwd);
		cmd.viewangles = EyeAngles();

		if( (!m_bEnemyOnSights || ( m_bEnemyOnSights && !m_bInRangeToAttack )) )
		{
			Vector forward;
			EyeVectors( &forward, NULL, NULL );

			// gonna check the space in front of bot, just in case...
			Vector pos = (GetLocalOrigin()+Vector(0,0,50)) + (forward * 40);

			if( SafePathAhead(pos) )
			{
				cmd.buttons |= IN_FORWARD; // this actually only needed for ladders, or bot won't be able to unmount them
				cmd.forwardmove = m_flSkill[BOT_SKILL_SPEED];

				if( fabs(flYawDelta) > 10 ) // decrease speed to avoid missing near waypoints
					cmd.forwardmove *= RemapValClamped(fabs(flYawDelta), 10.0f, 90.0f, 0.65f, 0.2f);
			}
		}

		if( ArrivedToWaypoint() )
		{
			if( m_flNextJump < gpGlobals->curtime && (m_Waypoints[0].TransientType == GO_JUMP || m_Waypoints[0].AttributeType & NAV_MESH_JUMP) )
			{
				m_flNextJump = gpGlobals->curtime + 0.5f; // bot jumps when reaching the waypoint that indicates it, I mean, it should happen right when reaching it, not before (when bot is moving towards it)
				cmd.buttons |= IN_JUMP;
			}

			m_Waypoints.Remove(0); // remove current waypoint
			m_bIsOnLadder = BotOnLadder( );

			if ( m_Waypoints.Count() == 0 )
				ResetNavigationParams();
		}
	}

	// SIDEWAYS MOVEMENT
	// bot shouldn't be an easy target unless you want it like that, so it strafes left and right randomly. Set m_flSkill[BOT_SKILL_STRAFE] = 0 to disable this behaviour
	if( m_flSkill[BOT_SKILL_STRAFE] > 0 && m_nBotState == BOT_NAVIG_DIRECT )
	{
		if( m_bEnemyOnSights
			&& m_flNextStrafeTime < gpGlobals->curtime
			&& m_flStrafeSkillRelatedTimer < gpGlobals->curtime
			&& !m_bInRangeToAttack
			&& m_flDontUseDirectNav < gpGlobals->curtime
			&& !pWalkToTarget )
		{
			Vector right;
			EyeVectors( NULL, &right, NULL );
			Vector origin = GetLocalOrigin()+Vector(0,0,10);
			m_flSideMove = ( random->RandomInt( 0, 10 ) > 5 ? 1.0f : -1.0f ); // select left or right, randomly

			float amount = (m_flSkill[BOT_SKILL_SPEED]*0.5f) * m_flSideMove; // amount is relative to bot walk speed

			trace_t tr, trb;

			// checking there's actual room to walk sideways
			UTIL_TraceHull( origin, origin + right * amount, GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );
			// checking bot walks sideways on solid ground
			UTIL_TraceLine( tr.endpos, tr.endpos - Vector(0,0,100) , MASK_PLAYERSOLID, this, 0, &trb );

			if( tr.fraction > 0.15f && trb.fraction < 0.25f )
			{
				m_flNextStrafeTime = gpGlobals->curtime + random->RandomFloat(0.1f, tr.fraction*0.5f );

				// depending the bot skill, strafe again or wait some more time. The higher is strafe skill, the smaller are the pauses between each strafe movement
				m_flStrafeSkillRelatedTimer = gpGlobals->curtime + RemapValClamped( m_flSkill[BOT_SKILL_STRAFE], SKILL_MAX_STRAFE, SKILL_MIN_STRAFE, 0.15f, random->RandomFloat(0.5f, 0.85f) );
			}
		}

		if( m_flNextStrafeTime > gpGlobals->curtime )
		{
			Vector right;
			EyeVectors( NULL, &right, NULL );

			Vector pos = (GetLocalOrigin()+Vector(0,0,10)) + (right * (m_flSideMove * 60));

			if( SafePathAhead(pos) )
			{
				//NDebugOverlay::Cross3DOriented( EyePosition(), QAngle(0,0,0), 20, 200, 0, 200, false, -1 );
				cmd.sidemove = m_flSkill[BOT_SKILL_SPEED] * m_flSideMove;
			}
		}
	}
}

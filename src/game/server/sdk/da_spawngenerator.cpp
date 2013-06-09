// --------------------------------------------------------------------------------------------------- //
// Helper class to generate a grid of spawnpoints from a given point
// --------------------------------------------------------------------------------------------------- //

#include "cbase.h"
#include "da_spawngenerator.h"
#include "sdk_shareddefs.h"

// set up initialization values and populate the grid
CSpawnPointGenerator::CSpawnPointGenerator( CBaseEntity *pRefEnt, int team, int numSpawns)
{
	m_vecCenter = pRefEnt->GetAbsOrigin();
	m_angles = pRefEnt->GetAbsAngles();
	m_pszPointName = (team == SDK_TEAM_BLUE ? "info_player_blue" : "info_player_red");
	m_iSpawnsDesired = numSpawns;
	m_iSpawnsCreated = 0;
	
	// make our trace hull very short so we can account for small z changes in geometry
	TRACE_HULL_MIN = VEC_DUCK_HULL_MIN;
	TRACE_HULL_MAX = VEC_DUCK_HULL_MAX;

	// trace down to see if our initial point is over water, if so we allow other points to be created over water
	trace_t tr;
	UTIL_TraceLine(m_vecCenter, m_vecCenter + Vector(0,0,-MAX_TRACE_LENGTH), (MASK_PLAYERSOLID|MASK_WATER), NULL, &tr);
	if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
		m_bWaterOk = true;
	else
		m_bWaterOk = false;

	// offset from ground so minor terrain changes don't block trace
	m_vecCenter.z = tr.endpos.z + 48.0f;

	Vector size = m_vecCenter - tr.endpos;

	iCurMinX = -EXTEND_FROM_CENTER;
	iCurMinY = -EXTEND_FROM_CENTER;

	InitSpawnGrid();
	PopulateSpawnGrid();
}

// populates the spawn grid and initializes all values
void CSpawnPointGenerator::InitSpawnGrid()
{		
	Vector vecForward, vecRight, vecUp;
	AngleVectors( m_angles, &vecForward, &vecRight, &vecUp );

	vecForward.z = 0.0f;
	vecRight.z = 0.0f;
	VectorNormalize(vecForward);
	VectorNormalize(vecRight);

	// straighten it out to closest NOR/EST/SOU/WES before we make a grid
	if (fabs(vecForward.x) > fabs(vecRight.x))
	{
		if (vecForward.x > 0)
		{
			vecForward.x = 1;
			vecRight.y = -1;
		}
		else
		{
			vecForward.x = -1;
			vecRight.y = 1;	
		}
		vecForward.y = 0;
		vecRight.x = 0;
	}
	else
	{
		if (vecRight.x > 0)
		{
			vecForward.y = 1;
			vecRight.x = 1;
		}
		else
		{
			vecForward.y = -1;
			vecRight.x = -1;	
		}
		vecForward.x = 0;
		vecRight.y = 0;
	}

	vecForward *= DIST_TO_NEXT;
	vecRight *= DIST_TO_NEXT;
	
	Vector vecLeft = -vecRight;
	Vector vecBack = -vecForward;

	// populate spawngrid
	// start from lowest index (frontmost/rightmost location)
	Vector vecStart;
	SpawnGridLoc *pLoc;

	// use coordinates to measure each point's origin relative to distance from center
	for (int i = iMinX; i <= iMaxX; i++)
	{
		vecStart = m_vecCenter + vecLeft * i;
		for (int k = iMinY; k <= iMaxY; k++)
		{
			pLoc = GridLoc(i,k);
			pLoc->vecOrigin = (vecStart + vecBack * k);
			pLoc->xIndex = i;
			pLoc->yIndex = k;
			pLoc->iStatus = UNCHECKED;
		}
	}
}

// start checking points, try to fill 
void CSpawnPointGenerator::PopulateSpawnGrid()
{
	// only allow us to expand in positive axes for now
	iCurMinX = 0;
	iCurMinY = 0;

	// start by filling a line back from our center point
	FillInDir(GridLoc(0,0), DIR_BACK);

	// iterate the try lists until all valid grid locations are assigned a spawn point
	ParseTryList(DIR_LEFT);

	// if we still need points, expand to the grid quadrant in the negative x axis
	SpawnGridLoc *pLoc;
	if (m_iSpawnsCreated < m_iSpawnsDesired)
	{
		iCurMinX = -EXTEND_FROM_CENTER;

		// have each of our rightmost points add themselves to the try lists
		for( int i = 0; i <= iMaxY; i++)
		{
			pLoc = GridLoc(0,i);
			if (pLoc->iStatus == OCCUPIED)
				UpdateTryLists(pLoc);
		}

		// iterate the try lists, starting with the check-right list
		ParseTryList(DIR_RIGHT);
	}

	// if we still need points then expand to the negative y axis
	if (m_iSpawnsCreated < m_iSpawnsDesired)
	{
		iCurMinY = -EXTEND_FROM_CENTER;
		for (int i = iMaxX; i >= iMinX; i--)
		{
			pLoc = GridLoc(i,0);
			if (pLoc->iStatus == OCCUPIED)
				UpdateTryLists(pLoc);
		}

		ParseTryList(DIR_FRONT);
	}
}

// based on given dir set x or y to the furthest point in that direction
void CSpawnPointGenerator::BoundsForDir( int dir, int *xmod, int *ymod)
{
	switch(dir)
	{
		case DIR_LEFT:
			{
			*xmod = iMaxX;
			break;
			}
		case DIR_RIGHT:
			{
			*xmod = iCurMinX;
			break;
			}
		case DIR_FRONT:
			{
			*ymod = iCurMinY;
			break;
			}
		// defaults to back dir
		default:
			*ymod = iMaxY;
	}
}


// checks the Z-axis validity of the given point
bool CSpawnPointGenerator::CheckZ( SpawnGridLoc *refLoc )
{
	//return true;
	trace_t tr;
	Vector refOrigin = refLoc->vecOrigin;

	// check for a point below within z tolerance
	UTIL_TraceHull(refOrigin, refOrigin + Vector(0,0,ZTOL), TRACE_HULL_MIN, TRACE_HULL_MAX, MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);
	
	refOrigin = tr.endpos;

	// if we didn't hit anything or we hit water when we're not supposed to set the point to blocked
	if ( ( tr.fraction == 1.0f ) || (!m_bWaterOk && (enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME))) )
		return false;
	else
		// if the down trace worked out trace up about the size of a spawnpoint
		UTIL_TraceHull(refOrigin, refOrigin + Vector(0,0,72), Vector(-16,-16,0), Vector(16,16,2), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);

	// this time we prefer not to hit anything
	if ( tr.fraction == 1.0f )
	{
		refLoc->vecOrigin = refOrigin;
		return true;
	}

	return false;
}

// trace a line from the given point to the furthest grid bound in given direction
void CSpawnPointGenerator::StepThroughDir( SpawnGridLoc *refLoc, int dir )
{
	int x = refLoc->xIndex;
	int y = refLoc->yIndex;

	// get max boundaries for the given direction
	int xmod = x;
	int ymod = y;
	BoundsForDir(dir, &xmod, &ymod);

	trace_t tr;
	int iClearPoints;

	// just a multiplier for a cheap way to determine direction
	int inc = 1;
		
	UTIL_TraceHull(refLoc->vecOrigin, GridLoc(xmod,ymod)->vecOrigin, TRACE_HULL_MIN, TRACE_HULL_MAX, MASK_PLAYERSOLID,
		NULL, COLLISION_GROUP_NONE, &tr);

	SpawnGridLoc *pLoc;

	// if our xbound hasn't changed it means we're iterating through the y axis
	if ( xmod == x )
	{
		if ( ymod < y )
			inc = -1;

		// take the fraction of max possible points in the given direction
		iClearPoints = inc*((-y) + ymod);
		iClearPoints *= tr.fraction;
		int i;
		for (i = 1; i <= iClearPoints; i++)
		{
			pLoc = GridLoc(x,y+(inc*i));
			// CheckZ makes sure nothing fishy is going on
			if (CheckZ(pLoc))
				pLoc->iStatus = OPEN;
			else
				pLoc->iStatus = BLOCKED;
		}
		// if we stopped before hitting our bound it means an obstruction is at this index
		i *= inc;
		i += y;
		if ( (iMinY <= i) && (i <= iMaxY) )
			GridLoc(x,i)->iStatus = BLOCKED;
	}
	else
	{
		if ( xmod < x )
			inc = -1;

		// take the fraction of max possible points in the given direction
		iClearPoints = inc*((-x) + xmod);
		iClearPoints *= tr.fraction;
		int i;
		for (i = 1; i <= iClearPoints; i++)
		{
			pLoc = GridLoc(x+(inc*i),y);
			// CheckZ makes sure nothing fishy is going on
			if (CheckZ(pLoc))
				pLoc->iStatus = OPEN;
			else
				pLoc->iStatus = BLOCKED;
		}
		// if we stopped before hitting our bound it means an obstruction is at this index
		i *= inc;
		i += x;
		if ( (iMinX <= i) && (i <= iMaxX) )
			GridLoc(i,y)->iStatus = BLOCKED;
	}
}

// for every clear point in the given direction add a spawn point, return number created
int CSpawnPointGenerator::FillInDir( SpawnGridLoc *refLoc, int dir )
{
	StepThroughDir(refLoc, dir);
	int numCreated = 0;

	// create a new spot in each open space
	do {
		CBaseEntity *newSpot = CreateEntityByName( m_pszPointName );
		newSpot->SetAbsAngles(m_angles);
		newSpot->SetAbsOrigin(refLoc->vecOrigin);
		refLoc->iStatus = OCCUPIED;
		m_iSpawnsCreated++;
		numCreated++;
		UpdateTryLists(refLoc);
		refLoc = CheckStatusInDir(refLoc, dir);
	} while (refLoc && (refLoc->iStatus == OPEN));

	return numCreated;
}

// return status 1 point from the given point in the direction specified
CSpawnPointGenerator::SpawnGridLoc* CSpawnPointGenerator::CheckStatusInDir( SpawnGridLoc *pLoc, int dir )
{
	int x = pLoc->xIndex;
	int y = pLoc->yIndex;
	SpawnGridLoc *refLoc;

	switch(dir)
	{
	case DIR_LEFT:
		refLoc = GridLoc(x+1,y);
		break;
	case DIR_RIGHT:
		refLoc = GridLoc(x-1,y);
		break;
	case DIR_FRONT:
		refLoc = GridLoc(x,y-1);
		break;
	default:
		refLoc = GridLoc(x,y+1);
		break;
	}

	// if the point hasn't been checked trace in the given direction
	if (refLoc->iStatus == UNCHECKED)
		StepThroughDir(pLoc, dir);
	return refLoc;
}


void CSpawnPointGenerator::ParseTryList(int dir)
{
	int iSkipPoints = 0;
	int iListCount;
	SpawnGridLoc *pLoc;

	switch(dir)
	{
	case DIR_LEFT:
		{
			int iListCount = m_arrCanLeft.Count();
			while (iListCount > 0)
			{
				iSkipPoints = 0;
				// returns the point 1 unit in this direction and makes sure it's checked
				pLoc = CheckStatusInDir(m_arrCanLeft.Head(), dir);
										
				// start this at 1 since we always remove one more list element than points created
				iSkipPoints++;
					
				if (pLoc && pLoc->iStatus == OPEN)
				{
					// attempt to create a row of points back
					iSkipPoints += FillInDir(pLoc, DIR_BACK);

					// remove the number of points created from our list of points to try, but don't spill into the next row
					iSkipPoints = min(iSkipPoints, iListCount);
				}
				iListCount -= iSkipPoints;
				m_arrCanLeft.RemoveMultiple(0,iSkipPoints);
			}
			// if anything happened check the list again, otherwise proceed to the right list
			if (iSkipPoints > 0)
			{
				ParseTryList(DIR_LEFT);
				break;
			}
		}
	case DIR_RIGHT:
		{
			iListCount = m_arrCanRight.Count();
			while (iListCount > 0)
			{
				iSkipPoints = 0;
				// returns the point 1 unit in this direction and makes sure it's checked
				pLoc = CheckStatusInDir(m_arrCanRight.Head(), dir);
										
				// start this at 1 since we always remove one more list element than points created
				iSkipPoints++;
					
				if (pLoc && pLoc->iStatus == OPEN)
				{
					// attempt to create a row of points back
					iSkipPoints += FillInDir(pLoc, DIR_BACK);

					// remove the number of points created from our list of points to try, but don't spill into the next row
					iSkipPoints = min(iSkipPoints, iListCount);
				}
				iListCount -= iSkipPoints;
				m_arrCanRight.RemoveMultiple(0,iSkipPoints);
			}
			// if anything happened check the list again, otherwise proceed to the front list
			if (iSkipPoints > 0)
			{
				ParseTryList(DIR_RIGHT);
				break;
			}
		}
	case DIR_FRONT:
		{
			iListCount = m_arrCanFront.Count();
			while (iListCount > 0)
			{
				iSkipPoints = 0;
				// returns the point 1 unit in this direction and makes sure it's checked
				pLoc = CheckStatusInDir(m_arrCanFront.Head(), dir);
										
				// start this at 1 since we always remove one more list element than points created
				iSkipPoints++;
					
				if (pLoc && pLoc->iStatus == OPEN)
				{
					// attempt to create a row of points to the right
					iSkipPoints += FillInDir(pLoc, DIR_RIGHT);

					// remove the number of points created from our list of points to try, but don't spill into the next row
					iSkipPoints = min(iSkipPoints, iListCount);
				}
				iListCount -= iSkipPoints;
				m_arrCanFront.RemoveMultiple(0,iSkipPoints);
			}
			// if any points were created check the left list
			if (iSkipPoints > 0)
			{
				ParseTryList(DIR_LEFT);
			}
		} break;
	}
}

// for each adjacent grid point that isn't known to be invalid, add the given
// point to the 
void CSpawnPointGenerator::UpdateTryLists( SpawnGridLoc *refLoc )
{
	int x = refLoc->xIndex;
	int y = refLoc->yIndex;
		
	if ( (x < iMaxX) && (GridLoc(x+1,y)->iStatus < BLOCKED) )
		m_arrCanLeft.AddToTail(refLoc);
	if ( (x > iCurMinX) && (GridLoc(x-1,y)->iStatus < BLOCKED) )
		m_arrCanRight.AddToTail(refLoc);
	if ( (y > iCurMinY) && (GridLoc(x,y-1)->iStatus < BLOCKED) )
		m_arrCanFront.AddToTail(refLoc);
}

int CSpawnPointGenerator::CalculateIndex(int x, int y)
{
	// offset by our center coordinates
	x += iMaxX;
	y += iMaxY;

	// y values are consecutive in the index
	x *= DIAMETER;
	return (x+y);
}

CSpawnPointGenerator::SpawnGridLoc* CSpawnPointGenerator::GridLoc(int x, int y)
{
	return &m_arrSpawnGrid[CalculateIndex(x,y)];
}
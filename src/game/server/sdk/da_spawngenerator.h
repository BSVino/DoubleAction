// --------------------------------------------------------------------------------------------------- //
// Helper class to generate a grid of spawnpoints from a given point
// --------------------------------------------------------------------------------------------------- //

#define DIST_TO_NEXT 38.0f //32x32 spawnpoint size + buffer of 6
#define EXTEND_FROM_CENTER 4 //this is how far we extend from our center point (bounding area is (2n+1)^2)
#define DIAMETER 9 // 2 * EXTEND_FROM_CENTER + 1 plz
#define ZTOL -300.0f // our z axis wiggle room
#define SPAWNMASK MASK_PLAYERSOLID
class CSpawnPointGenerator
{
public:
	enum GridLocState
	{
		UNCHECKED = 0,
		OPEN,
		BLOCKED,
		OCCUPIED
	};

	enum ExpandGridDir
	{
		DIR_LEFT = 0,
		DIR_RIGHT,
		DIR_FRONT,
		DIR_BACK
	};

	struct SpawnGridLoc
	{
		Vector vecOrigin;
		int xIndex;
		int yIndex;
		int iStatus;
	};

	CSpawnPointGenerator( CBaseEntity *pRefSpot, int team, int numSpawns);
    CSpawnPointGenerator( const CSpawnPointGenerator& o )
    {
        *this = o;
    }
	~CSpawnPointGenerator(){};

	int SpawnsCreated() { return m_iSpawnsCreated; }

private:
	void InitSpawnGrid();
	void PopulateSpawnGrid();

	void BoundsForDir( int dir, int *xmod, int *ymod);

	bool CheckZ( SpawnGridLoc *refLoc );

	void StepThroughDir( SpawnGridLoc *refLoc, int dir );

	int FillInDir( SpawnGridLoc *refLoc, int dir );

	// returns gridloc 1 unit in the direction checked
	SpawnGridLoc *CheckStatusInDir( SpawnGridLoc *pLoc, int dir );

	void ParseTryList(int dir);

	void UpdateTryLists( SpawnGridLoc *refLoc );
	int CalculateIndex(int x, int y);

	SpawnGridLoc *GridLoc(int x, int y);

private:
	static const int iMaxX = EXTEND_FROM_CENTER;
	static const int iMaxY = EXTEND_FROM_CENTER;
	static const int iMinY = -EXTEND_FROM_CENTER;
	static const int iMinX = -EXTEND_FROM_CENTER;

	int iCurMinX;
	int iCurMinY;

	int m_iSpawnsDesired;
	int m_iSpawnsCreated;
	bool m_bWaterOk;

	Vector m_vecCenter;
	Vector TRACE_HULL_MIN;
	Vector TRACE_HULL_MAX;

	SpawnGridLoc m_arrSpawnGrid[DIAMETER*DIAMETER];

	CUtlVector<SpawnGridLoc*> m_arrCanRight, m_arrCanLeft, m_arrCanFront;
	QAngle m_angles;
	const char *m_pszPointName;
};

#pragma once

#ifdef WITH_DATA_COLLECTION

#include "vstdlib/jobthread.h"

class CDataManager : public CAutoGameSystemPerFrame
{
public:
	CDataManager( char const *name );

public:
	virtual void LevelInitPostEntity();
	virtual void FrameUpdatePostEntityThink();
	virtual void LevelShutdownPostEntity();

	virtual void SavePositions();

	bool IsSendingData();

	const CUtlVector<Vector>& GetPlayerPositions() const { return m_avecPlayerPositions; }
	void ClearData();

private:
	float              m_flNextPositionsUpdate;
	CUtlVector<Vector> m_avecPlayerPositions;

	CJob* m_pSendData;
};

#endif

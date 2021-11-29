#include "cbase.h"

class CRatraceCheckpoint : public CPointEntity
{
public:
	DECLARE_CLASS(CRatraceCheckpoint, CPointEntity);

	//DECLARE_DATADESC();

	// Constructor
	CRatraceCheckpoint ()
	{
	}
	
};
LINK_ENTITY_TO_CLASS( ratrace_checkpoint, CRatraceCheckpoint );
#include "cbase.h"

#include "triggers.h"

#include "sdk_player.h"
#include "da_briefcase.h"
#include "sdk_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CSuperfallTrigger : public CBaseTrigger
{
public:
	CSuperfallTrigger()
	{
	}

	DECLARE_CLASS(CSuperfallTrigger, CBaseTrigger);

	void Spawn(void);
	void Touch(CBaseEntity *pOther);

	DECLARE_DATADESC();

};

BEGIN_DATADESC(CSuperfallTrigger)


END_DATADESC()


LINK_ENTITY_TO_CLASS(trigger_superfall, CSuperfallTrigger);


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CSuperfallTrigger::Spawn(void)
{
	BaseClass::Spawn();

	InitTrigger();

	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
}

void CSuperfallTrigger::Touch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer())
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(pOther);
		if (pPlayer){
			pPlayer->m_Shared.StartSuperfallDiving();
		}
	}
}

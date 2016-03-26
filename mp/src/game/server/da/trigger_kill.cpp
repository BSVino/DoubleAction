#include "cbase.h"

#include "triggers.h"

#include "da_player.h"
#include "da_briefcase.h"
#include "da_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTriggerKill : public CBaseTrigger
{
public:
	CTriggerKill()
	{
	}

	DECLARE_CLASS( CTriggerKill, CBaseTrigger );

	void Spawn( void );
	void Touch( CBaseEntity *pOther );

	DECLARE_DATADESC();

	int      m_bitsDamageInflict;
	string_t m_sKillerPrintString;
};

BEGIN_DATADESC( CTriggerKill )

	// Fields
	DEFINE_KEYFIELD( m_bitsDamageInflict, FIELD_INTEGER, "damagetype" ),
	DEFINE_KEYFIELD( m_sKillerPrintString, FIELD_STRING, "killerstring" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_kill, CTriggerKill );


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerKill::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();

	SetNextThink( TICK_NEVER_THINK );
	SetThink( NULL );
}

void CTriggerKill::Touch( CBaseEntity *pOther )
{
	if (pOther->IsPlayer())
	{
		CTakeDamageInfo info( this, this, 10000, m_bitsDamageInflict );
		pOther->TakeDamage( info );
		ToDAPlayer(pOther)->PlayerDeathThink();
		ToDAPlayer(pOther)->SetKilledByString(m_sKillerPrintString);
	}
	else if (dynamic_cast<CBriefcase*>(pOther))
	{
		// This code doesn't work, the briefcase needs some sort of flag
		// to be triggered and I can't be bothered to figure it out right now.
		AssertMsg(false, "Untested code");
		DAGameRules()->CleanupMiniObjective_Briefcase();
		DAGameRules()->SetupMiniObjective_Briefcase();
	}
}

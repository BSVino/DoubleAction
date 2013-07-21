#include "cbase.h"
#include "sdk_bot.h"

#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CSDKBot::AcquireEnemy()
{
	float minDist = FLT_MAX;
	bool Success = false;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer && pPlayer != NULL && pPlayer->IsAlive() && pPlayer != this ) // acquiring only human players
		{
			float dist = (GetLocalOrigin() - pPlayer->GetLocalOrigin()).Length();

			if( dist < minDist )
			{
				minDist = dist;
				hEnemy.Set(pPlayer);
				Success = true;
			}
		}
	}

	return Success;
}

void CSDKBot::Attack( CUserCmd &cmd )
{
	// EXCEPTIONS
	if( !m_bEnemyOnSights || !m_bInRangeToAttack || m_flNextBotAttack > gpGlobals->curtime )
		return;

	cmd.buttons |= IN_ATTACK;
	m_flNextBotAttack = gpGlobals->curtime + 0.75f;
}

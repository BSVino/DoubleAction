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

		if ( pPlayer && pPlayer != NULL && pPlayer->IsAlive() && pPlayer != this )
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
	if (!m_bEnemyOnSights)
		return;

	if (!GetActiveSDKWeapon())
		return;

	if (m_bInRangeToAttack && gpGlobals->curtime > m_flNextBotMeleeAttack )
	{
		if (GetActiveSDKWeapon()->IsMeleeWeapon())
		{
			cmd.buttons |= (random->RandomInt(0, 1) == 0)?IN_ATTACK:IN_ATTACK2;
			m_flNextBotMeleeAttack = gpGlobals->curtime + 0.75f;
		}
		else
		{
			cmd.buttons |= IN_ATTACK2;
			m_flNextBotMeleeAttack = gpGlobals->curtime + 0.75f;
		}
	}
	else
	{
		if (GetActiveSDKWeapon()->IsFullAuto())
			cmd.buttons |= IN_ATTACK;
		else
		{
			if (random->RandomInt(0, 1))
				cmd.buttons |= IN_ATTACK;
		}
	}
}

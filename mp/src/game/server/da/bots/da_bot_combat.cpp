#include "cbase.h"
#include "da_bot.h"
#include "da_gamerules.h"

#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CDABot::AcquireEnemy()
{
	float minDist = FLT_MAX;
	bool Success = false;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CDAPlayer *pPlayer = ToDAPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (pPlayer == this)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if (DAGameRules()->PlayerRelationship(this, pPlayer) == GR_TEAMMATE)
			continue;

		float dist = (GetLocalOrigin() - pPlayer->GetLocalOrigin()).Length();

		if( dist < minDist )
		{
			minDist = dist;
			hEnemy.Set(pPlayer);
			Success = true;
		}
	}

	return Success;
}

void CDABot::Attack( CUserCmd &cmd )
{
	if (!m_bEnemyOnSights)
		return;

	if (!GetActiveDAWeapon())
		return;

	if (m_bInRangeToAttack && gpGlobals->curtime > m_flNextBotMeleeAttack )
	{
		if (GetActiveDAWeapon()->IsMeleeWeapon())
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
		if (GetActiveDAWeapon()->IsFullAuto())
		{
			if (GetActiveDAWeapon()->Clip1())
				cmd.buttons |= IN_ATTACK;
		}
		else
		{
			RandomSeed(gpGlobals->curtime*1000); // Without this it stops being very random for some reason.
			if (RandomInt(0, 1))
				cmd.buttons |= IN_ATTACK;
		}
	}

	// Try to throw a grenade.
	if (random->RandomInt(0, 1))
		cmd.buttons |= IN_ALT2;

	// Use slow motion
	ActivateSlowMo();
}

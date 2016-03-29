#include "cbase.h"
#include "weapon_akimbobase.h"

#if defined(CLIENT_DLL)
	#include "c_da_player.h"
	#include "prediction.h"
#else
	#include "da_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(AkimboBase, DT_AkimboBase)
BEGIN_NETWORK_TABLE(CAkimboBase, DT_AkimboBase)
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()
#ifndef CLIENT_DLL
	BEGIN_DATADESC(CAkimboBase)
	END_DATADESC()
#else
	BEGIN_PREDICTION_DATA(CAkimboBase)
	END_PREDICTION_DATA()
#endif
LINK_ENTITY_TO_CLASS(weapon_akimbobase, CAkimboBase);

static bool akimbo_reload(CAkimboBase *self)
{
	CBaseCombatCharacter *owner = self->GetOwner();
	if (!owner)
	{
		return false;
	}
	int sum = self->rightclip + self->leftclip;
	int total = owner->GetAmmoCount(self->m_iPrimaryAmmoType);
	int delta = min(self->GetMaxClip1() - sum, total);
	if (delta != 0)
	{
		return true;
	}
	return false;
}

CAkimboBase::CAkimboBase()
{
	reload_delegate = (delegate_t)akimbo_reload;
	shootright = false;
}

Activity CAkimboBase::ActivityOverride(Activity baseAct, bool *pRequired)
{
	// Remap baseAct to the approrpiate left/right akimbo firing animation.
	Activity act = baseAct;
	if (ACT_DA_PRIMARYATTACK <= act && act <= ACT_DA_PRIMARYATTACK_ROLL)
	{
		unsigned ndx = act - ACT_DA_PRIMARYATTACK;
		if (shootright)
			act = (Activity)(ACT_DA_AKIMBO_RIGHT + ndx);
		else
			act = (Activity)(ACT_DA_AKIMBO_LEFT + ndx);
	}
	return BaseClass::ActivityOverride(act, pRequired);
}

int CAkimboBase::GetTracerAttachment(void)
{
	if (shootright)
		return 2;

	return 1;
}

Activity CAkimboBase::GetIdleActivity(void)
{
#if 0
	if (rightclip > 0 && leftclip > 0)
		return ACT_VM_IDLE;
	if (rightclip > 0 && leftclip == 0)
		return ACT_VM_IDLE_EMPTY_LEFT;
#endif
	return ACT_VM_IDLE_EMPTY;
}

bool CAkimboBase::Deploy()
{
	// Transfer iClip1 of single pistol to rightclip
	CWeaponDABase *from = GetPlayerOwner()->m_hSwitchFrom;
	DAWeaponID eFromId = DA_WEAPON_NONE;
	DAWeaponID eSingleId = AliasToWeaponID(GetSDKWpnData().m_szSingle);
	if (from)
		eFromId = from->GetWeaponID();
	if (eFromId == eSingleId)
	{
		rightclip = from->m_iClip1;
		m_iClip1 = leftclip + rightclip;
	}
	return BaseClass::Deploy();
}

bool CAkimboBase::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Transfer rightclip into iClip1 of single pistol
	CWeaponDABase *to = (CWeaponDABase *)pSwitchingTo;
	DAWeaponID eToId = DA_WEAPON_NONE;
	DAWeaponID eSingleId = AliasToWeaponID(GetSDKWpnData().m_szSingle);
	if (to)
		eToId = to->GetWeaponID();
	if (eToId == eSingleId)
	{
		to->m_iClip1 = rightclip;
	}
	return BaseClass::Holster(pSwitchingTo);
}

void CAkimboBase::PrimaryAttack(void)
{
	Activity act;
	if (rightclip <= 0 && leftclip <= 0) 
	{
		Reload();
		return;
	}
	CDAPlayer *pPlayer = GetPlayerOwner();
	if (!pPlayer)
	{
		return;
	}
	if (pPlayer->GetShotsFired() > 0) 
	{
		return;
	}

	if (shootright)
	{
		act = ACT_VM_SECONDARYATTACK;
#if 0
		if (rightclip == 1)
			act = ACT_VM_DRYFIRE;
#endif
		rightclip--;
	}
	else
	{
		act = ACT_VM_PRIMARYATTACK;
#if 0
		if (leftclip == 1)
			act = ACT_VM_DRYFIRE_LEFT;
#endif
		leftclip--;
	}

	SendWeaponAnim(act);
	m_iClip1 = leftclip + rightclip;
	FinishAttack(pPlayer);

	if (rightclip > 0 && leftclip > 0)
		shootright = !shootright;
	else if (rightclip > 0)
		shootright = true;
	else
		shootright = false;
}

void CAkimboBase::FinishReload(void)
{
	CDAPlayer *owner = GetPlayerOwner();
	int clipsize = GetMaxClip1() / 2;
	int total = owner->GetAmmoCount(m_iPrimaryAmmoType);
	if (!owner)
	{
		return;
	}
	int take = min(clipsize * 2 - (rightclip + leftclip), total);
	if (!owner->IsStyleSkillActive(SKILL_MARKSMAN))
	{
		owner->RemoveAmmo(take, m_iPrimaryAmmoType);
	}
	int r = min(clipsize - rightclip, take);
	if (r != 0)
	{
		rightclip += r;
		take -= r;
	}
	int l = min(clipsize - leftclip, take);
	if (l != 0)
	{
		leftclip += l;
		take -= l;
	}
	m_iClip1 = rightclip + leftclip;
}

void CAkimboBase::GiveDefaultAmmo(void)
{
	leftclip = rightclip = GetMaxClip1() / 2;
	m_iClip1 = rightclip + leftclip;
}

void CAkimboBase::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
#ifdef GAME_DLL
	CDAPlayer *pl = ToDAPlayer(pNewOwner);
	Assert(pl != NULL);
	if (!pl)
	{
		return;
	}
	// Ensure player has a single with their akimbos
	char name[32];
	Q_snprintf(name, sizeof(name), "weapon_%s", GetSDKWpnData().m_szSingle);
	CWeaponDABase *single = pl->FindWeapon(AliasToWeaponID(name));
	if (!single)
	{
		// Give them a single too
		pl->GiveNamedItem(name);
	}
	GiveDefaultAmmo();
	BaseClass::OnPickedUp(pNewOwner);
#endif
}

void CAkimboBase::CheckReload(void)
{
	BaseClass::CheckReload();
}

int CAkimboBase::GetMaxClip1() const
{
	DAWeaponID eSingleID = AliasToWeaponID(GetSDKWpnData().m_szSingle);

	CSDKWeaponInfo* pSingleInfo = CSDKWeaponInfo::GetWeaponInfo(eSingleID);

	Assert(pSingleInfo);
	if (!pSingleInfo)
		return BaseClass::GetMaxClip1();

	return pSingleInfo->iMaxClip1 * 2;
}

// For weight purposes an akimbo weighs the same as a single.
// This is so that the second entity in the player's inventory just adds another single's worth of weight.
int CAkimboBase::GetWeight() const
{
	DAWeaponID eSingleID = AliasToWeaponID(GetSDKWpnData().m_szSingle);

	CSDKWeaponInfo* pSingleInfo = CSDKWeaponInfo::GetWeaponInfo(eSingleID);

	Assert(pSingleInfo);
	if (!pSingleInfo)
		return BaseClass::GetWeight();

	return pSingleInfo->iWeight;
}

const Vector CAkimboBase::GetShootPosition(CDAPlayer* pShooter)
{
	Assert(pShooter);
	if (!pShooter)
		return Vector(0, 0, 0);

	Vector vecPosition, vecRight;
	pShooter->EyePositionAndVectors(&vecPosition, NULL, &vecRight, NULL);

	return vecPosition + vecRight * (shootright ? 4 : -4);
}

#include "cbase.h"
#include "weapon_akimbobase.h"

#if defined (CLIENT_DLL)
	#include "c_sdk_player.h"
	#include "prediction.h"
#else
	#include "sdk_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(Akimbobase, DT_Akimbobase)
BEGIN_NETWORK_TABLE(CAkimbobase, DT_Akimbobase)
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE()
#ifndef CLIENT_DLL
	BEGIN_DATADESC(CAkimbobase)
	END_DATADESC()
#else
	BEGIN_PREDICTION_DATA(CAkimbobase)
	END_PREDICTION_DATA()
#endif
LINK_ENTITY_TO_CLASS(weapon_akimbobase, CAkimbobase);

static bool 
akimbo_reload (CAkimbobase *self)
{
	CBaseCombatCharacter *owner = self->GetOwner ();
	if (!owner)
	{
		return false;
	}
	int sum = (self->rightclip + self->leftclip);
	int total = owner->GetAmmoCount (self->m_iPrimaryAmmoType);
	int delta = min ((self->GetMaxClip1 ()<<1) - sum, total);
	if (delta != 0)
	{
		return true;
	}
	return false;
}
CAkimbobase::CAkimbobase ()
{
	reload_delegate = (delegate_t)akimbo_reload;
	leftclip = GetMaxClip1 ();
	rightclip = GetMaxClip1 ();
	shootright = false;
}
Activity 
CAkimbobase::ActivityOverride (Activity baseAct, bool *pRequired)
{/*Remap baseAct to the approrpiate left/right akimbo firing animation.*/
	Activity act = baseAct;
	if (ACT_DAB_PRIMARYATTACK <= act && act <= ACT_DAB_PRIMARYATTACK_ROLL)
	{
		unsigned ndx = act - ACT_DAB_PRIMARYATTACK;
		if (!shootright) act = (Activity)(ACT_DAB_AKIMBO_LEFT + ndx);
		else act = (Activity)(ACT_DAB_AKIMBO_RIGHT + ndx);
	}
	return BaseClass::ActivityOverride (act, pRequired);
}
int
CAkimbobase::GetTracerAttachment (void)
{
	if (shootright) return 2;
	return 1;
}
Activity 
CAkimbobase::GetIdleActivity (void)
{
	//if (rightclip > 0 && leftclip > 0) return ACT_VM_IDLE;
	//if (rightclip > 0 && leftclip == 0) return ACT_VM_IDLE_EMPTY_LEFT;
	return ACT_VM_IDLE_EMPTY;
}
bool 
CAkimbobase::Deploy ()
{/*Transfer iClip1 of single pistol to rightclip*/
	CWeaponSDKBase *from = GetPlayerOwner ()->switchfrom;
	SDKWeaponID id1 = SDK_WEAPON_NONE;
	SDKWeaponID id2 = AliasToWeaponID (GetSDKWpnData ().akimbo);
	if (from) id1 = from->GetWeaponID ();
	if (id1 == id2)
	{
		rightclip = from->m_iClip1;
		m_iClip1 = leftclip + rightclip;
	}
	return BaseClass::Deploy ();
}
bool 
CAkimbobase::Holster (CBaseCombatWeapon *pSwitchingTo)
{/*Transfer rightclip into iClip1 of single pistol*/
	CWeaponSDKBase *to = (CWeaponSDKBase *)pSwitchingTo;
	SDKWeaponID id1 = SDK_WEAPON_NONE;
	SDKWeaponID id2 = AliasToWeaponID (GetSDKWpnData ().akimbo);
	if (to) id1 = to->GetWeaponID ();
	if (id1 == id2)
	{
		to->m_iClip1 = rightclip;
	}
	return BaseClass::Holster (pSwitchingTo);
}
void 
CAkimbobase::PrimaryAttack (void)
{
	Activity act;
	if (rightclip <= 0 && leftclip <= 0) 
	{
		Reload ();
		return;
	}
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if (!pPlayer)
	{
		return;
	}
	if (pPlayer->GetShotsFired() > 0) 
	{
		return;
	}
	if (rightclip > 0 && leftclip > 0) shootright = (bool)(shootright^1);
	else if (rightclip > 0) shootright = true;
	else shootright = false;
	if (shootright)
	{
		//if (rightclip != 1) act = ACT_VM_PRIMARYATTACK;
		//else act = ACT_VM_DRYFIRE_LEFT;
		act = ACT_VM_PRIMARYATTACK;
		rightclip--;
	}
	else
	{
		//if (leftclip != 1) act = ACT_VM_SECONDARYATTACK;
		//else act = ACT_VM_DRYFIRE;
		act = ACT_VM_SECONDARYATTACK;
		leftclip--;
	}
	SendWeaponAnim (act);
	m_iClip1 = leftclip + rightclip;
	finishattack (pPlayer);
}
void
CAkimbobase::FinishReload (void)
{
	CSDKPlayer *owner = GetPlayerOwner();
	int clipsize = GetMaxClip1 ();
	int total = owner->GetAmmoCount (m_iPrimaryAmmoType);
	if (!owner)
	{
		return;
	}
	int take = min ((clipsize<<1) - (rightclip + leftclip), total);
	if (!owner->IsStyleSkillActive (SKILL_MARKSMAN))
	{
		owner->RemoveAmmo (take, m_iPrimaryAmmoType);
	}
	int r = min (clipsize - rightclip, take);
	if (r != 0)
	{
		rightclip += r;
		take -= r;
	}
	int l = min (clipsize - leftclip, take);
	if (l != 0)
	{
		leftclip += l;
		take -= l;
	}
	m_iClip1 = rightclip + leftclip;
}
void 
CAkimbobase::GiveDefaultAmmo (void)
{
	rightclip = GetMaxClip1 ();
	leftclip = GetMaxClip1 ();
	m_iClip1 = rightclip + leftclip;
}
void
CAkimbobase::OnPickedUp (CBaseCombatCharacter *pNewOwner)
{
#ifdef GAME_DLL
	CSDKPlayer *pl;
	CWeaponSDKBase *single;
	char name[32];

	pl = ToSDKPlayer (pNewOwner);
	Assert (pl != NULL);
	if (!pl)
	{
		return;
	}
	/*Ensure player has a single with their akimbos*/
	Q_snprintf (name, sizeof (name), "weapon_%s", GetSDKWpnData ().akimbo);
	single = pl->findweapon (AliasToWeaponID (name));
	if (!single)
	{/*Give them a single too*/
		pl->GiveNamedItem (name);
	}
	GiveDefaultAmmo ();
	BaseClass::OnPickedUp (pNewOwner);
#endif
}
void 
CAkimbobase::CheckReload (void)
{
	BaseClass::CheckReload ();
}
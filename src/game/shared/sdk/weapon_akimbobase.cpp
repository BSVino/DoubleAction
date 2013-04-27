#include "cbase.h"
#include "weapon_akimbobase.h"

IMPLEMENT_NETWORKCLASS_ALIASED(Akimbobase, DT_Akimbobase)
BEGIN_NETWORK_TABLE(CAkimbobase, DT_Akimbobase)
END_NETWORK_TABLE()
BEGIN_PREDICTION_DATA(CAkimbobase)
END_PREDICTION_DATA()
#ifndef CLIENT_DLL
	BEGIN_DATADESC(CAkimbobase)
	END_DATADESC()
#endif
LINK_ENTITY_TO_CLASS(weapon_akimbobase, CAkimbobase);
int
CAkimbobase::GetTracerAttachment (void)
{
	if ((m_iClip1&1) == 0) return 1;
	return 2;
}
Activity 
CAkimbobase::GetIdleActivity (void)
{
	if (m_iClip1 == 1) return ACT_VM_IDLE_EMPTY_LEFT;
	return ACT_VM_IDLE;
}
Activity 
CAkimbobase::GetPrimaryAttackActivity (void)
{
	if ((m_iClip1&1) == 0)
	{
		if (m_iClip1 == 1) return ACT_VM_DRYFIRE;
		return ACT_VM_PRIMARYATTACK;
	}
	if (m_iClip1 == 2) return ACT_VM_DRYFIRE_LEFT;
	return ACT_VM_SECONDARYATTACK;
}
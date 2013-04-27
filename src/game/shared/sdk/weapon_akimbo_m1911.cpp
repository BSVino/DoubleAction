#include "cbase.h"
#include "weapon_sdkbase.h"
#include "weapon_akimbobase.h"

#if defined( CLIENT_DLL )
	#define CAkimbo_m1911 C_Akimbo_m1911
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif
class CAkimbo_m1911 : public CAkimbobase
{
public:
	DECLARE_CLASS(CAkimbo_m1911, CWeaponSDKBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	virtual SDKWeaponID GetWeaponID (void) const 
	{
		return SDK_WEAPON_AKIMBO_M1911;
	}
	virtual int GetFireMode () const
	{
		return FM_SEMIAUTOMATIC;
	}
};

IMPLEMENT_NETWORKCLASS_ALIASED(Akimbo_m1911, DT_Akimbo_m1911)

BEGIN_NETWORK_TABLE(CAkimbo_m1911, DT_Akimbo_m1911)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CAkimbo_m1911)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_akimbo_m1911, CAkimbo_m1911);
PRECACHE_WEAPON_REGISTER(weapon_akimbo_m1911);

acttable_t CAkimbo_m1911::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_M1911,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_M1911,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_M1911,					false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_M1911,				false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_M1911,			false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_M1911,              false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_M1911,               false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_M1911,                false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_M1911,             false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_M1911,         false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_M1911,			false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_M1911,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_M1911,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_M1911,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_M1911,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_M1911,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_M1911,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_M1911,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_M1911,				false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_M1911,				false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_M1911,                    false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_M1911,             false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_M1911,              false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_M1911,              false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_M1911,               false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_M1911,               false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_M1911,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_M1911,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_M1911,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_M1911,						false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_M1911,					false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_M1911,					false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_M1911,						false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_M1911,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_M1911,					false },
	{ ACT_DAB_DIVESLIDE,                ACT_DAB_DIVESLIDE_M1911,                false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_M1911,           false },
};
IMPLEMENT_ACTTABLE(CAkimbo_m1911);
#include "cbase.h"
#include "weapon_sdkbase.h"
#include "weapon_akimbobase.h"

#if defined( CLIENT_DLL )
	#define CAkimbo_p99 C_Akimbo_p99
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif
class CAkimbo_p99 : public CAkimbobase
{
public:
	DECLARE_CLASS(CAkimbo_p99, CWeaponSDKBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	virtual SDKWeaponID GetWeaponID (void) const 
	{
		return SDK_WEAPON_AKIMBO_P99;
	}
	virtual int GetFireMode () const
	{
		return FM_SEMIAUTOMATIC;
	}
};

IMPLEMENT_NETWORKCLASS_ALIASED(Akimbo_p99, DT_Akimbo_p99)

BEGIN_NETWORK_TABLE(CAkimbo_p99, DT_Akimbo_p99)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CAkimbo_p99)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_akimbo_p99, CAkimbo_p99);
PRECACHE_WEAPON_REGISTER(weapon_akimbo_p99);

acttable_t CAkimbo_p99::m_acttable[] = 
{
	{ ACT_DAB_STAND_IDLE,				ACT_DAB_STAND_IDLE_P99,				false },
	{ ACT_DAB_WALK_IDLE,				ACT_DAB_WALK_IDLE_P99,				false },
	{ ACT_DAB_RUN_IDLE,					ACT_DAB_RUN_IDLE_P99,				false },
	{ ACT_DAB_CROUCH_IDLE,				ACT_DAB_CROUCH_IDLE_P99,			false },
	{ ACT_DAB_CROUCHWALK_IDLE,			ACT_DAB_CROUCHWALK_IDLE_P99,		false },
	{ ACT_DAB_STAND_READY,              ACT_DAB_STAND_READY_P99,            false },
	{ ACT_DAB_WALK_READY,               ACT_DAB_WALK_READY_P99,             false },
	{ ACT_DAB_RUN_READY,                ACT_DAB_RUN_READY_P99,              false },
	{ ACT_DAB_CROUCH_READY,             ACT_DAB_CROUCH_READY_P99,           false },
	{ ACT_DAB_CROUCHWALK_READY,         ACT_DAB_CROUCHWALK_READY_P99,       false },
	{ ACT_DAB_PRONECHEST_IDLE,			ACT_DAB_PRONECHEST_IDLE_P99,		false },
	{ ACT_DAB_PRONEBACK_IDLE,			ACT_DAB_PRONEBACK_IDLE_P99,			false },
	{ ACT_DAB_CRAWL_IDLE,				ACT_DAB_CRAWL_IDLE_P99,				false },
	{ ACT_DAB_PRIMARYATTACK,			ACT_DAB_PRIMARYATTACK_P99,			false },
	{ ACT_DAB_PRIMARYATTACK_CROUCH,		ACT_DAB_PRIMARYATTACK_CROUCH_P99,	false },
	{ ACT_DAB_PRIMARYATTACK_PRONE,		ACT_DAB_PRIMARYATTACK_PRONE_P99,	false },
	{ ACT_DAB_PRIMARYATTACK_SLIDE,		ACT_DAB_PRIMARYATTACK_SLIDE_P99,	false },
	{ ACT_DAB_PRIMARYATTACK_DIVE,		ACT_DAB_PRIMARYATTACK_DIVE_P99,		false },
	{ ACT_DAB_PRIMARYATTACK_ROLL,		ACT_DAB_PRIMARYATTACK_ROLL_P99,		false },
	{ ACT_DAB_RELOAD,					ACT_DAB_RELOAD_P99,					false },
	{ ACT_DAB_RELOAD_CROUCH,			ACT_DAB_RELOAD_CROUCH_P99,			false },
	{ ACT_DAB_RELOAD_PRONE,				ACT_DAB_RELOAD_PRONE_P99,			false },
	{ ACT_DAB_RELOAD_SLIDE,				ACT_DAB_RELOAD_SLIDE_P99,			false },
	{ ACT_DAB_BRAWL,                    ACT_DAB_BRAWL_P99,                  false },
	{ ACT_DAB_BRAWL_CROUCH,             ACT_DAB_BRAWL_CROUCH_P99,           false },
	{ ACT_DAB_BRAWL_PRONE,              ACT_DAB_BRAWL_PRONE_P99,            false },
	{ ACT_DAB_BRAWL_SLIDE,              ACT_DAB_BRAWL_SLIDE_P99,            false },
	{ ACT_DAB_BRAWL_DIVE,               ACT_DAB_BRAWL_DIVE_P99,             false },
	{ ACT_DAB_BRAWL_ROLL,               ACT_DAB_BRAWL_ROLL_P99,             false },
	{ ACT_DAB_JUMP_START,				ACT_DAB_JUMP_START_P99,				false },
	{ ACT_DAB_JUMP_FLOAT,				ACT_DAB_JUMP_FLOAT_P99,				false },
	{ ACT_DAB_JUMP_LAND,				ACT_DAB_JUMP_LAND_P99,				false },
	{ ACT_DAB_DIVE,						ACT_DAB_DIVE_P99,					false },
	{ ACT_DAB_DIVEFALL,					ACT_DAB_DIVEFALL_P99,				false },
	{ ACT_DAB_DIVEROLL,					ACT_DAB_DIVEROLL_P99,				false },
	{ ACT_DAB_ROLL,						ACT_DAB_ROLL_P99,					false },
	{ ACT_DAB_SLIDESTART,				ACT_DAB_SLIDESTART_P99,				false },
	{ ACT_DAB_SLIDE,					ACT_DAB_SLIDE_P99,					false },
	{ ACT_DAB_DIVESLIDE,                ACT_DAB_DIVESLIDE_P99,              false },
	{ ACT_DAB_PRONE_TO_STAND,           ACT_DAB_PRONE_TO_STAND_P99,         false },
};
IMPLEMENT_ACTTABLE(CAkimbo_p99);
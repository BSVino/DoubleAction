#include "cbase.h"
#include "sdk_player.h"

enum powerup_e
{
	POWERUP_HEALTH = 0,
	POWERUP_REGEN,
	POWERUP_RAMBO,
	POWERUP_SLOWMO,
	POWERUP_GRENADE,
	POWERUP_DOUBLEACTION,
	MAX_POWERUP
};
static const char *models[] =
{/*Must be given in the same order as powerup_e*/
	"models/gibs/airboat_broken_engine.mdl",
	"models/gibs/airboat_broken_engine.mdl",
	"models/gibs/airboat_broken_engine.mdl",
	"models/gibs/airboat_broken_engine.mdl",
	"models/gibs/airboat_broken_engine.mdl",
	"models/gibs/airboat_broken_engine.mdl",
};
class Powerup : public CBaseAnimating
{
public:
	DECLARE_CLASS(Powerup, CBaseAnimating);
	DECLARE_DATADESC();

	void Spawn (void);
	void Precache (void);
	void pickup (CBaseEntity *other);
	void manifest (void);
public:
	bool random;
	int type;
	float delay;
};
LINK_ENTITY_TO_CLASS(da_powerup, Powerup);
BEGIN_DATADESC(Powerup)
	DEFINE_KEYFIELD(random, FIELD_BOOLEAN, "random"),
	DEFINE_KEYFIELD(type, FIELD_INTEGER, "type"),
	DEFINE_KEYFIELD(delay, FIELD_FLOAT, "delay"),
	DEFINE_ENTITYFUNC(pickup),
	DEFINE_THINKFUNC(manifest),
END_DATADESC()

void 
Powerup::Precache (void)
{
	if (random)
	{
		int i;
		for (i = 0; i < MAX_POWERUP; i++) PrecacheModel (models[i]);
		return;
	}
	Assert (0 <= type && type < MAX_POWERUP);
	PrecacheModel (models[type]);
}
void 
Powerup::Spawn (void)
{
	if (type < 0 || MAX_POWERUP <= type)
	{
		Msg ("Bad powerup type; removing...\n");
		Remove ();
	}
	Precache ();
	manifest ();
}
void
Powerup::manifest (void)
{
	if (random)
	{
		type = RandomInt (0, MAX_POWERUP);
	}
	SetModel (models[type]);
	RemoveEffects (EF_NODRAW);
	SetSolid (SOLID_BBOX);
	AddSolidFlags (FSOLID_TRIGGER);
	SetMoveType (MOVETYPE_NONE);
	UTIL_SetSize (this, -Vector (32,32,32), Vector (32,32,32));

	SetThink (NULL);
	SetTouch (&Powerup::pickup);

	CPASFilter filter (GetAbsOrigin ());
	filter.UsePredictionRules ();
	EmitSound (filter, entindex (), "Item.Materialize");
}
void
Powerup::pickup (CBaseEntity *other)
{
	const int MAXENTS = 64;
	CSDKPlayer *ents[MAXENTS];
	CSDKPlayer *taker;
	int len;
	int i;

	if (!other->IsPlayer ())
	{
		return;
	}
	/*Fire off 'Denied!' to relevant players; award taker style for it*/
	taker = (CSDKPlayer *)other;
	len = UTIL_EntitiesInSphere ((CBaseEntity **)ents, 
								 MAXENTS, 
								 GetAbsOrigin (), 
								 96, 
								 FL_CLIENT);
	for (i = 0; i < len; i++)
	{
		if (ents[i] != taker) 
		{
			Vector org, forward;

			AngleVectors (ents[i]->EyeAngles (), &forward);
			VectorSubtract (ents[i]->GetAbsOrigin (), GetAbsOrigin (), org);
			VectorNormalize (org);
			if (fabs (DotProduct (forward, org)) < 0.7)
			{
				/*ents[i]->SendNotice (NOTICE_DENIED);*/
			}
		}
		else if (len != 1) 
		{
			taker->SendAnnouncement (ANNOUNCEMENT_COOL, STYLE_POINT_STYLISH);
		}
	}
	CPASFilter filter (GetAbsOrigin ());
	filter.UsePredictionRules ();
	EmitSound (filter, entindex (), "HealthVial.Touch");
	/*Impart pickup bonus*/
	switch (type)
	{
	case POWERUP_HEALTH: taker->TakeHealth (100, DMG_GENERIC); break;
	case POWERUP_REGEN: Msg ("Regen\n"); break;
	case POWERUP_RAMBO: Msg ("Rambo mode\n"); break;
	case POWERUP_SLOWMO: taker->GiveSlowMo (5); break;
	case POWERUP_GRENADE: taker->GiveAmmo (1, "grenade", true); break;
	case POWERUP_DOUBLEACTION: Msg ("Double action\n"); break;
	default: /*Shouldn't get here*/ break;
	}
	/*Go inactive until respawn timer expires*/
	AddEffects (EF_NODRAW);
	AddSolidFlags (FSOLID_NOT_SOLID);
	SetTouch (NULL);
	SetThink (&Powerup::manifest);
	SetNextThink (gpGlobals->curtime + delay);
}
void powerup(const CCommand &args)
{
	CBasePlayer *player = ToBasePlayer (UTIL_GetCommandClient ()); 
	if (!player)
	{
		return;
	}
	Vector org;
	Vector forward;

	AngleVectors (player->EyeAngles (), &forward);
	VectorMA (player->GetAbsOrigin () + Vector (0, 0, 36), 80, forward, org);

	Powerup *p = (Powerup *)CreateEntityByName ("da_powerup");
	p->random = true;
	p->type = 0;
	p->delay = 10;
	p->Spawn ();
	p->SetAbsOrigin (org);
}
static ConCommand test_powerup ("test_powerup", powerup);
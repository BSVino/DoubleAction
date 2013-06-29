//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// the 1 / 2 / 3 respectively are all identical in our template mod to start, I've made the base ones (pc_class1, pc_class2, pc_class3) and then duplicated them for the teams.
//Tony;  for our template we have two versions.
#if defined ( SDK_USE_PLAYERCLASSES ) && defined ( SDK_USE_TEAMS )
const char *pszTeamBlueClasses[] = 
{
	"blue_class1",
	"blue_class2",
	"blue_class3",
	NULL
};

const char *pszTeamRedClasses[] = 
{
	"red_class1",
	"red_class2",
	"red_class3",
	NULL
};
ConVar	mp_limit_blue_class1(		"mp_limit_blue_class1", "-1", FCVAR_REPLICATED, "Class limit for Blue class 1" );
ConVar	mp_limit_blue_class2(		"mp_limit_blue_class2", "-1", FCVAR_REPLICATED, "Class limit for Blue class 2" );
ConVar	mp_limit_blue_class3(		"mp_limit_blue_class3", "-1", FCVAR_REPLICATED, "Class limit for Blue class 3" );

ConVar	mp_limit_red_class1(		"mp_limit_red_class1", "-1", FCVAR_REPLICATED, "Class limit for Red class 1" );
ConVar	mp_limit_red_class2(		"mp_limit_red_class2", "-1", FCVAR_REPLICATED, "Class limit for Red class 2" );
ConVar	mp_limit_red_class3(		"mp_limit_red_class3", "-1", FCVAR_REPLICATED, "Class limit for Red class 3" );

//Tony; not using teams, but we are using classes
#elif defined ( SDK_USE_PLAYERCLASSES ) && !defined( SDK_USE_TEAMS )
const char *pszPlayerClasses[] =
{
	"pc_class1",
	"pc_class2",
	"pc_class3",
	NULL
};
ConVar	mp_limit_pc_class1(		"mp_limit_pc_class1", "-1", FCVAR_REPLICATED, "Class limit for class 1" );
ConVar	mp_limit_pc_class2(		"mp_limit_pc_class2", "-1", FCVAR_REPLICATED, "Class limit for class 2" );
ConVar	mp_limit_pc_class3(		"mp_limit_pc_class3", "-1", FCVAR_REPLICATED, "Class limit for class 3" );
#endif

const char *pszTeamNames[] =
{
	"#DAB_Team_Unassigned",
	"#DAB_Team_Spectator",
	"#DAB_Team_Blue",
	"#DAB_Team_Red",
	"#DAB_Team_Deathmatch",
};

//Tony; We need to precache all possible player models that we're going to use
const char *pszPossiblePlayerModels[] =
{
	"models/player/playermale.mdl",
	"models/player/wish.mdl",
	NULL
};

// ----------------------------------------------------------------------------- //
// Global Weapon Definitions
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	"none",		// WEAPON_NONE
	"fal",		// SDK_WEAPON_FAL
	"mossberg", // SDK_WEAPON_MOSSBERG
	"vector",	// SDK_WEAPON_VECTOR
	"mp5k",		// SDK_WEAPON_MP5
	"m1911",	// SDK_WEAPON_M1911
	"beretta",		// SDK_WEAPON_P99
	"crowbar",	// SDK_WEAPON_CROWBAR
	"grenade",	// SDK_WEAPON_GRENADE
	"brawl",	// SDK_WEAPON_BRAWL
	"akimbo_m1911",
	"akimbo_beretta",
	NULL,		// WEAPON_NONE
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
SDKWeaponID AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return (SDKWeaponID)i;
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( SDKWeaponID id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}

static const char * s_SkillAliasInfo[] = 
{
	"none",			// SKILL_NONE
	"bouncer",      // SKILL_BOUNCER
	"athletic",     // SKILL_ATHLETIC
	"resilient",    // SKILL_RESILIENT
	"reflexes",     // SKILL_REFLEXES
	"marksman",     // SKILL_MARKSMAN
	"troll",        // SKILL_TROLL
	NULL,
};

SkillID AliasToSkillID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_SkillAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_SkillAliasInfo[i], alias ))
				return (SkillID)i;
	}

	return SKILL_NONE;
}

const char *SkillIDToAlias( SkillID id )
{
	if ( (id >= SKILL_MAX) || (id < 0) )
		return NULL;

	return s_SkillAliasInfo[id];
}

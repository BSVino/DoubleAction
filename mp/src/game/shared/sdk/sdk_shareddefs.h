//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SDK_SHAREDDEFS_H
#define SDK_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "da.h"

//=========================
// GAMEPLAY RELATED OPTIONS
//=========================
// NOTES: The Wizard automatically replaces these strings! If you extract the source as is, you will have to add the defines manually!
//
// Will your mod be team based?
// define SDK_USE_TEAMS
#define SDK_USE_TEAMS

//
// Do you use player classes?
// define SDK_USE_PLAYERCLASSES
//#define SDK_USE_PLAYERCLASSES

//================================
// PLAYER MOVEMENT RELATED OPTIONS
//================================

//
// Do your players have stamina? - this is a pre-requisite for sprinting, if you define sprinting, and don't uncomment this, it will be included anyway.
// define SDK_USE_STAMINA
//#define SDK_USE_STAMINA

//
// Are your players able to sprint?
// define SDK_USE_SPRINTING
//#define SDK_USE_SPRINTING

//Tony; stamina is a pre-requisite to sprinting, if you don't declare stamina but you do declare sprinting
//stamina needs to be included.
#if defined ( SDK_USE_SPRINTING ) && !defined( SDK_USE_STAMINA )
#define SDK_USE_STAMINA
#endif

//
// Can your players go prone?
// define SDK_USE_PRONE
#define SDK_USE_PRONE

//=====================
// EXTRA WEAPON OPTIONS
//=====================

//
// If you're allowing sprinting, do you want to be able to shoot while sprinting?
// define SDK_SHOOT_WHILE_SPRINTING
//#define SDK_SHOOT_WHILE_SPRINTING

//
// Do you want your players to be able to shoot while climing ladders?
// define SDK_SHOOT_ON_LADDERS
//#define SDK_SHOOT_ON_LADDERS

//
// Do you want your players to be able to shoot while jumping?
// define SDK_SHOOT_WHILE_JUMPING
#define SDK_SHOOT_WHILE_JUMPING



#define SDK_GAME_DESCRIPTION	"Double Action: Boogaloo"

//================================================================================
// Most elements below here are specific to the options above.
//================================================================================

enum sdkteams_e
	{
		SDK_TEAM_BLUE = LAST_SHARED_TEAM+1,
		SDK_TEAM_RED,
		SDK_TEAM_DEATHMATCH,
	};

#if defined ( SDK_USE_PRONE )

	#define TIME_TO_PRONE	1.0f
	#define VEC_PRONE_HULL_MIN	SDKGameRules()->GetSDKViewVectors()->m_vProneHullMin
	#define VEC_PRONE_HULL_MAX	SDKGameRules()->GetSDKViewVectors()->m_vProneHullMax
	#define VEC_PRONE_VIEW SDKGameRules()->GetSDKViewVectors()->m_vProneView

#endif // SDK_USE_PRONE

#if defined ( SDK_USE_SPRINTING )

	#define INITIAL_SPRINT_STAMINA_PENALTY 15
	#define LOW_STAMINA_THRESHOLD	35

#endif // SDK_USE_SPRINTING

	#define TIME_TO_UNSLIDE	0.2f
	#define VEC_SLIDE_HULL_MIN	SDKGameRules()->GetSDKViewVectors()->m_vSlideHullMin
	#define VEC_SLIDE_HULL_MAX	SDKGameRules()->GetSDKViewVectors()->m_vSlideHullMax
	#define VEC_SLIDE_VIEW		SDKGameRules()->GetSDKViewVectors()->m_vSlideView

	#define VEC_DIVE_HULL_MIN	SDKGameRules()->GetSDKViewVectors()->m_vDiveHullMin
	#define VEC_DIVE_HULL_MAX	SDKGameRules()->GetSDKViewVectors()->m_vDiveHullMax
	#define VEC_DIVE_VIEW		SDKGameRules()->GetSDKViewVectors()->m_vDiveView

#if defined ( SDK_USE_PLAYERCLASSES )
	#define SDK_NUM_PLAYERCLASSES 3		//Tony; our template sample has 3 player classes.
	#define SDK_PLAYERCLASS_IMAGE_LENGTH 64

	#define PLAYERCLASS_RANDOM		-2
	#define PLAYERCLASS_UNDEFINED	-1

	#if defined ( SDK_USE_TEAMS )
		//Tony; using teams with classes, so make sure the team class panel names are defined.
		#define PANEL_CLASS_BLUE		"class_blue"
		#define PANEL_CLASS_RED			"class_red"

		extern const char *pszTeamBlueClasses[];
		extern const char *pszTeamRedClasses[];
	#else
		#define PANEL_CLASS_NOTEAMS		"class_noteams"
		extern const char *pszPlayerClasses[];
	#endif // SDK_USE_TEAMS

#endif // SDK_USE_PLAYERCLASSES

#define SDK_PLAYER_MODEL "models/player/frank.mdl"

//Tony; We need to precache all possible player models that we're going to use
extern const char *pszPossiblePlayerModels[];

extern const char *pszTeamNames[];

//--------------------------------------------------------------------------------------------------------
//
// Weapon IDs for all SDK Game weapons
//
// Put this in order of importance in a loadout. Lower values will become primaries in the buy menu.
typedef enum
{
	WEAPON_NONE = 0,

	SDK_WEAPON_NONE = WEAPON_NONE,

	// "Long guns"
	SDK_WEAPON_FAL,
	SDK_WEAPON_MOSSBERG,
	SDK_WEAPON_M16,

	// SMG's
	SDK_WEAPON_MAC10,
	SDK_WEAPON_MP5K,

	// Akimbo pistols
	SDK_WEAPON_AKIMBO_M1911,
	SDK_WEAPON_AKIMBO_BERETTA,

	// Single pistols
	SDK_WEAPON_M1911,
	SDK_WEAPON_BERETTA,

	// Other
	SDK_WEAPON_GRENADE,
	SDK_WEAPON_BRAWL,
	WEAPON_MAX,		// number of weapons weapon index

} SDKWeaponID;

typedef enum
{
	FM_AUTOMATIC = 0,
	FM_SEMIAUTOMATIC,
	FM_BURST,

} SDK_Weapon_Firemodes;

const char *WeaponIDToAlias( SDKWeaponID id );
SDKWeaponID AliasToWeaponID( const char *alias );
const char* NoticeToString( notice_t id );
const char* WeaponTypeToAlias( weapontype_t id );

typedef enum
{
	SKILL_NONE = 0,

	SKILL_BOUNCER,
	SKILL_ATHLETIC,
	SKILL_REFLEXES,
	SKILL_MARKSMAN,
	SKILL_TROLL,
	SKILL_LAST_CHOOSEABLE =  SKILL_TROLL,

	SKILL_SUPER,    // All skills combined. Special, not chooseable from the menu.

	SKILL_MAX,		// number of weapons weapon index

	SKILL_RESILIENT, // This has been removed.
} SkillID;

const char *SkillIDToAlias( SkillID id );
SkillID AliasToSkillID( const char *alias );

// The various states the player can be in during the join game process.
enum SDKPlayerState
{
	// Happily running around in the game.
	// You can't move though if CSGameRules()->IsFreezePeriod() returns true.
	// This state can jump to a bunch of other states like STATE_PICKINGCLASS or STATE_DEATH_ANIM.
	STATE_ACTIVE=0,
	
	// This is the state you're in when you first enter the server.
	// It's switching between intro cameras every few seconds, and there's a level info 
	// screen up.
	STATE_WELCOME,			// Show the level intro screen.
	STATE_MAPINFO,          // Show the map info screen.

	// During these states, you can either be a new player waiting to join, or
	// you can be a live player in the game who wants to change teams.
	// Either way, you can't move while choosing team or class (or while any menu is up).
#if defined ( SDK_USE_TEAMS )
	STATE_PICKINGTEAM,			// Choosing team.
#endif
#if defined ( SDK_USE_PLAYERCLASSES )
	STATE_PICKINGCLASS,			// Choosing class.
#endif

	STATE_PICKINGCHARACTER,		// Choosing player model character.
	STATE_BUYINGWEAPONS,		// Buying weapons.
	STATE_PICKINGSKILL,			// Choosing special skill.

	STATE_DEATH_ANIM,			// Playing death anim, waiting for that to finish.
	STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.

	NUM_PLAYER_STATES
};
#define SDK_PLAYER_DEATH_TIME 2.0f	//Minimum Time before respawning

// Special Damage types
enum
{
	SDK_DMG_CUSTOM_NONE = 0,
	SDK_DMG_CUSTOM_SUICIDE,
};

// Player avoidance
#define PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)

#endif // SDK_SHAREDDEFS_H

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "weapon_sdkbase.h"


#ifdef CLIENT_DLL

	#include "precache_register.h"
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"

#else
	
	#include "voice_gamemgr.h"
	#include "sdk_player.h"
	#include "sdk_team.h"
	#include "sdk_playerclass_info_parse.h"
	#include "player_resource.h"
	#include "mapentities.h"
	#include "sdk_basegrenade_projectile.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifndef CLIENT_DLL

class CSpawnPoint : public CPointEntity
{
public:
	bool IsDisabled() { return m_bDisabled; }
	void InputEnable( inputdata_t &inputdata ) { m_bDisabled = false; }
	void InputDisable( inputdata_t &inputdata ) { m_bDisabled = true; }

private:
	bool m_bDisabled;
	DECLARE_DATADESC();
};

BEGIN_DATADESC(CSpawnPoint)

	// Keyfields
	DEFINE_KEYFIELD( m_bDisabled,	FIELD_BOOLEAN,	"StartDisabled" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

END_DATADESC();

	LINK_ENTITY_TO_CLASS( info_player_deathmatch, CSpawnPoint );
#if defined( SDK_USE_TEAMS )
	LINK_ENTITY_TO_CLASS( info_player_blue, CSpawnPoint );
	LINK_ENTITY_TO_CLASS( info_player_red, CSpawnPoint );
#endif

#endif


REGISTER_GAMERULES_CLASS( CSDKGameRules );


BEGIN_NETWORK_TABLE_NOBASE( CSDKGameRules, DT_SDKGameRules )
#if defined ( CLIENT_DLL )
		RecvPropFloat( RECVINFO( m_flGameStartTime ) ),
#else
		SendPropFloat( SENDINFO( m_flGameStartTime ), 32, SPROP_NOSCALE ),
#endif
END_NETWORK_TABLE()

#if defined ( SDK_USE_PLAYERCLASSES )
	ConVar mp_allowrandomclass( "mp_allowrandomclass", "1", FCVAR_REPLICATED, "Allow players to select random class" );
#endif


LINK_ENTITY_TO_CLASS( sdk_gamerules, CSDKGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( SDKGameRulesProxy, DT_SDKGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_SDKGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		RecvPropDataTable( "sdk_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_SDKGameRules ), RecvProxy_SDKGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_SDKGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CSDKGameRules *pRules = SDKGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CSDKGameRulesProxy, DT_SDKGameRulesProxy )
		SendPropDataTable( "sdk_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_SDKGameRules ), SendProxy_SDKGameRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL
	ConVar sk_plr_dmg_grenade( "sk_plr_dmg_grenade","0");		
#endif

ConVar mp_limitteams( 
	"mp_limitteams", 
	"2", 
	FCVAR_REPLICATED | FCVAR_NOTIFY,
	"Max # of players 1 team can have over another (0 disables check)",
	true, 0,	// min value
	true, 30	// max value
);

static CSDKViewVectors g_SDKViewVectors(

	Vector( 0, 0, 58 ),			//VEC_VIEW
								
	Vector(-16, -16, 0 ),		//VEC_HULL_MIN
	Vector( 16,  16,  72 ),		//VEC_HULL_MAX
													
	Vector(-16, -16, 0 ),		//VEC_DUCK_HULL_MIN
	Vector( 16,  16, 45 ),		//VEC_DUCK_HULL_MAX
	Vector( 0, 0, 34 ),			//VEC_DUCK_VIEW
													
	Vector(-10, -10, -10 ),		//VEC_OBS_HULL_MIN
	Vector( 10,  10,  10 ),		//VEC_OBS_HULL_MAX
													
	Vector( 0, 0, 14 )			//VEC_DEAD_VIEWHEIGHT
#if defined ( SDK_USE_PRONE )			
	,Vector(-16, -16, 0 ),		//VEC_PRONE_HULL_MIN
	Vector( 16,  16, 24 ),		//VEC_PRONE_HULL_MAX
	Vector( 0,	0, 10 )			//VEC_PRONE_VIEW
#endif
);

const CViewVectors* CSDKGameRules::GetViewVectors() const
{
	return (CViewVectors*)GetSDKViewVectors();
}

const CSDKViewVectors *CSDKGameRules::GetSDKViewVectors() const
{
	return &g_SDKViewVectors;
}

#ifdef CLIENT_DLL


#else

// --------------------------------------------------------------------------------------------------- //
// Voice helper
// --------------------------------------------------------------------------------------------------- //

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		// Dead players can only be heard by other dead team mates
		if ( pTalker->IsAlive() == false )
		{
			if ( pListener->IsAlive() == false )
				return ( pListener->InSameTeam( pTalker ) );

			return false;
		}

		return ( pListener->InSameTeam( pTalker ) );
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;



// --------------------------------------------------------------------------------------------------- //
// Globals.
// --------------------------------------------------------------------------------------------------- //
static const char *s_PreserveEnts[] =
{
	"player",
	"viewmodel",
	"worldspawn",
	"soundent",
	"ai_network",
	"ai_hint",
	"sdk_gamerules",
	"sdk_team_manager",
	"sdk_team_unassigned",
	"sdk_team_blue",
	"sdk_team_red",
	"sdk_player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sprite",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_illusionary",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_red",
	"info_player_blue",
	"point_viewcontrol",
	"shadow_control",
	"sky_camera",
	"scene_manager",
	"trigger_soundscape",
	"point_commentary_node",
	"func_precipitation",
	"func_team_wall",
	"", // END Marker
};

// --------------------------------------------------------------------------------------------------- //
// Global helper functions.
// --------------------------------------------------------------------------------------------------- //

// World.cpp calls this but we don't use it in SDK.
void InitBodyQue()
{
}


// --------------------------------------------------------------------------------------------------- //
// CSDKGameRules implementation.
// --------------------------------------------------------------------------------------------------- //

CSDKGameRules::CSDKGameRules()
{
	InitTeams();

	m_bLevelInitialized = false;

#if defined ( SDK_USE_TEAMS )
	m_iSpawnPointCount_Blue = 0;
	m_iSpawnPointCount_Red = 0;
#endif // SDK_USE_TEAMS

	m_flGameStartTime = 0;

}
void CSDKGameRules::ServerActivate()
{
	//Tony; initialize the level
	CheckLevelInitialized();

	//Tony; do any post stuff
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}
}
void CSDKGameRules::CheckLevelInitialized()
{
	if ( !m_bLevelInitialized )
	{
#if defined ( SDK_USE_TEAMS )
		// Count the number of spawn points for each team
		// This determines the maximum number of players allowed on each

		CBaseEntity* ent = NULL;

		m_iSpawnPointCount_Blue		= 0;
		m_iSpawnPointCount_Red		= 0;

		while ( ( ent = gEntList.FindEntityByClassname( ent, "info_player_blue" ) ) != NULL )
		{
			if ( IsSpawnPointValid( ent, NULL ) )
			{
				m_iSpawnPointCount_Blue++;
			}
			else
			{
				Warning("Invalid blue spawnpoint at (%.1f,%.1f,%.1f)\n",
					ent->GetAbsOrigin()[0],ent->GetAbsOrigin()[2],ent->GetAbsOrigin()[2] );
			}
		}

		while ( ( ent = gEntList.FindEntityByClassname( ent, "info_player_red" ) ) != NULL )
		{
			if ( IsSpawnPointValid( ent, NULL ) ) 
			{
				m_iSpawnPointCount_Red++;
			}
			else
			{
				Warning("Invalid red spawnpoint at (%.1f,%.1f,%.1f)\n",
					ent->GetAbsOrigin()[0],ent->GetAbsOrigin()[2],ent->GetAbsOrigin()[2] );
			}
		}
#endif // SDK_USE_TEAMS
		m_bLevelInitialized = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKGameRules::~CSDKGameRules()
{
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: TF2 Specific Client Commands
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CSDKGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
	CSDKPlayer *pPlayer = ToSDKPlayer( pEdict );
#if 0
	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "somecommand" ) )
	{
		if ( args.ArgC() < 2 )
			return true;

		// Do something here!

		return true;
	}
	else 
#endif
	// Handle some player commands here as they relate more directly to gamerules state
	if ( pPlayer->ClientCommand( args ) )
	{
		return true;
	}
	else if ( BaseClass::ClientCommand( pEdict, args ) )
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just spawned. Equip them.
//-----------------------------------------------------------------------------

void CSDKGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore )
{
	RadiusDamage( info, vecSrcIn, flRadius, iClassIgnore, false );
}

// Add the ability to ignore the world trace
void CSDKGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld )
{
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;
	Vector		vecToTarget;
	Vector		vecEndPos;

	Vector vecSrc = vecSrcIn;

	if ( flRadius )
		falloff = info.GetDamage() / flRadius;
	else
		falloff = 1.0;

	int bInWater = (UTIL_PointContents ( vecSrc ) & MASK_WATER) ? true : false;

	vecSrc.z += 1;// in case grenade is lying on the ground

	// iterate on all entities in the vicinity.
	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( pEntity->m_takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->GetWaterLevel() == 0)
				continue;
			if (!bInWater && pEntity->GetWaterLevel() == 3)
				continue;

			// radius damage can only be blocked by the world
			vecSpot = pEntity->BodyTarget( vecSrc );



			bool bHit = false;

			if( bIgnoreWorld )
			{
				vecEndPos = vecSpot;
				bHit = true;
			}
			else
			{
				UTIL_TraceLine( vecSrc, vecSpot, MASK_SOLID_BRUSHONLY, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );

				if (tr.startsolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.endpos = vecSrc;
					tr.fraction = 0.0;
				}

				vecEndPos = tr.endpos;

				if( tr.fraction == 1.0 || tr.m_pEnt == pEntity )
				{
					bHit = true;
				}
			}

			if ( bHit )
			{
				// the explosion can 'see' this entity, so hurt them!
				//vecToTarget = ( vecSrc - vecEndPos );
				vecToTarget = ( vecEndPos - vecSrc );

				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = vecToTarget.Length() * falloff;
				flAdjustedDamage = info.GetDamage() - flAdjustedDamage;

				if ( flAdjustedDamage > 0 )
				{
					CTakeDamageInfo adjustedInfo = info;
					adjustedInfo.SetDamage( flAdjustedDamage );

					Vector dir = vecToTarget;
					VectorNormalize( dir );

					// If we don't have a damage force, manufacture one
					if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
					{
						CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc, 1.5	/* explosion scale! */ );
					}
					else
					{
						// Assume the force passed in is the maximum force. Decay it based on falloff.
						float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
						adjustedInfo.SetDamageForce( dir * flForce );
						adjustedInfo.SetDamagePosition( vecSrc );
					}

					pEntity->TakeDamage( adjustedInfo );

					// Now hit all triggers along the way that respond to damage... 
					pEntity->TraceAttackToTriggers( adjustedInfo, vecSrc, vecEndPos, dir );
				}
			}
		}
	}
}

void CSDKGameRules::Think()
{
	BaseClass::Think();
}
Vector DropToGround( 
					CBaseEntity *pMainEnt, 
					const Vector &vPos, 
					const Vector &vMins, 
					const Vector &vMaxs )
{
	trace_t trace;
	UTIL_TraceHull( vPos, vPos + Vector( 0, 0, -500 ), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );
	return trace.endpos;
}


void TestSpawnPointType( const char *pEntClassName )
{
	// Find the next spawn spot.
	CBaseEntity *pSpot = gEntList.FindEntityByClassname( NULL, pEntClassName );

	while( pSpot )
	{
		// check if pSpot is valid
		if( g_pGameRules->IsSpawnPointValid( pSpot, NULL ) )
		{
			// the successful spawn point's location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 0, 255, 0, 100, 60 );

			// drop down to ground
			Vector GroundPos = DropToGround( NULL, pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX );

			// the location the player will spawn at
			NDebugOverlay::Box( GroundPos, VEC_HULL_MIN, VEC_HULL_MAX, 0, 0, 255, 100, 60 );

			// draw the spawn angles
			QAngle spotAngles = pSpot->GetLocalAngles();
			Vector vecForward;
			AngleVectors( spotAngles, &vecForward );
			NDebugOverlay::HorzArrow( pSpot->GetAbsOrigin(), pSpot->GetAbsOrigin() + vecForward * 32, 10, 255, 0, 0, 255, true, 60 );
		}
		else
		{
			// failed spawn point location
			NDebugOverlay::Box( pSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX, 255, 0, 0, 100, 60 );
		}

		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	}
}

void TestSpawns()
{
	TestSpawnPointType( "info_player_deathmatch" );
#if defined ( SDK_USE_TEAMS )
	TestSpawnPointType( "info_player_blue" );
	TestSpawnPointType( "info_player_red" );
#endif // SDK_USE_TEAMS
}
ConCommand cc_TestSpawns( "map_showspawnpoints", TestSpawns, "Dev - test the spawn points, draws for 60 seconds", FCVAR_CHEAT );

CBaseEntity *CSDKGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	// get valid spawn point
	CBaseEntity *pSpawnSpot = pPlayer->EntSelectSpawnPoint();

	// drop down to ground
	Vector GroundPos = DropToGround( pPlayer, pSpawnSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX );

	// Move the player to the place it said.
	pPlayer->Teleport( &GroundPos, &pSpawnSpot->GetLocalAngles(), &vec3_origin );
	pPlayer->m_Local.m_vecPunchAngle = vec3_angle;

	return pSpawnSpot;
}

// checks if the spot is clear of players
bool CSDKGameRules::IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer )
{
	if ( !pSpot->IsTriggered( pPlayer ) )
	{
		return false;
	}

	// Check if it is disabled by Enable/Disable
	CSpawnPoint *pSpawnPoint = dynamic_cast< CSpawnPoint * >( pSpot );
	if ( pSpawnPoint )
	{
		if ( pSpawnPoint->IsDisabled() )
		{
			return false;
		}
	}

	Vector mins = GetViewVectors()->m_vHullMin;
	Vector maxs = GetViewVectors()->m_vHullMax;

	Vector vTestMins = pSpot->GetAbsOrigin() + mins;
	Vector vTestMaxs = pSpot->GetAbsOrigin() + maxs;

	// First test the starting origin.
	return UTIL_IsSpaceEmpty( pPlayer, vTestMins, vTestMaxs );
}

void CSDKGameRules::PlayerSpawn( CBasePlayer *p )
{	
	CSDKPlayer *pPlayer = ToSDKPlayer( p );

	int team = pPlayer->GetTeamNumber();

	if( team != TEAM_SPECTATOR )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		if( pPlayer->m_Shared.DesiredPlayerClass() == PLAYERCLASS_RANDOM )
		{
			ChooseRandomClass( pPlayer );
			ClientPrint( pPlayer, HUD_PRINTTALK, "#game_now_as", GetPlayerClassName( pPlayer->m_Shared.PlayerClass(), team ) );
		}
		else
		{
			pPlayer->m_Shared.SetPlayerClass( pPlayer->m_Shared.DesiredPlayerClass() );
		}

		int playerclass = pPlayer->m_Shared.PlayerClass();

		if( playerclass != PLAYERCLASS_UNDEFINED )
		{
			//Assert( PLAYERCLASS_UNDEFINED < playerclass && playerclass < NUM_PLAYERCLASSES );

			CSDKTeam *pTeam = GetGlobalSDKTeam( team );
			const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( playerclass );

			Assert( pClassInfo.m_iTeam == team );

			pPlayer->SetModel( pClassInfo.m_szPlayerModel );
			pPlayer->SetHitboxSet( 0 );

			char buf[64];
			int bufsize = sizeof(buf);

			//Give weapons

			// Primary weapon
			Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iPrimaryWeapon) );
			CBaseEntity *pPrimaryWpn = pPlayer->GiveNamedItem( buf );
			Assert( pPrimaryWpn );

			// Secondary weapon
			CBaseEntity *pSecondaryWpn = NULL;
			if ( pClassInfo.m_iSecondaryWeapon != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iSecondaryWeapon) );
				pSecondaryWpn = pPlayer->GiveNamedItem( buf );
			}

			// Melee weapon
			if ( pClassInfo.m_iMeleeWeapon )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iMeleeWeapon) );
				pPlayer->GiveNamedItem( buf );
			}

			CWeaponSDKBase *pWpn = NULL;

			// Primary Ammo
			pWpn = dynamic_cast<CWeaponSDKBase *>(pPrimaryWpn);

			if( pWpn )
			{
				int iNumClip = pWpn->GetSDKWpnData().m_iDefaultAmmoClips - 1;	//account for one clip in the gun
				int iClipSize = pWpn->GetSDKWpnData().iMaxClip1;
				pPlayer->GiveAmmo( iNumClip * iClipSize, pWpn->GetSDKWpnData().szAmmo1 );
			}

			// Secondary Ammo
			if ( pSecondaryWpn )
			{
				pWpn = dynamic_cast<CWeaponSDKBase *>(pSecondaryWpn);

				if( pWpn )
				{
					int iNumClip = pWpn->GetSDKWpnData().m_iDefaultAmmoClips - 1;	//account for one clip in the gun
					int iClipSize = pWpn->GetSDKWpnData().iMaxClip1;
					pPlayer->GiveAmmo( iNumClip * iClipSize, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}				

			// Grenade Type 1
			if ( pClassInfo.m_iGrenType1 != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iGrenType1) );
				CBaseEntity *pGrenade = pPlayer->GiveNamedItem( buf );
				Assert( pGrenade );
				
				pWpn = dynamic_cast<CWeaponSDKBase *>(pGrenade);

				if( pWpn )
				{
					pPlayer->GiveAmmo( pClassInfo.m_iNumGrensType1 - 1, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}

			// Grenade Type 2
			if ( pClassInfo.m_iGrenType2 != WEAPON_NONE )
			{
				Q_snprintf( buf, bufsize, "weapon_%s", WeaponIDToAlias(pClassInfo.m_iGrenType2) );
				CBaseEntity *pGrenade2 = pPlayer->GiveNamedItem( buf );
				Assert( pGrenade2 );
				
				pWpn = dynamic_cast<CWeaponSDKBase *>(pGrenade2);

				if( pWpn )
				{
					pPlayer->GiveAmmo( pClassInfo.m_iNumGrensType2 - 1, pWpn->GetSDKWpnData().szAmmo1 );
				}
			}

			pPlayer->Weapon_Switch( (CBaseCombatWeapon *)pPrimaryWpn );

//			DevMsg("setting spawn armor to: %d\n", pClassInfo.m_iArmor );
			pPlayer->SetSpawnArmorValue( pClassInfo.m_iArmor );

		}
		else
		{
//			Assert( !"Player spawning with PLAYERCLASS_UNDEFINED" );
			pPlayer->SetModel( SDK_PLAYER_MODEL );
		}
#else
		pPlayer->GiveDefaultItems();
#endif // SDK_USE_PLAYERCLASSES
		pPlayer->SetMaxSpeed( 600 );
	}
}
#if defined ( SDK_USE_PLAYERCLASSES )
void CSDKGameRules::ChooseRandomClass( CSDKPlayer *pPlayer )
{
	int i;
	int numChoices = 0;
	int choices[16];
	int firstclass = 0;

	CSDKTeam *pTeam = GetGlobalSDKTeam( pPlayer->GetTeamNumber() );

	int lastclass = pTeam->GetNumPlayerClasses();

	int previousClass = pPlayer->m_Shared.PlayerClass();

	// Compile a list of the classes that aren't full
	for( i=firstclass;i<lastclass;i++ )
	{
		// don't join the same class twice in a row
		if ( i == previousClass )
			continue;

		if( CanPlayerJoinClass( pPlayer, i ) )
		{	
			choices[numChoices] = i;
			numChoices++;
		}
	}

	// If ALL the classes are full
	if( numChoices == 0 )
	{
		Msg( "Random class found that all classes were full - ignoring class limits for this spawn\n" );

		pPlayer->m_Shared.SetPlayerClass( random->RandomFloat( firstclass, lastclass ) );
	}
	else
	{
		// Choose a slot randomly
		i = random->RandomInt( 0, numChoices-1 );

		// We are now the class that was in that slot
		pPlayer->m_Shared.SetPlayerClass( choices[i] );
	}
}
bool CSDKGameRules::CanPlayerJoinClass( CSDKPlayer *pPlayer, int cls )
{
	if( cls == PLAYERCLASS_RANDOM )
	{
		return mp_allowrandomclass.GetBool();
	}

	if( ReachedClassLimit( pPlayer->GetTeamNumber(), cls ) )
		return false;

	return true;
}

bool CSDKGameRules::ReachedClassLimit( int team, int cls )
{
	Assert( cls != PLAYERCLASS_UNDEFINED );
	Assert( cls != PLAYERCLASS_RANDOM );

	// get the cvar
	int iClassLimit = GetClassLimit( team, cls );

	// count how many are active
	int iClassExisting = CountPlayerClass( team, cls );

	if( iClassLimit > -1 && iClassExisting >= iClassLimit )
	{
		return true;
	}

	return false;
}

int CSDKGameRules::CountPlayerClass( int team, int cls )
{
	int num = 0;
	CSDKPlayer *pSDKPlayer;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pSDKPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		if (pSDKPlayer == NULL)
			continue;

		if (FNullEnt( pSDKPlayer->edict() ))
			continue;

		if( pSDKPlayer->GetTeamNumber() != team )
			continue;

		if( pSDKPlayer->m_Shared.DesiredPlayerClass() == cls )
			num++;
	}

	return num;
}

int CSDKGameRules::GetClassLimit( int team, int cls )
{
	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	Assert( pTeam );

	const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( cls );

	int iClassLimit;

	ConVar *pLimitCvar = ( ConVar * )cvar->FindVar( pClassInfo.m_szLimitCvar );

	Assert( pLimitCvar );

	if( pLimitCvar )
		iClassLimit = pLimitCvar->GetInt();
	else
		iClassLimit = -1;

	return iClassLimit;
}

bool CSDKGameRules::IsPlayerClassOnTeam( int cls, int team )
{
	if( cls == PLAYERCLASS_RANDOM )
		return true;

	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	return ( cls >= 0 && cls < pTeam->GetNumPlayerClasses() );
}

#endif // SDK_USE_PLAYERCLASSES

void CSDKGameRules::InitTeams( void )
{
	Assert( g_Teams.Count() == 0 );

	g_Teams.Purge();	// just in case

#if defined ( SDK_USE_PLAYERCLASSES )
	// clear the player class data
	ResetFilePlayerClassInfoDatabase();
#endif // SDK_USE_PLAYERCLASSES

	// Create the team managers

	//Tony; we have a special unassigned team incase our mod is using classes but not teams.
	CTeam *pUnassigned = static_cast<CTeam*>(CreateEntityByName( "sdk_team_unassigned" ));
	Assert( pUnassigned );
	pUnassigned->Init( pszTeamNames[TEAM_UNASSIGNED], TEAM_UNASSIGNED );
	g_Teams.AddToTail( pUnassigned );

	//Tony; just use a plain ole sdk_team_manager for spectators
	CTeam *pSpectator = static_cast<CTeam*>(CreateEntityByName( "sdk_team_manager" ));
	Assert( pSpectator );
	pSpectator->Init( pszTeamNames[TEAM_SPECTATOR], TEAM_SPECTATOR );
	g_Teams.AddToTail( pSpectator );

	//Tony; don't create these two managers unless teams are being used!
#if defined ( SDK_USE_TEAMS )
	//Tony; create the blue team
	CTeam *pBlue = static_cast<CTeam*>(CreateEntityByName( "sdk_team_blue" ));
	Assert( pBlue );
	pBlue->Init( pszTeamNames[SDK_TEAM_BLUE], SDK_TEAM_BLUE );
	g_Teams.AddToTail( pBlue );

	//Tony; create the red team
	CTeam *pRed = static_cast<CTeam*>(CreateEntityByName( "sdk_team_red" ));
	Assert( pRed );
	pRed->Init( pszTeamNames[SDK_TEAM_RED], SDK_TEAM_RED );
	g_Teams.AddToTail( pRed );
#endif 
}

/* create some proxy entities that we use for transmitting data */
void CSDKGameRules::CreateStandardEntities()
{
	// Create the player resource
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create( "sdk_player_manager", vec3_origin, vec3_angle );

	// Create the entity that will send our data to the client.
#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
		CBaseEntity::Create( "sdk_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
}
int CSDKGameRules::SelectDefaultTeam()
{
	int team = TEAM_UNASSIGNED;

#if defined ( SDK_USE_TEAMS )
	CSDKTeam *pBlue = GetGlobalSDKTeam(SDK_TEAM_BLUE);
	CSDKTeam *pRed = GetGlobalSDKTeam(SDK_TEAM_RED);

	int iNumBlue = pBlue->GetNumPlayers();
	int iNumRed = pRed->GetNumPlayers();

	int iBluePoints = pBlue->GetScore();
	int iRedPoints  = pRed->GetScore();

	// Choose the team that's lacking players
	if ( iNumBlue < iNumRed )
	{
		team = SDK_TEAM_BLUE;
	}
	else if ( iNumBlue > iNumRed )
	{
		team = SDK_TEAM_RED;
	}
	// choose the team with fewer points
	else if ( iBluePoints < iRedPoints )
	{
		team = SDK_TEAM_BLUE;
	}
	else if ( iBluePoints > iRedPoints )
	{
		team = SDK_TEAM_RED;
	}
	else
	{
		// Teams and scores are equal, pick a random team
		team = ( random->RandomInt(0,1) == 0 ) ? SDK_TEAM_BLUE : SDK_TEAM_RED;		
	}

	if ( TeamFull( team ) )
	{
		// Pick the opposite team
		if ( team == SDK_TEAM_BLUE )
		{
			team = SDK_TEAM_RED;
		}
		else
		{
			team = SDK_TEAM_BLUE;
		}

		// No choices left
		if ( TeamFull( team ) )
			return TEAM_UNASSIGNED;
	}
#endif // SDK_USE_TEAMS
	return team;
}
#if defined ( SDK_USE_TEAMS )
//Tony; we only check this when using teams, unassigned can never get full.
bool CSDKGameRules::TeamFull( int team_id )
{
	switch ( team_id )
	{
	case SDK_TEAM_BLUE:
		{
			int iNumBlue = GetGlobalSDKTeam(SDK_TEAM_BLUE)->GetNumPlayers();
			return iNumBlue >= m_iSpawnPointCount_Blue;
		}
	case SDK_TEAM_RED:
		{
			int iNumRed = GetGlobalSDKTeam(SDK_TEAM_RED)->GetNumPlayers();
			return iNumRed >= m_iSpawnPointCount_Red;
		}
	}
	return false;
}

//checks to see if the desired team is stacked, returns true if it is
bool CSDKGameRules::TeamStacked( int iNewTeam, int iCurTeam  )
{
	//players are allowed to change to their own team
	if(iNewTeam == iCurTeam)
		return false;

#if defined ( SDK_USE_TEAMS )
	int iTeamLimit = mp_limitteams.GetInt();

	// Tabulate the number of players on each team.
	int iNumBlue = GetGlobalTeam( SDK_TEAM_BLUE )->GetNumPlayers();
	int iNumRed = GetGlobalTeam( SDK_TEAM_RED )->GetNumPlayers();

	switch ( iNewTeam )
	{
	case SDK_TEAM_BLUE:
		if( iCurTeam != TEAM_UNASSIGNED && iCurTeam != TEAM_SPECTATOR )
		{
			if((iNumBlue + 1) > (iNumRed + iTeamLimit - 1))
				return true;
			else
				return false;
		}
		else
		{
			if((iNumBlue + 1) > (iNumRed + iTeamLimit))
				return true;
			else
				return false;
		}
		break;
	case SDK_TEAM_RED:
		if( iCurTeam != TEAM_UNASSIGNED && iCurTeam != TEAM_SPECTATOR )
		{
			if((iNumRed + 1) > (iNumBlue + iTeamLimit - 1))
				return true;
			else
				return false;
		}
		else
		{
			if((iNumRed + 1) > (iNumBlue + iTeamLimit))
				return true;
			else
				return false;
		}
		break;
	}
#endif // SDK_USE_TEAMS

	return false;
}

#endif // SDK_USE_TEAMS
//-----------------------------------------------------------------------------
// Purpose: determine the class name of the weapon that got a kill
//-----------------------------------------------------------------------------
const char *CSDKGameRules::GetKillingWeaponName( const CTakeDamageInfo &info, CSDKPlayer *pVictim, int *iWeaponID )
{
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = SDKGameRules()->GetDeathScorer( pKiller, pInflictor, pVictim );

	const char *killer_weapon_name = "world";
	*iWeaponID = SDK_WEAPON_NONE;

	if ( pScorer && pInflictor && ( pInflictor == pScorer ) )
	{
		// If the inflictor is the killer,  then it must be their current weapon doing the damage
		if ( pScorer->GetActiveWeapon() )
		{
			killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname(); 
			if ( pScorer->IsPlayer() )
			{
				*iWeaponID = ToSDKPlayer(pScorer)->GetActiveSDKWeapon()->GetWeaponID();
			}
		}
	}
	else if ( pInflictor )
	{
		killer_weapon_name = STRING( pInflictor->m_iClassname );

		CWeaponSDKBase *pWeapon = dynamic_cast< CWeaponSDKBase * >( pInflictor );
		if ( pWeapon )
		{
			*iWeaponID = pWeapon->GetWeaponID();
		}
		else
		{
			CBaseGrenadeProjectile *pBaseGrenade = dynamic_cast<CBaseGrenadeProjectile*>( pInflictor );
			if ( pBaseGrenade )
			{
				*iWeaponID = pBaseGrenade->GetWeaponID();
			}
		}
	}

	// strip certain prefixes from inflictor's classname
	const char *prefix[] = { "weapon_", "NPC_", "func_" };
	for ( int i = 0; i< ARRAYSIZE( prefix ); i++ )
	{
		// if prefix matches, advance the string pointer past the prefix
		int len = Q_strlen( prefix[i] );
		if ( strncmp( killer_weapon_name, prefix[i], len ) == 0 )
		{
			killer_weapon_name += len;
			break;
		}
	}

	// grenade projectiles need to be translated to 'grenade' 
	if ( 0 == Q_strcmp( killer_weapon_name, "grenade_projectile" ) )
	{
		killer_weapon_name = "grenade";
	}

	return killer_weapon_name;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//			*pKiller - 
//			*pInflictor - 
//-----------------------------------------------------------------------------
void CSDKGameRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	int killer_ID = 0;

	// Find the killer & the scorer
	CSDKPlayer *pSDKPlayerVictim = ToSDKPlayer( pVictim );
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor, pVictim );
//	CSDKPlayer *pAssister = ToSDKPlayer( GetAssister( pVictim, pScorer, pInflictor ) );

	// Work out what killed the player, and send a message to all clients about it
	int iWeaponID;
	const char *killer_weapon_name = GetKillingWeaponName( info, pSDKPlayerVictim, &iWeaponID );

	if ( pScorer )	// Is the killer a client?
	{
		killer_ID = pScorer->GetUserID();
	}

	IGameEvent * event = gameeventmanager->CreateEvent( "player_death" );

	if ( event )
	{
		event->SetInt( "userid", pVictim->GetUserID() );
		event->SetInt( "attacker", killer_ID );
//		event->SetInt( "assister", pAssister ? pAssister->GetUserID() : -1 );
		event->SetString( "weapon", killer_weapon_name );
		event->SetInt( "weaponid", iWeaponID );
		event->SetInt( "damagebits", info.GetDamageType() );
		event->SetInt( "customkill", info.GetDamageCustom() );
		event->SetInt( "priority", 7 );	// HLTV event priority, not transmitted
		gameeventmanager->FireEvent( event );
	}		
}
#endif


bool CSDKGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	//Don't stand on COLLISION_GROUP_WEAPON
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

//Tony; keep this in shared space.
#if defined ( SDK_USE_PLAYERCLASSES )
const char *CSDKGameRules::GetPlayerClassName( int cls, int team )
{
	CSDKTeam *pTeam = GetGlobalSDKTeam( team );

	if( cls == PLAYERCLASS_RANDOM )
	{
		return "#class_random";
	}

	if( cls < 0 || cls >= pTeam->GetNumPlayerClasses() )
	{
		Assert( false );
		return NULL;
	}

	const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( cls );

	return pClassInfo.m_szPrintName;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Init CS ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			1	

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef* GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		for (int i=WEAPON_NONE+1;i<WEAPON_MAX;i++)
		{
			//Tony; ignore grenades, shotgun and the crowbar, grenades and shotgun are handled seperately because of their damage type not being DMG_BULLET.
			if (i == SDK_WEAPON_GRENADE || i == SDK_WEAPON_CROWBAR || i == SDK_WEAPON_SHOTGUN)
				continue;

			def.AddAmmoType( WeaponIDToAlias(i), DMG_BULLET, TRACER_LINE_AND_WHIZ, 0, 0, 200/*max carry*/, 1, 0 );
		}

		// def.AddAmmoType( BULLET_PLAYER_50AE,		DMG_BULLET, TRACER_LINE, 0, 0, "ammo_50AE_max",		2400, 0, 10, 14 );
		def.AddAmmoType( "shotgun", DMG_BUCKSHOT, TRACER_NONE, 0, 0,	200/*max carry*/, 1, 0 );
		def.AddAmmoType( "grenades", DMG_BLAST, TRACER_NONE, 0, 0,	4/*max carry*/, 1, 0 );

		//Tony; added for the sdk_jeep
		def.AddAmmoType( "JeepAmmo",	DMG_SHOCK,					TRACER_NONE,			"sdk_jeep_weapon_damage",		"sdk_jeep_weapon_damage", "sdk_jeep_max_rounds", BULLET_IMPULSE(650, 8000), 0 );
	}

	return &def;
}


#ifndef CLIENT_DLL

const char *CSDKGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	//Tony; no prefix for now, it isn't needed.
	return "";
}

const char *CSDKGameRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
		return NULL;

	const char *pszFormat = NULL;

	if ( bTeamOnly )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			pszFormat = "SDK_Chat_Spec";
		else
		{
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "SDK_Chat_Team_Dead";
			else
				pszFormat = "SDK_Chat_Team";
		}
	}
	else
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
			pszFormat = "SDK_Chat_AllSpec";
		else
		{
			if (pPlayer->m_lifeState != LIFE_ALIVE )
				pszFormat = "SDK_Chat_All_Dead";
			else
				pszFormat = "SDK_Chat_All";
		}
	}

	return pszFormat;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CSDKGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
		return GR_TEAMMATE;

#endif

	return GR_NOTTEAMMATE;
}

float CSDKGameRules::GetMapRemainingTime()
{
#ifdef GAME_DLL
	if ( nextlevel.GetString() && *nextlevel.GetString() && engine->IsMapValid( nextlevel.GetString() ) )
	{
		return 0;
	}
#endif

	// if timelimit is disabled, return -1
	if ( mp_timelimit.GetInt() <= 0 )
		return -1;

	// timelimit is in minutes
	float flTimeLeft =  ( m_flGameStartTime + mp_timelimit.GetInt() * 60 ) - gpGlobals->curtime;

	// never return a negative value
	if ( flTimeLeft < 0 )
		flTimeLeft = 0;

	return flTimeLeft;
}

float CSDKGameRules::GetMapElapsedTime( void )
{
	return gpGlobals->curtime;
}
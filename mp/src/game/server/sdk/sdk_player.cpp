//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "sdk_player.h"
#include "sdk_team.h"
#include "sdk_gamerules.h"
#include "weapon_sdkbase.h"
#include "weapon_grenade.h"
#include "predicted_viewmodel.h"
#include "iservervehicle.h"
#include "viewport_panel_names.h"
#include "info_camera_link.h"
#include "GameStats.h"
#include "obstacle_pushaway.h"
#include "in_buttons.h"
#include "vprof.h"
#include "engine/IEngineSound.h"
#include "datacache/imdlcache.h"

#include "weapon_akimbobase.h"
#include "vcollide_parse.h"
#include "vphysics/player_controller.h"
#include "igamemovement.h"
#include "da_ammo_pickup.h"
#include "bots/bot_main.h"
#include "ammodef.h"
#include "dove.h"
#include "da_datamanager.h"
#include "da_briefcase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int gEvilImpulse101;

ConVar bot_mimic( "bot_mimic", "0", FCVAR_CHEAT );
ConVar bot_freeze( "bot_freeze", "0", FCVAR_CHEAT );
ConVar bot_crouch( "bot_crouch", "0", FCVAR_CHEAT );
ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "180", FCVAR_CHEAT );

ConVar SDK_ShowStateTransitions( "sdk_ShowStateTransitions", "-2", FCVAR_CHEAT, "sdk_ShowStateTransitions <ent index or -1 for all>. Show player state transitions." );


EHANDLE g_pLastDMSpawn;
#if defined ( SDK_USE_TEAMS )
EHANDLE g_pLastBlueSpawn;
EHANDLE g_pLastRedSpawn;
#endif
// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	//Tony; pull the player who is doing it out of the recipientlist, this is predicted!!
	filter.RemoveRecipient( pPlayer );

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //
BEGIN_DATADESC( CSDKPlayer )
DEFINE_THINKFUNC( SDKPushawayThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( player, CSDKPlayer );
PRECACHE_REGISTER(player);

// CSDKPlayerShared Data Tables
//=============================

// specific to the local player
BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKSharedLocalPlayerExclusive )
#if defined ( SDK_USE_PLAYERCLASSES )
	SendPropInt( SENDINFO( m_iPlayerClass), 4 ),
	SendPropInt( SENDINFO( m_iDesiredPlayerClass ), 4 ),
#endif
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerShared )
#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	SendPropFloat( SENDINFO( m_flStamina ), 0, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#endif

#if defined ( SDK_USE_PRONE )
	SendPropBool( SENDINFO( m_bProne ) ),
	SendPropTime( SENDINFO( m_flGoProneTime ) ),
	SendPropTime( SENDINFO( m_flUnProneTime ) ),
	SendPropTime( SENDINFO( m_flDisallowUnProneTime ) ),
	SendPropBool( SENDINFO( m_bProneSliding ) ),
#endif
#if defined ( SDK_USE_SPRINTING )
	SendPropBool( SENDINFO( m_bIsSprinting ) ),
#endif
	SendPropBool( SENDINFO( m_bSliding ) ),
	SendPropBool( SENDINFO( m_bInAirSlide ) ),
	SendPropVector( SENDINFO(m_vecSlideDirection) ),
	SendPropTime( SENDINFO( m_flSlideTime ) ),
	SendPropTime( SENDINFO( m_flUnSlideTime ) ),
	SendPropVector( SENDINFO(m_vecUnSlideEyeStartOffset) ),
	SendPropBool( SENDINFO( m_bMustDuckFromSlide ) ),
	SendPropBool( SENDINFO( m_bDiveSliding ) ),
	SendPropTime( SENDINFO( m_flLastDuckPress ) ),
	SendPropBool( SENDINFO( m_bRolling ) ),
	SendPropBool( SENDINFO( m_bRollingFromDive ) ),
	SendPropVector( SENDINFO(m_vecRollDirection) ),
	SendPropTime( SENDINFO( m_flRollTime ) ),
	SendPropBool( SENDINFO( m_bDiving ) ),
	SendPropVector( SENDINFO(m_vecDiveDirection) ),
	SendPropBool( SENDINFO( m_bRollAfterDive ) ),
	SendPropTime( SENDINFO( m_flDiveTime ) ),
	SendPropTime( SENDINFO( m_flTimeLeftGround ) ),
	SendPropFloat( SENDINFO( m_flDiveLerped ) ),
	SendPropFloat( SENDINFO( m_flDiveToProneLandTime ) ),
	SendPropBool( SENDINFO( m_bAimedIn ) ),
	SendPropFloat( SENDINFO( m_flAimIn ) ),
	SendPropFloat( SENDINFO( m_flSlowAimIn ) ),
	SendPropInt( SENDINFO( m_iStyleSkill ) ),
	SendPropBool( SENDINFO( m_bSuperSkill ) ),
	
	SendPropInt (SENDINFO (m_iWallFlipCount)),
	SendPropBool (SENDINFO (m_bIsWallFlipping)),
	SendPropFloat (SENDINFO (m_flWallFlipEndTime)),
	SendPropFloat (SENDINFO (m_flMantelTime)),
	SendPropVector (SENDINFO (m_vecMantelWallNormal)),

	SendPropBool( SENDINFO( m_bSuperFalling ) ),
	SendPropBool( SENDINFO( m_bSuperFallOthersVisible ) ),
	SendPropTime( SENDINFO( m_flSuperFallOthersNextCheck ) ),

	SendPropDataTable( "sdksharedlocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKSharedLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
END_SEND_TABLE()
extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

BEGIN_SEND_TABLE_NOBASE( CArmament, DT_Loadout )
	SendPropInt(SENDINFO(m_iCount)),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayer, DT_SDKLocalPlayerExclusive )
	SendPropInt( SENDINFO( m_iShotsFired ), 8, SPROP_UNSIGNED ),
	// send a hi-res origin to the local player for use in prediction
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
//	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

	SendPropInt( SENDINFO( m_ArmorValue ), 8, SPROP_UNSIGNED ),

	SendPropTime		( SENDINFO( m_flFreezeUntil ) ),
	SendPropFloat		( SENDINFO( m_flFreezeAmount ) ),

	SendPropFloat		( SENDINFO( m_flDisarmRedraw ) ),

	SendPropArray3( SENDINFO_ARRAY3(m_aLoadout), SendPropDataTable( SENDINFO_DT( m_aLoadout ), &REFERENCE_SEND_TABLE( DT_Loadout ) ) ),
	SendPropInt( SENDINFO( m_iLoadoutWeight ), 8, SPROP_UNSIGNED ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayer, DT_SDKNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

	SendPropFloat( SENDINFO_VECTORELEM(m_vecViewOffset, 0), 8, SPROP_ROUNDDOWN, -32.0, 32.0f),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecViewOffset, 1), 8, SPROP_ROUNDDOWN, -32.0, 32.0f),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecViewOffset, 2), 20, SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, 256.0f),
END_SEND_TABLE()

void* SendProxy_SendNonLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient( objectID - 1 );
	return ( void * )pVarData;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendNonLocalDataTable );


// main table
IMPLEMENT_SERVERCLASS_ST( CSDKPlayer, DT_SDKPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	
	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	// Data that only gets sent to the local player.
	SendPropDataTable( SENDINFO_DT( m_Shared ), &REFERENCE_SEND_TABLE( DT_SDKPlayerShared ) ),

	// Data that only gets sent to the local player.
	SendPropDataTable( "sdklocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
	// Data that gets sent to all other players
	SendPropDataTable( "sdknonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropInt( SENDINFO( m_iPlayerState ), Q_log2( NUM_PLAYER_STATES )+1, SPROP_UNSIGNED ),

	SendPropBool( SENDINFO( m_bSpawnInterpCounter ) ),

	SendPropInt( SENDINFO( m_flStylePoints ) ),
	SendPropFloat( SENDINFO( m_flStyleSkillCharge ) ),

	SendPropInt( SENDINFO( m_iSlowMoType ), 4, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bHasSuperSlowMo ) ),
	SendPropFloat		( SENDINFO( m_flSlowMoSeconds ) ),
	SendPropTime		( SENDINFO( m_flSlowMoTime ) ),
	SendPropTime		( SENDINFO( m_flSlowMoMultiplier ) ),
	SendPropFloat( SENDINFO( m_flCurrentTime ), -1, SPROP_CHANGES_OFTEN ),
	SendPropFloat( SENDINFO( m_flLastSpawnTime ) ),
	SendPropTime		( SENDINFO( m_flReadyWeaponUntil ) ),

	SendPropBool( SENDINFO( m_bHasPlayerDied ) ),
	SendPropBool( SENDINFO( m_bThirdPerson ) ),
	SendPropBool( SENDINFO( m_bThirdPersonCamSide ) ),
	SendPropStringT( SENDINFO( m_iszCharacter ) ),

	SendPropEHandle( SENDINFO( m_hBriefcase ) ),

	SendPropBool( SENDINFO( m_bCoderHacks ) ),
	SendPropInt( SENDINFO( m_nCoderHacksButtons ), 32, SPROP_UNSIGNED ),

	SendPropEHandle( SENDINFO( m_hKiller ) ),
	SendPropEHandle( SENDINFO( m_hInflictor ) ),
	SendPropBool( SENDINFO( m_bWasKilledByExplosion ) ),
	SendPropVector (SENDINFO (m_vecKillingExplosionPosition)),

	SendPropEHandle( SENDINFO( m_hSwitchFrom ) ),
END_SEND_TABLE()

class CSDKRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CSDKRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	void Spawn();

	bool IsRagdollVisible();

	void DisappearThink();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( sdk_ragdoll, CSDKRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CSDKRagdoll, DT_SDKRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()

void CSDKRagdoll::Spawn()
{
	BaseClass::Spawn();

	SetThink(&CSDKRagdoll::DisappearThink);
	SetNextThink(gpGlobals->curtime + 15);
}

void CSDKRagdoll::DisappearThink()
{
	if (IsRagdollVisible())
	{
		SetNextThink(gpGlobals->curtime + 5);
		return;
	}

	UTIL_Remove(this);
}

bool CSDKRagdoll::IsRagdollVisible()
{
	Vector vecOrigin = m_vecRagdollOrigin; // Ragdoll may have moved but it all happens client-side so this is close enough.

	for (int i = 1; i < gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		Vector vecToRagdoll = vecOrigin - pPlayer->GetAbsOrigin();
		vecToRagdoll.NormalizeInPlace();
		Vector vecForward;
		pPlayer->GetVectors(&vecForward, NULL, NULL);

		if (vecForward.Dot(vecToRagdoll) < 0.4)
			continue;

		if (pPlayer->IsVisible(GetAbsOrigin()))
			return true;
	}

	return false;
}

// -------------------------------------------------------------------------------- //

void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT );

void CSDKPlayer::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );

	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
	PointCameraSetupVisibility( this, area, pvs, pvssize );
}

CSDKPlayer::CSDKPlayer()
{
	//Tony; create our player animation state.
	m_PlayerAnimState = CreateSDKPlayerAnimState( this );
	m_iLastWeaponFireUsercmd = 0;
	
	m_Shared.Init( this );

	UseClientSideAnimation();

	m_angEyeAngles.Init();

	m_pCurStateInfo = NULL;	// no state yet

	m_flFreezeUntil = .1;
	m_flFreezeAmount = 0;

	m_flFreezeUntil = -1;

	m_flNextRegen = 0;
	m_flNextHealthDecay = 0;

	m_bHasPlayerDied = false;
	m_bThirdPerson = false;

	m_iKills = m_iDeaths = 0;

	m_flCurrentTime = gpGlobals->curtime;

	m_iszCharacter = NULL_STRING;
}


CSDKPlayer::~CSDKPlayer()
{
	DropBriefcase();
	DestroyRagdoll();
	m_PlayerAnimState->Release();
}


CSDKPlayer *CSDKPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CSDKPlayer::s_PlayerEdict = ed;
	return (CSDKPlayer*)CreateEntityByName( className );
}

int CSDKPlayer::UpdateTransmitState()
{
	if (SDKGameRules()->GetBountyPlayer() == this)
		return SetTransmitState( FL_EDICT_ALWAYS );

	return BaseClass::UpdateTransmitState();
}

void CSDKPlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	BaseClass::LeaveVehicle( vecExitPoint, vecExitAngles );

	//teleport physics shadow too
	// Vector newPos = GetAbsOrigin();
	// QAngle newAng = GetAbsAngles();

	// Teleport( &newPos, &newAng, &vec3_origin );
}

bool CSDKPlayer::FVisible(CBaseEntity *pEntity, int iTraceMask, CBaseEntity **ppBlocker)
{
	if (pEntity->IsPlayer())
		return IsVisible(dynamic_cast<CSDKPlayer*>(pEntity));
	else
		return CBasePlayer::FVisible(pEntity, iTraceMask, ppBlocker);
}

bool CSDKPlayer::IsVisible( const Vector &pos, bool testFOV, const CBaseEntity *ignore ) const
{
	// is it in my general viewcone?
	if (testFOV && !const_cast<CSDKPlayer*>(this)->FInViewCone( pos ))
		return false;

	// check line of sight
	// Must include CONTENTS_MONSTER to pick up all non-brush objects like barrels
	trace_t result;
	CTraceFilterNoNPCsOrPlayer traceFilter( ignore, COLLISION_GROUP_NONE );
	UTIL_TraceLine( const_cast<CSDKPlayer*>(this)->EyePosition(), pos, MASK_OPAQUE, &traceFilter, &result );
	if (result.fraction != 1.0f)
		return false;

	return true;
}

bool CSDKPlayer::IsVisible(CSDKPlayer *pPlayer, bool testFOV, unsigned char *visParts) const
{
	if (!pPlayer)
		return false;

	// optimization - assume if center is not in FOV, nothing is
	// we're using WorldSpaceCenter instead of GUT so we can skip GetPartPosition below - that's
	// the most expensive part of this, and if we can skip it, so much the better.
	if (testFOV && !(const_cast<CSDKPlayer*>(this)->FInViewCone( pPlayer->WorldSpaceCenter() )))
	{
		return false;
	}

	unsigned char testVisParts = VIS_NONE;

	// check gut
	Vector partPos = GetPartPosition( pPlayer, VIS_GUT );

	// finish gut check
	if (IsVisible( partPos, testFOV ))
	{
		if (visParts == NULL)
			return true;

		testVisParts |= VIS_GUT;
	}


	// check top of head
	partPos = GetPartPosition( pPlayer, VIS_HEAD );
	if (IsVisible( partPos, testFOV ))
	{
		if (visParts == NULL)
			return true;

		testVisParts |= VIS_HEAD;
	}

	// check feet
	partPos = GetPartPosition( pPlayer, VIS_FEET );
	if (IsVisible( partPos, testFOV ))
	{
		if (visParts == NULL)
			return true;

		testVisParts |= VIS_FEET;
	}

	// check "edges"
	partPos = GetPartPosition( pPlayer, VIS_LEFT_SIDE );
	if (IsVisible( partPos, testFOV ))
	{
		if (visParts == NULL)
			return true;

		testVisParts |= VIS_LEFT_SIDE;
	}

	partPos = GetPartPosition( pPlayer, VIS_RIGHT_SIDE );
	if (IsVisible( partPos, testFOV ))
	{
		if (visParts == NULL)
			return true;

		testVisParts |= VIS_RIGHT_SIDE;
	}

	if (visParts)
		*visParts = testVisParts;

	if (testVisParts)
		return true;

	return false;
}

Vector CSDKPlayer::GetPartPosition(CSDKPlayer* player, VisiblePartType part) const
{
	// which PartInfo corresponds to the given player
	PartInfo* info = &m_partInfo[ player->entindex() % MAX_PLAYERS ];

	if (gpGlobals->framecount > info->m_validFrame)
	{
		// update part positions
		const_cast<CSDKPlayer*>(this)->ComputePartPositions( player );
		info->m_validFrame = gpGlobals->framecount;
	}

	// return requested part position
	switch( part )
	{
		default:
		{
			AssertMsg( false, "GetPartPosition: Invalid part" );
			// fall thru to GUT
		}

		case VIS_GUT:
			return info->m_gutPos;

		case VIS_HEAD:
			return info->m_headPos;
			
		case VIS_FEET:
			return info->m_feetPos;

		case VIS_LEFT_SIDE:
			return info->m_leftSidePos;
			
		case VIS_RIGHT_SIDE:
			return info->m_rightSidePos;
	}
}

CSDKPlayer::PartInfo CSDKPlayer::m_partInfo[ MAX_PLAYERS ];

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute part positions from bone location.
 */
void CSDKPlayer::ComputePartPositions( CSDKPlayer *player )
{
	const int headBox = 1;
	const int gutBox = 3;
	const int leftElbowBox = 11;
	const int rightElbowBox = 9;
	//const int hipBox = 0;
	//const int leftFootBox = 4;
	//const int rightFootBox = 8;
	const int maxBoxIndex = leftElbowBox;

	// which PartInfo corresponds to the given player
	PartInfo *info = &m_partInfo[ player->entindex() % MAX_PLAYERS ];

	// always compute feet, since it doesn't rely on bones
	info->m_feetPos = player->GetAbsOrigin();
	info->m_feetPos.z += 5.0f;

	// get bone positions for interesting points on the player
	MDLCACHE_CRITICAL_SECTION();
	CStudioHdr *studioHdr = player->GetModelPtr();
	if (studioHdr)
	{
		mstudiohitboxset_t *set = studioHdr->pHitboxSet( player->GetHitboxSet() );
		if (set && maxBoxIndex < set->numhitboxes)
		{
			QAngle angles;
			mstudiobbox_t *box;

			// gut
			box = set->pHitbox( gutBox );
			player->GetBonePosition( box->bone, info->m_gutPos, angles );	

			// head
			box = set->pHitbox( headBox );
			player->GetBonePosition( box->bone, info->m_headPos, angles );

			Vector forward, right;
			AngleVectors( angles, &forward, &right, NULL );

			// in local bone space
			const float headForwardOffset = 4.0f;
			const float headRightOffset = 2.0f;
			info->m_headPos += headForwardOffset * forward + headRightOffset * right;

			/// @todo Fix this hack - lower the head target because it's a bit too high for the current T model
			info->m_headPos.z -= 2.0f;


			// left side
			box = set->pHitbox( leftElbowBox );
			player->GetBonePosition( box->bone, info->m_leftSidePos, angles );	

			// right side
			box = set->pHitbox( rightElbowBox );
			player->GetBonePosition( box->bone, info->m_rightSidePos, angles );	

			return;
		}
	}


	// default values if bones are not available
	info->m_headPos = player->GetCentroid();
	info->m_gutPos = info->m_headPos;
	info->m_leftSidePos = info->m_headPos;
	info->m_rightSidePos = info->m_headPos;
}

Vector CSDKPlayer::GetCentroid( ) const
{
	Vector centroid = GetAbsOrigin();

	const Vector &mins = WorldAlignMins();
	const Vector &maxs = WorldAlignMaxs();

	centroid.z += (maxs.z - mins.z)/2.0f;

	return centroid;
}

CSDKPlayer* CSDKPlayer::FindClosestFriend(float flMaxDistance, bool bFOV)
{
	CSDKPlayer* pClosest = NULL;
	for (int i = 1; i < gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer == this)
			continue;

		if (SDKGameRules()->PlayerRelationship(this, pPlayer) == GR_NOTTEAMMATE)
			continue;

		if (!IsVisible(pPlayer, bFOV))
			continue;

		float flDistance = (GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();
		if (flDistance > flMaxDistance)
			continue;

		if (pClosest && (pClosest->GetAbsOrigin() - GetAbsOrigin()).Length() > flDistance)
			continue;

		pClosest = pPlayer;
	}

	return pClosest;
}

bool CSDKPlayer::RunMimicCommand( CUserCmd& cmd )
{
	if ( !IsBot() )
		return false;

	if ( bot_freeze.GetBool() )
		return true;

	int iMimic = abs( bot_mimic.GetInt() );
	if ( iMimic > gpGlobals->maxClients )
		return false;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( iMimic );
	if ( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand() )
		return false;

	cmd = *pPlayer->GetLastUserCommand();
	cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

	pl.fixangle = FIXANGLE_NONE;

	return true;
}

inline bool CSDKPlayer::IsReloading( void ) const
{
	CWeaponSDKBase *pPrimary = GetActiveSDKWeapon();

	if (pPrimary && pPrimary->m_bInReload)
		return true;

	return false;
}

ConVar da_regenamount( "da_regenamount", "5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player regenerate each tick?" );
ConVar da_decayamount( "da_decayamount", "1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player decay each tick, when total health is greater than max?" );
ConVar da_regenamount_secondwind( "da_regenamount_secondwind", "10", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does a player with the second wind style skill regenerate each tick?" );

void CSDKPlayer::PreThink(void)
{
	m_vecTotalBulletForce = vec3_origin;

	if (IsInThirdPerson())
		UpdateThirdCamera(Weapon_ShootPosition(), EyeAngles() + GetPunchAngle());
	else
	{
		m_flCameraLerp = 0;
		m_flStuntLerp = 0;
	}

	UpdateCurrentTime();

	if (IsAlive())
	{
		if (!IsStyleSkillActive() && m_flCurrentTime > m_flNextHealthDecay && GetHealth() > GetMaxHealth())
		{
			m_iHealth -= da_decayamount.GetFloat();

			m_flNextHealthDecay = m_flCurrentTime + 1;
		}

		if (m_flCurrentTime > m_flNextRegen)
		{
			float flRatio = da_regenamount_secondwind.GetFloat()/da_regenamount.GetFloat();
			float flModifier = (flRatio - 1)/2;
			float flHealth = m_Shared.ModifySkillValue(da_regenamount.GetFloat(), flModifier, SKILL_RESILIENT);

			m_flNextRegen = m_flCurrentTime + 1;

			// Heal up to 50% of the player's health.
			int iMaxHealth = GetMaxHealth()/2;

			if (IsStyleSkillActive(SKILL_RESILIENT))
				// If Resilient is active, heal up to 100%, which is actually 200 health
				iMaxHealth = GetMaxHealth();
			else if (m_Shared.m_iStyleSkill == SKILL_RESILIENT)
				// If it's passive heal to 100%
				iMaxHealth = GetMaxHealth();

			int iHealthTaken = 0;
			if (GetHealth() < iMaxHealth)
				iHealthTaken = TakeHealth(min(flHealth, iMaxHealth - GetHealth()), 0);

			UseStyleCharge(SKILL_RESILIENT, iHealthTaken/2);
		}
	}

	if (IsAlive() && ( m_Shared.IsDiving() || m_Shared.IsRolling() || m_Shared.IsSliding() || (!GetGroundEntity() && GetAbsVelocity().z < -280.0f) ))
	{
		Vector vecNormalizedVelocity = GetAbsVelocity();
		vecNormalizedVelocity.NormalizeInPlace();

		trace_t	tr;

		// we need to start from a higher offset if we're not diving (18 works!)
		int iOffset = 13*!m_Shared.IsDiving();

		UTIL_TraceHull(GetAbsOrigin() + Vector(0, 0, 5+iOffset), GetAbsOrigin() + vecNormalizedVelocity*40 + Vector(0, 0, 10), Vector(-16, -16, -16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		CBaseEntity* pHit = NULL;

		bool bIsBreakable = false;
		if (tr.m_pEnt)
		{
			bIsBreakable = FStrEq(tr.m_pEnt->GetClassname(), "func_breakable");
			if (!bIsBreakable)
				bIsBreakable = FStrEq(tr.m_pEnt->GetClassname(), "func_breakable_surf");
		}

		if (tr.fraction < 1.0f && bIsBreakable)
			pHit = tr.m_pEnt;

		// if we're falling break anything under us
		if (!pHit && vecNormalizedVelocity.z < 0)
		{
			UTIL_TraceHull(GetAbsOrigin() + Vector(0, 0, 5), GetAbsOrigin() - Vector(0, 0, 5), Vector(-16, -16, -16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			if (tr.fraction < 1.0f && bIsBreakable)
				pHit = tr.m_pEnt;
		}

		if (pHit)
			pHit->TakeDamage(CTakeDamageInfo(this, this, 50, DMG_CRUSH));
	}

	// Make sure the server caches the players visible part for the client.
	m_Shared.CanSuperFallRespawn();

	State_PreThink();

	// Riding a vehicle?
	if ( IsInAVehicle() )	
	{
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();		
		CheckTimeBasedDamage();

		// Allow the suit to recharge when in the vehicle.
		CheckSuitUpdate();
		
		WaterMove();	
		return;
	}

	BaseClass::PreThink();
}

ConVar sv_drawserverhitbox("sv_drawserverhitbox", "0", FCVAR_CHEAT|FCVAR_REPLICATED, "Shows server's hitbox representation." );

void CSDKPlayer::PostThink()
{
	BaseClass::PostThink();

	if ( !g_fGameOver && !IsPlayerLockedInPlace() )
	{
		if ( IsAlive() )
		{
			// set correct collision bounds (may have changed in player movement code)
			VPROF_SCOPE_BEGIN( "CBasePlayer::PostThink-Bounds" );
			if ( m_Shared.IsRolling() )
				SetCollisionBounds( VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX );
			else if ( m_Shared.IsDiving() )
				SetCollisionBounds( VEC_DIVE_HULL_MIN, VEC_DIVE_HULL_MAX );
			else if ( m_Shared.IsSliding() )
				SetCollisionBounds( VEC_SLIDE_HULL_MIN, VEC_SLIDE_HULL_MAX );
			else if ( m_Shared.IsProne() )
				SetCollisionBounds( VEC_PRONE_HULL_MIN, VEC_PRONE_HULL_MAX );
			VPROF_SCOPE_END();
		}
	}

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();
	QAngle angCharacterEyeAngles = m_angEyeAngles;

	if (IsInThirdPerson())
	{
		VectorAngles(m_vecThirdTarget - EyePosition(), angCharacterEyeAngles);

		angCharacterEyeAngles[YAW] = AngleNormalize(angCharacterEyeAngles[YAW]);
		angCharacterEyeAngles[PITCH] = AngleNormalize(angCharacterEyeAngles[PITCH]);
	}

	m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH], angCharacterEyeAngles[YAW], angCharacterEyeAngles[PITCH] );

	if ( sv_drawserverhitbox.GetBool() )
		DrawServerHitboxes( 2*gpGlobals->frametime, true );

	if (SDKGameRules()->CoderHacks())
		m_nCoderHacksButtons = m_nButtons;
}

bool CSDKPlayer::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return m_iIgnoreGlobalChat != CHAT_IGNORE_ALL;

	// check if we're ignoring all chat
	if ( m_iIgnoreGlobalChat == CHAT_IGNORE_ALL )
		return false;

	// check if we're ignoring all but teammates
	if ( m_iIgnoreGlobalChat == CHAT_IGNORE_TEAM && g_pGameRules->PlayerRelationship( this, pPlayer ) != GR_TEAMMATE )
		return false;

	return true;
}

void CSDKPlayer::Precache()
{
	PrecacheScriptSound( "Player.GoRoll" );
	PrecacheScriptSound( "Player.GoDive" );
	PrecacheScriptSound( "Player.DiveLand" );
	PrecacheScriptSound( "Player.GoProne" );
	PrecacheScriptSound( "Player.UnProne" );
	PrecacheScriptSound( "Player.GoSlide" );
	PrecacheScriptSound( "Player.UnSlide" );

	PrecacheScriptSound( "HudMeter.Activate" );
	PrecacheScriptSound( "HudMeter.End" );
	PrecacheScriptSound( "HudMeter.FillLarge" );
	PrecacheScriptSound( "HudMeter.FillSmall" );
	PrecacheScriptSound( "HudMeter.FillStylish" );
	PrecacheScriptSound( "HudMeter.Knockout" );

	PrecacheScriptSound( "MiniObjective.Begin" );

	PrecacheScriptSound( "Weapon_Brawl.PunchHit" );

	//Tony; go through our list of player models that we may be using and cache them
	int i = 0;
	while( pszPossiblePlayerModels[i] != NULL )
	{
		PrecacheModel( pszPossiblePlayerModels[i] );
		i++;
	}	

	PrecacheParticleSystem( "impact_concrete" );
	PrecacheParticleSystem( "impact_dirt" );
	PrecacheParticleSystem( "impact_metal" );
	PrecacheParticleSystem( "impact_glass" );
	PrecacheParticleSystem( "impact_tile" );
	PrecacheParticleSystem( "impact_wood" );
	PrecacheParticleSystem( "muzzleflash_pistol" );
	PrecacheParticleSystem( "muzzleflash_smg" );
	PrecacheParticleSystem( "muzzleflash_rifle" );
	PrecacheParticleSystem( "muzzleflash_shotgun" );
	PrecacheParticleSystem( "tracer_bullet" );

	BaseClass::Precache();
}

void CSDKPlayer::GiveDefaultItems()
{
	char szName[128];

	if ( State_Get() == STATE_ACTIVE )
	{
		GiveNamedItem( "weapon_brawl" );

		bool bHasGrenade = false;
		CWeaponSDKBase* pHeaviestWeapon = NULL;

		for (int i = 1; i < MAX_LOADOUT; i++)
		{
			SDKWeaponID eBuyWeapon = (SDKWeaponID)i;
			CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(eBuyWeapon);

			if (!pWeaponInfo)
				continue;

			bool bBuy = !!m_aLoadout[eBuyWeapon].m_iCount;
			if (*pWeaponInfo->m_szSingle)
			{
				// If we're buying a weapon that has a single version, buy the akimbo version if we want two of the singles.
				SDKWeaponID eSingleWeapon = AliasToWeaponID(pWeaponInfo->m_szSingle);
				if (m_aLoadout[eSingleWeapon].m_iCount == 2)
					bBuy = true;
			}

			if (bBuy)
			{
				Q_snprintf( szName, sizeof( szName ), "weapon_%s", WeaponIDToAlias(eBuyWeapon) );
				CWeaponSDKBase* pWeapon = static_cast<CWeaponSDKBase*>(GiveNamedItem( szName ));

				if (!pHeaviestWeapon)
					pHeaviestWeapon = pWeapon;
				else if (pWeapon && pWeapon->GetWeaponID() != SDK_WEAPON_GRENADE && pWeapon->GetSDKWpnData().iWeight > pHeaviestWeapon->GetSDKWpnData().iWeight)
					pHeaviestWeapon = pWeapon;

				CSDKWeaponInfo* pInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);
				if (pInfo)
				{
					if (!FStrEq(pInfo->szAmmo1, "grenades"))
						CBasePlayer::GiveAmmo( pInfo->iMaxClip1*pInfo->m_iDefaultAmmoClips, pInfo->szAmmo1);
					else
					{
						bHasGrenade = true;
						CBasePlayer::GiveAmmo( m_aLoadout[i].m_iCount-1, "grenades");
					}
				}
			}
		}

		if (pHeaviestWeapon)
			Weapon_Switch(pHeaviestWeapon);

		for (int i = 0; i < WeaponCount(); i++)
		{
			CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(GetWeapon(i));
			CWeaponSDKBase* pLastWeapon = dynamic_cast<CWeaponSDKBase*>(Weapon_GetLast());

			if (!pWeapon)
				continue;

			if (pWeapon == GetActiveWeapon())
				continue;

			if (!pWeapon)
				continue;

			if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
				continue;

			if (!pLastWeapon || pLastWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
			{
				Weapon_SetLast(pWeapon);
				continue;
			}

			// Only use a grenade as the lastweapon if there are no other weapons.
			if (pWeapon->IsGrenade())
				continue;

			// Always go to a higher weight weapon.
			if (pWeapon->GetWeight() > pLastWeapon->GetWeight())
			{
				Weapon_SetLast(pWeapon);
				continue;
			}

			// Always use the akimbo version instead of single.
			if (FStrEq(UTIL_VarArgs("weapon_%s", pWeapon->GetSDKWpnData().m_szSingle), pLastWeapon->GetSDKWpnData().szClassName))
			{
				Weapon_SetLast(pWeapon);
				continue;
			}

			break;
		}

		if (m_Shared.m_iStyleSkill == SKILL_TROLL)
		{
			if (bHasGrenade)
				CBasePlayer::GiveAmmo(1, "grenades");
			else
				GiveNamedItem( "weapon_grenade" );
		}
	}
}

#define SDK_PUSHAWAY_THINK_CONTEXT	"SDKPushawayThink"
void CSDKPlayer::SDKPushawayThink(void)
{
	// Push physics props out of our way.
	PerformObstaclePushaway( this );
	SetNextThink( gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, SDK_PUSHAWAY_THINK_CONTEXT );
}

void CSDKPlayer::Spawn()
{
	if ( gpGlobals->teamplay )
	{
		switch(GetTeamNumber())
		{
		case SDK_TEAM_BLUE:
			m_nSkin = 1;
			break;
		case SDK_TEAM_RED:
			m_nSkin = 2;
			break;
		}
	}
	else
		m_nSkin = 0;

	MDLCACHE_CRITICAL_SECTION();

	if (!STRING(m_iszCharacter.Get())[0])
		PickRandomCharacter();

	if (STRING(m_iszCharacter.Get())[0])
		SetModel( UTIL_VarArgs("models/player/%s.mdl", STRING(m_iszCharacter.Get())) );
	else
		SetModel( SDK_PLAYER_MODEL );

	SetBloodColor( BLOOD_COLOR_RED );

	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	m_nRenderFX = kRenderNormal;

	//Tony; if we're spawning in active state, equip the suit so the hud works. -- Gotta love base code !
	if ( State_Get() == STATE_ACTIVE )
	{
		EquipSuit( false );
//Tony; bleh, don't do this here.
//		GiveDefaultItems();
	}

	m_hRagdoll = NULL;
	
	BaseClass::Spawn();
#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	m_Shared.SetStamina( 100 );
#endif

#if defined ( SDK_USE_TEAMS )
	m_bTeamChanged	= false;
#endif

#if defined ( SDK_USE_PRONE )
	InitProne();
#endif

#if defined ( SDK_USE_SPRINTING )
	InitSprinting();
#endif

	// update this counter, used to not interp players when they spawn
	m_bSpawnInterpCounter = !m_bSpawnInterpCounter;

	for( int p=0; p<MAX_PLAYERS; ++p )
		m_partInfo[p].m_validFrame = 0;

	InitSpeeds(); //Tony; initialize player speeds.

	SetArmorValue(SpawnArmorValue());

	SetContextThink( &CSDKPlayer::SDKPushawayThink, gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, SDK_PUSHAWAY_THINK_CONTEXT );
	pl.deadflag = false;

	m_flStyleSkillCharge = 0;

	m_iSlowMoType = SLOWMO_NONE;
	m_bHasSuperSlowMo = false;
	m_flSlowMoSeconds = 0;
	m_flSlowMoTime = 0;
	m_flSlowMoMultiplier = 1;
	m_flDisarmRedraw = -1;
	m_iStyleKillStreak = 0;
	m_iCurrentStreak = 0;
	m_flNextSuicideTime = 0;

	m_bHasSuperSlowMo = (m_Shared.m_iStyleSkill == SKILL_REFLEXES);
	if (m_Shared.m_iStyleSkill == SKILL_REFLEXES)
		GiveSlowMo(1);

	SDKGameRules()->CalculateSlowMoForPlayer(this);

	m_flLastSpawnTime = gpGlobals->curtime;

	// ignore damage from all grenades currently in play
	m_arrIgnoreNadesByIndex.RemoveAll();
	CBaseEntity *pGrenade = gEntList.FindEntityByClassname( NULL, "grenade_projectile");

	while (pGrenade)
	{
		m_arrIgnoreNadesByIndex.AddToHead(pGrenade->entindex());
		pGrenade = gEntList.FindEntityByClassname( pGrenade, "grenade_projectile");
	}

	m_hKiller = NULL;
	m_hInflictor = NULL;
	m_bWasKilledByExplosion = false;
}

bool CSDKPlayer::SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot )
{
	// Find the next spawn spot.
	pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	if ( pSpot == NULL ) // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	CBaseEntity *pFirstSpot = pSpot;
	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetAbsOrigin() == Vector( 0, 0, 0 ) )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
					continue;
				}

				// if so, go to pSpot
				return true;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// if we're searching deathmatch spawns in teamplay and didn't get a valid one return false
	if (g_pGameRules->IsTeamplay() && !stricmp(pEntClassName, "info_player_deathmatch"))
		return false;

	DevMsg("CSDKPlayer::SelectSpawnSpot: couldn't find valid spawn point.\n");

	return true;
}


CBaseEntity* CSDKPlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pSpot = NULL;

	const char *pSpawnPointName = "";

	if (g_pGameRules->IsTeamplay()) //&& g_pGameRules->UseDeathmatchSpawns()
	{		
		pSpawnPointName = "info_player_deathmatch";
		pSpot = g_pLastDMSpawn;
		if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
		{
			g_pLastDMSpawn = pSpot;
		}
		else
			pSpot = NULL;
	}

	switch( GetTeamNumber() )
	{
	case SDK_TEAM_BLUE:
	case SDK_TEAM_RED:
	case TEAM_UNASSIGNED:
		{
			pSpawnPointName = "info_player_deathmatch";
			pSpot = g_pLastDMSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastDMSpawn = pSpot;
			}
		}		
		break;
	case TEAM_SPECTATOR:
	default:
		{
			pSpot = CBaseEntity::Instance( INDEXENT(0) );
		}
		break;		
	}

	if ( !pSpot )
	{
		Warning( "PutClientInServer: no %s on level\n", pSpawnPointName );
		return CBaseEntity::Instance( INDEXENT(0) );
	}

	return pSpot;
} 

//-----------------------------------------------------------------------------
// Purpose: Put the player in the specified team
//-----------------------------------------------------------------------------
//Tony; if we're not using actual teams, we don't need to override this.
void CSDKPlayer::ChangeTeam( int iTeamNum )
{
	if ( !GetGlobalTeam( iTeamNum ) )
	{
		Warning( "CSDKPlayer::ChangeTeam( %d ) - invalid team index.\n", iTeamNum );
		return;
	}

	int iOldTeam = GetTeamNumber();

	// if this is our current team, just abort
	if ( iTeamNum == iOldTeam )
		return;
	
	// do the team change:
	BaseClass::ChangeTeam( iTeamNum );

	// update client state 
	if ( iTeamNum == TEAM_UNASSIGNED )
	{
		if (SDKGameRules()->IsTeamplay())
			State_Transition( STATE_OBSERVER_MODE );
		else
			State_Transition( STATE_PICKINGCHARACTER );
	}
	else if ( iTeamNum == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );
		
		State_Transition( STATE_OBSERVER_MODE );
	}
	else // active player
	{
		if ( !IsDead() )
		{
			// Kill player if switching teams while alive
			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		if( iOldTeam == TEAM_SPECTATOR )
			SetMoveType( MOVETYPE_NONE );
//Tony; pop up the class menu if we're using classes, otherwise just spawn.
#if defined ( SDK_USE_PLAYERCLASSES )
		// Put up the class selection menu.
		State_Transition( STATE_PICKINGCLASS );
#else
		if (gpGlobals->teamplay)
			State_Transition( STATE_BUYINGWEAPONS );
		else
			State_Transition( STATE_PICKINGCHARACTER );
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayer::CommitSuicide( bool bExplode /* = false */, bool bForce /*= false*/ )
{
	// Don't suicide if we haven't picked a class for the first time, or we're not in active state
	if (
#if defined ( SDK_USE_PLAYERCLASSES )
		m_Shared.PlayerClass() == PLAYERCLASS_UNDEFINED || 
#endif
		State_Get() != STATE_ACTIVE 
		)
		return;

	if (!bForce && !SuicideAllowed())
	{		
		ClientPrint( this, HUD_PRINTCENTER, "DA_No_Suicide" );
		return;
	}
	
	m_iSuicideCustomKillFlags = SDK_DMG_CUSTOM_SUICIDE;

	BaseClass::CommitSuicide( bExplode, bForce );
}

void CSDKPlayer::InitialSpawn( void )
{
	m_flTotalStyle = 0;
	m_bThirdPerson = !!GetUserInfoInt("da_thirdperson", 0);

	m_iStuntKills = 0;
	m_iGrenadeKills = 0;
	m_iBrawlKills = 0;
	m_iStreakKills = 0;

	BaseClass::InitialSpawn();

	if (gpGlobals->eLoadType == MapLoad_Background)
		State_Enter( STATE_OBSERVER_MODE );
	else
		State_Enter( STATE_WELCOME );

	ClearLoadout();
}

void CSDKPlayer::OnDive()
{
}

void CSDKPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	Vector vecToDamagePoint = ptr->endpos - GetAbsOrigin();
	float flDistance = vecToDamagePoint.Length2D();

	if (flDistance > CollisionProp()->BoundingRadius2D())
	{
		vecToDamagePoint.z = 0;
		vecToDamagePoint.NormalizeInPlace();
		ptr->endpos = GetAbsOrigin() + vecToDamagePoint * CollisionProp()->BoundingRadius2D();
	}

	//Tony; disable prediction filtering, and call the baseclass.
	CDisablePredictionFiltering disabler;
	BaseClass::TraceAttack( inputInfo, vecDir, ptr, pAccumulator );
}

ConVar bot_easy( "bot_easy", "1", 0, "Make bots easier" );

int CSDKPlayer::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	CBaseEntity *pInflictor = info.GetInflictor();

	if ( !pInflictor )
		return 0;

	if ( GetMoveType() == MOVETYPE_NOCLIP || GetMoveType() == MOVETYPE_OBSERVER )
		return 0;

	// disallow suicide for 10 seconds
	m_flNextSuicideTime = GetCurrentTime() + 10.0f;

	m_vecTotalBulletForce += info.GetDamageForce();

	float flArmorBonus = 0.5f;
	float flArmorRatio = 0.5f;
	float flDamage = info.GetDamage();

	bool bCheckFriendlyFire = false;
	bool bFriendlyFire = friendlyfire.GetBool();
	//Tony; only check teams in teamplay
	if ( gpGlobals->teamplay )
		bCheckFriendlyFire = true;

	//R_Yell - mp_friendlyfire by default doesn't let us damage bots. This check turns FF off in bot vs human situations, you still can use mp_friendlyfire to disallow FF among humans
	CBasePlayer *pAttacker = ToBasePlayer(info.GetAttacker());
	if (pAttacker)
	{
		if (pAttacker->IsBot() != IsBot())
			bFriendlyFire = true;
	}
	//

	CSDKPlayer *pSDKAttacker = ToSDKPlayer(info.GetAttacker());
	if (pSDKAttacker)
	{
		// Turn down easy bots if the player is the bounty, to make it a tad harder.
		if (SDKGameRules()->GetBountyPlayer() != this && SDKGameRules()->GetBountyPlayer() != pSDKAttacker)
		{
			if (bot_easy.GetBool() && !IsBot() && pAttacker->IsBot())
				flDamage *= 0.3f;
		}
		else
		{
			if (bot_easy.GetBool() && !IsBot() && pAttacker->IsBot())
				flDamage *= 0.65f;
		}
	}

/*	if (IsStyleSkillActive(SKILL_IMPERVIOUS))
	{
		UseStyleCharge(flDamage * 0.2f);
		flDamage = m_Shared.ModifySkillValue(flDamage, 0.3f, SKILL_IMPERVIOUS);
	}*/

	if ( !(bCheckFriendlyFire && pInflictor->GetTeamNumber() == GetTeamNumber() ) || bFriendlyFire /*|| pInflictor == this ||	info.GetAttacker() == this*/ )
	{
		if ( bFriendlyFire && (info.GetDamageType() & DMG_BLAST) == 0 )
		{
			if ( pInflictor->GetTeamNumber() == GetTeamNumber() && bCheckFriendlyFire)
			{
				flDamage *= 0.35; // bullets hurt teammates less
			}
		}

		if ( info.GetDamageType() & DMG_BLAST )
		{
			// this timing only accounts for weapon_grenade (with fuse of 1.5s)
			if ( m_flLastSpawnTime > (gpGlobals->curtime - 2.5f) )
			{
				int inflictorindex = pInflictor->entindex();
				// if this is a grenade thrown before we spawned ignore the damage
				for (int i = 0; i < m_arrIgnoreNadesByIndex.Count(); i++)
				{
					if (m_arrIgnoreNadesByIndex[i] == inflictorindex)
						return 0;
				}
			}

			if ( m_Shared.IsDiving() || m_Shared.IsSliding() || m_Shared.IsRolling() )
			{
				Vector vecToPlayer = GetAbsOrigin() - info.GetDamagePosition();
				VectorNormalize( vecToPlayer );

				Vector vecDirection;

				if (m_Shared.IsDiving())
					vecDirection = m_Shared.GetDiveDirection();
				else
					vecDirection = m_Shared.GetSlideDirection();

				// stunting away from explosions reduces 30% damage
				if ( vecToPlayer.Dot( vecDirection ) > 0.5f )
				{
					flDamage *= 0.7f;//(1 - m_Shared.ModifySkillValue( 0.2f, 1.0f, SKILL_ATHLETIC ));

					// since we're being cool anyway give us a little push
					Vector vecPush = ( info.GetDamageForce() / 150.0f );
					SetBaseVelocity( vecPush );

					// award style points
					float flPoints = 10.0f;

					if (m_iSlowMoType != SLOWMO_NONE)
						flPoints *= 1.3f;

					AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_COOL, STYLE_POINT_SMALL);
					Instructor_LessonLearned("stuntfromexplo");
				}
			}
		}

		// keep track of amount of damage last sustained
		m_lastDamageAmount = flDamage;
		// Deal with Armour
		if ( ArmorValue() && !( info.GetDamageType() & (DMG_FALL | DMG_DROWN)) )
		{
			float flNew = flDamage * flArmorRatio;
			float flArmor = (flDamage - flNew) * flArmorBonus;

			// Does this use more armor than we have?
			if (flArmor > ArmorValue() )
			{
				//armorHit = (int)(flArmor);

				flArmor = ArmorValue();
				flArmor *= (1/flArmorBonus);
				flNew = flDamage - flArmor;
				SetArmorValue( 0 );
			}
			else
			{
				int oldValue = (int)(ArmorValue());
			
				if ( flArmor < 0 )
					 flArmor = 1;

				SetArmorValue( oldValue - flArmor );
				//armorHit = oldValue - (int)(pev->armorvalue);
			}
			
			flDamage = flNew;
			
			info.SetDamage( flDamage );
		}

		// round damage to integer
		info.SetDamage( (int)flDamage );

		if ( info.GetDamage() <= 0 )
			return 0;

		CSingleUserRecipientFilter user( this );
		user.MakeReliable();
		UserMessageBegin( user, "Damage" );
			WRITE_BYTE( (int)info.GetDamage() );
			WRITE_VEC3COORD( info.GetInflictor()->WorldSpaceCenter() );
		MessageEnd();

		// Do special explosion damage effect
		if ( info.GetDamageType() & DMG_BLAST )
		{
			OnDamagedByExplosion( info );
		}

		gamestats->Event_PlayerDamage( this, info );

		return CBaseCombatCharacter::OnTakeDamage( info );
	}
	else
	{
		return 0;
	}
}

int CSDKPlayer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// set damage type sustained
	m_bitsDamageType |= info.GetDamageType();

	if ( !CBaseCombatCharacter::OnTakeDamage_Alive( info ) )
		return 0;

	// fire global game event

	if (info.GetAttacker() && info.GetAttacker()->IsPlayer() && info.GetDamageType() == DMG_BULLET)
		ReadyWeapon();

	IGameEvent * event = gameeventmanager->CreateEvent( "player_hurt" );

	if ( event )
	{
		event->SetInt("userid", GetUserID() );
		event->SetInt("health", max(0, m_iHealth) );
		event->SetInt("armor", max(0, ArmorValue()) );

		if ( info.GetDamageType() & DMG_BLAST )
		{
			event->SetInt( "hitgroup", HITGROUP_GENERIC );
		}
		else
		{
			event->SetInt( "hitgroup", LastHitGroup() );
		}

		CBaseEntity * attacker = info.GetAttacker();
		const char *weaponName = "";

		if ( attacker->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( attacker );
			event->SetInt("attacker", player->GetUserID() ); // hurt by other player

			CBaseEntity *pInflictor = info.GetInflictor();
			if ( pInflictor )
			{
				if ( pInflictor == player )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( player->GetActiveWeapon() )
					{
						weaponName = player->GetActiveWeapon()->GetClassname();
					}
				}
				else
				{
					weaponName = STRING( pInflictor->m_iClassname );  // it's just that easy
				}
			}
		}
		else
		{
			event->SetInt("attacker", 0 ); // hurt by "world"
		}

		if ( strncmp( weaponName, "weapon_", 7 ) == 0 )
		{
			weaponName += 7;
		}
		else if( strncmp( weaponName, "grenade", 9 ) == 0 )	//"grenade_projectile"	
		{
			weaponName = "grenade";
		}

		event->SetString( "weapon", weaponName );
		event->SetInt( "priority", 5 );

		gameeventmanager->FireEvent( event );
	}
	
	CBaseEntity* pAttacker = info.GetAttacker();

	if( pAttacker && pAttacker->IsPlayer() )
	{
		CSDKPlayer* pAttackerSDK = ToSDKPlayer(pAttacker);

		// friendly fire is never stylish!
		if ( !(gpGlobals->teamplay && (GetTeamNumber() == pAttackerSDK->GetTeamNumber())) )
			pAttackerSDK->AwardStylePoints(this, false, info);

		// If the player is stunting and managed to damage another player while stunting, he's trained the be stylish hint.
		if (pAttackerSDK->m_Shared.IsDiving() || pAttackerSDK->m_Shared.IsSliding() || pAttackerSDK->m_Shared.IsRolling())
			pAttackerSDK->Instructor_LessonLearned("be_stylish");

		// we'll keep track of this in case the dive kills him, but not if we're on the same team! 
		if ( !(gpGlobals->teamplay && (GetTeamNumber() == pAttackerSDK->GetTeamNumber())) )
			pAttackerSDK->m_bDamagedEnemyDuringFall = true;
	}

	if (m_Shared.m_iStyleSkill != SKILL_RESILIENT)
		m_flNextRegen = m_flCurrentTime + 10;
	else if (!IsStyleSkillActive())
		m_flNextRegen = m_flCurrentTime + 6;
	else
		m_flNextRegen = m_flCurrentTime + 4;

	return 1;
}

CWeaponSDKBase* CSDKPlayer::FindAnyWeaponButBrawl()
{
	for (int i = 0; i < WeaponCount(); i++)
	{
		CWeaponSDKBase *pWeapon = (CWeaponSDKBase *)GetWeapon(i);
		if (!pWeapon)
			continue;

		if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
			continue;

		return pWeapon;
	}

	return NULL;
}

ConVar da_stylemetertotalcharge( "da_stylemetertotalcharge", "100", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "What is the total charge given when the style meter is activated?" );
extern ConVar da_stylemeteractivationcost;

void CSDKPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	m_hKiller = ToSDKPlayer(info.GetAttacker());
	m_hInflictor = info.GetInflictor();
	if (m_hInflictor.Get() != m_hKiller && info.GetDamageType() == DMG_BLAST)
	{
		// The object that's exploding will no longer exist by the time it gets to the client,
		// so we snack away its location now so that data will be still available later.
		m_bWasKilledByExplosion = true;
		m_vecKillingExplosionPosition = m_hInflictor->GetAbsOrigin();
	}

	StopSound( "Player.GoSlide" );

	if (m_Shared.IsSuperFalling() && m_bDamagedEnemyDuringFall)
	{
		AddStylePoints(10, STYLE_SOUND_LARGE, ANNOUNCEMENT_STYLISH, STYLE_POINT_LARGE);
		// Send "Worth it!" after the style points so that if the player gets their skill activated because of it, that message won't override the "Worth it!" message.
		SendNotice(NOTICE_WORTHIT);
	}

	if (GetActiveSDKWeapon() && GetActiveSDKWeapon()->GetWeaponID() == SDK_WEAPON_GRENADE)
	{
		CWeaponGrenade* pGrenadeWeapon = static_cast<CWeaponGrenade*>(GetActiveSDKWeapon());

		if (pGrenadeWeapon->IsPinPulled())
		{
			pGrenadeWeapon->DropGrenade();

			pGrenadeWeapon->DecrementAmmo( this );

			if (GetAmmoCount(pGrenadeWeapon->GetPrimaryAmmoType()) <= 0)
			{
				Weapon_Drop( pGrenadeWeapon, NULL, NULL );
				UTIL_Remove(pGrenadeWeapon);
			}
		}
	}

	CWeaponSDKBase* pWeapon;
	while ((pWeapon = FindAnyWeaponButBrawl()) != NULL)
		ThrowWeapon(pWeapon);

	CBaseEntity* pAttacker = info.GetAttacker();

	if (!IsBot() && pAttacker != this)
		DataManager().AddPlayerDeath(GetAbsOrigin());

	if( pAttacker && pAttacker->IsPlayer() && pAttacker != this )
	{
		CSDKPlayer* pSDKAttacker = ToSDKPlayer(pAttacker);

		pSDKAttacker->m_iCurrentStreak++;
		pSDKAttacker->m_iStreakKills = max(pSDKAttacker->m_iCurrentStreak, pSDKAttacker->m_iStreakKills);

		if (info.GetDamageType() == DMG_BLAST)
			pSDKAttacker->m_iGrenadeKills++;
		else
		{
			if (pSDKAttacker->m_Shared.IsDiving() || pSDKAttacker->m_Shared.IsSliding() || pSDKAttacker->m_Shared.IsRolling())
				pSDKAttacker->m_iStuntKills++;
			else if (info.GetDamageType() == DMG_CLUB)
				pSDKAttacker->m_iBrawlKills++;
		}

		// These are used to see how well the player is doing for the purposes
		// of adjusting style accretion. So, only count it if it's a "skill-based"
		// kill, ie one player shooting another, and not eg a suicide.
		m_iDeaths += 1;
		pSDKAttacker->m_iKills += 1;

		if (!pSDKAttacker->IsBot())
			DataManager().AddPlayerKill(pSDKAttacker->GetAbsOrigin());
	}

	// show killer in death cam mode
	// chopped down version of SetObserverTarget without the team check
	if( pAttacker && pAttacker->IsPlayer() )
	{
		// set new target
		m_hObserverTarget.Set( pAttacker ); 

		// reset fov to default
		SetFOV( this, 0 );

		CSDKPlayer* pAttackerSDK = ToSDKPlayer(pAttacker);

		if ( pAttacker != this && !(gpGlobals->teamplay && (GetTeamNumber() == pAttackerSDK->GetTeamNumber())) )
		{
			pAttackerSDK->AwardStylePoints(this, true, info);
			pAttackerSDK->GiveSlowMo(1);

			if (gpGlobals->teamplay && pAttackerSDK->GetTeam())
				pAttackerSDK->GetTeam()->AddScore(1);
		}
	}
	else
		m_hObserverTarget.Set( NULL );

	DropBriefcase();

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	State_Transition( STATE_DEATH_ANIM );	// Transition into the dying state.

	//Tony; after transition, remove remaining items
	RemoveAllItems( true );

	BaseClass::Event_Killed( info );

	// Force it to turn itself of so that the refund code doesn't run.
	if (m_flStylePoints >= 100)
		m_flStylePoints -= 0.1f;

	if (IsStyleSkillActive())
	{
		// If the player died while the style meter was active, refund the unused portion.
		float flUnused = m_flStyleSkillCharge/da_stylemetertotalcharge.GetFloat();
		float flRefund = flUnused*da_stylemeteractivationcost.GetFloat();
		SetStylePoints(m_flStylePoints + flRefund);
	}

	// Losing a whole activation can be rough, let's be a bit more forgiving.
	// Going down with lots of bar drops you more than going down with just a little bar.
	// This way, running around with your bar full you run a high risk.
	float flActivationCost = da_stylemeteractivationcost.GetFloat();

	float flLostPoints = RemapValClamped(m_flStylePoints, flActivationCost/4, flActivationCost, flActivationCost/8, flActivationCost/4);

	if (m_Shared.IsDiving() || m_Shared.IsSliding() || m_Shared.IsRolling())
		// Lose less bar if you die during a stunt. Going out with style is never a bad thing!
		flLostPoints /= 2;

	SetStylePoints(m_flStylePoints - flLostPoints);

	// Turn off slow motion.
	m_flSlowMoSeconds = 0;
	m_flSlowMoTime = 0;
	m_iSlowMoType = SLOWMO_NONE;

	m_bHasPlayerDied = true;

	SDKGameRules()->PlayerSlowMoUpdate(this);
}

void CSDKPlayer::OnDamagedByExplosion( const CTakeDamageInfo &info )
{
	UTIL_ScreenShake( info.GetInflictor()->GetAbsOrigin(), 10.0, 1.0, 0.5, 2000, SHAKE_START, false );
}

void CSDKPlayer::AwardStylePoints(CSDKPlayer* pVictim, bool bKilledVictim, const CTakeDamageInfo &info)
{
	if (pVictim == this)
		return;

	if (bKilledVictim && IsStyleSkillActive())
	{
		m_iStyleKillStreak++;

		if (m_iStyleKillStreak%3 == 0)
		{
			TakeHealth(50, DMG_GENERIC);

			SendNotice(NOTICE_STYLESTREAK);
		}
	}

	float flPoints = 1;

	if (bKilledVictim)
	{
		// Give me a quarter of the activation cost. I'll get another quarter from damaging.
		flPoints = da_stylemeteractivationcost.GetFloat()/4;
	}
	else
	{
		// Damaging someone stylishly for 100% of their health is enough to get one quarter bar.
		// The player will double their points from from killing. Need to kill two enemies to fill the bar.
		flPoints = RemapValClamped(info.GetDamage(), 0, 100, 0, da_stylemeteractivationcost.GetFloat()/4);
	}

	if (m_Shared.IsAimedIn())
		flPoints *= 1.2f;

	if (m_iSlowMoType != SLOWMO_NONE)
		flPoints *= 1.3f;

	float flDistance = GetAbsOrigin().DistTo(pVictim->GetAbsOrigin());
	flPoints *= RemapValClamped(flDistance, 800, 1200, 1, 1.5f);

	CSDKWeaponInfo *pWeaponInfo = GetActiveSDKWeapon()?CSDKWeaponInfo::GetWeaponInfo(GetActiveSDKWeapon()->GetWeaponID()):NULL;

	Vector vecVictimForward;
	pVictim->GetVectors(&vecVictimForward, NULL, NULL);

	Vector vecKillerToVictim = GetAbsOrigin()-pVictim->GetAbsOrigin();
	vecKillerToVictim.NormalizeInPlace();

	// Do grenades first so that something like diving while the grenade goes off doesn't give you a dive bonus.
	if (info.GetDamageType() == DMG_BLAST)
	{
		// Grenades are cool.
		if (bKilledVictim)
			AddStylePoints(flPoints*0.8f, bKilledVictim?STYLE_SOUND_LARGE:STYLE_SOUND_NONE, ANNOUNCEMENT_GRENADE_KILL, STYLE_POINT_STYLISH);
		else
			AddStylePoints(flPoints*0.8f, bKilledVictim?STYLE_SOUND_LARGE:STYLE_SOUND_NONE, ANNOUNCEMENT_GRENADE, STYLE_POINT_LARGE);

		if (!IsAlive() && bKilledVictim)
		{
			AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_STYLISH, STYLE_POINT_LARGE);
			SendNotice(NOTICE_WORTHIT);
		}
	}
	else if (bKilledVictim && GetActiveSDKWeapon() && pWeaponInfo->m_eWeaponType > WT_MELEE && GetActiveSDKWeapon()->m_iClip1 == 0 && info.GetDamageType() != DMG_CLUB)
	{
		// Killing a player with your last bullet.
		AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_LAST_BULLET, STYLE_POINT_STYLISH);
	}
	else if (bKilledVictim && pVictim->LastHitGroup() == HITGROUP_HEAD && vecVictimForward.Dot(vecKillerToVictim) < 0.3f && flDistance < 100)
	{
		// Killing a player by shooting in the head from behind.
		AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_EXECUTION, STYLE_POINT_STYLISH);
	}
	else if (pVictim->LastHitGroup() == HITGROUP_HEAD)
	{
		if (bKilledVictim)
			AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_HEADSHOT, STYLE_POINT_STYLISH);
		else
			AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_HEADSHOT, STYLE_POINT_LARGE);
	}
	else if (m_Shared.IsDiving() || m_Shared.IsSliding())
	{
		// Damaging a dude enough to kill him while stunting gives a full bar.
		if (m_Shared.IsDiving())
		{
			if (info.GetDamageType() == DMG_CLUB)
			{
				if (bKilledVictim)
					AddStylePoints(flPoints, STYLE_SOUND_KNOCKOUT, ANNOUNCEMENT_DIVEPUNCH, STYLE_POINT_STYLISH);
				else
					AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_DIVEPUNCH, STYLE_POINT_LARGE);
			}
			else
			{
				if (bKilledVictim)
					AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_DIVE_KILL, STYLE_POINT_STYLISH);
				else
					AddStylePoints(flPoints, STYLE_SOUND_SMALL, ANNOUNCEMENT_DIVE, STYLE_POINT_LARGE);
			}
		}
		else
		{
			if (info.GetDamageType() == DMG_CLUB)
			{
				if (bKilledVictim)
					AddStylePoints(flPoints, STYLE_SOUND_KNOCKOUT, ANNOUNCEMENT_SLIDEPUNCH, STYLE_POINT_STYLISH);
				else
					AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_SLIDEPUNCH, STYLE_POINT_LARGE);
			}
			else
			{
				if (bKilledVictim)
					AddStylePoints(flPoints, STYLE_SOUND_LARGE, ANNOUNCEMENT_SLIDE_KILL, STYLE_POINT_STYLISH);
				else
					AddStylePoints(flPoints, STYLE_SOUND_SMALL, ANNOUNCEMENT_SLIDE, STYLE_POINT_LARGE);
			}
		}
	}
	else if (flDistance < 60 && info.GetDamageType() != DMG_CLUB)
	{
		if (bKilledVictim)
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_LARGE, ANNOUNCEMENT_POINT_BLANK, STYLE_POINT_LARGE);
		else
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_LARGE, ANNOUNCEMENT_POINT_BLANK, STYLE_POINT_SMALL);
	}
	else if (flDistance > 1200)
	{
		// Long range.
		if (bKilledVictim)
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_LARGE, ANNOUNCEMENT_LONG_RANGE_KILL, STYLE_POINT_LARGE);
		else
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_SMALL, ANNOUNCEMENT_LONG_RANGE, STYLE_POINT_SMALL);
	}
	else if (!(info.GetDamageType() & DMG_DIRECT) && (info.GetDamageType() & DMG_BULLET))
	{
		// Damaging a dude through a wall with a firearm.
		if (bKilledVictim)
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_LARGE, ANNOUNCEMENT_THROUGH_WALL, STYLE_POINT_LARGE);
		else
			AddStylePoints(flPoints*0.6f, STYLE_SOUND_SMALL, ANNOUNCEMENT_THROUGH_WALL, STYLE_POINT_SMALL);
	}
	else if (m_Shared.IsRolling())
	{
		// Rolling, which is easier to do and typically happens after the dive, gives only half.
		if (bKilledVictim)
			AddStylePoints(flPoints*0.5f, STYLE_SOUND_LARGE, ANNOUNCEMENT_STUNT_KILL, STYLE_POINT_STYLISH);
		else
			AddStylePoints(flPoints*0.5f, STYLE_SOUND_SMALL, ANNOUNCEMENT_STUNT, STYLE_POINT_LARGE);
	}
	else if (info.GetDamageType() == DMG_CLUB)
	{
		// Brawl

		bool bExecution = false;
		if (bKilledVictim)
		{
			Vector vecForward;
			AngleVectors(pVictim->EyeAngles(), &vecForward, NULL, NULL);

			if (vecKillerToVictim.Dot(vecForward) < -0.7f)
				bExecution = true;
		}

		float flBrawlPoints = flPoints*0.5f;
		if (bExecution)
			flBrawlPoints *= 2;

		if (bKilledVictim)
		{
			if (bExecution)
				AddStylePoints(flBrawlPoints, STYLE_SOUND_KNOCKOUT, ANNOUNCEMENT_EXECUTION, STYLE_POINT_STYLISH);
			else
				AddStylePoints(flBrawlPoints, STYLE_SOUND_KNOCKOUT, ANNOUNCEMENT_BRAWL_KILL, STYLE_POINT_STYLISH);
		}
		else
			AddStylePoints(flBrawlPoints, STYLE_SOUND_SMALL, ANNOUNCEMENT_BRAWL, STYLE_POINT_LARGE);
	}
	else if (m_Shared.GetAimIn() >= 0.99f)
	{
		if (bKilledVictim)
			AddStylePoints(flPoints*0.5f, STYLE_SOUND_LARGE, ANNOUNCEMENT_TACTICOOL, STYLE_POINT_LARGE);
		else
			AddStylePoints(flPoints*0.5f, STYLE_SOUND_SMALL, ANNOUNCEMENT_TACTICOOL, STYLE_POINT_SMALL);
	}
	else if (bKilledVictim && m_flSlowMoMultiplier < 1)
	{
		AddStylePoints(flPoints*0.5f, STYLE_SOUND_LARGE, ANNOUNCEMENT_SLOWMO_KILL, STYLE_POINT_LARGE);
	}
	else
	{
		announcement_t eAnnouncement = ANNOUNCEMENT_NONE;
		if (bKilledVictim)
		{
			int iRandom = random->RandomInt(0, 1);
			switch (iRandom)
			{
			case 0:
			default:
				eAnnouncement = ANNOUNCEMENT_STYLISH;
				break;

			case 1:
				eAnnouncement = ANNOUNCEMENT_COOL;
				break;
			}
		}

		if (pVictim->m_Shared.IsDiving() || pVictim->m_Shared.IsRolling() || pVictim->m_Shared.IsSliding())
			// Damaging a stunting dude gives me slightly more bar than usual.
			AddStylePoints(flPoints*0.3f, STYLE_SOUND_NONE, eAnnouncement, STYLE_POINT_LARGE);
		else
			AddStylePoints(flPoints*0.2f, STYLE_SOUND_NONE, eAnnouncement, STYLE_POINT_LARGE);
	}
}

void CSDKPlayer::SendAnnouncement(announcement_t eAnnouncement, style_point_t ePointStyle, float flPoints)
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	// Start the message block
	UserMessageBegin( user, "StyleAnnouncement" );

		// Send our text to the client
		WRITE_LONG( eAnnouncement );
		WRITE_BYTE( ePointStyle );

		if (IsStyleSkillActive())
			WRITE_FLOAT( m_flStyleSkillCharge/da_stylemetertotalcharge.GetFloat() );
		else
			WRITE_FLOAT( GetStylePoints()/da_stylemeteractivationcost.GetFloat() );

		WRITE_FLOAT( flPoints );

	// End the message block
	MessageEnd();
}

void CSDKPlayer::SendNotice(notice_t eNotice)
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	UserMessageBegin( user, "Notice" );
		WRITE_LONG( eNotice );
		WRITE_BYTE( 0 );
	MessageEnd();
}

void CSDKPlayer::SendBroadcastNotice(notice_t eNotice, CSDKPlayer* pSubject)
{
	CBroadcastRecipientFilter filter;
	filter.MakeReliable();

	UserMessageBegin( filter, "Notice" );
		WRITE_LONG( eNotice );
		if (pSubject)
			WRITE_BYTE( pSubject->entindex() );
		else
			WRITE_BYTE( 0 );
	MessageEnd();
}

int CSDKPlayer::TakeHealth( float flHealth, int bitsDamageType )
{
	if ( !edict() || m_takedamage < DAMAGE_YES )
		return 0;

	int iMax = GetMaxHealth();

	float flMultiplier = 1.5f;

	if (IsStyleSkillActive(SKILL_RESILIENT))
		flMultiplier = 1;	// You already get double health with second wind, let's not make it triple.

// heal
	if ( m_iHealth >= iMax*flMultiplier )
		return 0;

	const int oldHealth = m_iHealth;

	m_iHealth += flHealth;

	if (m_iHealth > iMax*flMultiplier)
		m_iHealth = iMax*flMultiplier;

	return m_iHealth - oldHealth;

	// Don't call parent class, we override with special behavior
}

ConVar da_resilient_health_bonus( "da_resilient_health_bonus", "50", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player regenerate each tick?" );

int CSDKPlayer::GetMaxHealth() const
{
	if (IsStyleSkillActive(SKILL_RESILIENT))
		return BaseClass::GetMaxHealth() + da_resilient_health_bonus.GetInt();

	return BaseClass::GetMaxHealth();
}

bool CSDKPlayer::ThrowActiveWeapon( bool bAutoSwitch )
{
	return ThrowWeapon(GetActiveSDKWeapon());
}

bool CSDKPlayer::ThrowWeapon( CWeaponSDKBase* pWeapon, bool bAutoSwitch )
{
	if (pWeapon == NULL)
		return false;

	if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
		return false;

	if (pWeapon->CanWeaponBeDropped()) 
	{
		QAngle gunAngles;
		VectorAngles( BodyDirection2D(), gunAngles );

		Vector vecForward;
		AngleVectors( gunAngles, &vecForward, NULL, NULL );

		float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

		SDKThrowWeapon(pWeapon, vecForward, gunAngles, flDiameter);

		if (bAutoSwitch) SwitchToNextBestWeapon( NULL );
		return true;
	}

	return false;
}

bool CSDKPlayer::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	if (GetCurrentTime() < m_flDisarmRedraw)
		return false;

	m_hSwitchFrom = GetActiveSDKWeapon(); 

	bool bSwitched = BaseClass::Weapon_Switch(pWeapon, viewmodelindex);

	if (bSwitched)
		ReadyWeapon();

	return bSwitched;
}

void CSDKPlayer::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	m_flDisarmRedraw = -1;

	BaseClass::Weapon_Equip( pWeapon );
	dynamic_cast<CWeaponSDKBase*>(pWeapon)->SetDieThink( false );	//Make sure the context think for removing is gone!!
}

int CSDKPlayer::FindCurrentWeaponsWeight()
{
	int iWeaponsWeight = 0;
	for (int i = 0; i < WeaponCount(); i++)
	{
		if (!GetWeapon(i))
			continue;

		CWeaponSDKBase* pWeapon = GetSDKWeapon(i);

		if (pWeapon->GetWeaponID() == SDK_WEAPON_GRENADE)
		{
			int iGrenades = GetAmmoCount("grenades");

			// If I'm a nitrophiliac, forgive one grenade's worth of weight.
			// This way I can carry the extra grenade without a penalty to picking stuff up.
			if (m_Shared.m_iStyleSkill == SKILL_TROLL)
				iGrenades -= 1;

			iWeaponsWeight += iGrenades * CSDKWeaponInfo::GetWeaponInfo(SDK_WEAPON_GRENADE)->iWeight;
		}
		else
			iWeaponsWeight += pWeapon->GetWeight();
	}

	return iWeaponsWeight;
}

ConVar da_debug_weaponpickup("da_debug_weaponpickup", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CSDKPlayer::DropWeaponsToPickUp(CWeaponSDKBase* pWeapon)
{
	bool bDebug = da_debug_weaponpickup.GetBool();

	// Throw away weapons, lightest first, until we have room for this weapon.
	int iNewWeaponsWeight = FindCurrentWeaponsWeight();

	// This vector is a list the quantity of our current weapons.
	CUtlVector<int> aiWeapons;
	aiWeapons.AddMultipleToTail(WEAPON_MAX);
	memset(aiWeapons.Base(), 0, aiWeapons.Count()*sizeof(int));

	// Count up how many weapons we have currently.
	for (int i = 0; i < WeaponCount(); i++)
	{
		if (!GetSDKWeapon(i))
			continue;

		CWeaponSDKBase* pThisWeapon = GetSDKWeapon(i);

		if (pThisWeapon->IsGrenade())
			aiWeapons[pThisWeapon->GetWeaponID()] += GetAmmoCount(GetAmmoDef()->Index("grenades"));
		else if (pThisWeapon->IsAkimbo())
			aiWeapons[AliasToWeaponID(pThisWeapon->GetSDKWpnData().m_szSingle)] = 2;
		else if (aiWeapons[pThisWeapon->GetWeaponID()] == 0) // Don't overwrite if the akimbo already stored a 2
			aiWeapons[pThisWeapon->GetWeaponID()] = 1;
	}

	// Nitrophiliac forgives one grenade's weight.
	if (m_Shared.m_iStyleSkill == SKILL_TROLL && aiWeapons[SDK_WEAPON_GRENADE])
		aiWeapons[SDK_WEAPON_GRENADE] -= 1;

	if (bDebug)
	{
		DevMsg("Weapon count:\n");
		for (int i = 0; i < aiWeapons.Count(); i++)
			DevMsg(" %s: %d\n", WeaponIDToAlias((SDKWeaponID)i), aiWeapons[i]);
		DevMsg("Total weight: %d\n", iNewWeaponsWeight);
	}

	// Test for an early out. If this weapon has no akimbo version, and we already have one,
	// then just toss the one we have to make room for the new one.
	if (!pWeapon->IsGrenade() && !pWeapon->HasAkimbo())
	{
		if (aiWeapons[pWeapon->GetWeaponID()])
		{
			if (bDebug)
				DevMsg("Early out: throwing duplicate %s\n", WeaponIDToAlias(pWeapon->GetWeaponID()));

			QAngle gunAngles;
			VectorAngles( BodyDirection2D(), gunAngles );

			Vector vecForward;
			AngleVectors( gunAngles, &vecForward, NULL, NULL );

			float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

			SDKThrowWeapon(FindWeapon(pWeapon->GetWeaponID()), vecForward, gunAngles, flDiameter);

			return;
		}
	}

	// Never remove brawl.
	aiWeapons[SDK_WEAPON_BRAWL] = 0;

	// This vector is a list of how many of each weapon we're going to toss.
	CUtlVector<int> aiTossWeapons;
	aiTossWeapons.AddMultipleToTail(WEAPON_MAX);
	memset(aiTossWeapons.Base(), 0, aiTossWeapons.Count()*sizeof(int));

	// This O(n^2) algorithm could be O(nlogn) with a bit more work,
	// but it's only rarely run, and only on small data sets (n == WEAPON_MAX) so I'm not going to bother.
	while (pWeapon->GetWeight() + iNewWeaponsWeight > MAX_LOADOUT_WEIGHT)
	{
		int iLightestWeapon = -1;

		// Find the lightest weapon.
		for (int i = 0; i < aiWeapons.Count(); i++)
		{
			if (!aiWeapons[i])
				continue;

			if (aiTossWeapons[i] >= aiWeapons[i])
				continue;

			if (iLightestWeapon == -1)
			{
				iLightestWeapon = i;
				continue;
			}

			CSDKWeaponInfo* pLightestWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)iLightestWeapon);
			CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);

			if (pWeaponInfo->iWeight < pLightestWeaponInfo->iWeight)
			{
				iLightestWeapon = i;
				continue;
			}

			if (pWeaponInfo->iWeight == pLightestWeaponInfo->iWeight)
			{
				// If we're picking up a gun, prefer to toss grenades.
				if (pWeapon->GetWeaponID() != SDK_WEAPON_GRENADE && i == SDK_WEAPON_GRENADE)
				{
					iLightestWeapon = i;
					continue;
				}

				// If we're picking up a grenade, prefer to toss weapons.
				if (pWeapon->GetWeaponID() == SDK_WEAPON_GRENADE && i != SDK_WEAPON_GRENADE)
				{
					iLightestWeapon = i;
					continue;
				}
			}
		}

		if (iLightestWeapon == -1)
		{
			AssertMsg(false, UTIL_VarArgs("Can't throw away enough weapons to make room for %s!\n", pWeapon->GetClassname()));
			return;
		}

		aiTossWeapons[iLightestWeapon]++;
		iNewWeaponsWeight -= CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)iLightestWeapon)->iWeight;

		if (bDebug)
			DevMsg("Tossing %s, new weight %d\n", WeaponIDToAlias((SDKWeaponID)iLightestWeapon), iNewWeaponsWeight);
	}

	// Now add back any weapons that could have been retained.
	// This may help if the player has a heavy gun and a small gun and picks up a heavy gun.
	for (int i = 0; i < aiWeapons.Count(); i++)
	{
		if (!aiWeapons[i])
			continue;

		if (!aiTossWeapons[i])
			continue;

		CSDKWeaponInfo* pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);

		if (pWeapon->GetWeight() + iNewWeaponsWeight + pWeaponInfo->iWeight <= MAX_LOADOUT_WEIGHT)
		{
			aiTossWeapons[i] = false;
			iNewWeaponsWeight += pWeaponInfo->iWeight;

			if (bDebug)
				DevMsg("Restoring %s, new weight %d\n", WeaponIDToAlias((SDKWeaponID)i), iNewWeaponsWeight);
		}
	}

	for (int i = 0; i < aiTossWeapons.Count(); i++)
	{
		for (int j = 0; j < aiTossWeapons[i]; j++)
		{
			if (bDebug)
				DevMsg("Throwing %s\n", WeaponIDToAlias((SDKWeaponID)i));

			QAngle gunAngles;
			VectorAngles( BodyDirection2D(), gunAngles );

			Vector vecForward;
			AngleVectors( gunAngles, &vecForward, NULL, NULL );

			float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

			SDKThrowWeapon(FindWeapon((SDKWeaponID)i), vecForward, gunAngles, flDiameter);
		}
	}

	Assert(FindCurrentWeaponsWeight() + pWeapon->GetWeight() <= MAX_LOADOUT_WEIGHT);
}

void CSDKPlayer::SDKThrowWeapon( CWeaponSDKBase *pWeapon, const Vector &vecForward, const QAngle &vecAngles, float flDiameter  )
{
	Assert(pWeapon);
	if (!pWeapon)
		return;

	if (!FStrEq(pWeapon->GetSDKWpnData().m_szSingle, ""))
	{
		// This is an akimbo weapon. Toss two singles instead.

		CAkimboBase *pAkimbos = (CAkimboBase*)pWeapon;
		const char *pszSingle = pWeapon->GetSDKWpnData().m_szSingle;
		char name[32];
		int i;

		Q_snprintf (name, sizeof (name), "weapon_%s", pszSingle);
		for (i = 0; i < 2; i++)
		{
			CWeaponSDKBase *pThrow = (CWeaponSDKBase*)Weapon_Create(name);
			pThrow->VPhysicsDestroyObject(); 
			pThrow->VPhysicsInitShadow(true, true);
			if (i == 0)
				pThrow->m_iClip1 = pAkimbos->rightclip;
			else
				pThrow->m_iClip1 = pAkimbos->leftclip;

			SDKThrowWeaponInternal(pThrow, vecForward, vecAngles, flDiameter);
		}
		RemovePlayerItem (pAkimbos);

		// Remove the single version also.
		CWeaponSDKBase *pSingle = FindWeapon(AliasToWeaponID(pszSingle));
		AssertMsg (pSingle, "How do you have akimbos without a single?");
		if (pSingle) 
			RemovePlayerItem (pSingle);

		return;
	}

	if (!FStrEq(pWeapon->GetSDKWpnData().m_szAkimbo, ""))
	{
		// This is a single weapon that has an akimbo counterpart.
		// Check to see if we have the akimbo, and if we do, remove it before tossing.
		CAkimboBase *pAkimbo = (CAkimboBase*)FindWeapon(AliasToWeaponID(pWeapon->GetSDKWpnData().m_szAkimbo));
		if (pAkimbo)
		{
			// Toss out a single.
			CWeaponSDKBase *pThrow = (CWeaponSDKBase*)Weapon_Create(pWeapon->GetName());
			pThrow->VPhysicsDestroyObject(); 
			pThrow->VPhysicsInitShadow(true, true);

			// Throw the left one.
			pThrow->m_iClip1 = pAkimbo->leftclip;

			SDKThrowWeaponInternal(pThrow, vecForward, vecAngles, flDiameter);

			// Remove the akimbo weapon.
			RemovePlayerItem(pAkimbo);

			// Pretend that this wasn't the weapon we threw out, re-draw it.
			pWeapon->Holster(NULL);
			SetActiveWeapon(pWeapon);

			return;
		}
	}

	if (pWeapon->GetWeaponID() == SDK_WEAPON_GRENADE && GetAmmoCount(GetAmmoDef()->Index("grenades")) > 1)
	{
		CWeaponSDKBase *pGrenade = (CWeaponSDKBase*)Weapon_Create(pWeapon->GetName());
		pGrenade->VPhysicsDestroyObject();
		pGrenade->VPhysicsInitShadow(true, true);

		SDKThrowWeaponInternal(pGrenade, vecForward, vecAngles, flDiameter);

		RemoveAmmo( 1, GetAmmoDef()->Index("grenades") );
		return;
	}

	pWeapon->SetWeaponVisible( false );
	pWeapon->Holster(NULL);
	pWeapon->SetPrevOwner(this);
	Weapon_Detach( pWeapon );

	SDKThrowWeaponInternal(pWeapon, vecForward, vecAngles, flDiameter);

	// Throwing the last grenade doesn't remove the ammo. Must remove it manually.
	if (pWeapon->GetWeaponID() == SDK_WEAPON_GRENADE)
		RemoveAmmo( 1, GetAmmoDef()->Index("grenades") );
}

void CSDKPlayer::SDKThrowWeaponInternal( CWeaponSDKBase *pWeapon, const Vector &vecForward, const QAngle &vecAngles, float flDiameter  )
{
	Vector vecOrigin;
	CollisionProp()->RandomPointInBounds( Vector( 0.5f, 0.5f, 0.5f ), Vector( 0.5f, 0.5f, 1.0f ), &vecOrigin );

	Vector vecThrow;
	SDKThrowWeaponDir( pWeapon, vecForward, &vecThrow );

	Vector vecOffsetOrigin;
	VectorMA( vecOrigin, flDiameter, vecThrow, vecOffsetOrigin );

	trace_t	tr;
	UTIL_TraceLine( vecOrigin, vecOffsetOrigin, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
		
	if ( tr.startsolid || tr.allsolid || ( tr.fraction < 1.0f && tr.m_pEnt != pWeapon ) )
	{
		//FIXME: Throw towards a known safe spot?
		vecThrow.Negate();
		VectorMA( vecOrigin, flDiameter, vecThrow, vecOffsetOrigin );
	}

	vecThrow *= random->RandomFloat( 150.0f, 240.0f );

	vecThrow += GetAbsVelocity();

	pWeapon->SetAbsOrigin( vecOrigin );
	pWeapon->SetAbsAngles( vecAngles );
	pWeapon->Drop( vecThrow );
	pWeapon->SetRemoveable( false );

	pWeapon->SetDieThink( true );
}

void CSDKPlayer::SDKThrowWeaponDir( CWeaponSDKBase *pWeapon, const Vector &vecForward, Vector *pVecThrowDir )
{
	VMatrix zRot;
	MatrixBuildRotateZ( zRot, random->RandomFloat( -30.0f, 30.0f ) );

	Vector vecThrow;
	Vector3DMultiply( zRot, vecForward, *pVecThrowDir );

	pVecThrowDir->z = random->RandomFloat( 0.0f, 0.5f );
	VectorNormalize( *pVecThrowDir );
}

void CSDKPlayer::PlayerDeathThink()
{
	//overridden, do nothing - our states handle this now
}
void CSDKPlayer::CreateRagdollEntity()
{
	// If we already have a ragdoll, don't make another one.
	CSDKRagdoll *pRagdoll = dynamic_cast< CSDKRagdoll* >( m_hRagdoll.Get() );

	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
		pRagdoll = NULL;
	}
	Assert( pRagdoll == NULL );

	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CSDKRagdoll* >( CreateEntityByName( "sdk_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nSkin = m_nSkin;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->Spawn();
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}
//-----------------------------------------------------------------------------
// Purpose: Destroy's a ragdoll, called when a player is disconnecting.
//-----------------------------------------------------------------------------
void CSDKPlayer::DestroyRagdoll( void )
{
	CSDKRagdoll *pRagdoll = dynamic_cast<CSDKRagdoll*>( m_hRagdoll.Get() );	
	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
	}
}

void CSDKPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	m_PlayerAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

CWeaponSDKBase* CSDKPlayer::GetActiveSDKWeapon() const
{
	Assert(!GetActiveWeapon() || dynamic_cast<CWeaponSDKBase*>(GetActiveWeapon()));
	return static_cast< CWeaponSDKBase* >( GetActiveWeapon() );
}

CWeaponSDKBase* CSDKPlayer::GetSDKWeapon(int i) const
{
	Assert(!GetWeapon(i) || dynamic_cast<CWeaponSDKBase*>(GetWeapon(i)));
	return static_cast< CWeaponSDKBase* >( GetWeapon(i) );
}

void CSDKPlayer::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "da_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

void CSDKPlayer::ImpulseCommands()
{
	// Disable the flashlight
	if (GetImpulse() == 100)
		return;

	BaseClass::ImpulseCommands();
}

void CSDKPlayer::CheatImpulseCommands( int iImpulse )
{
	if ( !sv_cheats->GetBool() )
	{
		return;
	}

	if ( iImpulse != 101 )
	{
		BaseClass::CheatImpulseCommands( iImpulse );
		return ;
	}
	gEvilImpulse101 = true;

	EquipSuit();
	
	if ( GetHealth() < 100 )
	{
		TakeHealth( 25, DMG_GENERIC );
	}

	gEvilImpulse101		= false;
}

void CSDKPlayer::Instructor_LessonLearned(const char* pszLesson)
{
	if (gpGlobals->eLoadType == MapLoad_Background)
		return;

	CSingleUserRecipientFilter filter( this );
	filter.MakeReliable();

	CDisablePredictionFiltering disabler(true);

	UserMessageBegin( filter, "LessonLearned" );
		WRITE_STRING( pszLesson );
	MessageEnd();
}

void CSDKPlayer::FlashlightTurnOn( void )
{
	AddEffects( EF_DIMLIGHT );
}

void CSDKPlayer::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
}

int CSDKPlayer::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

bool CSDKPlayer::ClientCommand( const CCommand &args )
{
	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		int iTeam = atoi( args[1] );
		HandleCommand_JoinTeam( iTeam );
		return true;
	}
	else if( !Q_strncmp( pcmd, "cls_", 4 ) )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		CSDKTeam *pTeam = GetGlobalSDKTeam( GetTeamNumber() );

		Assert( pTeam );

		int iClassIndex = PLAYERCLASS_UNDEFINED;

		if( pTeam->IsClassOnTeam( pcmd, iClassIndex ) )
		{
			HandleCommand_JoinClass( iClassIndex );
		}
		else
		{
			DevMsg( "player tried to join a class that isn't on this team ( %s )\n", pcmd );
			ShowClassSelectMenu();
		}
#endif
		return true;
	}
	else if ( FStrEq( pcmd, "spectate" ) )
	{
		// instantly join spectators
		HandleCommand_JoinTeam( TEAM_SPECTATOR );
		return true;
	}
	else if ( FStrEq( pcmd, "mapinfo" ) )
	{
		// player just closed MOTD dialog
		if ( m_iPlayerState == STATE_WELCOME )
		{
			State_Transition( STATE_PICKINGCHARACTER );
		}
		
		return true;
	}
	else if ( FStrEq( pcmd, "joingame" ) )
	{
		// player just closed MOTD dialog
		if ( m_iPlayerState == STATE_MAPINFO )
		{
			State_Transition( STATE_PICKINGCHARACTER );
		}

		return true;
	}
	else if ( FStrEq( pcmd, "joinclass" ) ) 
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad joinclass syntax\n" );
		}

		int iClass = atoi( args[1] );
		HandleCommand_JoinClass( iClass );
#endif
		return true;
	}
	else if ( FStrEq( pcmd, "menuopen" ) )
	{
		SetBuyMenuOpen( true );
		return true;
	}
	else if ( FStrEq( pcmd, "menuclosed" ) )
	{
		SetBuyMenuOpen( false );
		return true;
	}
	else if ( FStrEq( pcmd, "teammenuopen" ) )
	{
		SetTeamMenuOpen( true );
		return true;
	}
	else if ( FStrEq( pcmd, "teammenuclosed" ) )
	{
		SetTeamMenuOpen( false );

		if ( State_Get() != STATE_OBSERVER_MODE && (State_Get() == STATE_PICKINGTEAM || IsDead()) )
			State_Transition( STATE_BUYINGWEAPONS );

		return true;
	}
	else if ( FStrEq( pcmd, "charmenuopen" ) )
	{
		SetCharacterMenuOpen( true );
		return true;
	}
	else if ( FStrEq( pcmd, "charmenuclosed" ) )
	{
		SetCharacterMenuOpen( false );
		return true;
	}
	else if ( FStrEq( pcmd, "skillmenuopen" ) )
	{
		SetSkillMenuOpen( true );
		return true;
	}
	else if ( FStrEq( pcmd, "skillmenuclosed" ) )
	{
		SetSkillMenuOpen( false );
		return true;
	}
	else if ( FStrEq( pcmd, "droptest" ) )
	{
		ThrowActiveWeapon();
		return true;
	}

	return BaseClass::ClientCommand( args );
}

// returns true if the selection has been handled and the player's menu 
// can be closed...false if the menu should be displayed again
bool CSDKPlayer::HandleCommand_JoinTeam( int team )
{
// FIXME: TEMPORARY HACK TO NOT LET PEOPLE JOIN BLUE TEAM IN DEATHMATCH (shmopaloppa)
	if (!gpGlobals->teamplay && team == 2)
		team = 0;

	int iOldTeam = GetTeamNumber();
	if ( !GetGlobalTeam( team ) )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

#if defined ( SDK_USE_TEAMS )
	CSDKGameRules *mp = SDKGameRules();

	if ( mp->IsTeamplay() )
	{
		// If we already died and changed teams once, deny
		if( m_bTeamChanged && team != TEAM_SPECTATOR && iOldTeam != TEAM_SPECTATOR )
		{
			ClientPrint( this, HUD_PRINTCENTER, "game_switch_teams_once" );
			return true;
		}

		if ( team == TEAM_UNASSIGNED )
		{
			// Attempt to auto-select a team, may set team to T, CT or SPEC
			team = mp->SelectDefaultTeam();

			if ( team == TEAM_UNASSIGNED )
			{
				// still team unassigned, try to kick a bot if possible	
			 
				ClientPrint( this, HUD_PRINTTALK, "#All_Teams_Full" );

				team = TEAM_SPECTATOR;
			}
		}
	}
#endif

	if ( team == iOldTeam )
		return true;	// we wouldn't change the team

#if defined ( SDK_USE_TEAMS )
	if ( mp->TeamFull( team ) )
	{
		if ( team == SDK_TEAM_BLUE )
		{
			ClientPrint( this, HUD_PRINTTALK, "#BlueTeam_Full" );
		}
		else if ( team == SDK_TEAM_RED )
		{
			ClientPrint( this, HUD_PRINTTALK, "#RedTeam_Full" );
		}
		ShowViewPortPanel( PANEL_TEAM );
		return false;
	}
#endif

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this if the cvar is set
		if ( !mp_allowspectators.GetInt() && !IsHLTV() )
		{
			ClientPrint( this, HUD_PRINTTALK, "#Cannot_Be_Spectator" );
			ShowViewPortPanel( PANEL_TEAM );
			return false;
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	
	// If the code gets this far, the team is not TEAM_UNASSIGNED

	// Player is switching to a new team (It is possible to switch to the
	// same team just to choose a new appearance)
#if defined ( SDK_USE_TEAMS )
	if (mp->TeamStacked( team, GetTeamNumber() ))//players are allowed to change to their own team so they can just change their model
	{
		// The specified team is full
		ClientPrint( 
			this,
			HUD_PRINTCENTER,
			( team == SDK_TEAM_BLUE ) ?	"#BlueTeam_full" : "#RedTeam_full" );

		ShowViewPortPanel( PANEL_TEAM );
		return false;
	}
#endif
	// Switch their actual team...
	ChangeTeam( team );

#if defined ( SDK_USE_PLAYERCLASSES )
	// Force them to choose a new class
	m_Shared.SetDesiredPlayerClass( PLAYERCLASS_UNDEFINED );
	m_Shared.SetPlayerClass( PLAYERCLASS_UNDEFINED );
#endif
	return true;
}

#if defined ( SDK_USE_PLAYERCLASSES )
//Tony; we don't have to check anything special for SDK_USE_TEAMS here; it's all pretty generic, except for the one assert.
bool CSDKPlayer::HandleCommand_JoinClass( int iClass )
{
	Assert( GetTeamNumber() != TEAM_SPECTATOR );
#if defined ( SDK_USE_TEAMS )
	Assert( GetTeamNumber() != TEAM_UNASSIGNED );
#endif

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( iClass == PLAYERCLASS_UNDEFINED )
		return false;	//they typed in something weird

	int iOldPlayerClass = m_Shared.DesiredPlayerClass();

	// See if we're joining the class we already are
	if( iClass == iOldPlayerClass )
		return true;

	if( !SDKGameRules()->IsPlayerClassOnTeam( iClass, GetTeamNumber() ) )
		return false;

	const char *classname = SDKGameRules()->GetPlayerClassName( iClass, GetTeamNumber() );

	if( SDKGameRules()->CanPlayerJoinClass( this, iClass ) )
	{
		m_Shared.SetDesiredPlayerClass( iClass );	//real class value is set when the player spawns

//Tony; don't do this until we have a spawn timer!!
//		if( State_Get() == STATE_PICKINGCLASS )
//			State_Transition( STATE_OBSERVER_MODE );

		if( iClass == PLAYERCLASS_RANDOM )
		{
			if( IsAlive() )
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_respawn_asrandom" );
			}
			else
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_spawn_asrandom" );
			}
		}
		else
		{
			if( IsAlive() )
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_respawn_as", classname );
			}
			else
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_spawn_as", classname );
			}
		}

		IGameEvent * event = gameeventmanager->CreateEvent( "player_changeclass" );
		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetInt( "class", iClass );

			gameeventmanager->FireEvent( event );
		}
	}
	else
	{
		ClientPrint(this, HUD_PRINTTALK, "#game_class_limit", classname );
		ShowClassSelectMenu();
	}

	// Incase we don't get the class menu message before the spawn timer
	// comes up, fake that we've closed the menu.
	SetClassMenuOpen( false );

	//Tony; TODO; this is temp, I may integrate with the teamplayroundrules; If I do, there will be wavespawn too.
	if ( State_Get() == STATE_PICKINGCLASS /*|| IsDead()*/ )	//Tony; undone, don't transition if dead; only go into active state at this point if we were picking class.
		State_Transition( STATE_ACTIVE ); //Done picking stuff and we're in the pickingclass state, or dead, so we can spawn now.

	return true;
}

void CSDKPlayer::ShowClassSelectMenu()
{
#if defined ( SDK_USE_TEAMS )
	if ( GetTeamNumber() == SDK_TEAM_BLUE )
	{
		ShowViewPortPanel( PANEL_CLASS_BLUE );
	}
	else if ( GetTeamNumber() == SDK_TEAM_RED	)
	{
		ShowViewPortPanel( PANEL_CLASS_RED );
	}

#else
	if ( GetTeamNumber() == TEAM_UNASSIGNED )
		ShowViewPortPanel( PANEL_CLASS_NOTEAMS );
#endif
}
void CSDKPlayer::SetClassMenuOpen( bool bOpen )
{
	m_bIsClassMenuOpen = bOpen;
}

bool CSDKPlayer::IsClassMenuOpen( void )
{
	return m_bIsClassMenuOpen;
}
#endif // SDK_USE_PLAYERCLASSES

#ifdef SDK_USE_TEAMS
void CSDKPlayer::SetTeamMenuOpen( bool bOpen )
{
	m_bIsTeamMenuOpen = bOpen;
}

bool CSDKPlayer::IsTeamMenuOpen( void )
{
	return m_bIsTeamMenuOpen;
}

void CSDKPlayer::ShowTeamMenu()
{
	ShowViewPortPanel( PANEL_TEAM );
}
#endif

void CSDKPlayer::SetCharacterMenuOpen( bool bOpen )
{
	m_bIsCharacterMenuOpen = bOpen;
}

bool CSDKPlayer::IsCharacterMenuOpen( void )
{
	return m_bIsCharacterMenuOpen;
}

void CSDKPlayer::ShowCharacterMenu()
{
	ShowViewPortPanel( PANEL_CLASS );
}

void CSDKPlayer::SetBuyMenuOpen( bool bOpen )
{
	m_bIsBuyMenuOpen = bOpen;
}

bool CSDKPlayer::IsBuyMenuOpen( void )
{
	return m_bIsBuyMenuOpen;
}

void CSDKPlayer::ShowBuyMenu()
{
	ShowViewPortPanel( PANEL_BUY );
}

void CSDKPlayer::ClearLoadout()
{
	for (int i = 0; i < MAX_LOADOUT; i++)
		m_aLoadout.GetForModify(i).m_iCount.Set(0);

	CountLoadoutWeight();
}

void CSDKPlayer::AddToLoadout(SDKWeaponID eWeapon)
{
	if (!CanAddToLoadout(eWeapon))
		return;

	m_aLoadout.GetForModify(eWeapon).m_iCount++;

	CountLoadoutWeight();
}

void CSDKPlayer::RemoveFromLoadout(SDKWeaponID eWeapon)
{
	if (m_aLoadout[eWeapon].m_iCount <= 0)
		return;

	m_aLoadout.GetForModify(eWeapon).m_iCount--;

	CountLoadoutWeight();
}

void CSDKPlayer::CountLoadoutWeight()
{
	m_iLoadoutWeight = 0;

	// Skip WEAPON_NONE, start at 1
	for (int i = 1; i < MAX_LOADOUT; i++)
	{
		CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)i);

		if (!pWeaponInfo)
			continue;

		m_iLoadoutWeight += pWeaponInfo->iWeight * m_aLoadout[i].m_iCount;
	}
}

void CSDKPlayer::BuyRandom()
{
	ClearLoadout();

	// Buy a primary weapon
	SDKWeaponID eWeapon = WEAPON_NONE;
	do
	{
		eWeapon = (SDKWeaponID)random->RandomInt(WEAPON_NONE+1, WEAPON_MAX-1);
	}
	while (eWeapon == SDK_WEAPON_BRAWL || eWeapon == SDK_WEAPON_GRENADE || !CanAddToLoadout(eWeapon));

	AddToLoadout(eWeapon);

	// Buy a secondary weapon
	do
	{
		eWeapon = (SDKWeaponID)random->RandomInt(WEAPON_NONE+1, WEAPON_MAX-1);
	}
	while (eWeapon == SDK_WEAPON_BRAWL || eWeapon == SDK_WEAPON_GRENADE || !CanAddToLoadout(eWeapon));

	AddToLoadout(eWeapon);

	// Fill the rest up with grenades.
	while (CanAddToLoadout(SDK_WEAPON_GRENADE))
		AddToLoadout(SDK_WEAPON_GRENADE);
}

bool CSDKPlayer::PickRandomCharacter()
{
	// Count # of possible player models for random
	int i;
	for (i = 0; ; i++)
	{
		if (pszPossiblePlayerModels[i] == NULL)
			break;
	}

	if (SetCharacter(pszPossiblePlayerModels[random->RandomInt(0, i-1)]))
		return true;

	return false;
}

void CSDKPlayer::PickRandomSkill()
{
	SetStyleSkill((SkillID)random->RandomInt(SKILL_NONE+1, SKILL_LAST_CHOOSEABLE));
}

void CSDKPlayer::SelectItem(const char *pstr, int iSubType)
{
	bool bPreviousWeaponWasOutOfAmmo = false;
	if (GetActiveSDKWeapon() && !GetActiveSDKWeapon()->HasPrimaryAmmo())
		bPreviousWeaponWasOutOfAmmo = true;

	BaseClass::SelectItem(pstr, iSubType);

	// If we switched from a weapon that's out of ammo to a weapon with ammo then we train that lesson.
	if (bPreviousWeaponWasOutOfAmmo && GetActiveSDKWeapon() && GetActiveSDKWeapon()->HasPrimaryAmmo())
		Instructor_LessonLearned("outofammo");
	else
		Instructor_LessonLearned("switchweapons");
}

void CSDKPlayer::SetSkillMenuOpen( bool bOpen )
{
	m_bIsSkillMenuOpen = bOpen;
}

bool CSDKPlayer::IsSkillMenuOpen( void )
{
	return m_bIsCharacterMenuOpen;
}

void CSDKPlayer::ShowSkillMenu()
{
	ShowViewPortPanel( PANEL_BUY_EQUIP_CT );
}

void CSDKPlayer::SetStyleSkill(SkillID eSkill)
{
	int iOldSkill = m_Shared.m_iStyleSkill;

	if (eSkill <= SKILL_NONE || eSkill >= SKILL_MAX)
		eSkill = SKILL_MARKSMAN;

	m_Shared.m_iStyleSkill = eSkill;
	SetStylePoints(0);
	m_flStyleSkillCharge = 0;

	m_bHasSuperSlowMo = (m_Shared.m_iStyleSkill == SKILL_REFLEXES);

	if (iOldSkill == SKILL_REFLEXES)
		GiveSlowMo(-1);

	if (m_Shared.m_iStyleSkill == SKILL_REFLEXES)
		GiveSlowMo(1);

	if (m_Shared.m_iStyleSkill != SKILL_TROLL)
	{
		ConVarRef da_max_grenades("da_max_grenades");
		while (GetAmmoCount("grenades") > da_max_grenades.GetInt())
		{
			QAngle gunAngles;
			VectorAngles( BodyDirection2D(), gunAngles );

			Vector vecForward;
			AngleVectors( gunAngles, &vecForward, NULL, NULL );

			float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

			SDKThrowWeapon(FindWeapon(SDK_WEAPON_GRENADE), vecForward, gunAngles, flDiameter);
		}
	}
}

#if defined ( SDK_USE_PRONE )
//-----------------------------------------------------------------------------
// Purpose: Initialize prone at spawn.
//-----------------------------------------------------------------------------
void CSDKPlayer::InitProne( void )
{
	m_Shared.SetProne( false, true );
	m_bUnProneToDuck = false;
}
#endif // SDK_USE_PRONE

#if defined ( SDK_USE_SPRINTING )
void CSDKPlayer::InitSprinting( void )
{
	m_Shared.SetSprinting( false );
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we are allowed to sprint now.
//-----------------------------------------------------------------------------
bool CSDKPlayer::CanSprint()
{
	return ( 
		//!IsWalking() &&									// Not if we're walking
		!( m_Local.m_bDucked && !m_Local.m_bDucking ) &&	// Nor if we're ducking
		(GetWaterLevel() != 3) );							// Certainly not underwater
}
#endif // SDK_USE_SPRINTING
// ------------------------------------------------------------------------------------------------ //
// Player state management.
// ------------------------------------------------------------------------------------------------ //

void CSDKPlayer::State_Transition( SDKPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CSDKPlayer::State_Enter( SDKPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	if ( SDK_ShowStateTransitions.GetInt() == -1 || SDK_ShowStateTransitions.GetInt() == entindex() )
	{
		if ( m_pCurStateInfo )
			Msg( "ShowStateTransitions: entering '%s'\n", m_pCurStateInfo->m_pStateName );
		else
			Msg( "ShowStateTransitions: entering #%d\n", newState );
	}
	
	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CSDKPlayer::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CSDKPlayer::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CSDKPlayerStateInfo* CSDKPlayer::State_LookupInfo( SDKPlayerState state )
{
	// This table MUST match the 
	static CSDKPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CSDKPlayer::State_Enter_ACTIVE, NULL, &CSDKPlayer::State_PreThink_ACTIVE },
		{ STATE_WELCOME,		"STATE_WELCOME",		&CSDKPlayer::State_Enter_WELCOME, NULL, &CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_MAPINFO,        "STATE_MAPINFO",        &CSDKPlayer::State_Enter_MAPINFO, NULL, &CSDKPlayer::State_PreThink_MAPINFO },
#if defined ( SDK_USE_TEAMS )
		{ STATE_PICKINGTEAM,	"STATE_PICKINGTEAM",	&CSDKPlayer::State_Enter_PICKINGTEAM, NULL,	&CSDKPlayer::State_PreThink_WELCOME },
#endif
#if defined ( SDK_USE_PLAYERCLASSES )
		{ STATE_PICKINGCLASS,	"STATE_PICKINGCLASS",	&CSDKPlayer::State_Enter_PICKINGCLASS, NULL,	&CSDKPlayer::State_PreThink_WELCOME },
#endif
		{ STATE_PICKINGCHARACTER, "STATE_PICKINGCHARACTER",	&CSDKPlayer::State_Enter_PICKINGCHARACTER, NULL, &CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_BUYINGWEAPONS,	"STATE_BUYINGWEAPONS",	&CSDKPlayer::State_Enter_BUYINGWEAPONS, NULL, &CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_PICKINGSKILL,   "STATE_PICKINGSKILL",	&CSDKPlayer::State_Enter_PICKINGSKILL, NULL, &CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_DEATH_ANIM,		"STATE_DEATH_ANIM",		&CSDKPlayer::State_Enter_DEATH_ANIM,	NULL, &CSDKPlayer::State_PreThink_DEATH_ANIM },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CSDKPlayer::State_Enter_OBSERVER_MODE,	NULL, &CSDKPlayer::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}
void CSDKPlayer::PhysObjectSleep()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Sleep();
}


void CSDKPlayer::PhysObjectWake()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Wake();
}

void CSDKPlayer::State_Enter_WELCOME()
{
	// Important to set MOVETYPE_NONE or our physics object will fall while we're sitting at one of the intro cameras.
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	PhysObjectSleep();

	// Show info panel
	if ( IsBot() )
	{
		// If they want to auto join a team for debugging, pretend they clicked the button.
		CCommand args;
		args.Tokenize( "joingame" );
		ClientCommand( args );
	}
	else
	{
		const ConVar *hostname = cvar->FindVar( "hostname" );
		const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

		// open info panel on client showing MOTD:
		KeyValues *data = new KeyValues("data");
		data->SetString( "title", title );		// info panel title
		data->SetString( "type", "1" );			// show userdata from stringtable entry
		data->SetString( "msg",	"motd" );		// use this stringtable entry
		data->SetInt( "cmd", TEXTWINDOW_CMD_MAPINFO );// exec this command if panel closed

		ShowViewPortPanel( PANEL_INFO, true, data );

		data->deleteThis();
	}
}

void CSDKPlayer::State_Enter_MAPINFO()
{
	// Important to set MOVETYPE_NONE or our physics object will fall while we're sitting at one of the intro cameras.
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	PhysObjectSleep();

	ShowViewPortPanel( PANEL_INTRO, true );
}

void CSDKPlayer::MoveToNextIntroCamera()
{
	m_pIntroCamera = gEntList.FindEntityByClassname( m_pIntroCamera, "point_viewcontrol" );

	// if m_pIntroCamera is NULL we just were at end of list, start searching from start again
	if(!m_pIntroCamera)
		m_pIntroCamera = gEntList.FindEntityByClassname(m_pIntroCamera, "point_viewcontrol");

	// find the target
	CBaseEntity *Target = NULL;
	
	if( m_pIntroCamera )
	{
		Target = gEntList.FindEntityByName( NULL, STRING(m_pIntroCamera->m_target) );
	}

	// if we still couldn't find a camera, goto T spawn
	if(!m_pIntroCamera)
		m_pIntroCamera = gEntList.FindEntityByClassname(m_pIntroCamera, "info_player_deathmatch");

	SetViewOffset( vec3_origin );	// no view offset
	UTIL_SetSize( this, vec3_origin, vec3_origin ); // no bbox

	if( !Target ) //if there are no cameras(or the camera has no target, find a spawn point and black out the screen
	{
		if ( m_pIntroCamera.IsValid() )
			SetAbsOrigin( m_pIntroCamera->GetAbsOrigin() + VEC_VIEW );

		SetAbsAngles( QAngle( 0, 0, 0 ) );
		
		m_pIntroCamera = NULL;  // never update again
		return;
	}
	

	Vector vCamera = Target->GetAbsOrigin() - m_pIntroCamera->GetAbsOrigin();
	Vector vIntroCamera = m_pIntroCamera->GetAbsOrigin();
	
	VectorNormalize( vCamera );
		
	QAngle CamAngles;
	VectorAngles( vCamera, CamAngles );

	SetAbsOrigin( vIntroCamera );
	SetAbsAngles( CamAngles );
	SnapEyeAngles( CamAngles );
	m_fIntroCamTime = gpGlobals->curtime + 6;
}

void CSDKPlayer::State_PreThink_WELCOME()
{
	// Update whatever intro camera it's at.
	if( m_pIntroCamera && (gpGlobals->curtime >= m_fIntroCamTime) )
	{
		MoveToNextIntroCamera();
	}
}

void CSDKPlayer::State_PreThink_MAPINFO()
{
	// Update whatever intro camera it's at.
	if( m_pIntroCamera && (gpGlobals->curtime >= m_fIntroCamTime) )
	{
		MoveToNextIntroCamera();
	}
}

void CSDKPlayer::State_Enter_DEATH_ANIM()
{
	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}

	// Used for a timer.
	m_flDeathTime = gpGlobals->curtime;

	StartObserverMode( OBS_MODE_DEATHCAM );	// go to observer mode

	RemoveEffects( EF_NODRAW );	// still draw player body
}

extern ConVar spec_freeze_time;

#define SDK_DEATH_ANIMATION_TIME 0.0f

void CSDKPlayer::State_PreThink_DEATH_ANIM()
{
	// If the anim is done playing, go to the next state (waiting for a keypress to 
	// either respawn the guy or put him into observer mode).
	if ( GetFlags() & FL_ONGROUND )
	{
		float flForward = GetAbsVelocity().Length() - 20;
		if (flForward <= 0)
		{
			SetAbsVelocity( vec3_origin );
		}
		else
		{
			Vector vAbsVel = GetAbsVelocity();
			VectorNormalize( vAbsVel );
			vAbsVel *= flForward;
			SetAbsVelocity( vAbsVel );
		}
	}

	if (gpGlobals->curtime < m_flDeathTime + SDK_DEATH_ANIMATION_TIME)
		return;

	float flTimeInFreeze = spec_freeze_time.GetFloat();
	float flFreezeEnd = (m_flDeathTime + SDK_DEATH_ANIMATION_TIME + flTimeInFreeze );

	// Allow the player to spawn early if he wants.
	if (gpGlobals->curtime > m_flDeathTime + SDK_PLAYER_DEATH_TIME)
	{
		if ((m_nButtons & IN_ATTACK) && !(m_afButtonLast & IN_ATTACK))
		{
			State_Transition( STATE_ACTIVE );
			return;
		}
	}

	if ( gpGlobals->curtime < flFreezeEnd )
	{
		if ( GetObserverMode() != OBS_MODE_FREEZECAM )
		{
			StartObserverMode( OBS_MODE_FREEZECAM );
			PhysObjectSleep();
		}
		return;
	}

	if ( gpGlobals->curtime < flFreezeEnd )
		return;

	if ( gpGlobals->curtime >= (m_flDeathTime + SDK_PLAYER_DEATH_TIME ) )	// let the death cam stay going up to min spawn time.
	{
		m_lifeState = LIFE_DEAD;

		StopAnimation();

		AddEffects( EF_NOINTERP );

		if ( GetMoveType() != MOVETYPE_NONE && (GetFlags() & FL_ONGROUND) )
			SetMoveType( MOVETYPE_NONE );

		// make sure we don't have slide friction stuck on
		m_surfaceFriction = 1.0f;

		CSingleUserRecipientFilter user( this );
		enginesound->SetPlayerDSP( user, 0, false );
	}

	//Tony; if we're now dead, and not changing classes, spawn
	if ( m_lifeState == LIFE_DEAD )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		//Tony; if the class menu is open, don't respawn them, wait till they're done.
		if (IsClassMenuOpen())
			return;
#endif
		if (IsTeamMenuOpen())
			return;

		if (IsCharacterMenuOpen())
			return;

		if (IsBuyMenuOpen())
			return;

		if (IsSkillMenuOpen())
			return;

		State_Transition( STATE_OBSERVER_MODE );
	}
}

void CSDKPlayer::State_Enter_OBSERVER_MODE()
{
	m_nRenderFX = kRenderNormal;

	if( m_hObserverTarget == NULL )
	{
		// find a new observer target
		CheckObserverSettings();
	}

	// Change our observer target to the nearest teammate
	CTeam *pTeam = GetGlobalTeam( GetTeamNumber() );

	CBasePlayer *pPlayer;
	Vector localOrigin = GetAbsOrigin();
	Vector targetOrigin;
	float flMinDist = FLT_MAX;
	float flDist;

	for ( int i=0;i<pTeam->GetNumPlayers();i++ )
	{
		pPlayer = pTeam->GetPlayer(i);

		if ( !pPlayer )
			continue;

		if ( !IsValidObserverTarget(pPlayer) )
			continue;

		targetOrigin = pPlayer->GetAbsOrigin();

		flDist = ( targetOrigin - localOrigin ).Length();

		if ( flDist < flMinDist )
		{
			m_hObserverTarget.Set( pPlayer );
			flMinDist = flDist;
		}
	}

	StartObserverMode( m_iObserverLastMode );
	PhysObjectSleep();
}

void CSDKPlayer::State_PreThink_OBSERVER_MODE()
{
	if ((m_nButtons & IN_ATTACK) && !(m_afButtonLast & IN_ATTACK))
		State_Transition( STATE_ACTIVE );

	//Tony; if we're in eye, or chase, validate the target - if it's invalid, find a new one, or go back to roaming
	if (  m_iObserverMode == OBS_MODE_IN_EYE || m_iObserverMode == OBS_MODE_CHASE )
	{
		//Tony; if they're not on a spectating team use the cbaseplayer validation method.
		if ( GetTeamNumber() != TEAM_SPECTATOR )
			ValidateCurrentObserverTarget();
		else
		{
			if ( !IsValidObserverTarget( m_hObserverTarget.Get() ) )
			{
				// our target is not valid, try to find new target
				CBaseEntity * target = FindNextObserverTarget( false );
				if ( target )
				{
					// switch to new valid target
					SetObserverTarget( target );	
				}
				else
				{
					// let player roam around
					ForceObserverMode( OBS_MODE_ROAMING );
				}
			}
		}
	}
}

#if defined ( SDK_USE_PLAYERCLASSES )
void CSDKPlayer::State_Enter_PICKINGCLASS()
{
	ShowClassSelectMenu();
	PhysObjectSleep();

}
#endif // SDK_USE_PLAYERCLASSES

#if defined ( SDK_USE_TEAMS )
void CSDKPlayer::State_Enter_PICKINGTEAM()
{
	//ShowViewPortPanel( PANEL_TEAM );

	StopObserverMode();
	ShowTeamMenu();
	PhysObjectSleep();

}
#endif // SDK_USE_TEAMS

void CSDKPlayer::State_Enter_PICKINGCHARACTER()
{
	if (GetTeamNumber() == TEAM_SPECTATOR)
		HandleCommand_JoinTeam(TEAM_UNASSIGNED);

	StopObserverMode();
	ShowCharacterMenu();
	PhysObjectSleep();
}

void CSDKPlayer::State_Enter_BUYINGWEAPONS()
{
	if (GetTeamNumber() == TEAM_SPECTATOR)
		HandleCommand_JoinTeam(TEAM_UNASSIGNED);

	StopObserverMode();
	ShowBuyMenu();
	PhysObjectSleep();
}

void CSDKPlayer::State_Enter_PICKINGSKILL()
{
	if (GetTeamNumber() == TEAM_SPECTATOR)
		HandleCommand_JoinTeam(TEAM_UNASSIGNED);

	StopObserverMode();
	ShowSkillMenu();
	PhysObjectSleep();
}

void CSDKPlayer::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );
    m_Local.m_iHideHUD = 0;
	PhysObjectWake();

	//Tony; call spawn again now -- remember; when we add respawn timers etc, to just put them into the spawn queue, and let the queue respawn them.
	Spawn();
}

void CSDKPlayer::State_PreThink_ACTIVE()
{
	if (m_flDisarmRedraw > 0 && GetCurrentTime() > m_flDisarmRedraw)
	{
		m_flDisarmRedraw = -1;

		SwitchToNextBestWeapon( NULL );
	}

	if (IsAlive() && !m_Shared.m_bSuperFalling && (GetFlags() & FL_ONGROUND))
		m_bDamagedEnemyDuringFall = false;
}

void CSDKPlayer::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

bool CSDKPlayer::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	return BaseClass::WantsLagCompensationOnEntity( pPlayer, pCmd, pEntityTransmitBits );
}

// Don't automatically pick weapons up just because the player touched them. You clepto.
bool CSDKPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	return false;
}

ConVar da_disarmredraw("da_disarmredraw", "0.7", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "After how long does the player draw his next weapon after he's been disarmed?");

void CSDKPlayer::Disarm()
{
	CWeaponSDKBase* pActiveWeapon = GetActiveSDKWeapon();
	if (!pActiveWeapon)
		return;

	if (pActiveWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
		return;

	// Rifles can't be disarmed
	if (pActiveWeapon->GetWeaponType() == WT_RIFLE)
		return;

	// Shotguns can't be disarmed
	if (pActiveWeapon->GetWeaponType() == WT_SHOTGUN)
		return;

	ThrowActiveWeapon(false);

	m_flDisarmRedraw = GetCurrentTime() + da_disarmredraw.GetFloat();
}

CBaseEntity	*CSDKPlayer::GiveNamedItem( const char *pszName, int iSubType )
{
	CBaseEntity* pEnt;
	if ((pEnt = BaseClass::GiveNamedItem(pszName, iSubType)) != NULL)
	{
		CBaseCombatWeapon* pWeapon;
		if ((pWeapon = dynamic_cast<CBaseCombatWeapon*>( (CBaseEntity*)pEnt )) != NULL)
		{
			// GiveNamedItem uses BumpWeapon to give players weapons but BumpWeapon is overridden
			// not to pick up weapons when the player touches them, so we have to do it here
			// instead.

			// If I already have it just take the ammo
			if (Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType())) 
			{
				if( Weapon_EquipAmmoOnly( pWeapon ) )
				{
					// Only remove me if I have no ammo left
					if ( pWeapon->HasPrimaryAmmo() )
						return pEnt;

					UTIL_Remove( pWeapon );
					return NULL;
				}
				else
					return pEnt;
			}
			// -------------------------
			// Otherwise take the weapon
			// -------------------------
			else 
			{
				pWeapon->CheckRespawn();

				pWeapon->AddSolidFlags( FSOLID_NOT_SOLID );
				pWeapon->AddEffects( EF_NODRAW );

				Weapon_Equip( pWeapon );
				if ( IsInAVehicle() )
					pWeapon->Holster();

				return pWeapon;
			}
		}
	}

	return pEnt;
}

void CSDKPlayer::AddStylePoints(float points, style_sound_t eStyle, announcement_t eAnnouncement, style_point_t ePointStyle)
{
	points *= GetDKRatio(0.7, 2, true);

	m_flTotalStyle += points;

	if (IsStyleSkillActive())
	{
		float flDampenedPoints = RemapValClamped(points, 0, da_stylemeteractivationcost.GetFloat(), 0, da_stylemetertotalcharge.GetFloat());
		flDampenedPoints /= 2;
		m_flStyleSkillCharge = (m_flStyleSkillCharge+flDampenedPoints > da_stylemetertotalcharge.GetFloat()) ? da_stylemetertotalcharge.GetFloat() : m_flStyleSkillCharge+flDampenedPoints;
	}
	else
		m_flStylePoints += points;

	if (m_flStylePoints > da_stylemeteractivationcost.GetFloat())
		ActivateMeter();

	CSingleUserRecipientFilter filter( this );

	EmitSound_t params;
	params.m_pOrigin = NULL;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = true;

	params.m_nFlags |= SND_CHANGE_PITCH|SND_DELAY;
	if (IsStyleSkillActive())
		params.m_nPitch = RemapValClamped(m_flStyleSkillCharge, 0, da_stylemetertotalcharge.GetFloat(), 80, 120);
	else
		params.m_nPitch = RemapValClamped(m_flStylePoints, 0, da_stylemeteractivationcost.GetFloat(), 80, 120);

	params.m_nPitch += random->RandomInt(-5, 5);

	if (eStyle == STYLE_SOUND_KNOCKOUT)
	{
		params.m_nFlags = 0;
		params.m_pSoundName = "HudMeter.Knockout";
		EmitSound(filter, entindex(), params);
	}
	else if (eStyle == STYLE_SOUND_SMALL)
	{
		params.m_pSoundName = "HudMeter.FillSmall";
		EmitSound(filter, entindex(), params);
	}
	else if (eStyle == STYLE_SOUND_LARGE)
	{
		params.m_pSoundName = "HudMeter.FillLarge";
		EmitSound(filter, entindex(), params);
	}

	if (eAnnouncement != ANNOUNCEMENT_NONE)
		SendAnnouncement(eAnnouncement, ePointStyle, points);
}

void CSDKPlayer::SetStylePoints(float flPoints)
{
	if (flPoints < 0)
	{
		m_flStylePoints = 0;
		return;
	}

	if (flPoints > da_stylemeteractivationcost.GetFloat())
	{
		m_flStylePoints = da_stylemeteractivationcost.GetFloat();
		return;
	}

	m_flStylePoints = flPoints;
}

bool CSDKPlayer::UseStylePoints (void)
{
	if (m_flStylePoints >= da_stylemeteractivationcost.GetFloat())
	{
		m_flStylePoints -= da_stylemeteractivationcost.GetFloat();

		return true;
	}

	return false;
}

void CSDKPlayer::ActivateMeter()
{
	if (!UseStylePoints())
		return;

	if (!IsAlive())
		return;

	m_flStylePoints = 0;

	m_flStyleSkillCharge = da_stylemetertotalcharge.GetFloat();

	m_iStyleKillStreak = 0;

	CSingleUserRecipientFilter filter( this );
	EmitSound(filter, entindex(), "HudMeter.Activate");

	// Take 50 health.
	TakeHealth(50, 0);

	// If we're still less than max health, take up to make health.
	if (GetHealth() < GetMaxHealth())
		TakeHealth(GetMaxHealth() - GetHealth(), 0);

	if (m_Shared.m_iStyleSkill != SKILL_TROLL && m_Shared.m_iStyleSkill != SKILL_REFLEXES)
		CDove::SpawnDoves(this);

	if (m_Shared.m_bSuperSkill || SDKGameRules()->GetBountyPlayer() == this)
		return;

	// Refill ammo
	if (m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
	{
		for (int i = 0; i < WeaponCount(); i++)
		{
			CBaseCombatWeapon* pWeapon = GetWeapon(i);
			if (!pWeapon)
				continue;

			CWeaponSDKBase* pSDKWeapon = static_cast<CWeaponSDKBase*>(pWeapon);
			CSDKWeaponInfo* pInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)pSDKWeapon->GetWeaponID());

			if (pInfo)
			{
				if (!FStrEq(pInfo->szAmmo1, "grenades"))
					CBasePlayer::GiveAmmo( pInfo->iMaxClip1*pInfo->m_iDefaultAmmoClips, pInfo->szAmmo1);
			}
		}

		SendNotice(NOTICE_MARKSMAN);
	}
	else if (m_Shared.m_iStyleSkill == SKILL_TROLL)
	{
		if ( Weapon_OwnsThisType("weapon_grenade") )
			CBasePlayer::GiveAmmo(1, "grenades");
		else
			GiveNamedItem( "weapon_grenade" );

		SendNotice(NOTICE_TROLL);

		// They used the superslow, empty the style meter.
		m_flStyleSkillCharge = 0;
		SetStylePoints(0);
	}
	else if (m_Shared.m_iStyleSkill == SKILL_REFLEXES)
	{
		GiveSlowMo(2); // You get one from the kill so it's three total.

		SendNotice(NOTICE_SUPERSLO);

		// They used the superslow, empty the style meter.
		m_flStyleSkillCharge = 0;
		SetStylePoints(0);
	}
	else if (m_Shared.m_iStyleSkill == SKILL_RESILIENT)
		SendNotice(NOTICE_RESILIENT);
	else if (m_Shared.m_iStyleSkill == SKILL_ATHLETIC)
		SendNotice(NOTICE_ATHLETIC);
	else if (m_Shared.m_iStyleSkill == SKILL_BOUNCER)
		SendNotice(NOTICE_BOUNCER);
}

void CSDKPlayer::SetSlowMoType(int iType)
{
	if (iType != m_iSlowMoType)
		m_iSlowMoType = iType;
}

void CSDKPlayer::GiveSlowMo(float flSeconds)
{
	float flMaxSlow = 5;

	if (IsStyleSkillActive(SKILL_REFLEXES))
		flMaxSlow += 3;

	if (flSeconds > 0 && m_flSlowMoSeconds < flMaxSlow)
		SendNotice(NOTICE_SLOMO);

	m_flSlowMoSeconds = clamp(m_flSlowMoSeconds+flSeconds, 0, flMaxSlow);
}

void CSDKPlayer::PickUpBriefcase(CBriefcase* pBriefcase)
{
	DropBriefcase();

	pBriefcase->FollowEntity(this);
	pBriefcase->SetOwnerEntity(this);

	m_hBriefcase = pBriefcase;

	SendBroadcastNotice(NOTICE_PLAYER_HAS_BRIEFCASE, this);

	SendBroadcastSound("MiniObjective.BriefcasePickup");
}

void CSDKPlayer::SendBroadcastSound(const char* pszName)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		CSingleUserRecipientFilter filter( pPlayer );
		pPlayer->EmitSound(filter, pPlayer->entindex(), pszName);
	}
}

void CSDKPlayer::DropBriefcase()
{
	if (!m_hBriefcase)
		return;

	m_hBriefcase->FollowEntity(NULL);
	m_hBriefcase->SetOwnerEntity(NULL);

	m_hBriefcase->Dropped(this);

	m_hBriefcase = NULL;

	SendBroadcastSound("MiniObjective.BriefcaseDrop");
}

bool CSDKPlayer::CanDoCoderHacks()
{
	// Steam ID to account ID conversion:
	// STEAM_X:Y:Z -> 2Z+Y
	static AccountID_t aiCoders[] = {
		8238,     // STEAM_0:0:4119     Vino
		15210559, // STEAM_0:1:7605279  Dementei
	};

	int iSize = sizeof(aiCoders)/sizeof(int);

	CSteamID ID;
	GetSteamID(&ID);

	if (!ID.IsValid())
		return false;

	for (int i = 0; i < iSize; i++)
	{
		if (aiCoders[0] == ID.GetAccountID())
			return true;
	}

	return false;
}

void CSDKPlayer::CoderHacks(bool bOn)
{
	if (!CanDoCoderHacks())
		return;

	if (m_bCoderHacks == bOn)
		return;

	m_bCoderHacks = bOn;

	if (bOn)
		UTIL_SayText( "Coder hacks ON.\n", this );
	else
		UTIL_SayText( "Coder hacks OFF.\n", this );

	SDKGameRules()->CoderHacksUpdate();
}

void CSDKPlayer::ThirdPersonToggle()
{
	m_bThirdPerson = !m_bThirdPerson;
	Instructor_LessonLearned("thirdperson");
}

void CSDKPlayer::ThirdPersonSwitchSide()
{
	m_bThirdPersonCamSide = !m_bThirdPersonCamSide;
	Instructor_LessonLearned("thirdpersonswitch");
}

bool CSDKPlayer::InSameTeam( CBaseEntity *pEntity ) const
{
	if (SDKGameRules()->IsTeamplay())
		return BaseClass::InSameTeam(pEntity);

	// If we're not in teamplay it's every man for himself.
	return false;
}

bool CSDKPlayer::SetCharacter(const char* pszCharacter)
{
	for (int i = 0; ; i++)
	{
		if (pszPossiblePlayerModels[i] == NULL)
			break;

		if (FStrEq(pszCharacter, pszPossiblePlayerModels[i]))
		{
			char szCharacter[100];
			Q_strcpy(szCharacter, pszCharacter+14);
			szCharacter[strlen(szCharacter)-4] = '\0';
			m_iszCharacter = AllocPooledString(szCharacter);
			return true;
		}

		if (FStrEq(UTIL_VarArgs("models/player/%s.mdl", pszCharacter), pszPossiblePlayerModels[i]))
		{
			m_iszCharacter = AllocPooledString(pszCharacter);
			return true;
		}
	}

	return false;
}
extern CMoveData *g_pMoveData;
extern ConVar sv_maxspeed;
void CSDKPlayer::SetVCollisionState( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, int collisionState )
{
	IPhysicsObject *hulls[] =
	{/*TODO: Just put these all into a member table?*/
		m_pShadowStand,
		m_pShadowCrouch,
		shadow_slide,
		shadow_dive
	};
	static const char *hs[] =
	{
		"Stand",
		"Crouch",
		"Slide",
		"Dive",
		"Noclip"
	};
	Assert (collisionState <= VPHYS_NOCLIP);
	if (collisionState != VPHYS_NOCLIP)
	{
		
		IPhysicsObject *o = hulls[collisionState];
		if (m_vphysicsCollisionState != VPHYS_NOCLIP)
		{
			hulls[m_vphysicsCollisionState]->EnableCollisions (false);
		}
		o->SetPosition (vecAbsOrigin, vec3_angle, true);
		o->SetVelocity (&vecAbsVelocity, NULL);
		o->EnableCollisions (true);
		m_pPhysicsController->SetObject (o);
		VPhysicsSwapObject (o);
		//Msg ("%s -> %s\n", hs[m_vphysicsCollisionState], hs[collisionState]);
	}
	else
	{
		int i;
		for (i = 0; i < VPHYS_NOCLIP; i++)
		{
			hulls[i]->EnableCollisions (false);
		}
	}
	m_vphysicsCollisionState = collisionState;
}
void CSDKPlayer::PostThinkVPhysics (void)
{
	int collisionState;
	// Check to see if things are initialized!
	if ( !m_pPhysicsController )
		return;

	Vector newPosition = GetAbsOrigin();
	float frametime = gpGlobals->frametime;
	if ( frametime <= 0 || frametime > 0.1f )
		frametime = 0.1f;

	IPhysicsObject *pPhysGround = GetGroundVPhysics();
	if ( !pPhysGround && m_touchedPhysObject && g_pMoveData->m_outStepHeight <= 0.f && (GetFlags() & FL_ONGROUND) )
	{
		newPosition = m_oldOrigin + frametime * g_pMoveData->m_outWishVel;
		newPosition = (GetAbsOrigin() * 0.5f) + (newPosition * 0.5f);
	}
	/*Decide what bbox to use*/
	if (GetMoveType() == MOVETYPE_NOCLIP || GetMoveType() == MOVETYPE_OBSERVER)
	{
		collisionState = VPHYS_NOCLIP;
	}
	else if (GetFlags ()&FL_ONGROUND)
	{
		if (m_Shared.IsSliding () || m_Shared.IsDiveSliding () || m_Shared.IsProne ())
		{
			collisionState = VPHYS_SLIDE;
		}
		else if ((GetFlags()&FL_DUCKING)) collisionState = VPHYS_CROUCH;
		else collisionState = VPHYS_WALK;
	}
	else if (m_Shared.IsDiving ())
	{
		collisionState = VPHYS_DIVE;
	}
	else 
	{/*Prevents slide going into stand when flying off edge*/
		collisionState = m_vphysicsCollisionState;
	}

	if ( collisionState != m_vphysicsCollisionState )
	{
		SetVCollisionState( GetAbsOrigin(), GetAbsVelocity(), collisionState );
	}

	if ( !(TouchedPhysics() || pPhysGround) )
	{
		float maxSpeed = m_Shared.m_flRunSpeed > 0.0f ? m_Shared.m_flRunSpeed : sv_maxspeed.GetFloat();
		g_pMoveData->m_outWishVel.Init( maxSpeed, maxSpeed, maxSpeed );
	}

	// teleport the physics object up by stepheight (game code does this - reflect in the physics)
	if ( g_pMoveData->m_outStepHeight > 0.1f )
	{
		if ( g_pMoveData->m_outStepHeight > 4.0f )
		{
			VPhysicsGetObject()->SetPosition( GetAbsOrigin(), vec3_angle, true );
		}
		else
		{
			// don't ever teleport into solid
			Vector position, end;
			VPhysicsGetObject()->GetPosition( &position, NULL );
			end = position;
			end.z += g_pMoveData->m_outStepHeight;
			trace_t trace;
			UTIL_TraceEntity( this, position, end, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );
			if ( trace.DidHit() )
			{
				g_pMoveData->m_outStepHeight = trace.endpos.z - position.z;
			}
			m_pPhysicsController->StepUp( g_pMoveData->m_outStepHeight );
		}
		m_pPhysicsController->Jump();
	}
	g_pMoveData->m_outStepHeight = 0.0f;
	
	// Store these off because after running the usercmds, it'll pass them
	// to UpdateVPhysicsPosition.	
	m_vNewVPhysicsPosition = newPosition;
	m_vNewVPhysicsVelocity = g_pMoveData->m_outWishVel;

	m_oldOrigin = GetAbsOrigin();
}
void CSDKPlayer::SetupVPhysicsShadow( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, CPhysCollide *pStandModel, const char *pStandHullName, CPhysCollide *pCrouchModel, const char *pCrouchHullName )
{
	solid_t solid;
	Q_strncpy( solid.surfaceprop, "player", sizeof(solid.surfaceprop) );
	solid.params = g_PhysDefaultObjectParams;
	solid.params.mass = 85.0f;
	solid.params.inertia = 1e24f;
	solid.params.enableCollisions = false;
	solid.params.dragCoefficient = 0;

	// create standing hull
	m_pShadowStand = PhysModelCreateCustom( this, pStandModel, GetLocalOrigin(), GetLocalAngles(), pStandHullName, false, &solid );
	m_pShadowStand->SetCallbackFlags( CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION );
	// create crouchig hull
	m_pShadowCrouch = PhysModelCreateCustom( this, pCrouchModel, GetLocalOrigin(), GetLocalAngles(), pCrouchHullName, false, &solid );
	m_pShadowCrouch->SetCallbackFlags( CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION );
#if 1
	CPhysCollide *box;
	box = PhysCreateBbox (VEC_SLIDE_HULL_MIN, VEC_SLIDE_HULL_MAX);
	shadow_slide = PhysModelCreateCustom (this, box, GetLocalOrigin(), GetLocalAngles(), "player_slide", false, &solid);
	shadow_slide->SetCallbackFlags (CALLBACK_GLOBAL_COLLISION|CALLBACK_SHADOW_COLLISION);

	box = PhysCreateBbox (VEC_DIVE_HULL_MIN, VEC_DIVE_HULL_MAX);
	shadow_dive = PhysModelCreateCustom (this, box, GetLocalOrigin(), GetLocalAngles(), "player_dive", false, &solid);
	shadow_dive->SetCallbackFlags (CALLBACK_GLOBAL_COLLISION|CALLBACK_SHADOW_COLLISION);
#endif
	// default to stand
	VPhysicsSetObject( m_pShadowStand );

	// tell physics lists I'm a shadow controller object
	PhysAddShadow( this );	
	m_pPhysicsController = physenv->CreatePlayerController( m_pShadowStand );
	m_pPhysicsController->SetPushMassLimit( 350.0f );
	m_pPhysicsController->SetPushSpeedLimit( 50.0f );
	
	// Give the controller a valid position so it doesn't do anything rash.
	UpdatePhysicsShadowToPosition( vecAbsOrigin );

	// init state
	if ( GetFlags() & FL_DUCKING )
	{
		SetVCollisionState( vecAbsOrigin, vecAbsVelocity, VPHYS_CROUCH );
	}
	else
	{
		SetVCollisionState( vecAbsOrigin, vecAbsVelocity, VPHYS_WALK );
	}
}
void CSDKPlayer::VPhysicsDestroyObject ()
{
	IPhysicsObject *hulls[] =
	{/*TODO: Just put these all into a member table?*/
		m_pShadowStand,
		m_pShadowCrouch,
		shadow_slide,
		shadow_dive
	};
	int i;
	// Since CBasePlayer aliases its pointer to the physics object, tell CBaseEntity to 
	// clear out its physics object pointer so we don't wind up deleting one of
	// the aliased objects twice.
	VPhysicsSetObject (NULL);
	PhysRemoveShadow (this);
	if (m_pPhysicsController)
	{
		physenv->DestroyPlayerController (m_pPhysicsController);
		m_pPhysicsController = NULL;
	}
	for (i = 0; i < VPHYS_NOCLIP; i++)
	{
		IPhysicsObject *o = hulls[i];
		if (o)
		{
			o->EnableCollisions (false);
			PhysDestroyObject(o);
		}
	}
	/*FIXME: Are these even needed?
	Does this function get called many times?*/
	m_pShadowStand = NULL;
	m_pShadowCrouch = NULL;
	shadow_slide = NULL;
	shadow_dive = NULL;

	BaseClass::VPhysicsDestroyObject();
}

float CSDKPlayer::GetUserInfoFloat(const char* pszCVar, float flBotDefault)
{
	if (IsBot())
		return flBotDefault;

	float flCVarValue = atof(engine->GetClientConVarValue( entindex(), pszCVar ));

	return flCVarValue;
}

int CSDKPlayer::GetUserInfoInt(const char* pszCVar, int iBotDefault)
{
	if (IsBot())
		return iBotDefault;

	int iCVarValue = atoi(engine->GetClientConVarValue( entindex(), pszCVar ));

	return iCVarValue;
}

float CSDKPlayer::GetDKRatio(float flMin, float flMax, bool bDampen) const
{
	float flDeathRatio = m_iDeaths;

	// D/K means that a player with a lot of deaths will get a higher ratio.
	if (m_iKills > 0)
		flDeathRatio = (float)m_iDeaths/(float)m_iKills;

	flDeathRatio = clamp(flDeathRatio, 0.7f, 2);

	if (bDampen)
	{
		// Dampen the ratio if there's not enough data.
		float flRatioWeight = RemapValClamped(m_iKills + m_iDeaths, 0, 10, 0, 1);

		return (flDeathRatio - 1) * flRatioWeight + 1;
	}
	else
		return flDeathRatio;
}

void CC_ActivateSlowmo_f (void)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	pPlayer->Instructor_LessonLearned("slowmo");

	if (pPlayer->m_flSlowMoSeconds > 0)
		pPlayer->ActivateSlowMo();
}

static ConCommand activateslowmo("activateslowmo", CC_ActivateSlowmo_f, "Activate slow motion." );

void CC_Character(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	if (args.ArgC() == 1)
	{
		if (pPlayer->IsAlive())
			pPlayer->ShowCharacterMenu();
		else
			pPlayer->State_Transition( STATE_PICKINGCHARACTER );

		return;
	}

	if (args.ArgC() == 2)
	{
		pPlayer->StopObserverMode();

		if (pPlayer->SetCharacter(args[1]))
		{
			if (!pPlayer->IsBot())
				DataManager().AddCharacterChosen(args[1]);

			if ( pPlayer->State_Get() != STATE_OBSERVER_MODE && (pPlayer->State_Get() == STATE_PICKINGCHARACTER || pPlayer->IsDead()) )
			{
				if (gpGlobals->teamplay)
					pPlayer->State_Transition( STATE_PICKINGTEAM );
				else
					pPlayer->State_Transition( STATE_BUYINGWEAPONS );
			}

			return;
		}

		if (FStrEq(args[1], "random"))
		{
			if (pPlayer->PickRandomCharacter())
			{
				if ( pPlayer->State_Get() != STATE_OBSERVER_MODE && (pPlayer->State_Get() == STATE_PICKINGCHARACTER || pPlayer->IsDead()) )
				{
					if (gpGlobals->teamplay)
						pPlayer->State_Transition( STATE_PICKINGTEAM );
					else
						pPlayer->State_Transition( STATE_BUYINGWEAPONS );
				}

				return;
			}
		}

		Error("Couldn't find that player model.\n");
	}
}

static ConCommand character("character", CC_Character, "Choose a character.", FCVAR_GAMEDLL);

void CC_Buy(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 
	SDKWeaponID eWeapon;

	if (!pPlayer)
		return;

	if (args.ArgC() == 1)
	{
		pPlayer->Instructor_LessonLearned("buy");

		if (pPlayer->IsAlive())
			pPlayer->ShowBuyMenu();
		else
			pPlayer->State_Transition( STATE_BUYINGWEAPONS );

		return;
	}

	if (Q_strncmp(args[1], "clear", 5) == 0)
	{
		pPlayer->StopObserverMode();
		pPlayer->ClearLoadout();
		return;
	}

	if (Q_strncmp(args[1], "random", 6) == 0)
	{
		pPlayer->StopObserverMode();
		pPlayer->BuyRandom();
		return;
	}

	if (args.ArgC() == 3 && Q_strncmp(args[1], "remove", 6) == 0)
	{
		eWeapon = AliasToWeaponID(args[2]);
		pPlayer->StopObserverMode();
		pPlayer->RemoveFromLoadout(eWeapon);
		return;
	}

	eWeapon = AliasToWeaponID(args[1]);
	if (eWeapon)
	{
		pPlayer->StopObserverMode();
		pPlayer->AddToLoadout(eWeapon);

		if (!pPlayer->IsBot())
			DataManager().AddWeaponChosen(eWeapon);

		return;
	}
}

static ConCommand buy("buy", CC_Buy, "Buy things.", FCVAR_GAMEDLL);

void CC_Skill(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	if (args.ArgC() == 1)
	{
		if (pPlayer->IsAlive())
			pPlayer->ShowSkillMenu();
		else
			pPlayer->State_Transition( STATE_PICKINGSKILL );

		return;
	}

	if (FStrEq(args[1], "random"))
	{
		pPlayer->PickRandomSkill();

		if ( pPlayer->State_Get() != STATE_OBSERVER_MODE && (pPlayer->State_Get() == STATE_PICKINGSKILL || pPlayer->IsDead()) )
			pPlayer->State_Transition( STATE_ACTIVE );

		return;
	}

	pPlayer->SetStyleSkill(AliasToSkillID(args[1]));

	if (!pPlayer->IsBot())
		DataManager().AddSkillChosen(AliasToSkillID(args[1]));

	if ( pPlayer->State_Get() != STATE_OBSERVER_MODE && (pPlayer->State_Get() == STATE_PICKINGSKILL || pPlayer->IsDead()) )
		pPlayer->State_Transition( STATE_ACTIVE );
}

static ConCommand skill("setskill", CC_Skill, "Open the skill menu.", FCVAR_GAMEDLL);

void CC_Drop(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	if (pPlayer->ThrowActiveWeapon())
		pPlayer->Instructor_LessonLearned("throw");
}

static ConCommand drop("drop", CC_Drop, "Drop the weapon the player currently holds.", FCVAR_GAMEDLL);

void CC_GiveSlowMo(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	pPlayer->GiveSlowMo(1);
}

static ConCommand give_slowmo("give_slowmo", CC_GiveSlowMo, "Give the player one second of slow motion.", FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CC_GiveStyle(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	pPlayer->AddStylePoints(10, STYLE_SOUND_SMALL, ANNOUNCEMENT_COOL, STYLE_POINT_SMALL);
}

static ConCommand give_style("give_style", CC_GiveStyle, "Give the player some style to his bar.", FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CC_DisarmMe(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	pPlayer->Disarm();
}

static ConCommand disarmme("disarmme", CC_DisarmMe, "Disarm the player as a test.", FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CC_ThirdPersonToggle(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	pPlayer->ThirdPersonToggle();
}

static ConCommand cam_thirdperson_toggle( "cam_thirdperson_toggle", ::CC_ThirdPersonToggle, "Toggle third person mode.", FCVAR_GAMEDLL );

void CC_ThirdPersonSwitchSide(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	pPlayer->ThirdPersonSwitchSide();
}

static ConCommand cam_thirdperson_switchside( "cam_thirdperson_switch", ::CC_ThirdPersonSwitchSide, "Switch third person camera position.", FCVAR_GAMEDLL );

void CC_HealMe_f(const CCommand &args)
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	int iDamage = 10;
	if ( args.ArgC() >= 2 )
	{
		iDamage = atoi( args[ 1 ] );
	}

	pPlayer->TakeHealth( iDamage, DMG_GENERIC );
}
static ConCommand healme("healme", CC_HealMe_f, "heals the player.\n\tArguments: <health to heal>", FCVAR_CHEAT);

void CC_CoderHacks_f(const CCommand &args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	if (args.ArgC() < 3)
		return;

	if (!FStrEq(args[0], "coder") || !FStrEq(args[1], "hacks"))
		return;

	if (FStrEq(args[2], "on"))
		pPlayer->CoderHacks(true);
	else if (FStrEq(args[2], "off"))
		pPlayer->CoderHacks(false);
	else if (FStrEq(args[2], "changelevel") && pPlayer->CanDoCoderHacks() && args.ArgC() >= 4)
	{
		ConVarRef nextlevel("nextlevel");
		nextlevel.SetValue(args[3]);
		SDKGameRules()->ChangeLevel();
	}
	else if (FStrEq(args[2], "list") && pPlayer->CanDoCoderHacks())
	{
		for (int i = 1; i < gpGlobals->maxClients; i++)
		{
			CSDKPlayer* pSDKPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));

			if (!pSDKPlayer)
				continue;

			CSteamID ID;
			pSDKPlayer->GetSteamID(&ID);

			if (ID.IsValid())
				UTIL_SayText(UTIL_VarArgs("%s (%d)\n", pSDKPlayer->GetPlayerName(), ID.GetAccountID()), pSDKPlayer);
			else
				UTIL_SayText(UTIL_VarArgs("%s (INVALID)\n", pSDKPlayer->GetPlayerName()), pSDKPlayer);
		}
	}
	else if (FStrEq(args[2], "da_ctb_changecap") && pPlayer->CanDoCoderHacks() && args.ArgC() >= 4)
	{
		ConVarRef da_ctb_changecap("da_ctb_changecap");
		da_ctb_changecap.SetValue(args[3]);
	}
}

static ConCommand coder("coder", CC_CoderHacks_f, "C0d3r h4c|<s", FCVAR_HIDDEN);

CON_COMMAND( give_weapon, "Give weapon to player.\n\tArguments: <weapon_name>" )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( pPlayer 
		&& (gpGlobals->maxClients == 1 || sv_cheats->GetBool()) 
		&& args.ArgC() >= 2 )
	{
		char item_to_give[ 256 ];
		Q_strncpy( item_to_give, args[1], sizeof( item_to_give ) );
		Q_strlower( item_to_give );

		string_t iszItem = AllocPooledString( item_to_give );	// Make a copy of the classname

		if (strncmp(item_to_give, "weapon_", 7) == 0)
		{
			CWeaponSDKBase* pDrop = (CWeaponSDKBase*)pPlayer->Weapon_Create(item_to_give);
			pDrop->VPhysicsDestroyObject(); 
			pDrop->VPhysicsInitShadow(true, true);

			Vector vecForward;
			pPlayer->GetVectors(&vecForward, NULL, NULL);

			pDrop->SetAbsOrigin(pPlayer->EyePosition() + vecForward * 40);

			pDrop->Drop( vecForward * 40 );
			pDrop->SetRemoveable( false );

			pDrop->SetDieThink( true );
		}
	}
}

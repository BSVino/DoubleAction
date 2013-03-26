//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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

#include "da_ammo_pickup.h"

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
	SendPropFloat( SENDINFO( m_flDiveLerped ) ),
	SendPropBool( SENDINFO( m_bAimedIn ) ),
	SendPropFloat( SENDINFO( m_flAimIn ) ),
	SendPropFloat( SENDINFO( m_flSlowAimIn ) ),
	SendPropInt( SENDINFO( m_iStyleSkill ) ),
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
END_SEND_TABLE()


// main table
IMPLEMENT_SERVERCLASS_ST( CSDKPlayer, DT_SDKPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	
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
	SendPropStringT( SENDINFO( m_iszCharacter ) ),
END_SEND_TABLE()

class CSDKRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CSDKRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

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

	m_flCurrentTime = gpGlobals->curtime;

	m_iszCharacter = NULL_STRING;
}


CSDKPlayer::~CSDKPlayer()
{
	DestroyRagdoll();
	m_PlayerAnimState->Release();
}


CSDKPlayer *CSDKPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CSDKPlayer::s_PlayerEdict = ed;
	return (CSDKPlayer*)CreateEntityByName( className );
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

ConVar dab_regenamount( "dab_regenamount", "5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player regenerate each tick?" );
ConVar dab_decayamount( "dab_decayamount", "1", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player decay each tick, when total health is greater than max?" );
ConVar dab_regenamount_secondwind( "dab_regenamount_secondwind", "10", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does a player with the second wind style skill regenerate each tick?" );

void CSDKPlayer::PreThink(void)
{
	m_vecTotalBulletForce = vec3_origin;

	if (IsInThirdPerson())
		UpdateThirdCamera(Weapon_ShootPosition(), EyeAngles() + GetPunchAngle());

	UpdateCurrentTime();

	if (IsAlive())
	{
		if (!IsStyleSkillActive() && m_flCurrentTime > m_flNextHealthDecay && GetHealth() > GetMaxHealth())
		{
			m_iHealth -= dab_decayamount.GetFloat();

			m_flNextHealthDecay = m_flCurrentTime + 1;
		}

		if (m_flCurrentTime > m_flNextRegen)
		{
			float flRatio = dab_regenamount_secondwind.GetFloat()/dab_regenamount.GetFloat();
			float flModifier = (flRatio - 1)/2;
			float flHealth = m_Shared.ModifySkillValue(dab_regenamount.GetFloat(), flModifier, SKILL_RESILIENT);

			m_flNextRegen = m_flCurrentTime + 1;

			// Heal up to 50% of the player's health.
			int iMaxHealth = GetMaxHealth()/2;

			if (IsStyleSkillActive(SKILL_RESILIENT))
				// If Resilient is active, heal up to 100%, which is actually 200 health
				iMaxHealth = GetMaxHealth();
			else if (m_Shared.m_iStyleSkill == SKILL_RESILIENT)
				// If it's passive heal higher than 100%
				iMaxHealth = GetMaxHealth() * 1.25f;

			int iHealthTaken = 0;
			if (GetHealth() < iMaxHealth)
				iHealthTaken = TakeHealth(min(flHealth, iMaxHealth - GetHealth()), 0);

			UseStyleCharge(SKILL_RESILIENT, iHealthTaken*2/3);
		}
	}

	if (IsAlive() && (m_Shared.IsDiving() || m_Shared.IsRolling() || m_Shared.IsSliding() || !GetGroundEntity()))
	{
		Vector vecNormalizedVelocity = GetAbsVelocity();
		vecNormalizedVelocity.NormalizeInPlace();

		trace_t	tr;
		UTIL_TraceHull(GetAbsOrigin() + Vector(0, 0, 5), GetAbsOrigin() + vecNormalizedVelocity*40 + Vector(0, 0, 5), Vector(-16, -16, -16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

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

		if (!pHit && vecNormalizedVelocity.z < 0 && (m_Shared.IsDiving() || !GetGroundEntity()))
		{
			UTIL_TraceHull(GetAbsOrigin() + Vector(0, 0, 5), GetAbsOrigin() - Vector(0, 0, 5), Vector(-16, -16, -16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

			if (tr.fraction < 1.0f && bIsBreakable)
				pHit = tr.m_pEnt;
		}

		if (pHit)
			pHit->TakeDamage(CTakeDamageInfo(this, this, 50, DMG_CRUSH));
	}

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

ConVar dab_stylemetertime( "dab_stylemetertime", "10", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How long does the style meter remain active after activation?" );

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

		for (int i = 0; i < MAX_LOADOUT; i++)
		{
			if (m_aLoadout[i].m_iCount)
			{
				Q_snprintf( szName, sizeof( szName ), "weapon_%s", WeaponIDToAlias((SDKWeaponID)i) );
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
			if (!GetWeapon(i))
				continue;

			if (GetWeapon(i) == GetActiveWeapon())
				continue;

			if (!dynamic_cast<CWeaponSDKBase*>(GetWeapon(i)))
				continue;

			if (static_cast<CWeaponSDKBase*>(GetWeapon(i))->GetWeaponID() == SDK_WEAPON_BRAWL)
				continue;

			Weapon_SetLast(GetWeapon(i));
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

	m_bHasSuperSlowMo = (m_Shared.m_iStyleSkill == SKILL_REFLEXES);
	if (m_Shared.m_iStyleSkill == SKILL_REFLEXES)
		GiveSlowMo(1);

	SDKGameRules()->CalculateSlowMoForPlayer(this);

	m_flLastSpawnTime = gpGlobals->curtime;
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

	DevMsg("CSDKPlayer::SelectSpawnSpot: couldn't find valid spawn point.\n");

	return true;
}


CBaseEntity* CSDKPlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pSpot = NULL;

	const char *pSpawnPointName = "";

	switch( GetTeamNumber() )
	{
#if defined ( SDK_USE_TEAMS )
	case SDK_TEAM_BLUE:
		{
			pSpawnPointName = "info_player_blue";
			pSpot = g_pLastBlueSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastBlueSpawn = pSpot;
			}
		}
		break;
	case SDK_TEAM_RED:
		{
			pSpawnPointName = "info_player_red";
			pSpot = g_pLastRedSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastRedSpawn = pSpot;
			}
		}		
		break;
#endif // SDK_USE_TEAMS
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
		State_Transition( STATE_ACTIVE );
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
	
	m_iSuicideCustomKillFlags = SDK_DMG_CUSTOM_SUICIDE;

	BaseClass::CommitSuicide( bExplode, bForce );
}

void CSDKPlayer::InitialSpawn( void )
{
	m_bThirdPerson = !!atoi(engine->GetClientConVarValue( entindex(), "cl_thirdperson" ));

	BaseClass::InitialSpawn();

	if (gpGlobals->eLoadType == MapLoad_Background)
		State_Enter( STATE_OBSERVER_MODE );
	else
		State_Enter( STATE_WELCOME );

	ClearLoadout();
}

void CSDKPlayer::OnDive()
{
	m_bDamagedEnemyDuringDive = false;
}

void CSDKPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
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
	BaseClass::TraceAttack( inputInfo, vecDir, ptr );
}

int CSDKPlayer::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	CBaseEntity *pInflictor = info.GetInflictor();

	if ( !pInflictor )
		return 0;

	if ( GetMoveType() == MOVETYPE_NOCLIP || GetMoveType() == MOVETYPE_OBSERVER )
		return 0;

	m_vecTotalBulletForce += info.GetDamageForce();

	float flArmorBonus = 0.5f;
	float flArmorRatio = 0.5f;
	float flDamage = info.GetDamage();

	bool bCheckFriendlyFire = false;
	bool bFriendlyFire = friendlyfire.GetBool();
	//Tony; only check teams in teamplay
	if ( gpGlobals->teamplay )
		bCheckFriendlyFire = true;

/*	if (IsStyleSkillActive(SKILL_IMPERVIOUS))
	{
		UseStyleCharge(flDamage * 0.2f);
		flDamage = m_Shared.ModifySkillValue(flDamage, 0.3f, SKILL_IMPERVIOUS);
	}*/

	if ( !(bFriendlyFire || ( bCheckFriendlyFire && pInflictor->GetTeamNumber() != GetTeamNumber() ) /*|| pInflictor == this ||	info.GetAttacker() == this*/ ) )
	{
		if ( bFriendlyFire && (info.GetDamageType() & DMG_BLAST) == 0 )
		{
			if ( pInflictor->GetTeamNumber() == GetTeamNumber() && bCheckFriendlyFire)
			{
				flDamage *= 0.35; // bullets hurt teammates less
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

		pAttackerSDK->AwardStylePoints(this, false, info);

		// If the player is stunting and managed to damage another player while stunting, he's trained the be stylish hint.
		if (pAttackerSDK->m_Shared.IsDiving() || pAttackerSDK->m_Shared.IsSliding() || pAttackerSDK->m_Shared.IsRolling())
			pAttackerSDK->Instructor_LessonLearned("be_stylish");

		pAttackerSDK->m_bDamagedEnemyDuringDive = true;
	}

	if (m_Shared.m_iStyleSkill != SKILL_RESILIENT)
		m_flNextRegen = m_flCurrentTime + 10;
	else if (!IsStyleSkillActive())
		m_flNextRegen = m_flCurrentTime + 6;
	else
		m_flNextRegen = m_flCurrentTime + 3;

	return 1;
}

ConVar dab_stylemeteractivationcost( "dab_stylemeteractivationcost", "25", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much (out of 100) does it cost to activate your style meter?" );
ConVar dab_stylemetertotalcharge( "dab_stylemetertotalcharge", "100", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "What is the total charge given when the style meter is activated?" );

void CSDKPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	StopSound( "Player.GoSlide" );

	if (FStrEq(info.GetInflictor()->GetClassname(), "trigger_hurt") && m_Shared.IsDiving() && m_bDamagedEnemyDuringDive)
	{
		SendNotice(NOTICE_WORTHIT);
		AddStylePoints(10, STYLE_POINT_STYLISH);
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

	while (ThrowActiveWeapon());

	CAmmoPickup* pAmmoPickup = static_cast<CAmmoPickup*>(CBaseEntity::Create( "da_ammo_pickup", GetAbsOrigin(), GetAbsAngles() ));

	if (!pAmmoPickup->TakeAmmoFromPlayer(this))
		UTIL_Remove(pAmmoPickup);

	pAmmoPickup->SetThink(&CAmmoPickup::SUB_Remove);
	pAmmoPickup->SetNextThink(gpGlobals->curtime + 30);

	CBaseEntity* pAttacker = info.GetAttacker();

	// show killer in death cam mode
	// chopped down version of SetObserverTarget without the team check
	if( pAttacker && pAttacker->IsPlayer() )
	{
		// set new target
		m_hObserverTarget.Set( pAttacker ); 

		// reset fov to default
		SetFOV( this, 0 );

		CSDKPlayer* pAttackerSDK = ToSDKPlayer(pAttacker);

		if (pAttacker != this)
		{
			pAttackerSDK->AwardStylePoints(this, true, info);
			pAttackerSDK->GiveSlowMo(1);
		}
	}
	else
		m_hObserverTarget.Set( NULL );

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
		float flUnused = m_flStyleSkillCharge/dab_stylemetertotalcharge.GetFloat();
		float flRefund = flUnused*dab_stylemeteractivationcost.GetFloat();
		SetStylePoints(m_flStylePoints + flRefund);
	}

	// Losing a whole activation can be rough, let's be a bit more forgiving.
	// Going down with lots of bar drops you more than going down with just a little bar.
	// This way, running around with your bar full you run a high risk.
	float flActivationCost = dab_stylemeteractivationcost.GetFloat();

	if (m_Shared.IsDiving() || m_Shared.IsSliding() || m_Shared.IsRolling())
		// Lose less bar if you die during a stunt. Going out with style is never a bad thing!
		SetStylePoints(m_flStylePoints - RemapValClamped(m_flStylePoints, flActivationCost/4, flActivationCost, flActivationCost/16, flActivationCost/4));
	else
		SetStylePoints(m_flStylePoints - RemapValClamped(m_flStylePoints, flActivationCost/4, flActivationCost, flActivationCost/8, flActivationCost/2));

	// Turn off slow motion.
	m_flSlowMoSeconds = 0;
	m_flSlowMoTime = 0;
	m_iSlowMoType = SLOWMO_NONE;

	m_bHasPlayerDied = true;

	SDKGameRules()->PlayerSlowMoUpdate(this);
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
		// Give me half of the activation cost. The other half I'll get from damaging.
		flPoints = dab_stylemeteractivationcost.GetFloat()/2;
	}
	else
	{
		// Damaging someone stylishly for 100% of their health is enough to get one bar.
		// The player will get half the points from damaging and the other half from killing.
		flPoints = RemapValClamped(info.GetDamage(), 0, 100, 0, dab_stylemeteractivationcost.GetFloat()/2);
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
		AddStylePoints(flPoints*0.8f, bKilledVictim?STYLE_POINT_STYLISH:STYLE_POINT_LARGE);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_GRENADE_KILL, STYLE_POINT_STYLISH);
		else
			SendAnnouncement(ANNOUNCEMENT_GRENADE, STYLE_POINT_LARGE);

		if (!IsAlive() && bKilledVictim)
		{
			AddStylePoints(flPoints, STYLE_POINT_STYLISH);
			SendNotice(NOTICE_WORTHIT);
		}
	}
	else if (bKilledVictim && GetActiveSDKWeapon() && pWeaponInfo->m_eWeaponType > WT_MELEE && GetActiveSDKWeapon()->m_iClip1 == 0 && info.GetDamageType() != DMG_CLUB)
	{
		// Killing a player with your last bullet.
		AddStylePoints(flPoints, STYLE_POINT_STYLISH);
		SendAnnouncement(ANNOUNCEMENT_LAST_BULLET, STYLE_POINT_STYLISH);
	}
	else if (bKilledVictim && pVictim->LastHitGroup() == HITGROUP_HEAD && vecVictimForward.Dot(vecKillerToVictim) < 0.3f && flDistance < 100)
	{
		// Killing a player by shooting in the head from behind.
		AddStylePoints(flPoints, STYLE_POINT_STYLISH);
		SendAnnouncement(ANNOUNCEMENT_EXECUTION, STYLE_POINT_STYLISH);
	}
	else if (pVictim->LastHitGroup() == HITGROUP_HEAD)
	{
		AddStylePoints(flPoints, STYLE_POINT_STYLISH);
		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_HEADSHOT, STYLE_POINT_STYLISH);
		else
			SendAnnouncement(ANNOUNCEMENT_HEADSHOT, STYLE_POINT_LARGE);
	}
	else if (m_Shared.IsDiving() || m_Shared.IsSliding())
	{
		// Damaging a dude enough to kill him while stunting gives a full bar.
		AddStylePoints(flPoints, bKilledVictim?STYLE_POINT_STYLISH:STYLE_POINT_LARGE);

		if (m_Shared.IsDiving())
		{
			if (info.GetDamageType() == DMG_CLUB)
			{
				if (bKilledVictim)
					SendAnnouncement(ANNOUNCEMENT_DIVEPUNCH, STYLE_POINT_STYLISH);
				else
					SendAnnouncement(ANNOUNCEMENT_DIVEPUNCH, STYLE_POINT_LARGE);
			}
			else
			{
				if (bKilledVictim)
					SendAnnouncement(ANNOUNCEMENT_DIVE_KILL, STYLE_POINT_STYLISH);
				else
					SendAnnouncement(ANNOUNCEMENT_DIVE, STYLE_POINT_LARGE);
			}
		}
		else
		{
			if (info.GetDamageType() == DMG_CLUB)
			{
				if (bKilledVictim)
					SendAnnouncement(ANNOUNCEMENT_SLIDEPUNCH, STYLE_POINT_STYLISH);
				else
					SendAnnouncement(ANNOUNCEMENT_SLIDEPUNCH, STYLE_POINT_LARGE);
			}
			else
			{
				if (bKilledVictim)
					SendAnnouncement(ANNOUNCEMENT_SLIDE_KILL, STYLE_POINT_STYLISH);
				else
					SendAnnouncement(ANNOUNCEMENT_SLIDE, STYLE_POINT_LARGE);
			}
		}
	}
	else if (flDistance < 60 && info.GetDamageType() != DMG_CLUB)
	{
		AddStylePoints(flPoints*0.6f, bKilledVictim?STYLE_POINT_LARGE:STYLE_POINT_SMALL);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_POINT_BLANK, STYLE_POINT_LARGE);
		else
			SendAnnouncement(ANNOUNCEMENT_POINT_BLANK, STYLE_POINT_SMALL);
	}
	else if (flDistance > 1200)
	{
		// Long range.
		AddStylePoints(flPoints*0.6f, bKilledVictim?STYLE_POINT_LARGE:STYLE_POINT_SMALL);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_LONG_RANGE_KILL, STYLE_POINT_LARGE);
		else
			SendAnnouncement(ANNOUNCEMENT_LONG_RANGE, STYLE_POINT_SMALL);
	}
	else if (!(info.GetDamageType() & DMG_DIRECT) && (info.GetDamageType() & DMG_BULLET))
	{
		// Damaging a dude through a wall with a firearm.
		AddStylePoints(flPoints*0.6f, bKilledVictim?STYLE_POINT_LARGE:STYLE_POINT_SMALL);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_THROUGH_WALL, STYLE_POINT_LARGE);
		else
			SendAnnouncement(ANNOUNCEMENT_THROUGH_WALL, STYLE_POINT_SMALL);
	}
	else if (m_Shared.IsRolling())
	{
		// Rolling, which is easier to do and typically happens after the dive, gives only half.
		AddStylePoints(flPoints*0.5f, bKilledVictim?STYLE_POINT_STYLISH:STYLE_POINT_LARGE);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_STUNT_KILL, STYLE_POINT_STYLISH);
		else
			SendAnnouncement(ANNOUNCEMENT_STUNT, STYLE_POINT_LARGE);
	}
	else if (info.GetDamageType() == DMG_CLUB)
	{
		// Brawl
		AddStylePoints(flPoints*0.5f, bKilledVictim?STYLE_POINT_STYLISH:STYLE_POINT_LARGE);

		if (bKilledVictim)
			SendAnnouncement(ANNOUNCEMENT_BRAWL_KILL, STYLE_POINT_STYLISH);
		else
			SendAnnouncement(ANNOUNCEMENT_BRAWL, STYLE_POINT_LARGE);
	}
	else
	{
		if (pVictim->m_Shared.IsDiving() || pVictim->m_Shared.IsRolling() || pVictim->m_Shared.IsSliding())
			// Damaging a stunting dude gives me slightly more bar than usual.
			AddStylePoints(flPoints*0.3f, bKilledVictim?STYLE_POINT_LARGE:STYLE_POINT_SMALL);
		else
			AddStylePoints(flPoints*0.2f, bKilledVictim?STYLE_POINT_LARGE:STYLE_POINT_SMALL);

		// Only some chance of sending a message at all.
		if (bKilledVictim)
		{
			int iRandom = random->RandomInt(0, 1);
			announcement_t eAnnouncement;
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

			if (m_Shared.IsAimedIn() && pWeaponInfo->m_bAimInSpeedPenalty)
				eAnnouncement = ANNOUNCEMENT_TACTICOOL;

			if (bKilledVictim && m_flSlowMoMultiplier < 1)
				eAnnouncement = ANNOUNCEMENT_SLOWMO_KILL;

			if (bKilledVictim)
				SendAnnouncement(eAnnouncement, STYLE_POINT_LARGE);
			else
				SendAnnouncement(eAnnouncement, STYLE_POINT_SMALL);
		}
	}
}

void CSDKPlayer::SendAnnouncement(announcement_t eAnnouncement, style_point_t ePointStyle)
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	// Start the message block
	UserMessageBegin( user, "StyleAnnouncement" );

		// Send our text to the client
		WRITE_LONG( eAnnouncement );
		WRITE_BYTE( ePointStyle );

		if (IsStyleSkillActive())
			WRITE_FLOAT( m_flStyleSkillCharge/dab_stylemetertotalcharge.GetFloat() );
		else
			WRITE_FLOAT( GetStylePoints()/dab_stylemeteractivationcost.GetFloat() );

	// End the message block
	MessageEnd();
}

void CSDKPlayer::SendNotice(notice_t eNotice)
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	UserMessageBegin( user, "Notice" );
		WRITE_LONG( eNotice );
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

ConVar dab_resilient_health_bonus( "dab_resilient_health_bonus", "100", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much health does the player regenerate each tick?" );

int CSDKPlayer::GetMaxHealth() const
{
	if (IsStyleSkillActive(SKILL_RESILIENT))
		return BaseClass::GetMaxHealth() + dab_resilient_health_bonus.GetInt();

	return BaseClass::GetMaxHealth();
}

bool CSDKPlayer::ThrowActiveWeapon( bool bAutoSwitch )
{
	CWeaponSDKBase *pWeapon = (CWeaponSDKBase *)GetActiveWeapon();

	if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
		return false;

	if( pWeapon && pWeapon->CanWeaponBeDropped() )
	{
		QAngle gunAngles;
		VectorAngles( BodyDirection2D(), gunAngles );

		Vector vecForward;
		AngleVectors( gunAngles, &vecForward, NULL, NULL );

		float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

		pWeapon->SetWeaponVisible( false );
		pWeapon->Holster(NULL);

		SDKThrowWeapon( pWeapon, vecForward, gunAngles, flDiameter );

		if (bAutoSwitch)
			SwitchToNextBestWeapon( NULL );

		return true;
	}

	return false;
}

bool CSDKPlayer::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	if (GetCurrentTime() < m_flDisarmRedraw)
		return false;

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

void CSDKPlayer::SDKThrowWeapon( CWeaponSDKBase *pWeapon, const Vector &vecForward, const QAngle &vecAngles, float flDiameter  )
{
	Vector vecOrigin;
	CollisionProp()->RandomPointInBounds( Vector( 0.5f, 0.5f, 0.5f ), Vector( 0.5f, 0.5f, 1.0f ), &vecOrigin );

	// Nowhere in particular; just drop it.
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
	Weapon_Detach( pWeapon );

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
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
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
	return dynamic_cast< CWeaponSDKBase* >( GetActiveWeapon() );
}

void CSDKPlayer::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "dab_viewmodel" );
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
	else if ( FStrEq( pcmd, "joingame" ) )
	{
		// player just closed MOTD dialog
		if ( m_iPlayerState == STATE_WELCOME )
		{
//Tony; using teams, go to picking team.
#if defined( SDK_USE_TEAMS )
			State_Transition( STATE_PICKINGTEAM );
//Tony; not using teams, but we are using classes, so go straight to class picking.
#elif !defined ( SDK_USE_TEAMS ) && defined ( SDK_USE_PLAYERCLASSES )
			State_Transition( STATE_PICKINGCLASS );
//Tony; not using teams or classes, go straight to active.
#else
			State_Transition( STATE_PICKINGCHARACTER );
#endif
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

		if ( State_Get() != STATE_OBSERVER_MODE && (State_Get() == STATE_BUYINGWEAPONS || IsDead()) )
			State_Transition( STATE_PICKINGSKILL );

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

		if ( State_Get() != STATE_OBSERVER_MODE && (State_Get() == STATE_PICKINGCHARACTER || IsDead()) )
			State_Transition( STATE_BUYINGWEAPONS );

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

		if ( State_Get() != STATE_OBSERVER_MODE && (State_Get() == STATE_PICKINGSKILL || IsDead()) )
			State_Transition( STATE_ACTIVE );

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
	int iOldTeam = GetTeamNumber();
	if ( !GetGlobalTeam( team ) )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

#if defined ( SDK_USE_TEAMS )
	// If we already died and changed teams once, deny
	if( m_bTeamChanged && team != TEAM_SPECTATOR && iOldTeam != TEAM_SPECTATOR )
	{
		ClientPrint( this, HUD_PRINTCENTER, "game_switch_teams_once" );
		return true;
	}

	CSDKGameRules *mp = SDKGameRules();
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
	SetStyleSkill((SkillID)random->RandomInt(SKILL_NONE+1, SKILL_MAX-1));
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
		data->SetString( "cmd", "joingame" );// exec this command if panel closed

		ShowViewPortPanel( PANEL_INFO, true, data );

		data->deleteThis();

	}	
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
		m_pIntroCamera = gEntList.FindEntityByClassname(m_pIntroCamera, "info_player_terrorist");

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
extern ConVar spec_freeze_traveltime;

#define SDK_DEATH_ANIMATION_TIME 0.5f

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

	float flTimeInFreeze = spec_freeze_traveltime.GetFloat() + spec_freeze_time.GetFloat();
	float flFreezeEnd = (m_flDeathTime + SDK_DEATH_ANIMATION_TIME + flTimeInFreeze );

	if ( GetObserverTarget() && GetObserverTarget() != this )
	{
		if ( gpGlobals->curtime < flFreezeEnd )
		{
			if ( GetObserverMode() != OBS_MODE_FREEZECAM )
			{
				StartObserverMode( OBS_MODE_FREEZECAM );
				PhysObjectSleep();
			}
			return;
		}
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
	}

	//Tony; if we're now dead, and not changing classes, spawn
	if ( m_lifeState == LIFE_DEAD )
	{
#if defined ( SDK_USE_PLAYERCLASSES )
		//Tony; if the class menu is open, don't respawn them, wait till they're done.
		if (IsClassMenuOpen())
			return;
#endif

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
	ShowViewPortPanel( PANEL_TEAM );
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
}

int CSDKPlayer::GetPlayerStance()
{
#if defined ( SDK_USE_PRONE )
	if (m_Shared.IsProne() || ( m_Shared.IsGoingProne() || m_Shared.IsGettingUpFromProne() ))
		return PINFO_STANCE_PRONE;
#endif

#if defined ( SDK_USE_SPRINTING )
	if (IsSprinting())
		return PINFO_STANCE_SPRINTING;
#endif
	if (m_Local.m_bDucking)
		return PINFO_STANCE_DUCKING;
	else
		return PINFO_STANCE_STANDING;
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

ConVar dab_disarmredraw("dab_disarmredraw", "0.7", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "After how long does the player draw his next weapon after he's been disarmed?");

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

	m_flDisarmRedraw = GetCurrentTime() + dab_disarmredraw.GetFloat();
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

void CSDKPlayer::AddStylePoints(float points, style_point_t eStyle)
{
	if (IsStyleSkillActive())
	{
		points = RemapValClamped(points, 0, dab_stylemeteractivationcost.GetFloat(), 0, dab_stylemetertotalcharge.GetFloat());
		points /= 2;
		m_flStyleSkillCharge = (m_flStyleSkillCharge+points > dab_stylemetertotalcharge.GetFloat()) ? dab_stylemetertotalcharge.GetFloat() : m_flStyleSkillCharge+points;
	}
	else
		m_flStylePoints = (m_flStylePoints+points > 100) ? 100 : m_flStylePoints+points;

	if (m_flStylePoints > dab_stylemeteractivationcost.GetFloat())
	{
		ActivateMeter();
		return;
	}

	CSingleUserRecipientFilter filter( this );
	if (eStyle == STYLE_POINT_SMALL)
		EmitSound(filter, entindex(), "HudMeter.FillSmall");
	else if (eStyle == STYLE_POINT_LARGE)
		EmitSound(filter, entindex(), "HudMeter.FillLarge");
	else if (eStyle == STYLE_POINT_STYLISH)
		EmitSound(filter, entindex(), "HudMeter.FillStylish");
}

void CSDKPlayer::SetStylePoints(float flPoints)
{
	if (flPoints < 0)
	{
		m_flStylePoints = 0;
		return;
	}

	if (flPoints > 100)
	{
		m_flStylePoints = 100;
		return;
	}

	m_flStylePoints = flPoints;
}

bool CSDKPlayer::UseStylePoints (void)
{
	if (m_flStylePoints >= dab_stylemeteractivationcost.GetFloat())
	{
		m_flStylePoints -= dab_stylemeteractivationcost.GetFloat();

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

	m_flStyleSkillCharge = dab_stylemetertotalcharge.GetFloat();

	m_iStyleKillStreak = 0;

	CSingleUserRecipientFilter filter( this );
	EmitSound(filter, entindex(), "HudMeter.Activate");

	// Take 50 health.
	TakeHealth(50, 0);

	// If we're still less than max health, take up to make health.
	if (GetHealth() < GetMaxHealth())
		TakeHealth(GetMaxHealth() - GetHealth(), 0);

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
		GiveNamedItem( "weapon_grenade" );

		SendNotice(NOTICE_TROLL);
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

void CSDKPlayer::ThirdPersonToggle()
{
	m_bThirdPerson = !m_bThirdPerson;
	Instructor_LessonLearned("thirdperson");
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
			return;

		if (FStrEq(args[1], "random"))
		{
			if (pPlayer->PickRandomCharacter())
				return;
		}

		Error("Couldn't find that player model.\n");
	}
}

static ConCommand character("character", CC_Character, "Choose a character.", FCVAR_GAMEDLL);

void CC_Buy(const CCommand& args)
{
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_GetCommandClient() ); 

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
		pPlayer->StopObserverMode();
		pPlayer->RemoveFromLoadout(AliasToWeaponID(args[2]));
		return;
	}

	SDKWeaponID eWeapon = AliasToWeaponID(args[1]);
	if (eWeapon)
	{
		pPlayer->StopObserverMode();
		pPlayer->AddToLoadout(eWeapon);
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
		return;
	}

	pPlayer->SetStyleSkill(AliasToSkillID(args[1]));
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

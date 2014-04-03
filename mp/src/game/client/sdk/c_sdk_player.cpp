//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_sdk_player.h"
#include "c_user_message_register.h"
#include "weapon_sdkbase.h"
#include "c_basetempentity.h"
#include "iclientvehicle.h"
#include "prediction.h"
#include "view.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "view_scene.h"
#include "fx.h"
#include "collisionutils.h"
#include "c_sdk_team.h"
#include "obstacle_pushaway.h"
#include "bone_setup.h"
#include "cl_animevent.h"
#include "input.h"
#include <baseviewport.h>
#include "clienteffectprecachesystem.h"
#include "model_types.h"
#include "sdk_gamerules.h"
#include "projectedlighteffect.h"
#include "voice_status.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "in_buttons.h"
#include "cam_thirdperson.h"
#include "sourcevr/isourcevirtualreality.h"
#include "engine/ivdebugoverlay.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "functionproxy.h"

#include "da_buymenu.h"
#include "sdk_teammenu.h"
#include "da_charactermenu.h"
#include "da_skillmenu.h"
#include "da_viewmodel.h"
#include "da_viewback.h"

// memdbgon must be the last include file in a .cpp file!!!
ConVar cl_ragdoll_physics_enable( "cl_ragdoll_physics_enable", "1", 0, "Enable/disable ragdoll physics." );
#include "tier0/memdbgon.h"

#if defined( CSDKPlayer )
	#undef CSDKPlayer
#endif




// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

void __MsgFunc_ReloadEffect( bf_read &msg )
{
	int iPlayer = msg.ReadShort();
	C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( C_BaseEntity::Instance( iPlayer ) );
	if ( pPlayer )
		pPlayer->PlayReloadEffect();

}
USER_MESSAGE_REGISTER( ReloadEffect );

// CSDKPlayerShared Data Tables
//=============================

// specific to the local player ( ideally should not be in CSDKPlayerShared! )
BEGIN_RECV_TABLE_NOBASE( CSDKPlayerShared, DT_SDKSharedLocalPlayerExclusive )
#if defined ( SDK_USE_PLAYERCLASSES )
	RecvPropInt( RECVINFO( m_iPlayerClass ) ),
	RecvPropInt( RECVINFO( m_iDesiredPlayerClass ) ),
#endif
END_RECV_TABLE()

void RecvProxy_Skill( const CRecvProxyData *pData, void *pStruct, void *pOut );

BEGIN_RECV_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerShared )
#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	RecvPropFloat( RECVINFO( m_flStamina ) ),
#endif

#if defined ( SDK_USE_PRONE )
	RecvPropBool( RECVINFO( m_bProne ) ),
	RecvPropTime( RECVINFO( m_flGoProneTime ) ),
	RecvPropTime( RECVINFO( m_flUnProneTime ) ),
	RecvPropTime( RECVINFO( m_flDisallowUnProneTime ) ),
	RecvPropBool( RECVINFO( m_bProneSliding ) ),
#endif
#if defined( SDK_USE_SPRINTING )
	RecvPropBool( RECVINFO( m_bIsSprinting ) ),
#endif
	RecvPropBool( RECVINFO( m_bSliding ) ),
	RecvPropBool( RECVINFO( m_bInAirSlide ) ),
	RecvPropVector( RECVINFO(m_vecSlideDirection) ),
	RecvPropTime( RECVINFO(m_flSlideTime) ),
	RecvPropTime( RECVINFO( m_flUnSlideTime ) ),
	RecvPropBool( RECVINFO( m_bMustDuckFromSlide ) ),
	RecvPropBool( RECVINFO( m_bDiveSliding ) ),
	RecvPropVector( RECVINFO(m_vecUnSlideEyeStartOffset) ),
	RecvPropTime( RECVINFO( m_flLastDuckPress ) ),
	RecvPropBool( RECVINFO( m_bRolling ) ),
	RecvPropBool( RECVINFO( m_bRollingFromDive ) ),
	RecvPropVector( RECVINFO(m_vecRollDirection) ),
	RecvPropTime( RECVINFO(m_flRollTime) ),
	RecvPropBool( RECVINFO( m_bDiving ) ),
	RecvPropVector( RECVINFO(m_vecDiveDirection) ),
	RecvPropBool( RECVINFO( m_bRollAfterDive ) ),
	RecvPropTime( RECVINFO(m_flDiveTime) ),
	RecvPropTime( RECVINFO(m_flTimeLeftGround) ),
	RecvPropFloat( RECVINFO(m_flDiveLerped) ),
	RecvPropFloat( RECVINFO(m_flDiveToProneLandTime) ),
	RecvPropBool( RECVINFO( m_bAimedIn ) ),
	RecvPropFloat( RECVINFO( m_flAimIn ) ),
	RecvPropFloat( RECVINFO( m_flSlowAimIn ) ),
	RecvPropInt( RECVINFO( m_iStyleSkill ), 0, RecvProxy_Skill ),
	RecvPropBool( RECVINFO( m_bSuperSkill ) ),
	RecvPropDataTable( "sdksharedlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_SDKSharedLocalPlayerExclusive) ),

	RecvPropInt (RECVINFO (m_iWallFlipCount)),
	RecvPropBool (RECVINFO (m_bIsWallFlipping)),
	RecvPropFloat (RECVINFO (m_flWallFlipEndTime)),
	RecvPropBool (RECVINFO (m_bIsManteling)),
	RecvPropVector (RECVINFO (m_vecMantelWallNormal)),

	RecvPropBool( RECVINFO( m_bSuperFalling ) ),
	RecvPropBool( RECVINFO( m_bSuperFallOthersVisible ) ),
	RecvPropTime( RECVINFO( m_flSuperFallOthersNextCheck ) ),
END_RECV_TABLE()

void RecvProxy_Loadout( const CRecvProxyData *pData, void *pStruct, void *pOut );
void RecvProxy_Character( const CRecvProxyData *pData, void *pStruct, void *pOut );

BEGIN_RECV_TABLE_NOBASE(CArmament, DT_Loadout)
	RecvPropInt(RECVINFO(m_iCount), 0, RecvProxy_Loadout),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_SDKPlayer, DT_SDKLocalPlayerExclusive )
	RecvPropInt( RECVINFO( m_iShotsFired ) ),
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropFloat( RECVINFO_NAME( m_angEyeAngles.x, m_angEyeAngles[0] ) ),
//	RecvPropFloat( RECVINFO_NAME( m_angEyeAngles.y, m_angEyeAngles[1] ) ),
	RecvPropInt( RECVINFO( m_ArmorValue ) ),

	RecvPropTime		( RECVINFO( m_flFreezeUntil ) ),
	RecvPropFloat		( RECVINFO( m_flFreezeAmount ) ),

	RecvPropFloat		( RECVINFO( m_flDisarmRedraw ) ),

	RecvPropArray3( RECVINFO_ARRAY(m_aLoadout), RecvPropDataTable(RECVINFO_DTNAME(m_aLoadout[0],m_aLoadout),0, &REFERENCE_RECV_TABLE(DT_Loadout)) ),
	RecvPropInt( RECVINFO( m_iLoadoutWeight ), 0, RecvProxy_Loadout ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_SDKPlayer, DT_SDKNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropFloat( RECVINFO_NAME( m_angEyeAngles.x, m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO_NAME( m_angEyeAngles.y, m_angEyeAngles[1] ) ),

	RecvPropFloat( RECVINFO_NAME(m_vecViewOffset.x, m_vecViewOffset[0]) ),
	RecvPropFloat( RECVINFO_NAME(m_vecViewOffset.y, m_vecViewOffset[1]) ),
	RecvPropFloat( RECVINFO_NAME(m_vecViewOffset.z, m_vecViewOffset[2]) ),
END_RECV_TABLE()

// main table
IMPLEMENT_CLIENTCLASS_DT( C_SDKPlayer, DT_SDKPlayer, CSDKPlayer )
	RecvPropDataTable( RECVINFO_DT( m_Shared ), 0, &REFERENCE_RECV_TABLE( DT_SDKPlayerShared ) ),

	RecvPropDataTable( "sdklocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_SDKLocalPlayerExclusive) ),
	RecvPropDataTable( "sdknonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_SDKNonLocalPlayerExclusive) ),

	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),

	RecvPropInt( RECVINFO( m_iPlayerState ) ),

	RecvPropBool( RECVINFO( m_bSpawnInterpCounter ) ),

	RecvPropInt( RECVINFO( m_flStylePoints ) ),
	RecvPropFloat( RECVINFO(m_flStyleSkillCharge) ),

	RecvPropInt( RECVINFO( m_iSlowMoType ) ),
	RecvPropBool( RECVINFO( m_bHasSuperSlowMo ) ),
	RecvPropFloat		( RECVINFO( m_flSlowMoSeconds ) ),
	RecvPropFloat		( RECVINFO( m_flSlowMoTime ) ),
	RecvPropFloat		( RECVINFO( m_flSlowMoMultiplier ) ),
	RecvPropFloat		( RECVINFO( m_flCurrentTime ) ),
	RecvPropFloat		( RECVINFO( m_flLastSpawnTime ) ),
	RecvPropTime		( RECVINFO( m_flReadyWeaponUntil ) ),

	RecvPropBool( RECVINFO( m_bHasPlayerDied ) ),
	RecvPropBool( RECVINFO( m_bThirdPerson ) ),
	RecvPropBool( RECVINFO( m_bThirdPersonCamSide ) ),	

	RecvPropBool( RECVINFO( m_bUsingVR ) ),

	RecvPropString( RECVINFO( m_iszCharacter ), 0, RecvProxy_Character ),

	RecvPropEHandle( RECVINFO( m_hBriefcase ) ),
	RecvPropInt( RECVINFO( m_iRaceWaypoint ) ),

	RecvPropBool( RECVINFO( m_bCoderHacks ) ),
	RecvPropInt( RECVINFO( m_nCoderHacksButtons ) ),

	RecvPropEHandle( RECVINFO( m_hKiller ) ),
	RecvPropEHandle( RECVINFO( m_hInflictor ) ),
	RecvPropBool( RECVINFO( m_bWasKilledByExplosion ) ),
	RecvPropVector( RECVINFO( m_vecKillingExplosionPosition ) ),
	RecvPropBool( RECVINFO( m_bWasKilledByGrenade ) ),
	RecvPropBool( RECVINFO( m_bWasKilledByBrawl ) ),
	RecvPropBool( RECVINFO( m_bWasKilledByString ) ),
	RecvPropString( RECVINFO( m_szKillerString ) ),

	RecvPropEHandle( RECVINFO( m_hSwitchFrom ) ),
END_RECV_TABLE()

// ------------------------------------------------------------------------------------------ //
// Prediction tables.
// ------------------------------------------------------------------------------------------ //
BEGIN_PREDICTION_DATA_NO_BASE( CSDKPlayerShared )
#if defined ( SDK_USE_STAMINA ) || defined ( SDK_USE_SPRINTING )
	DEFINE_PRED_FIELD( m_flStamina, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#endif
#if defined( SDK_USE_PRONE )
	DEFINE_PRED_FIELD( m_bProne, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flGoProneTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flUnProneTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDisallowUnProneTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bProneSliding, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
#if defined( SDK_USE_SPRINTING )
	DEFINE_PRED_FIELD( m_bIsSprinting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
	DEFINE_PRED_FIELD( m_bSliding, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bInAirSlide, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecSlideDirection, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flSlideTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flUnSlideTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecUnSlideEyeStartOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bMustDuckFromSlide, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDiveSliding, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flLastDuckPress, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bRolling, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bRollingFromDive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecRollDirection, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flRollTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDiving, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecDiveDirection, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bRollAfterDive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDiveTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flTimeLeftGround, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDiveLerped, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDiveToProneLandTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flViewTilt, FIELD_FLOAT, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_flViewBobRamp, FIELD_FLOAT, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_bAimedIn, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flAimIn, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flSlowAimIn, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecRecoilDirection, FIELD_VECTOR, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_flRecoilAccumulator, FIELD_FLOAT, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_iStyleSkill, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bSuperSkill, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD (m_iWallFlipCount, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD (m_bIsWallFlipping, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD (m_flWallFlipEndTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD (m_bIsManteling, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD (m_vecMantelWallNormal, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD( m_bSuperFalling, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bSuperFallOthersVisible, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flSuperFallOthersNextCheck, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

BEGIN_PREDICTION_DATA( C_SDKPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_Shared, CSDKPlayerShared ),
	DEFINE_PRED_FIELD( m_flFreezeUntil, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flFreezeAmount, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flReadyWeaponUntil, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flDisarmRedraw, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flStylePoints, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flStyleSkillCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flSlowMoSeconds, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flSlowMoTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flSlowMoMultiplier, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flCurrentTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flLastSpawnTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_bThirdPerson, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_bThirdPersonCamSide, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_vecThirdCamera, FIELD_VECTOR, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_vecThirdTarget, FIELD_VECTOR, FTYPEDESC_PRIVATE ),
	DEFINE_PRED_FIELD( m_bUsingVR, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),   
	DEFINE_PRED_FIELD( m_flMuzzleFlashYaw, FIELD_FLOAT, FTYPEDESC_PRIVATE ),   

	DEFINE_PRED_FIELD( m_bCoderHacks, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nCoderHacksButtons, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_hSwitchFrom, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( player, C_SDKPlayer );

ConVar cl_ragdoll_fade_time( "cl_ragdoll_fade_time", "15", FCVAR_CLIENTDLL );
ConVar cl_ragdoll_pronecheck_distance( "cl_ragdoll_pronecheck_distance", "64", FCVAR_GAMEDLL );

class C_SDKRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_SDKRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();

	C_SDKRagdoll();
	~C_SDKRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;

	virtual bool TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	virtual void ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName );

	void ClientThink( void );
	void StartFadeOut( float fDelay );
	
	bool IsRagdollVisible();
	int BloodColor()
	{
		return BLOOD_COLOR_RED; 
	}
private:

	C_SDKRagdoll( const C_SDKRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity );

	void CreateRagdoll();


private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
	float m_fDeathTime;
	bool  m_bFadingOut;
};


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_SDKRagdoll, DT_SDKRagdoll, CSDKRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) )
END_RECV_TABLE()


C_SDKRagdoll::C_SDKRagdoll()
{
	m_fDeathTime = -1;
	m_bFadingOut = false;
}

C_SDKRagdoll::~C_SDKRagdoll()
{
	PhysCleanupFrictionSounds( this );
}

void C_SDKRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
    	
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(),
				pDestEntry->watcher->GetDebugName() ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

bool C_SDKRagdoll::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	// This somehow avoids a stack overflow in the engine, but I don't have symbols so I can't figure out the real cause.
	return false;
}

void FX_BloodSpray( const Vector &origin, const Vector &normal, float scale, unsigned char r, unsigned char g, unsigned char b, int flags );
void C_SDKRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
//	DevMsg("C_SDKRagDoll::ImpactTrace: %i\n", iDamageType);
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 4000;  // adjust impact strength
				
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  

		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		// Blood spray!
		FX_BloodSpray( hitpos, dir, 3, 72, 0, 0, FX_BLOODSPRAY_ALL  );

		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	
		//Tony; throw in some bleeds! - just use a generic value for damage.
		TraceBleed( 40, dir, pTrace, iDamageType );

	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_SDKRagdoll::CreateRagdoll()
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( m_hPlayer.Get() );

	if (!pPlayer)
	{
		Release();
		return;
	}

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		Interp_Copy( pPlayer );

		SetAbsAngles( pPlayer->GetRenderAngles() );
		GetRotationInterpolator().Reset();

		m_flAnimTime = pPlayer->m_flAnimTime;
		SetSequence( pPlayer->GetSequence() );
		m_flPlaybackRate = pPlayer->GetPlaybackRate();

		m_nBody = pPlayer->GetBody();
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );

	}

	if (pPlayer)
	{
		SetModelIndex( m_nModelIndex );
		m_nSkin = pPlayer->m_nSkin;
	}

	// Turn it into a ragdoll.
	if ( cl_ragdoll_physics_enable.GetInt() )
	{
		// Make us a ragdoll..
		m_nRenderFX = kRenderFxRagdoll;

		matrix3x4_t boneDelta0[MAXSTUDIOBONES];
		matrix3x4_t boneDelta1[MAXSTUDIOBONES];
		matrix3x4_t currentBones[MAXSTUDIOBONES];
		const float boneDt = 0.05f;

		if ( pPlayer && pPlayer == C_BasePlayer::GetLocalPlayer() )
		{
			pPlayer->GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		}
		else
		{
			GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
		}

		InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt );
	}
	else
	{
		ClientLeafSystem()->SetRenderGroup( GetRenderHandle(), RENDER_GROUP_TRANSLUCENT_ENTITY );
	}		

	// Fade out the ragdoll in a while
	StartFadeOut( cl_ragdoll_fade_time.GetFloat() );
	SetNextClientThink( gpGlobals->curtime + 5.0f );
}

void C_SDKRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateRagdoll();
	}
	else 
	{
		if ( !cl_ragdoll_physics_enable.GetInt() )
		{
			// Don't let it set us back to a ragdoll with data from the server.
			m_nRenderFX = kRenderFxNone;
		}
	}
}
bool C_SDKRagdoll::IsRagdollVisible()
{
	Vector vMins = Vector(-1,-1,-1);	//WorldAlignMins();
	Vector vMaxs = Vector(1,1,1);	//WorldAlignMaxs();
		
	Vector origin = GetAbsOrigin();
	
	if( !engine->IsBoxInViewCluster( vMins + origin, vMaxs + origin) )
	{
		return false;
	}
	else if( engine->CullBox( vMins + origin, vMaxs + origin ) )
	{
		return false;
	}

	return true;
}

void C_SDKRagdoll::ClientThink( void )
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	if ( m_bFadingOut == true )
	{
		int iAlpha = GetRenderColor().a;
		int iFadeSpeed = 600.0f;

		iAlpha = std::max( iAlpha - ( iFadeSpeed * gpGlobals->frametime ), 0.0f );

		SetRenderMode( kRenderTransAlpha );
		SetRenderColorA( iAlpha );

		if ( iAlpha == 0 )
		{
			Release();
		}

		return;
	}

	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		C_SDKPlayer *pEnt = static_cast< C_SDKPlayer *> ( UTIL_PlayerByIndex( iClient ) );

		if(!pEnt || !pEnt->IsPlayer())
			continue;

		if ( m_hPlayer == NULL )
			continue;

		if ( pEnt->entindex() == m_hPlayer->entindex() )
			continue;
		
		if ( pEnt->GetHealth() <= 0 )
			continue;
#if defined ( SDK_USE_PRONE )
		if ( pEnt->m_Shared.IsProne() == false )
			continue;
#endif
		Vector vTargetOrigin = pEnt->GetAbsOrigin();
		Vector vMyOrigin =  GetAbsOrigin();

		Vector vDir = vTargetOrigin - vMyOrigin;

		if ( vDir.Length() > cl_ragdoll_pronecheck_distance.GetInt() ) 
			continue;

		SetNextClientThink( CLIENT_THINK_ALWAYS );
		m_bFadingOut = true;
		return;
	}

	//Tony; this is kind of silly, because.. whats the point of fading out?
	// if the player is looking at us, delay the fade
	if ( IsRagdollVisible() )
	{
		StartFadeOut( 5.0 );
		return;
	}

	if ( m_fDeathTime > gpGlobals->curtime )
		return;

	Release(); // Die
}

void C_SDKRagdoll::StartFadeOut( float fDelay )
{
	m_fDeathTime = gpGlobals->curtime + fDelay;
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}
IRagdoll* C_SDKRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

C_BaseAnimating * C_SDKPlayer::BecomeRagdollOnClient()
{
	return NULL;
}


IRagdoll* C_SDKPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_SDKRagdoll *pRagdoll = (C_SDKRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}



C_SDKPlayer::C_SDKPlayer() : 
	m_iv_angEyeAngles( "C_SDKPlayer::m_iv_angEyeAngles" )
{
	m_PlayerAnimState = CreateSDKPlayerAnimState( this );
	m_Shared.Init(this);

	m_angEyeAngles.Init();
	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_fNextThinkPushAway = 0.0f;

	m_flLastSlowMoMultiplier = 1;
	m_currentAlphaVal = 255.0f;

	m_hLeftArmGlow = NULL;
	m_hRightArmGlow = NULL;
	m_hLeftFootGlow = NULL;
	m_hRightFootGlow = NULL;
}


C_SDKPlayer::~C_SDKPlayer()
{
	m_PlayerAnimState->Release();

	if ( m_bFlashlightEnabled )
	{
		TurnOffFlashlight();
	}
}


void C_SDKPlayer::PreThink()
{
	UpdateCurrentTime();

	// Need to update view bob ramp for the in-eye target,
	// since PreThink (and thus UpdateCurrentTime()) is only called for the local player
	if (GetObserverMode() == OBS_MODE_IN_EYE && GetObserverTarget() && GetObserverTarget()->IsPlayer())
		ToSDKPlayer(GetObserverTarget())->UpdateViewBobRamp();

	BaseClass::PreThink();

	Instructor_Think();

	if (C_SDKPlayer::GetLocalSDKPlayer() == this && GetClientVoiceMgr()->IsLocalPlayerSpeaking())
		Instructor_LessonLearned("voicechat");

	bool bSend = (C_SDKPlayer::GetLocalOrSpectatedPlayer() == this);

	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		bSend = false;

	if (bSend)
	{
		if (IsStyleSkillActive())
			vb_data_send_float(ViewbackSystem().m_ePlayerStyle, m_flStyleSkillCharge);
		else
			vb_data_send_float(ViewbackSystem().m_ePlayerStyle, m_flStylePoints);

		vb_data_send_float(ViewbackSystem().m_ePlayerOriginX, GetAbsOrigin().x);
		vb_data_send_float(ViewbackSystem().m_ePlayerOriginY, GetAbsOrigin().y);
		vb_data_send_float(ViewbackSystem().m_ePlayerOriginZ, GetAbsOrigin().z);

		Vector vecRecoil = m_Shared.m_vecRecoilDirection * m_Shared.m_flRecoilAccumulator;
		vb_data_send_vector(ViewbackSystem().m_ePlayerRecoil, vecRecoil.x, -vecRecoil.y, vecRecoil.z);
		vb_data_send_float(ViewbackSystem().m_ePlayerRecoilFloat, m_Shared.m_flRecoilAccumulator);
		vb_data_send_float(ViewbackSystem().m_ePlayerViewPunch, -m_Local.m_vecPunchAngle.GetX());

		vb_data_send_float(ViewbackSystem().m_eAimIn, m_Shared.GetAimIn());
		vb_data_send_float(ViewbackSystem().m_eSlowMo, GetSlowMoMultiplier());
	}
}


C_SDKPlayer* C_SDKPlayer::GetLocalSDKPlayer()
{
	return ToSDKPlayer( C_BasePlayer::GetLocalPlayer() );
}

C_SDKPlayer* C_SDKPlayer::GetLocalOrSpectatedPlayer()
{
	C_SDKPlayer* pLocal = GetLocalSDKPlayer();

	if (pLocal && pLocal->GetObserverMode() == OBS_MODE_IN_EYE && pLocal->GetObserverTarget() && pLocal->GetObserverTarget()->IsPlayer())
		return ToSDKPlayer(pLocal->GetObserverTarget());

	return pLocal;
}

const QAngle &C_SDKPlayer::EyeAngles()
{
	if( IsLocalPlayer() )
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		return m_angEyeAngles;
	}
}
const QAngle& C_SDKPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

const Vector& C_SDKPlayer::GetRenderOrigin( void )
{
	return BaseClass::GetRenderOrigin();
}

void C_SDKPlayer::UpdateClientSideAnimation()
{
	QAngle angEyeAngles = EyeAngles();
	QAngle angCharacterEyeAngles = angEyeAngles;

	if (IsInThirdPerson() && this == GetLocalSDKPlayer())
		VectorAngles(m_vecThirdTarget - EyePosition(), Vector(0, 0, 1), angCharacterEyeAngles);

	m_PlayerAnimState->Update( angEyeAngles[YAW], angEyeAngles[PITCH], angCharacterEyeAngles[YAW], angCharacterEyeAngles[PITCH] );

	BaseClass::UpdateClientSideAnimation();

	// If in first person, still setup bones so that they're available for the ragdoll when the player dies.
	if (!IsInThirdPerson())
		SetupBones(NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CStudioHdr *C_SDKPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	InitializePoseParams();

	// Reset the players animation states, gestures
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->OnNewModel();
	}

	return hdr;
}
//-----------------------------------------------------------------------------
// Purpose: Clear all pose parameters
//-----------------------------------------------------------------------------
void C_SDKPlayer::InitializePoseParams( void )
{
	CStudioHdr *hdr = GetModelPtr();
	Assert( hdr );
	if ( !hdr )
		return;

	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}

}
//Tony; TODO!
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void C_SDKPlayer::OnPlayerClassChange( void )
{
	// Init the anim movement vars
	m_PlayerAnimState->SetRunSpeed( GetPlayerClass()->GetMaxSpeed() );
	m_PlayerAnimState->SetWalkSpeed( GetPlayerClass()->GetMaxSpeed() * 0.5 );
}
*/
void C_SDKPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );
	
	BaseClass::PostDataUpdate( updateType );

	bool bIsLocalPlayer = IsLocalPlayer();

	if( m_bSpawnInterpCounter != m_bSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();

		if ( bIsLocalPlayer )
		{
			LocalPlayerRespawn();
		}
		m_bSpawnInterpCounterCache = m_bSpawnInterpCounter.m_Value;
	}

}
// Called every time the player respawns
void C_SDKPlayer::LocalPlayerRespawn( void )
{
	ResetToneMapping(1.0);
#if defined ( SDK_USE_PRONE )
	m_bUnProneToDuck = false;
#endif

	InitSpeeds(); //Tony; initialize player speeds.

	if (C_SDKPlayer::GetLocalSDKPlayer() == this)
		m_pInstructor = new CInstructor();

	Instructor_Respawn();

	// clear attack inputs on spawn
	::input->ClearInputButton( IN_ATTACK | IN_ATTACK2 );
	::input->GetButtonBits( 0 ); 

	if (UseVR())
		engine->ServerCmd("vr on");
	else
		engine->ServerCmd("vr off");
}

void C_SDKPlayer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

void C_SDKPlayer::PlayReloadEffect()
{
	// Only play the effect for other players.
	if ( this == C_SDKPlayer::GetLocalSDKPlayer() )
	{
		Assert( false ); // We shouldn't have been sent this message.
		return;
	}

	// Get the view model for our current gun.
	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
	if ( !pWeapon )
		return;

	// The weapon needs two models, world and view, but can only cache one. Synthesize the other.
	const CSDKWeaponInfo &info = pWeapon->GetSDKWpnData();
	const model_t *pModel = modelinfo->GetModel( modelinfo->GetModelIndex( info.szViewModel ) );
	if ( !pModel )
		return;
	CStudioHdr studioHdr( modelinfo->GetStudiomodel( pModel ), mdlcache );
	if ( !studioHdr.IsValid() )
		return;

	// Find the reload animation.
	for ( int iSeq=0; iSeq < studioHdr.GetNumSeq(); iSeq++ )
	{
		mstudioseqdesc_t *pSeq = &studioHdr.pSeqdesc( iSeq );

		if ( pSeq->activity == ACT_VM_RELOAD )
		{
			float poseParameters[MAXSTUDIOPOSEPARAM];
			memset( poseParameters, 0, sizeof( poseParameters ) );
			float cyclesPerSecond = Studio_CPS( &studioHdr, *pSeq, iSeq, poseParameters );

			// Now read out all the sound events with their timing
			for ( int iEvent=0; iEvent < pSeq->numevents; iEvent++ )
			{
				mstudioevent_t *pEvent = pSeq->pEvent( iEvent );

				if ( pEvent->event == CL_EVENT_SOUND )
				{
					CSDKSoundEvent event;
					event.m_SoundName = pEvent->options;
					event.m_flEventTime = gpGlobals->curtime + pEvent->cycle / cyclesPerSecond;
					m_SoundEvents.AddToTail( event );
				}
			}

			break;
		}
	}	
}

void C_SDKPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( IsLocalPlayer() )
	{
		if ( ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
}

bool C_SDKPlayer::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if ( State_Get() == STATE_WELCOME )
		return false;

	if ( State_Get() == STATE_MAPINFO )
		return false;

#if defined ( SDK_USE_TEAMS )
	if ( State_Get() == STATE_PICKINGTEAM )
		return false;
#endif

#if defined ( SDK_USE_PLAYERCLASSES )
	if ( State_Get() == STATE_PICKINGCLASS )
		return false;
#endif

	if ( State_Get() == STATE_BUYINGWEAPONS )
		return false;

	if ( State_Get() == STATE_PICKINGSKILL )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;

	return BaseClass::ShouldDraw();
}

CWeaponSDKBase* C_SDKPlayer::GetActiveSDKWeapon() const
{
	return dynamic_cast< CWeaponSDKBase* >( GetActiveWeapon() );
}

//-----------------------------------------------------------------------------
// Purpose: Non-Caching CalcVehicleView for Scratch SDK + Multiplayer
// TODO: fix the normal CalcVehicleView so that Caching can work in multiplayer.
//-----------------------------------------------------------------------------
void C_SDKPlayer::CalcVehicleView(IClientVehicle *pVehicle,	Vector& eyeOrigin, QAngle& eyeAngles,	float& zNear, float& zFar, float& fov )
{
	Assert( pVehicle );

	// Start with our base origin and angles
	int nRole = pVehicle->GetPassengerRole( this );

	// Get our view for this frame
	pVehicle->GetVehicleViewPosition( nRole, &eyeOrigin, &eyeAngles, &fov );

	// Allows the vehicle to change the clip planes
	pVehicle->GetVehicleClipPlanes( zNear, zFar );

	// Snack off the origin before bob + water offset are applied
	Vector vecBaseEyePosition = eyeOrigin;

	CalcViewRoll( eyeAngles );

	// Apply punch angle
	VectorAdd( eyeAngles, m_Local.m_vecPunchAngle, eyeAngles );

	if ( !prediction->InPrediction() )
	{
		// Shake it up baby!
		vieweffects->CalcShake();
		vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );
	}
}

void C_SDKPlayer::CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	C_BaseEntity *target = GetObserverTarget();

	if ( !target ) 
	{
		// just copy a save in-map position
		VectorCopy( EyePosition(), eyeOrigin );
		VectorCopy( EyeAngles(), eyeAngles );
		return;
	};

	if ( !target->IsAlive() )
	{
		// if dead, show from 3rd person
		CalcChaseCamView( eyeOrigin, eyeAngles, fov );
		return;
	}

	fov = GetFOV();	// TODO use tragets FOV

	m_flObserverChaseDistance = 0.0;

	eyeAngles = target->EyeAngles() + GetPunchAngle();
	eyeOrigin = target->EyePosition();

	engine->SetViewAngles( eyeAngles );
}

ConVar da_deathframe_back( "da_deathframe_back", "40.0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY );
ConVar da_deathframe_side( "da_deathframe_side", "60.0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY );
ConVar da_deathframe_showvectors("da_deathframe_showvectors", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
ConVar da_deathframe_debug("da_deathframe_debug", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void C_SDKPlayer::CalcFreezeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	int iTotal = 6;

	int iStart = random->RandomInt(0, iTotal-1);
	int iCurrent = (iStart+1)%iTotal;

	Vector vecCamTarget, vecCamPlayer, vecCamLookAt;

	bool bOK = false;
	bool bDebugTargets = false;
	bool bDebugViews = da_deathframe_debug.GetBool() && !m_bSentFreezeFrame;

	while (iCurrent != iStart)
	{
		switch (iCurrent)
		{
		default:
		case 0:
			bOK = CalcFreezeCamThreeQuartersView(eyeOrigin, eyeAngles, fov, 1, vecCamTarget, vecCamPlayer, vecCamLookAt);
			if (bDebugViews)
				DevMsg("Three quarters view 1 %s\n", bOK?"OK":"not OK");
			bDebugTargets = true;
			break;

		case 1:
			bOK = CalcFreezeCamThreeQuartersView(eyeOrigin, eyeAngles, fov, -1, vecCamTarget, vecCamPlayer, vecCamLookAt);
			if (bDebugViews)
				DevMsg("Three quarters view -1 %s\n", bOK?"OK":"not OK");
			bDebugTargets = true;
			break;

		case 2:
			bOK = CalcFreezeCamHalfHalfView(eyeOrigin, eyeAngles, fov, 1, vecCamTarget, vecCamPlayer, vecCamLookAt);
			if (bDebugViews)
				DevMsg("Half half view 1 %s\n", bOK?"OK":"not OK");
			bDebugTargets = true;
			break;

		case 3:
			bOK = CalcFreezeCamHalfHalfView(eyeOrigin, eyeAngles, fov, -1, vecCamTarget, vecCamPlayer, vecCamLookAt);
			if (bDebugViews)
				DevMsg("Half half view -1 %s\n", bOK?"OK":"not OK");
			bDebugTargets = true;
			break;

		case 4:
			bOK = CalcFreezeCamPortraitView(eyeOrigin, eyeAngles, fov);
			if (bDebugViews)
				DevMsg("Portrait view %s\n", bOK?"OK":"not OK");
			bDebugTargets = false;
			break;

		case 5:
			bOK = CalcFreezeCamKillerAimInView(eyeOrigin, eyeAngles, fov);
			if (bDebugViews)
				DevMsg("Killer aim-in view %s\n", bOK?"OK":"not OK");
			bDebugTargets = false;
			break;
		}

		if (bOK)
			break;

		iCurrent = (iCurrent+1)%iTotal;
	}

	if (!bOK)
	{
		if (bDebugViews)
			DevMsg("All failed. Trying portrait view 3 more times.\n");

		// CalcFreezeCamPortraitView() tries from different angles at random
		// so if another angle can't be found, try this one a few times.
		for (int i = 0; i < 3; i++)
		{
			if (CalcFreezeCamPortraitView(eyeOrigin, eyeAngles, fov))
			{
				if (bDebugViews)
					DevMsg("Portrait view OK.\n");
	
				break;
			}

			if (bDebugViews)
				DevMsg("Portrait view failed.\n");
		}

		bDebugTargets = false;
	}

	if (m_bWasKilledByExplosion)
	{
		// Delay a tad if we were killed by an explosion, to allow the explosion particles to go.
		if (gpGlobals->curtime < m_flDeathTime + 0.2f)
			return;
	}
	else
	{
		if (gpGlobals->curtime < m_flDeathTime + 0.05f)
			return;
	}

	if ( !m_bSentFreezeFrame )
	{
		if (da_deathframe_showvectors.GetBool())
		{
			float flTime = 60;

			if (bDebugTargets)
			{
				debugoverlay->AddBoxOverlay( vecCamPlayer, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 0,0,255,127, flTime );
				DrawClientHitboxes( flTime, true );
				debugoverlay->AddBoxOverlay( vecCamTarget, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, flTime );
				if (ToSDKPlayer(GetObserverTarget()))
					ToSDKPlayer(GetObserverTarget())->DrawClientHitboxes( flTime, true );
				debugoverlay->AddBoxOverlay( vecCamLookAt, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,255,127, flTime );
				debugoverlay->AddLineOverlayAlpha( vecCamPlayer, vecCamTarget, 255, 255, 255, 127, false, flTime );
				debugoverlay->AddBoxOverlay( eyeOrigin, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 0,255,0,127, flTime );
				debugoverlay->AddLineOverlayAlpha( eyeOrigin, vecCamLookAt, 255, 255, 255, 127, false, flTime );
			}
			else
			{
				DrawClientHitboxes( flTime, true );
				debugoverlay->AddBoxOverlay( eyeOrigin, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 0,255,0,127, flTime );

				Vector vecForward;
				AngleVectors(eyeAngles, &vecForward);
				debugoverlay->AddLineOverlayAlpha( eyeOrigin, eyeOrigin+vecForward*100, 255, 255, 255, 127, false, flTime );
			}
		}

		m_bSentFreezeFrame = true;

		ConVarRef spec_freeze_time("spec_freeze_time");
		view->FreezeFrame( spec_freeze_time.GetFloat() );
	}
}

bool C_SDKPlayer::CalcFreezeCamThreeQuartersView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov, float flSide, Vector& vecCamTarget, Vector& vecCamPlayer, Vector& vecCamLookAt )
{
	C_BaseEntity* pTarget = GetObserverTarget();

	if (pTarget == this)
		pTarget = NULL;

	if (!pTarget)
		pTarget = GetInflictor();

	if (pTarget == this)
		pTarget = NULL;

	if (!pTarget && !m_bWasKilledByExplosion)
		return false;

	if (m_bWasKilledByExplosion)
		vecCamTarget = m_vecKillingExplosionPosition + Vector(0, 0, 35);
	else
		vecCamTarget = GetFreezeCamOrigin(pTarget);

	vecCamPlayer = GetFreezeCamOrigin(this);

	vecCamLookAt = Lerp(0.3f, vecCamPlayer, vecCamTarget);

	Vector vecForward = (vecCamTarget-vecCamPlayer).Normalized();
	Vector vecSide = vecForward.Cross(Vector(0, 0, 1)).Normalized();

	Vector vecCamPosition = vecCamPlayer - vecForward * da_deathframe_back.GetFloat() + vecSide * da_deathframe_side.GetFloat() * flSide;

	Vector vecCamSize(6, 6, 6);

	trace_t trace;
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( vecCamPlayer, vecCamPosition, -vecCamSize, vecCamSize, MASK_OPAQUE, pTarget, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if (trace.fraction < 1.0)
		return false;

	eyeOrigin = vecCamPosition;

	VectorAngles((vecCamLookAt - vecCamPosition).Normalized(), eyeAngles);

	return true;
}

bool C_SDKPlayer::CalcFreezeCamHalfHalfView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov, float flSide, Vector& vecCamTarget, Vector& vecCamPlayer, Vector& vecCamLookAt )
{
	C_SDKPlayer* pTarget = ToSDKPlayer(GetObserverTarget());

	if (pTarget == this)
		pTarget = NULL;

	if (!pTarget)
		return false;

	vecCamTarget = GetFreezeCamOrigin(pTarget);

	vecCamPlayer = GetFreezeCamOrigin(this);

	if ((vecCamTarget-vecCamPlayer).Length() > 200)
		return false;

	vecCamLookAt = Lerp(0.5f, vecCamPlayer, vecCamTarget);

	Vector vecForward = (vecCamTarget-vecCamPlayer).Normalized();
	Vector vecSide = vecForward.Cross(Vector(0, 0, 1)).Normalized();

	Vector vecCamPosition = vecCamLookAt + vecSide * da_deathframe_side.GetFloat() * 2 * flSide;

	Vector vecCamSize(6, 6, 6);

	trace_t trace;
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( vecCamPlayer, vecCamPosition, -vecCamSize, vecCamSize, MASK_OPAQUE, pTarget, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if (trace.fraction < 1.0)
		return false;

	eyeOrigin = vecCamPosition;

	VectorAngles((vecCamLookAt - vecCamPosition).Normalized(), eyeAngles);

	return true;
}

bool C_SDKPlayer::CalcFreezeCamPortraitView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	Vector vecCamPlayer = GetFreezeCamOrigin(this);

	Vector vecForward, vecSide, vecUp;
	GetVectors(&vecForward, &vecSide, NULL);

	float flRandomYaw = random->RandomFloat(-100, 100);
	float flForward = cos(flRandomYaw) * da_deathframe_side.GetFloat();
	float flSide = sin(flRandomYaw) * da_deathframe_side.GetFloat();

	Vector vecCamPosition = vecCamPlayer + vecForward * flForward + vecSide * flSide;

	Vector vecCamSize(6, 6, 6);

	trace_t trace;
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( vecCamPlayer, vecCamPosition, -vecCamSize, vecCamSize, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if (trace.fraction < 1.0)
		return false;

	eyeOrigin = vecCamPosition;

	VectorAngles((vecCamPlayer - vecCamPosition).Normalized(), eyeAngles);

	return true;
}

bool C_SDKPlayer::CalcFreezeCamKillerAimInView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	C_SDKPlayer* pTarget = ToSDKPlayer(GetObserverTarget());

	if (pTarget == this)
		pTarget = NULL;

	if (!pTarget)
		return false;

	if (!pTarget->GetActiveSDKWeapon())
		return false;

	if (pTarget->GetActiveSDKWeapon()->IsGrenade())
		return false;

	if (pTarget->GetActiveSDKWeapon()->IsMeleeWeapon())
		return false;

	if (GetInflictor() != pTarget)
		return false;

	// Set up some values since the target is remote and we might not have them proper.
	pTarget->m_flSideLerp = pTarget->m_bThirdPersonCamSide?1:-1;
	pTarget->m_flStuntLerp = (pTarget->m_Shared.IsDiving()||pTarget->m_Shared.IsRolling())?1:0;
	pTarget->m_flCameraLerp = 1;

	// Easy!
	eyeOrigin = pTarget->CalculateThirdPersonCameraPosition(pTarget->EyePosition(), pTarget->EyeAngles());
	eyeAngles = pTarget->EyeAngles();

	Vector vecCamSize(6, 6, 6);

	trace_t trace;
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( pTarget->EyePosition(), eyeOrigin, -vecCamSize, vecCamSize, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if (trace.fraction < 1.0)
		return false;

	return true;
}

Vector C_SDKPlayer::GetFreezeCamOrigin(C_BaseEntity* pTarget)
{
	C_SDKPlayer* pSDKTarget = ToSDKPlayer(pTarget);
	if (!pSDKTarget)
		return pTarget->GetObserverCamOrigin();

	Vector vecOrigin;

	if (pSDKTarget->m_hRagdoll != NULL && static_cast<C_BaseAnimating*>(pSDKTarget->m_hRagdoll.Get())->IsRagdoll())
		vecOrigin = pSDKTarget->m_hRagdoll->GetRenderOrigin();
	else
	{
		vecOrigin = pSDKTarget->GetObserverCamOrigin() + pSDKTarget->GetChaseCamViewOffset( pSDKTarget );

		if ( pSDKTarget->m_Shared.IsSliding() )
			vecOrigin.z -= 20;
		else if ( !pSDKTarget->IsAlive() )
			vecOrigin.z += 35;
	}

	return vecOrigin;
}

#if defined ( SDK_USE_PLAYERCLASSES )
bool C_SDKPlayer::CanShowClassMenu( void )
{
	#if defined ( SDK_USE_TEAMS )
		return ( GetTeamNumber() == SDK_TEAM_BLUE || GetTeamNumber() == SDK_TEAM_RED );
	#else
		return ( GetTeamNumber() != TEAM_SPECTATOR );
	#endif
}
#endif

#if defined ( SDK_USE_TEAMS )
bool C_SDKPlayer::CanShowTeamMenu( void )
{
	return true;
}
#endif

bool C_SDKPlayer::CanShowBuyMenu( void )
{
	return ( GetTeamNumber() != TEAM_SPECTATOR );
}

bool C_SDKPlayer::CanShowSkillMenu( void )
{
	return ( GetTeamNumber() != TEAM_SPECTATOR );
}

/*static*/ bool C_BasePlayer::ShouldDrawLocalPlayer()
{
	if ( UseVR() )
	{
		static ConVarRef vr_first_person_uses_world_model( "vr_first_person_uses_world_model" );
		return !LocalPlayerInFirstPersonView() || vr_first_person_uses_world_model.GetBool();
	}
	else
	{
		if (C_SDKPlayer::GetLocalSDKPlayer()->IsInThirdPerson())
			return true;
		
		if (ToolsEnabled() && ToolFramework_IsThirdPersonCamera())
			return true;

		static ConVarRef cl_first_person_uses_world_model( "cl_first_person_uses_world_model" );

		return !LocalPlayerInFirstPersonView() || cl_first_person_uses_world_model.GetBool();
	}
}

ConVar da_slowmo_motion_blur( "da_slowmo_motion_blur", "7.0" );

void C_SDKPlayer::ClientThink()
{
	bool bBrawlEffectOn = false;

	if (IsStyleSkillActive(SKILL_BOUNCER))
	{
		if (GetActiveSDKWeapon())
		{
			if (GetActiveSDKWeapon()->GetWeaponID() == SDK_WEAPON_BRAWL)
				bBrawlEffectOn = true;
			else if (GetActiveSDKWeapon()->GetSwingTime() > 0)
				bBrawlEffectOn = true;
		}
	}

	if (bBrawlEffectOn)
	{
		if (!m_hLeftArmGlow)
			m_hLeftArmGlow = ParticleProp()->Create("style_active_bouncer", PATTACH_POINT_FOLLOW, "lefthand");
		if (!m_hRightArmGlow)
			m_hRightArmGlow = ParticleProp()->Create("style_active_bouncer", PATTACH_POINT_FOLLOW, "righthand");
	}
	else
	{
		if (m_hLeftArmGlow)
		{
			ParticleProp()->StopEmission(m_hLeftArmGlow);
			m_hLeftArmGlow = NULL;
		}
		if (m_hRightArmGlow)
		{
			ParticleProp()->StopEmission(m_hRightArmGlow);
			m_hRightArmGlow = NULL;
		}
	}

	if (IsStyleSkillActive(SKILL_ATHLETIC))
	{
		if (!m_hLeftFootGlow)
			m_hLeftFootGlow = ParticleProp()->Create("style_active_athletic", PATTACH_POINT_FOLLOW, "leftfoot");
		if (!m_hRightFootGlow)
			m_hRightFootGlow = ParticleProp()->Create("style_active_athletic", PATTACH_POINT_FOLLOW, "rightfoot");
	}
	else
	{
		if (m_hLeftFootGlow)
		{
			ParticleProp()->StopEmission(m_hLeftFootGlow);
			m_hLeftFootGlow = NULL;
		}
		if (m_hRightFootGlow)
		{
			ParticleProp()->StopEmission(m_hRightFootGlow);
			m_hRightFootGlow = NULL;
		}
	}

	bool bWasInSlow = false;

	bool bLocalPlayer = (C_SDKPlayer::GetLocalOrSpectatedPlayer() == this);
	if (bLocalPlayer)
		bWasInSlow = (m_flLastSlowMoMultiplier < 1);

	if (bLocalPlayer)
	{
		bool bNowInSlow = (GetSlowMoMultiplier() < 1);

		if (!C_SDKPlayer::GetLocalOrSpectatedPlayer()->IsAlive())
			bNowInSlow = false;

		if (!bWasInSlow && bNowInSlow)
		{
			C_SDKPlayer::GetLocalOrSpectatedPlayer()->EmitSound( "SlowMo.Start" );
			C_SDKPlayer::GetLocalOrSpectatedPlayer()->EmitSound( "SlowMo.Loop" );
		}
		else if (bWasInSlow && !bNowInSlow)
		{
			C_SDKPlayer::GetLocalOrSpectatedPlayer()->StopSound( "SlowMo.Loop" );
			C_SDKPlayer::GetLocalOrSpectatedPlayer()->EmitSound( "SlowMo.End" );
		}
	}

	m_flLastSlowMoMultiplier = GetSlowMoMultiplier();

	ConVarRef mat_motion_blur_strength( "mat_motion_blur_strength" );
	mat_motion_blur_strength.SetValue(RemapValClamped(GetSlowMoMultiplier(), 0.8f, 1, da_slowmo_motion_blur.GetFloat(), 1));

	if (this == C_SDKPlayer::GetLocalSDKPlayer())
	{
		if (::input->CAM_IsThirdPerson() && !IsInThirdPerson())
		{
			::input->CAM_ToFirstPerson();
			ThirdPersonSwitch(false);
		}
		else if (!::input->CAM_IsThirdPerson() && IsInThirdPerson())
		{
			::input->CAM_ToThirdPerson();
			ThirdPersonSwitch(true);
		}
	}

	UpdateSoundEvents();

	// Pass on through to the base class.
	BaseClass::ClientThink();

	bool bFoundViewTarget = false;
	
	Vector vForward;
	AngleVectors( GetLocalAngles(), &vForward );

	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );
		if(!pEnt || !pEnt->IsPlayer())
			continue;

		if ( pEnt->entindex() == entindex() )
			continue;

		Vector vTargetOrigin = pEnt->GetAbsOrigin();
		Vector vMyOrigin =  GetAbsOrigin();

		Vector vDir = vTargetOrigin - vMyOrigin;
		
		if ( vDir.Length() > 128 ) 
			continue;

		VectorNormalize( vDir );

		if ( DotProduct( vForward, vDir ) < 0.0f )
			 continue;

		m_vLookAtTarget = pEnt->EyePosition();
		bFoundViewTarget = true;
		break;
	}

	if ( bFoundViewTarget == false )
	{
		m_vLookAtTarget = GetAbsOrigin() + vForward * 512;
	}

	UpdateIDTarget();

	// Avoidance
	if ( gpGlobals->curtime >= m_fNextThinkPushAway )
	{
		PerformObstaclePushaway( this );
		m_fNextThinkPushAway =  gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::UpdateLookAt( void )
{
	// head yaw
	if (m_headYawPoseParam < 0 || m_headPitchPoseParam < 0)
		return;

	// orient eyes
	m_viewtarget = m_vLookAtTarget;

	// Figure out where we want to look in world space.
	QAngle desiredAngles;
	Vector to = m_vLookAtTarget - EyePosition();
	VectorAngles( to, desiredAngles );

	// Figure out where our body is facing in world space.
	QAngle bodyAngles( 0, 0, 0 );
	bodyAngles[YAW] = GetLocalAngles()[YAW];


	float flBodyYawDiff = bodyAngles[YAW] - m_flLastBodyYaw;
	m_flLastBodyYaw = bodyAngles[YAW];
	

	// Set the head's yaw.
	float desired = AngleNormalize( desiredAngles[YAW] - bodyAngles[YAW] );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	m_flCurrentHeadYaw = ApproachAngle( desired, m_flCurrentHeadYaw, 130 * gpGlobals->frametime );

	// Counterrotate the head from the body rotation so it doesn't rotate past its target.
	m_flCurrentHeadYaw = AngleNormalize( m_flCurrentHeadYaw - flBodyYawDiff );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	
	SetPoseParameter( m_headYawPoseParam, m_flCurrentHeadYaw );

	
	// Set the head's yaw.
	desired = AngleNormalize( desiredAngles[PITCH] );
	desired = clamp( desired, m_headPitchMin, m_headPitchMax );
	
	m_flCurrentHeadPitch = ApproachAngle( desired, m_flCurrentHeadPitch, 130 * gpGlobals->frametime );
	m_flCurrentHeadPitch = AngleNormalize( m_flCurrentHeadPitch );
	SetPoseParameter( m_headPitchPoseParam, m_flCurrentHeadPitch );
}



int C_SDKPlayer::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's target entity
//-----------------------------------------------------------------------------
void C_SDKPlayer::UpdateIDTarget()
{
	if ( !IsLocalPlayer() )
		return;

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show id's in any state but active.
	if ( State_Get() != STATE_ACTIVE )
		return;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Try to steer away from any players and objects we might interpenetrate
//-----------------------------------------------------------------------------
#define SDK_AVOID_MAX_RADIUS_SQR		5184.0f			// Based on player extents and max buildable extents.
#define SDK_OO_AVOID_MAX_RADIUS_SQR		0.00029f

ConVar sdk_max_separation_force ( "sdk_max_separation_force", "256", FCVAR_CHEAT|FCVAR_HIDDEN );

extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

void C_SDKPlayer::AvoidPlayers( CUserCmd *pCmd )
{
// Player Avoidance is only active with teams
#if defined ( SDK_USE_TEAMS )
	// Don't test if the player doesn't exist or is dead.
	if ( IsAlive() == false )
		return;

	C_SDKTeam *pTeam = ( C_SDKTeam * )GetTeam();
	if ( !pTeam )
		return;

	// Up vector.
	static Vector vecUp( 0.0f, 0.0f, 1.0f );

	Vector vecSDKPlayerCenter = GetAbsOrigin();
	Vector vecSDKPlayerMin = GetPlayerMins();
	Vector vecSDKPlayerMax = GetPlayerMaxs();
	float flZHeight = vecSDKPlayerMax.z - vecSDKPlayerMin.z;
	vecSDKPlayerCenter.z += 0.5f * flZHeight;
	VectorAdd( vecSDKPlayerMin, vecSDKPlayerCenter, vecSDKPlayerMin );
	VectorAdd( vecSDKPlayerMax, vecSDKPlayerCenter, vecSDKPlayerMax );

	// Find an intersecting player or object.
	int nAvoidPlayerCount = 0;
	C_SDKPlayer *pAvoidPlayerList[MAX_PLAYERS];

	C_SDKPlayer *pIntersectPlayer = NULL;
	float flAvoidRadius = 0.0f;

	Vector vecAvoidCenter, vecAvoidMin, vecAvoidMax;
	for ( int i = 0; i < pTeam->GetNumPlayers(); ++i )
	{
		C_SDKPlayer *pAvoidPlayer = static_cast< C_SDKPlayer * >( pTeam->GetPlayer( i ) );
		if ( pAvoidPlayer == NULL )
			continue;
		// Is the avoid player me?
		if ( pAvoidPlayer == this )
			continue;

		// Save as list to check against for objects.
		pAvoidPlayerList[nAvoidPlayerCount] = pAvoidPlayer;
		++nAvoidPlayerCount;

		// Check to see if the avoid player is dormant.
		if ( pAvoidPlayer->IsDormant() )
			continue;

		// Is the avoid player solid?
		if ( pAvoidPlayer->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
			continue;

		Vector t1, t2;

		vecAvoidCenter = pAvoidPlayer->GetAbsOrigin();
		vecAvoidMin = pAvoidPlayer->GetPlayerMins();
		vecAvoidMax = pAvoidPlayer->GetPlayerMaxs();
		flZHeight = vecAvoidMax.z - vecAvoidMin.z;
		vecAvoidCenter.z += 0.5f * flZHeight;
		VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
		VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

		if ( IsBoxIntersectingBox( vecSDKPlayerMin, vecSDKPlayerMax, vecAvoidMin, vecAvoidMax ) )
		{
			// Need to avoid this player.
			if ( !pIntersectPlayer )
			{
				pIntersectPlayer = pAvoidPlayer;
				break;
			}
		}
	}

	// Anything to avoid?
	if ( !pIntersectPlayer )
		return;

	// Calculate the push strength and direction.
	Vector vecDelta;

	// Avoid a player - they have precedence.
	if ( pIntersectPlayer )
	{
		VectorSubtract( pIntersectPlayer->WorldSpaceCenter(), vecSDKPlayerCenter, vecDelta );

		Vector vRad = pIntersectPlayer->WorldAlignMaxs() - pIntersectPlayer->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}

	float flPushStrength = RemapValClamped( vecDelta.Length(), flAvoidRadius, 0, 0, sdk_max_separation_force.GetInt() ); //flPushScale;

	//Msg( "PushScale = %f\n", flPushStrength );

	// Check to see if we have enough push strength to make a difference.
	if ( flPushStrength < 0.01f )
		return;

	Vector vecPush;
	if ( GetAbsVelocity().Length2DSqr() > 0.1f )
	{
		Vector vecVelocity = GetAbsVelocity();
		vecVelocity.z = 0.0f;
		CrossProduct( vecUp, vecVelocity, vecPush );
		VectorNormalize( vecPush );
	}
	else
	{
		// We are not moving, but we're still intersecting.
		QAngle angView = pCmd->viewangles;
		angView.x = 0.0f;
		AngleVectors( angView, NULL, &vecPush, NULL );
	}

	// Move away from the other player/object.
	Vector vecSeparationVelocity;
	if ( vecDelta.Dot( vecPush ) < 0 )
	{
		vecSeparationVelocity = vecPush * flPushStrength;
	}
	else
	{
		vecSeparationVelocity = vecPush * -flPushStrength;
	}

	// Don't allow the max push speed to be greater than the max player speed.
	float flMaxPlayerSpeed = MaxSpeed();
	float flCropFraction = 1.33333333f;

	if ( ( GetFlags() & FL_DUCKING ) && ( GetGroundEntity() != NULL ) )
	{	
		flMaxPlayerSpeed *= flCropFraction;
	}	

	float flMaxPlayerSpeedSqr = flMaxPlayerSpeed * flMaxPlayerSpeed;

	if ( vecSeparationVelocity.LengthSqr() > flMaxPlayerSpeedSqr )
	{
		vecSeparationVelocity.NormalizeInPlace();
		VectorScale( vecSeparationVelocity, flMaxPlayerSpeed, vecSeparationVelocity );
	}

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;
	Vector currentdir;
	Vector rightdir;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );

	Vector vDirection = vecSeparationVelocity;

	VectorNormalize( vDirection );

	float fwd = currentdir.Dot( vDirection );
	float rt = rightdir.Dot( vDirection );

	float forward = fwd * flPushStrength;
	float side = rt * flPushStrength;

	//Msg( "fwd: %f - rt: %f - forward: %f - side: %f\n", fwd, rt, forward, side );

	pCmd->forwardmove	+= forward;
	pCmd->sidemove		+= side;

	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}

	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}

	float flScale = std::min( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "Pforwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
#endif
}

bool C_SDKPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{	
	static QAngle angMoveAngle( 0.0f, 0.0f, 0.0f );

	VectorCopy( pCmd->viewangles, angMoveAngle );

	BaseClass::CreateMove( flInputSampleTime, pCmd );

	AvoidPlayers( pCmd );

	return true;
}

void C_SDKPlayer::UpdateSoundEvents()
{
	int iNext;
	for ( int i=m_SoundEvents.Head(); i != m_SoundEvents.InvalidIndex(); i = iNext )
	{
		iNext = m_SoundEvents.Next( i );

		CSDKSoundEvent *pEvent = &m_SoundEvents[i];
		if ( gpGlobals->curtime >= pEvent->m_flEventTime )
		{
			CLocalPlayerFilter filter;
			EmitSound( filter, GetSoundSourceIndex(), STRING( pEvent->m_SoundName ) );

			m_SoundEvents.Remove( i );
		}
	}
}

static ConVar da_cam_back("da_cam_back", "68", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far back from the eye position the third person camera sits", true, 5, true, 150);
static ConVar da_cam_right("da_cam_right", "25", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far right from the eye position the third person camera sits", true, -30, true, 30);
static ConVar da_cam_up("da_cam_up", "8", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far up from the eye position the third person camera sits", true, -20, true, 30);

static ConVar da_cam_back_aim("da_cam_back_aim", "50", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far back from the eye position the third person camera sits while aiming in", true, 5, true, 150);
static ConVar da_cam_right_aim("da_cam_right_aim", "35", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far right from the eye position the third person camera sits while aiming in", true, -35, true, 35);
static ConVar da_cam_up_aim("da_cam_up_aim", "5", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far up from the eye position the third person camera sits while aiming in", true, -20, true, 30);

static ConVar da_cam_back_vr("da_cam_back_vr", "40", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far back from the eye position the third person camera sits when using VR", true, 5, true, 150);
static ConVar da_cam_right_vr("da_cam_right_vr", "0", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far right from the eye position the third person camera sits when using VR", true, -30, true, 30);
static ConVar da_cam_up_vr("da_cam_up_vr", "25", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far up from the eye position the third person camera sits when using VR", true, -20, true, 30);

static ConVar da_cam_back_aim_vr("da_cam_back_aim_vr", "20", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far back from the eye position the third person camera sits while aiming in when using VR", true, 5, true, 150);
static ConVar da_cam_right_aim_vr("da_cam_right_aim_vr", "25", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far right from the eye position the third person camera sits while aiming in when using VR", true, -35, true, 35);
static ConVar da_cam_up_aim_vr("da_cam_up_aim_vr", "5", FCVAR_USERINFO|FCVAR_ARCHIVE, "How far up from the eye position the third person camera sits while aiming in when using VR", true, -20, true, 30);

static ConVar da_cam_standing_back_mult( "da_cam_standing_back_mult", "0.7", FCVAR_USERINFO|FCVAR_ARCHIVE, "Scale the back distance by this much while standing." );

static ConVar da_aimin_fov_delta_high("da_aimin_fov_delta_high", "40", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar da_aimin_fov_delta_low("da_aimin_fov_delta_low", "10", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);
static ConVar da_aimin_slow_fov_delta("da_aimin_slow_fov_delta", "5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void C_SDKPlayer::OverrideView( CViewSetup *pSetup )
{
	if (C_SDKPlayer::GetLocalSDKPlayer() == this && IsInThirdPerson())
	{
		Vector vecOffset = g_ThirdPersonManager.GetCameraOffsetAngles();

		QAngle angCamera;
		angCamera[ PITCH ] = vecOffset[ PITCH ];
		angCamera[ YAW ] = vecOffset[ YAW ];
		angCamera[ ROLL ] = 0;

		UpdateThirdCamera(pSetup->origin, angCamera);
			
		pSetup->origin = GetThirdPersonCameraPosition();
	}
	else
	{
		m_flCameraLerp = 0;
		m_flStuntLerp = 0;
	}

	BaseClass::OverrideView(pSetup);

	float flTiltGoal;
	// only tilt first person view
	if (m_Shared.IsDiving() && !IsInThirdPerson())
		flTiltGoal = 1;
	else
		flTiltGoal = 0;

	m_Shared.m_flViewTilt = Approach(flTiltGoal, m_Shared.m_flViewTilt, gpGlobals->frametime*10*GetSlowMoMultiplier());

	// untilt the view if we die
	if (!IsAlive())
		pSetup->angles.z = 0.0f;
	else if (m_Shared.m_flViewTilt > 0)
	{
		Vector vecViewDirection;
		AngleVectors(pSetup->angles, &vecViewDirection);
		vecViewDirection.z = 0;
		vecViewDirection.NormalizeInPlace();

		Vector vecRight = Vector(0, 0, 1).Cross(m_Shared.GetDiveDirection());

		float flDot = vecViewDirection.Dot(vecRight);

		pSetup->angles.z += flDot * m_Shared.m_flViewTilt * 5;
	}

	C_WeaponSDKBase* pWeapon = GetActiveSDKWeapon();

	if (m_Shared.GetAimIn() > 0 && pWeapon && (pWeapon->FullAimIn() || pWeapon->HasAimInFireRateBonus() || pWeapon->HasAimInRecoilBonus() || IsStyleSkillActive(SKILL_MARKSMAN)))
	{
		if (pWeapon->HasAimInFireRateBonus())
			pSetup->fov -= m_Shared.m_flAimIn*da_aimin_fov_delta_low.GetFloat();
		else
			pSetup->fov -= m_Shared.m_flAimIn*da_aimin_fov_delta_high.GetFloat();
	}

	pSetup->fov -= Bias(m_Shared.m_flSlowAimIn, 0.65f)*da_aimin_slow_fov_delta.GetFloat();
}

void C_SDKPlayer::UpdateTeamMenu()
{
	if (!SDKGameRules()->IsTeamplay())
		return;

	static_cast<CFolderMenu*>(gViewPortInterface->FindPanelByName(PANEL_FOLDER))->MarkForUpdate();
}

void RecvProxy_Loadout( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	RecvProxy_Int32ToInt32( pData, pStruct, pOut );

	if (pData && C_SDKPlayer::GetLocalSDKPlayer() && pData->m_ObjectID == C_SDKPlayer::GetLocalSDKPlayer()->entindex())
	{
		static_cast<CFolderMenu*>(gViewPortInterface->FindPanelByName(PANEL_FOLDER))->MarkForUpdate();
	}
}

void RecvProxy_Character( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	RecvProxy_StringToString( pData, pStruct, pOut );

	if (pData && C_SDKPlayer::GetLocalSDKPlayer() && pData->m_ObjectID == C_SDKPlayer::GetLocalSDKPlayer()->entindex())
	{
		static_cast<CFolderMenu*>(gViewPortInterface->FindPanelByName(PANEL_FOLDER))->MarkForUpdate();
	}
}

void RecvProxy_Skill( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	RecvProxy_Int32ToInt32( pData, pStruct, pOut );

	if (pData && C_SDKPlayer::GetLocalSDKPlayer() && pData->m_ObjectID == C_SDKPlayer::GetLocalSDKPlayer()->entindex())
	{
		static_cast<CFolderMenu*>(gViewPortInterface->FindPanelByName(PANEL_FOLDER))->MarkForUpdate();
	}
}

class FlashlightSupressor
{
public:
	bool operator()( C_BasePlayer *player )
	{
		ToSDKPlayer(player)->TurnOffFlashlight();
		return true;
	}
};

void C_SDKPlayer::UpdateFlashlight()
{
	//Tony; keeping this variable for later.
	C_SDKPlayer *pFlashlightPlayer = this;

	if ( !IsAlive() )
	{
		if ( GetObserverMode() == OBS_MODE_IN_EYE )
			pFlashlightPlayer = ToSDKPlayer( GetObserverTarget() );

		ProjectedLightEffectManager(index).TurnOffFlashlight(true);
	}

	if ( !pFlashlightPlayer )
		return;

	///////////////////////////////////////////////////////////
	//Tony; flashlight VECTORS
	/////
	Vector vecForward, vecRight, vecUp;
	EyeVectors( &vecForward, &vecRight, &vecUp );

	if ( this == C_BasePlayer::GetLocalPlayer() )
	{
		CBaseViewModel *vm = GetViewModel( 0 );
		//Tony; if viewmodel is found, override.
		if ( vm )
		{
			Vector dummy;
			QAngle gunAttachmentAngles;
			//Tony; todo; we need a flashlight attachment point on the view models!
			if ( vm->GetAttachment( vm->LookupAttachment("muzzle"), dummy, gunAttachmentAngles ) )
				AngleVectors( gunAttachmentAngles, &vecForward, &vecRight, &vecUp );
		}

		if (GetActiveSDKWeapon())
			pFlashlightPlayer->m_vecFlashlightOrigin = GetActiveSDKWeapon()->GetShootPosition(this);
		else
			pFlashlightPlayer->m_vecFlashlightOrigin = EyePosition();

		pFlashlightPlayer->m_vecFlashlightForward = vecForward;
		pFlashlightPlayer->m_vecFlashlightRight = vecRight;
		pFlashlightPlayer->m_vecFlashlightUp = vecUp;
	}
	else
	{
		//Tony; in order to make this look okay for other players we need to swap the angles so it points down, otherwise it looks dumb.
		pFlashlightPlayer->m_vecFlashlightOrigin = pFlashlightPlayer->GetAbsOrigin() + vecForward * 16.0f; //shift ahead a bit.. i dunno how much i should actually shift yet. but..
		pFlashlightPlayer->m_vecFlashlightForward = -vecUp;//vecForward;
		pFlashlightPlayer->m_vecFlashlightRight = vecRight;
		pFlashlightPlayer->m_vecFlashlightUp = vecForward;//vecUp;
	}

	///////////////////////////////////////////////////////////

	int projManagerIdx = pFlashlightPlayer->index;

	if ( pFlashlightPlayer )
		ProjectedLightEffectManager(projManagerIdx).SetEntityIndex( pFlashlightPlayer->index );

	if ( pFlashlightPlayer && pFlashlightPlayer->IsAlive() && pFlashlightPlayer->IsEffectActive( EF_DIMLIGHT ) )
	{
		// Make sure we're using the proper flashlight texture
		const char *pszTextureName = pFlashlightPlayer->GetFlashlightTextureName();

		if ( !m_bFlashlightEnabled )
		{
			// Turned on the light; create it.
			if ( pszTextureName )
			{
				ProjectedLightEffectManager(projManagerIdx).TurnOnFlashlight( pFlashlightPlayer->index, pszTextureName, pFlashlightPlayer->GetFlashlightFOV(),
					pFlashlightPlayer->GetFlashlightFarZ(), pFlashlightPlayer->GetFlashlightLinearAtten() );
			}
			else
			{
				ProjectedLightEffectManager(projManagerIdx).TurnOnFlashlight( pFlashlightPlayer->index );
			}
			m_bFlashlightEnabled = true;
		}
	}
	else if ( m_bFlashlightEnabled )
	{
		ProjectedLightEffectManager(projManagerIdx).TurnOffFlashlight();
		m_bFlashlightEnabled = false;
	}

	if ( pFlashlightPlayer )
	{
		Vector vecForward, vecRight, vecUp;
		Vector vecPos;
		//Check to see if we have an externally specified flashlight origin, if not, use eye vectors/render origin
		if ( pFlashlightPlayer->m_vecFlashlightOrigin != vec3_origin && pFlashlightPlayer->m_vecFlashlightOrigin.IsValid() )
		{
			vecPos = pFlashlightPlayer->m_vecFlashlightOrigin;
			vecForward = pFlashlightPlayer->m_vecFlashlightForward;
			vecRight = pFlashlightPlayer->m_vecFlashlightRight;
			vecUp = pFlashlightPlayer->m_vecFlashlightUp;
		}
		else
		{
			EyeVectors( &vecForward, &vecRight, &vecUp );
			vecPos = GetRenderOrigin() + m_vecViewOffset;
		}

		// Give the muzzle flash a random rotation so that the texture looks more varied.
		VMatrix mMuzzleFlash(vecForward, -vecRight, vecUp, vecPos);

		VMatrix mRotate = SetupMatrixAxisRot(Vector(1, 0, 0), m_flMuzzleFlashYaw);

		VMatrix mNew = mMuzzleFlash * mRotate;

		mNew.GetBasisVectors(vecForward, vecRight, vecUp);
		vecRight = -vecRight;

		Assert((mMuzzleFlash.GetForward() - vecForward).LengthSqr() < 0.001f);

		// Update the light with the new position and direction.		
		ProjectedLightEffectManager(projManagerIdx).UpdateFlashlight( vecPos, vecForward, vecRight, vecUp, pFlashlightPlayer->GetFlashlightFOV(), 
			pFlashlightPlayer->CastsFlashlightShadows(), pFlashlightPlayer->GetFlashlightFarZ(), pFlashlightPlayer->GetFlashlightLinearAtten(),
			pFlashlightPlayer->GetFlashlightTextureName() );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Turns off flashlight if it's active (TERROR)
//-----------------------------------------------------------------------------
void C_SDKPlayer::TurnOffFlashlight( void )
{
	int nSlot = this->index;

	if ( m_bFlashlightEnabled )
	{
		ProjectedLightEffectManager( nSlot ).TurnOffFlashlight();
		m_bFlashlightEnabled = false;
	}
}

float C_SDKPlayer::GetUserInfoFloat(const char* pszCVar, float flBotDefault)
{
	ConVarRef UserInfoVar(pszCVar);

	return UserInfoVar.GetFloat();
}

int C_SDKPlayer::GetUserInfoInt(const char* pszCVar, int iBotDefault)
{
	ConVarRef UserInfoVar(pszCVar);

	return UserInfoVar.GetInt();
}

ConVar da_vr_hud( "da_vr_hud", "0", FCVAR_DEVELOPMENTONLY );

bool C_SDKPlayer::UseVRHUD() const
{
	return UseVR() || da_vr_hud.GetBool();
}

class CSlowIntensityProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
};


bool CSlowIntensityProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if (!CResultProxy::Init( pMaterial, pKeyValues ))
		return false;
	return true;
}

ConVar da_postprocess_slowmo("da_postprocess_slowmo", "0", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY);

void CSlowIntensityProxy::OnBind( void *pC_BaseEntity )
{
	Assert( m_pResult );

	float flValue = 0;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();

	if ( pPlayer )
	{
		if (da_postprocess_slowmo.GetBool())
			flValue = 1;
		else
			flValue = RemapValClamped(pPlayer->GetSlowMoMultiplier(), 1, pPlayer->GetSlowMoGoal()-0.01f, 0, 1);

		if (!pPlayer->IsAlive() && pPlayer->GetObserverMode() == OBS_MODE_FREEZECAM)
			flValue = 1;
	}

	SetFloatResult( flValue );

	if ( ToolsEnabled() )
		ToolFramework_RecordMaterialParams( GetMaterial() );
}

EXPOSE_INTERFACE( CSlowIntensityProxy, IMaterialProxy, "SlowIntensity" IMATERIAL_PROXY_INTERFACE_VERSION );

class CSlowMoMultiplierProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );

private:
	CFloatInput m_Input;
};


bool CSlowMoMultiplierProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if (!CResultProxy::Init( pMaterial, pKeyValues ))
		return false;

	m_Input.Init( pMaterial, pKeyValues, "input", 1.0f );

	return true;
}

void CSlowMoMultiplierProxy::OnBind( void *pC_BaseEntity )
{
	Assert( m_pResult );

	float flValue = 1;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalOrSpectatedPlayer();

	if ( pPlayer )
		flValue = m_Input.GetFloat() * pPlayer->GetSlowMoMultiplier();

	SetFloatResult( flValue );

	if ( ToolsEnabled() )
		ToolFramework_RecordMaterialParams( GetMaterial() );
}

EXPOSE_INTERFACE( CSlowMoMultiplierProxy, IMaterialProxy, "SlowMoMultiplier" IMATERIAL_PROXY_INTERFACE_VERSION );

class CMarksmanGoldSkinProxy : public CEntityMaterialProxy
{
public:
	CMarksmanGoldSkinProxy( void );
	virtual ~CMarksmanGoldSkinProxy( void );

public:
	virtual bool       Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void       OnBind( C_BaseEntity *pC_BaseEntity );
	virtual IMaterial* GetMaterial();

private:
	IMaterialVar* m_pDetail;
	IMaterialVar* m_pDetailScale;
	IMaterialVar* m_pDetailBlend;
	IMaterialVar* m_pDetailMode;
};

CMarksmanGoldSkinProxy::CMarksmanGoldSkinProxy()
{
	m_pDetail = NULL;
	m_pDetailScale = NULL;
	m_pDetailBlend = NULL;
	m_pDetailMode = NULL;
}

CMarksmanGoldSkinProxy::~CMarksmanGoldSkinProxy()
{
}

bool CMarksmanGoldSkinProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool bFoundDetail, bFoundScale, bFoundBlend, bFoundMode;

	m_pDetail = pMaterial->FindVar( "$detail", &bFoundDetail, false );
	m_pDetailScale = pMaterial->FindVar( "$detailscale", &bFoundScale, false );
	m_pDetailBlend = pMaterial->FindVar( "$detailblendfactor", &bFoundBlend, false );
	m_pDetailMode = pMaterial->FindVar( "$detailblendmode", &bFoundMode, false );

	return bFoundDetail && bFoundScale && bFoundBlend && bFoundMode;
}

void CMarksmanGoldSkinProxy::OnBind( C_BaseEntity *pEnt )
{
	C_WeaponSDKBase* pWeapon = dynamic_cast<C_WeaponSDKBase*>(pEnt);

	if (!pWeapon)
	{
		C_DAViewModel* pViewModel = dynamic_cast<C_DAViewModel*>(pEnt);
		if (pViewModel)
			pWeapon = pViewModel->GetDAWeapon();
	}

	float flBlend = 0;
	if (pWeapon)
		flBlend = pWeapon->GetMarksmanGold();

	if (m_pDetailBlend)
		m_pDetailBlend->SetFloatValue( flBlend );
}

IMaterial *CMarksmanGoldSkinProxy::GetMaterial()
{
	if ( !m_pDetail )
		return NULL;

	return m_pDetail->GetOwningMaterial();
}

EXPOSE_INTERFACE( CMarksmanGoldSkinProxy, IMaterialProxy, "GoldSkin" IMATERIAL_PROXY_INTERFACE_VERSION );

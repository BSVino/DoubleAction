//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_basesdkgrenade.h"
#include "in_buttons.h"	


#ifdef CLIENT_DLL

	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"
	#include "items.h"

#endif


#define GRENADE_RADIUS	2.0f // inches


IMPLEMENT_NETWORKCLASS_ALIASED( BaseSDKGrenade, DT_BaseSDKGrenade )

BEGIN_NETWORK_TABLE(CBaseSDKGrenade, DT_BaseSDKGrenade)

#ifndef CLIENT_DLL
	SendPropBool( SENDINFO(m_bRedraw) ),
	SendPropBool( SENDINFO(m_bPinPulled) ),
	SendPropFloat( SENDINFO(m_fThrowTime), 0, SPROP_NOSCALE ),
#else
	RecvPropBool( RECVINFO(m_bRedraw) ),
	RecvPropBool( RECVINFO(m_bPinPulled) ),
	RecvPropFloat( RECVINFO(m_fThrowTime) ),
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CBaseSDKGrenade )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_basesdkgrenade, CBaseSDKGrenade );


CBaseSDKGrenade::CBaseSDKGrenade()
{
	m_bRedraw = false;
	m_bPinPulled = false;
	m_fThrowTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseSDKGrenade::Precache()
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseSDKGrenade::Deploy()
{
	m_bRedraw = false;
	m_bPinPulled = false;
	m_fThrowTime = 0;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseSDKGrenade::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_bPinPulled = false; // when this is holstered make sure the pin isn’t pulled.
	m_fThrowTime = 0;

#ifndef CLIENT_DLL
	// If they attempt to switch weapons before the throw animation is done, 
	// allow it, but kill the weapon if we have to.
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		CBaseCombatCharacter *pOwner = (CBaseCombatCharacter *)pPlayer;
		pOwner->Weapon_Drop( this );
		UTIL_Remove(this);
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseSDKGrenade::PrimaryAttack()
{
	if ( m_bRedraw || m_bPinPulled )
		return;

	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer || pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	// The pull pin animation has to finish, then we wait until they aren't holding the primary
	// attack button, then throw the grenade.
	SendWeaponAnim( ACT_VM_PULLPIN );
	m_bPinPulled = true;
	
	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	//Tony; updated; minimum grenade tossing time is 1 second delay! + sequence
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseSDKGrenade::SecondaryAttack()
{
	if ( m_bRedraw )
		return;

	CSDKPlayer *pPlayer = GetPlayerOwner();
	
	if ( pPlayer == NULL )
		return;

	//See if we're ducking
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_HAULBACK );
	}

	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	//Tony; updated; minimum grenade tossing time is 1 second delay! + sequence
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseSDKGrenade::Reload()
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		//Tony; updated; minimum grenade tossing time is 1 second delay! + sequence
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;

		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseSDKGrenade::ItemPostFrame()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
	if ( !vm )
		return;

	// If they let go of the fire button, they want to throw the grenade.
	if ( m_bPinPulled && !(pPlayer->m_nButtons & IN_ATTACK) ) 
	{
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
//		if (m_bSecondary)
//			DropGrenade();
//		else
			ThrowGrenade();
		
		DecrementAmmo( pPlayer );
	
		m_bPinPulled = false;
		SendWeaponAnim( ACT_VM_THROW );	
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		m_bPinPulled = false;
//		m_bSecondary = false;
	}
	else if( m_bRedraw )
	{
		// Has the throw animation finished playing
		if( m_flTimeWeaponIdle < gpGlobals->curtime )
		{
			// if we're officially out of grenades, ditch this weapon
			if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
			{
#ifdef GAME_DLL
				pPlayer->Weapon_Drop( this, NULL, NULL );
				UTIL_Remove(this);
#endif
				pPlayer->SwitchToNextBestWeapon( NULL ); //Tony; now switch! cuz we rans outs!
			}
			else
			{
				m_bRedraw = false;
				m_flNextPrimaryAttack = gpGlobals->curtime + 1.2;
				m_flNextSecondaryAttack = gpGlobals->curtime + 1.2;
				SendWeaponAnim( GetDeployActivity() );			
			}
			return;	//don't animate this grenade any more!
		}	
	}
	else if( !m_bRedraw )
	{
		BaseClass::ItemPostFrame();
	}
}



#ifdef CLIENT_DLL

	void CBaseSDKGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
	}

	void CBaseSDKGrenade::DropGrenade()
	{
		m_bRedraw = true;
		m_fThrowTime = 0.0f;
		//Tony; updated; minimum grenade tossing time is 1 second delay! + sequence
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
	}

	void CBaseSDKGrenade::ThrowGrenade()
	{
		m_bRedraw = true;
		m_fThrowTime = 0.0f;
		//Tony; updated; minimum grenade tossing time is 1 second delay! + sequence
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration() + 1.0;
	}

	void CBaseSDKGrenade::StartGrenadeThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;
	}

#else

	BEGIN_DATADESC( CBaseSDKGrenade )
		DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	END_DATADESC()

	int CBaseSDKGrenade::CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1; 
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *pOwner - 
	//-----------------------------------------------------------------------------
	void CBaseSDKGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
	{
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	void CBaseSDKGrenade::StartGrenadeThrow()
	{
		m_fThrowTime = gpGlobals->curtime + 0.1f;
	}
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void CBaseSDKGrenade::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
			pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );

		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}

	void CBaseSDKGrenade::ThrowGrenade()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		QAngle angThrow = pPlayer->LocalEyeAngles();

		Vector vForward, vRight, vUp;

		if (angThrow.x < 90 )
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
		else
		{
			angThrow.x = 360.0f - angThrow.x;
			angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
		}

		float flVel = (90 - angThrow.x) * 8;

		if (flVel > 850)
			flVel = 850;

		AngleVectors( angThrow, &vForward, &vRight, &vUp );

		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

		vecSrc += vForward * 16;
	
		Vector vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();

		EmitGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, this );

		m_bRedraw = true;
		m_fThrowTime = 0.0f;
	}

	void CBaseSDKGrenade::DropGrenade()
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
		Vector vecSrc;
		pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
		vecSrc.z += GRENADE_RADIUS;

		Vector vecFacing = pPlayer->BodyDirection2D( );
		// no up/down direction
		vecFacing.z = 0;
		VectorNormalize( vecFacing );
		trace_t tr;
		UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			// compute forward vec parallel to floor plane and roll grenade along that
			Vector tangent;
			CrossProduct( vecFacing, tr.plane.normal, tangent );
			CrossProduct( tr.plane.normal, tangent, vecFacing );
		}
		vecSrc += (vecFacing * 18.0);
		CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

		Vector vecThrow;
		pPlayer->GetVelocity( &vecThrow, NULL );
		vecThrow += vecFacing * 300;
		// put it on its side
		QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
		// roll it
		AngularImpulse rotSpeed(0,0,320);

		EmitGrenade( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, this );

		m_bRedraw = true;
		m_fThrowTime = 0.0f;
	}

	void CBaseSDKGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
	{
		Assert( 0 && "CBaseSDKGrenade::EmitGrenade should not be called. Make sure to implement this in your subclass!\n" );
	}

	bool CBaseSDKGrenade::AllowsAutoSwitchFrom( void ) const
	{
		return !m_bPinPulled;
	}

#endif


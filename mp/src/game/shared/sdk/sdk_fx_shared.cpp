//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "sdk_fx_shared.h"
#include "weapon_sdkbase.h"
#include "weapon_akimbobase.h"

#ifdef CLIENT_DLL
#include "prediction.h"
#else
#include "ilagcompensationmanager.h"
#endif

#ifdef CLIENT_DLL

#include "fx_impact.h"
#include "c_sdk_player.h"
#include "projectedlighteffect.h"

	// this is a cheap ripoff from CBaseCombatWeapon::WeaponSound():
	void FX_WeaponSound(
		int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		CSDKWeaponInfo *pWeaponInfo )
	{

		// If we have some sounds from the weapon classname.txt file, play a random one of them
		const char *shootsound = pWeaponInfo->aShootSounds[ sound_type ]; 
		if ( !shootsound || !shootsound[0] )
			return;

		CBroadcastRecipientFilter filter; // this is client side only
		if ( !te->CanPredict() )
			return;
				
		CBaseEntity::EmitSound( filter, iPlayerIndex, shootsound, &vOrigin ); 
	}

	class CGroupedSound
	{
	public:
		string_t m_SoundName;
		Vector m_vPos;
	};

	CUtlVector<CGroupedSound> g_GroupedSounds;

	
	// Called by the ImpactSound function.
	void ShotgunImpactSoundGroup( const char *pSoundName, const Vector &vEndPos )
	{
		// Don't play the sound if it's too close to another impact sound.
		for ( int i=0; i < g_GroupedSounds.Count(); i++ )
		{
			CGroupedSound *pSound = &g_GroupedSounds[i];

			if ( vEndPos.DistToSqr( pSound->m_vPos ) < 300*300 )
			{
				if ( Q_stricmp( pSound->m_SoundName, pSoundName ) == 0 )
					return;
			}
		}

		// Ok, play the sound and add it to the list.
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, NULL, pSoundName, &vEndPos );

		int j = g_GroupedSounds.AddToTail();
		g_GroupedSounds[j].m_SoundName = pSoundName;
		g_GroupedSounds[j].m_vPos = vEndPos;
	}


	void StartGroupingSounds()
	{
		Assert( g_GroupedSounds.Count() == 0 );
		SetImpactSoundRoute( ShotgunImpactSoundGroup );
	}


	void EndGroupingSounds()
	{
		g_GroupedSounds.Purge();
		SetImpactSoundRoute( NULL );
	}

#else

	#include "sdk_player.h"
	#include "te_firebullets.h"
	
	// Server doesn't play sounds anyway.
	void StartGroupingSounds() {}
	void EndGroupingSounds() {}
	void FX_WeaponSound ( int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		CSDKWeaponInfo *pWeaponInfo ) {};

#endif



// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const QAngle &vAngles,
	int	iWeaponID,
	int	iMode,
	int iSeed,
	float flSpread
	)
{
	Assert(vOrigin.IsValid());

	bool bDoEffects = true;

#ifdef CLIENT_DLL
	C_SDKPlayer *pPlayer = ToSDKPlayer( ClientEntityList().GetBaseEntity( iPlayerIndex ) );
#else
	CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( iPlayerIndex) );
#endif

	CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo((SDKWeaponID)iWeaponID);
	if ( !pWeaponInfo )
	{
		DevMsg("FX_FireBullets: weapon alias for ID %i not found\n", iWeaponID );
		Assert(pWeaponInfo);
		return;
	}

#ifdef CLIENT_DLL
	bDoEffects = prediction->IsFirstTimePredicted();

	// Do the firing animation event.
	if ( pPlayer && !pPlayer->IsDormant() && bDoEffects )
	{
		pPlayer->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
	}
#else
	// if this is server code, send the effect over to client as temp entity
	// Dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( 
		iPlayerIndex,
		vOrigin, 
		vAngles, 
		iWeaponID,
		iMode,
		iSeed,
		flSpread
		);

	bDoEffects = false; // no effects on server
#endif

	iSeed++;

	int		iDamage = pWeaponInfo->m_iDamage;
	int		iAmmoType = pWeaponInfo->iAmmoType;

	WeaponSound_t sound_type = SINGLE;

	if ( bDoEffects)
	{
#ifdef CLIENT_DLL
		if (pPlayer)
			pPlayer->m_flMuzzleFlashYaw = random->RandomFloat(0, 360);
		//ProjectedLightEffectManager( iPlayerIndex ).TriggerMuzzleFlash();
#endif

		FX_WeaponSound( iPlayerIndex, sound_type, vOrigin, pWeaponInfo );
	}


	// Fire bullets, calculate impacts & effects

	if ( !pPlayer )
		return;

	if (*pWeaponInfo->m_szSingle)
	{
		CWeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();
		if (pWeapon)
		{
			Assert(pWeapon->IsAkimbo());
			CAkimboBase* pAkimbo = dynamic_cast<CAkimboBase*>(pWeapon);
			Assert(pAkimbo);
			if (pAkimbo)
				pPlayer->DoMuzzleFlash(pAkimbo->m_bShootRight?2:1);
		}
	}
	else
		pPlayer->DoMuzzleFlash();

	pPlayer->ReadyWeapon();

	StartGroupingSounds();

#if !defined (CLIENT_DLL)
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );

	const int iMaxPlayers = 64;
	Vector avecMins[iMaxPlayers];
	Vector avecMaxs[iMaxPlayers];

	for (int i = 1; i <= min(iMaxPlayers, gpGlobals->maxClients); i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		avecMins[i] = pPlayer->CollisionProp()->OBBMins();
		avecMaxs[i] = pPlayer->CollisionProp()->OBBMaxs();

		pPlayer->SetSize(avecMins[i]*3, avecMaxs[i]*3);
	}
#endif

	for ( int iBullet=0; iBullet < pWeaponInfo->m_iBullets; iBullet++ )
	{
		RandomSeed( iSeed );	// init random system with this seed

		// Get circular gaussian spread.
		float x, y;
		x = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
		y = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
	
		iSeed++; // use new seed for next bullet

		pPlayer->FireBullet(
			vOrigin,
			vAngles,
			flSpread,
			(SDKWeaponID)iWeaponID,
			iDamage,
			iAmmoType,
			pPlayer,
			bDoEffects,
			x,y );
	}

	// Adjust the style charge spend for the fire rate of the weapon so that weapons with a high ROF don't suck it up.
	pPlayer->UseStyleCharge(SKILL_MARKSMAN, pWeaponInfo->m_flCycleTime * 10);

#if !defined (CLIENT_DLL)
	for (int i = 1; i <= min(iMaxPlayers, gpGlobals->maxClients); i++)
	{
		CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer)
			continue;

		pPlayer->SetSize(avecMins[i], avecMaxs[i]);
	}

	lagcompensation->FinishLagCompensation( pPlayer );
#endif

	EndGroupingSounds();
}


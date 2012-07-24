//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_sdkbase.h"
#include "ammodef.h"

#include "sdk_fx_shared.h"
#include "sdk_gamerules.h"

#if defined( CLIENT_DLL )

	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif

// ----------------------------------------------------------------------------- //
// CWeaponSDKBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBase, DT_WeaponSDKBase )

BEGIN_NETWORK_TABLE( CWeaponSDKBase, DT_WeaponSDKBase )
#ifdef CLIENT_DLL
  	RecvPropFloat( RECVINFO( m_flDecreaseShotsFired ) ),
  	RecvPropFloat( RECVINFO( m_flAccuracyDecay ) ),
#else
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropFloat( SENDINFO( m_flDecreaseShotsFired ) ),
	SendPropFloat( SENDINFO( m_flAccuracyDecay ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSDKBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flAccuracyDecay, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sdk_base, CWeaponSDKBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponSDKBase )

		// New weapon Think and Touch Functions go here..

	END_DATADESC()

#endif

#ifdef CLIENT_DLL
bool CWeaponSDKBase::ShouldPredict()
{
       if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
               return true;

       return BaseClass::ShouldPredict();
}
#endif
// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponSDKBase::CWeaponSDKBase()
{
	SetPredictionEligible( true );

	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_flAccuracyDecay = 0;
}

const CSDKWeaponInfo &CWeaponSDKBase::GetSDKWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CSDKWeaponInfo *pSDKInfo;

	#ifdef _DEBUG
		pSDKInfo = dynamic_cast< const CSDKWeaponInfo* >( pWeaponInfo );
		Assert( pSDKInfo );
	#else
		pSDKInfo = static_cast< const CSDKWeaponInfo* >( pWeaponInfo );
	#endif

	return *pSDKInfo;
}

bool CWeaponSDKBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	
	return 0;
}

CSDKPlayer* CWeaponSDKBase::GetPlayerOwner() const
{
	return dynamic_cast< CSDKPlayer* >( GetOwner() );
}

#ifdef CLIENT_DLL
void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle	final = in + punch;

	//Clip each component
	for ( int i = 0; i < 3; i++ )
	{
		if ( final[i] > clip[i] )
		{
			final[i] = clip[i];
		}
		else if ( final[i] < -clip[i] )
		{
			final[i] = -clip[i];
		}

		//Return the result
		in[i] = final[i] - punch[i];
	}
}
#endif

ConVar dab_fulldecay( "dab_fulldecay", "0.5", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The maximum accuracy decay." );
ConVar dab_coldaccuracymultiplier( "dab_coldaccuracymultiplier", "0.25", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The accuracy of a cold barrel as a multiplier of the original accuracy." );
ConVar dab_decayrate( "dab_decayrate", "2", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "A multiplier for the accuracy decay rate of weapons as they fire." );

//Tony; added as a default primary attack if it doesn't get overridden, ie: by CSDKWeaponMelee
void CWeaponSDKBase::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	//Tony; check firemodes -- 
	switch(GetFireMode())
	{
	case FM_SEMIAUTOMATIC:
		if (pPlayer->GetShotsFired() > 0)
			return;
		break;
		//Tony; added an accessor to determine the max burst on a per-weapon basis.
	case FM_BURST:
		if (pPlayer->GetShotsFired() > MaxBurstShots())
			return;
		break;
	}
#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
	{
		// Marksmen don't consume ammo while their skill is active.
	}
	else
	{
		// Make sure we don't fire more than the amount in the clip
		if ( UsesClipsForAmmo1() )
			m_iClip1 --;
		else
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType );
	}

	pPlayer->IncreaseShotsFired();

	float flSpread = GetWeaponSpread();

	if (pPlayer->m_Shared.IsAimedIn() && !WeaponSpreadFixed())
	{
		if (GetSDKWpnData().m_bAimInSpreadBonus)
			flSpread *= RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, 0.5f);
		else
			// Since weapons without the bonus are generally capped at 50% aim in, this ends up being a .8 multiplier.
			flSpread *= RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 1, 0.6f);
	}

	if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
		flSpread *= 0.5f;

	if (!WeaponSpreadFixed())
		flSpread *= RemapValClamped(m_flAccuracyDecay, 0, dab_fulldecay.GetFloat(), dab_coldaccuracymultiplier.GetFloat(), 1);

	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + pPlayer->GetPunchAngle(),
		GetWeaponID(),
		0, //Tony; fire mode - this is unused at the moment, left over from CSS when SDK* was created in the first place.
		CBaseEntity::GetPredictionRandomSeed() & 255,
		flSpread
		);

	//Add our view kick in
	AddViewKick();

	//Tony; update our weapon idle time
	SetWeaponIdleTime( GetCurrentTime() + SequenceDuration() );

	float flFireRate = GetFireRate();
	if (pPlayer->m_Shared.IsAimedIn() && HasAimInFireRateBonus())
	{
		// We lerp from .8 instead of 1 to be a bit more forgiving when the player first taps the aim button.
		float flFireRateMultiplier = RemapVal(pPlayer->m_Shared.GetAimIn(), 0, 1, 0.8f, 0.7f);

		flFireRate *= flFireRateMultiplier;

		CBaseViewModel* vm = pPlayer->GetViewModel( m_nViewModelIndex );

		if (vm)
			vm->SetPlaybackRate( 1/flFireRateMultiplier );
	}

	if (m_flAccuracyDecay < 0)
		m_flAccuracyDecay = 0;

	// Weapons that fire quickly should decay slower.
	m_flAccuracyDecay += (flFireRate * dab_decayrate.GetFloat());

	if (m_flAccuracyDecay > dab_fulldecay.GetFloat())
		m_flAccuracyDecay = dab_fulldecay.GetFloat();

	m_flNextPrimaryAttack = GetCurrentTime() + flFireRate;
	m_flNextSecondaryAttack = GetCurrentTime() + SequenceDuration();
}

void CWeaponSDKBase::SecondaryAttack()
{
}

void CWeaponSDKBase::AddViewKick()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if ( pPlayer )
	{
		// Update punch angles.
		QAngle angle = pPlayer->GetPunchAngle();

		angle.x -= SharedRandomInt( "PunchAngle", 4, 6 );

		float flRecoilBonus = 1;
		if (GetPlayerOwner()->m_Shared.IsAimedIn())
		{
			if (HasAimInRecoilBonus())
				flRecoilBonus = RemapVal(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 1, 1, 0.5f);
			else
				// Since weapons without the bonus are generally capped at 50% aim in, this ends up being a .8 multiplier.
				flRecoilBonus = RemapVal(GetPlayerOwner()->m_Shared.GetAimIn(), 0, 1, 1, 0.6f);
		}

		if (pPlayer->IsStyleSkillActive() && pPlayer->m_Shared.m_iStyleSkill == SKILL_MARKSMAN)
			flRecoilBonus *= 0.5f;

		pPlayer->SetPunchAngle( angle * GetViewPunchMultiplier() * flRecoilBonus );
		pPlayer->m_Shared.SetRecoil(SharedRandomFloat("Recoil", 1, 1.1f) * GetRecoil() * flRecoilBonus);
	}
}

float CWeaponSDKBase::GetWeaponSpread()
{
	return GetSDKWpnData().m_flSpread;
}

#ifdef CLIENT_DLL
void CWeaponSDKBase::CreateMove(float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles)
{
	BaseClass::CreateMove(flInputSampleTime, pCmd, vecOldViewAngles);

	Vector vecRecoil = GetPlayerOwner()->m_Shared.GetRecoil(flInputSampleTime);
	pCmd->viewangles[PITCH] -= vecRecoil.y;
	pCmd->viewangles[YAW] += vecRecoil.x;
}
#endif

float CWeaponSDKBase::GetViewPunchMultiplier()
{
	return GetSDKWpnData().m_flViewPunchMultiplier;
}

float CWeaponSDKBase::GetRecoil()
{
	return GetSDKWpnData().m_flRecoil;
}

bool CWeaponSDKBase::HasAimInSpeedPenalty()
{
	return GetSDKWpnData().m_bAimInSpeedPenalty;
}

bool CWeaponSDKBase::HasAimInFireRateBonus()
{
	return GetSDKWpnData().m_bAimInFireRateBonus;
}

bool CWeaponSDKBase::HasAimInRecoilBonus()
{
	return GetSDKWpnData().m_bAimInRecoilBonus;
}

weapontype_t CWeaponSDKBase::GetWeaponType() const
{
	return GetSDKWpnData().m_eWeaponType;
}

ConVar dab_decaymultiplier( "dab_decaymultiplier", "0.7", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "The multiplier for the recoil decay rate." );

//Tony; added so we can have base functionality without implementing it into every weapon.
void CWeaponSDKBase::ItemPostFrame( void )
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	//
	//Tony; totally override the baseclass
	//

	if ( UsesClipsForAmmo1() )
		CheckReload();

	// A multiplier of 1 means that for every second of firing the player needs to wait one second to get back to full accuracy.
	m_flAccuracyDecay -= (gpGlobals->frametime * dab_decaymultiplier.GetFloat() * pPlayer->GetSlowMoMultiplier());

	if (m_flAccuracyDecay < 0)
		m_flAccuracyDecay = 0;

	bool bFired = false;

	// Secondary attack has priority
	if ((pPlayer->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
	{
		if (UsesSecondaryAmmo() && pPlayer->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < GetCurrentTime())
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = GetCurrentTime() + 0.5;
			}
		}
		else if (pPlayer->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = GetCurrentTime() + 0.2;
		}
		else
		{
			bFired = true;
			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}
	
	if ( !bFired && (pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= GetCurrentTime()) && pPlayer->CanAttack())
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() && (( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			HandleFireOnEmpty();
		}
		else if (pPlayer->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = GetCurrentTime() + 0.2;
		}
		else
		{
			PrimaryAttack();
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( pPlayer->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pPlayer->m_nButtons & IN_ATTACK) || (pPlayer->m_nButtons & IN_ATTACK2) || (pPlayer->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}

	// Tony; decrease shots fired count - tweak the time as necessary.
	if ( !( pPlayer->m_nButtons & IN_ATTACK ) )
	{
		//Tony; check firemodes -- If we're semi or burst, we will clear shots fired now that the player has let go of the button.
		switch(GetFireMode())
		{
		case FM_SEMIAUTOMATIC:
			if (pPlayer->GetShotsFired() > 0)
				pPlayer->ClearShotsFired();
			break;
			//Tony; TODO; add an accessor to determine the max burst on a per-weapon basis!!
			//DONE!
		case FM_BURST:
			if (pPlayer->GetShotsFired() > MaxBurstShots())
				pPlayer->ClearShotsFired();
			break;
		}

		m_bFireOnEmpty = false;
		if ( (pPlayer->GetShotsFired() > 0) && (m_flDecreaseShotsFired < GetCurrentTime())	)
		{
			m_flDecreaseShotsFired = GetCurrentTime() + 0.05495;
			pPlayer->DecreaseShotsFired();
		}
	}
}

extern bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer );

void CWeaponSDKBase::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
#ifdef GAME_DLL
	CSDKPlayer *pPlayer = ToSDKPlayer( pActivator );
	
	if ( !pPlayer )
		return;

	// Can I have this weapon type?
	if ( !pPlayer->IsAllowedToPickupWeapons() )
		return;

	if ( GetOwner() || !pPlayer->Weapon_CanUse( this ) || !g_pGameRules->CanHavePlayerItem( pPlayer, this ) )
		return;

	// Don't let the player touch the item unless unobstructed
	if ( !UTIL_ItemCanBeTouchedByPlayer( this, pPlayer ) )
		return;

	int iWeaponsWeight = 0;
	for (int i = 0; i < pPlayer->WeaponCount(); i++)
	{
		if (!pPlayer->GetWeapon(i))
			continue;

		iWeaponsWeight += static_cast<CWeaponSDKBase*>(pPlayer->GetWeapon(i))->GetWeight();
	}

	if (GetWeight() + iWeaponsWeight > MAX_LOADOUT_WEIGHT)
		return;

	// ----------------------------------------
	// If I already have it just take the ammo
	// ----------------------------------------
	if (pPlayer->Weapon_OwnsThisType( GetClassname(), GetSubType())) 
	{
		if ( pPlayer->Weapon_EquipAmmoOnly( this ) )
		{
			// Only remove me if I have no ammo left
			if ( HasPrimaryAmmo() )
				return;

			UTIL_Remove( this );
			OnPickedUp( pPlayer );
			return;
		}
		else
			return;
	}
	// -------------------------
	// Otherwise take the weapon
	// -------------------------
	else 
	{
		CheckRespawn();

		AddSolidFlags( FSOLID_NOT_SOLID );
		AddEffects( EF_NODRAW );

		pPlayer->Weapon_Equip( this );
		if ( pPlayer->IsInAVehicle() )
		{
			Holster(NULL);
		}
		else
		{
			// If it uses clips, load it full. (this is the first time you've picked up this type of weapon)
			if ( UsesClipsForAmmo1() )
			{
				m_iClip1 = GetMaxClip1();
			}

			pPlayer->Weapon_Switch( this );
		}

		OnPickedUp( pPlayer );
	}
#endif
}

void CWeaponSDKBase::WeaponIdle( void )
{
	//Idle again if we've finished
	if ( HasWeaponIdleTimeElapsed() )
	{
		SendWeaponAnim( GetIdleActivity() );
		SetWeaponIdleTime( GetCurrentTime() + SequenceDuration() );
	}
}

bool CWeaponSDKBase::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), GetReloadActivity() );
	if ( fRet )
	{
		float flSpeedMultiplier = GetSDKWpnData().m_flReloadTimeMultiplier;

		if (GetPlayerOwner()->IsStyleSkillActive() && GetPlayerOwner()->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
			flSpeedMultiplier *= 0.6f;

		float flSequenceEndTime = GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;

		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		if (pOwner)
		{
			CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );

			if (vm)
				vm->SetPlaybackRate( 1/flSpeedMultiplier );
		}

		MDLCACHE_CRITICAL_SECTION();
		if (GetPlayerOwner())
			GetPlayerOwner()->SetNextAttack( flSequenceEndTime );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

		SendReloadEvents();

		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
		
		WeaponSound( RELOAD );
		if (GetPlayerOwner()) 
			GetPlayerOwner()->ClearShotsFired();
	}

	return fRet;
}

void CWeaponSDKBase::SendReloadEvents()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#ifdef GAME_DLL
	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );

	UserMessageBegin( filter, "ReloadEffect" );
	WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();
#endif
	// Make the player play his reload animation.
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

void CWeaponSDKBase::CheckReload()
{
	if ( m_bReloadsSingly )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( !pOwner )
			return;

		if ((m_bInReload) && (m_flNextPrimaryAttack <= GetCurrentTime()))
		{
			if ( pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0 )
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <=0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				// Add them to the clip
				m_iClip1 += 1;
				pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				m_flNextPrimaryAttack	= GetCurrentTime();
				m_flNextSecondaryAttack = GetCurrentTime();
				return;
			}
		}
	}
	else
	{
		if ( (m_bInReload) && (m_flNextPrimaryAttack <= GetCurrentTime()))
		{
			FinishReload();
			m_flNextPrimaryAttack	= GetCurrentTime();
			m_flNextSecondaryAttack = GetCurrentTime();
			m_bInReload = false;
		}
	}
}

bool CWeaponSDKBase::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < GetCurrentTime() && m_flNextSecondaryAttack < GetCurrentTime() )
	{
		// weapon isn't useable, switch.
		if ( ( (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false ) && ( g_pGameRules->SwitchToNextBestWeapon( pOwner, this ) ) )
		{
			m_flNextPrimaryAttack = GetCurrentTime() + 0.3;
			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 m_flNextPrimaryAttack < GetCurrentTime() && 
			 m_flNextSecondaryAttack < GetCurrentTime() )
		{
			// if we're successfully reloading, we're done
			if ( Reload() )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponSDKBase::Deploy( )
{
	MDLCACHE_CRITICAL_SECTION();

	//Tony; on deploy clear shots fired.
	if (GetPlayerOwner())
		GetPlayerOwner()->ClearShotsFired();

	bool bDeploy = DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDeployActivity(), (char*)GetAnimPrefix() );

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());

	float flSpeedMultiplier = GetSDKWpnData().m_flDrawTimeMultiplier;

	if (pOwner->IsStyleSkillActive() && pOwner->m_Shared.m_iStyleSkill == SKILL_ADRENALINE)
		flSpeedMultiplier *= 0.6f;

	m_flNextPrimaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;
	m_flNextSecondaryAttack	= GetCurrentTime() + SequenceDuration() * flSpeedMultiplier;

	if (pOwner)
	{
		CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );

		if (vm)
			vm->SetPlaybackRate( 1/flSpeedMultiplier );

		pOwner->SetNextAttack( GetCurrentTime() + SequenceDuration() * flSpeedMultiplier );
	}

	pOwner->m_Shared.SetAimIn(0.0f);

	return bDeploy;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//Tony; use the same name as the base context one.
#define SDK_HIDEWEAPON_THINK_CONTEXT			"BaseCombatWeapon_HideThink"
bool CWeaponSDKBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	MDLCACHE_CRITICAL_SECTION();

	// cancel any reload in progress.
	m_bInReload = false; 

	// kill any think functions
	SetThink(NULL);

	// Some weapon's don't have holster anims yet, so detect that
	float flSequenceDuration = 0;
	SendWeaponAnim( GetHolsterActivity() );
	{
		flSequenceDuration = SequenceDuration();
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner)
	{
		pOwner->SetNextAttack( GetCurrentTime() + flSequenceDuration );
	}

	// If we don't have a holster anim, hide immediately to avoid timing issues
	if ( !flSequenceDuration )
	{
		SetWeaponVisible( false );
	}
	else
	{
		// Hide the weapon when the holster animation's finished
		SetContextThink( &CBaseCombatWeapon::HideThink, GetCurrentTime() + flSequenceDuration, SDK_HIDEWEAPON_THINK_CONTEXT );
	}

	return true;
}

bool CWeaponSDKBase::HasWeaponIdleTimeElapsed( void )
{
	if ( GetCurrentTime() > m_flTimeWeaponIdle )
		return true;

	return false;
}

float CWeaponSDKBase::GetCurrentTime() const
{
	if (!GetOwner())
		return gpGlobals->curtime;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());

	if (!pOwner)
		return gpGlobals->curtime;

	return pOwner->GetCurrentTime();
}

#ifdef GAME_DLL
void CWeaponSDKBase::SetDieThink( bool bDie )
{
	if( bDie )
		SetContextThink( &CWeaponSDKBase::Die, gpGlobals->curtime + 45.0f, "DieContext" );
	else
		SetContextThink( NULL, gpGlobals->curtime, "DieContext" );
}
void CWeaponSDKBase::Die( void )
{
	UTIL_Remove( this );
}
#endif

#include "cbase.h"

#include "dab_viewmodel.h"
#include "sdk_gamerules.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( dab_viewmodel, CDABViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( DABViewModel, DT_DABViewModel )

BEGIN_NETWORK_TABLE( CDABViewModel, DT_DABViewModel )
END_NETWORK_TABLE()

float CDABViewModel::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	float flSlow = 1;

	CSDKPlayer* pOwner = ToSDKPlayer(GetOwner());
	if (pOwner)
		flSlow *= pOwner->GetSlowMoMultiplier();

	return BaseClass::GetSequenceCycleRate(pStudioHdr, iSequence) * m_flPlaybackRate * flSlow;
}

void CDABViewModel::DoMuzzleFlash()
{
#ifdef CLIENT_DLL
	switch (GetDAWeapon()->GetWeaponType())
	{
	case WT_PISTOL:
	default:
		ParticleProp()->Create( "muzzleflash_pistol", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_SMG:
		ParticleProp()->Create( "muzzleflash_smg", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_RIFLE:
		ParticleProp()->Create( "muzzleflash_rifle", PATTACH_POINT_FOLLOW, "1" );
		break;

	case WT_SHOTGUN:
		ParticleProp()->Create( "muzzleflash_shotgun", PATTACH_POINT_FOLLOW, "1" );
		break;
	}
#endif
}

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include "ihudlcd.h"
#include "c_sdk_player.h"
#include "sdkviewport.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, CHudNumericDisplay );

public:
	CHudAmmo( const char *pElementName );
	virtual void ApplySchemeSettings( IScheme *scheme );
	void Init( void );
	void VidInit( void );
	void Reset();

	void SetAmmo(int ammo, bool playAnimation);
	void SetAmmo2(int ammo2, bool playAnimation);

	virtual void Paint();
	virtual void PaintBackground() {};

protected:
	virtual void OnThink();

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo( C_BasePlayer *player );

	CHudTexture* GetTexture();
	Vector2D GetRoundPosition(int i);

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;
	int		m_iAmmo;
	int		m_iAmmo2;

	CHudTexture* m_p762Round;
	CHudTexture* m_p9mmRound;
	CHudTexture* m_p45acpRound;
	CHudTexture* m_pBuckshotRound;

	class CFlyingRound
	{
	public:
		bool bActive;
		Vector2D vecPosition;
		Vector2D vecVelocity;
		float flAngle;
		float flAngularVelocity;
	};

	CUtlVector<CFlyingRound> m_aRounds;
};

DECLARE_HUDELEMENT( CHudAmmo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo( const char *pElementName ) : BaseClass(NULL, "HudAmmo"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	hudlcd->SetGlobalStat( "(ammo_primary)", "0" );
	hudlcd->SetGlobalStat( "(ammo_secondary)", "0" );
	hudlcd->SetGlobalStat( "(weapon_print_name)", "" );
	hudlcd->SetGlobalStat( "(weapon_name)", "" );

	m_aRounds.EnsureCapacity(20);
}

void CHudAmmo::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	m_p762Round = gHUD.GetIcon("round_762");
	m_p9mmRound = gHUD.GetIcon("round_9mm");
	m_p45acpRound = gHUD.GetIcon("round_45acp");
	m_pBuckshotRound = gHUD.GetIcon("round_buckshot");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Init( void )
{
	m_iAmmo		= -1;
	m_iAmmo2	= -1;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO");
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"AMMO");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	BaseClass::Reset();

	m_hCurrentActiveWeapon = NULL;
	m_hCurrentVehicle = NULL;
	m_iAmmo = 0;
	m_iAmmo2 = 0;

	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::UpdatePlayerAmmo( C_BasePlayer *player )
{
	// Clear out the vehicle entity
	m_hCurrentVehicle = NULL;

	C_BaseCombatWeapon *wpn = GetActiveWeapon();

	hudlcd->SetGlobalStat( "(weapon_print_name)", wpn ? wpn->GetPrintName() : " " );
	hudlcd->SetGlobalStat( "(weapon_name)", wpn ? wpn->GetName() : " " );

	if ( !wpn || !player || !wpn->UsesPrimaryAmmo() )
	{
		hudlcd->SetGlobalStat( "(ammo_primary)", "n/a" );
		hudlcd->SetGlobalStat( "(ammo_secondary)", "n/a" );

		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}

	hudlcd->SetGlobalStat( "(ammo_primary)", VarArgs( "%d", ammo1 ) );
	hudlcd->SetGlobalStat( "(ammo_secondary)", VarArgs( "%d", ammo2 ) );

	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
		SetAmmo2(ammo2, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);
		SetAmmo2(ammo2, false);

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			SetShouldDisplaySecondaryValue(true);
		}
		else
		{
			SetShouldDisplaySecondaryValue(false);
		}

		m_hCurrentActiveWeapon = wpn;
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::OnThink()
{
	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the ammo display counts
//-----------------------------------------------------------------------------
void CHudAmmo::UpdateAmmoDisplays()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	
	UpdatePlayerAmmo( player );
}

//-----------------------------------------------------------------------------
// Purpose: Updates ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		CHudTexture* pTexture = GetTexture();
		if (playAnimation && pTexture)
		{
			for (int i = ammo; i < m_iAmmo; i++)
			{
				int iSpot = -1;

				// Find a spot to put it.
				for (int j = 0; j < m_aRounds.Count(); j++)
				{
					if (!m_aRounds[j].bActive)
					{
						iSpot = j;
						break;
					}
				}

				if (iSpot == -1)
					iSpot = m_aRounds.AddToTail();

				m_aRounds[iSpot].bActive = true;
				m_aRounds[iSpot].flAngle = 0;
				m_aRounds[iSpot].flAngularVelocity = RandomFloat(10, 1000);
				m_aRounds[iSpot].vecVelocity.x = RandomFloat(-100, -300);
				m_aRounds[iSpot].vecVelocity.y = RandomFloat(-300, -500);
				m_aRounds[iSpot].vecPosition = GetRoundPosition(ammo);
			}
		}

		m_iAmmo = ammo;
	}

	SetDisplayValue(ammo);
}

//-----------------------------------------------------------------------------
// Purpose: Updates 2nd ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo2(int ammo2, bool playAnimation)
{
	if (ammo2 != m_iAmmo2)
	{
		m_iAmmo2 = ammo2;
	}

	SetSecondaryValue(ammo2);
}

CHudTexture* CHudAmmo::GetTexture()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return nullptr;

	if (!pPlayer->IsAlive())
		return nullptr;

	if (!pPlayer->GetActiveSDKWeapon())
		return nullptr;

	if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "9x19mm"))
		return m_p9mmRound;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "762x51mm"))
		return m_p762Round;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "45acp"))
		return m_p45acpRound;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "buckshot"))
		return m_pBuckshotRound;

	return nullptr;
}

ConVar hud_ammoscale("hud_ammoscale", "1.4", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much to scale the ammo display.");

Vector2D CHudAmmo::GetRoundPosition(int i)
{
	CHudTexture* pTexture = GetTexture();
	if ( !pTexture )
		return Vector2D(0, 0);

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	float flScale = 480.0f/(float)iHeight * hud_ammoscale.GetFloat();

	float flRightPadding = 40;
	float flBottomPadding = 40;

	int iSpacing = 10*flScale;

	int iMaxClip = pPlayer->GetActiveSDKWeapon()->GetMaxClip1();
	if ((pTexture->EffectiveWidth(flScale) + iSpacing) * iMaxClip > 450*flScale)
		iSpacing = -10*flScale;

	if (iSpacing < 0)
	{
		if (i%2 == 0)
			return Vector2D(iWidth - (i+1)*pTexture->EffectiveWidth(flScale) - i*iSpacing - flRightPadding*flScale, iHeight - pTexture->EffectiveHeight(flScale) - (flBottomPadding+10)*flScale);
		else
			return Vector2D(iWidth - (i+1)*pTexture->EffectiveWidth(flScale) - i*iSpacing - flRightPadding*flScale, iHeight - pTexture->EffectiveHeight(flScale) - flBottomPadding*flScale);
	}
	else
		return Vector2D(iWidth - (i+1)*pTexture->EffectiveWidth(flScale) - i*iSpacing - flRightPadding*flScale, iHeight - pTexture->EffectiveHeight(flScale) - flBottomPadding*flScale);
}

void CHudAmmo::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	if (!pPlayer->GetActiveSDKWeapon())
		return;

	CHudTexture* pTexture = GetTexture();

	if (!pTexture)
		return;

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	float flScale = 480.0f/(float)iHeight * hud_ammoscale.GetFloat();

	for (int i = 0; i < m_iAmmo; i++)
	{
		Vector2D vecRound = GetRoundPosition(i);
		pTexture->DrawSelf( vecRound.x, vecRound.y, pTexture->EffectiveWidth(flScale), pTexture->EffectiveHeight(flScale), Color(255, 255, 255, 255) );
	}

	float flFrameTime = gpGlobals->frametime * pPlayer->GetSlowMoMultiplier();

	for (int i = 0; i < m_aRounds.Count(); i++)
	{
		auto& oRound = m_aRounds[i];

		if (!oRound.bActive)
			continue;

		if (oRound.vecPosition.y > 1000)
		{
			oRound.bActive = false;
			continue;
		}

		oRound.vecPosition += flFrameTime * oRound.vecVelocity;
		oRound.vecVelocity.y += flFrameTime * 2000;
		oRound.flAngle += flFrameTime * oRound.flAngularVelocity;

		SDKViewport::DrawPolygon(pTexture,
			oRound.vecPosition.x, oRound.vecPosition.y,
			pTexture->EffectiveWidth(flScale), pTexture->EffectiveHeight(flScale), oRound.flAngle);
	}
}

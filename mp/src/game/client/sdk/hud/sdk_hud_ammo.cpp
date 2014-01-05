//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include "sdk_hud_ammo.h"

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
#include "weapon_sdkbase.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

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
	m_pGrenadeIcon = gHUD.GetIcon("grenade_icon");
	m_pGrenadeEmptyIcon = gHUD.GetIcon("grenade_empty_icon");
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

	if ( !wpn || !player )
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
	// Only update if we've lost ammo somehow.
	if (ammo != m_iAmmo)
	{
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

void CHudAmmo::ShotFired(C_WeaponSDKBase* pWeapon)
{
	UpdateAmmoDisplays();

	m_iAmmo = pWeapon->Clip1();

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
	m_aRounds[iSpot].vecPosition = GetRoundPosition(m_iAmmo);
	m_aRounds[iSpot].pTexture = GetTexture();
}

void CHudAmmo::Reload(C_WeaponSDKBase* pWeapon)
{
	UpdateAmmoDisplays();

	m_iAmmo = pWeapon->Clip1();
}

CHudTexture* CHudAmmo::GetTexture()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return NULL;

	if (!pPlayer->IsAlive())
		return NULL;

	if (!pPlayer->GetActiveSDKWeapon())
		return NULL;

	if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "9x19mm"))
		return m_p9mmRound;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "762x51mm"))
		return m_p762Round;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "45acp"))
		return m_p45acpRound;
	else if (FStrEq(pPlayer->GetActiveSDKWeapon()->GetSDKWpnData().szAmmo1, "buckshot"))
		return m_pBuckshotRound;

	return NULL;
}

ConVar hud_ammoscale("hud_ammoscale", "1.4", FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How much to scale the ammo display.");

Vector2D CHudAmmo::GetRoundPosition(int i)
{
	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	float flScale = 480.0f/(float)iHeight * hud_ammoscale.GetFloat();

	float flRightPadding = scheme()->GetProportionalScaledValueEx(GetScheme(), 40);
	float flBottomPadding = scheme()->GetProportionalScaledValueEx(GetScheme(), 40);

	CHudTexture* pTexture = GetTexture();
	if ( !pTexture )
		return Vector2D(iWidth - flRightPadding*flScale, iHeight - flBottomPadding*flScale);

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	float flSpacing = scheme()->GetProportionalScaledValueEx(GetScheme(), 10*flScale);

	int iMaxClip = pPlayer->GetActiveSDKWeapon()->GetMaxClip1();

	float flTotalWidth = (GetTextureDrawWidth(pTexture, flScale) + flSpacing) * iMaxClip;
	float flTotalSpace = scheme()->GetProportionalScaledValueEx(GetScheme(), 420*flScale);
	if (flTotalWidth > flTotalSpace)
		flSpacing = -scheme()->GetProportionalScaledValueEx(GetScheme(), 5)*flScale;

	if (flSpacing < 0)
	{
		if (i%2 == 0)
			return Vector2D(iWidth - (i+1)*GetTextureDrawWidth(pTexture, flScale) - i*flSpacing - flRightPadding*flScale, iHeight - GetTextureDrawHeight(pTexture, flScale) - (flBottomPadding+10)*flScale);
		else
			return Vector2D(iWidth - (i+1)*GetTextureDrawWidth(pTexture, flScale) - i*flSpacing - flRightPadding*flScale, iHeight - GetTextureDrawHeight(pTexture, flScale) - flBottomPadding*flScale);
	}
	else
		return Vector2D(iWidth - (i+1)*GetTextureDrawWidth(pTexture, flScale) - i*flSpacing - flRightPadding*flScale, iHeight - GetTextureDrawHeight(pTexture, flScale) - flBottomPadding*flScale);
}

float CHudAmmo::GetTextureDrawWidth(CHudTexture* pTexture, float flScale)
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), pTexture->EffectiveWidth(1))*flScale * 0.8f;
}

float CHudAmmo::GetTextureDrawHeight(CHudTexture* pTexture, float flScale)
{
	return scheme()->GetProportionalScaledValueEx(GetScheme(), pTexture->EffectiveHeight(1))*flScale * 0.8f;
}

Vector4D CHudAmmo::GetGrenadePosition(int i)
{
	float flGrenadeSize = scheme()->GetProportionalScaledValueEx(GetScheme(), 32);
	float flGrenadeMargin = scheme()->GetProportionalScaledValueEx(GetScheme(), 40);

	Vector2D vecFirstRound = GetRoundPosition(0);

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	return Vector4D(iWidth - flGrenadeMargin - flGrenadeSize - i*flGrenadeSize, vecFirstRound.y - flGrenadeMargin, flGrenadeSize, flGrenadeSize);
}

extern float Oscillate(float flTime, float flLength);

void CHudAmmo::Paint()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		return;

	if (!pPlayer->GetActiveSDKWeapon())
		return;

	int iWidth, iHeight;
	GetSize(iWidth, iHeight);

	float flScale = 480.0f/(float)iHeight * hud_ammoscale.GetFloat();

	CWeaponSDKBase* pGrenade = pPlayer->FindWeapon(SDK_WEAPON_GRENADE);
	if (pGrenade)
	{
		if (m_pGrenadeIcon)
		{
			for (int i = 0; i < pPlayer->GetAmmoCount(pGrenade->GetPrimaryAmmoType()); i++)
			{
				Vector4D vecGrenade = GetGrenadePosition(i);
				m_pGrenadeIcon->DrawSelf(vecGrenade.x, vecGrenade.y, vecGrenade.z, vecGrenade.w, Color(255, 255, 255, 255));
			}
		}

		if (m_pGrenadeEmptyIcon)
		{
			int iTotalGrenades = GetAmmoDef()->MaxCarry(GetAmmoDef()->Index("grenades"));
			if (pPlayer->m_Shared.m_iStyleSkill != SKILL_TROLL)
				iTotalGrenades = ConVarRef("da_max_grenades").GetInt();

			for (int i = pPlayer->GetAmmoCount(pGrenade->GetPrimaryAmmoType()); i < iTotalGrenades; i++)
			{
				Vector4D vecGrenade = GetGrenadePosition(i);
				m_pGrenadeEmptyIcon->DrawSelf(vecGrenade.x, vecGrenade.y, vecGrenade.z, vecGrenade.w, Color(255, 255, 255, 255));
			}
		}
	}

	CHudTexture* pTexture = GetTexture();

	if (!pTexture)
		return;

	for (int i = 0; i < m_iAmmo; i++)
	{
		Vector2D vecRound = GetRoundPosition(i);
		pTexture->DrawSelf( vecRound.x, vecRound.y, GetTextureDrawWidth(pTexture, flScale), GetTextureDrawHeight(pTexture, flScale), Color(255, 255, 255, 255) );
	}

	float flFrameTime = gpGlobals->frametime * pPlayer->GetSlowMoMultiplier();

	for (int i = 0; i < m_aRounds.Count(); i++)
	{
		CFlyingRound& oRound = m_aRounds[i];

		if (!oRound.bActive)
			continue;

		if (oRound.vecPosition.y > iHeight+1000)
		{
			oRound.bActive = false;
			continue;
		}

		if (!oRound.pTexture)
		{
			oRound.bActive = false;
			continue;
		}

		oRound.vecPosition += flFrameTime * oRound.vecVelocity;
		oRound.vecVelocity.y += flFrameTime * 2000;
		oRound.flAngle += flFrameTime * oRound.flAngularVelocity;

		SDKViewport::DrawPolygon(oRound.pTexture,
			oRound.vecPosition.x, oRound.vecPosition.y,
			GetTextureDrawWidth(oRound.pTexture, flScale), GetTextureDrawHeight(oRound.pTexture, flScale), oRound.flAngle);
	}

	wchar_t* pszActivate = g_pVGuiLocalize->Find("#DA_HUD_Ammo_Reload");

	if (pszActivate && m_iAmmo == 0)
	{
#define WSTRLEN 512
		// replace any key references with bound keys
		wchar_t wszHintLabel[WSTRLEN];
		UTIL_ReplaceKeyBindings( pszActivate, 0, wszHintLabel, sizeof( wszHintLabel ) );

		int iTextWide, iTextTall;
		surface()->GetTextSize( m_hHintFont, wszHintLabel, iTextWide, iTextTall );

		int iWidth, iHeight;
		GetSize(iWidth, iHeight);

		float flRightPadding = scheme()->GetProportionalScaledValueEx(GetScheme(), 40);
		float flBottomPadding = scheme()->GetProportionalScaledValueEx(GetScheme(), 40);

		surface()->DrawSetTextPos( iWidth - iTextWide - flRightPadding, iHeight - flBottomPadding - iTextTall );
		surface()->DrawSetTextColor( Color(255, 0, 0, Oscillate(gpGlobals->curtime, 1)*255) );
		surface()->DrawSetTextFont( m_hHintFont );
		surface()->DrawUnicodeString( wszHintLabel, vgui::FONT_DRAW_NONADDITIVE );
	}}

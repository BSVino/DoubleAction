//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include "weapon_selection.h"
#include "iclientmode.h"
#include "history_resource.h"
#include "input.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>

#include "vgui/ILocalize.h"

#include "basemodelpanel.h"
#include "weapon_dabase.h"
#include "c_da_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DA_MAX_WEAPON_SLOTS 9

//-----------------------------------------------------------------------------
// Purpose: hl2 weapon selection hud element
//-----------------------------------------------------------------------------
class CHudWeaponSelection : public CBaseHudWeaponSelection, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudWeaponSelection, vgui::Panel );

public:
	CHudWeaponSelection(const char *pElementName );

	virtual bool ShouldDraw() { return true; }
	virtual bool RealShouldDraw();

	virtual void CycleToNextWeapon( void );
	virtual void CycleToPrevWeapon( void );

	virtual C_BaseCombatWeapon *GetWeaponInSlot( int iSlot, int iSlotPos );
	virtual void SelectWeaponSlot( int iSlot );

	virtual C_BaseCombatWeapon	*GetSelectedWeapon( void )
	{ 
		return m_hSelectedWeapon;
	}

	virtual void OpenSelection( void );
	virtual void HideSelection( void );

	virtual void OnWeaponPickup( C_BaseCombatWeapon *pWeapon );

	virtual void LevelInit();

protected:
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual bool IsWeaponSelectable()
	{ 
		if (IsInSelectionMode())
			return true;

		return false;
	}

private:
	void AddWeaponTypeToSlots(int& iCurrentSlot, weapontype_t eWT);

	void FastWeaponSwitch( int iWeaponSlot );

	virtual	void SetSelectedWeapon( C_BaseCombatWeapon *pWeapon ) 
	{ 
		m_hSelectedWeapon = pWeapon;
	}

	CPanelAnimationVar( vgui::HFont, m_hNumberFont, "NumberFont", "HudSelectionNumbers" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudSelectionText" );

	CPanelAnimationVarAliasType( float, m_flSmallBoxSize, "SmallBoxSize", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flLargeBoxWide, "LargeBoxWide", "108", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flRifleBoxWide, "RifleBoxWide", "200", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flLargeBoxTall, "LargeBoxTall", "72", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flBoxGap, "BoxGap", "12", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flSelectionNumberXPos, "SelectionNumberXPos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flSelectionNumberYPos, "SelectionNumberYPos", "4", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flTextYPos, "TextYPos", "54", "proportional_float" );

	CPanelAnimationVar( float, m_flAlphaOverride, "Alpha", "255" );
	CPanelAnimationVar( float, m_flSelectionAlphaOverride, "SelectionAlpha", "255" );


	CPanelAnimationVar( Color, m_TextColor, "TextColor", "SelectionTextFg" );
	CPanelAnimationVar( Color, m_NumberColor, "NumberColor", "SelectionNumberFg" );
	CPanelAnimationVar( Color, m_EmptyBoxColor, "EmptyBoxColor", "SelectionEmptyBoxBg" );
	CPanelAnimationVar( Color, m_BoxColor, "BoxColor", "SelectionBoxBg" );
	CPanelAnimationVar( Color, m_SelectedBoxColor, "SelectedBoxClor", "SelectionSelectedBoxBg" );

	CPanelAnimationVar( float, m_flWeaponPickupGrowTime, "SelectionGrowTime", "0.1" );

	CPanelAnimationVar( float, m_flTextScan, "TextScan", "1.0" );

	bool m_bFadingOut;

	float m_flDeployCurr;
	float m_flDeployGoal;

	CHandle<C_WeaponDABase> m_ahWeaponSlots[DA_MAX_WEAPON_SLOTS];

	float m_aflSelectedGoal[DA_MAX_WEAPON_SLOTS];
	float m_aflSelectedCurr[DA_MAX_WEAPON_SLOTS];
};

DECLARE_HUDELEMENT( CHudWeaponSelection );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudWeaponSelection::CHudWeaponSelection( const char *pElementName ) : CBaseHudWeaponSelection(pElementName), BaseClass(NULL, "HudWeaponSelection")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	m_bFadingOut = false;

	m_flDeployGoal = m_flDeployCurr = 0;

	SetHiddenBits( 0 );
}

#define SELECTION_TIMEOUT_THRESHOLD		5.0f	// Seconds
#define SELECTION_FADEOUT_TIME			1.5f

//-----------------------------------------------------------------------------
// Purpose: updates animation status
//-----------------------------------------------------------------------------
void CHudWeaponSelection::OnThink( void )
{
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	if (!pPlayer->IsAlive())
		m_flDeployGoal = m_flDeployCurr = 0;

	if (pPlayer && gpGlobals->curtime - pPlayer->GetLastSpawnTime() < 0.2f)
	{
		// Force it to recalculate weapons
		OpenSelection();
		HideSelection();
		m_flDeployGoal = 1;
		m_flDeployCurr = 0;
		m_flSelectionTime = gpGlobals->curtime;
	}

	// Time out after awhile of inactivity
	if ( ( gpGlobals->curtime - m_flSelectionTime ) > SELECTION_TIMEOUT_THRESHOLD )
	{
		if (!m_bFadingOut)
		{
			m_bFadingOut = true;
			m_flDeployGoal = 0;
		}
		else if (gpGlobals->curtime - m_flSelectionTime > SELECTION_TIMEOUT_THRESHOLD + SELECTION_FADEOUT_TIME)
		{
			// finished fade, close
			HideSelection();
		}
	}
	else if (m_bFadingOut)
	{
		m_bFadingOut = false;
	}

	if (m_flDeployCurr >= m_flDeployGoal)
	{
		for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
			m_aflSelectedCurr[i] = Approach(m_aflSelectedGoal[i], m_aflSelectedCurr[i], gpGlobals->frametime * 10);
	}

	m_flDeployCurr = Approach(m_flDeployGoal, m_flDeployCurr, gpGlobals->frametime * 7);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the panel should draw
//-----------------------------------------------------------------------------
bool CHudWeaponSelection::RealShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer || !pPlayer->IsAlive())
		return false;

	if (m_flDeployCurr > 0 || m_flDeployGoal > 0)
		return true;

	if ( !pPlayer )
	{
		if ( IsInSelectionMode() )
		{
			HideSelection();
		}
		return false;
	}

	bool bret = CBaseHudWeaponSelection::ShouldDraw();
	if ( !bret )
		return false;

	return ( m_bSelectionVisible ) ? true : false;
}

void CHudWeaponSelection::OnWeaponPickup( C_BaseCombatWeapon *pWeapon )
{
	// Force it to recalculate weapons
	OpenSelection();
	HideSelection();
	m_flDeployGoal = 1;
	m_flSelectionTime = gpGlobals->curtime;
}

void CHudWeaponSelection::LevelInit()
{
	CHudElement::LevelInit();

	m_flDeployCurr = m_flDeployGoal = 0;
}

//-------------------------------------------------------------------------
// Purpose: draws the selection area
//-------------------------------------------------------------------------
void CHudWeaponSelection::Paint()
{
	if (!RealShouldDraw())
		return;

	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	surface()->DrawSetTextFont( m_hTextFont );
	float flTextLeft = scheme()->GetProportionalScaledValueEx(GetScheme(), 250);

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
	{
		if (!m_ahWeaponSlots[i])
			continue;

		wchar_t* pwszPrintName = g_pVGuiLocalize->Find(m_ahWeaponSlots[i]->GetSDKWpnData().szPrintName);
		if (!pwszPrintName)
			pwszPrintName = L"Missing Print Name";

		Vector clrText;
		if (m_ahWeaponSlots[i] == pPlayer->GetActiveDAWeapon())
			clrText = Vector(1, 0.75f, 0.08f);
		else
			clrText = Vector(1, 1, 1);

		clrText = Lerp<Vector>(m_aflSelectedCurr[i], clrText, Vector(0.08f, 0.9f, 0.9f));
		surface()->DrawSetTextColor( Color(clrText.x * 255, clrText.y * 255, clrText.z * 255, m_flDeployCurr * 255) );

		float flInset = Lerp<float>(m_aflSelectedCurr[i], 0, -50);
		float flDeploy = Lerp<float>(m_flDeployCurr, -150, 0);

		wchar_t wszNumber[3];
		wszNumber[0] = L'1' + i;
		wszNumber[1] = L'.';
		wszNumber[2] = L'\0';

		surface()->DrawSetTextPos( i * 10 + flTextLeft + flInset, flDeploy + i * surface()->GetFontTall(m_hTextFont) );
		surface()->DrawPrintText(wszNumber, 2);

		surface()->DrawSetTextPos( i * 10 + flTextLeft + flInset + surface()->GetFontTall(m_hTextFont), flDeploy + i * surface()->GetFontTall(m_hTextFont) );
		surface()->DrawPrintText(pwszPrintName, wcslen(pwszPrintName));
	}
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudWeaponSelection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	// set our size
	int screenWide, screenTall;
	int x, y;
	GetPos(x, y);
	GetHudSize(screenWide, screenTall);
	SetBounds(0, y, screenWide, screenTall - y);
}

void CHudWeaponSelection::AddWeaponTypeToSlots(int& iCurrentSlot, weapontype_t eWT)
{
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (!pWeapon)
			continue;

		C_WeaponDABase* pSDKWeapon = dynamic_cast<CWeaponDABase*>(pWeapon);
		if (!pSDKWeapon)
			continue;

		if (pSDKWeapon->GetSDKWpnData().m_eWeaponType != eWT)
			continue;

		if (pSDKWeapon->GetSDKWpnData().m_szSingle[0])
			continue;

		m_ahWeaponSlots[iCurrentSlot] = pSDKWeapon;

		iCurrentSlot++;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens weapon selection control
//-----------------------------------------------------------------------------
void CHudWeaponSelection::OpenSelection( void )
{
	Assert(!IsInSelectionMode());

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_ahWeaponSlots[i] = NULL;

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_aflSelectedCurr[i] = m_aflSelectedGoal[i] = 0;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer )
	{
		int iCurrentSlot = 0;

		AddWeaponTypeToSlots(iCurrentSlot, WT_MELEE);
		AddWeaponTypeToSlots(iCurrentSlot, WT_PISTOL);
		AddWeaponTypeToSlots(iCurrentSlot, WT_SMG);
		AddWeaponTypeToSlots(iCurrentSlot, WT_SHOTGUN);
		AddWeaponTypeToSlots(iCurrentSlot, WT_RIFLE);

		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
			if (!pWeapon)
				continue;

			C_WeaponDABase* pSDKWeapon = dynamic_cast<CWeaponDABase*>(pWeapon);
			if (!pSDKWeapon)
				continue;

			if (!pSDKWeapon->GetSDKWpnData().m_szSingle[0])
				continue;

			m_ahWeaponSlots[iCurrentSlot] = pSDKWeapon;

			iCurrentSlot++;
		}
	}

	CBaseHudWeaponSelection::OpenSelection();
	m_flDeployGoal = 1;
}

//-----------------------------------------------------------------------------
// Purpose: Closes weapon selection control immediately
//-----------------------------------------------------------------------------
void CHudWeaponSelection::HideSelection( void )
{
	CBaseHudWeaponSelection::HideSelection();
	m_bFadingOut = false;

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_aflSelectedGoal[i] = 0;

	if (!hud_fastswitch.GetBool())
		m_flDeployGoal = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the next item in the menu
//-----------------------------------------------------------------------------
void CHudWeaponSelection::CycleToNextWeapon( void )
{
	// Get the local player.
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	C_WeaponDABase* pActiveWeapon = pPlayer->GetActiveDAWeapon();
	if (!pActiveWeapon)
	{
		for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
			m_aflSelectedGoal[i] = 0;

		m_aflSelectedGoal[0] = 1;

		SetSelectedWeapon( m_ahWeaponSlots[0] );
		return;
	}

	if ( !IsInSelectionMode() )
		OpenSelection();

	int iSlot = -1;
	C_BaseCombatWeapon* pSelectedWeapon = GetSelectedWeapon();

	if (hud_fastswitch.GetBool())
		pSelectedWeapon = pPlayer->GetActiveDAWeapon();

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
	{
		if (m_ahWeaponSlots[i] == pSelectedWeapon)
		{
			iSlot = i;
			break;
		}
	}

	if (iSlot < 0)
		return;

	while (!m_ahWeaponSlots[(++iSlot) % DA_MAX_WEAPON_SLOTS]);

	iSlot %= DA_MAX_WEAPON_SLOTS;

	SetSelectedWeapon( m_ahWeaponSlots[iSlot] );

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_aflSelectedGoal[i] = 0;

	m_aflSelectedGoal[iSlot] = 1;

	m_flDeployGoal = 1;

	// Play the "cycle to next weapon" sound
	pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the previous item in the menu
//-----------------------------------------------------------------------------
void CHudWeaponSelection::CycleToPrevWeapon( void )
{
	// Get the local player.
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	C_WeaponDABase* pActiveWeapon = pPlayer->GetActiveDAWeapon();
	if (!pActiveWeapon)
	{
		for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
			m_aflSelectedGoal[i] = 0;

		m_aflSelectedGoal[0] = 1;

		SetSelectedWeapon( m_ahWeaponSlots[0] );
		return;
	}

	if ( !IsInSelectionMode() )
		OpenSelection();

	int iSlot = -1;
	C_BaseCombatWeapon* pSelectedWeapon = GetSelectedWeapon();

	if (hud_fastswitch.GetBool())
		pSelectedWeapon = pPlayer->GetActiveDAWeapon();

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
	{
		if (m_ahWeaponSlots[i] == pSelectedWeapon)
		{
			iSlot = i;
			break;
		}
	}

	if (iSlot < 0)
		return;

	// This because modulus doesn't handle negatives like you would think.
	iSlot += DA_MAX_WEAPON_SLOTS;

	while (!m_ahWeaponSlots[(--iSlot) % DA_MAX_WEAPON_SLOTS]);

	iSlot %= DA_MAX_WEAPON_SLOTS;

	SetSelectedWeapon( m_ahWeaponSlots[iSlot] );

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_aflSelectedGoal[i] = 0;

	m_aflSelectedGoal[iSlot] = 1;

	m_flDeployGoal = 1;

	// Play the "cycle to next weapon" sound
	pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
}

//-----------------------------------------------------------------------------
// Purpose: returns the weapon in the specified slot
//-----------------------------------------------------------------------------
C_BaseCombatWeapon *CHudWeaponSelection::GetWeaponInSlot( int iSlot, int iSlotPos )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return NULL;

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *pWeapon = player->GetWeapon(i);

		if ( pWeapon == NULL )
			continue;

		if ( pWeapon->GetSlot() == iSlot && pWeapon->GetPosition() == iSlotPos )
			return pWeapon;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Opens the next weapon in the slot
//-----------------------------------------------------------------------------
void CHudWeaponSelection::FastWeaponSwitch( int iWeaponSlot )
{
	// get the slot the player's weapon is in
	C_DAPlayer *pPlayer = C_DAPlayer::GetLocalDAPlayer();
	if ( !pPlayer )
		return;

	if (iWeaponSlot < 0 || iWeaponSlot >= DA_MAX_WEAPON_SLOTS)
		return;

	if (!m_ahWeaponSlots[iWeaponSlot])
	{
		pPlayer->EmitSound( "Player.DenyWeaponSelection" );
		return;
	}

	if (pPlayer->GetActiveDAWeapon() == GetSelectedWeapon())
		return;

	m_flDeployGoal = 1;

	// select the new weapon
	::input->MakeWeaponSelection( m_ahWeaponSlots[iWeaponSlot] );
}

//-----------------------------------------------------------------------------
// Purpose: Moves selection to the specified slot
//-----------------------------------------------------------------------------
void CHudWeaponSelection::SelectWeaponSlot( int iSlot )
{
	// iSlot is one higher than it should be, since it's the number key, not the 0-based index into the weapons
	--iSlot;

	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// Don't try and read past our possible number of slots
	if ( iSlot > DA_MAX_WEAPON_SLOTS )
		return;

	// Make sure the player's allowed to switch weapons
	if ( pPlayer->IsAllowedToSwitchWeapons() == false )
		return;

	if ( !IsInSelectionMode() )
		// open the weapon selection
		OpenSelection();

	// do a fast switch if set
	if ( hud_fastswitch.GetBool() )
	{
		FastWeaponSwitch( iSlot );
		return;
	}

	C_BaseCombatWeapon *pActiveWeapon = m_ahWeaponSlots[iSlot];
	if (!pActiveWeapon)
		return;

	// Mark the change
	SetSelectedWeapon( pActiveWeapon );

	for (int i = 0; i < DA_MAX_WEAPON_SLOTS; i++)
		m_aflSelectedGoal[i] = 0;

	m_aflSelectedGoal[iSlot] = 1;

	m_flDeployGoal = 1;

	pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
}

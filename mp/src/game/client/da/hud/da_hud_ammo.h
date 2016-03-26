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

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, CHudNumericDisplay );

public:
	CHudAmmo( const char *pElementName );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	void Init( void );
	void VidInit( void );
	void Reset();

	void SetAmmo(int ammo, bool playAnimation);
	void SetAmmo2(int ammo2, bool playAnimation);

	void ShotFired(class C_WeaponDABase* pWeapon, bool bAkimbo, bool bRight);
	void Reload(class C_WeaponDABase* pWeapon);

	virtual void Paint();
	virtual void PaintBackground() {};

	float GetTextureDrawWidth(class CHudTexture* pTexture, float flScale=1);
	float GetTextureDrawHeight(class CHudTexture* pTexture, float flScale=1);

	Vector4D GetGrenadePosition(int i);

protected:
	virtual void OnThink();

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo( C_BasePlayer *player );

	CHudTexture* GetTexture();
	Vector2D GetRoundPosition(int i);

	CPanelAnimationVar( vgui::HFont, m_hHintFont, "HintFont", "Default" );

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;
	int		m_iAmmo;
	int		m_iAmmo2;

	CHudTexture* m_p762Round;
	CHudTexture* m_p9mmRound;
	CHudTexture* m_p45acpRound;
	CHudTexture* m_pBuckshotRound;
	CHudTexture* m_pGrenadeIcon;
	CHudTexture* m_pGrenadeEmptyIcon;

	class CFlyingRound
	{
	public:
		bool         bActive;
		Vector2D     vecPosition;
		Vector2D     vecVelocity;
		float        flAngle;
		float        flAngularVelocity;
		CHudTexture* pTexture;
	};

	CUtlVector<CFlyingRound> m_aRounds;
};


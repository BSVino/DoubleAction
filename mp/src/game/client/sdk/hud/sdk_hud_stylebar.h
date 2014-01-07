#pragma once

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

#include <vgui_controls/Panel.h>

#include "da.h"

class CHudStyleBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudStyleBar, vgui::Panel );

public:
	CHudStyleBar( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void OnThink();

	virtual void Paint();
	virtual void PaintBackground() {};

	void	MsgFunc_StyleAnnouncement( bf_read &msg );
	void    Notice(notice_t eNotice);

	float GetIconX();
	float GetIconY();
	float GetIconW();
	float GetIconH();

private:
	float   m_flCurrentStyle;
	int		m_flGoalStyle;

	float   m_flStyleIconLerpStart;

	CPanelAnimationVarAliasType( float, m_flElementXPos, "barxpos", "r330", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flElementYPos, "barypos", "60", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flElementTall, "bartall", "220", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flElementWide, "barwide", "300", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flGap, "Gap", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "10", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hStyleFont, "StyleFont", "Default" );

	CHudTexture* m_apAnnouncements[TOTAL_ANNOUNCEMENTS];
	CHudTexture* m_apActiveSkillIcons[SKILL_MAX];
	CHudTexture* m_pGoldStar;
	CHudTexture* m_pSilverStar;
	CHudTexture* m_pBronzeStar;
	CHudTexture* m_pBriefcase;

	class CAnnouncement
	{
	public:
		float          m_flStartTime;
		announcement_t m_eAnnouncement;
		style_point_t  m_ePointStyle;
		float          m_flBarPosition;
		float          m_flStylePoints;
	};
	CUtlLinkedList<CAnnouncement> m_aAnnouncements;
};

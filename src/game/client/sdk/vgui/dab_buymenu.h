//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_BUYMENU_H
#define SDK_BUYMENU_H

#include <classmenu.h>
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "iconpanel.h"
#include <vgui_controls/CheckButton.h>

class CDABWeaponInfoPanel : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( CDABWeaponInfoPanel, vgui::EditablePanel );

public:
	CDABWeaponInfoPanel( vgui::Panel *parent, const char *panelName ) : vgui::EditablePanel( parent, panelName )
	{
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual vgui::Panel *CreateControlByName( const char *controlName );
};

class CWeaponButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( CWeaponButton, vgui::Button );
	
public:
	CWeaponButton(vgui::Panel *parent, const char *panelName, CDABWeaponInfoPanel *pPanel ) :
					Button( parent, panelName, "WeaponButton")
	{
		m_pPanel = new CDABWeaponInfoPanel( parent, NULL );
		m_pPanel->SetVisible( false );

		// copy size&pos from template panel
		int x,y,wide,tall;
		pPanel->GetBounds( x, y, wide, tall );
		m_pPanel->SetBounds( x, y, wide, tall );
		int px, py;
		pPanel->GetPinOffset( px, py );
		int rx, ry;
		pPanel->GetResizeOffset( rx, ry );
		// Apply pin settings from template, too
		m_pPanel->SetAutoResize( pPanel->GetPinCorner(), pPanel->GetAutoResize(), px, py, rx, ry );

		m_bPreserveArmedButtons = false;
		m_bUpdateDefaultButtons = false;
	}

	virtual void SetPreserveArmedButtons( bool bPreserve ){ m_bPreserveArmedButtons = bPreserve; }
	virtual void SetUpdateDefaultButtons( bool bUpdate ){ m_bUpdateDefaultButtons = bUpdate; }

	virtual void ShowPage()
	{
		if( m_pPanel )
		{
			m_pPanel->SetVisible( true );
			m_pPanel->MoveToFront();
			g_lastPanel = m_pPanel;
		}
	}
	
	virtual void HidePage()
	{
		if ( m_pPanel )
		{
			m_pPanel->SetVisible( false );
		}
	}

	const char *GetWeaponPage( const char *className )
	{
		static char classPanel[ _MAX_PATH ];
		Q_snprintf( classPanel, sizeof( classPanel ), "resource/weapons/%s.res", className);

		if ( g_pFullFileSystem->FileExists( classPanel, IsX360() ? "MOD" : "GAME" ) )
		{
		}
		else if (g_pFullFileSystem->FileExists( "resource/weapons/weapon_default.res", IsX360() ? "MOD" : "GAME" ) )
		{
			Q_snprintf ( classPanel, sizeof( classPanel ), "resource/weapons/weapon_default.res" );
		}
		else
		{
			return NULL;
		}

		return classPanel;
	}

	void RefreshClassPage( void )
	{
		m_pPanel->LoadControlSettings( GetWeaponPage( GetName() ) );
	}

	virtual void ApplySettings( KeyValues *resourceData ) 
	{
		BaseClass::ApplySettings( resourceData );

		// name, position etc of button is set, now load matching
		// resource file for associated info panel:
		m_pPanel->LoadControlSettings( GetWeaponPage( GetName() ) );
	}		

	CDABWeaponInfoPanel *GetInfoPanel( void ) { return m_pPanel; }

	virtual void OnCursorExited()
	{
		if ( !m_bPreserveArmedButtons )
		{
			BaseClass::OnCursorExited();
		}
	}

	virtual void OnCursorEntered() 
	{
		BaseClass::OnCursorEntered();

		if ( !IsEnabled() )
			return;

		// are we updating the default buttons?
		if ( m_bUpdateDefaultButtons )
		{
			SetAsDefaultButton( 1 );
		}

		// are we preserving the armed state (and need to turn off the old button)?
		if ( m_bPreserveArmedButtons )
		{
			if ( g_lastButton && g_lastButton != this )
			{
				g_lastButton->SetArmed( false );
			}

			g_lastButton = this;
		}

		// turn on our panel (if it isn't already)
		if ( m_pPanel && ( !m_pPanel->IsVisible() ) )
		{
			// turn off the previous panel
			if ( g_lastPanel && g_lastPanel->IsVisible() )
			{
				g_lastPanel->SetVisible( false );
			}

			ShowPage();
		}
	}

	virtual void OnKeyCodeReleased( vgui::KeyCode code )
	{
		BaseClass::OnKeyCodeReleased( code );

		if ( m_bPreserveArmedButtons )
		{
			if ( g_lastButton )
			{
				g_lastButton->SetArmed( true );
			}
		}
	}

private:

	CDABWeaponInfoPanel *m_pPanel;
	bool m_bPreserveArmedButtons;
	bool m_bUpdateDefaultButtons;
};

class CDABBuyMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CDABBuyMenu, vgui::Frame );

public:
	CDABBuyMenu(IViewPort *pViewPort);
	virtual ~CDABBuyMenu();

	virtual const char *GetName( void ) { return PANEL_BUY; }

	virtual void Reset();
	virtual void Update( void );
	void MoveToCenterOfScreen();
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnTick( void );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void SetVisible( bool state );
	virtual void ShowPanel(bool bShow);
	void OnCommand( const char *command );

	virtual void SetData(KeyValues *data) {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	MESSAGE_FUNC_CHARPTR( OnShowPage, "ShowPage", page );

	MESSAGE_FUNC_PTR( OnSuicideOptionChanged, "CheckButtonChecked", panel );

private:
	IViewPort	*m_pViewPort;
	vgui::EditablePanel *m_pPanel;

	CDABWeaponInfoPanel *m_pWeaponInfoPanel;
	CWeaponButton *m_pInitialButton;
	CheckButton *m_pSuicideOption;

	ButtonCode_t m_iBuyMenuKey;

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	// rounded corners
	Color					 m_bgColor;
	Color					 m_borderColor;
};

#endif //SDK_BUYMENU_H
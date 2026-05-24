//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "advanced_options.h"
#include <vgui/ISurface.h>
#include "filesystem.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar cl_showadvancedoptpanel( "cl_showadvancedoptpanel", "0", FCVAR_CLIENTDLL, "Sets the state of AdvancedOptPanel <state>" );

CAdvancedOptPanel::CAdvancedOptPanel( vgui::VPANEL parent ) : BaseClass( NULL, "AdvancedOptPanel" )
{
	SetParent( parent );

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

	SetProportional( true );
	SetTitleBarVisible( true );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( true );
	SetSizeable( false );
	SetMoveable( false );
	SetVisible( true );

	SetScheme( vgui::scheme()->LoadSchemeFromFile( "resource/SourceScheme.res", "SourceScheme" ) );

	LoadControlSettings( "resource/ui/advancedoptions.res" );

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );

	int width = screenWide * 0.6f;
	int height = screenTall * 0.7f;

	SetSize( width, height );

	int x = ( screenWide - width ) / 2;
	int y = ( screenTall - height ) / 2;

	SetPos( x, y );

	m_pTabSheet = new PropertySheet( this, "AdvancedOptTabs" );
	m_pTabSheet->SetBounds( 0, 30, GetWide(), GetTall() );

	m_pMultiplayerPage = new CAdvancedOptionsMultiplayer( m_pTabSheet, "MultiplayerPage" );
	m_pTabSheet->AddPage( m_pMultiplayerPage, "Multiplayer" );

	m_pGameplayPage = new CAdvancedOptionsGameplay( m_pTabSheet, "GamePlayPage" );
	m_pTabSheet->AddPage( m_pGameplayPage, "Gameplay" );

	m_pVisualsPage = new CAdvancedOptionsVisuals( m_pTabSheet, "VisualsPage" );
	m_pTabSheet->AddPage( m_pVisualsPage, "Visuals" );

	m_pCloseButton = new Button( this, "CloseButton", "Close" );
	m_pCloseButton->SetCommand( "CloseAdvancedPanel" );

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

void CAdvancedOptPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = GetWide();
	int h = GetTall();

	int titleHeight = static_cast< int >( h * 0.06f );
	int smallMargin = MAX( static_cast< int >( w * 0.01f ), 6 );

	m_pTabSheet->SetBounds( 0, titleHeight, w, h - titleHeight );

	int btnWidth = MAX( static_cast< int >( w * 0.12f ), 70 );
	int btnHeight = MAX( static_cast< int >( h * 0.06f ), 24 );

	m_pCloseButton->SetBounds( w - btnWidth - smallMargin, h - btnHeight - smallMargin, btnWidth, btnHeight );

	m_pCloseButton->SetZPos( 1 );
	m_pCloseButton->MoveToFront();
}

void CAdvancedOptPanel::OnCommand( const char *pcCommand )
{
	if ( FStrEq( pcCommand, "CloseAdvancedPanel" ) )
	{
		OnClose();
		return;
	}

	BaseClass::OnCommand( pcCommand );
}

void CAdvancedOptPanel::OnClose()
{
	cl_showadvancedoptpanel.SetValue( 0 );
	SetVisible( false );
}

void CAdvancedOptPanel::OnTick()
{
	BaseClass::OnTick();
	SetVisible( cl_showadvancedoptpanel.GetBool() );
}

class CAdvancedOptPanelInterface : public AdvancedOptPanel
{
public:
	CAdvancedOptPanel *advPanel;

	CAdvancedOptPanelInterface()
	{
		advPanel = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		advPanel = new CAdvancedOptPanel( parent );
	}

	void Destroy()
	{
		if ( advPanel )
		{
			advPanel->SetParent( (vgui::Panel *)NULL );
			delete advPanel;
		}
	}

	void Activate( void )
	{
		if ( advPanel )
		{
			advPanel->Activate();
		}
	}
};

static CAdvancedOptPanelInterface g_AdvancedOptPanel;
AdvancedOptPanel				 *advancedoptpanel = (AdvancedOptPanel *)&g_AdvancedOptPanel;

CON_COMMAND( OpenAdvancedOptions, "Toggles advanced options on or off" )
{
	cl_showadvancedoptpanel.SetValue( !cl_showadvancedoptpanel.GetBool() );
	g_AdvancedOptPanel.advPanel->InvalidateLayout( true, false );
	advancedoptpanel->Activate();
};

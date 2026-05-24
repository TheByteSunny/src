//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "advanced_visuals_tab.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CAdvancedOptionsVisuals::CAdvancedOptionsVisuals( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pFPSCheckbox = new vgui::CheckButton( this, "FPSCheckbox", "Show FPS" );
	m_pFPSCheckbox->AddActionSignalTarget( this );
	m_pFPSCheckbox->SetCommand( "FPSCheckboxToggled" );

	m_pNetGraphCheckbox = new vgui::CheckButton( this, "NetGraphCheckbox", "Show Net Graph" );
	m_pNetGraphCheckbox->AddActionSignalTarget( this );
	m_pNetGraphCheckbox->SetCommand( "NetGraphCheckboxToggled" );

	m_pCrosshairCheckbox = new vgui::CheckButton( this, "CrosshairCheckbox", "Show Crosshair" );
	m_pCrosshairCheckbox->AddActionSignalTarget( this );
	m_pCrosshairCheckbox->SetCommand( "CrosshairCheckboxToggled" );
}

void CAdvancedOptionsVisuals::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = GetWide();
	int h = GetTall();

	int margin = static_cast< int >( w * 0.05f );
	int controlHeight = static_cast< int >( h * 0.08f );
	int spacing = static_cast< int >( h * 0.04f );
	int controlWidth = w - 2 * margin;

	int y = margin;

	m_pFPSCheckbox->SetBounds( margin, y, controlWidth, controlHeight );
	y += controlHeight + spacing;

	m_pNetGraphCheckbox->SetBounds( margin, y, controlWidth, controlHeight );
	y += controlHeight + spacing;

	m_pCrosshairCheckbox->SetBounds( margin, y, controlWidth, controlHeight );
}

void CAdvancedOptionsVisuals::OnTick()
{
	BaseClass::OnTick();
}

void CAdvancedOptionsVisuals::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "FPSCheckboxToggled" ) )
	{
		int value = m_pFPSCheckbox->IsSelected() ? 1 : 0;
		engine->ClientCmd_Unrestricted( CFmtStr( "cl_showfps %d", value ) );
	}
	else if ( !Q_stricmp( command, "NetGraphCheckboxToggled" ) )
	{
		int value = m_pNetGraphCheckbox->IsSelected() ? 1 : 0;
		engine->ClientCmd_Unrestricted( CFmtStr( "net_graph %d", value ) );
	}
	else if ( !Q_stricmp( command, "CrosshairCheckboxToggled" ) )
	{
		int value = m_pCrosshairCheckbox->IsSelected() ? 1 : 0;
		engine->ClientCmd_Unrestricted( CFmtStr( "crosshair %d", value ) );
	}

	BaseClass::OnCommand( command );
}

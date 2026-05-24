//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "advanced_gameplay_tab.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CAdvancedOptionsGameplay::CAdvancedOptionsGameplay( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	SetProportional( true );
}

void CAdvancedOptionsGameplay::PerformLayout()
{
	BaseClass::PerformLayout();

	int marginX = static_cast< int >( GetWide() * 0.05f );
	int marginY = static_cast< int >( GetTall() * 0.05f );

	// TODO
}

void CAdvancedOptionsGameplay::OnCommand( const char *command )
{
}
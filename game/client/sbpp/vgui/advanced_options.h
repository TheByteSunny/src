//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef ADVANCED_OPTIONS_H
#define ADVANCED_OPTIONS_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <matsys_controls/mdlpanel.h>
#include <vgui_controls/ComboBox.h>
#include <matsys_controls/colorpickerpanel.h>
#include <vgui/ISurface.h>
#include <vector>
#include <string>

#include "advanced_multiplayer_tab.h"
#include "advanced_gameplay_tab.h"
#include "advanced_visuals_tab.h"

class CAdvancedOptPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CAdvancedOptPanel, vgui::Frame );

	CAdvancedOptPanel( vgui::VPANEL parent );
	~CAdvancedOptPanel(){};

protected:
	virtual void OnTick();
	virtual void OnCommand( const char *pcCommand );

	virtual void PerformLayout();

	virtual void OnClose();

public:
	vgui::PropertySheet			*m_pTabSheet;
	CAdvancedOptionsMultiplayer *m_pMultiplayerPage;
	CAdvancedOptionsGameplay	*m_pGameplayPage;
	CAdvancedOptionsVisuals		*m_pVisualsPage;

	vgui::Button *m_pCloseButton;
};

class AdvancedOptPanel
{
public:
	virtual void Create( vgui::VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void Activate( void ) = 0;
};

extern AdvancedOptPanel *advancedoptpanel;

#endif // ADVANCED_OPTIONS_H
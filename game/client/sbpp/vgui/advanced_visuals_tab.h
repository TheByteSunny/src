//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef ADVANCED_V_TAB_H
#define ADVANCED_V_TAB_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <matsys_controls/mdlpanel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <matsys_controls/colorpickerpanel.h>
#include <vgui/ISurface.h>
#include <vector>
#include <string>

class CAdvancedOptionsVisuals : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CAdvancedOptionsVisuals, vgui::PropertyPage );

public:
	CAdvancedOptionsVisuals( vgui::Panel *parent, const char *panelName );

protected:
	virtual void OnCommand( const char *command );
	virtual void PerformLayout();

	virtual void OnTick();

private:
	vgui::CheckButton *m_pFPSCheckbox;
	vgui::CheckButton *m_pNetGraphCheckbox;
	vgui::CheckButton *m_pCrosshairCheckbox;
};

#endif
//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef MENUBAR_H
#define MENUBAR_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>

class CMenuButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CMenuButton, vgui::Button );

public:
	CMenuButton( vgui::Panel *parent, const char *panelName, const char *text ) : vgui::Button( parent, panelName, text )
	{
		m_iIconTexture = -1;
		AddActionSignalTarget( parent );
	}

	void SetIcon( const char *file );

	virtual void PaintBackground() OVERRIDE;

private:
	int m_iIconTexture;
};

class CMenuBar : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CMenuBar, vgui::Panel );

public:
	CMenuBar( vgui::Panel *parent );

	virtual void Paint();
	virtual void PerformLayout();

	MESSAGE_FUNC( OnGamemodeButtonPressed, "GamemodeButtonPressed" );

private:
	CMenuButton *m_pGamemodeButton;
};

#endif

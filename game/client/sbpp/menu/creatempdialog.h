//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef CREATEMPDIALOG_H
#define CREATEMPDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include "imageextbutton.h"

using namespace vgui;

class MapListPanel : public vgui::PanelListPanel
{
	DECLARE_CLASS_SIMPLE( MapListPanel, vgui::PanelListPanel );

public:
	MapListPanel( vgui::Panel *parent, const char *pName );
	virtual void OnTick( void );
	virtual void PerformLayout();
	virtual void AddButton( MapListPanel *panel, const char *image, const char *command, const char *mapName );
	virtual void LoadMaps( MapListPanel *panel );
	virtual void OnCommand( const char *command );

	virtual int ComputeVPixelsNeeded() override;

private:
	CUtlVector< vgui::Panel * > layoutItems;

	ImageExtButton *m_pSelectedButton;
	bool			m_bMapsLoaded;
};

class GameMapsPanel : public MapListPanel
{
	DECLARE_CLASS_SIMPLE( GameMapsPanel, MapListPanel );

public:
	GameMapsPanel( vgui::Panel *parent, const char *pName );
};

class ServerSettingsPanel : public vgui::PanelListPanel
{
	DECLARE_CLASS_SIMPLE( ServerSettingsPanel, vgui::PanelListPanel );

public:
	ServerSettingsPanel( vgui::Panel *parent, const char *pName );
	virtual void OnTick( void );
	virtual void PerformLayout();

	virtual void OnCommand( const char *command );

	void LoadGamemodes();

	vgui::TextEntry *m_pMaxPlayers;
	vgui::TextEntry *m_pHostname;
	vgui::TextEntry *m_pPassword;

	vgui::ComboBox *m_pGamemodeCombo;
};

class MapList : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( MapList, vgui::PropertyDialog );

public:
	MapList( vgui::VPANEL *parent, const char *pName );
	void OnTick();

	virtual bool OnOK( bool applyOnly );
	virtual void OnClose();
	virtual void OnCancel();
};

class CMapList
{
public:
	virtual void Create( vgui::VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void Activate( void ) = 0;
};

extern CMapList *maplist;

#endif
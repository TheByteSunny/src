//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef SPAWNMENU_H
#define SPAWNMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/MenuButton.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/ScrollBar.h>
#include "filesystem.h"
#include <map>
#include <string>
#include <unordered_map>

using namespace vgui;

class SMLPanel
{
public:
	virtual void Create( vgui::VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void Activate( void ) = 0;
};

extern SMLPanel *smlmenu;

class CSMLButton : public MenuButton
{
public:
	CSMLButton( Panel *parent, const char *panelName, const char *text );
	~CSMLButton() override;

private:
	Menu *m_pMenu;
};

class CSMLCommandButton : public Button
{
public:
	CSMLCommandButton( Panel *parent, const char *panelName, const char *labelText, const char *command );
	void OnCommand( const char *command ) override;
	~CSMLCommandButton() override;
};

class CSMLPage : public PropertyPage
{
	DECLARE_CLASS_SIMPLE( CSMLPage, PropertyPage );

public:
	CSMLPage( Panel *parent, const char *panelName );
	~CSMLPage() override;
	void		OnTick() override;
	void		PerformLayout() override;
	void		Init( KeyValues *kv );
	static void CreateButtonX( CSMLPage *page, const char *label, const char *command );

	virtual void OnCommand( const char *command ) override;
	virtual void OnMouseWheeled( int delta ) override;

	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );

private:
	bool				  m_bInScrollUpdate;
	CUtlVector< Panel * > m_LayoutItems;
	Panel				 *m_pContentPanel;
	ScrollBar			 *m_pScrollBar;
};

class CSMLMenu : public PropertyDialog
{
public:
	CSMLMenu( vgui::VPANEL *parent, const char *panelName );
	~CSMLMenu() override;
	static void CreateButton( const char *pageName, const char *label, const char *command );
	CSMLPage   *FindOrCreatePage( const char *pageName );
	void		OnTick() override;

	virtual void OnClose() override;

public:
	static CSMLMenu								 *s_Instance;
	std::unordered_map< std::string, CSMLPage * > m_PageRegistry;
};

class CSMLPanelInterface : public SMLPanel
{
public:
	CSMLPanelInterface();
	~CSMLPanelInterface();
	void Create( vgui::VPANEL parent );
	void Destroy();
	void Activate();

private:
	CSMLMenu *SMLPanel;
};

void CreateButton( const char *pageName, const char *label, const char *command );

#endif
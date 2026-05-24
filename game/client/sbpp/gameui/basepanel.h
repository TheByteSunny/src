//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef BASEPANEL_H
#define BASEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "menubar.h"
#include "menu/imageextbutton.h"

namespace vgui
{
class Panel;
class IScheme;
class Label;
}; // namespace vgui

// I'm too lazy to include the file here
class CAutoGameSystem;

class CHoverButton : public vgui::Label
{
	DECLARE_CLASS_SIMPLE( CHoverButton, vgui::Label );

public:
	CHoverButton( vgui::Panel *parent, const char *panelName );

	virtual void OnCursorEntered() OVERRIDE;
	virtual void OnCursorExited() OVERRIDE;
	virtual void OnMousePressed( vgui::MouseCode code ) OVERRIDE;
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;

	void SetCommand( const char *cmd );

private:
	Color		m_DefaultColor;
	Color		m_HoverColor;
	vgui::HFont m_DefaultFont;
	vgui::HFont m_BoldFont;
	CUtlString	m_Command;
};

class CBackgroundPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CBackgroundPanel, vgui::Panel );

public:
	CBackgroundPanel( vgui::Panel *parent, const char *pName );
	virtual ~CBackgroundPanel();

	virtual void Paint() OVERRIDE;
	virtual void PerformLayout() OVERRIDE;

	void LoadBackgroundImages();

private:
	CUtlVector< int >		 m_BackgroundTextureIDs;
	CUtlVector< CUtlString > m_BackgroundFiles;
	int						 m_iCurrentBackground = 0;
	float					 m_flNextBackgroundSwitch = 0.f;
	float					 m_flFadeDuration = 2.f;
	float					 m_flZoomAmount = 0.15f;
	float					 m_flRotationAmount = 5.f;
	int						 m_maxLoadedBackgrounds = 2;

	void		   DestroyBackgroundTexture( int index );
	void		   EnsureBackgroundTextureLoaded( int index );
	int			   LoadImageAsTexture( const char *imagePath );
	unsigned char *LoadImageFromMemory( unsigned char *data, int dataSize, int &width, int &height, int &channels );
	int			   GetNextPowerOfTwo( int value );
	unsigned char *ResizeImage( unsigned char *src, int srcW, int srcH, int dstW, int dstH );
};

class CMainMenu : public vgui::Panel
{
	DECLARE_CLASS( CMainMenu, vgui::Panel );

public:
	CMainMenu( vgui::VPANEL parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
	virtual void PerformLayout() OVERRIDE;
	void		 LoadGameMenu();

	virtual void OnScreenSizeChanged( int iOldWide, int iOldTall ) OVERRIDE;

private:
	CMenuBar					*m_pMenuBar;
	CHoverButton				*m_pStartButton;
	ImageExtButton				*m_pLogo;
	CBackgroundPanel			*m_pBackground;
	CUtlVector< CHoverButton * > m_GameMenuButtons;
};

class CMainMenuSystem : public CAutoGameSystem
{
	DECLARE_CLASS( CMainMenuSystem, CAutoGameSystem );

public:
	CMainMenuSystem();

	virtual void PostInit() OVERRIDE;

	virtual void LevelInitPostEntity() OVERRIDE;
	virtual void LevelShutdownPostEntity() OVERRIDE;
};

#endif

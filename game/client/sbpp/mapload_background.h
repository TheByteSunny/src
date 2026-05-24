//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef MAPLOAD_BACKGROUND_H
#define MAPLOAD_BACKGROUND_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include "filesystem.h"
#include "menu/imageextbutton.h"

class PngImagePanel : public ImageExtButton
{
public:
	PngImagePanel( vgui::Panel *parent, const char *panelName, const char *normalImage ) : ImageExtButton( parent, panelName, normalImage )
	{
		m_bIgnoreInput = true;
	}

protected:
	virtual void OnCursorEntered() OVERRIDE
	{
	}
	virtual void OnCursorExited() OVERRIDE
	{
	}
	virtual void OnMousePressed( vgui::MouseCode code ) OVERRIDE
	{
	}
	virtual void OnMouseReleased( vgui::MouseCode code ) OVERRIDE
	{
	}

private:
	bool m_bIgnoreInput;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapLoadBG, vgui::EditablePanel );

public:
	CMapLoadBG( char const *panelName );
	~CMapLoadBG();

	void setServerName( const char *serverName )
	{
		m_pServerName->SetText( serverName );
	}

	void setGameModeName( const char *gameModeName )
	{
		m_pGameMode->SetText( gameModeName );
	}

	void setMapName( const char *mapName )
	{
		m_pMapName->SetText( mapName );

		char imageName[MAX_PATH] = "";

		const char *extensions[] = { ".png", ".jpg", ".jpeg", ".tga", ".vtf" };
		bool		foundExactMatch = false;

		for ( int i = 0; i < 5; i++ )
		{
			char testPath[MAX_PATH];
			Q_snprintf( testPath, sizeof( testPath ), "maps/thumb/%s%s", mapName, extensions[i] );

			if ( g_pFullFileSystem->FileExists( testPath, "MOD" ) )
			{
				Q_strncpy( imageName, testPath, sizeof( imageName ) );
				foundExactMatch = true;
				break;
			}
		}

		if ( !foundExactMatch )
		{
			FileFindHandle_t findhandle;
			char			 searchPattern[MAX_PATH];
			Q_snprintf( searchPattern, sizeof( searchPattern ), "maps/thumb/%s.*", mapName );

			const char *foundFile = g_pFullFileSystem->FindFirstEx( searchPattern, "MOD", &findhandle );
			if ( foundFile && *foundFile )
			{
				Q_snprintf( imageName, sizeof( imageName ), "maps/thumb/%s", foundFile );
				g_pFullFileSystem->FindClose( findhandle );
			}
			else
			{
				Q_strncpy( imageName, "materials/gui/noicon.png", sizeof( imageName ) );
			}
		}

		m_pMapIcon->DeletePanel();
		m_pMapIcon = new PngImagePanel( this, "PNGPanel", imageName );

		m_pMapIcon->SetBounds( 10, 10, 110, 110 );
	}

	virtual void OnThink();

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::ImagePanel *m_pBackground;
	PngImagePanel	 *m_pMapIcon;
	vgui::ImagePanel *m_pGradient;
	vgui::ImagePanel *m_pGameLogo;
	vgui::Label		 *m_pMapName;
	vgui::Label		 *m_pGameMode;
	vgui::Label		 *m_pServerName;

	int m_iLogoBaseWide;
	int m_iLogoBaseTall;
	int m_iLogoBaseX;
	int m_iLogoBaseY;
};

#endif // !MAPLOAD_BACKGROUND_H
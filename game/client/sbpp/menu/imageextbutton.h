//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef IMAGEEXTBUTTON_H
#define IMAGEEXTBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>
#include "filesystem.h"

#include <string>
#include <unordered_map>
#include <vector>

struct ImageData
{
	int	 textureId;
	int	 width;
	int	 height;
	bool isValid;

	ImageData() : textureId( -1 ), width( 0 ), height( 0 ), isValid( false )
	{
	}
};

struct TexInfo
{
	int texId;
	int width;
	int height;
	int refCount;

	TexInfo() : texId( -1 ), width( 0 ), height( 0 ), refCount( 0 )
	{
	}
	TexInfo( int t, int w, int h, int r = 1 ) : texId( t ), width( w ), height( h ), refCount( r )
	{
	}
};

class ImageExtButton : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( ImageExtButton, vgui::Panel );

public:
	const char *GetCommand() const
	{
		return m_command;
	}

	unsigned char *ResizeImage( unsigned char *originalData, int originalWidth, int originalHeight, int &newWidth, int &newHeight );

private:
	ImageData m_normalImage;
	ImageData m_mouseOverImage;
	ImageData m_mouseClickImage;

	char m_normalImagePath[MAX_PATH];
	char m_mouseOverImagePath[MAX_PATH];
	char m_mouseClickImagePath[MAX_PATH];

	bool m_hasMouseOverImage;
	bool m_hasMouseClickImage;
	bool m_hasCommand;
	bool m_bScaleImage;

	char m_command[MAX_PATH];

	ImageData *m_currentImage;

	vgui::Panel *m_pParent;

	bool LoadImage( const char *filename, ImageData &imageData );
	void SetCurrentImage( ImageData *image );
	int	 CreateOrGetTextureFromImageData( const char *key, unsigned char *data, int width, int height );
	void ReleaseTextureByKey( const char *key );

	bool LoadImageIfNeeded( ImageData &imageData, const char *path, bool &flag );

	int NearestPowerOfTwo( int value );

public:
	ImageExtButton( vgui::Panel *parent, const char *panelName, const char *normalImage, const char *mouseOverImage = NULL, const char *mouseClickImage = NULL, const char *pCmd = NULL );

	virtual ~ImageExtButton();

	virtual void Paint() OVERRIDE;
	virtual void OnCursorEntered() OVERRIDE;
	virtual void OnCursorExited() OVERRIDE;
	virtual void OnMousePressed( vgui::MouseCode code ) OVERRIDE;
	virtual void OnMouseReleased( vgui::MouseCode code ) OVERRIDE;
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;

	void SetNormalImage();
	void SetMouseOverImage();
	void SetMouseClickImage();

	void SetImage( const char *normalImagePath );

	void SetScaleImage( bool b )
	{
		m_bScaleImage = b;
	}

	ImageData *GetCurrentImage() const
	{
		return m_currentImage;
	}
	bool IsValid() const
	{
		return m_normalImage.isValid;
	}

	MESSAGE_FUNC_PARAMS( OnCommand, "Command", data );

private:
	static std::unordered_map< std::string, TexInfo > s_textureCache;
};

#endif // IMAGEEXTBUTTON_H

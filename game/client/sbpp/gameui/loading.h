//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LOADING_H
#define LOADING_H
#ifdef _WIN32
#pragma once
#endif

class ITexture;
class IMaterial;

#include <vgui/ISurface.h>

class CLoadingScreen
{
public:
	CLoadingScreen();
	~CLoadingScreen();

	void Initialize();
	void Shutdown();
	void UpdateState( const char *pszMessage, float progress );
	void Draw();

private:
	void CreateMaterials();
	void DestroyMaterials();

	float m_progress;
	char  m_message[2048];

	ITexture  *m_pLogoTexture;
	IMaterial *m_pBarMaterial;
	IMaterial *m_pLogoMaterial;

	bool m_bInitialized;

	vgui::HFont m_hFont;

	void RenderText( const char *text, int x, int y, int r, int g, int b, int a );

	ITexture *LoadPNGTexture( const char *pszFileName );
};

#endif
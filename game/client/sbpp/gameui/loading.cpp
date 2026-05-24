//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui/VGUI.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>

#include "loading.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterialvar.h"
#include "tier1/KeyValues.h"
#include "tier3/tier3.h"
#include "bitmap/imageformat.h"
#include "vtf/vtf.h"
#include "filesystem.h"
#include "tier2/renderutils.h"
#include "materialsystem/imesh.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include "stb_image.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: not much
//-----------------------------------------------------------------------------
class CLoadingScreenTextureRegenerator : public ITextureRegenerator
{
public:
	CLoadingScreenTextureRegenerator( unsigned char *pData, int width, int height ) : m_pImageData( pData ), m_nWidth( width ), m_nHeight( height )
	{
	}

	virtual ~CLoadingScreenTextureRegenerator()
	{
		if ( m_pImageData )
		{
			stbi_image_free( m_pImageData );
			m_pImageData = NULL;
		}
	}

	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
	{
		for ( int iFrame = 0; iFrame < pVTFTexture->FrameCount(); ++iFrame )
		{
			for ( int iFace = 0; iFace < pVTFTexture->FaceCount(); ++iFace )
			{
				int nWidth = pVTFTexture->Width();
				int nHeight = pVTFTexture->Height();
				int nDepth = pVTFTexture->Depth();
				for ( int z = 0; z < nDepth; ++z )
				{
					for ( int iMip = 0; iMip < pVTFTexture->MipCount(); ++iMip )
					{
						unsigned char *pData = pVTFTexture->ImageData( iFrame, iFace, iMip, 0, 0, z );
						int			   nMipWidth = pVTFTexture->Width() >> iMip;
						int			   nMipHeight = pVTFTexture->Height() >> iMip;

						if ( iMip == 0 && m_pImageData )
						{
							for ( int yy = 0; yy < nMipHeight; ++yy )
							{
								int			   srcY = yy;
								unsigned char *pSrcRow = m_pImageData + ( srcY * m_nWidth ) * 4;
								unsigned char *pDstRow = pData + ( yy * nMipWidth ) * 4;

								for ( int xx = 0; xx < nMipWidth; ++xx )
								{
									int			  srcX = xx;
									unsigned char r = pSrcRow[srcX * 4 + 0];
									unsigned char g = pSrcRow[srcX * 4 + 1];
									unsigned char b = pSrcRow[srcX * 4 + 2];
									unsigned char a = pSrcRow[srcX * 4 + 3];

									unsigned int  alpha = (unsigned int)a;
									unsigned char pr = (unsigned char)( ( r * alpha + 127 ) / 255 );
									unsigned char pg = (unsigned char)( ( g * alpha + 127 ) / 255 );
									unsigned char pb = (unsigned char)( ( b * alpha + 127 ) / 255 );

									pDstRow[xx * 4 + 0] = pb;
									pDstRow[xx * 4 + 1] = pg;
									pDstRow[xx * 4 + 2] = pr;
									pDstRow[xx * 4 + 3] = a;
								}
							}
						}
						else
						{
							V_memset( pData, 0, nMipWidth * nMipHeight * 4 );
						}
					}
				}
			}
		}
	}

	virtual void Release()
	{
		delete this;
	}

private:
	unsigned char *m_pImageData;
	int			   m_nWidth;
	int			   m_nHeight;
};

CLoadingScreen::CLoadingScreen()
{
	m_progress = 0.0f;
	m_message[0] = '\0';
	m_pLogoTexture = NULL;
	m_pBarMaterial = NULL;
	m_pLogoMaterial = NULL;
	m_bInitialized = false;
}

CLoadingScreen::~CLoadingScreen()
{
	Shutdown();
}

void CLoadingScreen::Initialize()
{
	if ( m_bInitialized )
		return;

	CreateMaterials();

	m_hFont = g_pMatSystemSurface->CreateFont();
	g_pMatSystemSurface->SetFontGlyphSet( m_hFont, "Arial", 18, 400, 0, 0, vgui::ISurface::FONTFLAG_ANTIALIAS );

	const wchar_t *pPrecache = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,:;!?-_%/\\";
	g_pMatSystemSurface->PrecacheFontCharacters( m_hFont, pPrecache );

	m_bInitialized = true;
}

void CLoadingScreen::Shutdown()
{
	if ( !m_bInitialized )
		return;

	DestroyMaterials();
	m_bInitialized = false;
}

void CLoadingScreen::CreateMaterials()
{
	m_pLogoTexture = LoadPNGTexture( "materials/introscreen/main.png" );

	KeyValues *pVMTKeyValues = new KeyValues( "UnlitGeneric" );
	pVMTKeyValues->SetInt( "$translucent", 1 );
	pVMTKeyValues->SetInt( "$ignorez", 1 );
	pVMTKeyValues->SetInt( "$nofog", 1 );
	pVMTKeyValues->SetInt( "$no_fullbright", 1 );
	pVMTKeyValues->SetInt( "$nocull", 1 );
	pVMTKeyValues->SetString( "$basetexture", "vgui/white" );
	m_pBarMaterial = g_pMaterialSystem->CreateMaterial( "__loading_bar", pVMTKeyValues );
	m_pBarMaterial->ColorModulate( 0.0f, 0.8f, 0.0f );
	m_pBarMaterial->AlphaModulate( 1.0f );

	if ( m_pLogoTexture )
	{
		KeyValues *pVMTKeyValuesLogo = new KeyValues( "UnlitGeneric" );
		pVMTKeyValuesLogo->SetInt( "$translucent", 1 );
		pVMTKeyValuesLogo->SetInt( "$ignorez", 1 );
		pVMTKeyValuesLogo->SetInt( "$nofog", 1 );
		pVMTKeyValuesLogo->SetInt( "$no_fullbright", 1 );
		pVMTKeyValuesLogo->SetInt( "$nocull", 1 );
		pVMTKeyValuesLogo->SetString( "$basetexture", "introscreen/main" );

		m_pLogoMaterial = g_pMaterialSystem->CreateMaterial( "__loading_logo", pVMTKeyValuesLogo );

		if ( m_pLogoMaterial )
		{
			bool		  bFound = false;
			IMaterialVar *pBaseTexVar = m_pLogoMaterial->FindVar( "$basetexture", &bFound, false );
			if ( pBaseTexVar && bFound )
			{
				pBaseTexVar->SetTextureValue( m_pLogoTexture );
				m_pLogoMaterial->Refresh();
			}
		}
	}
}

void CLoadingScreen::DestroyMaterials()
{
	if ( m_pBarMaterial )
	{
		m_pBarMaterial->Release();
		m_pBarMaterial = NULL;
	}

	if ( m_pLogoMaterial )
	{
		m_pLogoMaterial->Release();
		m_pLogoMaterial = NULL;
	}

	if ( m_pLogoTexture )
	{
		m_pLogoTexture->DecrementReferenceCount();
		m_pLogoTexture = NULL;
	}
}

void CLoadingScreen::RenderText( const char *text, int x, int y, int r, int g, int b, int a )
{
	if ( m_hFont == vgui::INVALID_FONT || !vgui::surface() )
		return;

	vgui::VPANEL root = vgui::surface()->GetEmbeddedPanel();
	if ( !root )
		return;

	wchar_t wtext[1024];
	g_pVGuiLocalize->ConvertANSIToUnicode( text, wtext, sizeof( wtext ) );

	vgui::surface()->DrawSetTextFont( m_hFont );
	vgui::surface()->DrawSetTextColor( r, g, b, a );
	vgui::surface()->DrawSetTextPos( x, y );
	vgui::surface()->DrawPrintText( wtext, wcslen( wtext ) );
	vgui::surface()->DrawFlushText();
}

void CLoadingScreen::UpdateState( const char *pszMessage, float progress )
{
	Q_strncpy( m_message, pszMessage, sizeof( m_message ) );
	m_progress = clamp( progress, 0.0f, 1.0f );

	Draw();
}

void CLoadingScreen::Draw()
{
	if ( !m_bInitialized || !m_pBarMaterial )
		return;

	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );

	int w, h;
	g_pMaterialSystem->GetBackBufferDimensions( w, h );

	pRenderContext->Viewport( 0, 0, w, h );
	pRenderContext->DepthRange( 0, 1 );

	pRenderContext->ClearColor4ub( 75, 138, 196, 255 );
	pRenderContext->ClearBuffers( true, true, true );

	pRenderContext->OverrideDepthEnable( true, false );

	float depth = 0.5f;
	int	  logoW = 0, logoH = 0;
	int	  logoX = 0, logoY = 0;

	// draw logo
	if ( m_pLogoTexture && m_pLogoMaterial )
	{
		logoW = m_pLogoTexture->GetActualWidth();
		logoH = m_pLogoTexture->GetActualHeight();

		if ( logoW > 0 && logoH > 0 )
		{
			logoX = ( w - logoW ) / 2;
			logoY = ( h - logoH ) / 2;
			DrawScreenSpaceRectangle( m_pLogoMaterial, logoX, logoY, logoW, logoH, 0, 0, logoW - 1, logoH - 1, logoW, logoH, NULL, 1, 1, depth );
		}
	}

	int barX, barY, barW, barH;
	if ( logoW > 0 && logoH > 0 )
	{
		barX = logoX + 18;
		barY = logoY + 489;
		barW = 476;
		barH = 14;
	}
	else
	{
		barW = 400;
		barH = 24;
		barX = ( w - barW ) / 2;
		barY = ( h / 2 ) + 100;
	}

	int fillW = int( barW * m_progress );
	if ( fillW > 0 )
	{
		DrawScreenSpaceRectangle( m_pBarMaterial, barX, barY, fillW, barH, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1, nullptr, 1, 1, 0.0f );
	}

	RenderText( m_message, w / 2 - 100, barY + barH + 20, 255, 255, 255, 255 );

	pRenderContext->OverrideDepthEnable( false, false );

	g_pMaterialSystem->SwapBuffers();
}

ITexture *CLoadingScreen::LoadPNGTexture( const char *pszFileName )
{
	FileHandle_t file = g_pFullFileSystem->Open( pszFileName, "rb", "GAME" );
	if ( file == FILESYSTEM_INVALID_HANDLE )
	{
		Warning( "LoadPNGTexture: Failed to open texture file: %s\n", pszFileName );
		return NULL;
	}

	int fileSize = g_pFullFileSystem->Size( file );
	if ( fileSize <= 0 )
	{
		g_pFullFileSystem->Close( file );
		Warning( "LoadPNGTexture: Invalid file size for: %s\n", pszFileName );
		return NULL;
	}

	unsigned char *fileBuffer = new unsigned char[fileSize];
	g_pFullFileSystem->Read( fileBuffer, fileSize, file );
	g_pFullFileSystem->Close( file );

	int			   width = 0, height = 0, channels = 0;
	unsigned char *imageData = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, 4 ); // force RGBA
	delete[] fileBuffer;

	if ( !imageData )
	{
		Warning( "LoadPNGTexture: Failed to decode PNG: %s\n", pszFileName );
		return NULL;
	}

	CLoadingScreenTextureRegenerator *pRegen = new CLoadingScreenTextureRegenerator( imageData, width, height );

	ITexture *pTexture =
		g_pMaterialSystem->CreateProceduralTexture( pszFileName, TEXTURE_GROUP_OTHER, width, height, IMAGE_FORMAT_RGBA8888, TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT );

	if ( !pTexture )
	{
		pRegen->Release();
		Warning( "LoadPNGTexture: Failed to create texture: %s\n", pszFileName );
		return NULL;
	}

	pTexture->SetTextureRegenerator( pRegen );

	pTexture->Download();

	int actualW = pTexture->GetActualWidth();
	int actualH = pTexture->GetActualHeight();
	if ( actualW == 0 || actualH == 0 )
	{
		pRegen->Release();
		Warning( "LoadPNGTexture: texture upload produced zero size for %s\n", pszFileName );
		return NULL;
	}

	return pTexture;
}

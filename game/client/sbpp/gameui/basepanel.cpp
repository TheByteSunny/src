//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <ienginevgui.h>
#include <vgui_controls/Panel.h>
#include <GameUI/IGameUI.h>
#include <vgui_controls/Label.h>
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "stb_image.h"

#include "basepanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static CMainMenuSystem g_MySystem;
static CMainMenu	  *g_pMainMenu = nullptr;

static const int MAX_BG_TEXTURE_SIZE = 2048;

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule
//  over and over again.
static CDllDemandLoader g_GameUIDLL( "GameUI" );

CHoverButton::CHoverButton( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName, "" )
{
	SetMouseInputEnabled( true );

	m_DefaultColor = Color( 200, 200, 200, 255 );
	m_HoverColor = Color( 255, 255, 0, 255 );

	m_DefaultFont = 0;
	m_BoldFont = 0;
}

void CHoverButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// default crap
	m_DefaultFont = pScheme->GetFont( "MainMenuFont", IsProportional() );
	m_BoldFont = pScheme->GetFont( "MainMenuFontBold", IsProportional() );

	SetFgColor( m_DefaultColor );
	SetFont( m_DefaultFont );
}

void CHoverButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	surface()->PlaySound( "ui/buttonrollover.wav" );

	SetCursor( vgui::dc_hand );

	SetFgColor( m_HoverColor );
	SetFont( m_BoldFont );
	SizeToContents();
	Repaint();
}

void CHoverButton::OnCursorExited()
{
	BaseClass::OnCursorExited();

	SetCursor( vgui::dc_arrow );

	SetFgColor( m_DefaultColor );
	SetFont( m_DefaultFont );
	SizeToContents();
	Repaint();
}

void CHoverButton::OnMousePressed( vgui::MouseCode code )
{
	BaseClass::OnMousePressed( code );

	surface()->PlaySound( "ui/buttonclickrelease.wav" );

	if ( !m_Command.IsEmpty() )
		engine->ExecuteClientCmd( m_Command.String() );
}

void CHoverButton::SetCommand( const char *cmd )
{
	m_Command = cmd;
}

CBackgroundPanel::CBackgroundPanel( vgui::Panel *parent, const char *pName ) : BaseClass( parent, pName )
{
	SetPaintBackgroundEnabled( false );
	SetMouseInputEnabled( false );
	m_BackgroundTextureIDs.RemoveAll();
	m_BackgroundFiles.RemoveAll();
}

CBackgroundPanel::~CBackgroundPanel()
{
	for ( int i = 0; i < m_BackgroundTextureIDs.Count(); ++i )
		DestroyBackgroundTexture( i );
}

void CBackgroundPanel::PerformLayout()
{
	vgui::Panel *pParent = GetParent();
	if ( pParent )
	{
		int w, h;
		pParent->GetSize( w, h );
		SetBounds( 0, 0, w, h );
	}
	BaseClass::PerformLayout();
}

void CBackgroundPanel::DestroyBackgroundTexture( int index )
{
	if ( index < 0 || index >= m_BackgroundTextureIDs.Count() )
		return;

	int texID = m_BackgroundTextureIDs[index];
	if ( texID >= 0 )
	{
		surface()->DestroyTextureID( texID );
		m_BackgroundTextureIDs[index] = -1;
	}
}

void CBackgroundPanel::LoadBackgroundImages()
{
	m_BackgroundFiles.RemoveAll();
	m_BackgroundTextureIDs.RemoveAll();

	FileFindHandle_t findHandle;
	const char		*pFilename = g_pFullFileSystem->FindFirstEx( "backgrounds/*.*", "MOD", &findHandle );

	while ( pFilename )
	{
		if ( !g_pFullFileSystem->FindIsDirectory( findHandle ) )
		{
			const char *ext = Q_GetFileExtension( pFilename );
			if ( ext )
			{
				char fullPath[MAX_PATH];
				Q_snprintf( fullPath, sizeof( fullPath ), "backgrounds/%s", pFilename );

				m_BackgroundFiles.AddToTail( CUtlString( fullPath ) );
				m_BackgroundTextureIDs.AddToTail( -1 );
			}
		}
		pFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );

	if ( m_BackgroundFiles.Count() > 1 )
	{
		for ( int i = m_BackgroundFiles.Count() - 1; i > 0; --i )
		{
			int j = RandomInt( 0, i );

			CUtlString tempFile = m_BackgroundFiles[i];
			m_BackgroundFiles[i] = m_BackgroundFiles[j];
			m_BackgroundFiles[j] = tempFile;

			int tempID = m_BackgroundTextureIDs[i];
			m_BackgroundTextureIDs[i] = m_BackgroundTextureIDs[j];
			m_BackgroundTextureIDs[j] = tempID;
		}
	}

	m_iCurrentBackground = 0;
	m_flNextBackgroundSwitch = engine->Time() + 10.0f;

	if ( m_BackgroundFiles.Count() > 0 )
		EnsureBackgroundTextureLoaded( m_iCurrentBackground );
	if ( m_BackgroundFiles.Count() > 1 )
		EnsureBackgroundTextureLoaded( ( m_iCurrentBackground + 1 ) % m_BackgroundFiles.Count() );
}

void CBackgroundPanel::EnsureBackgroundTextureLoaded( int index )
{
	if ( index < 0 || index >= m_BackgroundFiles.Count() )
		return;

	if ( index < m_BackgroundTextureIDs.Count() && m_BackgroundTextureIDs[index] >= 0 )
		return;

	const char *fullPath = m_BackgroundFiles[index].String();
	int			texID = LoadImageAsTexture( fullPath );
	if ( texID != -1 )
	{
		if ( index >= m_BackgroundTextureIDs.Count() )
		{
			// resize and init to -1
			while ( m_BackgroundTextureIDs.Count() <= index )
				m_BackgroundTextureIDs.AddToTail( -1 );
		}
		m_BackgroundTextureIDs[index] = texID;
	}

	int keepRadius = m_maxLoadedBackgrounds / 2;
	int keepStart = max( 0, index - keepRadius );
	int keepEnd = min( m_BackgroundFiles.Count() - 1, index + keepRadius );

	for ( int i = 0; i < m_BackgroundTextureIDs.Count(); ++i )
	{
		if ( i < keepStart || i > keepEnd )
			DestroyBackgroundTexture( i );
	}
}

int CBackgroundPanel::GetNextPowerOfTwo( int value )
{
	int power = 1;
	while ( power < value )
		power *= 2;
	return power;
}

unsigned char *CBackgroundPanel::ResizeImage( unsigned char *src, int srcW, int srcH, int dstW, int dstH )
{
	if ( !src || srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0 )
		return nullptr;

	unsigned char *dst = (unsigned char *)MemAlloc_Alloc( dstW * dstH * 4, __FILE__, __LINE__ );
	if ( !dst )
		return nullptr;

	float xRatio = (float)srcW / (float)dstW;
	float yRatio = (float)srcH / (float)dstH;

	for ( int y = 0; y < dstH; y++ )
	{
		for ( int x = 0; x < dstW; x++ )
		{
			int srcX = (int)( x * xRatio );
			int srcY = (int)( y * yRatio );

			if ( srcX >= srcW )
				srcX = srcW - 1;
			if ( srcY >= srcH )
				srcY = srcH - 1;

			int srcIdx = ( srcY * srcW + srcX ) * 4;
			int dstIdx = ( y * dstW + x ) * 4;

			dst[dstIdx + 0] = src[srcIdx + 0]; // R
			dst[dstIdx + 1] = src[srcIdx + 1]; // G
			dst[dstIdx + 2] = src[srcIdx + 2]; // B
			dst[dstIdx + 3] = src[srcIdx + 3]; // A
		}
	}

	return dst;
}

int CBackgroundPanel::LoadImageAsTexture( const char *imagePath )
{
	if ( !imagePath )
		return -1;

	unsigned char *imageData = nullptr;
	int			   width = 0, height = 0, channels = 0;

	{
		CUtlBuffer buf;
		if ( !g_pFullFileSystem->ReadFile( imagePath, "MOD", buf ) )
		{
			Warning( "Could not open image file: %s\n", imagePath );
			return -1;
		}
		int			   fileSize = buf.TellPut();
		unsigned char *fileData = (unsigned char *)buf.Base();

		imageData = LoadImageFromMemory( fileData, fileSize, width, height, channels );
		if ( !imageData )
		{
			Warning( "Unsupported or corrupt image: %s\n", imagePath );
			return -1;
		}
	}

	if ( !imageData || width <= 0 || height <= 0 )
	{
		if ( imageData )
			MemAlloc_Free( imageData );
		Warning( "Failed to decode image: %s\n", imagePath );
		return -1;
	}

	if ( channels != 4 )
	{
		unsigned char *rgba = (unsigned char *)MemAlloc_Alloc( width * height * 4, __FILE__, __LINE__ );
		if ( !rgba )
		{
			MemAlloc_Free( imageData );
			Warning( "Out of memory converting to RGBA: %s\n", imagePath );
			return -1;
		}

		for ( int i = 0; i < width * height; ++i )
		{
			int srcIdx = i * channels;
			int dstIdx = i * 4;
			rgba[dstIdx + 0] = imageData[srcIdx + 0];
			rgba[dstIdx + 1] = imageData[srcIdx + 1];
			rgba[dstIdx + 2] = imageData[srcIdx + 2];
			rgba[dstIdx + 3] = ( channels == 3 ) ? 255 : imageData[srcIdx + 3];
		}

		MemAlloc_Free( imageData );
		imageData = rgba;
		channels = 4;
	}

	int newWidth = GetNextPowerOfTwo( width );
	int newHeight = GetNextPowerOfTwo( height );

	if ( newWidth > MAX_BG_TEXTURE_SIZE )
		newWidth = MAX_BG_TEXTURE_SIZE;
	if ( newHeight > MAX_BG_TEXTURE_SIZE )
		newHeight = MAX_BG_TEXTURE_SIZE;

	unsigned char *finalData = imageData;
	if ( newWidth != width || newHeight != height )
	{
		finalData = ResizeImage( imageData, width, height, newWidth, newHeight );
		MemAlloc_Free( imageData );
		imageData = nullptr;
		width = newWidth;
		height = newHeight;
		if ( !finalData )
		{
			Warning( "Failed to resize image: %s\n", imagePath );
			return -1;
		}
	}

	int texID = surface()->CreateNewTextureID( true ); // procedural
	surface()->DrawSetTextureRGBA( texID, finalData, width, height, 1, false );

	if ( finalData )
		MemAlloc_Free( finalData );

	return texID;
}

unsigned char *CBackgroundPanel::LoadImageFromMemory( unsigned char *data, int dataSize, int &width, int &height, int &channels )
{
	if ( !data || dataSize <= 0 )
	{
		width = height = channels = 0;
		return nullptr;
	}

	int			   reqChannels = 4;
	unsigned char *stbiData = stbi_load_from_memory( data, dataSize, &width, &height, &channels, reqChannels );
	if ( !stbiData )
	{
		Warning( "Failed to load image: %s\n", stbi_failure_reason() );
		width = height = channels = 0;
		return nullptr;
	}

	// stbi gave us RGBA (reqChannels==4)
	channels = 4;
	int			   total = width * height * channels;
	unsigned char *result = (unsigned char *)MemAlloc_Alloc( total, __FILE__, __LINE__ );
	if ( !result )
	{
		stbi_image_free( stbiData );
		Warning( "Out of memory while loading image\n" );
		width = height = channels = 0;
		return nullptr;
	}

	memcpy( result, stbiData, total );
	stbi_image_free( stbiData );

	return result;
}

void CBackgroundPanel::Paint()
{
	if ( m_BackgroundTextureIDs.Count() == 0 )
		return;

	if ( engine->IsInGame() )
		return;

	int wide, tall;
	GetSize( wide, tall );

	surface()->DrawSetColor( 0, 0, 0, 255 );
	surface()->DrawFilledRect( 0, 0, wide, tall );

	float frametime = engine->Time();

	const float switchInterval = 10.0f;
	int			count = m_BackgroundTextureIDs.Count();
	int			iNextBackground = ( m_iCurrentBackground + 1 ) % count;

	EnsureBackgroundTextureLoaded( m_iCurrentBackground );
	if ( count > 1 )
		EnsureBackgroundTextureLoaded( iNextBackground );

	if ( m_flNextBackgroundSwitch <= 0.0f )
		m_flNextBackgroundSwitch = frametime + switchInterval;

	float switchTime = m_flNextBackgroundSwitch;
	float fadeDuration = ( m_flFadeDuration > 0.001f ) ? m_flFadeDuration : 4.0f;
	float fadeStart = switchTime - fadeDuration;

	while ( frametime >= m_flNextBackgroundSwitch )
	{
		m_iCurrentBackground = ( m_iCurrentBackground + 1 ) % count;
		m_flNextBackgroundSwitch += switchInterval;
	}

	float fade = 0.0f;
	if ( frametime >= fadeStart && fadeDuration > 0.0f )
	{
		float t = ( frametime - fadeStart ) / fadeDuration;
		t = clamp( t, 0.0f, 1.0f );
		fade = t * t * ( 3.0f - 2.0f * t );
	}

	float lastSwitchTime = m_flNextBackgroundSwitch - switchInterval;
	float growEnd = fadeStart;
	float growDur = growEnd - lastSwitchTime;
	if ( growDur < 0.001f )
		growDur = 0.001f;

	float zoom = 1.0f;
	float peakZoom = 1.0f + m_flZoomAmount;
	if ( frametime < fadeStart )
	{
		float pg = ( frametime - lastSwitchTime ) / growDur;
		pg = clamp( pg, 0.0f, 1.0f );
		float pg_eased = pg * pg * pg * ( pg * ( pg * 6 - 15 ) + 10 );
		zoom = 1.0f + ( ( peakZoom - 1.0f ) * pg_eased );
	}
	else
	{
		float tf = ( frametime - fadeStart ) / fadeDuration;
		tf = clamp( tf, 0.0f, 1.0f );
		zoom = peakZoom * ( 1.0f - tf ) + 1.0f * tf;
	}

	float rotation = 0.0f;
	float peakRotation = m_flRotationAmount;

	if ( frametime < fadeStart )
	{
		float pg = ( frametime - lastSwitchTime ) / growDur;
		pg = clamp( pg, 0.0f, 1.0f );
		float pg_eased = pg * pg * pg * ( pg * ( pg * 6 - 15 ) + 10 );
		rotation = peakRotation * pg_eased;
	}
	else
	{
		float tf = ( frametime - fadeStart ) / fadeDuration;
		tf = clamp( tf, 0.0f, 1.0f );
		rotation = peakRotation * ( 1.0f - tf );
	}

	int wZoomed = (int)( wide * zoom );
	int hZoomed = (int)( tall * zoom );

	float rotRad = rotation * ( M_PI / 180.0f );
	float cosR = cos( rotRad );
	float sinR = sin( rotRad );

	float centerX = wide / 2.0f;
	float centerY = tall / 2.0f;

	float halfW = wZoomed / 2.0f;
	float halfH = hZoomed / 2.0f;

	struct Vert
	{
		float x, y, u, v;
	};
	Vert  verts[4];
	float corners[4][2] = { { -halfW, -halfH }, { halfW, -halfH }, { halfW, halfH }, { -halfW, halfH } };

	for ( int i = 0; i < 4; ++i )
	{
		float x = corners[i][0];
		float y = corners[i][1];
		float rotX = x * cosR - y * sinR;
		float rotY = x * sinR + y * cosR;
		verts[i].x = centerX + rotX;
		verts[i].y = centerY + rotY;
		verts[i].u = ( i == 1 || i == 2 ) ? 1.0f : 0.0f;
		verts[i].v = ( i == 2 || i == 3 ) ? 1.0f : 0.0f;
	}

	// draw current
	int texIDCurr = m_BackgroundTextureIDs[m_iCurrentBackground];
	if ( texIDCurr >= 0 )
	{
		surface()->DrawSetTexture( texIDCurr );
		int alphaCurr = (int)( 255.0f * ( 1.0f - fade ) );
		surface()->DrawSetColor( 128, 128, 128, alphaCurr );

		vgui::Vertex_t v[4];
		for ( int i = 0; i < 4; ++i )
		{
			v[i].m_Position.x = verts[i].x;
			v[i].m_Position.y = verts[i].y;
			v[i].m_TexCoord.x = verts[i].u;
			v[i].m_TexCoord.y = verts[i].v;
		}
		surface()->DrawTexturedPolygon( 4, v );
	}

	if ( fade > 0.0f && count > 1 )
	{
		int texIDNext = m_BackgroundTextureIDs[iNextBackground];
		if ( texIDNext >= 0 )
		{
			surface()->DrawSetTexture( texIDNext );
			int alphaNext = (int)( 255.0f * fade );
			surface()->DrawSetColor( 128, 128, 128, alphaNext );
			vgui::Vertex_t v2[4];
			for ( int i = 0; i < 4; ++i )
			{
				v2[i].m_Position.x = verts[i].x;
				v2[i].m_Position.y = verts[i].y;
				v2[i].m_TexCoord.x = verts[i].u;
				v2[i].m_TexCoord.y = verts[i].v;
			}
			surface()->DrawTexturedPolygon( 4, v2 );
		}
	}
}

CMainMenuSystem::CMainMenuSystem() : CAutoGameSystem( "CMainMenuSystem" )
{
}

void CMainMenuSystem::PostInit()
{
	CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
	if ( !gameUIFactory )
		return;

	// and get the gameui object
	IGameUI *pGameUI = (IGameUI *)gameUIFactory( GAMEUI_INTERFACE_VERSION, NULL );
	if ( !pGameUI )
		return;

	VPANEL root = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	if ( !root )
		return;

	g_pMainMenu = new CMainMenu( root );
	if ( !g_pMainMenu )
		return;

	pGameUI->SetMainMenuOverride( g_pMainMenu->GetVPanel() );
}

void CMainMenuSystem::LevelInitPostEntity()
{
	g_pMainMenu->LoadGameMenu();
}

void CMainMenuSystem::LevelShutdownPostEntity()
{
	g_pMainMenu->LoadGameMenu();
}

CMainMenu::CMainMenu( VPANEL parent ) : Panel( NULL, "MainMenu" )
{
	SetParent( parent );

	SetVisible( true );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );

	SetPaintBackgroundEnabled( false );

	SetProportional( true );

	m_pMenuBar = new CMenuBar( this );

	// hack: hl2sbpp is default for sandbox mode so its fine
	m_pLogo = new ImageExtButton( this, "Logo", "materials/gamemode/sandbox.png" );

	m_pBackground = new CBackgroundPanel( this, "MainMenuBackground" );
	m_pBackground->SetZPos( -5 );
	m_pBackground->SetVisible( true );
	m_pBackground->LoadBackgroundImages();

	LoadGameMenu();
}

void CMainMenu::LoadGameMenu()
{
	for ( int i = 0; i < m_GameMenuButtons.Count(); i++ )
	{
		if ( m_GameMenuButtons[i] )
		{
			m_GameMenuButtons[i]->MarkForDeletion();
		}
	}
	m_GameMenuButtons.RemoveAll();

	KeyValues *pKV = new KeyValues( "GameMenu" );

	if ( !pKV->LoadFromFile( g_pFullFileSystem, "resource/gamemenu.res", "GAME" ) )
	{
		Warning( "Failed to load gamemenu.res!\n" );
		pKV->deleteThis();
		return;
	}

	int y = scheme()->GetProportionalScaledValue( 165 );
	if ( engine->IsInGame() )
		y = scheme()->GetProportionalScaledValue( 135 ); // hack: this sucks but at least it aligns correctly

	int x = scheme()->GetProportionalScaledValue( 75 );
	int spacing = scheme()->GetProportionalScaledValue( 25 );

	for ( KeyValues *pItem = pKV->GetFirstSubKey(); pItem; pItem = pItem->GetNextKey() )
	{
		const char *pszLabel = pItem->GetString( "label", "" );
		const char *pszCommand = pItem->GetString( "command", "" );

		if ( pszLabel[0] == '\0' && pszCommand[0] == '\0' )
		{
			y += scheme()->GetProportionalScaledValue( 25 );
			continue;
		}

		bool onlyInGame = pItem->GetInt( "OnlyInGame", 0 ) != 0;
		if ( onlyInGame && !engine->IsInGame() )
			continue;

		CUtlString finalCmd;

		if ( Q_strnicmp( pszCommand, "engine ", 7 ) == 0 )
		{
			const char *stripped = pszCommand + 7;
			finalCmd = stripped;
		}
		else
			finalCmd.Format( "gamemenucommand %s", pszCommand );

		CHoverButton *button = new CHoverButton( this, pszLabel );
		button->SetText( pszLabel );
		button->SetCommand( finalCmd );

		button->SetPos( x, y );
		//button->SetWide(250);
		//button->SetTall(20);
		button->InvalidateLayout( true, true );
		button->SizeToContents();

		m_GameMenuButtons.AddToTail( button );

		y += scheme()->GetProportionalScaledValue( 20 );
	}

	pKV->deleteThis();

	PerformLayout(); // todo: should this be here?
}

void CMainMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	// get the size...darn
	int wide, tall;
	GetSize( wide, tall );

	if ( m_pMenuBar )
		m_pMenuBar->SetBounds( 0, tall - 50, wide, 50 );

	float scaleX = (float)ScreenWidth() / 1920.0f;
	float scaleY = (float)ScreenHeight() / 1080.0f;

	int w = 912 * scaleX;
	int h = 512 * scaleY;

	if ( engine->IsInGame() )
		m_pLogo->SetBounds(80 * scaleX, -80 * scaleY, w, h);
	else
		m_pLogo->SetBounds(80 * scaleX, -25 * scaleY, w, h);

	if ( m_pBackground )
		m_pBackground->SetBounds( 0, 0, wide, tall );
}

void CMainMenu::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	BaseClass::OnScreenSizeChanged( iOldWide, iOldTall );

	int wide, tall;
	surface()->GetScreenSize( wide, tall );
	SetSize( wide, tall );

	LoadGameMenu();

	InvalidateLayout( true, true );
}

void CMainMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Resize the panel to the screen size
	// Otherwise, it'll just be in a little corner
	int wide, tall;
	surface()->GetScreenSize( wide, tall );
	SetSize( wide, tall );
}

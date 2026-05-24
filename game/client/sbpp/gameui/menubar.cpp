//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "menubar.h"

#include <vgui/ISurface.h>
#include "filesystem.h"
#include "stb_image.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define RNDBTN_SPACE "     "

static int GetWhiteTexture()
{
	static int texID = -1;
	if ( texID == -1 )
	{
		texID = surface()->CreateNewTextureID( true ); // proc
		unsigned char pixel[4] = { 255, 255, 255, 255 };
		surface()->DrawSetTextureRGBA( texID, pixel, 1, 1, 0, false );
	}
	return texID;
}

static void DrawRoundedBox( int x, int y, int w, int h, int radius, Color col )
{
	if ( w <= 0 || h <= 0 )
		return;

	int half = ( w < h ) ? ( w / 2 ) : ( h / 2 );
	if ( radius < 0 )
		radius = 0;
	if ( radius > half )
		radius = half;

	if ( radius == 0 )
	{
		surface()->DrawSetColor( col );
		surface()->DrawFilledRect( x, y, x + w, y + h );
		return;
	}

	int segments = ( radius < 6 ) ? 6 : radius;
	if ( segments > 64 )
		segments = 64;

	surface()->DrawSetColor( col );
	surface()->DrawFilledRect( x + radius, y, x + w - radius, y + h );

	surface()->DrawFilledRect( x, y + radius, x + radius, y + h - radius );
	surface()->DrawFilledRect( x + w - radius, y + radius, x + w, y + h - radius );

	const float PI = 3.14159265358979323846f;
	int			cornerCx[4];
	int			cornerCy[4];
	float		startAng[4];

	cornerCx[0] = x + radius;
	cornerCy[0] = y + radius;
	startAng[0] = PI; // top-left
	cornerCx[1] = x + w - radius;
	cornerCy[1] = y + radius;
	startAng[1] = -PI / 2.0f; // top-right
	cornerCx[2] = x + w - radius;
	cornerCy[2] = y + h - radius;
	startAng[2] = 0.0f; // bottom-right
	cornerCx[3] = x + radius;
	cornerCy[3] = y + h - radius;
	startAng[3] = PI / 2.0f; // bottom-left

	int whiteTex = GetWhiteTexture();

	for ( int c = 0; c < 4; ++c )
	{
		int		  fanSize = segments + 2;
		Vertex_t *verts = new Vertex_t[fanSize];

		float centerX = (float)cornerCx[c];
		float centerY = (float)cornerCy[c];

		float centerU = ( centerX - x ) / (float)w;
		float centerV = ( centerY - y ) / (float)h;

		verts[0] = Vertex_t( Vector2D( centerX, centerY ), Vector2D( centerU, centerV ) );

		for ( int i = 0; i <= segments; ++i )
		{
			float t = i / (float)segments; // 0..1
			float ang = startAng[c] + t * ( PI / 2.0f );
			float px = centerX + cosf( ang ) * radius;
			float py = centerY + sinf( ang ) * radius;
			float u = ( px - x ) / (float)w;
			float v = ( py - y ) / (float)h;
			verts[i + 1] = Vertex_t( Vector2D( px, py ), Vector2D( u, v ) );
		}

		surface()->DrawSetTexture( whiteTex );
		surface()->DrawSetColor( col );
		surface()->DrawTexturedPolygon( fanSize, verts );

		delete[] verts;
	}
}

static int LoadPNGTexture( const char *filename )
{
	CUtlBuffer buf;
	if ( !g_pFullFileSystem->ReadFile( filename, "MOD", buf ) )
		return -1;

	int			   w, h, c;
	unsigned char *rgba = stbi_load_from_memory( (unsigned char *)buf.Base(), buf.TellPut(), &w, &h, &c, 4 );

	if ( !rgba )
		return -1;

	int texID = surface()->CreateNewTextureID( true );
	surface()->DrawSetTextureRGBA( texID, rgba, w, h, 1, false );

	stbi_image_free( rgba );
	return texID;
}

void CMenuBar::OnGamemodeButtonPressed()
{
	// TODO
}

void CMenuButton::SetIcon( const char *file )
{
	m_iIconTexture = LoadPNGTexture( file );
}

void CMenuButton::PaintBackground()
{
	int w, t;
	GetSize( w, t );

	Color bg( 255, 255, 255, 255 );
	DrawRoundedBox( 0, 0, w, t, 8, bg );

	// icon
	surface()->DrawSetTexture( m_iIconTexture );
	surface()->DrawSetColor( 255, 255, 255, 255 );

	int iconSize = 16;
	int iconY = ( t - iconSize ) / 2;

	surface()->DrawTexturedRect( 4, iconY, 4 + iconSize, iconY + iconSize );
}

CMenuBar::CMenuBar( Panel *parent ) : Panel( parent, "BottomBar" )
{
	SetPaintBackgroundEnabled( true );

	m_pGamemodeButton = new CMenuButton( this, "Gamemodes", RNDBTN_SPACE "Sandbox" );
	m_pGamemodeButton->SetIcon( "materials/icon16/book_open.png" );
	m_pGamemodeButton->SetCommand( "GamemodeButtonPressed" );
	m_pGamemodeButton->AddActionSignalTarget( this );

	// TODO: make this shit work and set these to true
	m_pGamemodeButton->SetEnabled( false );
	m_pGamemodeButton->SetVisible( false );
}

void CMenuBar::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	int buttonWidth = 95;
	int buttonHeight = 50;

	m_pGamemodeButton->SetBounds( wide - buttonWidth, tall - buttonHeight, buttonWidth, buttonHeight );
}

void CMenuBar::Paint()
{
	int wide, tall;
	GetSize( wide, tall );

	surface()->DrawSetColor( Color( 0, 0, 0, 128 ) );
	surface()->DrawFilledRect( 0, 0, wide, tall );
}
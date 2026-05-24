//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "mapload_background.h"
#include <vgui/IScheme.h>
#include <string>
#include "filesystem.h"
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG( char const *panelName ) : EditablePanel( NULL, panelName )
{
	VPANEL toolParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( toolParent );

	// Fenix: We load a RES file rather than create the element here for taking advantage of the "F" parameter for wide and tall
	// Is the sole thing that makes fill the background to the entire screen regardless of the texture size
	// Congratulations to Valve for once again give options to only one side and not both
	SetProportional( true );

	m_pBackground = new ImagePanel( this, "LoadingImage" );
	m_pBackground->SetShouldScaleImage( true );
	m_pBackground->SetImage( "img/bg" );

	m_pGradient = new ImagePanel( this, "Gradient" );
	m_pGradient->SetShouldScaleImage( true );
	m_pGradient->SetImage( "img/gradient" );

	m_pGameLogo = new ImagePanel( this, "Logo" );
	m_pGameLogo->SetShouldScaleImage( true );
	m_pGameLogo->SetImage( "hl2sbpp" );

	m_pServerName = new Label( this, "ServerName", "" );
	m_pMapName = new Label( this, "MapName", "" );
	m_pGameMode = new Label( this, "GameMode", "" );

	m_pMapIcon = new PngImagePanel( this, "PNGPanel", "materials/gui/noicon.png" );

	SetZPos( 99999 );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose: not much
//-----------------------------------------------------------------------------
void CMapLoadBG::OnThink()
{
	BaseClass::OnThink();

	if ( m_pGameLogo )
	{
		float flTime = gpGlobals->curtime;
		float flWave = sin( flTime * 2.0f );
		float flScale = ( flWave + 1.0f ) * 0.5f;

		float flMinScale = 1.0f;
		float flMaxScale = 1.2f;
		float flCurrentScale = flMinScale + ( flMaxScale - flMinScale ) * flScale;

		int newWide = (int)( m_iLogoBaseWide * flCurrentScale );
		int newTall = (int)( m_iLogoBaseTall * flCurrentScale );

		int iWide, iTall;
		surface()->GetScreenSize( iWide, iTall );

		int centerX = iWide / 2;
		int centerY = iTall / 2;

		m_pGameLogo->SetSize( newWide, newTall );
		m_pGameLogo->SetPos( centerX - newWide / 2, centerY - newTall / 2 );
	}
}

void CMapLoadBG::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	int iWide, iTall;
	surface()->GetScreenSize( iWide, iTall );
	SetSize( iWide, iTall );

	m_pBackground->SetBounds( 0, 0, iWide, iTall );
	m_pGradient->SetBounds( 0, 0, 400, 130 );
	m_pGradient->SetAlpha( 128 );

	m_iLogoBaseWide = iWide / 6;
	m_iLogoBaseTall = m_iLogoBaseWide; // square

	int centerX = iWide / 2;
	int centerY = iTall / 2;

	int logoX = centerX - m_iLogoBaseWide / 2;
	int logoY = centerY - m_iLogoBaseTall / 2;

	m_pGameLogo->SetBounds( logoX, logoY, m_iLogoBaseWide, m_iLogoBaseTall );

	vgui::HFont hFont = vgui::surface()->CreateFont();
	int			fontSize = iTall / 30;
	vgui::surface()->SetFontGlyphSet( hFont, "Arial", fontSize, 500, 0, 0, NULL );

	m_pServerName->SetFont( hFont );
	m_pMapName->SetFont( hFont );
	m_pGameMode->SetFont( hFont );

	Color black( 0, 0, 0, 255 );
	m_pServerName->SetFgColor( black );
	m_pMapName->SetFgColor( black );
	m_pGameMode->SetFgColor( black );

	int iconX = iWide * 0.01;
	int iconY = iTall * 0.01;
	m_pMapIcon->SetBounds( iconX, iconY, 200, 200 );

	m_pServerName->SetPos( 135, 20 );
	m_pServerName->SetWide( 400 );

	m_pMapName->SetPos( 135, 20 + m_pServerName->GetTall() + iTall * 0.01 );
	m_pMapName->SetWide( 400 );

	m_pGameMode->SetPos( 135, m_pMapName->GetYPos() + m_pMapName->GetTall() + iTall * 0.01 );
	m_pGameMode->SetWide( 400 );
}

std::string CapitalizeFirstLetter( const char *str )
{
	if ( !str || str[0] == '\0' )
		return "";

	std::string result( str );
	if ( result[0] >= 'a' && result[0] <= 'z' )
	{
		result[0] = result[0] - 'a' + 'A';
	}
	return result;
}

static ConCommand *g_pRealMapCommand = nullptr;
static ConCommand *g_pMapCommand = nullptr;
static ConCommand *g_pRealChangeLevelCommand = nullptr;
static ConCommand *g_pChangeLevelCommand = nullptr;

CON_COMMAND( __map, "Start playing on specified map." )
{
	extern CMapLoadBG *pPanelBg;

	if ( args.ArgC() < 2 )
	{
		Msg( "Usage: map <mapname>\n" );
		return;
	}

	const char *mapName = args[1];

	std::string capServer = CapitalizeFirstLetter( cvar->FindVar( "hostname" )->GetString() );
	pPanelBg->setServerName( capServer.c_str() );

	std::string capGamemode = CapitalizeFirstLetter( cvar->FindVar( "gamemode" )->GetString() );
	pPanelBg->setGameModeName( capGamemode.c_str() );

	std::string capMap = CapitalizeFirstLetter( mapName );
	pPanelBg->setMapName( capMap.c_str() );

	char cmd[MAX_PATH];
	Q_snprintf( cmd, sizeof( cmd ), "progress_enable; __real_map %s", mapName );
	engine->ClientCmd_Unrestricted( cmd );
}

CON_COMMAND( __changelevel, "Custom changelevel wrapper" )
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Usage: changelevel <mapname>\n" );
		return;
	}

	const char *mapName = args[1];

	extern CMapLoadBG *pPanelBg;

	std::string capServer = CapitalizeFirstLetter( cvar->FindVar( "hostname" )->GetString() );
	pPanelBg->setServerName( capServer.c_str() );

	std::string capGamemode = CapitalizeFirstLetter( cvar->FindVar( "gamemode" )->GetString() );
	pPanelBg->setGameModeName( capGamemode.c_str() );

	std::string capMap = CapitalizeFirstLetter( mapName );
	pPanelBg->setMapName( capMap.c_str() );

	if ( g_pRealChangeLevelCommand )
	{
		char cmd[MAX_PATH];
		Q_snprintf( cmd, sizeof( cmd ), "__real_changelevel %s", mapName );
		engine->ClientCmd_Unrestricted( cmd );
	}
}

void SwapMapCommand()
{
	ConCommand *pRealMap = dynamic_cast< ConCommand * >( g_pCVar->FindCommand( "map" ) );
	ConCommand *pOurMap = dynamic_cast< ConCommand * >( g_pCVar->FindCommand( "__map" ) );

	if ( pRealMap && pOurMap )
	{
		g_pRealMapCommand = pRealMap;
		g_pMapCommand = pOurMap;

		pRealMap->Shutdown();
		pRealMap->CreateBase( "__real_map", "Original map command" );
		pRealMap->Init();

		pOurMap->Shutdown();
		pOurMap->CreateBase( "map", "Custom map command" );
		pOurMap->Init();
	}

	ConCommand *pRealChangeLevel = dynamic_cast< ConCommand * >( g_pCVar->FindCommand( "changelevel" ) );
	ConCommand *pOurChangeLevel = dynamic_cast< ConCommand * >( g_pCVar->FindCommand( "__changelevel" ) );

	if ( pRealChangeLevel && pOurChangeLevel )
	{
		g_pRealChangeLevelCommand = pRealChangeLevel;
		g_pChangeLevelCommand = pOurChangeLevel;

		pRealChangeLevel->Shutdown();
		pRealChangeLevel->CreateBase( "__real_changelevel", "Original changelevel command" );
		pRealChangeLevel->Init();

		pOurChangeLevel->Shutdown();
		pOurChangeLevel->CreateBase( "changelevel", "Custom changelevel command" );
		pOurChangeLevel->Init();
	}
}

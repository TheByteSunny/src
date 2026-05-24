//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "hud_watermark.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "hud_macros.h"
#include "vgui_controls/Label.h"
#include "hud.h"
#include "hud_macros.h"
#include "clientmode_hl2mpnormal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudWatermark );

using namespace vgui;

CHudWatermark::CHudWatermark( const char *pElementName ) : CHudElement( pElementName ), Panel( NULL, "HudWatermark" )
{
	SetHiddenBits( HIDEHUD_MISCSTATUS );
	SetPaintBackgroundEnabled( false );

	SetParent( GetClientModeHL2MPNormal()->GetViewport() );
	SetPos( 0, 0 );
	SetSize( ScreenWidth(), ScreenHeight() );

	wchar_t gameName[256];
	g_pVGuiLocalize->ConvertANSIToUnicode( "Half-Life 2: Sandbox++", gameName, sizeof( gameName ) );
	m_pGameName = new Label( this, "GameNameLabel", gameName );
	m_pGameName->SetAlpha( 16 );
	m_pGameName->SetContentAlignment( Label::a_northeast );
	m_pGameName->SetFont( vgui::scheme()->GetIScheme( vgui::scheme()->GetDefaultScheme() )->GetFont( "DefaultSmall", true ) );

	wchar_t discord[256];
	g_pVGuiLocalize->ConvertANSIToUnicode( "https://discord.gg/3DkET6fqXr", discord, sizeof( discord ) );
	m_pDiscord = new Label( this, "DiscordLabel", discord );
	m_pDiscord->SetAlpha( 16 );
	m_pDiscord->SetContentAlignment( Label::a_northeast );
	m_pDiscord->SetFont( vgui::scheme()->GetIScheme( vgui::scheme()->GetDefaultScheme() )->GetFont( "DefaultSmall", true ) );
}

void CHudWatermark::Paint()
{
	int padding = 10;

	int screenWidth, screenHeight;
	vgui::surface()->GetScreenSize( screenWidth, screenHeight );

	m_pGameName->SetFgColor( Color( 255, 255, 255, 16 ) );
	m_pDiscord->SetFgColor( Color( 255, 255, 255, 16 ) );

	int gameWidth, gameHeight, discordWidth, discordHeight;
	m_pGameName->GetContentSize( gameWidth, gameHeight );
	m_pDiscord->GetContentSize( discordWidth, discordHeight );

	m_pGameName->SetBounds( screenWidth - gameWidth - padding, padding, gameWidth, gameHeight );
	m_pDiscord->SetBounds( screenWidth - discordWidth - padding, padding + gameHeight + 2, discordWidth, discordHeight );

	BaseClass::Paint();
}

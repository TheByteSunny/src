//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "creatempdialog.h"
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include "sbpp/mapload_background.h"
#include "filesystem.h"
#include "tier1/utlvector.h"
#include <list>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar selmap( "selmap", "", FCVAR_DEVELOPMENTONLY );
ConVar mpdialog( "mpdialog", "0" );

CUtlVector< const char * > hardcodedMaps;

GameMapsPanel::GameMapsPanel( vgui::Panel *parent, const char *pName ) : MapListPanel( parent, pName )
{
	SetBounds( 0, 0, 800, 640 );

	char gamePaths[8192];
	char modPath[1024];
	g_pFullFileSystem->GetSearchPath( "GAME", true, gamePaths, sizeof( gamePaths ) );
	g_pFullFileSystem->GetSearchPath( "MOD", true, modPath, sizeof( modPath ) );

	CUtlVector< const char * > modPaths;
	char					  *modTok = strtok( modPath, ";" );
	while ( modTok )
	{
		modPaths.AddToTail( modTok );
		modTok = strtok( NULL, ";" );
	}

	char *path = strtok( gamePaths, ";" );
	while ( path )
	{
		bool isMod = false;
		for ( int i = 0; i < modPaths.Count(); i++ )
		{
			if ( !Q_stricmp( path, modPaths[i] ) )
			{
				isMod = true;
				break;
			}
		}

		if ( !isMod && !Q_stristr( path, "addons" ) )
		{
			const char *tempID = "MAPSCAN";
			g_pFullFileSystem->AddSearchPath( path, tempID );
			FileFindHandle_t handle;
			const char		*file = g_pFullFileSystem->FindFirstEx( "maps/*.bsp", tempID, &handle );
			if ( file )
			{
				do
				{
					if ( g_pFullFileSystem->FindIsDirectory( handle ) )
						continue;
					const char *mapName = file;
					if ( !Q_strnicmp( mapName, "maps/", 5 ) )
						mapName += 5;
					char mapNoExt[MAX_PATH];
					Q_StripExtension( mapName, mapNoExt, sizeof( mapNoExt ) );
					bool duplicate = false;
					for ( int i = 0; i < hardcodedMaps.Count(); i++ )
					{
						if ( !Q_stricmp( hardcodedMaps[i], mapNoExt ) )
						{
							duplicate = true;
							break;
						}
					}
					if ( !duplicate )
					{
						char *copy = new char[strlen( mapNoExt ) + 1];
						Q_strcpy( copy, mapNoExt );
						hardcodedMaps.AddToTail( copy );
					}
				} while ( ( file = g_pFullFileSystem->FindNext( handle ) ) );
				g_pFullFileSystem->FindClose( handle );
			}
			g_pFullFileSystem->RemoveSearchPath( path, tempID );
		}
		path = strtok( NULL, ";" );
	}

	for ( int i = 0; i < hardcodedMaps.Count(); i++ )
	{
		char pngPath[260];
		Q_snprintf( pngPath, sizeof( pngPath ), "maps/thumb/%s.png", hardcodedMaps[i] );

		char command[128];
		Q_snprintf( command, sizeof( command ), "select %s", hardcodedMaps[i] );

		if ( g_pFullFileSystem->FileExists( pngPath ) )
			AddButton( this, pngPath, command, hardcodedMaps[i] );
		else
			AddButton( this, "materials/gui/noicon.png", command, hardcodedMaps[i] );
	}

	InvalidateLayout( true );
	MoveScrollBarToTop();

	PerformLayout();
}

ServerSettingsPanel::ServerSettingsPanel( vgui::Panel *parent, const char *pName ) : BaseClass( parent, pName )
{
	SetBounds( 0, 0, 800, 640 );

	const int labelWidth = 180;
	const int inputWidth = 200;
	const int rowHeight = 30;
	const int xLabel = 10;
	const int xInput = xLabel + labelWidth + 10;
	int		  y = 10;

	vgui::Label *lblMaxPlayers = new vgui::Label( this, "MaxPlayersLabel", "Max Players:" );
	lblMaxPlayers->SetPos( xLabel, y );
	lblMaxPlayers->SetSize( labelWidth, rowHeight );
	lblMaxPlayers->SetContentAlignment( vgui::Label::a_west );

	m_pMaxPlayers = new vgui::TextEntry( this, "MaxPlayersEntry" );
	m_pMaxPlayers->SetText( "1" );
	m_pMaxPlayers->SetSize( inputWidth, rowHeight );
	m_pMaxPlayers->SetPos( xInput, y );
	y += rowHeight + 15;

	vgui::Label *lblHostname = new vgui::Label( this, "HostnameLabel", "Hostname:" );
	lblHostname->SetPos( xLabel, y );
	lblHostname->SetSize( labelWidth, rowHeight );
	lblHostname->SetContentAlignment( vgui::Label::a_west );

	m_pHostname = new vgui::TextEntry( this, "HostnameEntry" );
	m_pHostname->SetText( "My HL2SB++ Server" );
	m_pHostname->SetSize( inputWidth, rowHeight );
	m_pHostname->SetPos( xInput, y );
	y += rowHeight + 15;

	vgui::Label *lblPassword = new vgui::Label( this, "PasswordLabel", "Password:" );
	lblPassword->SetPos( xLabel, y );
	lblPassword->SetSize( labelWidth, rowHeight );
	lblPassword->SetContentAlignment( vgui::Label::a_west );

	m_pPassword = new vgui::TextEntry( this, "PasswordEntry" );
	m_pPassword->SetText( "" ); // placeholder
	m_pPassword->SetSize( inputWidth, rowHeight );
	m_pPassword->SetPos( xInput, y );
	m_pPassword->SetTextHidden( true );

	y += rowHeight + 15;

	vgui::Label *lblGamemode = new vgui::Label( this, "GamemodeLabel", "Gamemode:" );
	lblGamemode->SetPos( xLabel, y );
	lblGamemode->SetSize( labelWidth, rowHeight );
	lblGamemode->SetContentAlignment( vgui::Label::a_west );

	m_pGamemodeCombo = new vgui::ComboBox( this, "GamemodeCombo", 6, false );
	m_pGamemodeCombo->SetSize( inputWidth, rowHeight );
	m_pGamemodeCombo->SetPos( xInput, y );
	y += rowHeight + 15;

	LoadGamemodes();
}

void ServerSettingsPanel::LoadGamemodes()
{
	if ( !m_pGamemodeCombo )
		return;

	int defaultIndex = m_pGamemodeCombo->AddItem( "Default", NULL );
	m_pGamemodeCombo->ActivateItem( defaultIndex );

	FileFindHandle_t findhandle;
	for ( const char *p = g_pFullFileSystem->FindFirstEx( "gamemodes/*", "MOD", &findhandle ); p && *p; p = g_pFullFileSystem->FindNext( findhandle ) )
	{
		if ( strchr( p, '.' ) )
			continue;

		m_pGamemodeCombo->AddItem( p, NULL );
	}
	g_pFullFileSystem->FindClose( findhandle );
}

void ServerSettingsPanel::OnTick( void )
{
	BaseClass::OnTick();

	if ( !IsVisible() )
		return;
}

void ServerSettingsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void ServerSettingsPanel::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}

MapListPanel::MapListPanel( vgui::Panel *parent, const char *pName ) : BaseClass( parent, pName )
{
	SetBounds( 0, 0, 800, 640 );
	m_pSelectedButton = nullptr;
	m_bMapsLoaded = false;
}

void MapListPanel::OnTick( void )
{
	BaseClass::OnTick();

	if ( !IsVisible() )
		return;

	int c = layoutItems.Count();
	for ( int i = 0; i < c; i++ )
	{
		vgui::Panel *p = layoutItems[i];
		p->OnTick();
	}
}

void MapListPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = 127;
	int h = 127;
	int x = 5;
	int y = 5;
	int gap = 2;

	int c = layoutItems.Count();
	int wide = GetWide();

	for ( int i = 0; i < c; i++ )
	{
		vgui::Panel *p = layoutItems[i];
		p->SetBounds( x, y, w, h );

		x += ( w + gap );
		if ( x >= wide - w )
		{
			y += ( h + gap );
			x = 5;
		}
	}
}

class LabImageExtButton : public ImageExtButton
{
	DECLARE_CLASS_SIMPLE( LabImageExtButton, ImageExtButton );

public:
	LabImageExtButton( vgui::Panel *parent, const char *panelName, const char *normalImage, const char *mouseOverImage = nullptr, const char *mouseClickImage = nullptr, const char *pCmd = nullptr ) :
		ImageExtButton( parent, panelName, normalImage, mouseOverImage, mouseClickImage, pCmd ),
		m_bHovered( false ),
		m_bSelected( false ),
		m_overlayAlpha( 128 )
	{
		m_pLabel = new vgui::Label( this, "MapLabel", panelName );
		m_pLabel->SetContentAlignment( vgui::Label::a_center );
		m_pLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pLabel->SetBgColor( Color( 0, 0, 0, 0 ) );

		s_allButtons.push_back( this );
	}

	virtual ~LabImageExtButton()
	{
		s_allButtons.remove( this );
	}

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );
		m_pLabel->SetFont( pScheme->GetFont( "DefaultSmall", true ) );
	}

	virtual void PerformLayout() OVERRIDE
	{
		BaseClass::PerformLayout();

		int wide, tall;
		GetSize( wide, tall );
		int labelHeight = tall * 0.25f;
		m_pLabel->SetBounds( 0, tall - labelHeight, wide, labelHeight );
	}

	virtual void OnCursorEntered() OVERRIDE
	{
		BaseClass::OnCursorEntered();
		m_bHovered = true;
		Repaint();
	}

	virtual void OnCursorExited() OVERRIDE
	{
		BaseClass::OnCursorExited();
		m_bHovered = false;
		Repaint();
	}

	virtual void OnMouseReleased( vgui::MouseCode code ) OVERRIDE
	{
		BaseClass::OnMouseReleased( code );

		for ( auto *btn : s_allButtons )
		{
			if ( btn != this )
				btn->SetSelected( false );
		}

		// select self lol
		SetSelected( true );
		Repaint();
	}

	virtual void Paint() OVERRIDE
	{
		BaseClass::Paint();

		int wide, tall;
		GetSize( wide, tall );

		// overlay
		Color overlayColor( 0, 0, 0, m_overlayAlpha );
		vgui::surface()->DrawSetColor( overlayColor );
		int overlayHeight = tall * 0.25f;
		vgui::surface()->DrawFilledRect( 0, tall - overlayHeight, wide, tall );

		if ( m_bSelected )
		{
			Color selectedOverlay( 0, 255, 255, 64 ); // 25% im lazy pls send help
			vgui::surface()->DrawSetColor( selectedOverlay );
			vgui::surface()->DrawFilledRect( 0, 0, wide, tall );
		}

		if ( m_bHovered || m_bSelected )
		{
			Color borderColor = m_bSelected ? Color( 0, 255, 255, 255 ) : Color( 0, 255, 255, 128 );
			vgui::surface()->DrawSetColor( borderColor );
			vgui::surface()->DrawOutlinedRect( 0, 0, wide, tall );
		}
	}

	void SetSelected( bool state )
	{
		m_bSelected = state;
	}

	bool IsSelected() const
	{
		return m_bSelected;
	}

private:
	vgui::Label *m_pLabel;
	bool		 m_bHovered;
	bool		 m_bSelected;
	int			 m_overlayAlpha;

	static std::list< LabImageExtButton * > s_allButtons;
};

std::list< LabImageExtButton * > LabImageExtButton::s_allButtons;

void MapListPanel::AddButton( MapListPanel *panel, const char *image, const char *command, const char *mapName )
{
	LabImageExtButton *btn = new LabImageExtButton( panel, mapName, image, nullptr, nullptr, command );

	layoutItems.AddToTail( btn );
	panel->AddItem( NULL, btn );

	btn->SetFgColor( Color( 180, 180, 180, 255 ) );

	if ( BaseTooltip *pTooltip = btn->GetTooltip() )
		pTooltip->SetText( mapName );
}

void MapList::OnCancel()
{
	mpdialog.SetValue( 0 );
}

void MapList::OnClose()
{
	mpdialog.SetValue( 0 );
}

static const char *FilenameOnly( const char *path )
{
	if ( !path || !path[0] )
		return path;
	const char *p1 = strrchr( path, '/' );
	const char *p2 = strrchr( path, '\\' );
	const char *p = p1 > p2 ? p1 : p2;
	return p ? ( p + 1 ) : path;
}

void MapListPanel::LoadMaps( MapListPanel *panel )
{
	if ( m_bMapsLoaded )
		return;
	m_bMapsLoaded = true;

	layoutItems.RemoveAll();

	FileFindHandle_t mapHandle;
	for ( const char *pMap = g_pFullFileSystem->FindFirstEx( "maps/*.bsp", "MOD", &mapHandle ); pMap && *pMap; pMap = g_pFullFileSystem->FindNext( mapHandle ) )
	{
		char mapName[MAX_PATH];
		Q_FileBase( pMap, mapName, sizeof( mapName ) );

		char searchPattern[MAX_PATH];
		Q_snprintf( searchPattern, sizeof( searchPattern ), "maps/thumb/%s.*", mapName );

		FileFindHandle_t thumbHandle;
		char			 imageName[MAX_PATH] = "";

		for ( const char *foundFile = g_pFullFileSystem->FindFirstEx( searchPattern, "MOD", &thumbHandle ); foundFile && *foundFile; foundFile = g_pFullFileSystem->FindNext( thumbHandle ) )
		{
			char base[MAX_PATH];
			Q_FileBase( foundFile, base, sizeof( base ) );

			if ( !Q_stricmp( base, mapName ) ) // exact match
			{
				Q_snprintf( imageName, sizeof( imageName ), "maps/thumb/%s", foundFile );
				break;
			}
		}
		g_pFullFileSystem->FindClose( thumbHandle );

		if ( imageName[0] == '\0' )
		{
			Q_strncpy( imageName, "materials/gui/noicon.png", sizeof( imageName ) );
		}

		char imageCommand[MAX_PATH];
		Q_snprintf( imageCommand, sizeof( imageCommand ), "select %s", mapName );

		AddButton( panel, imageName, imageCommand, mapName );
	}

	g_pFullFileSystem->FindClose( mapHandle );

	panel->InvalidateLayout( true );
	panel->MoveScrollBarToTop();
}

void MapListPanel::OnCommand( const char *command )
{
	if ( Q_strnicmp( command, "select ", 7 ) == 0 )
	{
		const char *mapName = command + 7;
		char		mapNameNoExt[MAX_PATH];
		Q_strncpy( mapNameNoExt, mapName, sizeof( mapNameNoExt ) );
		Q_StripExtension( mapNameNoExt, mapNameNoExt, sizeof( mapNameNoExt ) );
		selmap.SetValue( mapNameNoExt );

		if ( m_pSelectedButton )
			m_pSelectedButton->SetFgColor( Color( 180, 180, 180, 255 ) );

		Msg( "Selected %s\n", mapNameNoExt );

		for ( int i = 0; i < layoutItems.Count(); i++ )
		{
			ImageExtButton *btn = dynamic_cast< ImageExtButton * >( layoutItems[i] );
			if ( !btn )
				continue;

			if ( Q_strcmp( btn->GetCommand(), command ) == 0 )
			{
				btn->SetFgColor( Color( 255, 255, 255, 255 ) );

				m_pSelectedButton = btn;
				break;
			}
		}
	}
}

MapList::MapList( vgui::VPANEL *parent, const char *pName ) : BaseClass( NULL, "MapList" )
{
	int iWide, iTall;
	surface()->GetScreenSize( iWide, iTall );

	SetSize( iWide / 1.25, iTall / 1.25 );
	SetTitle( "New Game", true );

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );
	SetPos( ( screenWide - GetWide() ) / 2, ( screenTall - GetTall() ) / 2 );

	MapListPanel		*maplist = new MapListPanel( this, NULL );
	ServerSettingsPanel *info = new ServerSettingsPanel( this, NULL );
	GameMapsPanel		*gamemaps = new GameMapsPanel( this, NULL );
	maplist->LoadMaps( maplist );
	AddPage( maplist, "Maps" );
	AddPage( gamemaps, "Game Maps" );
	AddPage( info, "Server Settings" );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );

	GetPropertySheet()->SetTabWidth( 72 );
	SetMoveable( true );
	SetVisible( true );
	SetSizeable( true );
	SetProportional( true );
	SetApplyButtonVisible( false );
}

bool MapList::OnOK( bool applyOnly )
{
	const char *mapName = selmap.GetString();
	if ( mapName && mapName[0] != '\0' )
	{
		int			maxPlayers = 16;
		const char *hostname = "My Server";
		const char *password = "";

		ServerSettingsPanel *infoPanel = dynamic_cast< ServerSettingsPanel * >( GetPropertySheet()->GetPage( 2 ) );

		char maxPlayersBuffer[2048];
		infoPanel->m_pMaxPlayers->GetText( maxPlayersBuffer, sizeof( maxPlayersBuffer ) );
		maxPlayers = atoi( maxPlayersBuffer );

		char hostnameBuffer[2048];
		infoPanel->m_pHostname->GetText( hostnameBuffer, sizeof( hostnameBuffer ) );

		char passwordBuffer[2048];
		infoPanel->m_pPassword->GetText( passwordBuffer, sizeof( passwordBuffer ) );

		char gamemodeBuffer[2048] = "";
		if ( infoPanel->m_pGamemodeCombo )
			infoPanel->m_pGamemodeCombo->GetText( gamemodeBuffer, sizeof( gamemodeBuffer ) );

		char szMapCommand[2048];
		if ( gamemodeBuffer[0] != '\0' && Q_strcmp( gamemodeBuffer, "Default" ) != 0 )
		{
			Q_snprintf( szMapCommand, sizeof( szMapCommand ), "disconnect\nwait\nwait\nsv_lan 1\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\ngamemode \"%s\"\nmap %s\n", maxPlayers, password, hostnameBuffer, gamemodeBuffer,
				selmap.GetString() );
		}
		else
		{
			Q_snprintf( szMapCommand, sizeof( szMapCommand ), "disconnect\nwait\nwait\nsv_lan 1\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nprogress_enable\nmap %s\ngamemode sandbox\n", maxPlayers, password, hostnameBuffer,
				selmap.GetString() );
		}

		engine->ClientCmd_Unrestricted( szMapCommand );
	}

	mpdialog.SetValue( "0" ); // cannibalism

	return true;
}

static bool isfront = false;

void MapList::OnTick()
{
	BaseClass::OnTick();

	if ( !mpdialog.GetBool() )
		isfront = false;

	SetVisible( mpdialog.GetBool() );
	if ( IsVisible() && isfront == false )
	{
		// LOL!
		MoveToFront();
		RequestFocus();
		isfront = true;
	}
}

int MapListPanel::ComputeVPixelsNeeded()
{
	int count = layoutItems.Count();
	if ( count == 0 )
		return 0;

	const int tileW = 127;
	const int tileH = 127;
	const int gap = 2;
	const int leftPadding = 5;
	int		  wide = GetWide();

	if ( wide <= tileW )
		wide = tileW + 1;

	int cols = ( wide - leftPadding ) / ( tileW + gap );
	if ( cols < 1 )
		cols = 1;

	int rows = ( count + cols - 1 ) / cols;

	int total = rows * ( tileH + gap ) + leftPadding;
	return total;
}

class MapListInterface : public CMapList
{
private:
	MapList *CMapList;

public:
	MapListInterface()
	{
		CMapList = NULL;
	}
	void Create( vgui::VPANEL parent )
	{
		CMapList = new MapList( &parent, NULL );
	}
	void Destroy()
	{
		if ( CMapList )
		{
			CMapList->SetParent( (vgui::Panel *)NULL );
			delete CMapList;
		}
	}
	void Activate( void )
	{
		if ( CMapList )
		{
			CMapList->Activate();
		}
	}
};
static MapListInterface g_MapList;
CMapList			   *maplist = (CMapList *)&g_MapList;
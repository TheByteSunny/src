//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "spawnmenu.h"
#include <vgui/IVGui.h>
#include "fmtstr.h"

using namespace vgui;

extern ConVar hl2_mounted;
extern ConVar portal_mounted;
extern ConVar css_mounted;
extern ConVar hl1_mounted;

ConVar spawn( "cl_propmenu", "0", FCVAR_CLIENTDLL, "spawn menu" );

CSMLMenu *CSMLMenu::s_Instance = nullptr;

CSMLButton::CSMLButton( Panel *parent, const char *panelName, const char *text ) : MenuButton( parent, panelName, text ), m_pMenu( nullptr )
{
}

CSMLButton::~CSMLButton()
{
	if ( m_pMenu )
	{
		delete m_pMenu;
		m_pMenu = nullptr;
	}
}

CSMLCommandButton::CSMLCommandButton( Panel *parent, const char *panelName, const char *labelText, const char *command ) : Button( parent, panelName, labelText )
{
	AddActionSignalTarget( this );
	SetCommand( command );
}

void CSMLCommandButton::OnCommand( const char *command )
{
	engine->ClientCmd( (char *)command );
}

CSMLCommandButton::~CSMLCommandButton()
{
}

CSMLPage::CSMLPage( Panel *parent, const char *panelName ) : PropertyPage( parent, panelName ), m_bInScrollUpdate( false )
{
	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );

	m_pContentPanel = new Panel( this, "ContentPanel" );

	m_pScrollBar = new ScrollBar( this, "PageScrollBar", true );
	m_pScrollBar->SetVisible( false );
	m_pScrollBar->AddActionSignalTarget( this );

	SetMouseInputEnabled( true );
}

CSMLPage::~CSMLPage()
{
	for ( int i = 0; i < m_LayoutItems.Count(); ++i )
	{
		delete m_LayoutItems[i];
	}
	m_LayoutItems.RemoveAll();
}

void CSMLPage::OnTick()
{
	if ( !IsVisible() )
		return;

	for ( int i = 0; i < m_LayoutItems.Count(); ++i )
	{
		Panel *p = m_LayoutItems[i];
		p->OnTick();
	}
}

void CSMLPage::OnSliderMoved( int value )
{
	if ( !m_pContentPanel || !m_pScrollBar->IsVisible() )
		return;

	m_pContentPanel->SetPos( 0, -value );
}

void CSMLPage::PerformLayout()
{
	int btnW = 150, btnH = 38, gap = 2;
	int scrollWide = 15;
	int numItems = m_LayoutItems.Count();
	if ( numItems == 0 )
		return;

	int availWide = GetWide() - 10;
	int cols = max( 1, availWide / ( btnW + gap ) );
	int itemsPerCol = ( numItems + cols - 1 ) / cols;
	int requiredTall = 10 + itemsPerCol * ( btnH + gap ) - gap;

	bool needScroll = requiredTall > GetTall() - 10;
	if ( needScroll )
	{
		availWide -= ( scrollWide + gap );
		cols = max( 1, availWide / ( btnW + gap ) );
		itemsPerCol = ( numItems + cols - 1 ) / cols;
		requiredTall = 10 + itemsPerCol * ( btnH + gap ) - gap;
	}

	m_pScrollBar->SetVisible( needScroll );
	int viewTall = GetTall() - 10;
	int contentTall = max( 1, requiredTall );
	if ( needScroll )
	{
		m_pScrollBar->SetBounds( GetWide() - scrollWide - 5, 5, scrollWide, GetTall() - 10 );

		m_pScrollBar->SetRange( 0, contentTall - 1 );
		m_pScrollBar->SetRangeWindow( viewTall );
		m_pScrollBar->InvalidateLayout( true, true );
	}

	int contentWide = GetWide() - ( needScroll ? scrollWide + gap + 5 : 10 );
	m_pContentPanel->SetBounds( 0, 0, contentWide, requiredTall );

	int itemIdx = 0;
	for ( int col = 0; col < cols && itemIdx < numItems; ++col )
	{
		int x = 5 + col * ( btnW + gap );
		int y = 5;
		for ( int row = 0; row < itemsPerCol && itemIdx < numItems; ++row, ++itemIdx )
		{
			Panel *p = m_LayoutItems[itemIdx];
			p->SetBounds( x, y, btnW, btnH );
			y += btnH + gap;
		}
	}

	if ( needScroll )
	{
		int val = m_pScrollBar->GetValue();
		m_pContentPanel->SetPos( 0, -val );
	}
	else
	{
		m_pContentPanel->SetPos( 0, 0 );
	}
}

void CSMLPage::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "ScrollBarSliderMoved" ) || !Q_stricmp( command, "ScrollBarButtonPressed" ) )
	{
		if ( m_pContentPanel && m_pScrollBar->IsVisible() )
		{
			InvalidateLayout( false, true ); // Trigger layout update
		}
	}
}

bool m_bInScrollUpdate = false;

void CSMLPage::OnMouseWheeled( int delta )
{
	if ( m_bInScrollUpdate )
		return;
	if ( m_pScrollBar && m_pScrollBar->IsVisible() )
	{
		m_bInScrollUpdate = true;

		int currentVal = m_pScrollBar->GetValue();
		int newVal = currentVal - ( delta * 3 * 18 );

		int minVal, maxVal;
		m_pScrollBar->GetRange( minVal, maxVal );
		newVal = clamp( newVal, minVal, maxVal );

		m_pScrollBar->SetValue( newVal );
		InvalidateLayout( false, true );

		m_bInScrollUpdate = false;
	}
}

void CSMLPage::Init( KeyValues *kv )
{
	for ( KeyValues *control = kv->GetFirstSubKey(); control != nullptr; control = control->GetNextKey() )
	{
		const char *m = control->GetString( "#", "" );
		if ( m && m[0] )
		{
			CreateButtonX( this, control->GetName(), m );
		}
	}
}

void CSMLPage::CreateButtonX( CSMLPage *page, const char *label, const char *command )
{
	if ( label && command && label[0] && command[0] )
	{
#ifdef HL2SB
		if ( Q_strlen( label ) > 4 && ( !Q_stricmp( label + Q_strlen( label ) - 5, "(EP2)" ) || !Q_stricmp( label + Q_strlen( label ) - 5, "(EP1)" ) || !Q_stricmp( label + Q_strlen( label ) - 12, "(Lost Coast)" ) ||
										  !Q_stricmp( label + Q_strlen( label ) - 7, "(Portal)" ) || !Q_stricmp( label + Q_strlen( label ) - 5, "(HL1)" ) ) )
		{
			if ( !hl2_mounted.GetBool() && ( !Q_stricmp( label + Q_strlen( label ) - 5, "(EP2)" ) || !Q_stricmp( label + Q_strlen( label ) - 5, "(EP1)" ) || !Q_stricmp( label + Q_strlen( label ) - 12, "(Lost Coast)" ) ) )
			{
				DevMsg( "HL2 not mounted, skipping..\n" );
				return;
			}
			else if ( !portal_mounted.GetBool() && !Q_stricmp( label + Q_strlen( label ) - 7, "(Portal)" ) )
			{
				DevMsg( "Portal not mounted, skipping..\n" );
				return;
			}
			else if ( !hl1_mounted.GetBool() && !Q_stricmp( label + Q_strlen( label ) - 5, "(HL1)" ) )
			{
				DevMsg( "HL1 not mounted, skipping..\n" );
				return;
			}
		}
#endif

		for ( int i = 0; i < page->m_LayoutItems.Count(); ++i )
		{
			CSMLCommandButton *existingBtn = dynamic_cast< CSMLCommandButton * >( page->m_LayoutItems[i] );
			if ( existingBtn )
			{
				char existingLabel[4086];
				existingBtn->GetText( existingLabel, sizeof( existingLabel ) );
				if ( !Q_stricmp( existingLabel, label ) )
				{
					DevMsg( "Button '%s' already exists. Skipping.\n", label );
					return;
				}
			}
		}

		CSMLCommandButton *btn = new CSMLCommandButton( page->m_pContentPanel, label, label, command );
		page->m_LayoutItems.AddToTail( btn );
	}
}

CSMLMenu::CSMLMenu( vgui::VPANEL *parent, const char *panelName ) : PropertyDialog( nullptr, "SMLenu" )
{
	SetTitle( "SPAWN MENU", true );
	SetPos( 0, 0 );

	/* size */
	int wide = ScreenWidth() / 1.25;
	int tall = ScreenHeight() / 1.25;

	SetWide( wide );
	SetTall( tall );

	/* center */
	int x = ( ScreenWidth() - wide ) / 2;
	int y = ( ScreenHeight() - tall ) / 2;
	SetPos( x, y );

	// ok
	SetMoveable( false );
	SetSizeable( false );

	FileFindHandle_t handle;
	const char		*filename = g_pFullFileSystem->FindFirst( "settings/*.txt", &handle );

	while ( filename )
	{
		KeyValues *kv = new KeyValues( "SMLenu" );
		if ( kv->LoadFromFile( g_pFullFileSystem, CFmtStr( "settings/%s", filename ) ) )
		{
			for ( KeyValues *dat = kv->GetFirstSubKey(); dat != nullptr; dat = dat->GetNextKey() )
			{
				// Skip unmounted content
				if ( !Q_stricmp( dat->GetName(), "Portal" ) && !portal_mounted.GetBool() )
					continue;
				if ( !Q_stricmp( dat->GetName(), "CSS" ) && !css_mounted.GetBool() )
					continue;

				CSMLPage *page = new CSMLPage( this, dat->GetName() );
				page->Init( dat );
				AddPage( page, dat->GetName() );
				m_PageRegistry.emplace( std::string( dat->GetName() ), page );
			}
		}
		kv->deleteThis();

		filename = g_pFullFileSystem->FindNext( handle );
	}
	g_pFullFileSystem->FindClose( handle );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
	GetPropertySheet()->SetTabWidth( 72 );
	SetVisible( true );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( false ); /* allow movement */
	SetSizeable( false );			  /* don't allow resizing */
}

CSMLMenu::~CSMLMenu()
{
	for ( auto &entry : m_PageRegistry )
	{
		delete entry.second;
	}
	m_PageRegistry.clear();
}

void CSMLMenu::OnClose()
{
	// hacky hack
	spawn.SetValue( "0" );
}

void CSMLMenu::CreateButton( const char *pageName, const char *label, const char *command )
{
	if ( !s_Instance )
	{
		return;
	}

	CSMLPage *page = s_Instance->FindOrCreatePage( pageName );
	if ( page )
	{
		CSMLPage::CreateButtonX( page, label, command );
	}
}

CSMLPage *CSMLMenu::FindOrCreatePage( const char *pageName )
{
	if ( !pageName || !pageName[0] )
	{
		return nullptr;
	}

	std::string key( pageName );
	auto		it = m_PageRegistry.find( key );
	if ( it != m_PageRegistry.end() )
	{
		return it->second;
	}

	CSMLPage *newPage = new CSMLPage( this, pageName );
	m_PageRegistry.emplace( std::move( key ), newPage );
	AddPage( newPage, pageName );
	return newPage;
}

void CSMLMenu::OnTick()
{
	/* size */
	int wide = ScreenWidth() / 1.25;
	int tall = ScreenHeight() / 1.25;

	SetWide( wide );
	SetTall( tall );

	/* center */
	int x = ( ScreenWidth() - wide ) / 2;
	int y = ( ScreenHeight() - tall ) / 2;
	SetPos( x, y );

	SetVisible( spawn.GetBool() );
}

CSMLPanelInterface::CSMLPanelInterface() : SMLPanel( nullptr )
{
}

CSMLPanelInterface::~CSMLPanelInterface()
{
	Destroy();
}

void CSMLPanelInterface::Create( vgui::VPANEL parent )
{
	if ( !SMLPanel )
	{
		SMLPanel = new CSMLMenu( &parent, "SMenu" );
		CSMLMenu::s_Instance = SMLPanel;
	}
}

void CSMLPanelInterface::Destroy()
{
	if ( SMLPanel )
	{
		SMLPanel->SetParent( (vgui::Panel *)nullptr );
		delete SMLPanel;
		SMLPanel = nullptr;
		CSMLMenu::s_Instance = nullptr;
	}
}

void CSMLPanelInterface::Activate()
{
	if ( SMLPanel )
	{
		SMLPanel->Activate();
	}
}

void CreateButton( const char *pageName, const char *label, const char *command )
{
	if ( !CSMLMenu::s_Instance )
	{
		return;
	}

	CSMLPage *page = CSMLMenu::s_Instance->FindOrCreatePage( pageName );
	if ( page )
	{
		CSMLPage::CreateButtonX( page, label, command );
	}
}

static CSMLPanelInterface g_SMLPanel;
SMLPanel				 *smlmenu = (SMLPanel *)&g_SMLPanel;

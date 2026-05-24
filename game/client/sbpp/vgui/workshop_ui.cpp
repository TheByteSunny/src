//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "workshop_ui.h"
#include "cdll_client_int.h"
#include "webmanager.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "menu/imageextbutton.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/PropertySheet.h>

#ifdef _WIN32
#undef MessageBox
#undef PostMessage
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define WORKSHOP_API_URL "https://workshop-bot.sbpp-workshop.workers.dev/addons.json"

static WorkshopClient  *g_pWorkshopClient = nullptr;
static CWorkshopDialog *g_pWorkshopDialog = nullptr;

static int Scale( int value )
{
	return vgui::scheme()->GetProportionalScaledValue( value );
}

DECLARE_BUILD_FACTORY( CBrowsePage );

WorkshopClient::WorkshopClient( const char *downloadFolder ) : m_downloadFolder( downloadFolder )
{
	filesystem->CreateDirHierarchy( m_downloadFolder.String(), "MOD" );

	m_tempFolder = "downloads/temp";
	filesystem->CreateDirHierarchy( m_tempFolder.String(), "MOD" );
}

WorkshopClient::~WorkshopClient()
{
}

static void SanitizeBrokenJSON( CUtlString &json )
{
	const char *src = json.String();
	int			len = V_strlen( src );

	CUtlBuffer out( 0, len + 64, CUtlBuffer::TEXT_BUFFER );

	bool inString = false;
	bool escape = false;

	for ( int i = 0; i < len; ++i )
	{
		char c = src[i];

		if ( !inString )
		{
			out.PutChar( c );
			if ( c == '"' )
				inString = true;
			continue;
		}

		if ( escape )
		{
			out.PutChar( c );
			escape = false;
			continue;
		}

		if ( c == '\\' )
		{
			out.PutChar( c );
			escape = true;
			continue;
		}

		if ( c == '"' )
		{
			int j = i + 1;
			while ( j < len && ( src[j] == ' ' || src[j] == '\t' || src[j] == '\r' || src[j] == '\n' ) )
				++j;

			char n = ( j < len ) ? src[j] : '\0';
			bool isTerminator = ( n == ',' || n == '}' || n == ']' || n == ':' || n == '\0' );

			if ( isTerminator )
			{
				out.PutChar( '"' );
				inString = false;
			}
			else
			{
				out.PutChar( '\\' );
				out.PutChar( '"' );
			}
			continue;
		}

		out.PutChar( c );
	}

	out.PutChar( '\0' );
	json = (const char *)out.Base();
}

bool WorkshopClient::ParseAddonsJSON( const char *jsonText, CUtlVector< Addon > &outAddons )
{
	if ( !jsonText || !*jsonText )
		return false;

	CUtlString sanitized = jsonText;
	SanitizeBrokenJSON( sanitized );

	rapidjson::Document doc;

	doc.Parse< rapidjson::kParseStopWhenDoneFlag >( sanitized.String() );

	if ( doc.HasParseError() )
	{
		Warning( "Workshop: JSON parse error at offset %zu: %s\n", doc.GetErrorOffset(), rapidjson::GetParseError_En( doc.GetParseError() ) );

		size_t		off = doc.GetErrorOffset();
		const char *src = sanitized.String();
		size_t		len = V_strlen( src );
		size_t		start = off > 40 ? off - 40 : 0;
		size_t		end = MIN( len, off + 40 );
		Warning( "Workshop: near: ...%.*s<<<HERE>>>%.*s...\n", (int)( off - start ), src + start, (int)( end - off ), src + off );
		return false;
	}

	if ( !doc.IsObject() || !doc.HasMember( "addons" ) || !doc["addons"].IsArray() )
	{
		Warning( "Workshop: malformed response (missing addons array)\n" );
		return false;
	}

	const auto &addons = doc["addons"];
	outAddons.EnsureCapacity( addons.Size() );

	auto GetStr = []( const rapidjson::Value &v, const char *key, const char *def = "" ) -> const char *
	{
		if ( !v.HasMember( key ) )
			return def;
		const auto &f = v[key];
		return f.IsString() ? f.GetString() : def;
	};

	auto GetUint64 = []( const rapidjson::Value &v, const char *key, uint64 def = 0 ) -> uint64
	{
		if ( !v.HasMember( key ) )
			return def;
		const auto &f = v[key];
		if ( f.IsUint64() )
			return f.GetUint64();
		if ( f.IsInt64() )
			return (uint64)f.GetInt64();
		if ( f.IsUint() )
			return (uint64)f.GetUint();
		if ( f.IsString() )
			return (uint64)V_atoui64( f.GetString() );
		return def;
	};

	for ( auto it = addons.Begin(); it != addons.End(); ++it )
	{
		if ( !it->IsObject() )
			continue;

		Addon a;
		a.id = GetStr( *it, "id" );
		a.name = GetStr( *it, "name" );
		a.description = GetStr( *it, "description" );
		a.version = GetStr( *it, "version" );
		a.author = GetStr( *it, "author" );
		a.authorId = GetStr( *it, "author_id" );
		a.thumbnailUrl = GetStr( *it, "thumbnail" );
		a.downloadUrl = GetStr( *it, "url" );
		a.sha256 = GetStr( *it, "sha256" );
		a.submittedAt = GetStr( *it, "submitted_at" );
		a.approvedAt = GetStr( *it, "approved_at" );
		a.approvedBy = GetStr( *it, "approved_by" );
		a.size = GetUint64( *it, "size" );

		if ( !a.id.IsEmpty() && !a.name.IsEmpty() )
			outAddons.AddToTail( a );
	}

	DevMsg( "Workshop: Parsed %d addons\n", outAddons.Count() );
	return outAddons.Count() > 0;
}

void WorkshopClient::FetchThumbnails( ThumbnailReadyCallback perItemCb )
{
	char tempAbs[MAX_PATH];
	tempAbs[0] = '\0';

	filesystem->CreateDirHierarchy( m_tempFolder.String(), "MOD" );

	{
		CUtlString sentinel;
		sentinel.Format( "%s/.sentinel", m_tempFolder.String() );
		FileHandle_t fh = filesystem->Open( sentinel.String(), "wb", "MOD" );
		if ( fh != FILESYSTEM_INVALID_HANDLE )
		{
			filesystem->Close( fh );
			char fullSentinel[MAX_PATH];
			if ( filesystem->RelativePathToFullPath( sentinel.String(), "MOD", fullSentinel, sizeof( fullSentinel ) ) )
			{
				V_strncpy( tempAbs, fullSentinel, sizeof( tempAbs ) );
				V_StripFilename( tempAbs );
			}
			filesystem->RemoveFile( sentinel.String(), "MOD" );
		}
	}

	if ( tempAbs[0] == '\0' )
	{
		Warning( "Workshop: Could not resolve temp folder absolute path\n" );
		return;
	}

	for ( int i = 0; i < m_cachedAddons.Count(); ++i )
	{
		Addon &a = m_cachedAddons[i];
		if ( a.thumbnailUrl.IsEmpty() )
			continue;

		CUtlString	cleanUrl = a.thumbnailUrl;
		const char *q = V_strrchr( cleanUrl.String(), '?' );
		if ( q )
			cleanUrl.SetLength( q - cleanUrl.String() );

		const char *ext = V_strrchr( cleanUrl.String(), '.' );
		char		filename[128];
		if ( ext && V_strlen( ext ) <= 5 )
			V_snprintf( filename, sizeof( filename ), "%s%s", a.id.String(), ext );
		else
			V_snprintf( filename, sizeof( filename ), "%s.png", a.id.String() );

		CUtlString relPath;
		relPath.Format( "%s/%s", m_tempFolder.String(), filename );

		if ( g_pFullFileSystem->FileExists( relPath.String(), "MOD" ) )
		{
			a.localThumbPath = relPath;
			if ( perItemCb )
				perItemCb( a.id, relPath );
			continue;
		}

		char absPath[MAX_PATH];
		V_snprintf( absPath, sizeof( absPath ), "%s%c%s", tempAbs, CORRECT_PATH_SEPARATOR, filename );

		CUtlString addonId = a.id;
		CUtlString relPathCpy = relPath;

		g_pWebManager->DownloadToFileAsync( a.thumbnailUrl.String(), absPath,
			[this, addonId, relPathCpy, perItemCb]( bool ok, const char * )
			{
				if ( !ok )
					return;

				for ( int k = 0; k < m_cachedAddons.Count(); ++k )
				{
					if ( !V_stricmp( m_cachedAddons[k].id.String(), addonId.String() ) )
					{
						m_cachedAddons[k].localThumbPath = relPathCpy;
						break;
					}
				}

				if ( perItemCb )
					perItemCb( addonId, relPathCpy );
			} );
	}
}

void WorkshopClient::FetchAddons( AddonListCallback cb )
{
	DevMsg( "Workshop: Fetching addon list...\n" );

	g_pWebManager->Get( WORKSHOP_API_URL,
		[this, cb]( const WebResult_t &r )
		{
			if ( !r.success )
			{
				Warning( "Workshop: Failed to fetch addon list\n" );
				if ( cb )
					cb( false, m_cachedAddons );
				return;
			}

			m_cachedAddons.RemoveAll();
			if ( !ParseAddonsJSON( r.body.c_str(), m_cachedAddons ) )
			{
				if ( cb )
					cb( false, m_cachedAddons );
				return;
			}

			//FetchThumbnails();

			if ( cb )
				cb( true, m_cachedAddons );
		} );
}

void WorkshopClient::DownloadAddon( const Addon &addon, AddonActionCallback cb )
{
	if ( addon.downloadUrl.IsEmpty() )
	{
		if ( cb )
			cb( false, "empty download url" );
		return;
	}

	filesystem->CreateDirHierarchy( m_downloadFolder.String(), "MOD" );

	char folderAbs[MAX_PATH];
	folderAbs[0] = '\0';
	{
		CUtlString sentinel;
		sentinel.Format( "%s/.sentinel", m_downloadFolder.String() );
		FileHandle_t fh = filesystem->Open( sentinel.String(), "wb", "MOD" );
		if ( fh != FILESYSTEM_INVALID_HANDLE )
		{
			filesystem->Close( fh );
			char fullSentinel[MAX_PATH];
			if ( filesystem->RelativePathToFullPath( sentinel.String(), "MOD", fullSentinel, sizeof( fullSentinel ) ) )
			{
				V_strncpy( folderAbs, fullSentinel, sizeof( folderAbs ) );
				V_StripFilename( folderAbs );
			}
			filesystem->RemoveFile( sentinel.String(), "MOD" );
		}
	}

	if ( folderAbs[0] == '\0' )
	{
		if ( cb )
			cb( false, "Could not resolve download folder" );
		return;
	}

	char absPath[MAX_PATH];
	V_snprintf( absPath, sizeof( absPath ), "%s%c%s.zip", folderAbs, CORRECT_PATH_SEPARATOR, addon.id.String() );

	DevMsg( "Workshop: Downloading %s to %s\n", addon.id.String(), absPath );

	if ( !g_pWebManager->DownloadToFile( addon.downloadUrl.String(), absPath ) )
	{
		if ( cb )
			cb( false, "download failed" );
		return;
	}

	DevMsg( "Workshop: Successfully downloaded %s\n", addon.id.String() );
	if ( cb )
		cb( true, nullptr );
}

void WorkshopClient::GetInstalledAddonIDs( CUtlVector< CUtlString > &outIDs )
{
	outIDs.RemoveAll();

	char		searchPattern[512];
	const char *folder = m_downloadFolder.String();
	int			folderLen = V_strlen( folder );
	if ( folderLen > 0 && folder[folderLen - 1] == '/' )
		V_snprintf( searchPattern, sizeof( searchPattern ), "%s*.zip", folder );
	else
		V_snprintf( searchPattern, sizeof( searchPattern ), "%s/*.zip", folder );

	FileFindHandle_t findHandle = 0;
	const char		*found = g_pFullFileSystem->FindFirst( searchPattern, &findHandle );
	while ( found )
	{
		CUtlString	name = found;
		const char *dot = V_strrchr( name.String(), '.' );
		if ( dot )
			name.SetLength( dot - name.String() );

		if ( !name.IsEmpty() )
			outIDs.AddToTail( name );

		found = g_pFullFileSystem->FindNext( findHandle );
	}

	if ( findHandle != 0 )
		g_pFullFileSystem->FindClose( findHandle );
}

bool WorkshopClient::IsAddonInstalled( const char *id )
{
	if ( !id || !*id )
		return false;

	CUtlString path;
	path.Format( "%s/%s.zip", m_downloadFolder.String(), id );
	return g_pFullFileSystem->FileExists( path.String() );
}

bool WorkshopClient::RemoveAddon( const char *id )
{
	if ( !id || !*id )
		return false;

	CUtlString path;
	path.Format( "%s/%s.zip", m_downloadFolder.String(), id );
	if ( g_pFullFileSystem->FileExists( path.String() ) )
	{
		filesystem->RemoveFile( path.String(), "MOD" );
		return true;
	}
	return false;
}

WorkshopClient *GetWorkshopClient()
{
	return g_pWorkshopClient;
}

void InitWorkshopClient( const char *downloadFolder )
{
	if ( g_pWorkshopClient )
	{
		Warning( "Workshop: Client already initialized\n" );
		return;
	}
	g_pWorkshopClient = new WorkshopClient( downloadFolder );
}

void ShutdownWorkshopClient()
{
	if ( g_pWorkshopClient )
	{
		delete g_pWorkshopClient;
		g_pWorkshopClient = nullptr;
	}
}

CAddonThumbnailPanel::CAddonThumbnailPanel( Panel *parent, const char *panelName, const Addon &addon, bool allowSubscribe ) :
	BaseClass( parent, panelName ),
	m_addon( addon ),
	m_bSubscribed( false ),
	m_bHovered( false ),
	m_allowSubscribe( allowSubscribe ),
	m_pImageContainer( nullptr ),
	m_pImageButton( nullptr ),
	m_pCheckbox( nullptr ),
	m_pNameLabel( nullptr ),
	m_pSizeLabel( nullptr )
{
	SetMouseInputEnabled( true );
	SetPaintBackgroundEnabled( true );

	const int padding = 8;
	const int imageW = 200;
	const int imageH = 112;
	const int checkboxSize = 22;
	const int nameH = 18;
	const int sizeH = 14;
	const int textGap = 2;
	const int bottomGap = 6;
	const int bottomPad = 8;

	const int bottomBlockH = nameH + textGap + sizeH;
	const int panelW = imageW + padding * 2;
	const int panelH = padding + imageH + bottomGap + bottomBlockH + bottomPad;

	SetSize( panelW, panelH );

	const char *thumbPath = addon.localThumbPath.IsEmpty() ? "" : addon.localThumbPath.String();

	m_pImageContainer = new Panel( this, "ImageContainer" );
	m_pImageContainer->SetBounds( padding, padding, imageW, imageH );
	m_pImageContainer->SetMouseInputEnabled( false );

	m_pImageButton = new ImageExtButton( m_pImageContainer, "AddonImageBtn", thumbPath, nullptr, nullptr, nullptr );
	m_pImageButton->SetBounds( 0, 0, imageW, imageH );
	m_pImageButton->SetScaleImage( true );
	m_pImageButton->SetMouseInputEnabled( false );

	const int checkX = panelW - padding - checkboxSize;
	const int checkY = panelH - padding - checkboxSize;

	m_pCheckbox = new CheckButton( this, "SubscribeCheck", "" );
	m_pCheckbox->SetBounds( checkX, checkY, checkboxSize, checkboxSize );
	m_pCheckbox->SetMouseInputEnabled( m_allowSubscribe );
	m_pCheckbox->SetVisible( m_allowSubscribe );
	m_pCheckbox->AddActionSignalTarget( this );
	m_pCheckbox->SetText( "" );
	m_pCheckbox->SetPaintBackgroundEnabled( false );
	m_pCheckbox->SetZPos( 10 );

	const int bottomY = padding + imageH + bottomGap;
	const int textX = padding;
	const int rightReserve = m_allowSubscribe ? ( checkboxSize + padding ) : 0;
	const int textW = panelW - padding - textX - rightReserve;

	m_pNameLabel = new Label( this, "AddonName", addon.name.String() );
	m_pNameLabel->SetBounds( textX, bottomY, textW, nameH );
	m_pNameLabel->SetContentAlignment( Label::a_west );
	m_pNameLabel->SetMouseInputEnabled( false );

	char sizeStr[64];
	V_snprintf( sizeStr, sizeof( sizeStr ), "%.2f MB", addon.size / ( 1024.0f * 1024.0f ) );
	m_pSizeLabel = new Label( this, "AddonSize", sizeStr );
	m_pSizeLabel->SetBounds( textX, bottomY + nameH + textGap, textW, sizeH );
	m_pSizeLabel->SetContentAlignment( Label::a_west );
	m_pSizeLabel->SetMouseInputEnabled( false );

	if ( CWorkshopDialog *pDlg = GetWorkshopDialog() )
		AddActionSignalTarget( pDlg );
}

CAddonThumbnailPanel::~CAddonThumbnailPanel()
{
}

void CAddonThumbnailPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( pScheme->GetBorder( "ButtonBorder" ) );
	SetBgColor( pScheme->GetColor( "ListPanel.BgColor", Color( 40, 40, 40, 255 ) ) );

	if ( m_pNameLabel )
		m_pNameLabel->SetFgColor( pScheme->GetColor( "Label.TextColor", Color( 255, 255, 255, 255 ) ) );
	if ( m_pSizeLabel )
		m_pSizeLabel->SetFgColor( pScheme->GetColor( "Label.DisabledTextColor", Color( 150, 150, 150, 255 ) ) );
}

void CAddonThumbnailPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CAddonThumbnailPanel::PaintBackground()
{
	BaseClass::PaintBackground();

	int w, h;
	GetSize( w, h );

	int mx, my;
	input()->GetCursorPos( mx, my );
	ScreenToLocal( mx, my );

	m_bHovered = ( mx >= 0 && mx < w && my >= 0 && my < h );

	if ( m_bHovered )
	{
		surface()->DrawSetColor( Color( 100, 150, 255, 50 ) );
		surface()->DrawFilledRect( 0, 0, w, h );
	}
}

void CAddonThumbnailPanel::SetSubscribed( bool subscribed )
{
	if ( m_bSubscribed == subscribed )
		return;

	m_bSubscribed = subscribed;

	if ( m_pCheckbox && m_pCheckbox->IsSelected() != subscribed )
		m_pCheckbox->SetSelected( subscribed );

	PropertyDialog *pDlg = dynamic_cast< PropertyDialog * >( GetWorkshopDialog() );
	if ( pDlg )
		PostMessage( pDlg, new KeyValues( "ApplyButtonEnable" ) );
}

void CAddonThumbnailPanel::OnMousePressed( MouseCode code )
{
	if ( code != MOUSE_LEFT )
		return;

	int mx, my;
	input()->GetCursorPos( mx, my );
	ScreenToLocal( mx, my );

	if ( m_pCheckbox && m_allowSubscribe )
	{
		int cbX, cbY, cbW, cbH;
		m_pCheckbox->GetBounds( cbX, cbY, cbW, cbH );
		if ( mx >= cbX && mx < cbX + cbW && my >= cbY && my < cbY + cbH )
			return;
	}

	ShowAddonDetails();
}

void CAddonThumbnailPanel::ShowAddonDetails()
{
	CUtlString text;
	text.Format( "Name: %s\n\nDescription:\n%s\n\n", m_addon.name.String(), m_addon.description.IsEmpty() ? "(no description)" : m_addon.description.String() );

	char buf[128];
	V_snprintf( buf, sizeof( buf ), "Size: %.2f MB\n", m_addon.size / ( 1024.0f * 1024.0f ) );
	text.Append( buf );

	if ( !m_addon.version.IsEmpty() )
	{
		text.Append( "Version: " );
		text.Append( m_addon.version.String() );
		text.Append( "\n" );
	}

	if ( !m_addon.author.IsEmpty() )
	{
		text.Append( "By: " );
		text.Append( m_addon.author.String() );
		text.Append( "\n" );
	}

	if ( !m_addon.approvedAt.IsEmpty() )
	{
		text.Append( "Approved: " );
		text.Append( m_addon.approvedAt.String() );
		text.Append( "\n" );
	}

	text.Append( "ID: " );
	text.Append( m_addon.id.String() );

	vgui::MessageBox *pMsg = new vgui::MessageBox( m_addon.name.String(), text.String(), this );
	pMsg->DoModal();
}

void CAddonThumbnailPanel::OnCheckButtonChecked( int state )
{
	if ( m_allowSubscribe )
	{
		SetSubscribed( m_pCheckbox->IsSelected() );
	}
}

CBrowsePage::CBrowsePage( Panel *parent, const char *panelName ) : BaseClass( parent, panelName ), m_pViewport( nullptr ), m_pContentPanel( nullptr ), m_pVScroll( nullptr ), m_lastContentWidth( 0 ), m_contentTall( 0 )
{
	SetProportional( true );

	m_pViewport = new EditablePanel( this, "AddonViewport" );
	m_pContentPanel = new EditablePanel( m_pViewport, "AddonContent" );
	m_pContentPanel->SetAutoDelete( false );

	m_pVScroll = new ScrollBar( this, "BrowseScrollBar", true );
	m_pVScroll->SetVisible( false );
	m_pVScroll->AddActionSignalTarget( this );

	SetBgColor( Color( 30, 30, 30, 255 ) );
}

CBrowsePage::~CBrowsePage()
{
}

void CBrowsePage::OnMouseWheeled( int delta )
{
	if ( !m_pVScroll || !m_pVScroll->IsVisible() )
		return;

	int cur = m_pVScroll->GetValue();
	int step = MAX( 1, m_pVScroll->GetRangeWindow() / 10 );
	cur -= delta * step;

	int mn, mx;
	m_pVScroll->GetRange( mn, mx );
	int window = m_pVScroll->GetRangeWindow();
	cur = clamp( cur, mn, MAX( mn, mx - window ) );
	m_pVScroll->SetValue( cur );

	InvalidateLayout( true );
}

void CBrowsePage::OnScrollBarMoved( KeyValues *pKV )
{
	InvalidateLayout( true );
}

void CBrowsePage::ComputeLayoutPositions()
{
	const int padding = 10;
	const int spacingX = 12;
	const int spacingY = 12;

	int tileW, tileH;
	CAddonThumbnailPanel::GetTileDimensions( tileW, tileH );

	int contentWidth = m_lastContentWidth;
	if ( contentWidth <= 0 )
	{
		m_contentTall = 0;
		m_tileRects.RemoveAll();
		return;
	}

	int usable = contentWidth - padding * 2;
	int columns = ( usable + spacingX ) / ( tileW + spacingX );
	if ( columns < 1 )
		columns = 1;

	m_tileRects.RemoveAll();
	m_tileRects.EnsureCapacity( m_AddonPanels.Count() );

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		int		 col = i % columns;
		int		 row = i / columns;
		TileRect r;
		r.x = padding + col * ( tileW + spacingX );
		r.y = padding + row * ( tileH + spacingY );
		r.w = tileW;
		r.h = tileH;
		m_tileRects.AddToTail( r );
	}

	int rows = ( m_AddonPanels.Count() + columns - 1 ) / columns;
	m_contentTall = padding * 2 + rows * tileH + ( rows > 0 ? ( rows - 1 ) * spacingY : 0 );
}

void CBrowsePage::UpdatePositions()
{
	for ( int i = 0; i < m_AddonPanels.Count() && i < m_tileRects.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->SetBounds( m_tileRects[i].x, m_tileRects[i].y, m_tileRects[i].w, m_tileRects[i].h );
	}
}

void CBrowsePage::PerformLayout()
{
	BaseClass::PerformLayout();

	const int scrollBarW = 16;
	const int padding = 4;

	int w, h;
	GetSize( w, h );
	if ( w <= 10 || h <= 10 )
		return;

	bool needScroll = m_contentTall > ( h - padding * 2 );

	int viewportW = w - padding * 2 - ( needScroll ? scrollBarW + padding : 0 );
	int viewportH = h - padding * 2;

	m_pViewport->SetBounds( padding, padding, viewportW, viewportH );

	int contentWidth = viewportW;

	bool needsRecompute = ( contentWidth != m_lastContentWidth ) || ( m_tileRects.Count() != m_AddonPanels.Count() );

	if ( needsRecompute )
	{
		m_lastContentWidth = contentWidth;
		ComputeLayoutPositions();
	}

	needScroll = m_contentTall > viewportH;

	m_pContentPanel->SetSize( contentWidth, MAX( m_contentTall, viewportH ) );

	UpdatePositions();

	m_pVScroll->SetVisible( needScroll );
	if ( needScroll )
	{
		m_pVScroll->SetBounds( w - scrollBarW - padding, padding, scrollBarW, viewportH );
		m_pVScroll->SetRange( 0, m_contentTall );
		m_pVScroll->SetRangeWindow( viewportH );
		m_pVScroll->SetButtonPressedScrollValue( MAX( 1, viewportH / 10 ) );

		int val = m_pVScroll->GetValue();
		m_pContentPanel->SetPos( 0, -val );
	}
	else
	{
		m_pContentPanel->SetPos( 0, 0 );
	}
}

void CBrowsePage::OnPageShow()
{
	BaseClass::OnPageShow();

	if ( m_AddonPanels.Count() == 0 )
		RefreshList();
}

void CBrowsePage::OnRefreshClicked()
{
	RefreshList();
}

void CBrowsePage::RefreshList()
{
	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->MarkForDeletion();
	}
	m_AddonPanels.RemoveAll();
	m_tileRects.RemoveAll();

	pClient->FetchAddons(
		[this]( bool success, const CUtlVector< Addon > & )
		{
			if ( success )
				PopulateGrid();
		} );
}

void CBrowsePage::PopulateGrid()
{
	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->MarkForDeletion();
	}
	m_AddonPanels.RemoveAll();

	const CUtlVector< Addon > &addons = pClient->GetCachedAddons();

	CUtlVector< CUtlString > installed;
	pClient->GetInstalledAddonIDs( installed );

	for ( int i = 0; i < addons.Count(); ++i )
	{
		CAddonThumbnailPanel *pPanel = new CAddonThumbnailPanel( m_pContentPanel, "AddonTile", addons[i], true );

		bool isInstalled = false;
		for ( int j = 0; j < installed.Count(); ++j )
		{
			if ( !V_stricmp( installed[j].String(), addons[i].id.String() ) )
			{
				isInstalled = true;
				break;
			}
		}
		pPanel->SetSubscribed( isInstalled );
		m_AddonPanels.AddToTail( pPanel );
	}

	m_tileRects.RemoveAll();

	InvalidateLayout( true, false );

	VPANEL hSelf = GetVPanel();

	pClient->FetchThumbnails(
		[ hSelf ]( const CUtlString &id, const CUtlString &path )
		{
			KeyValues *kv = new KeyValues( "ThumbReady" );
			kv->SetString( "id",   id.String() );
			kv->SetString( "path", path.String() );
			vgui::ivgui()->PostMessage( hSelf, kv, NULL );
		} );
}

void CBrowsePage::OnThumbReady( KeyValues *kv )
{
	const char *id = kv->GetString( "id" );
	const char *path = kv->GetString( "path" );

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		if ( m_AddonPanels[i] && !V_stricmp( m_AddonPanels[i]->GetAddon().id.String(), id ) )
		{
			m_AddonPanels[i]->SetThumbnailPath( path );
			break;
		}
	}
}

void CBrowsePage::ApplySubscriptionChanges()
{
	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		CAddonThumbnailPanel *pTile = m_AddonPanels[i];
		if ( !pTile )
			continue;

		const Addon &addon = pTile->GetAddon();
		bool		 wantSubscribed = pTile->IsSubscribed();
		bool		 isInstalled = pClient->IsAddonInstalled( addon.id.String() );

		if ( wantSubscribed && !isInstalled )
		{
			pClient->DownloadAddon( addon,
				[]( bool ok, const char *err )
				{
					if ( !ok )
						Warning( "Workshop: download failed: %s\n", err ? err : "(unknown)" );
				} );
		}
		else if ( !wantSubscribed && isInstalled )
		{
			pClient->RemoveAddon( addon.id.String() );
		}
	}

	if ( CWorkshopDialog *pDlg = GetWorkshopDialog() )
	{
		if ( pDlg->m_pSubscribedPage )
			pDlg->m_pSubscribedPage->RefreshList();
	}
}

CSubscribedPage::CSubscribedPage( Panel *parent, const char *panelName ) : BaseClass( parent, panelName ), m_pViewport( nullptr ), m_pContentPanel( nullptr ), m_pVScroll( nullptr ), m_lastContentWidth( 0 ), m_contentTall( 0 )
{
	SetProportional( true );

	m_pViewport = new EditablePanel( this, "AddonViewport" );
	m_pContentPanel = new EditablePanel( m_pViewport, "AddonContent" );
	m_pContentPanel->SetAutoDelete( false );

	m_pVScroll = new ScrollBar( this, "BrowseScrollBar", true );
	m_pVScroll->SetVisible( false );
	m_pVScroll->AddActionSignalTarget( this );

	SetBgColor( Color( 30, 30, 30, 255 ) );
}

CSubscribedPage::~CSubscribedPage()
{
}

void CSubscribedPage::OnMouseWheeled( int delta )
{
	if ( !m_pVScroll || !m_pVScroll->IsVisible() )
		return;

	int cur = m_pVScroll->GetValue();
	int step = MAX( 1, m_pVScroll->GetRangeWindow() / 10 );
	cur -= delta * step;

	int mn, mx;
	m_pVScroll->GetRange( mn, mx );
	int window = m_pVScroll->GetRangeWindow();
	cur = clamp( cur, mn, MAX( mn, mx - window ) );
	m_pVScroll->SetValue( cur );

	InvalidateLayout( true );
}

void CSubscribedPage::OnScrollBarMoved( KeyValues *pKV )
{
	InvalidateLayout( true );
}

void CSubscribedPage::ComputeLayoutPositions()
{
	const int padding = 10;
	const int spacingX = 12;
	const int spacingY = 12;

	int tileW, tileH;
	CAddonThumbnailPanel::GetTileDimensions( tileW, tileH );

	int contentWidth = m_lastContentWidth;
	if ( contentWidth <= 0 )
	{
		m_contentTall = 0;
		m_tileRects.RemoveAll();
		return;
	}

	int usable = contentWidth - padding * 2;
	int columns = ( usable + spacingX ) / ( tileW + spacingX );
	if ( columns < 1 )
		columns = 1;

	m_tileRects.RemoveAll();
	m_tileRects.EnsureCapacity( m_AddonPanels.Count() );

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		int		 col = i % columns;
		int		 row = i / columns;
		TileRect r;
		r.x = padding + col * ( tileW + spacingX );
		r.y = padding + row * ( tileH + spacingY );
		r.w = tileW;
		r.h = tileH;
		m_tileRects.AddToTail( r );
	}

	int rows = ( m_AddonPanels.Count() + columns - 1 ) / columns;
	m_contentTall = padding * 2 + rows * tileH + ( rows > 0 ? ( rows - 1 ) * spacingY : 0 );
}

void CSubscribedPage::UpdatePositions()
{
	for ( int i = 0; i < m_AddonPanels.Count() && i < m_tileRects.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->SetBounds( m_tileRects[i].x, m_tileRects[i].y, m_tileRects[i].w, m_tileRects[i].h );
	}
}

void CSubscribedPage::PerformLayout()
{
	BaseClass::PerformLayout();

	const int scrollBarW = 16;
	const int padding = 4;

	int w, h;
	GetSize( w, h );
	if ( w <= 10 || h <= 10 )
		return;

	bool needScroll = m_contentTall > ( h - padding * 2 );

	int viewportW = w - padding * 2 - ( needScroll ? scrollBarW + padding : 0 );
	int viewportH = h - padding * 2;

	m_pViewport->SetBounds( padding, padding, viewportW, viewportH );

	int contentWidth = viewportW;

	bool needsRecompute = ( contentWidth != m_lastContentWidth ) || ( m_tileRects.Count() != m_AddonPanels.Count() );

	if ( needsRecompute )
	{
		m_lastContentWidth = contentWidth;
		ComputeLayoutPositions();
	}

	needScroll = m_contentTall > viewportH;

	m_pContentPanel->SetSize( contentWidth, MAX( m_contentTall, viewportH ) );

	UpdatePositions();

	m_pVScroll->SetVisible( needScroll );
	if ( needScroll )
	{
		m_pVScroll->SetBounds( w - scrollBarW - padding, padding, scrollBarW, viewportH );
		m_pVScroll->SetRange( 0, m_contentTall );
		m_pVScroll->SetRangeWindow( viewportH );
		m_pVScroll->SetButtonPressedScrollValue( MAX( 1, viewportH / 10 ) );

		int val = m_pVScroll->GetValue();
		m_pContentPanel->SetPos( 0, -val );
	}
	else
	{
		m_pContentPanel->SetPos( 0, 0 );
	}
}

void CSubscribedPage::OnPageShow()
{
	BaseClass::OnPageShow();
	RefreshList();
	InvalidateLayout( true, true );
}

void CSubscribedPage::OnRefreshClicked()
{
	RefreshList();
	InvalidateLayout( true, true );
}

void CSubscribedPage::RefreshList()
{
	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->MarkForDeletion();
	}
	m_AddonPanels.RemoveAll();
	m_tileRects.RemoveAll();

	pClient->FetchAddons(
		[this]( bool success, const CUtlVector< Addon > & )
		{
			if ( success )
				PopulateGrid();
		} );
}

void CSubscribedPage::PopulateGrid()
{
	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	for ( int i = 0; i < m_AddonPanels.Count(); ++i )
	{
		if ( m_AddonPanels[i] )
			m_AddonPanels[i]->MarkForDeletion();
	}
	m_AddonPanels.RemoveAll();

	CUtlVector< CUtlString > installed;
	pClient->GetInstalledAddonIDs( installed );

	const CUtlVector< Addon > &addons = pClient->GetCachedAddons();

	for ( int i = 0; i < addons.Count(); ++i )
	{
		bool isInstalled = false;
		for ( int j = 0; j < installed.Count(); ++j )
		{
			if ( !V_stricmp( installed[j].String(), addons[i].id.String() ) )
			{
				isInstalled = true;
				break;
			}
		}
		if ( !isInstalled )
			continue;

		CAddonThumbnailPanel *pPanel = new CAddonThumbnailPanel( m_pContentPanel, "SubAddonTile", addons[i], false );
		pPanel->SetSubscribed( true );
		m_AddonPanels.AddToTail( pPanel );
	}

	m_tileRects.RemoveAll();
	InvalidateLayout( true, false );
}

CWorkshopDialog::CWorkshopDialog( Panel *parent ) : BaseClass( parent, "WorkshopDialog" ), m_pBrowsePage( nullptr ), m_pSubscribedPage( nullptr )
{
	SetTitle( "#SBPP_Workshop_Title", true );
	SetSize( 850, 650 );

	SetSizeable( true );
	SetMoveable( true );
	SetCloseButtonVisible( true );
	SetApplyButtonVisible( true );

	m_pBrowsePage = new CBrowsePage( this, "BrowsePage" );
	m_pSubscribedPage = new CSubscribedPage( this, "SubscribedPage" );

	AddPage( m_pBrowsePage, "#SBPP_BrowseAddons" );
	AddPage( m_pSubscribedPage, "#SBPP_Subscribed" );

	//GetPropertySheet()->InvalidateLayout( true, true );
}

CWorkshopDialog::~CWorkshopDialog()
{
	g_pWorkshopDialog = nullptr;
}

void CWorkshopDialog::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CWorkshopDialog::ApplyChanges()
{
	BaseClass::ApplyChanges();

	WorkshopClient *pClient = GetWorkshopClient();
	if ( !pClient )
		return;

	if ( m_pBrowsePage )
		m_pBrowsePage->ApplySubscriptionChanges();
}

void CWorkshopDialog::OnCommand( const char *cmd )
{
	if ( !V_stricmp( cmd, "OK" ) || !V_stricmp( cmd, "Apply" ) )
	{
		if ( m_pBrowsePage )
			m_pBrowsePage->ApplySubscriptionChanges();
	}
	BaseClass::OnCommand( cmd );
}

void CWorkshopDialog::Activate()
{
	BaseClass::Activate();
	MoveToCenterOfScreen();
}

CWorkshopDialog *GetWorkshopDialog()
{
	if ( !g_pWorkshopDialog )
		g_pWorkshopDialog = new CWorkshopDialog( nullptr );
	return g_pWorkshopDialog;
}

void ShowWorkshop()
{
	CWorkshopDialog *pDialog = GetWorkshopDialog();
	if ( pDialog )
		pDialog->Activate();
}

CON_COMMAND( sbpp_workshop, "Open the SBPP workshop" )
{
	if ( !GetWorkshopClient() )
	{
		Warning( "Workshop: client not initialized.\n" );
		return;
	}
	ShowWorkshop();
}

class CWorkshopGameSystem : public CAutoGameSystem
{
public:
	CWorkshopGameSystem()
	{
	}
	virtual ~CWorkshopGameSystem()
	{
	}

	virtual bool Init() override
	{
		InitWorkshopClient( "addons/" );
		return true;
	}

	virtual void Shutdown() override
	{
		ShutdownWorkshopClient();
	}
};
static CWorkshopGameSystem g_WorkshopSystem;
//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "spawnmenu_new.h"
#include <vgui/IVGui.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>
#include <unordered_map>
#include <vector>
#include <string>
#include "filesystem.h"
#include <vgui_controls/ScrollBar.h>
#include <matsys_controls/mdlpanel.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/TextEntry.h>
#include <algorithm>
#include "fmtstr.h"
#include "stb_image.h"
#ifdef LUA_SDK
#include "luamanager.h"
#include "luasrclib.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sv_spawnmenu_allowed;

using namespace vgui;

// 6 spaces
#define STARTER_SPACES_TAB "      "

ConVar cl_showspawnmenu( "cl_showspawnmenu", "0", FCVAR_CLIENTDLL, "Sets the state of spawn menu" );

class CPngPropertySheet : public PropertySheet
{
	DECLARE_CLASS_SIMPLE( CPngPropertySheet, PropertySheet );

public:
	CPngPropertySheet( Panel *parent, const char *panelName, bool draggableTabs = true );
	~CPngPropertySheet();

	void AddPageWithPNGIcon( Panel *page, const char *title, const char *pngPath, bool showContextMenu = false );
	bool SetTabPNGIcon( int tabIndex, const char *pngPath );

	int LoadPNGTexture( const char *pngPath );

protected:
	virtual void PerformLayout() OVERRIDE;
	virtual void PaintBackground() OVERRIDE;
	virtual void Paint() OVERRIDE;

private:
	struct TabIconInfo
	{
		int			textureId;
		int			width;
		int			height;
		std::string path;
	};

	std::unordered_map< int, TabIconInfo > m_tabIcons;
	std::unordered_map< std::string, int > m_textureCache;
	std::unordered_map< int, Panel * >	   m_tabIconPanels;

	void PremultiplyAlpha( unsigned char *data, int w, int h );
};

class CTabIconPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( CTabIconPanel, Panel );

public:
	CTabIconPanel( Panel *parent, const char *name, int texId = -1 ) : Panel( parent, name ), m_texId( texId )
	{
		SetMouseInputEnabled( false );
	}

	void SetTextureId( int id )
	{
		m_texId = id;
		InvalidateLayout();
		Repaint();
	}

	virtual void Paint() OVERRIDE
	{
		if ( m_texId == -1 )
			return;

		int w = GetWide();
		int h = GetTall();
		if ( w <= 0 || h <= 0 )
			return;

		surface()->DrawSetTexture( m_texId );
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawTexturedRect( 0, 0, w, h );
	}

private:
	int m_texId;
};

CPngPropertySheet::CPngPropertySheet( Panel *parent, const char *panelName, bool draggableTabs ) : BaseClass( parent, panelName, draggableTabs )
{
}

CPngPropertySheet::~CPngPropertySheet()
{
	for ( auto &kv : m_textureCache )
	{
		if ( kv.second != -1 )
		{
			surface()->DestroyTextureID( kv.second );
		}
	}
	m_textureCache.clear();
	m_tabIcons.clear();
}

void CPngPropertySheet::PremultiplyAlpha( unsigned char *data, int w, int h )
{
	const int pixels = w * h;
	for ( int i = 0; i < pixels; ++i )
	{
		unsigned char *p = data + ( i * 4 );
		unsigned int   a = p[3];
		if ( a == 255 || a == 0 )
			continue;
		p[0] = (unsigned char)( ( p[0] * a ) / 255 );
		p[1] = (unsigned char)( ( p[1] * a ) / 255 );
		p[2] = (unsigned char)( ( p[2] * a ) / 255 );
	}
}

static inline int NextPowerOfTwo( int v )
{
	if ( v <= 0 )
		return 1;
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

static unsigned char *ResizeNearestRGBA( const unsigned char *src, int srcW, int srcH, int dstW, int dstH )
{
	if ( !src || srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0 )
		return nullptr;

	unsigned char *dst = new unsigned char[dstW * dstH * 4];
	for ( int y = 0; y < dstH; ++y )
	{
		// map dst y to src y
		int srcY = ( y * srcH ) / dstH;
		if ( srcY < 0 )
			srcY = 0;
		if ( srcY >= srcH )
			srcY = srcH - 1;

		for ( int x = 0; x < dstW; ++x )
		{
			int srcX = ( x * srcW ) / dstW;
			if ( srcX < 0 )
				srcX = 0;
			if ( srcX >= srcW )
				srcX = srcW - 1;

			const unsigned char *s = src + ( srcY * srcW + srcX ) * 4;
			unsigned char		*d = dst + ( y * dstW + x ) * 4;
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
		}
	}
	return dst;
}

int CPngPropertySheet::LoadPNGTexture(const char* pngPath)
{
    if (!pngPath || pngPath[0] == '\0') 
        return -1;

    auto it = m_textureCache.find(pngPath);
    if (it != m_textureCache.end())
        return it->second;

    const char *ext = strrchr(pngPath, '.');
    if (!ext) ext = "";

    if (_stricmp(ext, ".vtf") == 0 || _stricmp(ext, ".vmt") == 0)
    {
        // strip ext
        char pathNoExt[MAX_PATH];
        Q_strncpy(pathNoExt, pngPath, sizeof(pathNoExt));
        int extLen = Q_strlen(ext);
        pathNoExt[Q_strlen(pathNoExt) - extLen] = '\0';
        
        int texId = surface()->DrawGetTextureId(pathNoExt);
        if (texId == -1)
        {
            texId = surface()->CreateNewTextureID();
            surface()->DrawSetTextureFile(texId, pathNoExt, false, true);
        }

        m_textureCache[pngPath] = texId;
        DevMsg("CPngPropertySheet: Loaded material '%s' -> ID %d\n", pathNoExt, texId);
        return texId;
    }

    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(pngPath, "MOD", buf))
    {
        Warning("CPngPropertySheet: failed to open '%s'\n", pngPath);
        return -1;
    }

    int bufSize = buf.TellPut();
    if (bufSize <= 0)
    {
        Warning("CPngPropertySheet: empty file '%s'\n", pngPath);
        return -1;
    }

    int channels = 0;
    int width = 0, height = 0;
    unsigned char* image = stbi_load_from_memory(
        reinterpret_cast<unsigned char*>(buf.Base()),
        bufSize,
        &width,
        &height,
        &channels,
        4
    );

    if (!image)
    {
        Warning("CPngPropertySheet: failed to decode '%s' - %s\n", pngPath, stbi_failure_reason());
        return -1;
    }

    const int MAX_POT = 4096;
    int potW = NextPowerOfTwo(width);
    int potH = NextPowerOfTwo(height);
    if (potW > MAX_POT) potW = MAX_POT;
    if (potH > MAX_POT) potH = MAX_POT;

    unsigned char* finalImage = image;
    int finalW = width;
    int finalH = height;
    bool resized = false;

    if (potW != width || potH != height)
    {
        unsigned char* resizedBuf = ResizeNearestRGBA(image, width, height, potW, potH);
        if (resizedBuf)
        {
            stbi_image_free(image);
            finalImage = resizedBuf;
            finalW = potW;
            finalH = potH;
            resized = true;
        }
        else
        {
            finalImage = image;
            finalW = width;
            finalH = height;
        }
    }

    PremultiplyAlpha(finalImage, finalW, finalH);

    int texId = surface()->CreateNewTextureID(true);
    surface()->DrawSetTextureRGBA(texId, finalImage, finalW, finalH, 1, false);

    if (resized && finalImage)
    {
        delete[] finalImage;
    }
    else
    {
        stbi_image_free(image);
    }

	m_textureCache[pngPath] = texId;
    
    DevMsg("CPngPropertySheet: Loaded texture '%s' (%dx%d -> %dx%d) -> ID %d\n", pngPath, width, height, finalW, finalH, texId);
    
    return texId;
}

void CPngPropertySheet::AddPageWithPNGIcon( Panel *page, const char *title, const char *pngPath, bool showContextMenu )
{
	int pageIndex = GetNumPages();
	AddPage( page, title, NULL, showContextMenu );

	if ( pngPath && pngPath[0] != '\0' )
	{
		int texId = LoadPNGTexture( pngPath );
		if ( texId != -1 )
		{
			TabIconInfo info;
			info.textureId = texId;
			info.width = 16;
			info.height = 16;
			info.path = pngPath;

			m_tabIcons[pageIndex] = info;
		}
	}
}

bool CPngPropertySheet::SetTabPNGIcon( int tabIndex, const char *pngPath )
{
	if ( tabIndex < 0 || tabIndex >= GetNumPages() )
		return false;

	if ( !pngPath || pngPath[0] == '\0' )
	{
		m_tabIcons.erase( tabIndex );
		return true;
	}

	int texId = LoadPNGTexture( pngPath );
	if ( texId == -1 )
		return false;

	TabIconInfo info;
	info.textureId = texId;
	info.width = 16;
	info.height = 16;
	info.path = pngPath;

	m_tabIcons[tabIndex] = info;
	return true;
}

void CPngPropertySheet::PerformLayout()
{
	BaseClass::PerformLayout();

	int numPages = GetNumPages();
	if ( numPages == 0 )
		return;

	struct TabCandidate
	{
		Panel *p;
		int	   x, y, w, h;
	};
	CUtlVector< TabCandidate > candidates;

	for ( int j = 0; j < GetChildCount(); ++j )
	{
		Panel *child = GetChild( j );
		if ( !child )
			continue;

		int cx, cy, cw, ch;
		child->GetBounds( cx, cy, cw, ch );
		if ( cy < 80 && ch < 80 && cw > 20 && cw < GetWide() )
		{
			TabCandidate t;
			t.p = child;
			t.x = cx;
			t.y = cy;
			t.w = cw;
			t.h = ch;
			candidates.AddToTail( t );
		}
	}

	if ( candidates.Count() == 0 )
		return;

	std::sort( candidates.Base(), candidates.Base() + candidates.Count(), []( const TabCandidate &a, const TabCandidate &b ) { return a.x < b.x; } );

	std::vector< int > toRemove;
	for ( auto &kv : m_tabIconPanels )
	{
		if ( kv.first >= numPages )
			toRemove.push_back( kv.first );
	}
	for ( int idx : toRemove )
	{
		auto it = m_tabIconPanels.find(idx);
		Panel *p = (it != m_tabIconPanels.end()) ? it->second : nullptr;
		if ( p )
		{
			p->SetParent( (Panel *)NULL );
			delete p;
		}
		
		m_tabIconPanels.erase( idx );
	}

	for ( int pageIndex = 0; pageIndex < numPages; ++pageIndex )
	{
		auto itIcon = m_tabIcons.find( pageIndex );
		if ( itIcon == m_tabIcons.end() || itIcon->second.textureId == -1 )
			continue;

		if ( pageIndex >= candidates.Count() )
			continue;

		Panel *tabButton = candidates[pageIndex].p;
		int	   tabW = candidates[pageIndex].w;
		int	   tabH = candidates[pageIndex].h;

		Panel *iconPanel = nullptr;
		auto   itPanel = m_tabIconPanels.find( pageIndex );
		if ( itPanel == m_tabIconPanels.end() || itPanel->second->GetParent() != tabButton )
		{
			// if old exists, delete it
			if ( itPanel != m_tabIconPanels.end() )
			{
				Panel *old = itPanel->second;
				if ( old )
				{
					old->SetParent( (Panel *)NULL );
					old->MarkForDeletion();
				}
				m_tabIconPanels.erase( itPanel );
			}

			CTabIconPanel *newIcon = new CTabIconPanel( tabButton, "TabIconPanel", itIcon->second.textureId );
			newIcon->SetMouseInputEnabled( false );
			newIcon->SetZPos( 200 );
			m_tabIconPanels[pageIndex] = newIcon;
			iconPanel = newIcon;
		}
		else
		{
			iconPanel = m_tabIconPanels[pageIndex];
			if ( CTabIconPanel *cip = dynamic_cast< CTabIconPanel * >( iconPanel ) )
				cip->SetTextureId( itIcon->second.textureId );
		}

		const int iconSize = itIcon->second.width;
		const int iconPadding = 6;
		int		  iconX = iconPadding;
		int		  iconY = ( tabH - iconSize ) / 2;
		iconPanel->SetBounds( iconX, iconY, iconSize, iconSize );
	}
}

void CPngPropertySheet::Paint()
{
	BaseClass::Paint();

	int numPages = GetNumPages();
	if ( numPages == 0 )
		return;

	struct TabCandidate
	{
		Panel *p;
		int	   x, y, w, h;
	};
	CUtlVector< TabCandidate > candidates;

	for ( int j = 0; j < GetChildCount(); ++j )
	{
		Panel *child = GetChild( j );
		if ( !child )
			continue;

		int cx, cy, cw, ch;
		child->GetBounds( cx, cy, cw, ch );

		if ( cy < 50 && ch < 60 && cw > 20 && cw < GetWide() )
		{
			TabCandidate t;
			t.p = child;
			t.x = cx;
			t.y = cy;
			t.w = cw;
			t.h = ch;
			candidates.AddToTail( t );
		}
	}

	if ( candidates.Count() == 0 )
		return;

	std::sort( candidates.Base(), candidates.Base() + candidates.Count(), []( const TabCandidate &a, const TabCandidate &b ) { return a.x < b.x; } );

	for ( int pageIndex = 0; pageIndex < numPages; ++pageIndex )
	{
		auto it = m_tabIcons.find( pageIndex );
		if ( it == m_tabIcons.end() || it->second.textureId == -1 )
			continue;

		if ( pageIndex >= candidates.Count() )
			continue;

		TabCandidate &tab = candidates[pageIndex];

		const int iconSize = 16;
		const int iconPadding = 6;
		int		  iconX = tab.x + iconPadding;
		int		  iconY = tab.y + ( tab.h - iconSize ) / 2;

		surface()->DrawSetTexture( it->second.textureId );
		int	 activeIndex = GetActivePageNum();
		bool isActive = ( pageIndex == activeIndex );

		if ( isActive )
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		else
			surface()->DrawSetColor( Color( 200, 200, 200, 200 ) );

		surface()->DrawTexturedRect( iconX, iconY, iconX + iconSize, iconY + iconSize );
	}
}

void CPngPropertySheet::PaintBackground()
{
	BaseClass::PaintBackground();
}

class CStaticMDLPanel : public CMDLPanel
{
	DECLARE_CLASS_SIMPLE( CStaticMDLPanel, CMDLPanel );

public:
	CStaticMDLPanel( vgui::Panel *parent, const char *panelName, const char *command ) : CMDLPanel( parent, panelName )
	{
		m_command = command;
	}

	virtual void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
	{
	}
	virtual void OnKeyCodeReleased( vgui::KeyCode code ) OVERRIDE
	{
	}
	virtual void OnMousePressed( vgui::MouseCode code ) OVERRIDE
	{
		engine->ClientCmd_Unrestricted( m_command.Get() );
	}
	virtual void OnMouseReleased( vgui::MouseCode code ) OVERRIDE
	{
	}
	virtual void OnMouseCaptureLost() OVERRIDE
	{
	}
	virtual void OnTick() OVERRIDE
	{
		if ( IsCursorOver() )
			SetCursor( vgui::dc_hand );
		else
			SetCursor( vgui::dc_arrow );
	}

private:
	CUtlString m_command;
};

class CModelButton : public Button
{
	DECLARE_CLASS_SIMPLE( CModelButton, Button );

public:
	CModelButton( Panel *parent, const char *panelName, const char *modelPath, const char *displayName, const char *command, int hoverTexId = -1 ) :
		Button( parent, panelName, "" ),
		m_modelPath( modelPath ),
		m_command( command ),
		m_bSelected( false ),
		m_hoverTexId( hoverTexId ),
		m_bModelLoaded( false ),
		m_flYaw( 0.0f )
	{
		m_displayName = strdup( displayName );

		m_pModelPanel = new CStaticMDLPanel( this, "ModelPanel", command );
		m_bModelApplied = false;
		m_flLoadTime = gpGlobals->curtime;

		if ( BaseTooltip *pTooltip = m_pModelPanel->GetTooltip() )
			pTooltip->SetText( displayName );

		vgui::ivgui()->AddTickSignal( GetVPanel(), 50 );

		m_bCameraStored = false;
	}

	void ApplyModelIfNeeded()
	{
		if ( m_bModelApplied || !m_modelPath.Get() || m_modelPath.Get()[0] == '\0' )
			return;

		if ( g_pFullFileSystem && !g_pFullFileSystem->FileExists( m_modelPath.Get() ) )
		{
			DevMsg( "CModelButton: model file not found: %s\n", m_modelPath.Get() );
			return;
		}

		m_pModelPanel->SetMDL( m_modelPath.Get() );
		m_pModelPanel->SetLOD( -1 );
		m_flLoadTime = gpGlobals->curtime;
		m_bModelApplied = true;
	}

	virtual void OnTick() OVERRIDE
	{
		BaseClass::OnTick();

		if ( !m_bModelApplied )
			ApplyModelIfNeeded();

		if ( !m_bModelLoaded )
		{
			m_pModelPanel->LookAtMDL();
			m_pModelPanel->SetCameraFOV( 35.f );
			m_pModelPanel->SetLockView( false );
			m_bModelLoaded = true;
		}

		if ( m_bModelLoaded && !m_bCameraStored )
		{
			m_pModelPanel->GetCameraPositionAndAngles( m_vecCamPos, m_angCamDir );
			m_bCameraStored = true;
		}

		if ( m_bCameraStored && ( IsCursorOver() || IsSelected() ) )
		{
			m_flYaw += 2.0f;
			if ( m_flYaw >= 360.0f )
				m_flYaw -= 360.0f;

			QAngle newAngles = m_angCamDir;
			newAngles.y = m_flYaw;

			m_pModelPanel->SetCameraPositionAndAngles( m_vecCamPos, newAngles, false );
		}
		else if ( m_bCameraStored )
		{
			m_pModelPanel->SetCameraPositionAndAngles( m_vecCamPos, m_angCamDir, false );
			m_flYaw = m_angCamDir.y;
		}
	}

	virtual void PerformLayout() OVERRIDE
	{
		int w, h;
		GetSize( w, h );
		m_pModelPanel->SetBounds( 0, 0, w, h );
	}

	virtual void Paint() OVERRIDE
	{
		Button::Paint();

		int w, h;
		GetSize( w, h );

		if ( ( IsCursorOver() || IsSelected() ) && m_hoverTexId != -1 )
		{
			surface()->DrawSetTexture( m_hoverTexId );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			surface()->DrawTexturedRect( 0, 0, w, h );
		}
	}

	virtual void OnMousePressed( MouseCode code ) OVERRIDE
	{
		m_bSelected = !m_bSelected;
	}

public:
	CStaticMDLPanel *m_pModelPanel;
	CUtlString		 m_modelPath;
	CUtlString		 m_displayName;
	CUtlString		 m_command;
	bool			 m_bSelected;
	bool			 m_bModelLoaded;
	bool			 m_bCameraStored;
	float			 m_flLoadTime;
	float			 m_flYaw;
	Vector			 m_vecCamPos;
	QAngle			 m_angCamDir;
	int				 m_hoverTexId = -1;
	bool			 m_bModelApplied = false;
};

class CSearchPanel : public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CSearchPanel, EditablePanel );

public:
	CSearchPanel( Panel *parent, const char *name ) : BaseClass( parent, name )
	{
		m_pSearchEntry = new TextEntry( this, "SearchEntry" );
		m_pSearchEntry->AddActionSignalTarget( this );
		isTypingShit = false;
	}

	MESSAGE_FUNC( TextChanged, "TextChanged" )
	{
		isTypingShit = true;
		char textBuffer[2048];
		m_pSearchEntry->GetText( textBuffer, sizeof( textBuffer ) );
		PostActionSignal( new KeyValues( "SearchTextChanged", "text", textBuffer ) );
	}

	virtual void PerformLayout() OVERRIDE
	{
		BaseClass::PerformLayout();

		int w = GetWide();
		int h = GetTall();
		int searchHeight = max( 20.f, h * 0.02f );

		m_pSearchEntry->SetBounds( 0, 0, w, searchHeight );
	}
	virtual void Paint() OVERRIDE
	{
		surface()->DrawSetColor( 255, 255, 255, 255 );
		surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
	}

	TextEntry *m_pSearchEntry;

	bool isTypingShit;
};

static CUtlString NormalizeFloatDisplayStringBecauseTheLastMethodWasWayTooDirtyYesThisIsTheActualFunctionName( const char *inStr, bool dropLeadingZero = true )
{
	if ( !inStr || !inStr[0] )
		return CUtlString( "" );

	std::string s( inStr );

	auto pos = s.find( '.' );
	if ( pos == std::string::npos )
		return CUtlString( s.c_str() );

	while ( !s.empty() && s.back() == '0' )
		s.pop_back();

	if ( !s.empty() && s.back() == '.' )
		s.pop_back();

	if ( dropLeadingZero && s.size() >= 2 && s[0] == '0' && s[1] == '.' )
		s.erase( 0, 1 );

	return CUtlString( s.c_str() );
}

class CImageButton : public Button
{
	DECLARE_CLASS_SIMPLE( CImageButton, Button );

public:
	CImageButton( Panel *parent, const char *panelName, int texId, const char *displayName, const char *command, int hoverTexId = -1, int contenticonid = -1, int contenticonhoverid = -1 ) :
		Button( parent, panelName, "" ),
		m_texId( texId ),
		m_hoverTexId( hoverTexId ),
		m_contenticon( contenticonid ),
		m_contenticon_hov( contenticonhoverid )
	{
		m_command = command ? command : "";
		m_displayName = NormalizeFloatDisplayStringBecauseTheLastMethodWasWayTooDirtyYesThisIsTheActualFunctionName( displayName ? displayName : "", /*dropLeadingZero*/ true );

		// tooltip
		if ( BaseTooltip *pTooltip = GetTooltip() )
			pTooltip->SetText( m_displayName.Get() );

		m_pName = new Label( this, "Name", displayName );

		SetMouseInputEnabled( true );
	}

	virtual void PerformLayout() OVERRIDE
	{
		m_pName->SizeToContents();

		int labelWide, labelTall;
		m_pName->GetSize( labelWide, labelTall );

		int xPos = ( GetWide() - labelWide ) / 2;
		int yPos = GetTall() - labelTall - 8;

		m_pName->SetPos( xPos, yPos );
	}

	virtual void ApplySchemeSettings( IScheme *pScheme ) OVERRIDE
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_pName->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pName->SetContentAlignment( Label::a_center );
		m_pName->SizeToContents(); // ahh
	}

	virtual void Paint() OVERRIDE
	{
		Button::Paint();

		int w, h;
		GetSize( w, h );

		if ( m_texId != -1 )
		{
			int shrink = 6;
			surface()->DrawSetTexture( m_texId );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

			surface()->DrawTexturedRect( shrink, shrink, w - shrink, h - shrink );
		}

		if ( ( IsCursorOver() || IsSelected() ) && m_hoverTexId != -1 )
		{
			int shrink = 2;
			surface()->DrawSetTexture( m_contenticon_hov );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

			surface()->DrawTexturedRect( shrink, shrink, w - shrink, h - shrink );

#if 0
			surface()->DrawSetTexture(m_hoverTexId);
			surface()->DrawSetColor(Color(255,255,255,255));
			surface()->DrawTexturedRect(0, 0, w, h);
#endif

			SetCursor( vgui::dc_hand );
		}
		else
		{
			int shrink = 2;
			surface()->DrawSetTexture( m_contenticon );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

			surface()->DrawTexturedRect( shrink, shrink, w - shrink, h - shrink );

			SetCursor( vgui::dc_arrow );
		}
	}

	virtual void OnMousePressed( MouseCode code ) OVERRIDE
	{
		if ( m_command.Get() )
			engine->ClientCmd_Unrestricted( m_command.Get() );
	}

private:
	int		   m_texId = -1;
	int		   m_hoverTexId = -1;
	int		   m_contenticon = -1;
	int		   m_contenticon_hov = -1;
	CUtlString m_displayName;
	CUtlString m_command;

	Label *m_pName;
};

class CSpawnPage : public PropertyPage
{
	DECLARE_CLASS_SIMPLE( CSpawnPage, PropertyPage );

public:
	CSpawnPage( Panel *parent, const char *name, const char *spawnListName = nullptr ) : BaseClass( parent, name ), m_pViewport( nullptr ), m_pContentPanel( nullptr ), m_pVScroll( nullptr ), m_pSearch( nullptr ), m_scrollPadding( 5 )
	{
		SetSize( 400, 300 );
		vgui::ivgui()->AddTickSignal( GetVPanel(), 500 );

		m_pViewport = new EditablePanel( this, "SpawnViewport" );
		m_pContentPanel = new EditablePanel( m_pViewport, "SpawnContent" );
		m_pContentPanel->SetAutoDelete( false ); // we'll manage children

		if ( auto sheet = dynamic_cast< CPngPropertySheet * >( GetParent() ) )
		{
			m_hoverTexId = sheet->LoadPNGTexture( "materials/gui/sm_hover.png" );
		}

		// todo: make this less hacky?
		SetBgColor(Color(108, 111, 114, 128));

		m_pVScroll = new ScrollBar( this, "PageScrollBar", true );
		m_pVScroll->SetVisible( false );
		m_pVScroll->AddActionSignalTarget( this );

		m_pSearch = new CSearchPanel( this, "SearchPanel" );
		m_pSearch->SetVisible( true );
		m_pSearch->AddActionSignalTarget( this );

		if ( spawnListName && spawnListName[0] )
			m_spawnListName = spawnListName;
		else if ( name && name[0] )
			m_spawnListName = name;
		else
			m_spawnListName.clear();

		m_tick = 0;

		if ( !m_spawnListName.empty() )
		{
			LoadSpawnList( m_spawnListName.c_str() );
		}
	}

	virtual ~CSpawnPage()
	{
		ClearAllEntries();
	}

	enum EntryType
	{
		ENTRY_HEADER,
		ENTRY_PROP,
		ENTRY_TEXT,
		ENTRY_IMAGE
	};

	struct SpawnEntry
	{
		EntryType	type;
		std::string name;
		std::string model;
		std::string image;
		std::string command;

		int	 x, y, w, h;
		bool visibleByFilter;

		SpawnEntry() : type( ENTRY_TEXT ), x( 0 ), y( 0 ), w( 0 ), h( 0 ), visibleByFilter( true )
		{
		}
	};

	virtual void OnTick() OVERRIDE
	{
		BaseClass::OnTick();

		if ( !engine->IsInGame() )
		{
			return;
		}

		UpdateVisiblePanels();
	}

	MESSAGE_FUNC_PARAMS( ScrollBarSliderMoved, "ScrollBarSliderMoved", pKV )
	{
		int position = pKV->GetInt( "position", 0 );
		InvalidateLayout( true );
	}

	MESSAGE_FUNC_PARAMS( OnSearchTextChanged, "SearchTextChanged", pKV )
	{
		const char *searchText = pKV->GetString( "text", "" );
		FilterSpawnList( searchText );
	}

	void FilterSpawnList( const char *searchText )
	{
		bool empty = ( searchText == nullptr || searchText[0] == '\0' );

		for ( size_t i = 0; i < m_entries.size(); ++i )
		{
			SpawnEntry &e = m_entries[i];
			if ( e.type == ENTRY_PROP || e.type == ENTRY_IMAGE )
			{
				if ( empty )
				{
					e.visibleByFilter = true;
				}
				else
				{
					const std::string nameLower = ToLower( e.name );
					const std::string searchLower = ToLower( searchText );
					e.visibleByFilter = ( nameLower.find( searchLower ) != std::string::npos );
				}
			}
			else
			{
				e.visibleByFilter = true;
			}
		}

		ComputeLayoutPositions();
		InvalidateLayout( true );
		UpdateVisiblePanels();
	}

	virtual void OnMouseWheeled( int delta ) OVERRIDE
	{
		if ( !m_pVScroll->IsVisible() )
			return; // @ThePixelMoon: get rid of the shitty bug with being able to scroll in non-scrollable pages

		int cur = m_pVScroll->GetValue();
		int step = max( 1, m_pVScroll->GetRangeWindow() / 10 );
		cur -= delta * step;

		int min, max;
		m_pVScroll->GetRange( min, max );
		cur = clamp( cur, min, max );
		m_pVScroll->SetValue( cur );

		InvalidateLayout( true );
	}

	virtual void PerformLayout() OVERRIDE
	{
		BaseClass::PerformLayout();

		const int scrollBarW = 16;
		const int padding = m_scrollPadding;
		const int searchPanelWidth = 200;

		int w = GetWide();
		int h = GetTall();

		int searchPanelHeight = h - padding * 2;
		m_pSearch->SetBounds( padding, padding, searchPanelWidth, searchPanelHeight );

		int viewportX = padding + searchPanelWidth + padding;
		int viewportW = w - viewportX - ( padding + ( m_pVScroll->IsVisible() ? scrollBarW + padding : 0 ) );
		int contentWidth = viewportW - padding * 2;

		m_pViewport->SetBounds( viewportX, padding, viewportW, searchPanelHeight );

		if ( contentWidth != m_lastContentWidth )
		{
			m_lastContentWidth = contentWidth;
			ComputeLayoutPositions();
		}

		m_pContentPanel->SetBounds( 0, 0, contentWidth, m_contentTall );

		bool needScroll = m_contentTall > searchPanelHeight;
		m_pVScroll->SetVisible( needScroll );
		if ( needScroll )
		{
			m_pVScroll->SetBounds( w - scrollBarW - padding, padding, scrollBarW, searchPanelHeight );
			m_pVScroll->SetRange( 0, m_contentTall - 1 );
			m_pVScroll->SetRangeWindow( searchPanelHeight );
			m_pVScroll->SetButtonPressedScrollValue( max( 1, searchPanelHeight / 10 ) );
		}

		if ( needScroll )
		{
			int val = m_pVScroll->GetValue();
			m_pContentPanel->SetPos( 0, -val );
		}
		else
		{
			m_pContentPanel->SetPos( 0, 0 );
		}

		UpdateVisiblePanels();
	}

	int AddModelEntry( const char *displayName, const char *modelPath, const char *command )
	{
		SpawnEntry e;
		e.type = ENTRY_PROP;
		e.name = displayName ? displayName : "";
		e.model = modelPath ? modelPath : "";
		e.command = command ? command : "";
		e.visibleByFilter = true;
		m_entries.push_back( std::move( e ) );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return (int)m_entries.size() - 1;
	}

	int AddImageEntry( const char *displayName, const char *imagePath, const char *command )
	{
		SpawnEntry e;
		e.type = ENTRY_IMAGE;
		e.name = displayName ? displayName : "";
		e.image = imagePath ? imagePath : "";
		e.command = command ? command : "";
		e.visibleByFilter = true;
		m_entries.push_back( std::move( e ) );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return (int)m_entries.size() - 1;
	}

	int AddHeader( const char *text )
	{
		SpawnEntry e;
		e.type = ENTRY_HEADER;
		e.name = text ? text : "";
		e.visibleByFilter = true;
		m_entries.push_back( std::move( e ) );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return (int)m_entries.size() - 1;
	}

	bool HasHeader( const char *text ) const
	{
		if ( !text )
			return false;

		for ( const auto &e : m_entries )
		{
			if ( e.type == ENTRY_HEADER )
			{
				const char *entryText = e.name.c_str();
				if ( entryText && Q_stricmp( entryText, text ) == 0 )
				{
					return true;
				}
			}
		}
		return false;
	}

	int AddModelEntryToHeader( const char *headerName, const char *displayName, const char *modelPath, const char *command )
	{
		if ( !headerName || !headerName[0] )
			return AddModelEntry( displayName, modelPath, command );

		int headerIndex = -1;
		for ( int i = 0; i < (int)m_entries.size(); ++i )
		{
			if ( m_entries[i].type == ENTRY_HEADER )
			{
				const char *hn = m_entries[i].name.c_str();
				if ( hn && Q_stricmp( hn, headerName ) == 0 )
				{
					headerIndex = i;
					break;
				}
			}
		}

		if ( headerIndex == -1 )
		{
			AddHeader( headerName );
			return AddModelEntry( displayName, modelPath, command );
		}

		int insertPos = headerIndex + 1;
		while ( insertPos < (int)m_entries.size() && m_entries[insertPos].type != ENTRY_HEADER )
			++insertPos;

		SpawnEntry e;
		e.type = ENTRY_PROP;
		e.name = displayName ? displayName : "";
		e.model = modelPath ? modelPath : "";
		e.command = command ? command : "";
		e.visibleByFilter = true;

		m_entries.insert( m_entries.begin() + insertPos, std::move( e ) );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return insertPos;
	}

	int AddImageEntryToHeader( const char *headerName, const char *displayName, const char *imagePath, const char *command )
	{
		if ( !headerName || !headerName[0] )
		{
			return AddImageEntry( displayName, imagePath, command );
		}

		int headerIndex = -1;
		for ( int i = 0; i < (int)m_entries.size(); ++i )
		{
			if ( m_entries[i].type == ENTRY_HEADER )
			{
				const char *hn = m_entries[i].name.c_str();
				if ( hn && Q_stricmp( hn, headerName ) == 0 )
				{
					headerIndex = i;
					break;
				}
			}
		}

		if ( headerIndex == -1 )
		{
			AddHeader( headerName );
			return AddImageEntry( displayName, imagePath, command );
		}

		int insertPos = headerIndex + 1;
		while ( insertPos < (int)m_entries.size() && m_entries[insertPos].type != ENTRY_HEADER )
			++insertPos;

		SpawnEntry e;
		e.type = ENTRY_IMAGE;
		e.name = displayName ? displayName : "";
		e.image = imagePath ? imagePath : "";
		e.command = command ? command : "";
		e.visibleByFilter = true;

		m_entries.insert( m_entries.begin() + insertPos, std::move( e ) );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return insertPos;
	}

	bool HasModelButton( const char *displayName, const char *modelPath ) const
	{
		if ( !displayName || !modelPath )
			return false;

		for ( const auto &e : m_entries )
		{
			if ( e.type == ENTRY_PROP )
			{
				const char *entryName = e.name.c_str();
				const char *entryModel = e.model.c_str();
				if ( entryName && entryModel && Q_stricmp( entryName, displayName ) == 0 && Q_stricmp( entryModel, modelPath ) == 0 )
				{
					return true;
				}
			}
		}
		return false;
	}

	bool HasImageButton( const char *displayName, const char *imagePath ) const
	{
		if ( !displayName || !imagePath )
			return false;

		for ( const auto &e : m_entries )
		{
			if ( e.type == ENTRY_IMAGE )
			{
				const char *entryName = e.name.c_str();
				const char *entryImage = e.image.c_str();
				if ( entryName && entryImage && Q_stricmp( entryName, displayName ) == 0 && Q_stricmp( entryImage, imagePath ) == 0 )
				{
					return true;
				}
			}
		}
		return false;
	}

	bool RemoveEntry( int index )
	{
		if ( index < 0 || index >= (int)m_entries.size() )
			return false;
		m_entries.erase( m_entries.begin() + index );
		ComputeLayoutPositions();
		InvalidateLayout( true );
		return true;
	}

	void ClearEntries()
	{
		m_entries.clear();
		ComputeLayoutPositions();
		InvalidateLayout( true );
	}

private:
	void LoadSpawnList( const char *name )
	{
		if ( !name || !name[0] )
			return;

		ClearAllEntries();

		char fullPath[MAX_PATH];
		V_snprintf( fullPath, sizeof( fullPath ), "settings/spawnlists/%s", name );
		KeyValues *pKV = new KeyValues( name );
		if ( !pKV->LoadFromFile( filesystem, fullPath, "MOD" ) )
		{
			pKV->deleteThis();
			DevMsg( "Failed to load %s\n", fullPath );
			return;
		}

		for ( KeyValues *pChild = pKV->GetFirstSubKey(); pChild; pChild = pChild->GetNextKey() )
		{
			const char *keyName = pChild->GetName();

			if ( Q_stricmp( keyName, "page_icon" ) == 0 )
				continue;

			if ( Q_stricmp( keyName, "header" ) == 0 )
			{
				SpawnEntry e;
				e.type = ENTRY_HEADER;
				e.name = pChild->GetString( "text", "" );
				m_entries.push_back( std::move( e ) );
			}
			else if ( Q_stricmp( keyName, "prop" ) == 0 )
			{
				const char *displayName = pChild->GetString( "name", "" );
				const char *model = pChild->GetString( "model", "" );
				bool		asMdlPanel = pChild->GetBool( "as_mdlpanel", false );

				if ( asMdlPanel )
				{
					SpawnEntry e;
					e.type = ENTRY_PROP;
					e.name = displayName ? displayName : "";
					e.model = model ? model : "";

					char command[MAX_PATH];
					Q_snprintf( command, sizeof( command ), "prop_physics_create %s", strncmp( e.model.c_str(), "models/", 7 ) == 0 ? e.model.c_str() + 7 : e.model.c_str() );
					e.command = command;
					m_entries.push_back( std::move( e ) );
				}
			}
			else if ( Q_stricmp( keyName, "image_button" ) == 0 )
			{
				const char *displayName = pChild->GetString( "name", "" );
				const char *imagePath = pChild->GetString( "image", "" );
				const char *cmd = pChild->GetString( "command", "" );

				if ( imagePath && imagePath[0] != '\0' )
				{
					SpawnEntry e;
					e.type = ENTRY_IMAGE;
					e.name = displayName ? displayName : "";
					e.image = imagePath ? imagePath : "";
					e.command = cmd ? cmd : "";
					m_entries.push_back( std::move( e ) );
				}
			}
			else
			{
				SpawnEntry e;
				e.type = ENTRY_TEXT;
				e.name = pChild->GetString( "text", "" );
				m_entries.push_back( std::move( e ) );
			}
		}

		pKV->deleteThis();

		// initial layout
		ComputeLayoutPositions();
		InvalidateLayout( true );
	}

	void ComputeLayoutPositions()
	{
		const int padding = m_scrollPadding;
		const int labelHeight = 16;
		const int itemMargin = 4;

		const int propBtnW = 78;
		const int propBtnH = 64;
		const int imageBtnW = 110;
		const int imageBtnH = 96;
		const int headerHeight = 24;

		int contentWidth = m_lastContentWidth;
		if ( contentWidth <= 0 )
		{
			m_contentTall = 0;
			return;
		}

		int x = padding;
		int y = padding;

		int gridColsProp = max( 1, ( contentWidth - padding * 2 ) / ( propBtnW + padding ) );
		int gridColsImage = max( 1, ( contentWidth - padding * 2 ) / ( imageBtnW + padding ) );

		int countInSection = 0;

		enum GridType
		{
			GRID_NONE = 0,
			GRID_PROP = 1,
			GRID_IMAGE = 2
		};
		GridType activeGrid = GRID_NONE;

		auto flushSection = [&]( void )
		{
			if ( countInSection <= 0 || activeGrid == GRID_NONE )
			{
				countInSection = 0;
				return;
			}

			int cols = ( activeGrid == GRID_IMAGE ) ? gridColsImage : gridColsProp;
			int rows = ( countInSection + cols - 1 ) / cols;
			int perRowH = ( activeGrid == GRID_IMAGE ) ? ( imageBtnH + labelHeight + padding ) : ( propBtnH + labelHeight + padding );
			y += rows * perRowH + itemMargin;
			countInSection = 0;
			activeGrid = GRID_NONE;
		};

		for ( size_t i = 0; i < m_entries.size(); ++i )
		{
			SpawnEntry &e = m_entries[i];

			if ( !e.visibleByFilter )
			{
				e.x = e.y = e.w = e.h = 0;
				continue;
			}

			if ( e.type == ENTRY_HEADER )
			{
				flushSection();

				int headerWidth = contentWidth - padding * 2;
				e.x = padding;
				e.y = y;
				e.w = headerWidth;
				e.h = headerHeight;
				y += headerHeight + itemMargin;
			}
			else if ( e.type == ENTRY_PROP )
			{
				if ( activeGrid != GRID_PROP && activeGrid != GRID_NONE )
				{
					flushSection();
				}
				activeGrid = GRID_PROP;

				int cols = gridColsProp;
				int col = countInSection % cols;
				int row = countInSection / cols;

				int itemX = padding + col * ( propBtnW + padding );
				int itemY = y + row * ( propBtnH + labelHeight + padding );

				e.x = itemX;
				e.y = itemY;
				e.w = propBtnW;
				e.h = propBtnH + labelHeight;

				countInSection++;
			}
			else if ( e.type == ENTRY_IMAGE )
			{
				if ( activeGrid != GRID_IMAGE && activeGrid != GRID_NONE )
				{
					flushSection();
				}
				activeGrid = GRID_IMAGE;

				int cols = gridColsImage;
				int col = countInSection % cols;
				int row = countInSection / cols;

				int itemX = padding + col * ( imageBtnW + padding );
				int itemY = y + row * ( imageBtnH + labelHeight + padding );

				e.x = itemX;
				e.y = itemY;
				e.w = imageBtnW;
				e.h = imageBtnH + labelHeight;

				countInSection++;
			}
			else // ENTRY_TEXT
			{
				flushSection();

				int textH = 18;
				int textW = contentWidth - padding * 2;
				e.x = padding;
				e.y = y;
				e.w = textW;
				e.h = textH;
				y += textH + itemMargin;
			}
		}

		flushSection();

		m_contentTall = y + padding;
	}

	void UpdateVisiblePanels()
	{
		if ( !m_pContentPanel || m_lastContentWidth <= 0 )
			return;

		int viewTall = m_pViewport->GetTall();
		int scrollVal = m_pVScroll ? m_pVScroll->GetValue() : 0;

		int visibleTop = scrollVal;
		int visibleBottom = scrollVal + viewTall;

		// For every entry, if its rect intersects visible rect -> ensure created
		for ( size_t i = 0; i < m_entries.size(); ++i )
		{
			SpawnEntry &e = m_entries[i];

			if ( !e.visibleByFilter || e.w == 0 || e.h == 0 )
			{
				// ensure panel removed if present
				auto it = m_activePanels.find( (int)i );
				if ( it != m_activePanels.end() )
				{
					Panel *p = it->second;
					if ( p )
					{
						if ( CModelButton *mb = dynamic_cast< CModelButton * >( p ) )
						{
							mb->m_pModelPanel->SetMDL( "" );
							mb->m_pModelPanel->SetVisible( false );
						}
						p->SetVisible( false );
						p->MarkForDeletion();
					}
					m_activePanels.erase( it );
				}
				continue;
			}

			int itemTop = e.y;
			int itemBottom = e.y + e.h;

			bool intersects = !( itemBottom < visibleTop || itemTop > visibleBottom );

			auto it = m_activePanels.find( (int)i );

			if ( intersects )
			{
				if ( it == m_activePanels.end() )
				{
					// create
					Panel *p = CreatePanelForEntry( i );
					if ( p )
						m_activePanels[(int)i] = p;
				}
				else
				{
					// already created - ensure bounds are up-to-date
					Panel *p = it->second;
					if ( p )
					{
						p->SetBounds( e.x, e.y, e.w, e.h );
						p->SetVisible( true );
					}
				}
			}
			else
			{
				// outside view - destroy if exists
				if ( it != m_activePanels.end() )
				{
					Panel *p = it->second;
					if ( p )
						p->MarkForDeletion();
					m_activePanels.erase( it );
				}
			}
		}
	}

	Panel *CreatePanelForEntry( int index )
	{
		if ( index < 0 || index >= (int)m_entries.size() )
			return nullptr;

		SpawnEntry &e = m_entries[index];

		if ( e.type == ENTRY_HEADER )
		{
			Label	*pLabel = new Label( m_pContentPanel, nullptr, e.name.c_str() );
			IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
			if ( pScheme )
			{
				HFont hf = pScheme->GetFont( "DefaultLarge", true );
				if ( hf )
					pLabel->SetFont( hf );
			}
			pLabel->SetBounds( e.x, e.y, e.w, e.h );
			m_spawnItems.AddToTail( pLabel );
			return pLabel;
		}
		else if ( e.type == ENTRY_PROP )
		{
			CModelButton *pBtn = new CModelButton( m_pContentPanel, CFmtStr( "ModelBtn_%d", index ), e.model.c_str(), e.name.c_str(), e.command.c_str(), m_hoverTexId );
			pBtn->SetBounds( e.x, e.y, e.w, e.h );
			m_spawnItems.AddToTail( pBtn );
			return pBtn;
		}
		else if ( e.type == ENTRY_IMAGE )
		{
			int texId = -1;
			int conId = -1;
			int conhovId = -1;
			if ( Panel *pparent = GetParent() )
			{
				CPngPropertySheet *sheet = dynamic_cast< CPngPropertySheet * >( pparent );
				if ( sheet )
				{
					texId = sheet->LoadPNGTexture( e.image.c_str() );
					conId = sheet->LoadPNGTexture( "materials/gui/contenticon-normal.png" );
					conhovId = sheet->LoadPNGTexture( "materials/gui/contenticon-hovered.png" );
				}
			}

			CImageButton *pBtn = new CImageButton( m_pContentPanel, CFmtStr( "ImageBtn_%d", index ), texId, e.name.c_str(), e.command.c_str(), m_hoverTexId, conId, conhovId );
			pBtn->SetBounds( e.x, e.y, e.w, e.h );
			m_spawnItems.AddToTail( pBtn );
			return pBtn;
		}
		else // ENTRY_TEXT
		{
			Label *pLabel = new Label( m_pContentPanel, nullptr, e.name.c_str() );
			pLabel->SetBounds( e.x, e.y, e.w, e.h );
			m_spawnItems.AddToTail( pLabel );
			return pLabel;
		}
	}

	void ClearAllEntries()
	{
		// mark active panels for deletion
		for ( auto &p : m_activePanels )
		{
			if ( p.second )
				p.second->MarkForDeletion();
		}
		m_activePanels.clear();

		m_spawnItems.RemoveAll();

		m_entries.clear();
		m_contentTall = 0;
	}

	std::string ToLower( const std::string &s )
	{
		std::string out = s;
		std::transform( out.begin(), out.end(), out.begin(), ::tolower );
		return out;
	}

public:
	EditablePanel *m_pViewport;
	EditablePanel *m_pContentPanel;
	ScrollBar	  *m_pVScroll;
	CSearchPanel  *m_pSearch;

	int m_lastContentWidth = 0;
	int m_contentTall = 0;
	int m_scrollPadding = 5;

	std::vector< SpawnEntry >		   m_entries;
	std::unordered_map< int, Panel * > m_activePanels; // index -> panel

	CUtlVector< Panel * > m_spawnItems; // keep references so we can MarkForDeletion when clearing

	std::string m_spawnListName;
	int			m_tick;

	int m_hoverTexId = -1;
};

class CSpawnMenu : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( CSpawnMenu, vgui::PropertyDialog );

public:
	CSpawnMenu( vgui::VPANEL parent );
	virtual ~CSpawnMenu() = default;

	int	 Lua_CreateTab( const char *tabName, const char *spawnListName = nullptr, const char *iconPath = nullptr );
	bool Lua_AddModelButton( int tabIndex, const char *displayName, const char *modelPath, const char *command = nullptr );
	bool Lua_AddImageButton( int tabIndex, const char *displayName, const char *imagePath, const char *command = nullptr );
	bool Lua_AddHeader( int tabIndex, const char *text );

	bool Lua_HeaderExists( int tabIndex, const char *text )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		return page->HasHeader( text );
	}

	bool Lua_ModelButtonExists( int tabIndex, const char *displayName, const char *modelPath )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		return page->HasModelButton( displayName, modelPath );
	}

	bool Lua_ImageButtonExists( int tabIndex, const char *displayName, const char *imagePath )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		return page->HasImageButton( displayName, imagePath );
	}

	bool Lua_RemoveEntry( int tabIndex, int entryIndex )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		return page->RemoveEntry( entryIndex );
	}

	bool Lua_ClearTab( int tabIndex )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		page->ClearEntries();
		return true;
	}

	bool Lua_AddModelButtonToHeader( int tabIndex, const char *headerName, const char *displayName, const char *modelPath, const char *command = nullptr )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		page->AddModelEntryToHeader( headerName ? headerName : "", displayName ? displayName : "", modelPath ? modelPath : "", command ? command : "" );
		return true;
	}

	bool Lua_AddImageButtonToHeader( int tabIndex, const char *headerName, const char *displayName, const char *imagePath, const char *command = nullptr )
	{
		if ( !m_pPngSheet )
			return false;
		if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
			return false;

		Panel	   *p = m_pPngSheet->GetPage( tabIndex );
		CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
		if ( !page )
			return false;

		page->AddImageEntryToHeader( headerName ? headerName : "", displayName ? displayName : "", imagePath ? imagePath : "", command ? command : "" );
		return true;
	}

protected:
	virtual void OnTick();
	virtual void OnCommand( const char *pcCommand );

public:
	void CreateTabs();

	CPngPropertySheet *m_pPngSheet;
	Button			  *m_pCloseButton;
};

CSpawnMenu::CSpawnMenu( vgui::VPANEL parent ) : BaseClass( NULL, "SpawnMenu" )
{
	int iWide, iTall;
	vgui::surface()->GetScreenSize( iWide, iTall );

	SetSize( iWide / 1.25, iTall / 1.25 );
	SetPos( iWide / 8, iTall / 8 );

	SetParent( parent );

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

	SetProportional( false );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetOKButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );
	SetVisible( false );

	SetScheme( vgui::scheme()->LoadSchemeFromFile( "resource/SourceScheme.res", "SourceScheme" ) );

	// todo: make this less hacky?
	SetBgColor(Color(108, 111, 114, 128));

	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );

	if ( _propertySheet )
		_propertySheet->SetVisible( false );

	m_pPngSheet = new CPngPropertySheet( this, "PngTabPanel", true );
	m_pPngSheet->SetPos( 0, 0 );
	m_pPngSheet->SetSize( GetWide(), GetTall() );

	m_pCloseButton = new Button( this, "Button", "Close", this, "turnoff" );
	m_pCloseButton->SetDepressedSound( "common/bugreporter_succeeded.wav" );
	m_pCloseButton->SetReleasedSound( "ui/buttonclick.wav" );
	m_pCloseButton->SetZPos( 10000 );

	int parentW, parentH;
	GetSize( parentW, parentH );

	int buttonW = 100;
	int buttonH = 30;
	int padding = 10;

	int x = parentW - buttonW - padding - 20;
	int y = parentH - buttonH - padding;

	m_pCloseButton->SetBounds( x, y, buttonW, buttonH );

	CreateTabs();
}

int CSpawnMenu::Lua_CreateTab( const char *tabName, const char *spawnListName, const char *iconPath )
{
	if ( !m_pPngSheet || !tabName )
		return -1;

	CUtlString	sTabName = tabName;
	CUtlString	sSpawnName = spawnListName ? spawnListName : "";
	CSpawnPage *page = new CSpawnPage( m_pPngSheet, sTabName.Get(), sSpawnName.Get() );

	CUtlString	tabFullName = CUtlString( STARTER_SPACES_TAB ) + CUtlString( sTabName.Get() );
	const char *icon = ( iconPath && iconPath[0] ) ? iconPath : "materials/icon16/folder_page.png";
	m_pPngSheet->AddPageWithPNGIcon( page, tabFullName.Get(), icon, false );

	return m_pPngSheet->GetNumPages() - 1;
}

bool CSpawnMenu::Lua_AddModelButton( int tabIndex, const char *displayName, const char *modelPath, const char *command )
{
	if ( !m_pPngSheet )
		return false;
	if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
		return false;

	Panel	   *p = m_pPngSheet->GetPage( tabIndex );
	CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
	if ( !page )
		return false;

	page->AddModelEntry( displayName ? displayName : "", modelPath ? modelPath : "", command ? command : "" );
	return true;
}

bool CSpawnMenu::Lua_AddImageButton( int tabIndex, const char *displayName, const char *imagePath, const char *command )
{
	if ( !m_pPngSheet )
		return false;
	if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
		return false;

	Panel	   *p = m_pPngSheet->GetPage( tabIndex );
	CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
	if ( !page )
		return false;

	page->AddImageEntry( displayName ? displayName : "", imagePath ? imagePath : "", command ? command : "" );
	return true;
}

bool CSpawnMenu::Lua_AddHeader( int tabIndex, const char *text )
{
	if ( !m_pPngSheet )
		return false;
	if ( tabIndex < 0 || tabIndex >= m_pPngSheet->GetNumPages() )
		return false;

	Panel	   *p = m_pPngSheet->GetPage( tabIndex );
	CSpawnPage *page = dynamic_cast< CSpawnPage * >( p );
	if ( !page )
		return false;

	page->AddHeader( text ? text : "" );
	return true;
}

static void GetKVFiles( const char *folder, CUtlVector< CUtlString > &kvFiles )
{
	char searchPath[MAX_PATH];
	V_snprintf( searchPath, sizeof( searchPath ), "%s/*.kv", folder );

	FileFindHandle_t findHandle;
	const char		*fileName = g_pFullFileSystem->FindFirst( searchPath, &findHandle );
	if ( fileName == nullptr || findHandle == FILESYSTEM_INVALID_FIND_HANDLE )
		return;

	kvFiles.AddToTail( CUtlString( fileName ) );

	while ( ( fileName = g_pFullFileSystem->FindNext( findHandle ) ) != nullptr )
	{
		kvFiles.AddToTail( CUtlString( fileName ) );
	}

	g_pFullFileSystem->FindClose( findHandle );
}

void CSpawnMenu::CreateTabs()
{
	CUtlVector< CUtlString > kvFiles;
	GetKVFiles( "settings/spawnlists", kvFiles );

	for ( int i = 0; i < kvFiles.Count(); i++ )
	{
		CUtlString kvFile = kvFiles[i];

		char path[MAX_PATH];
		V_snprintf( path, sizeof( path ), "settings/spawnlists/%s", kvFile.Get() );

		KeyValues *pKV = new KeyValues( "tab" );
		if ( !pKV->LoadFromFile( g_pFullFileSystem, path, "MOD" ) )
		{
			pKV->deleteThis();
			continue;
		}

		KeyValues *pTabKey = pKV->GetFirstSubKey();
		if ( !pTabKey )
		{
			pKV->deleteThis();
			continue;
		}

		const char *tabName = pKV->GetName();
		const char *iconPath = pKV->GetString( "page_icon", "materials/icon16/script.png" );

		CUtlString fileBase = kvFile;
		int		   len = Q_strlen( fileBase.Get() );
		if ( len > 3 && Q_stricmp( fileBase.Get() + len - 3, "kv" ) == 0 )
		{
			// remove .kv
			char tmp[MAX_PATH];
			Q_strncpy( tmp, fileBase.Get(), sizeof( tmp ) );
			tmp[len - 3 - 1] = '\0';
			fileBase = CUtlString( tmp );
		}
		CSpawnPage *page = new CSpawnPage( m_pPngSheet, tabName, fileBase.Get() );
		CUtlString	tabFullName = CUtlString( STARTER_SPACES_TAB ) + CUtlString( tabName );
		m_pPngSheet->AddPageWithPNGIcon( page, tabFullName.Get(), iconPath, false );

		pKV->deleteThis();
	}
}

#ifdef ANDROID
#define TOUCH_DEFAULT "1"
#else
#define TOUCH_DEFAULT "0"
#endif

void CSpawnMenu::OnTick()
{
	if ( !sv_spawnmenu_allowed.GetBool() )
	{
		SetVisible( false );
		return;
	}

	BaseClass::OnTick();

	CSpawnPage *page = (CSpawnPage *)m_pPngSheet->GetPage( m_pPngSheet->GetActivePageNum() );
	if ( page )
	{
		if ( !page->m_pSearch->isTypingShit )
		{
			SetVisible( cl_showspawnmenu.GetBool() );

			extern ConVar touch_enable;
			if (cl_showspawnmenu.GetBool())
				touch_enable.SetValue(0);
			else
				touch_enable.SetValue(TOUCH_DEFAULT);
		}
	}

	for ( int i = 0; i < m_pPngSheet->GetNumPages(); i++ )
	{
		CSpawnPage *page = (CSpawnPage *)m_pPngSheet->GetPage( i );
		if ( page )
		{
			page->InvalidateLayout( true );
		}
	}
}

void CSpawnMenu::OnCommand( const char *pcCommand )
{
	BaseClass::OnCommand( pcCommand );

	if ( !Q_stricmp( pcCommand, "turnoff" ) || !Q_stricmp( pcCommand, "Close" ) )
	{
		cl_showspawnmenu.SetValue( 0 );
		SetVisible( false );

		extern ConVar touch_enable;
		touch_enable.SetValue(TOUCH_DEFAULT);

		for ( int i = 0; i < m_pPngSheet->GetNumPages(); i++ )
		{
			CSpawnPage *page = (CSpawnPage *)m_pPngSheet->GetPage( i );
			if ( page )
			{
				page->m_pSearch->m_pSearchEntry->SetText( "" );
				page->m_pSearch->TextChanged(); // dirty hack
				page->m_pSearch->isTypingShit = false;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Spawn menu interface implementation
//-----------------------------------------------------------------------------
class CSpawnMenuInterface : public SpawnMenu
{
public:
	CSpawnMenu *SpawnMenu;

	CSpawnMenuInterface()
	{
		SpawnMenu = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		SpawnMenu = new CSpawnMenu( parent );
	}

	void Destroy()
	{
		if ( SpawnMenu )
		{
			SpawnMenu->SetParent( (vgui::Panel *)NULL );
			delete SpawnMenu;
		}
	}

	void Activate( void )
	{
		if ( SpawnMenu )
		{
			SpawnMenu->Activate();
		}
	}
};

static CSpawnMenuInterface g_SpawnMenu;
SpawnMenu				  *spawnmenu = (SpawnMenu *)&g_SpawnMenu;

CON_COMMAND( OpenSpawnMenu, "Toggles spawn menu on or off" )
{
	cl_showspawnmenu.SetValue( !cl_showspawnmenu.GetBool() );
	spawnmenu->Activate();
}

//------
// new spawnmenu lua funcs lol
//------

#ifdef LUA_SDK
static int sm_CreateTab( lua_State *L )
{
	const char *tabName = luaL_checkstring( L, 1 );
	const char *spawnListName = luaL_optstring( L, 2, nullptr );
	const char *iconPath = luaL_optstring( L, 3, nullptr );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushnil( L );
		lua_pushstring( L, "spawn menu not created" );
		return 2;
	}

	int idx = g_SpawnMenu.SpawnMenu->Lua_CreateTab( tabName, spawnListName, iconPath );
	if ( idx < 0 )
	{
		lua_pushnil( L );
		lua_pushstring( L, "failed to create tab" );
		return 2;
	}
	lua_pushinteger( L, idx );
	return 1;
}

static int sm_CreateModelButton( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *displayName = luaL_checkstring( L, 2 );
	const char *modelPath = luaL_checkstring( L, 3 );
	const char *command = luaL_optstring( L, 4, nullptr );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	bool ok = g_SpawnMenu.SpawnMenu->Lua_AddModelButton( tabIndex, displayName, modelPath, command );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_CreateImageButton( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *displayName = luaL_checkstring( L, 2 );
	const char *imagePath = luaL_checkstring( L, 3 );
	const char *command = luaL_optstring( L, 4, nullptr );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	bool ok = g_SpawnMenu.SpawnMenu->Lua_AddImageButton( tabIndex, displayName, imagePath, command );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_CreateHeader( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *text = luaL_checkstring( L, 2 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	bool ok = g_SpawnMenu.SpawnMenu->Lua_AddHeader( tabIndex, text );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_FindTabByName( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	if ( !g_SpawnMenu.SpawnMenu || !g_SpawnMenu.SpawnMenu->m_pPngSheet )
	{
		lua_pushnil( L );
		return 1;
	}

	for ( int i = 0; i < g_SpawnMenu.SpawnMenu->m_pPngSheet->GetNumPages(); i++ )
	{
		const char *pageName = g_SpawnMenu.SpawnMenu->m_pPngSheet->GetPage( i )->GetName();
		if ( pageName && Q_stricmp( pageName, name ) == 0 )
		{
			lua_pushinteger( L, i );
			return 1;
		}
	}
	lua_pushnil( L );
	return 1;
}

static int sm_TabExists( lua_State *L )
{
	const char *name = luaL_checkstring( L, 1 );
	if ( !g_SpawnMenu.SpawnMenu || !g_SpawnMenu.SpawnMenu->m_pPngSheet )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	for ( int i = 0; i < g_SpawnMenu.SpawnMenu->m_pPngSheet->GetNumPages(); i++ )
	{
		const char *pageName = g_SpawnMenu.SpawnMenu->m_pPngSheet->GetPage( i )->GetName();
		if ( pageName && Q_stricmp( pageName, name ) == 0 )
		{
			lua_pushboolean( L, 1 );
			return 1;
		}
	}
	lua_pushboolean( L, 0 );
	return 1;
}

static int sm_GetNumTabs( lua_State *L )
{
	if ( !g_SpawnMenu.SpawnMenu || !g_SpawnMenu.SpawnMenu->m_pPngSheet )
	{
		lua_pushinteger( L, 0 );
		return 1;
	}
	lua_pushinteger( L, g_SpawnMenu.SpawnMenu->m_pPngSheet->GetNumPages() );
	return 1;
}

static int sm_GetTabName( lua_State *L )
{
	int idx = (int)luaL_checkinteger( L, 1 );
	if ( !g_SpawnMenu.SpawnMenu || !g_SpawnMenu.SpawnMenu->m_pPngSheet )
	{
		lua_pushnil( L );
		return 1;
	}
	if ( idx < 0 || idx >= g_SpawnMenu.SpawnMenu->m_pPngSheet->GetNumPages() )
	{
		lua_pushnil( L );
		return 1;
	}

	const char *pageName = g_SpawnMenu.SpawnMenu->m_pPngSheet->GetPage( idx )->GetName();
	if ( !pageName )
	{
		lua_pushnil( L );
		return 1;
	}

	lua_pushstring( L, pageName );
	return 1;
}

static int sm_HeaderExists( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *text = luaL_checkstring( L, 2 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}
	bool ok = g_SpawnMenu.SpawnMenu->Lua_HeaderExists( tabIndex, text );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_ModelButtonExists( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *displayName = luaL_checkstring( L, 2 );
	const char *modelPath = luaL_checkstring( L, 3 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}
	bool ok = g_SpawnMenu.SpawnMenu->Lua_ModelButtonExists( tabIndex, displayName, modelPath );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_ImageButtonExists( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *displayName = luaL_checkstring( L, 2 );
	const char *imagePath = luaL_checkstring( L, 3 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}
	bool ok = g_SpawnMenu.SpawnMenu->Lua_ImageButtonExists( tabIndex, displayName, imagePath );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_RemoveEntry( lua_State *L )
{
	int tabIndex = (int)luaL_checkinteger( L, 1 );
	int entryIndex = (int)luaL_checkinteger( L, 2 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}
	bool ok = g_SpawnMenu.SpawnMenu->Lua_RemoveEntry( tabIndex, entryIndex );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_ClearTab( lua_State *L )
{
	int tabIndex = (int)luaL_checkinteger( L, 1 );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}
	bool ok = g_SpawnMenu.SpawnMenu->Lua_ClearTab( tabIndex );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_CreateModelButtonInHeader( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *headerName = luaL_checkstring( L, 2 );
	const char *displayName = luaL_checkstring( L, 3 );
	const char *modelPath = luaL_checkstring( L, 4 );
	const char *command = luaL_optstring( L, 5, nullptr );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	bool ok = g_SpawnMenu.SpawnMenu->Lua_AddModelButtonToHeader( tabIndex, headerName, displayName, modelPath, command );
	lua_pushboolean( L, ok );
	return 1;
}

static int sm_CreateImageButtonInHeader( lua_State *L )
{
	int			tabIndex = (int)luaL_checkinteger( L, 1 );
	const char *headerName = luaL_checkstring( L, 2 );
	const char *displayName = luaL_checkstring( L, 3 );
	const char *imagePath = luaL_checkstring( L, 4 );
	const char *command = luaL_optstring( L, 5, nullptr );

	if ( !g_SpawnMenu.SpawnMenu )
	{
		lua_pushboolean( L, 0 );
		return 1;
	}

	bool ok = g_SpawnMenu.SpawnMenu->Lua_AddImageButtonToHeader( tabIndex, headerName, displayName, imagePath, command );
	lua_pushboolean( L, ok );
	return 1;
}

static const luaL_Reg smlib[] = { { "CreateTab", sm_CreateTab }, { "CreateModelButton", sm_CreateModelButton }, { "CreateImageButton", sm_CreateImageButton }, { "CreateHeader", sm_CreateHeader }, { "FindTabByName", sm_FindTabByName },
	{ "TabExists", sm_TabExists }, { "GetNumTabs", sm_GetNumTabs }, { "GetTabName", sm_GetTabName }, { "HeaderExists", sm_HeaderExists }, { "ModelButtonExists", sm_ModelButtonExists }, { "ImageButtonExists", sm_ImageButtonExists },
	{ "RemoveEntry", sm_RemoveEntry }, { "ClearTab", sm_ClearTab }, { "CreateModelButtonInHeader", sm_CreateModelButtonInHeader }, { "CreateImageButtonInHeader", sm_CreateImageButtonInHeader }, { NULL, NULL } };

/*
** Open spawnmenu library
*/
LUALIB_API int luaopen_sm( lua_State *L )
{
	luaL_register( L, LUA_SPAWNMENULIBNAME, smlib );
	return 1;
}
#endif

// there it is
#ifdef LUA_SDK

class CSMLToolMenu : public vgui::Frame
{
public:
    CSMLToolMenu() : BaseClass(nullptr, "SMLToolMenu")
    {
        SetTitle("TOOL MENU", true);
        SetSize(400, 500);
        SetMoveable(true);
        SetSizeable(false);
        SetDeleteSelfOnClose(false);

        Center();

        m_pContent = new vgui::Panel(this, "Content");
        m_pContent->Dock(FILL);

        m_bPopulated = false;
    }

    void AddButton(const char* text, const char* command)
    {
        vgui::Button* btn = new vgui::Button(m_pContent, text, text);
        btn->SetTall(28);
        btn->Dock(TOP);
        btn->SetCommand(command);
        btn->AddActionSignalTarget(this);

        m_Buttons.AddToTail(btn);
    }

    void OnCommand(const char* cmd) override
    {
        engine->ClientCmd((char*)cmd);
    }

    void Clear()
    {
        for (int i = 0; i < m_Buttons.Count(); i++)
        {
            if (m_Buttons[i])
                m_Buttons[i]->MarkForDeletion();
        }
        m_Buttons.RemoveAll();
        m_bPopulated = false;
    }

    void Populate(lua_State* L)
    {
        if (m_bPopulated)
            return;

        m_bPopulated = true;

        lua_getglobal(L, "hook");
        lua_getfield(L, -1, "Run");

        lua_pushstring(L, "PopulateToolmenu");

        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            Warning("PopulateToolmenu error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    bool IsPopulated() const { return m_bPopulated; }

private:
    vgui::Panel* m_pContent;
    CUtlVector<vgui::Button*> m_Buttons;
    bool m_bPopulated;
};

static CSMLToolMenu* g_ToolMenu = nullptr;

void OpenToolMenu(lua_State* L)
{
    if (!g_ToolMenu)
    {
        g_ToolMenu = new CSMLToolMenu();
    }

    g_ToolMenu->SetVisible(true);
    g_ToolMenu->MoveToFront();
    g_ToolMenu->RequestFocus();

    g_ToolMenu->Populate(L);
}

static int tm_CreateButton(lua_State* L)
{
    const char* text = luaL_checkstring(L, 1);
    const char* command = luaL_checkstring(L, 2);

    if (!g_ToolMenu)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    g_ToolMenu->AddButton(text, command);

    lua_pushboolean(L, 1);
    return 1;
}

static int tm_Clear(lua_State* L)
{
    if (g_ToolMenu)
        g_ToolMenu->Clear();

    lua_pushboolean(L, 1);
    return 1;
}

// a little useless 
static int tm_Open(lua_State* L)
{
    OpenToolMenu(L);
    return 0;
}

// register 
static const luaL_Reg toolmenu_lib[] = {
    { "CreateButton", tm_CreateButton },
    { "Clear", tm_Clear },
    { "Open", tm_Open },
    { NULL, NULL }
};

LUALIB_API int luaopen_toolmenu(lua_State* L)
{
    luaL_register(L, "toolmenu_Private", toolmenu_lib);
    return 1;
}

#endif

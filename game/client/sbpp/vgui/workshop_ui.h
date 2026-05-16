//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#ifndef WORKSHOP_UI_H
#define WORKSHOP_UI_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "menu/imageextbutton.h"

#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ScrollableEditablePanel.h>
#include <vgui/IImage.h>

#include <functional>

using namespace vgui;

struct Addon
{
	CUtlString id;
	CUtlString name;
	CUtlString description;
	CUtlString version;
	CUtlString author;
	CUtlString authorId;
	CUtlString thumbnailUrl;
	CUtlString downloadUrl;
	CUtlString sha256;
	long long  size; // bytes
	CUtlString submittedAt;
	CUtlString approvedAt;
	CUtlString approvedBy;

	CUtlString localThumbPath;

	Addon() : size( 0 )
	{
	}
};

typedef std::function< void( bool success, const CUtlVector< Addon > &addons ) > AddonListCallback;
typedef std::function< void( bool success, const char *errorMsg ) >				 AddonActionCallback;
typedef std::function< void( const CUtlString &id, const CUtlString &path ) >	 ThumbnailReadyCallback;

class WorkshopClient
{
public:
	WorkshopClient( const char *downloadFolder );
	~WorkshopClient();

	void FetchAddons( AddonListCallback cb );
	void DownloadAddon( const Addon &addon, AddonActionCallback cb );

	const CUtlVector< Addon > &GetCachedAddons() const
	{
		return m_cachedAddons;
	}

	void GetInstalledAddonIDs( CUtlVector< CUtlString > &outIDs );
	bool IsAddonInstalled( const char *id );
	bool RemoveAddon( const char *id );

	const char *GetDownloadFolder() const
	{
		return m_downloadFolder.String();
	}

public:
	bool ParseAddonsJSON( const char *jsonText, CUtlVector< Addon > &outAddons );
	void FetchThumbnails( ThumbnailReadyCallback perItemCb );

	CUtlString			m_downloadFolder;
	CUtlString			m_tempFolder;
	CUtlVector< Addon > m_cachedAddons;
};

WorkshopClient *GetWorkshopClient();
void			InitWorkshopClient( const char *downloadFolder );
void			ShutdownWorkshopClient();

class CWorkshopDialog;

class CAddonThumbnailPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( CAddonThumbnailPanel, Panel );

public:
	CAddonThumbnailPanel( Panel *parent, const char *panelName, const Addon &addon, bool allowSubscribe = true );
	~CAddonThumbnailPanel();

	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void OnMousePressed( MouseCode code );
	virtual void PaintBackground();
	virtual void PerformLayout();

	void ShowAddonDetails();
	void SetSubscribed( bool subscribed );
	bool IsSubscribed() const
	{
		return m_bSubscribed;
	}
	const char *GetAddonID() const
	{
		return m_addon.id.String();
	}
	const Addon &GetAddon() const
	{
		return m_addon;
	}

	void SetThumbnailPath( const char *path )
	{
		if ( !m_pImageButton )
			return;

		m_pImageButton->SetImage( path );
	}

	static void GetTileDimensions( int &outW, int &outH )
	{
		const int padding = 8;
		const int imageW = 200;
		const int imageH = 112;
		const int nameH = 18;
		const int sizeH = 14;
		const int textGap = 2;
		const int bottomGap = 6;
		const int bottomPad = 8;
		outW = imageW + padding * 2;
		outH = padding + imageH + bottomGap + ( nameH + textGap + sizeH ) + bottomPad;
	}

	MESSAGE_FUNC_INT( OnCheckButtonChecked, "CheckButtonChecked", state );

private:
	Addon m_addon;
	bool  m_bSubscribed;
	bool  m_bHovered;
	bool  m_allowSubscribe;

	Panel		   *m_pImageContainer;
	ImageExtButton *m_pImageButton;
	CheckButton	   *m_pCheckbox;
	Label		   *m_pNameLabel;
	Label		   *m_pSizeLabel;
};

class CBrowsePage : public PropertyPage
{
	DECLARE_CLASS_SIMPLE( CBrowsePage, PropertyPage );

public:
	CBrowsePage( Panel *parent, const char *panelName );
	~CBrowsePage();

	virtual void OnPageShow();
	virtual void PerformLayout();

	void RefreshList();
	void PopulateGrid();

	virtual void OnMouseWheeled( int delta ) OVERRIDE;

	void ApplySubscriptionChanges();

	MESSAGE_FUNC( OnRefreshClicked, "RefreshClicked" );
	MESSAGE_FUNC_PARAMS( OnScrollBarMoved, "ScrollBarSliderMoved", pKV );
	MESSAGE_FUNC_PARAMS( OnThumbReady, "ThumbReady", kv );

private:
	CUtlVector< CAddonThumbnailPanel * > m_AddonPanels;

	EditablePanel *m_pViewport;
	EditablePanel *m_pContentPanel;
	ScrollBar	  *m_pVScroll;

	int m_lastContentWidth;
	int m_contentTall;

	void   ComputeLayoutPositions();
	void   UpdatePositions();
	Panel *GetVScrollBar()
	{
		return (Panel *)m_pVScroll;
	}

	struct TileRect
	{
		int x, y, w, h;
	};
	CUtlVector< TileRect > m_tileRects;
};

class CSubscribedPage : public PropertyPage
{
	DECLARE_CLASS_SIMPLE( CSubscribedPage, PropertyPage );

public:
	CSubscribedPage( Panel *parent, const char *panelName );
	~CSubscribedPage();

	virtual void OnPageShow();
	virtual void PerformLayout();

	void RefreshList();
	void PopulateGrid();

	virtual void OnMouseWheeled( int delta ) OVERRIDE;

	MESSAGE_FUNC( OnRefreshClicked, "RefreshClicked" );
	MESSAGE_FUNC_PARAMS( OnScrollBarMoved, "ScrollBarSliderMoved", pKV );

private:
	CUtlVector< CAddonThumbnailPanel * > m_AddonPanels;

	EditablePanel *m_pViewport;
	EditablePanel *m_pContentPanel;
	ScrollBar	  *m_pVScroll;

	int m_lastContentWidth;
	int m_contentTall;

	void   ComputeLayoutPositions();
	void   UpdatePositions();
	Panel *GetVScrollBar()
	{
		return (Panel *)m_pVScroll;
	}

	struct TileRect
	{
		int x, y, w, h;
	};
	CUtlVector< TileRect > m_tileRects;
};

class CWorkshopDialog : public PropertyDialog
{
	DECLARE_CLASS_SIMPLE( CWorkshopDialog, PropertyDialog );

public:
	CWorkshopDialog( Panel *parent );
	~CWorkshopDialog();

	virtual void Activate();
	virtual void PerformLayout();

	virtual void ApplyChanges() OVERRIDE;
	virtual void OnCommand( const char *cmd ) OVERRIDE;

public:
	CBrowsePage		*m_pBrowsePage;
	CSubscribedPage *m_pSubscribedPage;
};

CWorkshopDialog *GetWorkshopDialog();
void			 ShowWorkshop();

#endif // WORKSHOP_UI_H
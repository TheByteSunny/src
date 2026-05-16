//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "bugreporter_ui.h"
#include "webmanager.h"
#include "sbpp_globaldef.h"
#include "id.h"

#include <cstdio>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>
#include "ienginevgui.h"
#include <vgui/IVGui.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

const char *CBugReportPanel::kBugReportUrl = "https://workshop-bot.sbpp-workshop.workers.dev/bug-report";

static const int kTitleMin = 3;
static const int kTitleMax = 120;
static const int kDescMin = 10;
static const int kDescMax = 4000;
static const int kReporterMax = 64;
static const int kAddonIdMax = 32;
static const int kVersionMax = 32;

static void GetEntryText( TextEntry *pEntry, char *out, int outSize )
{
	if ( !pEntry )
	{
		out[0] = 0;
		return;
	}
	pEntry->GetText( out, outSize );
}

static void JsonEscape( const char *src, char *dst, int dstSize )
{
	int j = 0;
	for ( int i = 0; src[i] && j < dstSize - 7; i++ )
	{
		unsigned char c = (unsigned char)src[i];
		switch ( c )
		{
		case '\"':
			dst[j++] = '\\';
			dst[j++] = '\"';
			break;
		case '\\':
			dst[j++] = '\\';
			dst[j++] = '\\';
			break;
		case '\n':
			dst[j++] = '\\';
			dst[j++] = 'n';
			break;
		case '\r':
			dst[j++] = '\\';
			dst[j++] = 'r';
			break;
		case '\t':
			dst[j++] = '\\';
			dst[j++] = 't';
			break;
		case '\b':
			dst[j++] = '\\';
			dst[j++] = 'b';
			break;
		case '\f':
			dst[j++] = '\\';
			dst[j++] = 'f';
			break;
		default:
			if ( c < 0x20 )
				j += Q_snprintf( dst + j, dstSize - j, "\\u%04x", c );
			else
				dst[j++] = c;
		}
	}
	dst[j] = 0;
}

static bool JsonExtractString( const char *json, const char *key, char *out, int outSize )
{
	if ( !json || !key || !out || outSize <= 0 )
		return false;

	out[0] = 0;

	char needle[64];
	Q_snprintf( needle, sizeof( needle ), "\"%s\"", key );

	const char *p = Q_stristr( json, needle );
	if ( !p )
		return false;

	p += Q_strlen( needle );
	while ( *p && ( *p == ' ' || *p == '\t' || *p == ':' ) )
		p++;
	if ( *p != '\"' )
		return false;
	p++;

	int j = 0;
	while ( *p && *p != '\"' && j < outSize - 1 )
	{
		if ( *p == '\\' && p[1] )
		{
			char esc = p[1];
			if ( esc == 'n' )
				out[j++] = '\n';
			else if ( esc == 't' )
				out[j++] = '\t';
			else if ( esc == 'r' )
				out[j++] = '\r';
			else if ( esc == '\"' )
				out[j++] = '\"';
			else if ( esc == '\\' )
				out[j++] = '\\';
			else
				out[j++] = esc;
			p += 2;
		}
		else
		{
			out[j++] = *p++;
		}
	}
	out[j] = 0;
	return true;
}

CBugReportPanel::CBugReportPanel( Panel *parent ) : BaseClass( parent, "BugReportPanel" )
{
	SetProportional( true );

	SetTitle( "#SBPP_BugReport_Title", true );
	SetSizeable( false );
	SetMoveable( true );
	SetDeleteSelfOnClose( false );
	SetVisible( false );

	m_bSubmitting = false;

	m_pLblName = new Label( this, "LblName", "#SBPP_BugReport_Name" );
	m_pName = new TextEntry( this, "Name" );
	m_pName->SetMaximumCharCount( kReporterMax );

	m_pLblTitle = new Label( this, "LblTitle", "#SBPP_BugReport_BugTitle" );
	m_pTitle = new TextEntry( this, "Title" );
	m_pTitle->SetMaximumCharCount( kTitleMax );

	m_pLblAddonId = new Label( this, "LblAddonId", "#SBPP_BugReport_AddonId" );
	m_pAddonId = new TextEntry( this, "AddonId" );
	m_pAddonId->SetMaximumCharCount( kAddonIdMax );

	m_pLblDescription = new Label( this, "LblDescription", "#SBPP_BugReport_Description" );
	m_pDescription = new TextEntry( this, "Description" );
	m_pDescription->SetMultiline( true );
	m_pDescription->SetCatchEnterKey( true );
	m_pDescription->SetVerticalScrollbar( true );
	m_pDescription->SetMaximumCharCount( kDescMax );

	m_pSubmit = new Button( this, "Submit", "#SBPP_BugReport_Submit", this, "submit" );
	m_pCancel = new Button( this, "Cancel", "#SBPP_BugReport_Cancel", this, "cancel" );

	LoadControlSettings( "resource/ui/BugReportPanel.res" );
	SetVisible( false );
}

CBugReportPanel::~CBugReportPanel()
{
}

void CBugReportPanel::Activate()
{
	BaseClass::Activate();
	MoveToCenterOfScreen();
	RequestFocus();
	if ( m_pName )
		m_pName->RequestFocus();
}

void CBugReportPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void CBugReportPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CBugReportPanel::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "cancel" ) )
	{
		ClearForm();
		SetVisible( false );
		return;
	}
	if ( !Q_stricmp( command, "submit" ) )
	{
		SubmitReport();
		return;
	}
	BaseClass::OnCommand( command );
}

void CBugReportPanel::OnSubmitDoneInfo( const char *text )
{
	m_bSubmitting = false;
	m_pSubmit->SetEnabled( true );
	m_pCancel->SetEnabled( true );

	ShowInfo( "#SBPP_BugReport_OkTitle", text );
	ClearForm();
	SetVisible( false );
}

void CBugReportPanel::OnSubmitDoneError( const char *text )
{
	m_bSubmitting = false;
	m_pSubmit->SetEnabled( true );
	m_pCancel->SetEnabled( true );

	ShowError( "#SBPP_BugReport_ErrTitle", text );
}

void CBugReportPanel::ClearForm()
{
	m_pName->SetText( "" );
	m_pTitle->SetText( "" );
	m_pAddonId->SetText( "" );
	m_pDescription->SetText( "" );
}

void CBugReportPanel::ShowInfo( const char *title, const char *text )
{
	MessageBox *pBox = new MessageBox( title, text, NULL );
	pBox->SetParent( enginevgui->GetPanel( PANEL_GAMEUIDLL ) );
	pBox->MakePopup( false, true );
	pBox->MoveToFront();
	pBox->SetOKButtonVisible( true );
	pBox->SetCloseButtonVisible( false );
	pBox->DoModal();
}

void CBugReportPanel::ShowError( const char *title, const char *text )
{
	MessageBox *pBox = new MessageBox( title, text, NULL );
	pBox->SetParent( enginevgui->GetPanel( PANEL_GAMEUIDLL ) );
	pBox->MakePopup( false, true );
	pBox->MoveToFront();
	pBox->SetOKButtonVisible( true );
	pBox->SetCloseButtonVisible( false );
	pBox->DoModal();
}

void CBugReportPanel::SubmitReport()
{
	if ( m_bSubmitting )
		return;

	char name[128], title[256], addon[64];
	char description[kDescMax + 16];

	GetEntryText( m_pName, name, sizeof( name ) );
	GetEntryText( m_pTitle, title, sizeof( title ) );
	GetEntryText( m_pAddonId, addon, sizeof( addon ) );
	GetEntryText( m_pDescription, description, sizeof( description ) );

	int nameLen = Q_strlen( name );
	int titleLen = Q_strlen( title );
	int descLen = Q_strlen( description );

	if ( nameLen < 1 )
	{
		ShowError( "#SBPP_BugReport_ErrTitle", "Please enter your name." );
		return;
	}
	if ( titleLen < kTitleMin || titleLen > kTitleMax )
	{
		char buf[128];
		Q_snprintf( buf, sizeof( buf ), "Title must be %d-%d characters.", kTitleMin, kTitleMax );
		ShowError( "#SBPP_BugReport_ErrTitle", buf );
		return;
	}
	if ( descLen < kDescMin || descLen > kDescMax )
	{
		char buf[128];
		Q_snprintf( buf, sizeof( buf ), "Description must be %d-%d characters.", kDescMin, kDescMax );
		ShowError( "#SBPP_BugReport_ErrTitle", buf );
		return;
	}

	if ( !g_pWebManager )
		return;

	char eName[256], eTitle[512], eAddon[128];
	char eDesc[kDescMax * 2 + 16];

	JsonEscape( name, eName, sizeof( eName ) );
	JsonEscape( title, eTitle, sizeof( eTitle ) );
	JsonEscape( addon, eAddon, sizeof( eAddon ) );
	JsonEscape( description, eDesc, sizeof( eDesc ) );

	char reporter[256];
	const char* userID = CUserID::Get().GetID();

	Q_snprintf(reporter, sizeof(reporter),
		"%s (%s)",
		eName,
		userID ? userID : "unknown"
	);

	char body[kDescMax * 2 + 1024];
	Q_snprintf( body, sizeof( body ),
		"{"
		"\"reporter\":\"%s\","
		"\"title\":\"%s\","
		"\"description\":\"%s\","
		"\"addon_id\":\"%s\","
		"\"version\":\"%s\""
		"}",
		reporter, eTitle, eDesc, eAddon, SBPP_VERSION );

	m_bSubmitting = true;
	m_pSubmit->SetEnabled( false );
	m_pCancel->SetEnabled( false );

	VPANEL hSelf = GetVPanel();

	g_pWebManager->Post( kBugReportUrl, body,
		[hSelf]( const WebResult_t &r )
		{
			char msg[512];

			bool success = ( r.httpCode >= 200 && r.httpCode < 300 );

			if ( success )
			{
				Q_strncpy( msg, "Bug report sent! Thank you.", sizeof( msg ) );

				KeyValues *kv = new KeyValues( "SubmitDoneInfo" );
				kv->SetString( "text", msg );
				vgui::ivgui()->PostMessage( hSelf, kv, NULL );
			}
			else
			{
				char serverErr[256] = { 0 };
				if ( !r.body.empty() && JsonExtractString( r.body.c_str(), "error", serverErr, sizeof( serverErr ) ) && serverErr[0] )
				{
					Q_snprintf( msg, sizeof( msg ), "%s", serverErr );
				}
				else if ( r.httpCode == 0 )
				{
					Q_strncpy( msg, "Couldn't reach the server. Check your internet connection.", sizeof( msg ) );
				}
				else if ( r.httpCode == 429 )
				{
					Q_strncpy( msg, "Too many reports. Please wait and try again.", sizeof( msg ) );
				}
				else if ( r.httpCode == 409 )
				{
					Q_strncpy( msg, "Duplicate report (already submitted recently).", sizeof( msg ) );
				}
				else if ( r.httpCode >= 500 )
				{
					Q_snprintf( msg, sizeof( msg ), "Server error (HTTP %d). Try again later.", r.httpCode );
				}
				else
				{
					Q_snprintf( msg, sizeof( msg ), "Failed to send (HTTP %d).", r.httpCode );
				}

				KeyValues *kv = new KeyValues( "SubmitDoneError" );
				kv->SetString( "text", msg );
				vgui::ivgui()->PostMessage( hSelf, kv, NULL );
			}
		} );
}

//---

static CBugReportPanel *g_pBugReportPanel = NULL;

CON_COMMAND( sbpp_bugreport, "Open the bug report panel" )
{
	if ( !g_pBugReportPanel )
	{
		VPANEL gameUiRoot = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		g_pBugReportPanel = new CBugReportPanel( NULL );
		g_pBugReportPanel->SetParent( gameUiRoot );
	}

	g_pBugReportPanel->Activate();
	g_pBugReportPanel->SetVisible( true );
	g_pBugReportPanel->MoveToFront();
	g_pBugReportPanel->MakePopup( false, true );
}

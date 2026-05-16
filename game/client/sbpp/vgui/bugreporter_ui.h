//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//=============================================================================//
#ifndef BUGREPORTER_UI_H
#define BUGREPORTER_UI_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

namespace vgui
{
class Label;
class TextEntry;
class Button;
} // namespace vgui

class CBugReportPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CBugReportPanel, vgui::Frame );

public:
	CBugReportPanel( vgui::Panel *parent );
	virtual ~CBugReportPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
	virtual void PerformLayout() OVERRIDE;
	virtual void OnCommand( const char *command ) OVERRIDE;
	virtual void Activate() OVERRIDE;

private:
	void SubmitReport();
	void ClearForm();

	void ShowInfo( const char *title, const char *text );
	void ShowError( const char *title, const char *text );

	void OnSubmitResult( bool success, int statusCode, const char *errorMsg );

	MESSAGE_FUNC_CHARPTR( OnSubmitDoneInfo,  "SubmitDoneInfo",  text );
	MESSAGE_FUNC_CHARPTR( OnSubmitDoneError, "SubmitDoneError", text );

	static const char *kBugReportUrl;

	vgui::Label		*m_pLblName;
	vgui::TextEntry *m_pName;

	vgui::Label		*m_pLblTitle;
	vgui::TextEntry *m_pTitle;

	vgui::Label		*m_pLblAddonId;
	vgui::TextEntry *m_pAddonId;

	vgui::Label		*m_pLblVersion;
	vgui::TextEntry *m_pVersion;

	vgui::Label		*m_pLblDescription;
	vgui::TextEntry *m_pDescription;

	vgui::Button *m_pSubmit;
	vgui::Button *m_pCancel;

	bool m_bSubmitting;
};

#endif // BUGREPORTER_UI_H
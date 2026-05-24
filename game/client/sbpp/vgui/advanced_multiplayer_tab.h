//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef ADVANCED_MP_TAB_H
#define ADVANCED_MP_TAB_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <matsys_controls/mdlpanel.h>
#include <vgui_controls/ComboBox.h>
#include <matsys_controls/colorpickerpanel.h>
#include <vgui/ISurface.h>
#include <vector>
#include <string>

struct ColorPreset
{
	const char *name;
	int			r, g, b;
};

struct HandModelInfo
{
	std::string key;
	std::string model;
	int			skin;
	std::string name;
};

class CColorPickerForwardPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CColorPickerForwardPanel, vgui::Panel );

public:
	CColorPickerForwardPanel( vgui::Panel *parent, vgui::Panel *pForwardTo, int target ) : vgui::Panel( parent, "ColorPickerForward" ), m_pForwardTo( pForwardTo ), m_iTarget( target )
	{
	}

	MESSAGE_FUNC_PARAMS( OnColorPicked_Internal, "ColorPickerPicked", data )
	{
		if ( !data || !m_pForwardTo )
			return;

		KeyValues *pKV = new KeyValues( "ColorPickerPicked" );
		pKV->SetInt( "r", data->GetInt( "r", -1 ) );
		pKV->SetInt( "g", data->GetInt( "g", -1 ) );
		pKV->SetInt( "b", data->GetInt( "b", -1 ) );
		pKV->SetInt( "color", data->GetInt( "color", -1 ) );
		pKV->SetInt( "target", m_iTarget );

		PostMessage( m_pForwardTo, pKV );
	}

private:
	vgui::Panel *m_pForwardTo;
	int			 m_iTarget;
};

class CMDLPanelAdv : public CMDLPanel
{
	DECLARE_CLASS_SIMPLE( CMDLPanelAdv, CMDLPanel );

public:
	CMDLPanelAdv( vgui::Panel *pParent, const char *pName );

	virtual void SetMDL( const char *pMDLName, void *pProxyData = NULL );
	virtual void OnTick();

	void PlayActivity( Activity activity );

private:
	virtual void PrePaint3D( IMatRenderContext *pRenderContext );
};

class CAdvancedOptionsMultiplayer : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CAdvancedOptionsMultiplayer, vgui::PropertyPage );

public:
	CAdvancedOptionsMultiplayer( vgui::Panel *parent, const char *panelName );

	enum EColorTarget
	{
		COLORTARGET_NONE = -1,
		COLORTARGET_PLAYER = 0,
		COLORTARGET_WEAPON = 1
	};

protected:
	virtual void OnCommand( const char *command );
	virtual void PerformLayout();

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", kv );

	void		 PopulatePlayerModels();
	virtual void OnTick();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;

	MESSAGE_FUNC_PARAMS( OnColorPicked, "ColorPickerPicked", data );

private:
	vgui::Label				  *m_pNameLabel;
	vgui::TextEntry			  *m_pNameEntry;
	CMDLPanelAdv			  *m_pPMModel;
	vgui::Panel				  *m_pVerticalSeparator;
	vgui::ComboBox			  *m_pPMSelector;
	std::vector< std::string > m_PMPaths;
	int						   m_iLastPMIndex;

	CColorPickerButton *m_pPlayerColorBtn;
	CColorPickerButton *m_pWeaponColorBtn;

	CColorPickerForwardPanel *m_pPlayerForward;
	CColorPickerForwardPanel *m_pWeaponForward;

	vgui::Label *m_pPlayerColorLabel;
	vgui::Label *m_pWeaponColorLabel;
	vgui::Panel *m_pHorizontalSeparator;

	CUtlVector< vgui::Button * > m_PlayerPresetBtns;
	CUtlVector< vgui::Button * > m_WeaponPresetBtns;

	vgui::Label *m_pPlayerPresetsLabel;
	vgui::Label *m_pWeaponPresetsLabel;

	vgui::ComboBox				*m_pHandModelSelector;
	std::vector< HandModelInfo > m_HandModels;

	vgui::Button *m_pRefreshPMBtn;

	bool m_bFirstInit;
};

#endif
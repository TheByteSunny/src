//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "advanced_multiplayer_tab.h"
#include "advanced_options.h"
#include "animation.h"
#include <vgui/ISurface.h>
#include "filesystem.h"
#include "fmtstr.h"
#include "luamanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static bool s_bPopulatingNameField = false;
static int	s_iLastHandIndex = -1;

#define DEFAULT_PM "models/player/kleiner.mdl"
#define CFG_PLUS "cfg/plus_settings.cfg"

static void SaveSettingsToFile( const char *playerName, const char *handModel )
{
	KeyValues *kv = new KeyValues( "PlusSettings" );

	kv->SetString( "PlayerName", playerName ? playerName : "" );
	kv->SetString( "HandModel", handModel ? handModel : "" );

	kv->SaveToFile( g_pFullFileSystem, CFG_PLUS );
	kv->deleteThis();
}

static void LoadSettingsFromFile( CUtlString &outPlayerName, CUtlString &outHandModel )
{
	outPlayerName = "";
	outHandModel = "";

	KeyValues *kv = new KeyValues( "PlusSettings" );
	if ( kv->LoadFromFile( g_pFullFileSystem, CFG_PLUS ) )
	{
		const char *name = kv->GetString( "PlayerName", "" );
		const char *hand = kv->GetString( "HandModel", "" );

		outPlayerName = name ? name : "";
		outHandModel = hand ? hand : "";
	}
	kv->deleteThis();
}

static bool LoadPMCache( std::vector< std::string > &out )
{
	out.clear();
	FileHandle_t fh = g_pFullFileSystem->Open( "cache/pm_cache.txt", "r" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
		return false;

	char line[MAX_PATH];
	while ( g_pFullFileSystem->ReadLine( line, sizeof( line ), fh ) )
	{
		int len = Q_strlen( line );
		while ( len > 0 && ( line[len - 1] == '\n' || line[len - 1] == '\r' ) )
			line[--len] = '\0';
		if ( len > 0 )
			out.push_back( line );
	}
	g_pFullFileSystem->Close( fh );
	return !out.empty();
}

static void SavePMCache( const std::vector< std::string > &paths )
{
	FileHandle_t fh = g_pFullFileSystem->Open( "cache/pm_cache.txt", "w" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
		return;
	for ( auto &p : paths )
	{
		g_pFullFileSystem->Write( p.c_str(), p.size(), fh );
		g_pFullFileSystem->Write( "\n", 1, fh );
	}
	g_pFullFileSystem->Close( fh );
}

ColorPreset GlobalPresets[] = { { "Red", 255, 0, 0 }, { "Green", 0, 255, 0 }, { "Blue", 0, 0, 255 }, { "White", 255, 255, 255 }, { "Cyan", 38, 207, 232 }, { "Purple", 128, 0, 128 }, { "Yellow", 255, 255, 0 }, { "Orange", 255, 128, 0 } };

static void LoadHandModelsFromLua( std::vector< HandModelInfo > &out )
{
	out.clear();
	out.reserve( 32 );

	lua_State *Lstate = ( LGameUI ? LGameUI : L );

	lua_getglobal( Lstate, "HandModels" );
	if ( !lua_istable( Lstate, -1 ) )
	{
		lua_pop( Lstate, 1 );
		return;
	}

	auto extract_model = [&]( const char *key_override ) -> bool
	{
		lua_getfield( Lstate, -1, "model" );
		if ( !lua_isstring( Lstate, -1 ) )
		{
			lua_pop( Lstate, 1 );
			return false;
		}

		HandModelInfo info;
		info.model = lua_tostring( Lstate, -1 );
		lua_pop( Lstate, 1 );

		lua_getfield( Lstate, -1, "key" );
		if ( lua_isstring( Lstate, -1 ) )
			info.key = lua_tostring( Lstate, -1 );
		else if ( key_override )
			info.key = key_override;
		lua_pop( Lstate, 1 );

		lua_getfield( Lstate, -1, "skin" );
		info.skin = lua_isnumber( Lstate, -1 ) ? (int)lua_tointeger( Lstate, -1 ) : 0;
		lua_pop( Lstate, 1 );

		lua_getfield( Lstate, -1, "name" );
		if ( lua_isstring( Lstate, -1 ) )
			info.name = lua_tostring( Lstate, -1 );
		else if ( !info.key.empty() )
			info.name = info.key;
		else if ( key_override )
			info.name = key_override;
		lua_pop( Lstate, 1 );

		out.push_back( info );
		return true;
	};

	lua_rawgeti( Lstate, -1, 1 );
	bool is_array = !lua_isnil( Lstate, -1 );
	lua_pop( Lstate, 1 );

	if ( is_array )
	{
		int	 idx = 1;
		char key_buf[32];

		while ( true )
		{
			lua_rawgeti( Lstate, -1, idx );
			if ( lua_isnil( Lstate, -1 ) )
			{
				lua_pop( Lstate, 1 );
				break;
			}

			if ( lua_istable( Lstate, -1 ) )
			{
				Q_snprintf( key_buf, sizeof( key_buf ), "%d", idx );

				if ( !extract_model( key_buf ) )
				{
					lua_rawgeti( Lstate, -1, 1 );
					bool is_nested_array = !lua_isnil( Lstate, -1 );
					lua_pop( Lstate, 1 );

					if ( is_nested_array )
					{
						int	 sub_idx = 1;
						char sub_key[64];
						while ( true )
						{
							lua_rawgeti( Lstate, -1, sub_idx );
							if ( lua_isnil( Lstate, -1 ) )
							{
								lua_pop( Lstate, 1 );
								break;
							}

							if ( lua_istable( Lstate, -1 ) )
							{
								Q_snprintf( sub_key, sizeof( sub_key ), "%d.%d", idx, sub_idx );
								extract_model( sub_key );
							}
							lua_pop( Lstate, 1 );
							sub_idx++;
						}
					}
					else
					{
						lua_pushnil( Lstate );
						while ( lua_next( Lstate, -2 ) != 0 )
						{
							if ( lua_istable( Lstate, -1 ) && lua_type( Lstate, -2 ) == LUA_TSTRING )
							{
								extract_model( lua_tostring( Lstate, -2 ) );
							}
							lua_pop( Lstate, 1 );
						}
					}
				}
			}
			lua_pop( Lstate, 1 );
			idx++;
		}
	}
	else
	{
		lua_pushnil( Lstate );
		while ( lua_next( Lstate, -2 ) != 0 )
		{
			const char *top_key = ( lua_type( Lstate, -2 ) == LUA_TSTRING ) ? lua_tostring( Lstate, -2 ) : nullptr;

			if ( top_key && lua_istable( Lstate, -1 ) )
			{
				// Try direct extraction
				if ( !extract_model( top_key ) )
				{
					// Process nested hash table
					lua_pushnil( Lstate );
					while ( lua_next( Lstate, -2 ) != 0 )
					{
						if ( lua_istable( Lstate, -1 ) )
						{
							const char *sub_key = ( lua_type( Lstate, -2 ) == LUA_TSTRING ) ? lua_tostring( Lstate, -2 ) : nullptr;

							if ( sub_key )
							{
								lua_getfield( Lstate, -1, "name" );
								std::string sub_name = lua_isstring( Lstate, -1 ) ? lua_tostring( Lstate, -1 ) : sub_key;
								lua_pop( Lstate, 1 );

								if ( extract_model( sub_key ) && !out.empty() )
								{
									out.back().name = std::string( top_key ) + " - " + sub_name;
								}
							}
						}
						lua_pop( Lstate, 1 );
					}
				}
			}
			lua_pop( Lstate, 1 );
		}
	}

	lua_pop( Lstate, 1 );
}

static void ApplyColorToConvarsByTarget( int target, int r, int g, int b )
{
	if ( target == CAdvancedOptionsMultiplayer::COLORTARGET_PLAYER )
	{
		static ConVarRef playercolor_r( "playercolor_r" );
		static ConVarRef playercolor_g( "playercolor_g" );
		static ConVarRef playercolor_b( "playercolor_b" );

		if ( playercolor_r.IsValid() )
			playercolor_r.SetValue( r );

		if ( playercolor_g.IsValid() )
			playercolor_g.SetValue( g );

		if ( playercolor_b.IsValid() )
			playercolor_b.SetValue( b );
	}
	else if ( target == CAdvancedOptionsMultiplayer::COLORTARGET_WEAPON )
	{
		static ConVarRef physgun_r( "physgun_r" );
		static ConVarRef physgun_g( "physgun_g" );
		static ConVarRef physgun_b( "physgun_b" );

		if ( physgun_r.IsValid() )
			physgun_r.SetValue( r );

		if ( physgun_g.IsValid() )
			physgun_g.SetValue( g );

		if ( physgun_b.IsValid() )
			physgun_b.SetValue( b );
	}
}

CMDLPanelAdv::CMDLPanelAdv( vgui::Panel *pParent, const char *pName ) : CMDLPanel( pParent, pName )
{
	SetSkin( 0 );
	SetLookAtCamera( true );
	SetGroundGrid( true );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 10 );
}

void CMDLPanelAdv::SetMDL( const char *pMDLName, void *pProxyData )
{
	if ( !pMDLName || !*pMDLName )
		return;

	MDLHandle_t h = mdlcache->FindMDL( pMDLName );
	if ( h == MDLHANDLE_INVALID )
		return;

	studiohdr_t *pHdr = mdlcache->GetStudioHdr( h );
	if ( !pHdr )
	{
		mdlcache->Release( h );
		return;
	}

	mdlcache->GetVirtualModel( h );

	BaseClass::SetMDL( pMDLName, pProxyData );

	PlayActivity( ACT_HL2MP_IDLE ); // HACK!!

	m_RootMDL.m_flCycleStartTime = Plat_FloatTime();

	mdlcache->Release( h );
}

void CMDLPanelAdv::PrePaint3D( IMatRenderContext *pRenderContext )
{
	//BaseClass::PrePaint3D( pRenderContext );

	StudioRenderConfig_t cfg;
	g_pStudioRender->GetCurrentConfig( cfg );

	cfg.drawEntities = 1;
	cfg.bTeeth = true;
	cfg.bEyes = true;
	cfg.bFlex = true;
	cfg.bEyeMove = true;
	cfg.bWireframe = false;
	cfg.bDrawNormals = false;
	cfg.bDrawTangentFrame = false;
	cfg.bNoHardware = false;
	cfg.bNoSoftware = false;

	g_pStudioRender->UpdateConfig( cfg );
}

void CMDLPanelAdv::OnTick()
{
	BaseClass::OnTick();

	static float s_flBaseRealTime = Plat_FloatTime();
	float		 flElapsed = Plat_FloatTime() - s_flBaseRealTime;

	m_RootMDL.m_flCycleStartTime = gpGlobals->curtime - flElapsed;

	Repaint();
}

void CMDLPanelAdv::PlayActivity( Activity activity )
{
	if ( m_RootMDL.m_MDL.GetMDL() == MDLHANDLE_INVALID )
		return;

	studiohdr_t *pHdr = mdlcache->GetStudioHdr( m_RootMDL.m_MDL.GetMDL() );
	if ( !pHdr )
		return;

	CStudioHdr studioHdr( pHdr, mdlcache );

	int iSeq = SelectWeightedSequence( &studioHdr, activity );
	if ( iSeq < 0 )
		iSeq = 0;

	SetSequence( iSeq, true );

	m_RootMDL.m_flCycleStartTime = Plat_FloatTime();
}

CAdvancedOptionsMultiplayer::CAdvancedOptionsMultiplayer( Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName ),
	m_pPlayerColorBtn( nullptr ),
	m_pWeaponColorBtn( nullptr ),
	m_pPlayerForward( nullptr ),
	m_pWeaponForward( nullptr )
{
	SetProportional( true );

	m_pNameLabel = new Label( this, "NameLabel", "Player Name:" );
	m_pNameEntry = new TextEntry( this, "NameEntry" );
	m_pPMModel = new CMDLPanelAdv( this, "PMModel" );

	m_pNameEntry->SetAllowNonAsciiCharacters( true );
	m_pNameEntry->SetMaximumCharCount( 32 );
	m_pNameEntry->AddActionSignalTarget( this );

	m_pPMSelector = new ComboBox( this, "PMSelector", 8, false );
	m_pPMSelector->SetAllowNonAsciiCharacters( false );

	m_pPlayerForward = new CColorPickerForwardPanel( this, this, COLORTARGET_PLAYER );
	m_pWeaponForward = new CColorPickerForwardPanel( this, this, COLORTARGET_WEAPON );

	m_pPlayerColorBtn = new CColorPickerButton( this, "PlayerColorBtn", m_pPlayerForward );
	m_pPlayerColorBtn->SetTooltip( nullptr, "Change the color of the player character" );

	m_pWeaponColorBtn = new CColorPickerButton( this, "WeaponColorBtn", m_pWeaponForward );
	m_pWeaponColorBtn->SetTooltip( nullptr, "Change the color of the physics gun" );

	m_pPlayerColorLabel = new Label( this, "PlayerColorLabel", "Player Color" );
	m_pWeaponColorLabel = new Label( this, "WeaponColorLabel", "Weapon Color" );

	m_pVerticalSeparator = new Panel( this, "VerticalSeparator" );
	m_pVerticalSeparator->SetBgColor( Color( 128, 128, 128, 255 ) ); // gray

	m_pHorizontalSeparator = new Panel( this, "HorizontalSeparator" );
	m_pHorizontalSeparator->SetBgColor( Color( 128, 128, 128, 255 ) ); // gray

	m_pPlayerPresetsLabel = new Label( this, "PlayerPresetsLabel", "Player Color Presets" );
	m_pWeaponPresetsLabel = new Label( this, "WeaponPresetsLabel", "Weapon Color Presets" );

	m_pHandModelSelector = new ComboBox( this, "HandModelSelector", 4, false );

	m_pRefreshPMBtn = new Button( this, "RefreshPMBtn", "Refresh", this, "RefreshModels" );

	for ( int i = 0; i < ARRAYSIZE( GlobalPresets ); i++ )
	{
		char buf[256];
		Q_snprintf( buf, sizeof( buf ), "PlayerPresetBtn%d", i );
		Button *btn = new Button( this, buf, GlobalPresets[i].name, this, buf );
		btn->SetCommand( buf );
		btn->SetFgColor( Color( GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255 ) );
		m_PlayerPresetBtns.AddToTail( btn );
	}

	for ( int i = 0; i < ARRAYSIZE( GlobalPresets ); i++ )
	{
		char buf[256];
		Q_snprintf( buf, sizeof( buf ), "WeaponPresetBtn%d", i );
		Button *btn = new Button( this, buf, GlobalPresets[i].name, this, buf );
		btn->SetCommand( buf );
		btn->SetFgColor( Color( GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255 ) );
		m_WeaponPresetBtns.AddToTail( btn );
	}

	m_bFirstInit = false;

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

void CAdvancedOptionsMultiplayer::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

    if ( m_bFirstInit )
        return;

    m_bFirstInit = true;

	PopulatePlayerModels();

	if ( m_pPlayerColorBtn )
	{
		static ConVarRef playercolor_r( "playercolor_r" );
		static ConVarRef playercolor_g( "playercolor_g" );
		static ConVarRef playercolor_b( "playercolor_b" );

		int pr = playercolor_r.IsValid() ? playercolor_r.GetInt() : 128;
		int pg = playercolor_g.IsValid() ? playercolor_g.GetInt() : 128;
		int pb = playercolor_b.IsValid() ? playercolor_b.GetInt() : 128;

		m_pPlayerColorBtn->SetColor( pr, pg, pb, 255 );
	}

	if ( m_pWeaponColorBtn )
	{
		static ConVarRef physgun_r( "physgun_r" );
		static ConVarRef physgun_g( "physgun_g" );
		static ConVarRef physgun_b( "physgun_b" );

		int wr = physgun_r.IsValid() ? physgun_r.GetInt() : 128;
		int wg = physgun_g.IsValid() ? physgun_g.GetInt() : 128;
		int wb = physgun_b.IsValid() ? physgun_b.GetInt() : 128;

		m_pWeaponColorBtn->SetColor( wr, wg, wb, 255 );
	}

	m_pHandModelSelector->DeleteAllItems();
	m_HandModels.clear();

	LoadHandModelsFromLua( m_HandModels );

	if ( m_HandModels.empty() )
	{
		struct HandModelEntry
		{
			const char *key;
			const char *path;
			const char *name;
			int			skin;
		};
		HandModelEntry models[] = { { "citizen", "models/weapons/c_arms_citizen.mdl", "Citizen", 0 } };

		for ( int i = 0; i < ARRAYSIZE( models ); ++i )
		{
			HandModelInfo info;
			info.key = models[i].key;
			info.model = models[i].path;
			info.skin = models[i].skin;
			info.name = models[i].name;
			m_HandModels.push_back( info );
		}
	}

	for ( size_t i = 0; i < m_HandModels.size(); ++i )
	{
		m_pHandModelSelector->AddItem( m_HandModels[i].name.c_str(), nullptr );
	}

	CUtlString dummyName, savedHandKey;
	LoadSettingsFromFile( dummyName, savedHandKey );

	int selIndex = 0;
	if ( savedHandKey.Length() > 0 )
	{
		for ( size_t i = 0; i < m_HandModels.size(); ++i )
		{
			if ( Q_stricmp( savedHandKey.Get(), m_HandModels[i].key.c_str() ) == 0 )
			{
				selIndex = (int)i;
				break;
			}
		}
	}
	else
	{
		static ConVarRef c_handmodel( "c_handmodel" );
		const char		*pHandModel = c_handmodel.IsValid() ? c_handmodel.GetString() : "citizen";

		for ( size_t i = 0; i < m_HandModels.size(); ++i )
		{
			if ( Q_stricmp( pHandModel, m_HandModels[i].key.c_str() ) == 0 )
			{
				selIndex = (int)i;
				break;
			}
		}
	}

	m_pHandModelSelector->ActivateItem( selIndex );
	s_iLastHandIndex = selIndex;

	// model
	m_pPMModel->SetGroundGrid( true );
	//m_pPMModel->SetBackgroundColor( Color( 30, 30, 30, 255 ) );

	s_bPopulatingNameField = true;
	CUtlString fileLoadedName, fileLoadedHand;
	LoadSettingsFromFile( fileLoadedName, fileLoadedHand );

	if ( fileLoadedName.Length() > 0 )
		m_pNameEntry->SetText( fileLoadedName.Get() );
	else
	{
		static ConVarRef name( "name" );

		if ( name.IsValid() && name.GetString() )
			m_pNameEntry->SetText( name.GetString() );
		else
			m_pNameEntry->SetText( "" );
	}

	s_bPopulatingNameField = false;
}

void CAdvancedOptionsMultiplayer::PopulatePlayerModels()
{
	if ( !m_PMPaths.empty() )
		return;

	if ( !m_pPMSelector )
		return;

	m_PMPaths.clear();
	m_pPMSelector->DeleteAllItems();

	auto shouldExclude = []( const char *filename ) -> bool { return V_stristr( filename, ".phy.mdl" ) != nullptr || V_stristr( filename, "_anim" ) != nullptr; };

	if ( !LoadPMCache( m_PMPaths ) )
	{
		m_PMPaths.reserve( 256 );

		std::vector< std::string > stack;
		stack.push_back( "models/player" );

		while ( !stack.empty() )
		{
			std::string dir = stack.back();
			stack.pop_back();

			FileFindHandle_t fh;
			const char		*file = g_pFullFileSystem->FindFirst( ( dir + "/*" ).c_str(), &fh );

			while ( file )
			{
				if ( file[0] != '.' )
				{
					std::string fullPath = dir + "/" + file;

					if ( g_pFullFileSystem->IsDirectory( fullPath.c_str() ) )
					{
						stack.push_back( fullPath );
					}
					else
					{
						const char *ext = Q_GetFileExtension( file );
						if ( ext && Q_stricmp( ext, "mdl" ) == 0 && !shouldExclude( file ) )
							m_PMPaths.push_back( fullPath );
					}
				}

				file = g_pFullFileSystem->FindNext( fh );
			}

			g_pFullFileSystem->FindClose( fh );
		}

		SavePMCache( m_PMPaths );
	}

	for ( auto &path : m_PMPaths )
		m_pPMSelector->AddItem( path.c_str(), nullptr );

	static ConVarRef cl_playermodel( "cl_playermodel" );

	const char *playermodel = ( cl_playermodel.IsValid() && cl_playermodel.GetString() ) ? cl_playermodel.GetString() : DEFAULT_PM;

	if ( !m_PMPaths.empty() )
	{
		char curBuf[MAX_PATH];
		Q_strncpy( curBuf, playermodel, sizeof( curBuf ) );
		V_FixSlashes( curBuf );
		Q_strlower( curBuf );

		auto normalize = []( const char *in, char *out, int outSize )
		{
			Q_strncpy( out, in, outSize );
			V_FixSlashes( out );
			Q_strlower( out );
		};

		auto ends_with = []( const char *str, const char *suffix ) -> bool
		{
			if ( !str || !suffix )
				return false;
			int slen = Q_strlen( str );
			int suflen = Q_strlen( suffix );
			if ( suflen > slen )
				return false;
			return ( Q_stricmp( str + slen - suflen, suffix ) == 0 );
		};

		int foundIndex = -1;
		for ( int i = 0; i < (int)m_PMPaths.size(); ++i )
		{
			char normalizedPath[MAX_PATH];
			normalize( m_PMPaths[i].c_str(), normalizedPath, sizeof( normalizedPath ) );
			if ( !Q_stricmp( normalizedPath, curBuf ) )
			{
				foundIndex = i;
				break;
			}
		}

		if ( foundIndex == -1 )
		{
			for ( int i = 0; i < (int)m_PMPaths.size(); ++i )
			{
				char normalizedPath[MAX_PATH];
				normalize( m_PMPaths[i].c_str(), normalizedPath, sizeof( normalizedPath ) );

				if ( ends_with( normalizedPath, curBuf ) || ends_with( curBuf, normalizedPath ) )
				{
					foundIndex = i;
					break;
				}

				const char *lastSlash = Q_strrchr( normalizedPath, '/' );
				const char *baseName = lastSlash ? lastSlash + 1 : normalizedPath;
				if ( !Q_stricmp( baseName, curBuf ) || ends_with( curBuf, baseName ) )
				{
					foundIndex = i;
					break;
				}
			}
		}

		if ( foundIndex == -1 )
		{
			for ( int i = 0; i < (int)m_PMPaths.size(); ++i )
			{
				if ( !Q_stricmp( m_PMPaths[i].c_str(), DEFAULT_PM ) )
				{
					foundIndex = i;
					break;
				}
			}

			if ( foundIndex == -1 )
				foundIndex = 0;
		}

		m_pPMSelector->ActivateItem( foundIndex );
		m_iLastPMIndex = foundIndex;

		if ( m_pPMModel )
		{
			m_pPMModel->SetMDL( m_PMPaths[foundIndex].c_str() );
			m_pPMModel->LookAtMDL();
		}
	}
}

void CAdvancedOptionsMultiplayer::OnTextChanged( KeyValues *pKeyValues )
{
	if ( s_bPopulatingNameField )
		return;

	char buf[MAX_PATH];
	m_pNameEntry->GetText( buf, sizeof( buf ) );

	char tmp[MAX_PATH];
	int	 j = 0;
	for ( int i = 0; buf[i] != '\0' && j < (int)sizeof( tmp ) - 1; ++i )
	{
		if ( buf[i] == '"' )
			continue;
		tmp[j++] = buf[i];
	}
	tmp[j] = '\0';
	Q_strncpy( buf, tmp, sizeof( buf ) );

	// trim left
	int start = 0;
	while ( buf[start] && isspace( (unsigned char)buf[start] ) )
		start++;
	if ( start > 0 )
		memmove( buf, buf + start, Q_strlen( buf + start ) + 1 );
	// trim right
	int len = Q_strlen( buf );
	while ( len > 0 && isspace( (unsigned char)buf[len - 1] ) )
	{
		buf[len - 1] = '\0';
		--len;
	}

	buf[m_pNameEntry->GetMaximumCharCount()] = '\0';

	CUtlString dummyHand;
	CUtlString currentName( buf );
	CUtlString currentHand;
	LoadSettingsFromFile( dummyHand, currentHand );
	SaveSettingsToFile( currentName.Get(), currentHand.Get() );

	static ConVarRef name( "name" );
	if ( name.IsValid() )
		name.SetValue( buf );

	char cmd[MAX_PATH];
	Q_snprintf( cmd, sizeof( cmd ), "name \"%s\"", buf );
	engine->ClientCmd_Unrestricted( cmd );
}

void CAdvancedOptionsMultiplayer::OnTick()
{
	BaseClass::OnTick();

	int sel = m_pPMSelector->GetActiveItem();
	if ( sel != m_iLastPMIndex && sel >= 0 && sel < (int)m_PMPaths.size() )
	{
		m_iLastPMIndex = sel;
		const char *modelPath = m_PMPaths[sel].c_str();

		if ( m_pPMModel )
		{
			m_pPMModel->SetMDL( modelPath );
			m_pPMModel->LookAtMDL();
		}

		char cmd[MAX_PATH * 2];
		Q_snprintf( cmd, sizeof( cmd ), "cl_playermodel \"%s\"\n", modelPath );
		engine->ClientCmd_Unrestricted( cmd );
	}

	sel = m_pHandModelSelector->GetActiveItem();
	if ( sel >= 0 && sel < (int)m_HandModels.size() )
	{
		if ( sel != s_iLastHandIndex )
		{
			s_iLastHandIndex = sel;

			// apply selected handmodel
			char cmd[256];
			Q_snprintf( cmd, sizeof( cmd ), "c_handmodel %s", m_HandModels[sel].key.c_str() );
			engine->ClientCmd_Unrestricted( cmd );

			CUtlString savedName, dummy;
			LoadSettingsFromFile( savedName, dummy );
			SaveSettingsToFile( savedName.Get(), m_HandModels[sel].key.c_str() );
		}
	}
}

void CAdvancedOptionsMultiplayer::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );

	if ( !Q_stricmp( command, "RefreshModels" ) )
	{
		g_pFullFileSystem->RemoveFile( "cache/pm_cache.txt" );
		m_PMPaths.clear();
		m_pPMSelector->DeleteAllItems();
		PopulatePlayerModels();
		return;
	}

	for ( int i = 0; i < ARRAYSIZE( GlobalPresets ); i++ )
	{
		char buf[MAX_PATH];
		Q_snprintf( buf, sizeof( buf ), "PlayerPresetBtn%d", i );
		if ( !Q_stricmp( command, buf ) )
		{
			ApplyColorToConvarsByTarget( COLORTARGET_PLAYER, GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b );
			if ( m_pPlayerColorBtn )
				m_pPlayerColorBtn->SetColor( GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255 );
			return;
		}
	}

	for ( int i = 0; i < ARRAYSIZE( GlobalPresets ); i++ )
	{
		char buf[MAX_PATH];
		Q_snprintf( buf, sizeof( buf ), "WeaponPresetBtn%d", i );
		if ( !Q_stricmp( command, buf ) )
		{
			ApplyColorToConvarsByTarget( COLORTARGET_WEAPON, GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b );
			if ( m_pWeaponColorBtn )
				m_pWeaponColorBtn->SetColor( GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255 );
			return;
		}
	}
}

void CAdvancedOptionsMultiplayer::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = GetWide();
	int h = GetTall();

	int marginX = MAX( static_cast< int >( w * 0.04f ), 16 );
	int marginY = MAX( static_cast< int >( h * 0.03f ), 12 );
	int spacingY = MAX( static_cast< int >( h * 0.015f ), 6 );
	int spacingX = MAX( static_cast< int >( w * 0.015f ), 6 );

	int controlHeight = MAX( static_cast< int >( h * 0.06f ), 24 );

	int leftColumnX = marginX;
	int leftColumnWidth = static_cast< int >( w * 0.45f );
	leftColumnWidth = MAX( leftColumnWidth, 220 );

	int separatorX = leftColumnX + leftColumnWidth + marginX;

	int rightColumnX = separatorX + marginX;
	int rightColumnWidth = w - rightColumnX - marginX;
	rightColumnWidth = MAX( rightColumnWidth, 180 );

	int yLeft = marginY;

	int nameLabelWidth = MAX( static_cast< int >( leftColumnWidth * 0.35f ), 90 );
	int nameEntryWidth = leftColumnWidth - nameLabelWidth - spacingX;

	m_pNameLabel->SetBounds( leftColumnX, yLeft, nameLabelWidth, controlHeight );
	m_pNameEntry->SetBounds( leftColumnX + nameLabelWidth + spacingX, yLeft, nameEntryWidth, controlHeight );

	yLeft += controlHeight + spacingY * 2;

	int modelSize = MIN( leftColumnWidth, static_cast< int >( h * 0.35f ) );
	modelSize = clamp( modelSize, 140, 280 );

	int modelX = leftColumnX + ( leftColumnWidth - modelSize ) / 2;
	m_pPMModel->SetBounds( modelX, yLeft, modelSize, modelSize );

	yLeft += modelSize + spacingY;

	int refreshBtnWidth = MAX( static_cast< int >( leftColumnWidth * 0.25f ), 70 );
	int comboWidth = leftColumnWidth - refreshBtnWidth - spacingX;
	int comboHeight = controlHeight;

	m_pPMSelector->SetBounds( leftColumnX, yLeft, comboWidth, comboHeight );
	m_pRefreshPMBtn->SetBounds( leftColumnX + comboWidth + spacingX, yLeft, refreshBtnWidth, comboHeight );

	yLeft += comboHeight + spacingY;

	int colorLabelHeight = static_cast< int >( controlHeight * 0.55f );
	colorLabelHeight = MAX( colorLabelHeight, 16 );

	m_pPlayerColorLabel->SetBounds( leftColumnX, yLeft, leftColumnWidth, colorLabelHeight );
	yLeft += colorLabelHeight + spacingY / 2;

	m_pPlayerColorBtn->SetBounds( leftColumnX, yLeft, leftColumnWidth, controlHeight );
	yLeft += controlHeight + spacingY;

	m_pWeaponColorLabel->SetBounds( leftColumnX, yLeft, leftColumnWidth, colorLabelHeight );
	yLeft += colorLabelHeight + spacingY / 2;

	m_pWeaponColorBtn->SetBounds( leftColumnX, yLeft, leftColumnWidth, controlHeight );
	yLeft += controlHeight + spacingY;

	m_pHandModelSelector->SetBounds( leftColumnX, yLeft, leftColumnWidth, controlHeight );

	int sepTop = marginY;
	int sepBottom = h - marginY;
	m_pVerticalSeparator->SetBounds( separatorX, sepTop, 2, sepBottom - sepTop );

	int yRight = marginY;

	int presetLabelHeight = static_cast< int >( controlHeight * 0.7f );
	presetLabelHeight = MAX( presetLabelHeight, 18 );

	int columns = 2;
	int presetSpacing = MAX( static_cast< int >( w * 0.012f ), 6 );
	int presetBtnW = ( rightColumnWidth - ( columns - 1 ) * presetSpacing ) / columns;
	presetBtnW = MAX( presetBtnW, 60 );

	int presetBtnH = MAX( static_cast< int >( h * 0.05f ), 24 );

	m_pPlayerPresetsLabel->SetBounds( rightColumnX, yRight, rightColumnWidth, presetLabelHeight );
	yRight += presetLabelHeight + spacingY;

	int playerRows = ( m_PlayerPresetBtns.Count() + columns - 1 ) / columns;
	for ( int i = 0; i < m_PlayerPresetBtns.Count(); i++ )
	{
		int row = i / columns;
		int col = i % columns;

		int px = rightColumnX + col * ( presetBtnW + presetSpacing );
		int py = yRight + row * ( presetBtnH + presetSpacing );

		m_PlayerPresetBtns[i]->SetBounds( px, py, presetBtnW, presetBtnH );
	}

	yRight += playerRows * ( presetBtnH + presetSpacing ) + spacingY;

	m_pHorizontalSeparator->SetBounds( rightColumnX, yRight, rightColumnWidth, 2 );
	yRight += spacingY * 2;

	m_pWeaponPresetsLabel->SetBounds( rightColumnX, yRight, rightColumnWidth, presetLabelHeight );
	yRight += presetLabelHeight + spacingY;

	int weaponRows = ( m_WeaponPresetBtns.Count() + columns - 1 ) / columns;
	for ( int i = 0; i < m_WeaponPresetBtns.Count(); i++ )
	{
		int row = i / columns;
		int col = i % columns;

		int px = rightColumnX + col * ( presetBtnW + presetSpacing );
		int py = yRight + row * ( presetBtnH + presetSpacing );

		m_WeaponPresetBtns[i]->SetBounds( px, py, presetBtnW, presetBtnH );
	}
}

void CAdvancedOptionsMultiplayer::OnColorPicked( KeyValues *data )
{
	if ( !data )
		return;

	int target = data->GetInt( "target", COLORTARGET_NONE );

	int r = data->GetInt( "r", -1 );
	int g = data->GetInt( "g", -1 );
	int b = data->GetInt( "b", -1 );

	if ( ( r < 0 || g < 0 || b < 0 ) && data->GetInt( "color", -1 ) != -1 )
	{
		int packed = data->GetInt( "color", -1 );
		// try ARGB (A R G B) (  A   R   G   B  ) (     A     R     G     B     ) (       A       R       G       B       )
		r = ( packed >> 16 ) & 0xFF;
		g = ( packed >> 8 ) & 0xFF;
		b = ( packed ) & 0xFF;
	}

	if ( r >= 0 && g >= 0 && b >= 0 && target != COLORTARGET_NONE )
	{
		ApplyColorToConvarsByTarget( target, r, g, b );

		if ( target == COLORTARGET_PLAYER && m_pPlayerColorBtn )
			m_pPlayerColorBtn->SetColor( r, g, b, 255 );
		else if ( target == COLORTARGET_WEAPON && m_pWeaponColorBtn )
			m_pWeaponColorBtn->SetColor( r, g, b, 255 );
	}
}

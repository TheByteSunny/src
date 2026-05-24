//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#include "cbase.h"
#include "convar.h"
#include "filesystem.h"
#include "bitmap/tgaloader.h"
#include "vtf/vtf.h"
#include "tier1/utlbuffer.h"
#include "tier1/strtools.h"
#include "view.h"
#include "hudelement.h"
#include "hud.h"
#include "clientmode.h"
#include "c_baseplayer.h"
#include "prediction.h"

extern ConVar r_drawviewmodel;
extern ConVar cl_drawhud;

static float g_flOriginalFOV = 90.0f;
static bool	 g_bOriginalViewModel = true;
static bool	 g_bOriginalHUD = true;
static bool	 g_bValuesStored = false;

bool ConvertTGAtoVTF( const char *pSourceTGA, const char *pDestVTF, int pVTFFlags, bool pRemoveSourceTGA = false )
{
	enum ImageFormat indexImageFormat;
	int				 indexImageSize;
	float			 gamma;
	int				 w, h;

	if ( !TGALoader::GetInfo( pSourceTGA, &w, &h, &indexImageFormat, &gamma ) )
	{
		Warning( "Unable to find TGA: %s\n", pSourceTGA );
		return false;
	}

	indexImageSize = ImageLoader::GetMemRequired( w, h, 1, indexImageFormat, false );
	unsigned char *pImage = (unsigned char *)new unsigned char[indexImageSize];

	if ( !TGALoader::Load( pImage, pSourceTGA, w, h, indexImageFormat, gamma, false ) )
	{
		Warning( "Unable to load TGA: %s\n", pSourceTGA );
		delete[] pImage;
		return false;
	}

	IVTFTexture *vtf = CreateVTFTexture();

	vtf->Init( w, h, 1, indexImageFormat, pVTFFlags, 1 );
	unsigned char *pDest = vtf->ImageData( 0, 0, 0 );

	memcpy( pDest, pImage, indexImageSize );

	CUtlBuffer buffer;
	vtf->Serialize( buffer );
	bool vtfWriteSuccess = g_pFullFileSystem->WriteFile( pDestVTF, "MOD", buffer );

	DestroyVTFTexture( vtf );
	buffer.Clear();
	delete[] pImage;

	if ( !vtfWriteSuccess )
	{
		Warning( "Unable to write VTF: %s\n", pDestVTF );
		return false;
	}

	if ( pRemoveSourceTGA )
	{
		g_pFullFileSystem->RemoveFile( pSourceTGA, "MOD" );
	}

	return true;
}

bool CreateVMTFile( const char *pVMTPath, const char *pVTFName )
{
	CUtlBuffer vmtBuffer( 0, 0, CUtlBuffer::TEXT_BUFFER );

	vmtBuffer.Printf( "\"UnlitGeneric\"\n" );
	vmtBuffer.Printf( "{\n" );
	vmtBuffer.Printf( "\t\"$basetexture\" \"%s\"\n", pVTFName );
	vmtBuffer.Printf( "\t\"$translucent\" \"1\"\n" );
	vmtBuffer.Printf( "\t\"$vertexcolor\" \"1\"\n" );
	vmtBuffer.Printf( "\t\"$vertexalpha\" \"1\"\n" );
	vmtBuffer.Printf( "\t\"$ignorez\" \"1\"\n" );
	vmtBuffer.Printf( "}\n" );

	bool success = g_pFullFileSystem->WriteFile( pVMTPath, "MOD", vmtBuffer );
	vmtBuffer.Clear();

	return success;
}

void StoreOriginalValues()
{
	if ( !g_bValuesStored )
	{
		g_bOriginalViewModel = r_drawviewmodel.GetBool();
		g_bOriginalHUD = cl_drawhud.GetBool();
		g_bValuesStored = true;
	}
}

void RestoreOriginalValues()
{
	if ( g_bValuesStored )
	{
		r_drawviewmodel.SetValue( g_bOriginalViewModel );
		cl_drawhud.SetValue( g_bOriginalHUD );
	}
}

// Main screenshot command
void CC_TakeVTFScreenshot( const CCommand &args )
{
	StoreOriginalValues();

	cl_drawhud.SetValue( 0 );
	r_drawviewmodel.SetValue( 0 );

	engine->ClientCmd( "wait; vtf_screenshot_internal" );
}

void CC_TakeVTFScreenshotInternal( const CCommand &args )
{
	// Get current map name
	const char *mapName = engine->GetLevelName();
	if ( !mapName || !mapName[0] )
	{
		Warning( "Could not get current map name\n" );
		RestoreOriginalValues();
		return;
	}

	char cleanMapName[MAX_PATH];
	V_FileBase( mapName, cleanMapName, sizeof( cleanMapName ) );
	Q_strlower( cleanMapName );

	char tgaPath[MAX_PATH];
	char vtfPath[MAX_PATH];
	char vmtPath[MAX_PATH];
	char vtfName[MAX_PATH];

	Q_snprintf( vtfPath, sizeof( vtfPath ), "materials/vgui/thumb/%s.vtf", cleanMapName );
	Q_snprintf( vmtPath, sizeof( vmtPath ), "materials/vgui/thumb/%s.vmt", cleanMapName );
	Q_snprintf( vtfName, sizeof( vtfName ), "vgui/thumb/%s", cleanMapName );

	g_pFullFileSystem->CreateDirHierarchy( "screenshots", "MOD" );
	g_pFullFileSystem->CreateDirHierarchy( "materials/vgui/thumb", "MOD" );

	engine->ClientCmd( "wait; wait; screenshot; wait; wait; vtf_process_screenshot" );

	static char s_tgaPath[MAX_PATH];
	static char s_vtfPath[MAX_PATH];
	static char s_vmtPath[MAX_PATH];
	static char s_vtfName[MAX_PATH];

	Q_strcpy( s_tgaPath, tgaPath );
	Q_strcpy( s_vtfPath, vtfPath );
	Q_strcpy( s_vmtPath, vmtPath );
	Q_strcpy( s_vtfName, vtfName );
}

void CC_ProcessVTFScreenshot( const CCommand &args )
{
	const char *mapName = engine->GetLevelName();
	if ( !mapName || !mapName[0] )
	{
		Warning( "Could not get current map name for processing\n" );
		RestoreOriginalValues();
		return;
	}

	char cleanMapName[MAX_PATH];
	V_FileBase( mapName, cleanMapName, sizeof( cleanMapName ) );
	Q_strlower( cleanMapName );

	char vtfPath[MAX_PATH];
	char vmtPath[MAX_PATH];
	char vtfName[MAX_PATH];

	Q_snprintf( vtfPath, sizeof( vtfPath ), "materials/vgui/thumb/%s.vtf", cleanMapName );
	Q_snprintf( vmtPath, sizeof( vmtPath ), "materials/vgui/thumb/%s.vmt", cleanMapName );
	Q_snprintf( vtfName, sizeof( vtfName ), "vgui/thumb/%s", cleanMapName );

	char searchPattern[MAX_PATH];
	Q_snprintf( searchPattern, sizeof( searchPattern ), "screenshots/%s*.tga", cleanMapName );

	FileFindHandle_t handle;
	const char		*pFileName = g_pFullFileSystem->FindFirst( searchPattern, &handle );

	int	 bestIndex = -1;
	char bestPath[MAX_PATH] = "";

	while ( pFileName )
	{
		int			number = -1;
		const char *numStr = pFileName + Q_strlen( cleanMapName );
		sscanf( numStr, "%04d.tga", &number );

		if ( number > bestIndex )
		{
			bestIndex = number;
			Q_snprintf( bestPath, sizeof( bestPath ), "screenshots/%s", pFileName );
		}

		pFileName = g_pFullFileSystem->FindNext( handle );
	}
	g_pFullFileSystem->FindClose( handle );

	if ( bestIndex == -1 )
	{
		Warning( "No screenshots found for map: %s\n", cleanMapName );
		RestoreOriginalValues();
		return;
	}

	int vtfFlags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD;

	if ( !ConvertTGAtoVTF( bestPath, vtfPath, vtfFlags, true ) )
	{
		Warning( "Failed to convert screenshot to VTF\n" );
		RestoreOriginalValues();
		return;
	}

	if ( !CreateVMTFile( vmtPath, vtfName ) )
	{
		Warning( "Failed to create VMT file: %s\n", vmtPath );
		RestoreOriginalValues();
		return;
	}

	RestoreOriginalValues();

	Msg( "Screenshot successfully converted to VTF and VMT for map: %s\n", cleanMapName );
	Msg( "Using screenshot: %s\n", bestPath );
	Msg( "VTF: %s\n", vtfPath );
	Msg( "VMT: %s\n", vmtPath );
	Msg( "Material name: %s\n", vtfName );
}

static ConCommand vtf_screenshot( "vtf_screenshot", CC_TakeVTFScreenshot, "Takes a screenshot and converts it to VTF/VMT format", FCVAR_NONE );
static ConCommand vtf_screenshot_internal( "vtf_screenshot_internal", CC_TakeVTFScreenshotInternal, "Internal command for VTF screenshot", FCVAR_HIDDEN );
static ConCommand vtf_process_screenshot( "vtf_process_screenshot", CC_ProcessVTFScreenshot, "Internal command to process VTF screenshot", FCVAR_HIDDEN );
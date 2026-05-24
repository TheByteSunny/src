//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "webmanager.h"
#include "filesystem.h"
#include <thread>

CWebManager::CWebManager()
{
}

CWebManager::~CWebManager()
{
	Shutdown();
}

bool CWebManager::Init()
{
	CURLcode code = curl_global_init( CURL_GLOBAL_DEFAULT );
	if ( code != 0 )
	{
		Warning( "curl_global_init failed: %s\n", curl_easy_strerror( code ) );
		return false;
	}

	return true;
}

void CWebManager::Shutdown()
{
	curl_global_cleanup();
}

size_t CWebManager::WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
	size_t		 totalSize = size * nmemb;
	std::string *mem = static_cast< std::string * >( userp );
	try
	{
		mem->append( static_cast< char * >( contents ), totalSize );
	}
	catch ( const std::bad_alloc & )
	{
		return 0;
	}

	return totalSize;
}

bool CWebManager::DownloadToFileAsync( const char *url, const char *localPath, WebDownloadCallback cb )
{
	if ( !url || !*url || !localPath || !*localPath )
	{
		if ( cb ) cb( false, localPath );
		return false;
	}

	std::string urlStr  = url;
	std::string pathStr = localPath;

	std::thread( [ this, urlStr, pathStr, cb ]()
	{
		bool ok = DownloadToFile( urlStr, pathStr );
		if ( cb )
			cb( ok, pathStr.c_str() );
	} ).detach();

	return true;
}

size_t CWebManager::WriteFileCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
	size_t		 totalSize = size * nmemb;
	FileHandle_t file = static_cast< FileHandle_t >( userp );
	if ( !file )
		return 0;

	int written = g_pFullFileSystem->Write( contents, totalSize, file );
	if ( written < 0 || static_cast< size_t >( written ) != totalSize )
		return 0;

	return totalSize;
}

bool CWebManager::LoadCACertBlob( CURL *curl )
{
	FileHandle_t hFile = g_pFullFileSystem->Open( "settings/cacert.pem", "rb", "GAME" );
	if ( !hFile )
	{
		Warning( "Could not open settings/cacert.pem - SSL may fail.\n" );
		return false;
	}

	int nSize = g_pFullFileSystem->Size( hFile );
	if ( nSize <= 0 )
	{
		g_pFullFileSystem->Close( hFile );
		Warning( "cacert.pem is empty or invalid.\n" );
		return false;
	}

	char *pBuf = new ( std::nothrow ) char[nSize];
	if ( !pBuf )
	{
		g_pFullFileSystem->Close( hFile );
		Warning( "Out of memory loading cacert.pem\n" );
		return false;
	}

	g_pFullFileSystem->Read( pBuf, nSize, hFile );
	g_pFullFileSystem->Close( hFile );

	struct curl_blob blob;
	blob.data = pBuf;
	blob.len = nSize;
	blob.flags = CURL_BLOB_COPY;

	CURLcode rc = curl_easy_setopt( curl, CURLOPT_CAINFO_BLOB, &blob );
	delete[] pBuf;

	if ( rc != CURLE_OK )
	{
		Warning( "CURLOPT_CAINFO_BLOB failed: %s\n", curl_easy_strerror( rc ) );
		return false;
	}
	return true;
}

bool CWebManager::ApplyCommonOptions( CURL *curl, char *errbuf )
{
	errbuf[0] = '\0';
	curl_easy_setopt( curl, CURLOPT_ERRORBUFFER, errbuf );
	curl_easy_setopt( curl, CURLOPT_USERAGENT, "HL2SBPP/1.1" );
	curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
	curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 10L );
	curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 1L );
	curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 2L );
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 10L );
	curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1L );
	curl_easy_setopt( curl, CURLOPT_FAILONERROR, 0L );
	return true;
}

WebResult_t CWebManager::PerformAndBuildResult( CURL *curl, char *errbuf )
{
	WebResult_t result = {};
	result.success = false;
	result.error = WEB_OK;
	result.httpCode = 0;
	result.curlCode = CURLE_OK;

	CURLcode res = curl_easy_perform( curl );
	result.curlCode = res;

	if ( res != CURLE_OK )
	{
		result.error = WEB_ERR_CURL_FAILED;
		const char *msg = ( errbuf && errbuf[0] ) ? errbuf : curl_easy_strerror( res );
		result.errorMessage = msg ? msg : "Unknown curl error";
		Warning( "curl_easy_perform failed: %s\n", result.errorMessage.c_str() );
		return result;
	}

	long code = 0;
	curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	result.httpCode = code;

	if ( code >= 200 && code < 300 )
		result.success = true;
	else
	{
		result.error = WEB_ERR_HTTP_STATUS;
		char buf[128];
		V_snprintf( buf, sizeof( buf ), "HTTP status %ld", code );
		result.errorMessage = buf;
		Warning( "%s\n", buf );
	}

	return result;
}

bool CWebManager::Get( const std::string &url, RequestCallback callback )
{
	if ( url.empty() )
	{
		if ( callback )
		{
			WebResult_t r = {};
			r.error = WEB_ERR_INVALID_URL;
			r.errorMessage = "Empty URL";
			callback( r );
		}
		return false;
	}

	CURL *curl = curl_easy_init();
	if ( !curl )
	{
		Warning( "curl_easy_init failed\n" );
		if ( callback )
		{
			WebResult_t r = {};
			r.error = WEB_ERR_INIT_FAILED;
			r.errorMessage = "curl_easy_init failed";
			callback( r );
		}
		return false;
	}

	char errbuf[CURL_ERROR_SIZE] = { 0 };
	LoadCACertBlob( curl ); // non-fatal

	std::string response;
	ApplyCommonOptions( curl, errbuf );
	curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L );

	WebResult_t result = PerformAndBuildResult( curl, errbuf );
	result.body = std::move( response );

	curl_easy_cleanup( curl );

	if ( callback )
		callback( result );

	return result.success;
}

bool CWebManager::Post( const std::string &url, const std::string &jsonBody, RequestCallback callback )
{
	if ( url.empty() )
	{
		if ( callback )
		{
			WebResult_t r = {};
			r.error = WEB_ERR_INVALID_URL;
			r.errorMessage = "Empty URL";
			callback( r );
		}
		return false;
	}

	CURL *curl = curl_easy_init();
	if ( !curl )
	{
		Warning( "curl_easy_init failed\n" );
		if ( callback )
		{
			WebResult_t r = {};
			r.error = WEB_ERR_INIT_FAILED;
			r.errorMessage = "curl_easy_init failed";
			callback( r );
		}
		return false;
	}

	char errbuf[CURL_ERROR_SIZE] = { 0 };
	LoadCACertBlob( curl );

	std::string response;

	struct curl_slist *headers = NULL;
	headers = curl_slist_append( headers, "Content-Type: application/json" );
	headers = curl_slist_append( headers, "Accept: application/json" );

	ApplyCommonOptions( curl, errbuf );
	curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( curl, CURLOPT_POST, 1L );
	curl_easy_setopt( curl, CURLOPT_POSTFIELDS, jsonBody.c_str() );
	curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, (long)jsonBody.size() );
	curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30L );

	WebResult_t result = PerformAndBuildResult( curl, errbuf );
	result.body = std::move( response );

	curl_slist_free_all( headers );
	curl_easy_cleanup( curl );

	if ( callback )
		callback( result );

	return result.success;
}

bool CWebManager::DownloadToFile( const std::string &url, const std::string &filePath )
{
	if ( url.empty() || filePath.empty() )
	{
		Warning( "DownloadToFile: invalid arguments\n" );
		return false;
	}

	CURL *curl = curl_easy_init();
	if ( !curl )
	{
		Warning( "curl_easy_init failed\n" );
		return false;
	}

	char errbuf[CURL_ERROR_SIZE] = { 0 };
	LoadCACertBlob( curl );

	FileHandle_t file = g_pFullFileSystem->Open( filePath.c_str(), "wb" );
	if ( !file )
	{
		Warning( "Could not open output file: %s\n", filePath.c_str() );
		curl_easy_cleanup( curl );
		return false;
	}

	ApplyCommonOptions( curl, errbuf );
	curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFileCallback );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, file );
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 300L );

	WebResult_t result = PerformAndBuildResult( curl, errbuf );

	g_pFullFileSystem->Close( file );
	curl_easy_cleanup( curl );

	if ( !result.success )
	{
		g_pFullFileSystem->RemoveFile( filePath.c_str() );
		Warning( "Download failed (%s), removed partial file %s\n", result.errorMessage.c_str(), filePath.c_str() );
	}

	return result.success;
}

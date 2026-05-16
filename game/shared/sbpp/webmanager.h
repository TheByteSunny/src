//========== Copyright (C) 2026, Team HL2SB++, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef WEBMANAGER_H
#define WEBMANAGER_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <curl/curl.h>
#include <string>
#include <functional>

enum WebError_t
{
	WEB_OK = 0,
	WEB_ERR_INIT_FAILED,
	WEB_ERR_CURL_FAILED,
	WEB_ERR_HTTP_STATUS,
	WEB_ERR_FILE_OPEN_FAILED,
	WEB_ERR_INVALID_URL,
};

struct WebResult_t
{
	bool		success;
	WebError_t	error;
	long		httpCode;
	CURLcode	curlCode;
	std::string errorMessage;
	std::string body;
};

typedef std::function< void( const WebResult_t & ) > RequestCallback;
typedef std::function< void( bool success, const char *localPath ) > WebDownloadCallback;

class CWebManager
{
public:
	CWebManager();
	~CWebManager();

	bool Init();
	void Shutdown();

	bool Get( const std::string &url, RequestCallback callback );
	bool Post( const std::string &url, const std::string &jsonBody, RequestCallback callback );
	bool DownloadToFile( const std::string &url, const std::string &filePath );

	bool DownloadToFileAsync( const char *url, const char *localPath, WebDownloadCallback cb );

private:
	static size_t WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp );
	static size_t WriteFileCallback( void *contents, size_t size, size_t nmemb, void *userp );

	bool		ApplyCommonOptions( CURL *curl, char *errbuf );
	bool		LoadCACertBlob( CURL *curl );
	WebResult_t PerformAndBuildResult( CURL *curl, char *errbuf );
};

#endif // WEBMANAGER_H
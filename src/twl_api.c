#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <wchar.h>

#define TWL_API_EXPORTS
#include "twl_api.h"

#include "VTS_table.h"

#pragma comment(lib, "winhttp.lib")

#ifdef _DEBUG
#define TWL_DEBUG_REQUEST_HEADERS 1
#define TWL_DEBUG_REQUEST_QUERY 1
#define TWL_DEBUG_REQUEST_BODY 1
#define TWL_DEBUG_REQUEST_RESPONSE 1
#define TWL_DEBUG_READ_DATA 1
#define TWL_DEBUG_EVENT_READ_DATA 1
#endif


BOOL g_is_shutdown = FALSE;
const FTWLUserCredentials *g_current_user = NULL;

BOOL DllMain(HINSTANCE hInstDLL, DWORD reason, LPVOID reserved) {

	switch(reason) {

		case DLL_PROCESS_ATTACH:
			OutputDebugString("DLL_PROCESS_ATTACH\n");
			break;

		case DLL_THREAD_ATTACH:
			OutputDebugString("DLL_THREAD_ATTACH\n");
			break;

		case DLL_THREAD_DETACH:
			OutputDebugString("DLL_THREAD_DETACH\n");
			break;

		case DLL_PROCESS_DETACH:
			OutputDebugString("DLL_PROCESS_DETACH\n");

			if(reserved != NULL) {
				OutputDebugString("reserved is not NULL\n");
				break;
			}
			break;
	}

	return TRUE;
}

// #TODO What about split the connection and request handles from the user credentials
// and so use only one handles and multiple user credentials?
// I mean, at the end the client id and oauth token go in the request headers  and has
// no use when we enstablish the connection to the endpoint.
// ======================================== //
//            TWITCH LIBRARY API            //
// ======================================== //
BOOL TWLConnect(FTWLUserCredentials *user_credentials) {

	printf(YELLOW("Opening session..\n"));

	user_credentials->hSession = WinHttpOpen(L"TWL_Session", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(user_credentials->hSession == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);

		return FALSE;
	}

	printf(YELLOW("Connecting to api.twitch.tv..\n"));
	user_credentials->hConnection = WinHttpConnect(user_credentials->hSession, L"api.twitch.tv", INTERNET_DEFAULT_HTTPS_PORT, 0);
	if(user_credentials->hConnection == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);

		return FALSE;
	}

	g_current_user = user_credentials;
	printf(GREEN("Connection successfully enstablished!\n"));

	return TRUE;
}


BOOL TWLShutdown(FTWLUserCredentials *user_credentials) {

	wmemset(user_credentials->client_id, L'\0', ARRAYSIZE(user_credentials->client_id));
	wmemset(user_credentials->oauth2_token, L'\0', ARRAYSIZE(user_credentials->oauth2_token));

	printf(YELLOW("Closing connection handle..\n"));
	BOOL result_a = WinHttpCloseHandle(user_credentials->hConnection);
	if(result_a == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	printf(YELLOW("Closing session handle..\n"));
	BOOL result_b = WinHttpCloseHandle(user_credentials->hSession);
	if(result_b == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	g_is_shutdown = result_a && result_b;

	g_is_shutdown ? 
		printf(GREEN("Connection and session successfully closed!\n")) : 
		printf(RED("An error occurred while closing connection and session!\n"));

	return g_is_shutdown;
}


BOOL TWLSetCurrentUser(const FTWLUserCredentials *user_credentials) {

	if(
			user_credentials == NULL || 
			user_credentials->hSession == NULL || 
			user_credentials->hConnection == NULL
	  ) return FALSE;

	g_current_user = user_credentials;

	return TRUE;
}

// ========================= //
//            ADS            //
// ========================= //
BOOL TWLStartCommercial(const char *broadcaster_id, int32_t length, char **request_status) {
		
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, 
			"{"
			"\"broadcaster_id\":\"%s\","
			"\"length\":%i"
			"}", 
			broadcaster_id,
			length);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/channels/commercial", request_body_buffer, &status_code);

	*request_status = TWLReadData(hRequest);
	if(*request_status == NULL) return FALSE;
	

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *request_status);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetAdSchedule(const char *broadcaster_id, char **schedule) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/ads?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*schedule = TWLReadData(hRequest);
	if(*schedule == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *schedule);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSnoozeNextAd(const char *broadcaster_id, char **snoozes) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/ads/schedule/snooze?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);

	*snoozes = TWLReadData(hRequest);
	if(*snoozes == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *snoozes);
		return FALSE;
	}

	return TRUE;
}


// =============================== //
//            ANALYTICS            //
// =============================== //
BOOL TWLGetExtensionAnalytics(
		const char *extension_id,
		const char *type,
		const char *started_at,
		const char *ended_at,
		char **reports) {
	
	wchar_t request_query_buffer[512] = {};
	wchar_t request_query_optional[512] = {};

	if(extension_id != NULL && strlen(extension_id) > 0) {
		wchar_t _extension_id[64] = {};
		mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&extension_id=%s", _extension_id);
	}

	if(type != NULL && strlen(type) > 0) {
		wchar_t _type[64] = {};
		mbstowcs(_type, type, ARRAYSIZE(_type));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&type=%s", _type);
	}
	
	if(started_at != NULL && strlen(started_at) > 0) {
		wchar_t _started_at[64] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&started_at=%s", _started_at);
	}
	if(ended_at != NULL && strlen(ended_at) > 0) {
		wchar_t _ended_at[64] = {};
		mbstowcs(_ended_at, ended_at, ARRAYSIZE(_ended_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&ended_at=%s", _ended_at);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/analytics/extensions?%s", request_query_optional + 1);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*reports = TWLReadData(hRequest);
	if(*reports == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *reports);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGameAnalytics(const char *game_id, const char *type, const char *started_at, const char *ended_at, char **reports) {
	
	wchar_t request_query_buffer[512] = {};
	wchar_t request_query_optional[256] = {};

	if(game_id != NULL && strlen(game_id) > 0) {
		wchar_t _game_id[64] = {};
		mbstowcs(_game_id, game_id, ARRAYSIZE(_game_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&game_id=%s", _game_id);
	}

	if(type != NULL && strlen(type) > 0) {
		wchar_t _type[64] = {};
		mbstowcs(_type, type, ARRAYSIZE(_type));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&type=%s", _type);
	}
	
	if(started_at != NULL && strlen(started_at) > 0) {
		wchar_t _started_at[64] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&started_at=%s", _started_at);
	}
	if(ended_at != NULL && strlen(ended_at) > 0) {
		wchar_t _ended_at[64] = {};
		mbstowcs(_ended_at, ended_at, ARRAYSIZE(_ended_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&ended_at=%s", _ended_at);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/analytics/games?%s", request_query_optional + 1);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*reports = TWLReadData(hRequest);
	if(*reports == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *reports);
		return FALSE;
	}

	return TRUE;
}


// ========================== //
//            BITS            //
// ========================== //
BOOL TWLGetBitsLeaderboard(int32_t count, const char *period, const char *started_at, const char *user_id, char **leaderboard) {
	
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[256] = {};

	if(count > 0) {
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&count=%i", min(count, 100));
	}

	if(period != NULL && strlen(period) > 0) {
		wchar_t _period[6] = {};
		mbstowcs(_period, period, ARRAYSIZE(_period));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional),
				L"&period=%s",
				_period);
	}

	if(period != NULL && strlen(period) > 0 && started_at != NULL && strlen(started_at) > 0) {
		wchar_t _started_at[30] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional),
				L"&started_at=%s",
				_started_at);
	}

	if(user_id != NULL && strlen(user_id) > 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional),
				L"&user_id=%s",
				_user_id);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/bits/leaderboard?%s", request_query_optional + 1);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*leaderboard = TWLReadData(hRequest);
	if(*leaderboard == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *leaderboard);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetCheermotes(const char *broadcaster_id, char **cheermotes) {
	
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[128] = {};
	
	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {
		wchar_t _broadcaster_id[64] = {};
		mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?broadcaster_id=%s", _broadcaster_id);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/bits/cheermotes%s", request_query_optional);
	
	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*cheermotes = TWLReadData(hRequest);
	if(*cheermotes == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *cheermotes);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetExtensionTransactions(const char *extension_id, const char *transaction_id, char **transactions) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[8192] = {};
	wchar_t request_query_optional[7000] = {};
	
	if(transaction_id != NULL && strlen(transaction_id) > 0) {
		
		char _transaction_id[7000] = {};
		const char *p = transaction_id;
		int counter = 0;
		size_t i = 0;
		
		while(*p != '\0' && counter < 100) {

			strcpy(_transaction_id + strlen(_transaction_id), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_transaction_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7000] = {};	
		mbstowcs(_ids, _transaction_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/bits/extensions/transactions?extension_id=%s%s",
			_extension_id,
			request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*transactions = TWLReadData(hRequest);
	if(*transactions == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *transactions);
		return FALSE;
	}

	return TRUE;
}


// ============================== //
//            CHANNELS            //
// ============================== //
BOOL TWLGetChannelInfo(const char *broadcaster_id, char **channel_info) {

	wchar_t request_query_buffer[8500] = {};
		
	char broadcaster_ids[8000] = {};
	const char *p = broadcaster_id;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 100) {

		strcpy(broadcaster_ids + strlen(broadcaster_ids), "&broadcaster_id=");
		i += strlen("&broadcaster_id=");

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			broadcaster_ids[i] = *p;
		}

		p++;
		counter++;
	}

	wchar_t _ids[8000] = {};
	mbstowcs(_ids, broadcaster_ids, ARRAYSIZE(_ids));

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels?%s", _ids+1);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*channel_info = TWLReadData(hRequest);
	if(*channel_info == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *channel_info);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLModifyChannelInformation(
		const char *broadcaster_id,
		const char *game_id,
		const char *language,
		const char *title,
		int32_t delay,
		const char tags[10][26],
		const FTWLCCLabel CCLs[6],
		BOOL is_branded_content) {


	wchar_t request_query_buffer[256] = {};
	
	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels?broadcaster_id=%s", _broadcaster_id);

	char request_body_buffer[4096] = {};
	char request_body_optional[4096] = {};

	// this first because is a bool, is alway set to TRUE or FALSE
	// because C doesn't have optional parameters, so we solve the
	// "comma problem" and also the condition of must set at least
	// one parameter.
	sprintf(request_body_optional, "\"is_branded_content\":%s", (is_branded_content == TRUE) ? "true" : "false");

	if(game_id != NULL) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"game_id\":\"%s\"", game_id);
	}

	if(language != NULL && strlen(language) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional),
				",\"broadcaster_lamguage\":\"%s\"", language);
	}

	if(title != NULL && strlen(title) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional),
				",\"title\":\"%s\"", title);
	}

	if(delay > -1) {
		sprintf(request_body_optional + strlen(request_body_optional),
				",\"delay\":%i", delay);
	}

	if(tags != NULL) {

		// (FIX) to reset the tags pass an empty array
		// https://github.com/twitchdev/issues/issues/995
		if(strlen(tags[0]) == 0) {

			sprintf(request_body_optional + strlen(request_body_optional), ",\"tags\":{\"unset\":\"please\"}");
		
		} else {
		
			char _tags[260] = {};
			char tmp[260] = {};
			char *p = _tags;
			for(size_t i=0; i<10; i++) {
				if(strlen(tags[i]) == 0) break;
				sprintf(tmp, ",\"%s\"", tags[i]);
				sprintf(p, "%s", tmp);
				p += strlen(tmp);
			}
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"tags\":[%s]", _tags+1);
		}
	}

	if(CCLs != NULL) {
		char _ccls[260] = {};
		char tmp[260] = {};
		char *p = _ccls;
		for(size_t i=0; i<6; i++) {
			if(strlen(CCLs[i].id) == 0) break;
			sprintf(tmp, ",{\"id\":\"%s\",\"is_enabled\":%s}", CCLs[i].id, (CCLs[i].is_enabled == TRUE) ? "true" : "false");
			sprintf(p, "%s", tmp);
			p += strlen(tmp);
		}
		sprintf(request_body_optional + strlen(request_body_optional),
				",\"content_classification_labels\":[%s]", _ccls+1);
	}

	sprintf(request_body_buffer, 
			"{"
			"%s"
			"}", 
			request_body_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);

	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);

		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetChannelEditors(const char *broadcaster_id, char **channel_editors) {	

	wchar_t request_query_buffer[256] = {};
	
	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/editors?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*channel_editors = TWLReadData(hRequest);
	if(*channel_editors == NULL) return FALSE;
	

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *channel_editors);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetFollowedChannels(const char *user_id, const char *broadcaster_id, char **followed) {
	
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[128] = {};
	
	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {
	 
		wchar_t _broadcaster_id[64] = {};
		mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&broadcaster_id=%s", _broadcaster_id);
	}
	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/followed?user_id=%s%s", _user_id, request_query_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*followed = TWLReadData(hRequest);
	if(*followed == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *followed);
		return FALSE;
	}

	return TRUE;
}


TWL_API BOOL TWLGetChannelFollowers(const char *broadcaster_id, const char *user_id, char **followers) {

	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[128] = {};
	
	if(user_id != NULL && strlen(user_id) > 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"&user_id=%s", _user_id);
	}
	
	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/followers?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*followers = TWLReadData(hRequest);
	if(*followers == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *followers);
		return FALSE;
	}

	return TRUE;
}


// ==================================== //
//            CHANNEL POINTS            //
// ==================================== //
BOOL TWLCreateCustomRewards(
		const char *broadcaster_id,
		const char title[46], 
		const int64_t cost,
		const FTWLReward *reward_options,
		char **reward) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channel_points/custom_rewards?broadcaster_id=%s", _broadcaster_id);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[4096] = {};

	if(reward_options != NULL) {
		if(reward_options->is_enabled == FALSE) {
			sprintf(request_body_optional, ",\"is_enabled\":false");
		}

		if(strlen(reward_options->background_color) > 0) {
			sprintf(request_body_optional + strlen(request_body_optional), 
					",\"background_color\":\"%s\"", 
					reward_options->background_color);
		}

		if(reward_options->is_user_input_required == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"is_user_input_required\":true,\"prompt\":\"%s\"",
					reward_options->prompt);
		}

		if(reward_options->is_max_per_stream_enabled == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"is_max_per_stream_enabled\":true,\"max_per_stream\":%i",
				   (reward_options->max_per_stream < 1) ? 1 : reward_options->max_per_stream);
		}

		if(reward_options->is_max_per_user_per_stream_enabled == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"is_max_per_user_per_stream_enabled\":true,\"max_per_user_per_stream\":%i",
				   (reward_options->max_per_user_per_stream < 1) ? 1 : reward_options->max_per_user_per_stream);
		}

		if(reward_options->is_global_cooldown_enabled == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"is_global_cooldown_enabled\":true,\"global_cooldown_seconds\":%i",
				   (reward_options->global_cooldown_seconds < 1) ? 1 : reward_options->global_cooldown_seconds);
		}

		if(reward_options->should_redemptions_skip_request_queue == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional),
					",\"should_redemptions_skip_request_queue\":true");
		}

	}

	sprintf(request_body_buffer, 
			"{"
			"\"title\":\"%s\","
			"\"cost\":%lli"
			"%s"
			"}", 
			title,
			cost,
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*reward = TWLReadData(hRequest);
	if(*reward == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *reward);
		return FALSE;
	}

	return TRUE;
}

	
BOOL TWLDeleteCustomReward(const char *broadcaster_id, const char *id) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t _id[128] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channel_points/custom_rewards?broadcaster_id=%s&id=%s", _broadcaster_id, _id);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetCustomReward(const char *broadcaster_id, const char *id, BOOL only_manageable_rewards, char **custom_rewards) {
	
	wchar_t request_query_buffer[4000] = {};
	wchar_t request_query_optional[4000] = {};
	
	if(id != NULL && strlen(id) > 0) {
		char ids[3500] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;
		
		while(*p != '\0' && counter < 50) {

			strcpy(ids + strlen(ids), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[3500] = {};	
		mbstowcs(_ids, ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	if(only_manageable_rewards == TRUE) {
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&only_manageable_rewards=true");
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channel_points/custom_rewards?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);

	*custom_rewards = TWLReadData(hRequest);
	if(*custom_rewards == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *custom_rewards);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetCustomRewardRedemption(
		const char *broadcaster_id,
		const char *reward_id,
		const char *status,
		const char *id,
		const char *sort,
		char **redemptions) {
	
	wchar_t request_query_buffer[4000] = {};
	wchar_t request_query_optional[4000] = {};
	
	if(id == NULL || strlen(id) == 0) {
		wchar_t _status[16] = {};
		mbstowcs(_status, status, ARRAYSIZE(_status));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&status=%s", _status);
	}

	if(id != NULL && strlen(id) > 0) {
		
		char _id[3500] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;
		
		while(*p != '\0' && counter < 50) {

			strcpy(_id + strlen(_id), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[3500] = {};	
		mbstowcs(_ids, _id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	if(sort != NULL && strlen(sort) > 0) {
		wchar_t _sort[8] = {};
		mbstowcs(_sort, sort, ARRAYSIZE(_sort));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&sort=%s", _sort);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	wchar_t _reward_id[64] = {};
	mbstowcs(_reward_id, reward_id, ARRAYSIZE(_reward_id));	
	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/channel_points/custom_rewards/redemptions?broadcaster_id=%s&reward_id=%s%s", _broadcaster_id, _reward_id, request_query_optional);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*redemptions = TWLReadData(hRequest);
	if(*redemptions == NULL) return FALSE;


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *redemptions);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateCustomReward(const char *broadcaster_id, const char *id, const FTWLReward *reward, char **updated_reward) {
	
	wchar_t request_query_buffer[256] = {};
	
	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	wchar_t _id[128] = {};
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channel_points/custom_rewards?broadcaster_id=%s&id=%s", _broadcaster_id, _id);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[4096] = {};

	sprintf(request_body_optional, "\"is_enabled\":%s,", (reward->is_enabled == TRUE) ? "true" : "false");

	sprintf(request_body_optional + strlen(request_body_optional), 
			"\"background_color\":\"%s\",", 
			reward->background_color);

	if(reward->is_user_input_required == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_user_input_required\":true,\"prompt\":\"%s\",",
				reward->prompt);
	} else {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_user_input_required\":false,");
	}

	if(reward->is_max_per_stream_enabled == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_max_per_stream_enabled\":true,\"max_per_stream\":%i,",
				(reward->max_per_stream < 1) ? 1 : reward->max_per_stream);
	} else {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_max_per_stream_enabled\":false,");
	}

	if(reward->is_max_per_user_per_stream_enabled == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_max_per_user_per_stream_enabled\":true,\"max_per_user_per_stream\":%i,",
				(reward->max_per_user_per_stream < 1) ? 1 : reward->max_per_user_per_stream);
	} else {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_max_per_user_per_stream_enabled\":false,");
	}

	if(reward->is_global_cooldown_enabled == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_global_cooldown_enabled\":true,\"global_cooldown_seconds\":%i,",
				(reward->global_cooldown_seconds < 1) ? 1 : reward->global_cooldown_seconds);
	} else {
		sprintf(request_body_optional + strlen(request_body_optional),
				"\"is_global_cooldown_enabled\":false,");
	}

	sprintf(request_body_optional + strlen(request_body_optional),
			"\"is_paused\":%s,",
			(reward->is_paused == TRUE) ? "true" : "false");

	sprintf(request_body_optional + strlen(request_body_optional),
			"\"should_redemptions_skip_request_queue\":%s",
			(reward->should_redemptions_skip_request_queue == TRUE) ? "true" : "false");

	sprintf(request_body_buffer, 
			"{"
			"\"title\":\"%s\","
			"\"cost\":%i,"
			"%s"
			"}", 
			reward->title,
			reward->cost,
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);

	*updated_reward = TWLReadData(hRequest);
	if(*updated_reward == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_reward);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateRedemptionStatus(
			const char *broadcaster_id,
			const char *reward_id,
			const char *id,
			const char *status,
			char **updated_redemption) {
	
	wchar_t request_query_buffer[4096] = {};
	
	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	wchar_t _reward_id[128] = {};
	mbstowcs(_reward_id, reward_id, ARRAYSIZE(_reward_id));	
	
	char _id[3500] = {};
	const char *p = id;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 50) {

		strcpy(_id + strlen(_id), "&id=");
		i += strlen("&id=");

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			_id[i] = *p;
		}

		p++;
		counter++;
	}

	wchar_t _ids[3500] = {};	
	mbstowcs(_ids, _id, ARRAYSIZE(_ids));

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/channel_points/custom_rewards/redemptions?broadcaster_id=%s&reward_id=%s%s", _broadcaster_id, _reward_id, _ids);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[256] = {};

	sprintf(request_body_buffer, 
			"{"
			"\"status\":\"%s\""
			"}", 
			status);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);

	*updated_redemption = TWLReadData(hRequest);
	if(*updated_redemption == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_redemption);
		return FALSE;
	}

	return TRUE;
}

	
// ============================= //
//            CHARITY            //
// ============================= //
BOOL TWLGetCharityCampaign(const char *broadcaster_id, char **campaign) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/charity/campaigns?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*campaign = TWLReadData(hRequest);
	if(*campaign == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n\n"), *campaign);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetCharityCampaignDonations(const char *broadcaster_id, char **donations) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/charity/donations?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*donations = TWLReadData(hRequest);
	if(*donations == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n\n"), *donations);
		return FALSE;
	}
	
	return TRUE;
}


// ========================== //
//            CHAT            //
// ========================== //
BOOL TWLGetChatters(const char *broadcaster_id, const char *moderator_id, char **chatters) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/chatters?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*chatters = TWLReadData(hRequest);
	if(*chatters == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n\n"), *chatters);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetChannelEmotes(const char *broadcaster_id, char **emotes) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/emotes?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*emotes = TWLReadData(hRequest);
	if(*emotes == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *emotes);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGlobalEmotes(char **emotes) {
	
	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", L"/helix/chat/emotes/global", "", &status_code);

	*emotes = TWLReadData(hRequest);
	if(*emotes == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *emotes);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetEmoteSets(const char *emote_set_id, char **emote_sets) {
	
	wchar_t request_query_buffer[256] = {};

	char _id[3500] = {};
	const char *p = emote_set_id;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 25) {

		// This is not really necessary.
		if(i == 0) {
			strcpy(_id + strlen(_id), "emote_set_id=");
			i += strlen("emote_set_id=");
		} else {
			strcpy(_id + strlen(_id), "&emote_set_id=");
			i += strlen("&emote_set_id=");
		}

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			_id[i] = *p;
		}

		p++;
		counter++;
	}

	wchar_t _emote_set_id[3500] = {};	
	mbstowcs(_emote_set_id, _id, ARRAYSIZE(_emote_set_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/emotes/set?%s", _emote_set_id);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*emote_sets = TWLReadData(hRequest);
	if(*emote_sets == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *emote_sets);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetChannelChatBadges(const char *broadcaster_id, char **badges) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/badges?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*badges = TWLReadData(hRequest);
	if(*badges == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *badges);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGlobalChatBadges(char **badges) {
	
	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", L"/helix/chat/badges/global", "", &status_code);

	*badges = TWLReadData(hRequest);
	if(*badges == NULL) return FALSE;
	if(badges == NULL) {
		printf(RED("ERROR: unable to read data!\n"));
		return FALSE;
	}

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *badges);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetChatSettings(const char *broadcaster_id, const char *moderator_id, char **chat_settings) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[100] = {};
	
	if(moderator_id != NULL && strlen(moderator_id) > 0) {
		wchar_t _moderator_id[64] = {};
		mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&moderator_id=%s", _moderator_id);
	}

	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/settings?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*chat_settings = TWLReadData(hRequest);
	if(*chat_settings == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *chat_settings);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetSharedChatSession(const char *broadcaster_id, char **chat_session) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/shared_chat/session?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*chat_session = TWLReadData(hRequest);
	if(*chat_session == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *chat_session);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetUserEmotes(const char *user_id, const char *broadcaster_id, char **emotes) {
	
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[100] = {};

	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {
		wchar_t _broadcaster_id[64] = {};
		mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&broadcaster_id=%s", _broadcaster_id);
	}
	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/emotes/user?user_id=%s%s", _user_id, request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*emotes = TWLReadData(hRequest);
	if(*emotes == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *emotes);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLUpdateChatSettings(const char *broadcaster_id, const char *moderator_id, const FTWLChatSettings *settings, char **updated_settings) {

	wchar_t request_query_buffer[256] = {};

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	wchar_t _moderator_id[64] = {};
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/settings?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);

	char request_body_buffer[4096] = {};
	char request_body_optional[4096] = {};

	if(settings != NULL) {

		sprintf(request_body_optional, "\"emote_mode\":%s", (settings->is_emote_mode == TRUE) ? "true" : "false");
		sprintf(request_body_optional + strlen(request_body_optional), ",\"subscriber_mode\":%s", (settings->is_subscriber_mode == TRUE) ? "true" : "false");
		sprintf(request_body_optional + strlen(request_body_optional), ",\"unique_chat_mode\":%s", (settings->is_unique_chat_mode == TRUE) ? "true" : "false");

		if(settings->is_follower_mode == TRUE) {
			if(settings->follower_mode_duration < 0) {
				sprintf(request_body_optional + strlen(request_body_optional), ",\"follower_mode\":true");
			} else {
				sprintf(request_body_optional + strlen(request_body_optional), 
						",\"follower_mode\":true"
						",\"follower_mode_duration\":%i",
						settings->follower_mode_duration);
			}
		} else {
			sprintf(request_body_optional + strlen(request_body_optional), ",\"follower_mode\":false");
		}

		if(settings->is_non_moderator_chat_delay == TRUE) {
			sprintf(request_body_optional + strlen(request_body_optional), 
					",\"non_moderator_chat_delay\":true"
					",\"non_moderator_chat_delay_duration\":%i",
					settings->non_moderator_chat_delay_duration);
		} else {
			sprintf(request_body_optional + strlen(request_body_optional), ",\"non_moderator_chat_delay\":false");
		}

		if(settings->is_slow_mode == TRUE) {
			if(settings->slow_mode_wait_time < 0) {
				sprintf(request_body_optional + strlen(request_body_optional), ",\"slow_mode\":true");
			} else {
				sprintf(request_body_optional + strlen(request_body_optional),
						",\"slow_mode\":true"
						",\"slow_mode_wait_time\":%i",
						settings->slow_mode_wait_time);
			}
		} else {
			sprintf(request_body_optional + strlen(request_body_optional), ",\"slow_mode\":false");
		}
		

		sprintf(request_body_buffer, "{%s}", request_body_optional);
	}


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);

	*updated_settings =	TWLReadData(hRequest);
	if(*updated_settings == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_settings);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSendChatAnnouncement(
		const char *broadcaster_id,
		const char *moderator_id,
		const char announcement[501],
		const char *color) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/announcements?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);

	char request_body_buffer[4096] = {};
	char request_body_optional[64] = {};

	if(color != NULL && strlen(color) > 0) {
		sprintf(request_body_optional, ",\"color\":\"%s\"", color);
	}

	sprintf(request_body_buffer, 
			"{"
			"\"message\":\"%s\""
			"%s"
			"}",
			announcement,
			request_body_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSendShoutout(const char *from_broadcaster_id, const char *to_broadcaster_id, const char *moderator_id) {
	
	wchar_t _from_broadcaster_id[64] = {};
	wchar_t _to_broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_from_broadcaster_id, from_broadcaster_id, ARRAYSIZE(_from_broadcaster_id));	
	mbstowcs(_to_broadcaster_id, to_broadcaster_id, ARRAYSIZE(_to_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/shoutouts?from_broadcaster_id=%s&to_broadcaster_id=%s&moderator_id=%s", _from_broadcaster_id, _to_broadcaster_id, _moderator_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	
	}

	return TRUE;
}


BOOL TWLSendChatMessage(
		const char *broadcaster_id,
		const char *sender_id,
		const char message[501],
		const char *reply_parent_message_id,
		int32_t for_source_only,
		char **sent_message) {
	
	char request_body_buffer[4096] = {};
	char request_body_optional[128] = {};

	if(reply_parent_message_id != NULL && strlen(reply_parent_message_id) > 0) {
		sprintf(request_body_optional, ",\"reply_parent_message_id\":\"%s\"", reply_parent_message_id);
	}

	sprintf(request_body_buffer, 
			"{"
			"\"broadcaster_id\":\"%s\","
			"\"sender_id\":\"%s\","
			"\"message\":\"%s\""
			"%s"
			"%s"
			"}", 
			broadcaster_id,
			sender_id,
			message,
		   	(for_source_only != 0) ? 
				(for_source_only  > 0) ? ",\"for_source_only\":true" : ",\"for_source_only\":false" 
			: "",
			request_body_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/chat/messages", request_body_buffer, &status_code);

	*sent_message = TWLReadData(hRequest);
	if(*sent_message == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *sent_message);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetUserChatColor(const char *user_id, char **color) {
	
	wchar_t request_query_buffer[8192] = {};
	
	char _ids[7000] = {};
	const char *p = user_id;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 100) {

		strcpy(_ids + strlen(_ids), "&user_id=");
		i += strlen("&user_id=");

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			_ids[i] = *p;
		}

		p++;
		counter++;
	}
	wchar_t _user_ids[7000] = {};
	mbstowcs(_user_ids, _ids, ARRAYSIZE(_user_ids));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/color?%s", _user_ids + 1);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*color = TWLReadData(hRequest);
	if(*color == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *color);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLUpdateUserChatColor(const char *user_id, const char *color) {
	
	wchar_t _user_id[64] = {};
	wchar_t _color[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	mbstowcs(_color, color, ARRAYSIZE(_color));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/chat/color?user_id=%s&color=%s", _user_id, _color);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}
	
	return TRUE;
}


// =========================== //
//            CLIPS            //
// =========================== //
BOOL TWLCreateClip(const char *broadcaster_id, BOOL has_delay, char **clip) {
	
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[16] = {};
	
	if(has_delay == TRUE) {
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&has_delay=true");
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/clips?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);

	*clip = TWLReadData(hRequest);
	if(*clip == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *clip);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetClips(
		const char *broadcaster_id,
		const char *game_id,
		const char *clip_id,
		const char *started_at,
		const char *ended_at,
		int32_t is_featured,
		char **clips) {
	
	wchar_t request_query_buffer[1024] = {};
	wchar_t request_query_optional[256] = {};
	
	if(started_at != NULL && strlen(started_at) > 0) {
		wchar_t _started_at[64] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&started_at=%s", _started_at);
	}
	
	if(ended_at != NULL && strlen(ended_at) > 0) {
		wchar_t _ended_at[64] = {};
		mbstowcs(_ended_at, ended_at, ARRAYSIZE(_ended_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&ended_at=%s", _ended_at);
	}

	if(is_featured > 0) {
		wchar_t _started_at[64] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&is_featured=true");
	}

	/* We ignore this because to get all the clips we just have to do not set the parameter in the request
	if(is_featured == 0) {
	}
	*/

	if(is_featured < 0) {
		wchar_t _started_at[64] = {};
		mbstowcs(_started_at, started_at, ARRAYSIZE(_started_at));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&is_featured=false");
	}

	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {

		wchar_t _broadcaster_id[64] = {};
		mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/clips?broadcaster_id=%s", _broadcaster_id);
	
	} else if(game_id != NULL && strlen(game_id) > 0) {
	
		wchar_t _game_id[64] = {};
		mbstowcs(_game_id, game_id, ARRAYSIZE(_game_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/clips?game_id=%s", _game_id);
	
	} else if(clip_id != NULL && strlen(clip_id) > 0) {

#if 1
		char _ids[7000] = {};
		const char *p = clip_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(_ids + strlen(_ids), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_ids[i] = *p;
			}

			p++;
			counter++;
		}
		wchar_t _clip_ids[7000] = {};
		mbstowcs(_clip_ids, _ids, ARRAYSIZE(_clip_ids));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/clips?%s", _clip_ids + 1);
#endif

	} else {

		printf(YELLOW("WARNING: no parameter set!\n"));
		return FALSE;
	}

	swprintf(request_query_buffer + wcslen(request_query_buffer), ARRAYSIZE(request_query_buffer), L"%s", request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*clips = TWLReadData(hRequest);
	if(*clips == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *clips);
		return FALSE;
	}

	return TRUE;
}


// ============================== //
//            CONDUITS            //
// ============================== //
BOOL TWLGetConduits(char **conduits) {

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", L"/helix/eventsub/conduits", NULL, &status_code);

	*conduits = TWLReadData(hRequest);
	if(*conduits == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *conduits);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreateConduits(int32_t shard_count, char **conduit) {
	
	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer, "{\"shard_count\":%i}", shard_count);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/eventsub/conduits", request_body_buffer, &status_code);

	*conduit = TWLReadData(hRequest);
	if(*conduit == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *conduit);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLUpdateConduits(const char *id, int32_t shard_count, char **updated_conduit) {
	
	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer, "{\"id\":\"%s\",\"shard_count\":%i}", id, shard_count);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", L"/helix/eventsub/conduits", request_body_buffer, &status_code);

	*updated_conduit = TWLReadData(hRequest);
	if(*updated_conduit == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_conduit);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLDeleteConduit(const char *id) {
	
	wchar_t request_query_buffer[256] = {};
	
	wchar_t _id[64] = {};
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/eventsub/conduits?id=%s", _id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetConduitShards(const char *conduit_id, const char *status, char **shards) {
	
	wchar_t _conduit_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[256] = {};
	
	if(status != NULL && strlen(status) > 0) {
		wchar_t _status[64] = {};
		mbstowcs(_status, status, ARRAYSIZE(_status));	
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&status=%s", _status);
	}

	mbstowcs(_conduit_id, conduit_id, strlen(conduit_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/eventsub/conduits/shards?conduit_id=%s%s", _conduit_id, request_query_optional);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*shards = TWLReadData(hRequest);
	if(*shards == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *shards);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateConduitShards(const char *conduit_id, const FTWLShard *shards, char **updated_shards) {
	
	char *request_body_buffer = (char*)malloc(20000*sizeof(FTWLShard));
	memset(request_body_buffer, '\0', 20000*sizeof(FTWLShard));

	char *_shards = (char*)malloc(20000*sizeof(FTWLShard));
	memset(_shards, '\0', 20000*sizeof(FTWLShard));

	char *tmp_format = 
		"{"
		"\"id\":\"%i\","
		"\"transport\":"
		"{"
		"\"method\":\"%s\","
		"%s"
		"}"
		"},";

	char *tmp_method_webhook =
		"\"callback\":\"%s\","
		"\"secret\":\"%s\"";

	char *tmp_method_websocket =
		"\"session_id\":\"%s\"";

	char tmp_method[64] = {};
	char tmp[512] = {};

	char *p = _shards;
	for(size_t i=0; i<20000; i++) {
		if(strlen(shards[i].id) == 0) break;

		if(strcmp(shards[i].method, "websocket") == 0) {
			sprintf(tmp_method, tmp_method_websocket, shards[i].session_id);
		} else {
			sprintf(tmp_method, tmp_method_webhook, shards[i].callback, shards[i].secret);
		}

		sprintf(tmp, tmp_format, i, shards[i].method, tmp_method);
		sprintf(p, "%s", tmp);

		p += strlen(tmp);

	}

	_shards[strlen(_shards)-1] = '\0';

	sprintf(request_body_buffer, 
			"{"
			"\"conduit_id\":\"%s\","
			"\"shards\":[%s]"
			"}",
			conduit_id,
			_shards);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", L"/helix/eventsub/conduits/shards", request_body_buffer, &status_code);

	*updated_shards = TWLReadData(hRequest);
	if(*updated_shards == NULL) return FALSE;

	free(_shards);
	free(request_body_buffer);

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_shards);
		return FALSE;
	}
	
	return TRUE;
}


// ========================== //
//            CCLs            //
// ========================== //
BOOL TWLGetContentClassificationLabels(const char *locale, char **ccls) {
	
	wchar_t _locale[8] = {};
	wchar_t request_query_buffer[256] = {};
	
	if(locale != NULL && strlen(locale) > 0) {
		mbstowcs(_locale, locale, ARRAYSIZE(_locale));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/content_classification_labels?locale=%s", _locale);
	} else {
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/content_classification_labels?locale=en-US");
	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*ccls = TWLReadData(hRequest);
	if(*ccls == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *ccls);
		return FALSE;
	}
	
	return TRUE;
}

	
// ================================== //
//            ENTITLEMENTS            //
// ================================== //
BOOL TWLGetDropsEntitlements(
		const char *id,
		const char *user_id,
		const char *game_id,
		const char *fulfillment_status,
		char **entitlements) {
	
	wchar_t request_query_buffer[8192] = {};
	wchar_t request_query_optional[8192] = {};

	if(id != NULL && strlen(id) > 0) {
#if 1
		char _ids[7000] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			// This is not really necessary.
			if(i == 0) {
				strcpy(_ids + strlen(_ids), "id=");
				i += strlen("id=");
			} else {
				strcpy(_ids + strlen(_ids), "&id=");
				i += strlen("&id=");
			}

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_ids[i] = *p;
			}

			p++;
			counter++;
		}
		wchar_t _clip_ids[7000] = {};
		mbstowcs(_clip_ids, _ids, ARRAYSIZE(_clip_ids));	
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _clip_ids);
#endif
	}

	if(user_id != NULL && strlen(user_id) > 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&user_id=%s", _user_id);
	}
	
	if(game_id != NULL && strlen(game_id) > 0) {
		wchar_t _game_id[64] = {};
		mbstowcs(_game_id, game_id, ARRAYSIZE(_game_id));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&game_id=%s", _game_id);
	}

	if(fulfillment_status != NULL && strlen(fulfillment_status) > 0) {
		wchar_t _fulfillment_status[64] = {};
		mbstowcs(_fulfillment_status, fulfillment_status, ARRAYSIZE(_fulfillment_status));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&fulfillment_status=%s", _fulfillment_status);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/entitlements/drops?%s", request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*entitlements = TWLReadData(hRequest);
	if(*entitlements == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *entitlements);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLUpdateDropsEntitlements(const char *entitlement_ids, const char *fulfillment_status, char **entitlements) {
	
	char request_body_buffer[4096] = {};	
	char request_body_optional[4096] = {};

	if(fulfillment_status != NULL && strlen(fulfillment_status) > 0) {
		sprintf(request_body_optional, "\"fulfillment_status\":\"%s\"", fulfillment_status);
	}

	if(entitlement_ids != NULL && strlen(entitlement_ids) > 0) {
		
		char ids[3300] = {};
		const char *p = entitlement_ids;
		int counter = 0;
		size_t i = 0;
		
		while(*p != '\0' && counter < 100) {
		
			ids[i] = '"';
			i++;
			
			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				ids[i] = *p;
			}
		
			ids[i] = '"';
		
			i++;
			ids[i] = ',';
		
			i++;
			p++;
			counter++;
		}
		ids[--i] = '\0';

		sprintf(request_body_optional + strlen(request_body_optional),
				(strlen(request_body_optional) > 0) ? ",\"entitlement_ids\":[%s]" : "\"entitlement_ids\":[%s]", ids);

	}

	sprintf(request_body_buffer, "{%s}", request_body_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", L"/helix/entitlements/drops", request_body_buffer, &status_code);

	*entitlements = TWLReadData(hRequest);
	if(*entitlements == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *entitlements);
		return FALSE;
	}
	
	return TRUE;
}


// ================================ //
//            EXTENSIONS            //
// ================================ //
BOOL TWLGetExtensionConfigurationSegment(
		const char *extension_id,
		const char *segment,
		const char *broadcaster_id,
		char **configuration_segments) {
	
	wchar_t request_query_buffer[1024] = {};
	wchar_t request_query_optional[1024] = {};

	wchar_t _extension_id[64] = {};
	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));
	
	char _tmp_segments[30] = {};
	const char *p = segment;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 3) {

		strcpy(_tmp_segments + strlen(_tmp_segments), "&segment=");
		i += strlen("&segment=");

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			_tmp_segments[i] = *p;
		}

		p++;
		counter++;
	}

	wchar_t _segments[30] = {};	
	mbstowcs(_segments, _tmp_segments, ARRAYSIZE(_tmp_segments));

	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {
		wchar_t _broadcaster_id[64] = {};
		mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&broadcaster_id=%s", _broadcaster_id);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/extensions/configurations?extension_id=%s%s%s",
			_extension_id,
			_segments,
			request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*configuration_segments = TWLReadData(hRequest);
	if(*configuration_segments == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *configuration_segments);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLSetExtensionConfigurationSegment(
			const char *extension_id,
			const char *segment,
			const char *broadcaster_id,
			const char *content,
			const char *version) {
	
		
	char request_body_buffer[4096] = {};
	char request_body_optional[256] = {};

	if(broadcaster_id != NULL && strlen(broadcaster_id) > 0) {
		sprintf(request_body_optional, ",\"broadcaster_id\":\"%s\"", broadcaster_id);
	}

	if(content != NULL && strlen(content) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"content\":\"%s\"", content);
	}

	if(version != NULL && strlen(version) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"version\":\"%s\"", version);
	}

	sprintf(request_body_buffer, 
			"{"
			"\"extension_id\":\"%s\","
			"\"segment\":\"%s\""
			"%s"
			"}", 
			extension_id,
			segment,
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", L"/helix/extensions/configurations", request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSetExtensionRequiredConfiguration(
			const char *broadcaster_id,
			const char *extension_id,
			const char *extension_version,
			const char *required_configuration) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/required_configuration?broadcaster_id=%s", _broadcaster_id);

	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer,
			"{"
			"\"extension_id\":\"%s\","
			"\"extension_version\":\"%s\","
			"\"required_configuration\":\"%s\""
			"}",
			extension_id,
			extension_version, 
			required_configuration);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSendExtensionPubSubMessage() {

	printf(YELLOW("PUBSUB HAS BEEN DECOMMISIONED!\n"));
	return FALSE;
}


BOOL TWLGetExtensionLiveChannels(const char *extension_id, char **channels) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/live?extension_id=%s", _extension_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*channels = TWLReadData(hRequest);
	if(*channels == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *channels);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetExtensionSecrets(const char *extension_id, char **shared_secrets) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/jwt/secrets?extension_id=%s", _extension_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*shared_secrets = TWLReadData(hRequest);
	if(*shared_secrets == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *shared_secrets);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreateExtensionSecret(const char *extension_id, int32_t delay, char **new_secrets) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	delay = delay < 300 ? 300 : delay;

	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/jwt/secrets?extension_id=%s&delay=%i", _extension_id, delay);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, NULL, &status_code);

	*new_secrets = TWLReadData(hRequest);
	if(*new_secrets == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *new_secrets);
		return FALSE;
	}

	return TRUE;
}
	

BOOL TWLSendExtensionChatMessage(
			const char *broadcaster_id,
			const char *extension_id,
			const char *extension_version,
			const char text[281]) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/chat?broadcaster_id=%s", _broadcaster_id);

	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer,
			"{"
			"\"text\":\"%s\","
			"\"extension_id\":\"%s\","
			"\"extension_version\":\"%s\""
			"}",
			text,
			extension_id,
			extension_version);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}
	

BOOL TWLGetExtensions(const char *extension_id, const char *extension_version, char **extension) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[64] = {};
	
	if(extension_version != NULL && strlen(extension_version) > 0) {
		wchar_t _extension_version[64] = {};
		mbstowcs(_extension_version, extension_version, ARRAYSIZE(_extension_version));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&extension_version=%s", _extension_version);
	}

	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions?extension_id=%s%s", _extension_id, request_query_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetReleasedExtensions(const char *extension_id, const char *extension_version, char **released_extension) {
	
	wchar_t _extension_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[64] = {};
	
	if(extension_version != NULL && strlen(extension_version) > 0) {
		wchar_t _extension_version[64] = {};
		mbstowcs(_extension_version, extension_version, ARRAYSIZE(_extension_version));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&extension_version=%s", _extension_version);
	}

	mbstowcs(_extension_id, extension_id, ARRAYSIZE(_extension_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/extensions/released?extension_id=%s%s", _extension_id, request_query_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*released_extension = TWLReadData(hRequest);
	if(*released_extension == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *released_extension);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetExtensionBitsProducts(BOOL should_include_all, char **bits_products) {

	wchar_t request_query_buffer[256] = {};

	if(should_include_all == TRUE) {
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/bits/extensions?should_include_all=true");
	} else {
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/bits/extensions");
	}


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*bits_products = TWLReadData(hRequest);
	if(*bits_products == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *bits_products);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateExtensionBitsProduct(
			const char *sku,
			const FTWLProductCost *cost,
			const char display_name[256],
			BOOL in_development,
			const char *expiration,
			BOOL is_broadcast,
			char **bits_products) {
	
	char request_body_buffer[4096] = {};
	char request_body_optional[256] = {};

	if(in_development == TRUE) {
		sprintf(request_body_optional, ",\"in_development\":true");
	}

	if(expiration != NULL && strlen(expiration) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"expiration\":\"%s\"", expiration);
	}

	if(is_broadcast == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"is_broadcast\":true");
	}

	sprintf(request_body_buffer, 
			"{"
				"\"sku\":\"%s\","
				"\"cost\":"
				"{"
					"\"amount\":%i,"
					"\"type\":\"%s\""
				"},"
				"\"display_name\":\"%s\""
				"%s"
			"}",
			sku,
			cost->amount,
			cost->type,
			display_name,
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", L"/helix/bits/extensions", request_body_buffer, &status_code);

	*bits_products = TWLReadData(hRequest);
	if(*bits_products == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *bits_products);
		return FALSE;
	}

	return TRUE;
}


// ============================== //
//            EVENTSUB            //
// ============================== //
BOOL TWLEventSub(FTWLEventSubSession *event_sub) {
	
	event_sub->hSession = WinHttpOpen(L"TWL_WebSocketSession", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(event_sub->hSession == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);

		return FALSE;
	}

	event_sub->hConnection = WinHttpConnect(event_sub->hSession, L"eventsub.wss.twitch.tv", INTERNET_DEFAULT_HTTPS_PORT, 0);
	if(event_sub->hConnection == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);

		return FALSE;
	}

	HINTERNET hRequest = WinHttpOpenRequest(event_sub->hConnection, L"GET", L"/ws", L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if(hRequest == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	BOOL result = WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0);
	if(result == FALSE) {
		int error_code = GetLastError();
		char error_message[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, error_message, sizeof(error_message), NULL);
		printf(RED("ERROR CODE: %i\n%s\n"), error_code, error_message);
	}

	result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	result = WinHttpReceiveResponse(hRequest, 0);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	int status_code = 0;
	DWORD query_status_size = sizeof(status_code);
	result = WinHttpQueryHeaders(
			hRequest,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			(LPVOID)&status_code,
			&query_status_size,
			WINHTTP_NO_HEADER_INDEX);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("%s\n"), buffer);
	}

	char status_log[64] = {};
	switch(status_code) {
		case HTTP_STATUS_SWITCH_PROTOCOLS:
			sprintf(status_log, "%s (%i)", GREEN("SWITCH PROTOCOLS"), status_code);
			break;

		case HTTP_STATUS_OK:
			sprintf(status_log, "%s (%i)", GREEN("OK"), status_code);
			break;

		case HTTP_STATUS_NO_CONTENT:
			sprintf(status_log, "%s (%i)", GREEN("NO CONTENT"), status_code);
			break;

		case HTTP_STATUS_BAD_REQUEST:
			sprintf(status_log, "%s (%i)", RED("BAD REQUEST"), status_code);
			break;

		case HTTP_STATUS_DENIED:
			sprintf(status_log, "%s (%i)", RED("ACCESS DENIED"), status_code);
			break;

		case HTTP_STATUS_FORBIDDEN:
			sprintf(status_log, "%s (%i)", RED("FORBIDDEN"), status_code);
			break;

		case HTTP_STATUS_NOT_FOUND:
			sprintf(status_log, "%s (%i)", RED("NOT FOUND"), status_code);
			break;

		case HTTP_STATUS_CONFLICT:
			sprintf(status_log, "%s (%i)", RED("CONFLICT"), status_code);
			break;

		case 429:
			sprintf(status_log, "%s (%i)", RED("TOO MANY REQUESTS"), status_code);
			break;

		case HTTP_STATUS_SERVER_ERROR:
			sprintf(status_log, "%s (%i)", RED("INTERNAL SERVER ERROR"), status_code);
			break;

		default:
			sprintf(status_log, "%s (%i)", YELLOW("WARNING: Undefined status code!"), status_code);
			break;
	}

#if TWL_DEBUG_REQUEST_RESPONSE
	printf("%s\n", status_log);
#endif

	event_sub->hWebsocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
	if(event_sub->hWebsocket == NULL) {
		int error_code = GetLastError();
		char error_message[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, error_message, sizeof(error_message), NULL);
		printf(RED("ERROR CODE: %i\n%s\n"), error_code, error_message);
		return FALSE;
	}

	result = WinHttpCloseHandle(hRequest);
	if(result == FALSE) {
		int error_code = GetLastError();
		char error_message[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, error_message, sizeof(error_message), NULL);
		printf(RED("ERROR CODE: %i\n%s\n"), error_code, error_message);
	}

	PVOID websck_recv_buffer[1024];
	DWORD websck_recv_read;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE websck_recv_buffer_type;
	DWORD websck_recv_result = WinHttpWebSocketReceive(event_sub->hWebsocket, websck_recv_buffer, sizeof(websck_recv_buffer), &websck_recv_read, &websck_recv_buffer_type);
	if(websck_recv_result != NO_ERROR) {
		int error_code = GetLastError();
		char error_message[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, error_message, sizeof(error_message), NULL);
		printf(RED("ERROR CODE: %i\n%s\n"), error_code, error_message);
	}
	
#if TWL_DEBUG_EVENT_READ_DATA
	printf(YELLOW("Received %lu bytes\n"), websck_recv_read);
	printf(CYAN("%s\n"), (char*)websck_recv_buffer);
#endif

	char *p = (char*)websck_recv_buffer;

	while(*p++ != '\0') {

		if(*p == '{') {

			p++;
			char tmp[5] = {};

			tmp[0] = *p++;
			tmp[1] = *p++;
			tmp[2] = *p++;
			tmp[3] = *p++;
			tmp[4] = '\0';


			if(strcmp(tmp, "\"id\"") == 0) {
				p += 2;
				size_t i=0;
				while(*p != '"') {
					event_sub->session_id[i++] = *p++;
				}
				break;
			}
		}
	}

	return TRUE;
}


BOOL TWLEventSubShutdown(const FTWLEventSubSession *event_sub) {
	DWORD wsk_close = WinHttpWebSocketClose(event_sub->hWebsocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
	BOOL result = WinHttpCloseHandle(event_sub->hWebsocket);
	result = WinHttpCloseHandle(event_sub->hConnection);
	result = WinHttpCloseHandle(event_sub->hSession);
	return result && !wsk_close;
}


BOOL TWLCreateEventSubscription(const FTWLEventType *event_type, char **event) {

	char request_body_buffer[4096] = {};
	char *request_body_format = 
		"{"
		"\"type\":\"%s\","
		"\"version\":\"%s\","
		"\"condition\":%s,"
		"\"transport\":%s"
		"}";
	sprintf(request_body_buffer, request_body_format, event_type->type, event_type->version, event_type->condition, event_type->transport);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/eventsub/subscriptions", request_body_buffer, &status_code);

	*event = TWLReadData(hRequest);
	if(*event == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *event);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLDeleteEventSubscription(const char *subscription_id) {
	
	wchar_t request_query_buffer[128] = {};
	wchar_t _subscription_id[64] = {};
	mbstowcs(_subscription_id, subscription_id, ARRAYSIZE(_subscription_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/eventsub/subscriptions?id=%s", _subscription_id);
	
	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, NULL, &status_code);
	
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetEventSubSubscriptions(const char *status, const char *type, const char *user_id, const char *subscription_id, char **subscriptions) {
	
	wchar_t request_query_buffer[1024] = {};
	wchar_t request_query_optional[1024] = {};

	if(status != NULL && strlen(status) > 0) {
		wchar_t _status[64] = {};
		mbstowcs(_status, status, ARRAYSIZE(_status));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?status=%s", _status);
	} else if(type != NULL && strlen(type) > 0) {
		wchar_t _type[64] = {};
		mbstowcs(_type, type, ARRAYSIZE(_type));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?type=%s", _type);
	} else if(user_id != NULL && strlen(user_id) > 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?user_id=%s", _user_id);
	} else if(subscription_id != NULL && strlen(subscription_id) > 0) {
		wchar_t _subscription_id[64] = {};
		mbstowcs(_subscription_id, subscription_id, ARRAYSIZE(_subscription_id));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?subscription_id=%s", _subscription_id);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/eventsub/subscriptions%s", request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*subscriptions = TWLReadData(hRequest);		
	if(*subscriptions == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *subscriptions);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLReadEventData(const FTWLEventSubSession *event_sub, char **data) {
	DWORD ws_recv_read = 0;
	DWORD ws_chunk_size = 2048;
	DWORD ws_total_read = 0;
	int32_t ws_buffer_size = 4096;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE ws_buffer_type = WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE;

	*data = (char*)malloc(ws_buffer_size);
	memset(*data, '\0', ws_buffer_size);
	char *p = *data;

	while(ws_buffer_type == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE) {

		DWORD websck_recv_result = WinHttpWebSocketReceive(event_sub->hWebsocket, p, ws_chunk_size, &ws_recv_read, &ws_buffer_type);
		if(websck_recv_result != NO_ERROR) {
			int error_code = GetLastError();
			char error_message[256];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, error_message, sizeof(error_message), NULL);
			printf(RED("ERROR CODE: %i\n%s\n"), error_code, error_message);
			return FALSE;
		}

		ws_total_read += ws_recv_read;

		if(ws_buffer_type == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE && (ws_total_read + ws_chunk_size) > ws_buffer_size) {
			
			ws_buffer_size += 4096;
			
			char *tmp = (char*)realloc(*data, ws_buffer_size);
			if(tmp == NULL) {
				 printf(RED("ERROR while reallocating the TWLReadEventData internal buffer!\n"));
				 free(*data);
				 return FALSE;
			}

#if TWL_DEBUG_EVENT_READ_DATA
			printf(GREEN("Successfully reallocated TWLReadEventData internal buffer from %i to %i bytes!\n"), ws_buffer_size - 4096, ws_buffer_size);
#endif
			*data = tmp;
			memset(*data + ws_total_read, '\0', ws_buffer_size - ws_total_read);
			p = *data;
			p += ws_total_read;

		} else {
			p += ws_recv_read;
		}

#if TWL_DEBUG_EVENT_READ_DATA
		switch(ws_buffer_type) {
			case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:
				printf(YELLOW("WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:\n"));
				break;
			case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:
				printf(YELLOW("WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:\n"));
				break;
			case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:
				printf(YELLOW("WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE\n"));
				break;
			case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:
				printf(YELLOW("WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE\n"));
				break;
			case WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:
				printf(YELLOW("WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:\n"));
				break;
		}
		printf(YELLOW("Received %lu/%lu bytes Chunk size: %i bytes Buffer size: %i bytes\n"), ws_recv_read, ws_total_read, ws_chunk_size, ws_buffer_size);
#endif
	}

	return TRUE;
}


// =========================== //
//            GAMES            //
// =========================== //
BOOL TWLGetTopGames(char **top_games) {

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", L"/helix/games/top", "", &status_code);

	*top_games = TWLReadData(hRequest);
	if(*top_games == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *top_games);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGames(const char *id, const char *name, const char *igdb_id, char **games) {

	wchar_t request_query_buffer[8192] = {};
	wchar_t request_query_optional[8192] = {};
	
	if(id != NULL && strlen(id) > 0) {
		
		char category_ids[7000] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			// This is not really necessary.
			if(i == 0) {
				strcpy(category_ids + strlen(category_ids), "id=");
				i += strlen("id=");
			} else {
				strcpy(category_ids + strlen(category_ids), "&id=");
				i += strlen("&id=");
			}

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				category_ids[i] = *p;
			}

			p++;
			counter++;
		}
		wchar_t _category_ids[7000] = {};
		mbstowcs(_category_ids, category_ids, ARRAYSIZE(_category_ids));	
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _category_ids);
	}
	
	if(name != NULL && strlen(name) > 0) {
		
		char names[7000] = {};
		const char *p = name;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			// This is not really necessary.
			if(i == 0 && (id == NULL || strlen(id) == 0)) {
				strcpy(names + strlen(names), "name=");
				i += strlen("name=");
			} else {
				strcpy(names + strlen(names), "&name=");
				i += strlen("&name=");
			}

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				names[i] = *p;
			}

			p++;
			counter++;
		}
		wchar_t _names[7000] = {};
		mbstowcs(_names, names, ARRAYSIZE(_names));	
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _names);
	}
	
	if(igdb_id != NULL && strlen(igdb_id) > 0) {
		
		char igdb_ids[7000] = {};
		const char *p = igdb_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			// This is not really necessary.
			if(i == 0 && (id == NULL || strlen(id) == 0) && (name == NULL || strlen(name) == 0)) {
				strcpy(igdb_ids + strlen(igdb_ids), "igdb_id=");
				i += strlen("igdb_id=");
			} else {
				strcpy(igdb_ids + strlen(igdb_ids), "&igdb_id=");
				i += strlen("&igdb_id=");
			}

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				igdb_ids[i] = *p;
			}

			p++;
			counter++;
		}
		wchar_t _igdb_ids[7000] = {};
		mbstowcs(_igdb_ids, igdb_ids, ARRAYSIZE(_igdb_ids));	
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _igdb_ids);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/games?%s", request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*games = TWLReadData(hRequest);
	if(*games == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *games);
		return FALSE;
	}

	return TRUE;
}

	
// =========================== //
//            GOALS            //
// =========================== //
BOOL TWLGetCreatorGoals(const char *broadcaster_id, char **goals) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/goals?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*goals = TWLReadData(hRequest);
	if(*goals == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *goals);
		return FALSE;
	}

	return TRUE;
}


// ================================ //
//            GUEST STAR            //
// ================================ //
BOOL TWLGetChannelGuestStarSettings(const char *broadcaster_id, const char *moderator_id, char **settings) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/channel_settings?broadcaster_id=%s&moderator_id=%s", 
			_broadcaster_id,
			_moderator_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*settings = TWLReadData(hRequest);
	if(*settings == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *settings);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateChannelGuestStarSettings(
		const char *broadcaster_id,
		BOOL is_moderator_send_live_enabled,
		int32_t slot_count,
		BOOL is_browser_source_audio_enabled,
		const char *group_layout,
		BOOL regenerate_browser_sources) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/channel_settings?broadcaster_id=%s", 
			_broadcaster_id);
	
	char request_body_buffer[4096] = {};
	char request_body_optional[1024] = {};

	if(is_moderator_send_live_enabled == TRUE) {
		sprintf(request_body_optional, "\"is_moderator_send_live_enabled\":true");
	} else {
		sprintf(request_body_optional, "\"is_moderator_send_live_enabled\":false");
	}

	if(slot_count > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"slot_count\":%i", slot_count < 7 ? slot_count : 6);
	}

	if(is_browser_source_audio_enabled == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"is_browser_source_audio_enabled\":true");
	} else {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"is_browser_source_audio_enabled\":false");
	}

	if(group_layout != NULL && strlen(group_layout) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"group_layout\":\"%s\"", group_layout);
	}

	if(regenerate_browser_sources == TRUE) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"regenerate_browser_sources\":true");
	} else {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"regenerate_browser_sources\":false");
	}

	sprintf(request_body_buffer, 
			"{"
			"%s"
			"}",
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGuestStarSession(const char *broadcaster_id, const char *moderator_id, char **session) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/session?broadcaster_id=%s&moderator_id=%s", 
			_broadcaster_id,
			_moderator_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*session = TWLReadData(hRequest);
	if(*session == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *session);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreateGuestStarSession(const char *broadcaster_id, char **session) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/session?broadcaster_id=%s", 
			_broadcaster_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, NULL, &status_code);

	*session = TWLReadData(hRequest);
	if(*session == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *session);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLEndGuestStarSession(const char *broadcaster_id, const char *session_id, char **session) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/session?broadcaster_id=%s&session_id=%s", 
			_broadcaster_id,
			_session_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, NULL, &status_code);

	*session = TWLReadData(hRequest);
	if(*session == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *session);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetGuestStarInvites(const char *broadcaster_id, const char *moderator_id, const char *session_id, char **invites) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/invites?broadcaster_id=%s&moderator_id=%s&session_id=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*invites = TWLReadData(hRequest);
	if(*invites == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *invites);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSendGuestStarInvite(const char *broadcaster_id, const char *moderator_id, const char *session_id, const char *guest_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _guest_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_guest_id, guest_id, ARRAYSIZE(_guest_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/invites?broadcaster_id=%s&moderator_id=%s&session_id=%s&guest_id=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_guest_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLDeleteGuestStarInvite(const char *broadcaster_id, const char *moderator_id, const char *session_id, const char *guest_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _guest_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_guest_id, guest_id, ARRAYSIZE(_guest_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/invites?broadcaster_id=%s&moderator_id=%s&session_id=%s&guest_id=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_guest_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLAssignGuestStarSlot(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *session_id,
		const char *guest_id,
		const char *slot_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _guest_id[64] = {};
	wchar_t _slot_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_guest_id, guest_id, ARRAYSIZE(_guest_id));	
	mbstowcs(_slot_id, slot_id, ARRAYSIZE(_slot_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/guest_star/slot?broadcaster_id=%s&moderator_id=%s&session_id=%s&guest_id=%s&slot_id=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_guest_id,
			_slot_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateGuestStarSlot(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *session_id,
		const char *source_slot_id,
		const char *destination_slot_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _source_slot_id[64] = {};
	wchar_t _destination_slot_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_source_slot_id, source_slot_id, ARRAYSIZE(_source_slot_id));	
	mbstowcs(_destination_slot_id, destination_slot_id, ARRAYSIZE(_destination_slot_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
			L"/helix/guest_star/slot?broadcaster_id=%s&moderator_id=%s&session_id=%s&source_slot_id=%s&destination_slot_id=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_source_slot_id,
			_destination_slot_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLDeleteGuestStarSlot(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *session_id,
		const char *guest_id,
		const char *slot_id,
		BOOL should_reinvite_guest) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _guest_id[64] = {};
	wchar_t _slot_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_guest_id, guest_id, ARRAYSIZE(_guest_id));	
	mbstowcs(_slot_id, slot_id, ARRAYSIZE(_slot_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
			L"/helix/guest_star/slot?"
			"broadcaster_id=%s"
			"&moderator_id=%s"
			"&session_id=%s"
			"&guest_id=%s"
			"&slot_id=%s"
			"&should_reinvite_guest=%s", 
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_guest_id,
			_slot_id,
			(should_reinvite_guest == TRUE) ? L"true" : L"false");
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateGuestStarSlotSettings(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *session_id,
		const char *slot_id,
		BOOL is_audio_enabled,
		BOOL is_video_enabled,
		BOOL is_live,
		int32_t volume) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _session_id[64] = {};
	wchar_t _slot_id[64] = {};
	wchar_t request_query_buffer[1024] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_session_id, session_id, ARRAYSIZE(_session_id));	
	mbstowcs(_slot_id, slot_id, ARRAYSIZE(_slot_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
			L"/helix/guest_star/slot_settings?"
			"broadcaster_id=%s"
			"&moderator_id=%s"
			"&session_id=%s"
			"&slot_id=%s"
			"&is_audio_enabled=%s"
			"&is_video_enabled=%s"
			"&is_live=%s"
			"&volume=%i", // WARNING: it give me malformed parameter
			_broadcaster_id,
			_moderator_id,
			_session_id,
			_slot_id,
			(is_audio_enabled == TRUE) ? L"true" : L"false",
			(is_video_enabled == TRUE) ? L"true" : L"false",
			(is_live == TRUE) ? L"true" : L"false",
			volume);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, NULL, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


// ================================ //
//            HYPE TRAIN            //
// ================================ //
BOOL TWLGetHypeTrainEvents(const char *broadcaster_id, char **hype_train_events) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/hypetrain/events?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*hype_train_events = TWLReadData(hRequest);
	if(*hype_train_events == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *hype_train_events);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetHypeTrainStatus(const char *broadcaster_id, char **hype_train_status) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/hypetrain/status?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*hype_train_status = TWLReadData(hRequest);
	if(*hype_train_status == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *hype_train_status);
		return FALSE;
	}

	return TRUE;
}


// ================================ //
//            MODERATION            //
// ================================ //
BOOL TWLCheckAutomodStatus(const char *broadcaster_id, FTWLAutoModMessage messages_to_check[100], char **checked_messages) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/enforcements/status?broadcaster_id=%s", _broadcaster_id);
		
	char request_body_buffer[65536] = {};

	sprintf(request_body_buffer, "{\"data\":[");
	char *p = request_body_buffer;
	p += strlen(request_body_buffer);

	// this buffer is max 500+64+28 characters.
	char tmp_buffer[600] = {};	
	for(size_t i=0; i<100; i++) {

		if(strcmp(messages_to_check[i].msg_id, "") == 0) {
			i > 0 ? p-- : 0;	// we remove the last ',' if we have processed at least one message.
			break;
		}

		memset(tmp_buffer, '\0', 600);
		sprintf(tmp_buffer, 
				"{"
				"\"msg_id\":\"%s\","
				"\"msg_text\":\"%s\""
				"},",
				messages_to_check[i].msg_id,
				messages_to_check[i].msg_text);

		sprintf(p, "%s", tmp_buffer);
		p += strlen(tmp_buffer);
	}
	sprintf(p, "]}");


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*checked_messages = TWLReadData(hRequest);
	if(*checked_messages == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *checked_messages);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLManageHeldAutoModMessages(const char *user_id, const char *msg_id, const char *action) {
	
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, 
			"{"
			"\"user_id\":\"%s\","
			"\"msg_id\":\"%s\","
			"\"action\":\"%s\""
			"}",
			user_id, msg_id, action);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/moderation/automod/message", request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetAutoModSettings(const char *broadcaster_id, const char *moderator_id, char **settings) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/automod/settings?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*settings = TWLReadData(hRequest);
	if(*settings == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *settings);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateAutoModSettings(const char *broadcaster_id, const char *moderator_id, const FTWLAutoModSettings *new_settings, char **updated_settings) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/automod/settings?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);
		
	char request_body_buffer[4096] = {};

	if(new_settings->overall_level >= 0) {
		sprintf(request_body_buffer, 
				"{"
				"\"overall_level\":%i"
				"}",
				new_settings->overall_level);
	} else {
		sprintf(request_body_buffer, 
				"{"
				"\"aggression\":%i,"
				"\"bullying\":%i,"
				"\"disability\":%i,"
				"\"misogyny\":%i,"
				"\"race_ethnicity_or_religion\":%i,"
				"\"sex_based_terms\":%i,"
				"\"sexuality_sex_or_gender\":%i,"
				"\"swearing\":%i"
				"}",
				new_settings->aggresion,
				new_settings->bullying,
				new_settings->disability,
				new_settings->misogyny,
				new_settings->race_ethnicity_or_religion,
				new_settings->sex_based_terms,
				new_settings->sexuality_sex_or_gender,
				new_settings->swearing);
	}


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, request_body_buffer, &status_code);

	*updated_settings = TWLReadData(hRequest);
	if(*updated_settings == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_settings);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetBannedUsers(const char *broadcaster_id, const char *user_id, char **banned_users) {
	
	wchar_t request_query_buffer[4096] = {};
	wchar_t request_query_optional[4096] = {};


	if(user_id != NULL && strlen(user_id) > 0) {
		
		char _user_id[3300] = {};
		const char *p = user_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(_user_id + strlen(_user_id), "&user_id=");
			i += strlen("&user_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_user_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[3300] = {};	
		mbstowcs(_ids, _user_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/banned?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*banned_users = TWLReadData(hRequest);
	if(*banned_users == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *banned_users);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLBanUser(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *user_id,
		int32_t duration,
		const char reason[501],
		char **banned_user) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/bans?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[4096] = {};

	if(duration > 0) {		
		sprintf(request_body_optional, ",\"duration\":%i", duration);
	}

	if(reason != NULL && strlen(reason) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"reason\":\"%s\"", reason);
	}

	sprintf(request_body_buffer, 
			"{"
				"\"data\":"
				"{"
					"\"user_id\":\"%s\"%s"
				"}"
			"}",
			user_id, request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*banned_user = TWLReadData(hRequest);
	if(*banned_user == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *banned_user);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUnbanUser(const char *broadcaster_id, const char *moderator_id, const char *user_id) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/bans?broadcaster_id=%s&moderator_id=%s&user_id=%s", _broadcaster_id, _moderator_id, _user_id);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetUnbanRequest(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *status,
		const char *user_id,
		char **unban_requests) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _status[16] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_status, status, ARRAYSIZE(_status));	
	
	if(user_id != NULL && strlen(user_id) > 0) {

		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
				L"/helix/moderation/unban_requests?broadcaster_id=%s&moderator_id=%s&status=%s&user_id=%s", _broadcaster_id, _moderator_id, _status, _user_id);

	} else {
		
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
				L"/helix/moderation/unban_requests?broadcaster_id=%s&moderator_id=%s&status=%s", _broadcaster_id, _moderator_id, _status);

	}


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*unban_requests = TWLReadData(hRequest);
	if(*unban_requests == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *unban_requests);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLResolveUnbanRequests(
		const char *broadcaster_id,
		const char *moderator_id,
		const char *unban_request_id,
		const char *status,
		const char resolution_text[501],
		char **unban_resolution) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _unban_request_id[64] = {};
	wchar_t _status[16] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_unban_request_id, unban_request_id, ARRAYSIZE(_unban_request_id));	
	mbstowcs(_status, status, ARRAYSIZE(_status));	
	
	if(resolution_text != NULL && strlen(resolution_text) > 0) {

		wchar_t _resolution_text[64] = {};
		mbstowcs(_resolution_text, resolution_text, ARRAYSIZE(_resolution_text));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
				L"/helix/moderation/unban_requests?broadcaster_id=%s&moderator_id=%s&unban_request_id=%s&status=%s&resolution_text=%s", 
				_broadcaster_id,
				_moderator_id,
				_unban_request_id,
				_status,
				_resolution_text);

	} else {
		
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
				L"/helix/moderation/unban_requests?broadcaster_id=%s&moderator_id=%s&unban_request_id=%s&status=%s",
				_broadcaster_id,
				_moderator_id,
				_unban_request_id,
				_status);

	}


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, "", &status_code);

	*unban_resolution = TWLReadData(hRequest);
	if(*unban_resolution == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *unban_resolution);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetBlockedTerms(const char *broadcaster_id, const char *moderator_id, char **blocked_terms) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
			
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
			L"/helix/moderation/blocked_terms?broadcaster_id=%s&moderator_id=%s",
			_broadcaster_id,
			_moderator_id);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*blocked_terms = TWLReadData(hRequest);
	if(*blocked_terms == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *blocked_terms);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLAddBlockedTerm(const char *broadcaster_id, const char *moderator_id, const char text[501], char **blocked_term) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/blocked_terms?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);
		
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, "{\"text\":\"%s\"}", text);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*blocked_term = TWLReadData(hRequest);
	if(*blocked_term == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *blocked_term);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLRemoveBlockedTerm(const char *broadcaster_id, const char *moderator_id, const char *id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/moderation/blocked_terms?broadcaster_id=%s&moderator_id=%s&id=%s", 
			_broadcaster_id, _moderator_id, _id);
		


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLDeleteChatMessages(const char *broadcaster_id, const char *moderator_id, const char *message_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t _message_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	
	if(message_id != NULL && strlen(message_id) > 0) {

		mbstowcs(_message_id, message_id, ARRAYSIZE(_message_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
				L"/helix/moderation/chat?broadcaster_id=%s&moderator_id=%s&message_id=%s", 
				_broadcaster_id, _moderator_id, _message_id);
	
	} else {
		
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
				L"/helix/moderation/chat?broadcaster_id=%s&moderator_id=%s", 
				_broadcaster_id, _moderator_id);

	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetModeratedChannels(const char *user_id, char **moderated_channels) {
	
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/channels?user_id=%s", _user_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*moderated_channels = TWLReadData(hRequest);
	if(*moderated_channels == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *moderated_channels);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetModerators(const char *broadcaster_id, const char *user_id, char **moderators) {
	
	wchar_t request_query_buffer[4096] = {};
	wchar_t request_query_optional[4096] = {};

	if(user_id != NULL && strlen(user_id) > 0) {
		
		char _user_id[3300] = {};
		const char *p = user_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(_user_id + strlen(_user_id), "&user_id=");
			i += strlen("&user_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_user_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[3300] = {};	
		mbstowcs(_ids, _user_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/moderators?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*moderators = TWLReadData(hRequest);
	if(*moderators == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *moderators);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLAddChannelModerator(const char *broadcaster_id, const char *user_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/moderators?broadcaster_id=%s&user_id=%s", _broadcaster_id, _user_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLRemoveChannelModerator(const char *broadcaster_id, const char *user_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/moderators?broadcaster_id=%s&user_id=%s", _broadcaster_id, _user_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetVIPs(const char *broadcaster_id, const char *user_id, char **VIPs) {
	
	wchar_t request_query_buffer[4096] = {};
	wchar_t request_query_optional[4096] = {};

	if(user_id != NULL && strlen(user_id) > 0) {
		
		char _user_id[3300] = {};
		const char *p = user_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(_user_id + strlen(_user_id), "&user_id=");
			i += strlen("&user_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_user_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[3300] = {};	
		mbstowcs(_ids, _user_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/vips?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*VIPs = TWLReadData(hRequest);
	if(*VIPs == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *VIPs);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLAddChannelVIP(const char *broadcaster_id, const char *user_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/vips?broadcaster_id=%s&user_id=%s", _broadcaster_id, _user_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLRemoveChannelVIP(const char *broadcaster_id, const char *user_id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/channels/vips?broadcaster_id=%s&user_id=%s", _broadcaster_id, _user_id);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateShieldModeStatus(const char *broadcaster_id, const char *moderator_id, BOOL is_active, char **shield_mode_status) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/shield_mode?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);
		
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, "{\"is_active\":%s}", (is_active == TRUE) ? "true" : "false");


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, request_body_buffer, &status_code);

	*shield_mode_status = TWLReadData(hRequest);
	if(*shield_mode_status == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *shield_mode_status);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetShieldModeStatus(const char *broadcaster_id, const char *moderator_id, char **shield_mode_status) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/shield_mode?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*shield_mode_status = TWLReadData(hRequest);
	if(*shield_mode_status == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *shield_mode_status);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLWarnChatUser(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *user_id,
			const char reason[501],
			char **warning_info) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _moderator_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_moderator_id, moderator_id, ARRAYSIZE(_moderator_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/moderation/warnings?broadcaster_id=%s&moderator_id=%s", _broadcaster_id, _moderator_id);
		
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, "{\"data\":{\"user_id\":\"%s\",\"reason\":\"%s\"}}", user_id, reason);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*warning_info = TWLReadData(hRequest);
	if(*warning_info == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *warning_info);
		return FALSE;
	}

	return TRUE;
}
// =========================== //
//            POLLS            //
// =========================== //
BOOL TWLGetPolls(const char *broadcaster_id, const char *id, char **polls) {

	wchar_t request_query_buffer[2048] = {};
	wchar_t request_query_optional[1400] = {};
	
	if(id != NULL && strlen(id) > 0) {
		
		char _user_id[1400] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 20) {

			strcpy(_user_id + strlen(_user_id), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_user_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[1400] = {};	
		mbstowcs(_ids, _user_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/polls?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*polls = TWLReadData(hRequest);
	if(*polls == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *polls);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreatePoll(
		const char *broadcaster_id, 
		const char title[61],
		const char choices[5][26],
		int32_t duration,
		BOOL channel_points_voting_enabled,
		int32_t channel_points_per_vote,
		char **poll) {
	

	char request_body_buffer[4096] = {};
	char request_body_optional[1024] = {};

	if(channel_points_voting_enabled == TRUE) {
		sprintf(request_body_optional, 
				",\"channel_points_voting_enabled\":true,"
				"\"channel_points_per_vote\":%i", 
				channel_points_per_vote);
	}


	sprintf(request_body_buffer, 
			"{"
				"\"broadcaster_id\":\"%s\","
				"\"title\":\"%s\","
				"\"choices\":[",
			broadcaster_id,
			title);

	char *p = request_body_buffer + strlen(request_body_buffer);

	for(size_t i=0; i<5; i++) {
		if(strlen(choices[i]) == 0) break;

		char tmp[40];
		sprintf(tmp, "{\"title\":\"%s\"},", choices[i]);
		sprintf(p, "%s", tmp);
		p += strlen(tmp);
	}

	// remove the last ','
	p--;

	sprintf(p,
			"],"
			"\"duration\":%i"
			"%s"
			"}", 
			duration,
			request_body_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/polls", request_body_buffer, &status_code);

	*poll = TWLReadData(hRequest);
	if(*poll == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *poll);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLEndPoll(const char *broadcaster_id, const char *id, const char *status, char **poll) {
	
	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer, 
			"{"
			"\"broadcaster_id\":\"%s\","
			"\"id\":\"%s\","
			"\"status\":\"%s\""
			"}", broadcaster_id, id, status);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", L"/helix/polls", request_body_buffer, &status_code);

	*poll = TWLReadData(hRequest);
	if(*poll == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *poll);
		return FALSE;
	}

	return TRUE;
}


// ================================= //
//            PREDICTIONS            //
// ================================= //
BOOL TWLGetPredictions(const char *broadcaster_id, const char *id, char **predictions) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[2048] = {};
	wchar_t request_query_optional[2048] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	if(id != NULL && strlen(id) > 0) {
		
		char _prediction_id[2048] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 25) {

			strcpy(_prediction_id + strlen(_prediction_id), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_prediction_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[2048] = {};	
		mbstowcs(_ids, _prediction_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer),
			L"/helix/predictions?broadcaster_id=%s%s",
			_broadcaster_id,
			request_query_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*predictions = TWLReadData(hRequest);
	if(*predictions == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *predictions);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreatePrediction(
			const char *broadcaster_id,
			const char title[46],
			const char outcomes[10][26],
			int32_t prediction_window,
			char **prediction) {
	
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, 
			"{"
				"\"broadcaster_id\":\"%s\","
				"\"title\":\"%s\","
				"\"outcomes\":[",
			broadcaster_id,
			title);

	char *p = request_body_buffer + strlen(request_body_buffer);

	for(size_t i=0; i<10; i++) {
		if(strlen(outcomes[i]) == 0) break;

		char tmp[40];
		sprintf(tmp, "{\"title\":\"%s\"},", outcomes[i]);
		sprintf(p, "%s", tmp);
		p += strlen(tmp);
	}

	// remove the last ','
	p--;

	sprintf(p,
			"],"
			"\"prediction_window\":%i"			
			"}", 
			prediction_window);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/predictions", request_body_buffer, &status_code);

	*prediction = TWLReadData(hRequest);
	if(*prediction == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *prediction);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLEndPrediction(const char *broadcaster_id, const char *id, const char *status, const char *winning_outcome_id, char **prediction) {
	
	char request_body_buffer[4096] = {};

	if(strcmp(status, "RESOLVED") == 0) {

		sprintf(request_body_buffer, 
				"{"
				"\"broadcaster_id\":\"%s\","
				"\"id\":\"%s\","
				"\"status\":\"%s\","
				"\"winning_outcome_id\":\"%s\""
				"}", broadcaster_id, id, status, winning_outcome_id);

	} else {

	sprintf(request_body_buffer, 
			"{"
			"\"broadcaster_id\":\"%s\","
			"\"id\":\"%s\","
			"\"status\":\"%s\""
			"}", broadcaster_id, id, status);

	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", L"/helix/predictions", request_body_buffer, &status_code);

	*prediction = TWLReadData(hRequest);
	if(*prediction == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *prediction);
		return FALSE;
	}

	return TRUE;
}


// =========================== //
//            RAIDS            //
// =========================== //
BOOL TWLStartRaid(const char *from_broadcaster_id, const char *to_broadcaster_id, char **data) {
	wchar_t _from_broadcaster_id[64] = {};
	wchar_t _to_broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_from_broadcaster_id, from_broadcaster_id, ARRAYSIZE(_from_broadcaster_id));	
	mbstowcs(_to_broadcaster_id, to_broadcaster_id, ARRAYSIZE(_to_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/raids?from_broadcaster_id=%s&to_broadcaster_id=%s", _from_broadcaster_id, _to_broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, "", &status_code);

	*data = TWLReadData(hRequest);
	if(*data == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *data);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCancelRaid(const char *broadcaster_id) {
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/raids?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


// ============================== //
//            SCHEDULE            //
// ============================== //
BOOL TWLGetChannelStreamSchedule(const char *broadcaster_id, const char *id, const char *start_time, char **schedule) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[12000] = {};
	wchar_t request_query_optional[12000] = {};

	if(id != NULL && strlen(id) > 0) {
		
		char _prediction_id[12000] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(_prediction_id + strlen(_prediction_id), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				_prediction_id[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[12000] = {};	
		mbstowcs(_ids, _prediction_id, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	if(start_time != NULL && strlen(start_time) > 0) {
		wchar_t _start_time[32] = {};
		mbstowcs(_start_time, start_time, ARRAYSIZE(_start_time));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&start_time=%s", _start_time);
	}

	mbstowcs(_broadcaster_id, broadcaster_id, strlen(broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule?broadcaster_id=%s%s", _broadcaster_id, request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*schedule = TWLReadData(hRequest);
	if(*schedule == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *schedule);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetChanneliCalendar(const char *broadcaster_id, char **iCalendar) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule/icalendar?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*iCalendar = TWLReadData(hRequest);
	if(*iCalendar == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *iCalendar);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateChannelStreamSchedule(
			const char *broadcaster_id,
			const BOOL is_vacation_enabled,
			const char *vacation_start_time,
			const char *vacation_end_time,
			const char *timezone) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	
	if(is_vacation_enabled == TRUE) {
	
		wchar_t _vacation_start_time[32] = {};
		mbstowcs(_vacation_start_time, vacation_start_time, ARRAYSIZE(_vacation_start_time));
		
		wchar_t _vacation_end_time[32] = {};
		mbstowcs(_vacation_end_time, vacation_end_time, ARRAYSIZE(_vacation_end_time));
		
		wchar_t _timezone[32] = {};
		mbstowcs(_timezone, timezone, ARRAYSIZE(_timezone));

		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
				L"/helix/schedule/settings?broadcaster_id=%s"
				"&is_vacation_enabled=true"
				"&vacation_start_time=%s"
				"&vacation_end_time=%s"
				"&timezone=%s",
				_broadcaster_id, _vacation_start_time, _vacation_end_time, _timezone);
	
	} else {
	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule/settings?broadcaster_id=%s&is_vacation_enabled=false", _broadcaster_id);

	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreateChannelStreamScheduleSegment(
		const char *broadcaster_id,
		const char *start_time,
		const char *timezone,
		const char *duration,
		const BOOL is_recurring,
		const char *category_id,
		const char *title,
		char **scheduled_stream) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule/segment?broadcaster_id=%s", _broadcaster_id);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[256] = {};

	if(category_id != NULL && strlen(category_id) > 0) {
		sprintf(request_body_optional, ",\"category_id\":\"%s\"", category_id);
	}

	if(title != NULL && strlen(title) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"title\":\"%s\"", title);
	}

	sprintf(request_body_buffer, 
			"{"
			"\"start_time\":\"%s\","
			"\"timezone\":\"%s\","
			"\"is_recurring\":%s,"
			"\"duration\":\"%s\""
			"%s"
			"}", 
			start_time,
			timezone,
			(is_recurring == TRUE) ? "true" : "false",
			duration,
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);

	*scheduled_stream = TWLReadData(hRequest);
	if(*scheduled_stream == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *scheduled_stream);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateChannelStreamScheduleSegment(
			const char *broadcaster_id,
			const char *id,
			const char *start_time,
			const char *duration,
			const char *category_id,
			const char *title,
			const BOOL is_canceled,
			const char *timezone,
			char **scheduled_stream) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _id[128] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule/segment?broadcaster_id=%s&id=%s", _broadcaster_id, _id);
		
	char request_body_buffer[4096] = {};
	char request_body_optional[256] = {};

	if(start_time != NULL && strlen(start_time) > 0) {
		sprintf(request_body_optional, ",\"start_time\":\"%s\"", start_time);
	}

	if(duration != NULL && strlen(duration) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"duration\":\"%s\"", duration);
	}

	if(category_id != NULL && strlen(category_id) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"category_id\":\"%s\"", category_id);
	}

	if(title != NULL && strlen(title) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"title\":\"%s\"", title);
	}

	if(timezone != NULL && strlen(timezone) > 0) {
		sprintf(request_body_optional + strlen(request_body_optional), ",\"time_zone\":\"%s\"", timezone);
	}

	sprintf(request_body_buffer, 
			"{"
			"\"is_canceled\":%s"
			"%s"
			"}", 
			(is_canceled == TRUE) ? "true" : "false",
			request_body_optional);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PATCH", request_query_buffer, request_body_buffer, &status_code);

	*scheduled_stream = TWLReadData(hRequest);
	if(*scheduled_stream == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *scheduled_stream);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLDeleteChannelStreamScheduleSegment(const char *broadcaster_id, const char *id) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t _id[128] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_id, id, ARRAYSIZE(_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/schedule/segment?broadcaster_id=%s&id=%s", _broadcaster_id, _id);
		

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


// ============================ //
//            SEARCH            //
// ============================ //
BOOL TWLSearchCategories(const char *query, char ** matches) {
	
	wchar_t _query[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_query, query, ARRAYSIZE(_query));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/search/categories?query=%s", _query);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*matches = TWLReadData(hRequest);
	if(*matches == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *matches);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLSearchChannels(const char *query, const BOOL live_only, char **matches) {

	wchar_t _query[64] = {};
	wchar_t *_live_only = live_only == TRUE ? L"true" : L"false";
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_query, query, ARRAYSIZE(_query));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/search/channels?query=%s&live_only=%s", _query, _live_only);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*matches = TWLReadData(hRequest);
	if(*matches == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *matches);
		return FALSE;
	}

	return TRUE;
}

// ============================= //
//            STREAMS            //
// ============================= //
BOOL TWLGetStreamKey(const char *broadcaster_id, char **stream_key) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/streams/key?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*stream_key = TWLReadData(hRequest);
	if(*stream_key == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *stream_key);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetStreams(
			const char *user_id,
			const char *user_login,
			const char *game_id,
			const char *type,
			const char *language,
			char **streams) {

	wchar_t request_query_buffer[27000] = {};
	wchar_t request_query_optional[27000] = {};

	if(user_id != NULL && strlen(user_id) > 0) {

		char user_ids[7500] = {};
		const char *p = user_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(user_ids + strlen(user_ids), "&user_id=");
			i += strlen("&user_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				user_ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7500] = {};
		mbstowcs(_ids, user_ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	if(user_login != NULL && strlen(user_login) > 0) {

		char user_logins[4000] = {};
		const char *p = user_login;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(user_logins + strlen(user_logins), "&user_login=");
			i += strlen("&user_login=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				user_logins[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _logins[4000] = {};	
		mbstowcs(_logins, user_logins, ARRAYSIZE(_logins));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _logins);
	}

	if(game_id != NULL && strlen(game_id) > 0) {

		char game_ids[7500] = {};
		const char *p = game_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(game_ids + strlen(game_ids), "&game_id=");
			i += strlen("&game_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				game_ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7500] = {};
		mbstowcs(_ids, game_ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	if(type != NULL && strlen(type) > 0) {
		wchar_t _type[5] = {};
		mbstowcs(_type, type, ARRAYSIZE(_type));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&type=%s", _type);
	}

	if(language != NULL && strlen(language) > 0) {

		if(strcmp(language, "other") == 0) {
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&language=other");
		} else {
			char languages[7500] = {};
			const char *p = language;
			int counter = 0;
			size_t i = 0;

			while(*p != '\0' && counter < 100) {

				strcpy(languages + strlen(languages), "&language=");
				i += strlen("&language=");

				for(; *p != '|'; i++, p++) {
					if(*p == '\0') {p--; break;}
					languages[i] = *p;
				}

				p++;
				counter++;
			}

			wchar_t _languages[7500] = {};
			mbstowcs(_languages, languages, ARRAYSIZE(_languages));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _languages);
		}
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/streams?%s", request_query_optional+1);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, NULL, &status_code);

	*streams = TWLReadData(hRequest);
	if(*streams == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *streams);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetFollowedStreams(const char *user_id, char **followed_streams) {

	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/streams/followed?user_id=%s", _user_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*followed_streams = TWLReadData(hRequest);
	if(*followed_streams == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *followed_streams);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCreateStreamMarker(const char *user_id, const char description[141], char **marker) {

	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer,
			"{"
				"\"user_id\":\"%s\","
				"\"description\":\"%s\""
			"}",
			user_id,
			description);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", L"/helix/streams/markers", request_body_buffer, &status_code);

	*marker = TWLReadData(hRequest);
	if(*marker == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *marker);
		return FALSE;
	}
	
	return TRUE;
}


BOOL TWLGetStreamMarkers(const char *user_id, const char *video_id, char **markers) {
	
	wchar_t request_query_buffer[256] = {};
	
	if(video_id == NULL || strlen(video_id) == 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/streams/markers?user_id=%s", _user_id);
	} else {
		wchar_t _video_id[64] = {};
		mbstowcs(_video_id, video_id, ARRAYSIZE(_video_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/streams/markers?video_id=%s", _video_id);
	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*markers = TWLReadData(hRequest);
	if(*markers == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *markers);
		return FALSE;
	}

	return TRUE;
}


// =================================== //
//            SUBSCRIPTIONS            //
// =================================== //
BOOL TWLGetBroadcasterSubscriptions(const char *broadcaster_id, const char *user_id, char **subscribers) {

	wchar_t request_query_buffer[8000] = {};
	wchar_t request_query_optional[7500] = {};
	 
	if(user_id != NULL && strlen(user_id) > 0) {

		char user_ids[7500] = {};
		const char *p = user_id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(user_ids + strlen(user_ids), "&user_id=");
			i += strlen("&user_id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				user_ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7500] = {};
		mbstowcs(_ids, user_ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}

	wchar_t _broadcaster_id[64] = {};
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/subscriptions?broadcaster_id=%s%s", 
			_broadcaster_id, 
			request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*subscribers = TWLReadData(hRequest);
	if(*subscribers == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *subscribers);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLCheckUserSubscription(const char *broadcaster_id, const char *user_id, char **subscription) {

	wchar_t _broadcaster_id[64] = {};
	wchar_t _user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/subscriptions/user?broadcaster_id=%s&user_id=%s", _broadcaster_id, _user_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*subscription = TWLReadData(hRequest);
	if(*subscription == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *subscription);
		return FALSE;
	}

	return TRUE;
}


// =========================== //
//            TEAMS            //
// =========================== //
BOOL TWLGetChannelTeams(const char *broadcaster_id, char **teams) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/teams/channel?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*teams = TWLReadData(hRequest);
	if(*teams == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *teams);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetTeams(const char *name, const char *id, char **team) {
	
	wchar_t request_query_buffer[256] = {};
	
	if(name != NULL && strlen(name) >0) {
	
		wchar_t _name[64] = {};
		mbstowcs(_name, name, ARRAYSIZE(_name));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/teams?name=%s", _name);

	} else if(id != NULL && strlen(id) > 0) {
		
		wchar_t _id[64] = {};
		mbstowcs(_id, id, ARRAYSIZE(_id));
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/teams?id=%s", _id);

	}

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*team = TWLReadData(hRequest);
	if(*team == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *team);
		return FALSE;
	}

	return TRUE;
}


// =========================== //
//            USERS            //
// =========================== //
BOOL TWLGetUsers(const char *id, const char *login, char **users) {

	wchar_t request_query_buffer[10500] = {};
	wchar_t request_query_optional[10500] = {};

	if(id != NULL && strlen(id) > 0) {

		char ids[7500] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(ids + strlen(ids), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7500] = {};
		mbstowcs(_ids, ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"%s", _ids);
	}
	
	if(login != NULL && strlen(login) > 0) {

		char logins[3500] = {};
		const char *p = login;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(logins + strlen(logins), "&login=");
			i += strlen("&login=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				logins[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _logins[3500] = {};
		mbstowcs(_logins, logins, ARRAYSIZE(_logins));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"%s", _logins);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), 
			L"/helix/users?%s", 
			request_query_optional+1);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*users = TWLReadData(hRequest);
	if(*users == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *users);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateUser(const char description[301], char **user) {
	
	wchar_t request_query_buffer[512] = {};
	
	wchar_t _description[301] = {};
	mbstowcs(_description, description, ARRAYSIZE(_description));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/users?description=%s", _description);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, "", &status_code);

	*user = TWLReadData(hRequest);
	if(*user == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *user);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetUserBlockList(const char *broadcaster_id, char **blocked_users) {
	
	wchar_t _broadcaster_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_broadcaster_id, broadcaster_id, ARRAYSIZE(_broadcaster_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/users/blocks?broadcaster_id=%s", _broadcaster_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*blocked_users = TWLReadData(hRequest);
	if(*blocked_users == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *blocked_users);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLBlockUser(const char *target_user_id, const char *source_context, const char *reason) {
	
	wchar_t _target_user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[256] = {};

	if(source_context != NULL && strlen(source_context) > 0) {
		wchar_t _source_context[8] = {};
		mbstowcs(_source_context, source_context, ARRAYSIZE(_source_context));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&source_context=%s", _source_context);
	}

	if(reason != NULL && strlen(reason) > 0) {
		wchar_t _reason[16] = {};
		mbstowcs(_reason, reason, ARRAYSIZE(_reason));
		swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&reason=%s", _reason);
	}

	mbstowcs(_target_user_id, target_user_id, ARRAYSIZE(_target_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/users/blocks?target_user_id=%s%s", _target_user_id, request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUnblockUser(const char *target_user_id) {
	
	wchar_t _target_user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_target_user_id, target_user_id, ARRAYSIZE(_target_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/users/blocks?target_user_id=%s", _target_user_id);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetUserExtensions(char **extensions) {
	
	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", L"/helix/users/extensions/list", "", &status_code);

	*extensions = TWLReadData(hRequest);
	if(*extensions == NULL) return FALSE;
	
	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *extensions);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLGetUserActiveExtensions(const char *user_id, char **active_extensions) {
	
	wchar_t request_query_buffer[256] = {};
	wchar_t request_query_optional[100] = {};
	
	if(user_id != NULL && strlen(user_id) > 0) {
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"?user_id=%s", _user_id);
	}

	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/users/extensions%s", request_query_optional);
	

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*active_extensions = TWLReadData(hRequest);
	if(*active_extensions == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *active_extensions);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLUpdateUserExtensions(const char *data, char **updated_extensions) {
	
	char request_body_buffer[4096] = {};
	sprintf(request_body_buffer, "{%s}", data);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"PUT", L"/helix/users/extensions", request_body_buffer, &status_code);

	*updated_extensions = TWLReadData(hRequest);
	if(*updated_extensions == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *updated_extensions);
		return FALSE;
	}

	return TRUE;
}


// ============================ //
//            VIDEOS            //
// ============================ //
BOOL TWLGetVideos(
		const char *id, 
		const char *user_id, 
		const char *game_id, 
		const char *language,
		const char *period,
		const char *sort,
		const char *type,
		char **videos) {

	
	wchar_t request_query_buffer[7000] = {};
	wchar_t request_query_optional[200] = {};
	
	if(id != NULL && strlen(id) > 0) {

		char ids[7000] = {};
		const char *p = id;
		int counter = 0;
		size_t i = 0;

		while(*p != '\0' && counter < 100) {

			strcpy(ids + strlen(ids), "&id=");
			i += strlen("&id=");

			for(; *p != '|'; i++, p++) {
				if(*p == '\0') {p--; break;}
				ids[i] = *p;
			}

			p++;
			counter++;
		}

		wchar_t _ids[7000] = {};
		mbstowcs(_ids, ids, ARRAYSIZE(_ids));
		swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"/helix/videos?%s", _ids + 1);

	} else if(user_id != NULL && strlen(user_id) > 0) {
	
		wchar_t _user_id[64] = {};
		mbstowcs(_user_id, user_id, ARRAYSIZE(_user_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/videos?user_id=%s", _user_id);

		if(period != NULL && strlen(period) > 0) {
			wchar_t _period[6] = {};
			mbstowcs(_period, period, ARRAYSIZE(_period));
			swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&period=%s", _period);
		}
		
		if(sort != NULL && strlen(sort) > 0) {
			wchar_t _sort[9] = {};
			mbstowcs(_sort, sort, ARRAYSIZE(_sort));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&sort=%s", _sort);
		}

		if(type != NULL && strlen(type) > 0) {
			wchar_t _type[10] = {};
			mbstowcs(_type, type, ARRAYSIZE(_type));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&type=%s", _type);
		}

	} else if(game_id != NULL && strlen(game_id) > 0) {
	
		wchar_t _game_id[64] = {};
		mbstowcs(_game_id, game_id, ARRAYSIZE(_game_id));	
		swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/videos?game_id=%s", _game_id);
		
		if(language != NULL && strlen(language) > 0) {
			wchar_t _language[6] = {};
			mbstowcs(_language, language, ARRAYSIZE(_language));
			swprintf(request_query_optional, ARRAYSIZE(request_query_optional), L"&language=%s", _language);
		}
		if(period != NULL && strlen(period) > 0) {
			wchar_t _period[6] = {};
			mbstowcs(_period, period, ARRAYSIZE(_period));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&period=%s", _period);
		}
		
		if(sort != NULL && strlen(sort) > 0) {
			wchar_t _sort[9] = {};
			mbstowcs(_sort, sort, ARRAYSIZE(_sort));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&sort=%s", _sort);
		}

		if(type != NULL && strlen(type) > 0) {
			wchar_t _type[10] = {};
			mbstowcs(_type, type, ARRAYSIZE(_type));
			swprintf(request_query_optional + wcslen(request_query_optional), ARRAYSIZE(request_query_optional), L"&type=%s", _type);
		}
	}

	swprintf(request_query_buffer + wcslen(request_query_buffer), ARRAYSIZE(request_query_buffer), L"%s", request_query_optional);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"GET", request_query_buffer, "", &status_code);

	*videos = TWLReadData(hRequest);
	if(*videos == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *videos);
		return FALSE;
	}

	return TRUE;
}


BOOL TWLDeleteVideos(const char *id, char **deleted_videos) {
	
	wchar_t request_query_buffer[512] = {};
	
	char ids[350] = {};
	const char *p = id;
	int counter = 0;
	size_t i = 0;

	while(*p != '\0' && counter < 5) {

		strcpy(ids + strlen(ids), "&id=");
		i += strlen("&id=");

		for(; *p != '|'; i++, p++) {
			if(*p == '\0') {p--; break;}
			ids[i] = *p;
		}

		p++;
		counter++;
	}

	wchar_t _ids[350] = {};
	mbstowcs(_ids, ids, ARRAYSIZE(_ids));
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/videos?%s", _ids+1);


	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"DELETE", request_query_buffer, "", &status_code);

	*deleted_videos = TWLReadData(hRequest);
	if(*deleted_videos == NULL) return FALSE;

	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), *deleted_videos);
		return FALSE;
	}

	return TRUE;
}


// ============================== //
//            WHISPERS            //
// ============================== //
BOOL TWLSendWhisper(const char *from_user_id, const char *to_user_id, const char *message) {
	
	wchar_t _from_user_id[64] = {};
	wchar_t _to_user_id[64] = {};
	wchar_t request_query_buffer[256] = {};
	
	mbstowcs(_from_user_id, from_user_id, ARRAYSIZE(_from_user_id));	
	mbstowcs(_to_user_id, to_user_id, ARRAYSIZE(_to_user_id));	
	swprintf(request_query_buffer, ARRAYSIZE(request_query_buffer), L"/helix/whispers?from_user_id=%s&to_user_id=%s", _from_user_id, _to_user_id);
		
	char request_body_buffer[4096] = {};

	sprintf(request_body_buffer, "{""\"message\":\"%s\"}", message);

	int status_code = 0;
	HINTERNET hRequest = TWLMakeRequest(L"POST", request_query_buffer, request_body_buffer, &status_code);


	if(status_code >= 400) {

#if TWL_DEBUG_REQUEST_HEADERS
		TWLDebugQueryHeaders(hRequest);
#endif
		char *data_buffer = TWLReadData(hRequest);
		if(data_buffer == NULL) return FALSE;
		printf(RED("ERROR:\n"));
		printf(MAGENTA("%s\n"), data_buffer);
		free(data_buffer);
		return FALSE;
	}

	return TRUE;
}

// =========================== //		
//          UTILITIES          //
// =========================== //		


// =========================== //		
//          INTERNALS          //
// =========================== //		
HINTERNET TWLMakeRequest(const wchar_t *verb, const wchar_t *request_query, const char *request_body, int *status_code) {

	// So we can set request_body parameter as NULL pointer or ""
	// when we don't need it and avoid the function crash later because of strln(NULL)
	if(request_body == NULL) {
		request_body = "";
	}

#if TWL_DEBUG_REQUEST_QUERY
	printf(YELLOW("REQUEST QUERY CONTENT:\n"));
	wprintf(MAGENTA(L"- %s\n"), request_query);
#endif

#if TWL_DEBUG_REQUEST_BODY	
	printf(YELLOW("REQUEST BODY CONTENT:\n"));
	printf(MAGENTA("- %s\n"), request_body);
#endif

	HINTERNET hRequest = WinHttpOpenRequest(g_current_user->hConnection, verb, request_query, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if(hRequest == NULL) {
		int error = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), 0);		
		OutputDebugString(buffer);
		OutputDebugString("\n");
		printf(RED("ERROR - Open Request failed:\n%s\n"), buffer);
	}

	wchar_t req_headers_buffer[4096] = {};
	swprintf(req_headers_buffer, ARRAYSIZE(req_headers_buffer),
			L"Authorization: Bearer %s\r\nClient-Id: %s\r\nContent-Type: application/json\r\n",
			g_current_user->oauth2_token,
			g_current_user->client_id);

	BOOL result = WinHttpSendRequest(hRequest, req_headers_buffer, (DWORD)-1L, (LPVOID)request_body, (DWORD)strlen(request_body), (DWORD)strlen(request_body), 0);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("ERROR - Send Request failed:\n%s\n"), buffer);
	}

	result = WinHttpReceiveResponse(hRequest, 0);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("ERROR - Receive Response failed:\n%s\n"), buffer);
	}

	DWORD query_status_size = sizeof(*status_code);
	result = WinHttpQueryHeaders(
			hRequest,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			(LPVOID)status_code,
			&query_status_size,
			WINHTTP_NO_HEADER_INDEX);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
		printf(RED("ERROR - Query Headers failed:\n%s\n"), buffer);
	}

	char status_log[64] = {};
	switch(*status_code) {
		case HTTP_STATUS_OK:
			sprintf(status_log, "%s (%i)", GREEN("OK"), *status_code);
			break;

		case HTTP_STATUS_ACCEPTED:
			sprintf(status_log, "%s (%i)", GREEN("ACCEPTED"), *status_code);
			break;

		case HTTP_STATUS_NO_CONTENT:
			sprintf(status_log, "%s (%i)", GREEN("NO CONTENT"), *status_code);
			break;

		case HTTP_STATUS_BAD_REQUEST:
			sprintf(status_log, "%s (%i)", RED("BAD REQUEST"), *status_code);
			break;

		case HTTP_STATUS_DENIED:
			sprintf(status_log, "%s (%i)", RED("ACCESS DENIED"), *status_code);
			break;

		case HTTP_STATUS_FORBIDDEN:
			sprintf(status_log, "%s (%i)", RED("FORBIDDEN"), *status_code);
			break;

		case HTTP_STATUS_NOT_FOUND:
			sprintf(status_log, "%s (%i)", RED("NOT FOUND"), *status_code);
			break;

		case HTTP_STATUS_CONFLICT:
			sprintf(status_log, "%s (%i)", RED("CONFLICT"), *status_code);
			break;

		case 422:
			printf(status_log, "%s (%i)", RED("UNPROCESSABLE ENTITY"), *status_code);
			break;

		case 429:
			sprintf(status_log, "%s (%i)", RED("TOO MANY REQUESTS"), *status_code);
			break;

		case HTTP_STATUS_SERVER_ERROR:
			sprintf(status_log, "%s (%i)", RED("INTERNAL SERVER ERROR"), *status_code);
			break;

		default:
			sprintf(status_log, "%s (%i)", YELLOW("WARNING: Undefined status code!"), *status_code);
			break;			
	}

#if TWL_DEBUG_REQUEST_RESPONSE
	printf("%s\n", status_log);
#endif
	return hRequest;
}


char *TWLReadData(const HINTERNET hRequest) {

	size_t buffer_size = 8192;
	DWORD total_bytes = 0;
	DWORD data_bytes_available = 0;
	DWORD data_bytes_readed;
	BOOL result = FALSE;
	
	char *data_buffer = (char*)malloc(buffer_size);
	if(data_buffer == NULL) {
		printf(RED("ERROR while allocating memory in TWLReadData\n"));
		WinHttpCloseHandle(hRequest);
		return NULL;
	}

	memset(data_buffer, '\0', buffer_size);
	char *p = data_buffer;
	
	do {

		result = WinHttpQueryDataAvailable(hRequest, &data_bytes_available);
		if(result == FALSE) {
			int error_id = GetLastError();
			char buffer[512] = {};
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
			OutputDebugString(buffer);		
			OutputDebugString("\n");
			printf(RED("%s\n"), buffer);
		}

		// No need to go further if we don't have ant data to read.
		if(data_bytes_available == 0) break;

		if(total_bytes + data_bytes_available > buffer_size) {
		
			size_t offset = p - data_buffer;

			// Maybe doubling the buffer is not a good idea but i'm
			// confident that the most demanding request will not 
			// have a big response in term of data size.
			// The only thing i can do here it's just test and see.
			buffer_size *= 2;
			char *tmp = (char*)realloc(data_buffer, buffer_size);
			
			if(tmp == NULL) {
				printf(RED("ERROR while reallocating the TWLReadData internal buffer!\n"));
				free(data_buffer);
				return NULL;
			}

			printf(GREEN("Successfully reallocated TWLReadData internal buffer to %zu bytes!\n"), buffer_size);

			data_buffer = tmp;
			p = data_buffer + offset;
			memset(data_buffer + total_bytes, '\0', buffer_size - total_bytes);
		}

		result = WinHttpReadData(hRequest, p, data_bytes_available, &data_bytes_readed);
		if(result == FALSE) {
			int error_id = GetLastError();
			char buffer[512] = {};
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
			OutputDebugString(buffer);		
			OutputDebugString("\n");
			printf(RED("%s\n"), buffer);			
		}

		p += data_bytes_readed;
		total_bytes += data_bytes_readed;
		
#if TWL_DEBUG_READ_DATA
		printf(YELLOW("Total: %lu - Available: %lu - Readed: %lu\n"), total_bytes, data_bytes_available, data_bytes_readed);
#endif

	} while(data_bytes_available > 0);

	WinHttpCloseHandle(hRequest);

#if TWL_DEBUG_READ_DATA
	printf(YELLOW("DATA BUFFER CONTENT:\n"));
	printf(YELLOW("Final buffer size: %zu bytes\nTotal data readed: %lu bytes\n"), buffer_size, total_bytes);
	printf(MAGENTA("%s\n\n"), data_buffer);
#endif

	return data_buffer;
}


void TWLDebugQueryHeaders(const HINTERNET hRequest) {

	WCHAR query_buffer[4096] = {};
	DWORD query_buffer_length = sizeof(query_buffer);
	BOOL result = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, query_buffer, &query_buffer_length, WINHTTP_NO_HEADER_INDEX);
	if(result == FALSE) {
		int error_id = GetLastError();
		char buffer[512] = {};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_id, 0, buffer, sizeof(buffer), 0);
		OutputDebugString(buffer);		
		OutputDebugString("\n");
	}

	wprintf(YELLOW(L"HEADERS BUFFER CONTENT:\n"));
	wprintf(MAGENTA(L"%s"), query_buffer);
}


void TWLFree(void *data) {
	free(data);
}

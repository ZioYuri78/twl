#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "twl_api.h"
#include "twl_parser.h"
#include "VTS_table.h"

#ifdef _DEBUG
#pragma comment(lib, "lib/debug/twl_api_D.lib")
#else
#pragma comment(lib, "lib/release/twl_api.lib")
#endif

#pragma comment(lib, "User32.lib")

int main(int argc, char **argv) {

	if(argc < 2) {
		printf(COL_BF_YELLOW DEC_MODE);
		DEC_LINE_UP(97)
		printf(ASCII_MODE "\n" DEC_MODE DEC_LINE_VERT ASCII_MODE
		"WARNING:                                                                                         "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n"
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"you must call the program with the your twitch username!                                         "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n"
		DEC_MODE DEC_LINE_VERT ASCII_MODE 
		"EXAMPLE: test_client.exe YourTwitchUsername                                                      "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n");
		printf(DEC_MODE);
		DEC_LINE_BOTTOM(97);
		printf(ASCII_MODE COL_DEFAULT "\n");
		return 0;
	}

	// Set up your client id and OAuth token
	// If you don't know what i'm talking about, go here
	// https://dev.twitch.tv/docs/api/get-started
	
	// This user id has user:read:emotes scope
	FTWLUserCredentials api_user_1= {
		.client_id = L"<YOUR CLIENT ID>",
		.oauth2_token = L"<YOUR OAUTH TOKEN>",
	};

	// This user id has user:read:follows scope
	FTWLUserCredentials api_user_2= {
		.client_id = L"<YOUR CLIENT ID>",
		.oauth2_token = L"<YOUR OAUTH TOKEN>",
	};

	printf("\n");

	// Connect to the Twitch API server
	BOOL result = TWLConnect();
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to connect!\n" COL_DEFAULT);
		return 0;
	}
	
	// First call to SetCurrentUser is MANDATORY!!
	result = TWLSetCurrentUser(&api_user_1);

	printf("\n");

	char *user_info_data = NULL;
	result = TWLGetUsers(NULL, argv[1], &user_info_data);
	if(result == FALSE) {
		TWLFree(user_info_data);
		TWLShutdown();
		return 0;
	}

	FTWLUserInfo user_info = {};
	result = TWLPUserInfo(user_info_data, &user_info);

	char *emotes_data = NULL;
	result = TWLGetUserEmotes(user_info.id, NULL, &emotes_data);
	if(result == FALSE) {
		TWLFree(emotes_data);
		TWLShutdown();
		return 0;
	}

	printf(GREEN("\n%s\n"), emotes_data);
	TWLFree(emotes_data);

	// Change user id and token here, now every call refer to this.
	result = TWLSetCurrentUser(&api_user_2);

	char *follows_data = NULL;
	result = TWLGetFollowedStreams(user_info.id, &follows_data);
	if(result == FALSE) {
		TWLFree(follows_data);
		TWLShutdown();
		return 0;
	}

	printf(GREEN("\n%s\n"), follows_data);
	TWLFree(follows_data);

	// Shutdown the API session
	result = TWLShutdown();

	return 0;
}


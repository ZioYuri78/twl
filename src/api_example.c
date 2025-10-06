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
		printf(COL_BF_YELLOW "WARNING: you must call the program with a broadcaster name!\nEXAMPLE: api_example.exe ZioYuri78\n" COL_DEFAULT);
		return 0;
	}

	// Step 1:
	// Set up your client id and OAuth token
	// If you don't know what i'm talking about, go here
	// https://dev.twitch.tv/docs/api/get-started
	FTWLUserCredentials user = {
		.client_id = L"<YOUR CLIENT ID>",
		.oauth2_token = L"<YOUR OAUTH TOKEN>",
	};
	
	// Step 2:
	// connect to Twitch API server
	BOOL result = TWLConnect();
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to connect!\n" COL_DEFAULT);
		return 0;
	}

	// Step 3:
	// set the current user, from here every calls to API functions will
	// refer to this user.
	// Call this function everytime you want to call the API with another user.
	result = TWLSetCurrentUser(&user);

	// Step 4:
	// call your API function, all the TWLGet function will allocate the memory for you.
	char *user_info_data = NULL;
	result = TWLGetUsers(NULL, argv[1], &user_info_data);
	
	if(result == FALSE) {
	 	printf(COL_BF_RED "ERROR: Cannot get %s info data!\n", argv[1]);
		TWLShutdown();
		return 0;
	}

	// Step 5:
	// parse your data, all the TWLP functions that receive a buffer (const char *data) 
	// will free the allocated memory for you.
	FTWLUserInfo user_info = {};
	result = TWLPUserInfo(user_info_data, &user_info);
	if(result == FALSE) {
		// if parsing function fail you must free the allocated memory if you no longer need it.
		TWLFree(user_info_data);
	}


	TWLPrintUserInfo(&user_info);

	// Step 6:
	// Shutdown the connection to the Twitch API server
	result = TWLShutdown();

	return 0;
}

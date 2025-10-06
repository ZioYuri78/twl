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

	if(argc < 3) {
		printf(COL_BF_YELLOW DEC_MODE);
		DEC_LINE_UP(97)
		printf(ASCII_MODE "\n" DEC_MODE DEC_LINE_VERT ASCII_MODE
		"WARNING:                                                                                         "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n"
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"you must call the program with the name of the broadcaster whose chat room you want to read and  "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n"
		DEC_MODE DEC_LINE_VERT ASCII_MODE 
		"your twitch username!                                                                            "
		DEC_MODE DEC_LINE_VERT ASCII_MODE
		"\n"
		DEC_MODE DEC_LINE_VERT ASCII_MODE 
		"EXAMPLE: test_client.exe YourFavoriteStreamer YourTwitchUsername                                 "
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
	//
	// IMPORTANT: Because later we are going to subscribe to a channel.chat.message event
	// you need an oauth token with user:read:chat scope.
	FTWLUserCredentials api_user_credentials = {
		.client_id = L"<YOUR CLIENT ID>",
		.oauth2_token = L"<YOUR OAUTH TOKEN>",
	};

	printf("\n");

	// Connect to the Twitch API server.
	BOOL result = TWLConnect();
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to connect!\n" COL_DEFAULT);
		return 0;
	}

	// Set the user the API will refer to from now.
	result = TWLSetCurrentUser(&api_user_credentials);
	
	printf("\n");

	// Get the broadcaster info you want to read the chat room,
	// every TWLGet functions allocate the memory for you.
	char *broadcaster_info_data = NULL;
	result = TWLGetUsers(NULL, argv[1], &broadcaster_info_data);
	
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to get the broadcaster info data!\n" COL_DEFAULT);
		TWLFree(broadcaster_info_data);
		TWLShutdown();
		return 0;
	}
	
	// Parse the broadcaster info data,
	// every TWLP function free the memory allocated by any TWLGet functions
	FTWLUserInfo broadcaster_info = {};
	result = TWLPUserInfo(broadcaster_info_data, &broadcaster_info);
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to parse the broadcaster info data!\n" COL_DEFAULT);
		TWLFree(broadcaster_info_data);
		TWLShutdown();
		return 0;
	}

	TWLPrintUserInfo(&broadcaster_info);

	printf("\n");
	
	// Get your user info 
	char *your_user_info_data = NULL;
	result = TWLGetUsers(NULL, argv[2], &your_user_info_data);
	
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to get your user info data!\n" COL_DEFAULT);
		TWLFree(your_user_info_data);
		TWLShutdown();
		return 0;
	}
	
	// Parse your user info
	FTWLUserInfo your_user_info = {};
	result = TWLPUserInfo(your_user_info_data, &your_user_info);
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to parse the user info data!\n" COL_DEFAULT);
		TWLFree(your_user_info_data);
		TWLShutdown();
		return 0;
	}

	TWLPrintUserInfo(&your_user_info);


	const char *broadcaster_id = broadcaster_info.id;
	const char *your_user_id = your_user_info.id;

	printf("\n");
	
	// Enstablish the EventSub websocket connection.
	// This struct will contain the websocket
	// connection handlers and the session ID.
	FTWLEventSubSession event_sub_session = {};
	result = TWLEventSub(&event_sub_session);

	// Prepare the event type we want to subscribe to.
	FTWLEventType sub_channel_chat_message = {};
	strcpy(sub_channel_chat_message.type, "channel.chat.message");
	strcpy(sub_channel_chat_message.version, "1");
	sprintf(sub_channel_chat_message.condition, "{\"broadcaster_user_id\":\"%s\",\"user_id\":\"%s\"}", broadcaster_id, your_user_id);
	sprintf(sub_channel_chat_message.transport, "{\"method\":\"websocket\", \"session_id\":\"%s\"}", event_sub_session.session_id);
	
	// Subscribe to the channel.chat.message event.
	// This function allocate the memory.
	char *data = NULL;
	result = TWLCreateEventSubscription(&sub_channel_chat_message, &data);
	
	// Parse the result of the subscription.
	// This function free the allocated memory.
	FTWLEventSub event_sub = {};
	result = TWLPEventSubscription(data, &event_sub);

	if(result) {
		printf(BLUE("%-16s") GREEN("%s\n"), "Event type: ", event_sub.type);
		printf(BLUE("%-16s") GREEN("%s\n"), ("ID: "), event_sub.id);
		printf(BLUE("%-16s") GREEN("%s\n"), ("Status: "), event_sub.status);
		printf(BLUE("%-16s") GREEN("%s\n"), ("Version: "), event_sub.version);
		printf(BLUE("%-16s") GREEN("%s\n"), ("Condition: "), event_sub.condition);
		printf(BLUE("%-16s") GREEN("%s\n"), ("Created at: "), event_sub.created_at);
		printf(BLUE("%-16s") GREEN("%s\n"), ("Transport: "), event_sub.transport);
		printf(BLUE("%-16s") GREEN("%i\n"), ("Cost: "), event_sub.cost);
		printf(BLUE("%-16s") GREEN("%i\n"), ("Total: "), event_sub.total);
		printf(BLUE("%-16s") GREEN("%i\n"), ("Max total cost: "), event_sub.max_total_cost);
		printf(BLUE("%-16s") GREEN("%i\n"), ("Total cost: "), event_sub.total_cost);
	}

	printf("\n");
	
	// Listen to incoming EventSub notifications, press ESC to exit the loop
	GetAsyncKeyState(VK_ESCAPE);	// Flush the key state.
	do {
		// This function sit and wait for the incoming websocket notifications,
		// it allocate the memory, i know is not the best way tomanage this here
		// but i keep it for now.
		char *data_buffer = NULL;
		TWLReadEventData(&event_sub_session, &data_buffer);
		
		printf(COL_BF_MAGENTA "========== INCOMING MESSAGE (PRESS ESC TO EXIT) ==========\n");
		printf(COL_BF_GREEN "%s\n\n", data_buffer);
		
		// Remember to manually free the memory!
		TWLFree(data_buffer);

	} while((0x01 & GetAsyncKeyState(VK_ESCAPE)) == 0x00);

	printf("\n");
	
	// Get a list of all the EventSub subscriptions.
	// (same pattern: allocate and deallocate with TWLP or manually)
	char *subs = NULL;
	result = TWLGetEventSubSubscriptions(NULL, "", NULL, "", &subs);
	
	printf(COL_BF_BLUE "List of subscriptions:\n" COL_BF_GREEN "%s\n", subs);

	TWLFree(subs);

	printf("\n");
	
	// Delete the subscription we made in this session
	printf(YELLOW("Deleting subscription %s\n"), event_sub.id);

	result = TWLDeleteEventSubscription(event_sub.id);
	if(result) {
		printf(GREEN("Subscription successfully deleted!\n"));
	} else {
		printf(RED("ERROR: unable to delete subscriptions %s\n"), "");
	}
	
	// Shutdown the EventSub websocket connection
	result = TWLEventSubShutdown(&event_sub_session);

	printf("\n");
	
	// Shutdown the Twitch API connection.
	result = TWLShutdown();

	return 0;
}

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


void EventSubTest(const char *broadcaster_id);

int main(int argc, char **argv) {

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "     TWL TEST CLIENT     " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");


	if(argc < 2) {
		printf(COL_BF_YELLOW "WARNING: you must call the program with a broadcaster name!\nEXAMPLE: test_client.exe ZioYuri78\n" COL_DEFAULT);
		return 0;
	}

	FTWLUserCredentials user = {
		.client_id = L"<YOUR CLIENT ID>",
		.oauth2_token = L"<YOUR OAUTH TOKEN>",
	};

	
	BOOL result = TWLConnect(&user);
	if(result == FALSE) {
		printf(COL_BF_RED "ERROR: Unable to connect!\n" COL_DEFAULT);
	}
	

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "      GET USER INFO      " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");
	
	char *user_info_data = NULL;
	result = TWLGetUsers(NULL, argv[1], &user_info_data);
	
	if(result) {
	 	
		FTWLUserInfo user_info = {};
		result = TWLPUserInfo(user_info_data, &user_info);
		if(result == FALSE) {
			TWLFree(user_info_data);
		}

		TWLPrintUserInfo(&user_info);

		EventSubTest(user_info.id);
	}


	result = TWLShutdown(&user);

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "           END           " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");

	return 0;
}



void EventSubTest(const char *broadcaster_id) {

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "   CONNECT TO EVENTSUB   " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");

	FTWLEventSubSession event_session = {};
	BOOL result = TWLEventSub(&event_session);

	printf(GREEN("Session ID: %s\n"), event_session.session_id);

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "   SUBSCRIBE TO EVENT    " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");

	FTWLEventType sub_channel_chat_message = {};

	strcpy(sub_channel_chat_message.type, "channel.chat.message");
	strcpy(sub_channel_chat_message.version, "1");
	sprintf(sub_channel_chat_message.condition, "{\"broadcaster_user_id\":\"%s\",\"user_id\":\"%s\"}", broadcaster_id, "28352553");
	sprintf(sub_channel_chat_message.transport, "{\"method\":\"websocket\", \"session_id\":\"%s\"}", event_session.session_id);
	
	char *data = NULL;
	result = TWLCreateEventSubscription(&sub_channel_chat_message, &data);
	
	FTWLEventSub event_sub = {};
	result = TWLPEventSubscription(data, &event_sub);

	if(result) {
		printf(GREEN("Subscribed to %s event.\n"), event_sub.type);
		printf(GREEN("ID: %s\n"), event_sub.id);
		printf(GREEN("Status: %s\n"), event_sub.status);
		printf(GREEN("Version: %s\n"), event_sub.version);
		printf(GREEN("Condition: %s\n"), event_sub.condition);
		printf(GREEN("Created at: %s\n"), event_sub.created_at);
		printf(GREEN("Transport: %s\n"), event_sub.transport);
		printf(GREEN("Cost: %i\n"), event_sub.cost);
		printf(GREEN("Total: %i\n"), event_sub.total);
		printf(GREEN("Max total cost: %i\n"), event_sub.max_total_cost);
		printf(GREEN("Total cost: %i\n"), event_sub.total_cost);
	}

	
	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "  ENTERING THE MAIN LOOP " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");
	
	GetAsyncKeyState(VK_ESCAPE);	// Flush the key state.
	do {
		char *data_buffer = NULL;
		TWLReadEventData(&event_session, &data_buffer);
		printf(MAGENTA("===== INCOMING MESSAGE (PRESS ESC TO EXIT) =====\n"));
		printf(GREEN("%s\n\n"), data_buffer);
		TWLFree(data_buffer);
	} while((0x01 & GetAsyncKeyState(VK_ESCAPE)) == 0x00);


	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "     LIST EVENT SUBS     " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");
	
	char *subs = NULL;
	result = TWLGetEventSubSubscriptions(NULL, "", NULL, "", &subs);
	
	printf(GREEN("Subs: %s\n"), subs);
	TWLFree(subs);

	printf(COL_BF_CYAN DEC_MODE "\n");
	DEC_LINE_UP(25)
	printf("\n" DEC_LINE_VERT ASCII_MODE "    DELETE EVENT SUBS    " DEC_MODE DEC_LINE_VERT "\n");
	DEC_LINE_BOTTOM(25)
	printf(ASCII_MODE COL_DEFAULT "\n");

	printf(YELLOW("Deleting subscription %s\n"), event_sub.id);

	result = TWLDeleteEventSubscription(event_sub.id);
	if(result) {
		printf(GREEN("Subscription successfully deleted!\n"));
	} else {
		printf(RED("ERROR: unable to delete subscriptions %s\n"), "");
	}
	

	result = TWLEventSubShutdown(&event_session);
}

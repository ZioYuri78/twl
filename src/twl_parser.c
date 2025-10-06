#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define TWL_PARSER_EXPORTS
#include "twl_parser.h"
#include "VTS_table.h"

#ifdef _DEBUG
#define TWLP_DEBUG_GET_VALUE 0
#define TWLP_DEBUG_GET_VALUES 0
#endif

// #TODO template for copy paste, remove when finished
typedef struct FTWL {char c;} FTWL; 
BOOL TWLP(const char *data, FTWL *s) {

	const char *p = data;

	free((void*)data);
	return TRUE;
}
// ========================= //
//            ADS            //
// ========================= //

BOOL TWLPGetAdSchedule(const char *data, FTWLAdSchedule *ad_schedule) {

	const char *p =  data;

	// {"data":[{"snooze_count":
	p += 25;
	char tmp[16] = {};
	TWLPGetValue(&p, tmp);
	ad_schedule->snooze_count = atoi(tmp);

	// "snooze_refresh_at":
	p += 21;
	TWLPGetValue(&p, ad_schedule->snooze_refresh_at);

	// "next_ad_at":
	p += 14;
	TWLPGetValue(&p, ad_schedule->next_ad_at);

	// "duration":
	p += 12;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	ad_schedule->duration = atoi(tmp);

	// "last_ad_at":
	p += 14;
	TWLPGetValue(&p, ad_schedule->last_ad_at);

	// "preroll_free_time":
	p += 21;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	ad_schedule->preroll_free_time = atoi(tmp);

	free((void*)data);
	return TRUE;
}


// =============================== //
//            ANALYTICS            //
// =============================== //
	
BOOL TWLPGetExtensionAnalytics(const char *data, FTWLExtensionAnalytics *reports) {

	const char *p = data;

	// {"data":[{"extension_id":
	p += 25;
	TWLPGetValue(&p, reports->extension_id);

	// "URL":
	p += 7;
	TWLPGetValue(&p, reports->url);

	// "type":
	p += 8;
	TWLPGetValue(&p, reports->type);

	// "started_at":
	p += 14;
	TWLPGetValue(&p, reports->started_at);

	// "ended_at":
	p += 12;
	TWLPGetValue(&p, reports->ended_at);

	free((void*)data);
	return TRUE;
}


BOOL TWLPGetGameAnalytics(const char *data, FTWLGameAnalytics *reports) {

	const char *p = data;

	// {"data":[{"game_id":
	p += 25;
	TWLPGetValue(&p, reports->game_id);

	// "URL":
	p += 7;
	TWLPGetValue(&p, reports->url);

	// "type":
	p += 8;
	TWLPGetValue(&p, reports->type);

	// "started_at":
	p += 14;
	TWLPGetValue(&p, reports->started_at);

	// "ended_at":
	p += 12;
	TWLPGetValue(&p, reports->ended_at);

	free((void*)data);
	return TRUE;
}

// ========================== //
//            BITS            //
// ========================== //

BOOL TWLPGetBitsLeaderboard(const char *data, FTWLBitsLeaderboard *leaderboard) {

	const char *p = data;

	// we land on [, so we can better manage the { in the loop
	// {"data":[
	p += 8;
	
	// run until ]
	int i = 0;
	while(*p != ']') {
		// {"user_id":
		p += 12;
		TWLPGetValue(&p, leaderboard->leaders[i].user_id);
		// "user_login":
		p += 14;
		TWLPGetValue(&p, leaderboard->leaders[i].user_login);
		// "user_name":
		p += 13;
		TWLPGetValue(&p, leaderboard->leaders[i].user_name);
		// "rank":
		p += 8;
		char tmp[8] = {};
		TWLPGetValue(&p, tmp);
		leaderboard->leaders[i].rank = atoi(tmp);
		// "score":
		p += 9;
		memset(tmp, '\0', sizeof(tmp));
		leaderboard->leaders[i].score = atoi(tmp);
		// }, or }]
		p += 2;
		i++;
	}

	// ,"date_range":{"started_at":
	p += 29;
	TWLPGetValue(&p, leaderboard->started_at);

	// "ended_at":
	p += 12;
	TWLPGetValue(&p, leaderboard->ended_at);

	// ,"total":
	p += 10;
	char tmp[8] = {};
	TWLPGetValue(&p, tmp);
	leaderboard->total = atoi(tmp);

	free((void*)data);
	return TRUE;
}


BOOL TWLPGetCheermotes(const char *data, FTWLCheermoteSet *cheermotes) {
	
	const char *p = data;

	free((void*)data);
	return TRUE;
}

// ============================== //
//            CHANNELS            //
// ============================== //
// #TODO fix CCLs
BOOL TWLPChannelInfo(const char *data, FTWLChannelInfo *channel_info) {
	
	const char *p = data;
	// skip the whole {"data":[{"broadcaster_id": part
	p += 27;
	TWLPGetValue(&p, channel_info->broadcaster_id);

	p += 21;
	TWLPGetValue(&p, channel_info->broadcaster_login);

	p += 20;
	TWLPGetValue(&p, channel_info->broadcaster_name);

	p += 24;
	TWLPGetValue(&p, channel_info->broadcaster_language);

	p += 11;
	TWLPGetValue(&p, channel_info->game_id);
	
	p += 13;
	TWLPGetValue(&p, channel_info->game_name);
	
	p += 9;
	TWLPGetValue(&p, channel_info->title);
	
	p += 9;
	char tmp[1024] = {};
	TWLPGetValue(&p, tmp);
	channel_info->delay = atoi(tmp);
	
	p += 8;
	TWLPGetValues(&p, channel_info->tags);
	
	p += 33;
	//TWLPGetValues(&p, channel_info->ccl);
	
	p += 22;
	memset(tmp, 0x00, sizeof(tmp));
	TWLPGetValue(&p, tmp);
	channel_info->is_branded_content = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;
	
	free((void*)data);
	return TRUE;
}

// #TODO This get only the first one editor
BOOL TWLPChannelEditors(const char *data, FTWLChannelEditor *channel_editors) {
	
	const char *p = data;

	// Skip {"data":[
	p += 9;
	
	// See if the list is empty
	if(*p == ']') {
		printf(YELLOW("WARNING: There are no editors for this channel!\n"));
		return FALSE;
	}

	p +=11;
	TWLPGetValue(&p, channel_editors->user_id);

	p +=13;
	TWLPGetValue(&p, channel_editors->user_name);

	p += 14;
	TWLPGetValue(&p, channel_editors->created_at);

	free((void*)data);
	return TRUE;
}

// #TODO this get only the first channel
BOOL TWLPFollowedChannels(const char *data, FTWLFollowedChannel *followed_channels) {

	const char *p = data;

	// skip the whole {"total":
	p += 9;
	char tmp[64] = {};
	TWLPGetValue(&p, tmp);
	int total = atoi(tmp);

	if(total == 0) {
		printf(YELLOW("WARNING: There are no followed channels!\n"));
		return FALSE;
	}

	printf(GREEN("Followed channels: %i\n"), total);

	if(*(p+9) == ']') {
		printf(YELLOW("WARNING: The user doesn't follow this channel!\n"));
		return FALSE;
	}

	// skip the whole "data":[{"broadcaster_id":
	p += 27;
	TWLPGetValue(&p, followed_channels->broadcaster_id);

	p += 21;
	TWLPGetValue(&p, followed_channels->broadcaster_login);
	
	p += 20;
	TWLPGetValue(&p, followed_channels->broadcaster_name);
	
	p += 15;
	TWLPGetValue(&p, followed_channels->followed_at);

	free((void*)data);
	return TRUE;
}

// #TODO this get only the first follower
BOOL TWLPChannelFollowers(const char *data, FTWLChannelFollower *followers) {
	
	const char *p = data;

	// skip the whole {"total":
	p += 9;
	char tmp[64] = {};
	TWLPGetValue(&p, tmp);
	int total = atoi(tmp);

	if(total == 0) {
		printf(YELLOW("WARNING: There are no followers!\n"));
		return FALSE;
	}

	printf(GREEN("Followers: %i\n"), total);

	if(*(p+9) == ']') {
		printf(YELLOW("WARNING: The user doesn't follow this channel!\n"));
		return FALSE;
	}
	
	// skip the whole "data":[{"user_id":
	p += 20;
	TWLPGetValue(&p, followers->user_id);

	p += 14;
	TWLPGetValue(&p, followers->user_login);
	
	p += 14;
	TWLPGetValue(&p, followers->user_name);
	
	p += 15;
	TWLPGetValue(&p, followers->followed_at);

	free((void*)data);
	return TRUE;
}


// ========================== //
//            CHAT            //
// ========================== //
// #TODO this get only the first chatter
BOOL TWLPChatters(const char *data, FTWLChatter *chatters) {

	if(strcmp(data, "{\"data\":[],\"pagination\":{},\"total\":0}") == 0) {
		printf(YELLOW("WARNING: there aren't any chatters!\n"));
		return FALSE;
	}

	const char *p = data;

	// skip {"data":[{"user_id":
	p += 20;
	TWLPGetValue(&p, chatters->user_id);

	p += 14;
	TWLPGetValue(&p, chatters->user_login);

	p += 13;
	TWLPGetValue(&p, chatters->user_name);

	free((void*)data);
	return TRUE;
}


// #TODO this get only the first emote
BOOL TWLPChannelEmotes(const char *data, FTWLEmote *emotes) {
	
	if(strcmp(data, "{\"data\":[],\"template\":\"https://static-cdn.jtvnw.net/emoticons/v2/{{id}}/{{format}}/{{theme_mode}}/{{scale}}\"}") == 0) {
		printf(YELLOW("WARNING: this channel doesn't have custom emotes!\n"));
		return FALSE;
	}

	const char *p = data;

	// slip {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, emotes->id);
	
	p += 8;
	TWLPGetValue(&p, emotes->name);

	p += 20;
	TWLPGetValue(&p, emotes->images[0]);

	p += 10;
	TWLPGetValue(&p, emotes->images[1]);

	p += 10;
	TWLPGetValue(&p, emotes->images[2]);

	p += 9;
	TWLPGetValue(&p, emotes->tier);

	p += 14;
	TWLPGetValue(&p, emotes->emote_type);

	p += 16;
	TWLPGetValue(&p, emotes->emote_set_id);
	
	p += 10;
	TWLPGetValues(&p, emotes->format);

	p += 9;
	TWLPGetValues(&p, emotes->scale);

	p += 14;
	TWLPGetValues(&p, emotes->theme_mode);

	free((void*)data);
	return TRUE;
}


// #TODO this get only the first emote
BOOL TWLPGlobalEmotes(const char *data, FTWLEmote *emotes) {
	
	const char *p = data;

	// slip {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, emotes->id);
	
	p += 8;
	TWLPGetValue(&p, emotes->name);

	p += 20;
	TWLPGetValue(&p, emotes->images[0]);

	p += 10;
	TWLPGetValue(&p, emotes->images[1]);

	p += 10;
	TWLPGetValue(&p, emotes->images[2]);
	
	p += 11;
	TWLPGetValues(&p, emotes->format);

	p += 9;
	TWLPGetValues(&p, emotes->scale);

	p += 14;
	TWLPGetValues(&p, emotes->theme_mode);

	free((void*)data);
	return TRUE;
}


// #TODO this get only the first emote set
BOOL TWLPEmoteSets(const char *data, FTWLEmoteSet *emote_sets) {

	if(strcmp(data, "{\"data\":[],\"template\":\"https://static-cdn.jtvnw.net/emoticons/v2/{{id}}/{{format}}/{{theme_mode}}/{{scale}}\"}") == 0) {
		printf(YELLOW("WARNING: this channel doesn't have emote sets!\n"));
		return FALSE;
	}

	const char *p = data;

	// slip {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, emote_sets->id);
	
	p += 8;
	TWLPGetValue(&p, emote_sets->name);

	p += 20;
	TWLPGetValue(&p, emote_sets->images[0]);

	p += 10;
	TWLPGetValue(&p, emote_sets->images[1]);

	p += 10;
	TWLPGetValue(&p, emote_sets->images[2]);

	p += 15;
	TWLPGetValue(&p, emote_sets->emote_type);

	p += 16;
	TWLPGetValue(&p, emote_sets->emote_set_id);
	
	p += 12;
	TWLPGetValue(&p, emote_sets->owner_id);
	
	p += 10;
	TWLPGetValues(&p, emote_sets->format);

	p += 9;
	TWLPGetValues(&p, emote_sets->scale);

	p += 14;
	TWLPGetValues(&p, emote_sets->theme_mode);
	
	free((void*)data);
	return TRUE;
}


// #TODO this get only the first badges
BOOL TWLPChannelChatBadges(const char *data, FTWLBadge *badges) {
	
	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: This channel doesn't have any badges!\n"));
		return FALSE;
	}

	const char *p = data;

	// Skip {"data":[{"set_id":
	p += 19;
	TWLPGetValue(&p, badges->set_id);
	
	p += 19;
	TWLPGetValue(&p, badges->versions[0]);
	
	p += 16;
	TWLPGetValue(&p, badges->versions[1]);
	
	p += 16;
	TWLPGetValue(&p, badges->versions[2]);
	
	p += 16;
	TWLPGetValue(&p, badges->versions[3]);
	
	p += 9;
	TWLPGetValue(&p, badges->versions[4]);
	
	p += 15;
	TWLPGetValue(&p, badges->versions[5]);
	
	p += 16;
	TWLPGetValue(&p, badges->versions[6]);
	
	p += 13;
	TWLPGetValue(&p, badges->versions[7]);

	free((void*)data);
	return TRUE;
}


// #TODO this get only the first badges
BOOL TWLPGlobalChatBadges(const char *data, FTWLBadge *badges) {
	return TWLPChannelChatBadges(data, badges);
}


BOOL TWLPChatSettings(const char *data, FTWLChatSettings *chat_settings) {

	// I don't think this will ever happen
	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: No chat settings!\n"));
		return FALSE;
	}

	const char *p = data;

	// skip {"data":[{"broadcaster_id":
	p += 27;
	TWLPGetValue(&p, chat_settings->broadcaster_id);

	char tmp[8] = {};
	p += 13;
	TWLPGetValue(&p, tmp);
	chat_settings->is_slow_mode = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	p += 23;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->slow_mode_wait_time = atoi(tmp);

	p += 17;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->is_follower_mode = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	p += 26;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->follower_mode_duration = atoi(tmp);

	p += 19;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->is_subscriber_mode = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	p += 14;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->is_emote_mode = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	p += 20;
	memset(tmp, '\0', sizeof(tmp));
	TWLPGetValue(&p, tmp);
	chat_settings->is_unique_chat_mode = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	if(*p == ',') {

		p += 27;
		memset(tmp, '\0', sizeof(tmp));
		TWLPGetValue(&p, tmp);
		chat_settings->is_non_moderator_chat_delay = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

		p += 27;
		memset(tmp, '\0', sizeof(tmp));
		TWLPGetValue(&p, tmp);
		chat_settings->non_moderator_chat_delay_duration = atoi(tmp);
	}

	free((void*)data);
	return TRUE;
}


BOOL TWLPSharedChatSession(const char *data, FTWLChatSession *chat_session) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Can't find a shared chat session!\n"));
		return FALSE;
	}
/*
   {"data":[{
   "session_id":"cb2250e6-3916-4b8a-b3f4-c5b03627b53c",
   "host_broadcaster_id":"36769016",
   "participants":[{"broadcaster_id":"36769016"},{"broadcaster_id":"21130533"},{"broadcaster_id":"514775924"},{"broadcaster_id":"43683025"},{"broadcaster_id":"192666468"}],
   "created_at":"2025-07-11T17:33:11.932496276Z",
   "updated_at":"2025-07-11T18:10:31.121655355Z"}]}
*/
	const char *p = data;

	// Skip {"data":[{"session_id":
	p += 23;
	TWLPGetValue(&p, chat_session->session_id);

	p += 23;
	TWLPGetValue(&p, chat_session->host_broadcaster_id);

	// #TODO write a GetValues function that can works with arrays of {name:value},{..} pairs
	p += 35;
	size_t i=0;
	while(*p != ']') {
		TWLPGetValue(&p, chat_session->participants[i]);
		if(*(p+1) == ']') break;
		i++;
		p += 20;
	}

	p += 16;
	TWLPGetValue(&p, chat_session->created_at);

	p += 14;
	TWLPGetValue(&p, chat_session->updated_at);

	free((void*)data);
	return TRUE;
}

// #TODO this return only the first emote
BOOL TWLPUserEmotes(const char *data, FTWLUserEmote *emotes) {
/*
{"data":[{"id":"302303581",
"name":"PrideDragon",
"emote_type":"limitedtime",
"emote_set_id":"",
"owner_id":"537206155",
"format":["static"],
"scale":["1.0","2.0","3.0"],
"theme_mode":["light","dark"]}
*/
	const char *p = data;

	// Skip {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, emotes->id);

	p += 8;
	TWLPGetValue(&p, emotes->name);

	p += 14;
	TWLPGetValue(&p, emotes->emote_type);

	p += 16;
	TWLPGetValue(&p, emotes->emote_set_id);

	p += 12;
	TWLPGetValue(&p, emotes->owner_id);

	p += 10;
	TWLPGetValues(&p, emotes->format);

	p += 9;
	TWLPGetValues(&p, emotes->scale);

	p += 14;
	TWLPGetValues(&p, emotes->theme_mode);

	free((void*)data);
	return TRUE;
}

// #TODO this return only one user
BOOL TWLPUserChatColor(const char *data, FTWLUserChatColor *chat_color) {
	
	const char *p = data;

	// {"data":[{"user_id":
	p += 20;
	TWLPGetValue(&p, chat_color->user_id);

	// "user_login":
	p += 14;
	TWLPGetValue(&p, chat_color->user_login);

	// "user_name":
	p += 13;
	TWLPGetValue(&p, chat_color->user_name);

	// "color":
	p += 9;
	TWLPGetValue(&p, chat_color->color);

	free((void*)data);
	return TRUE;
}


// =========================== //
//            CLIPS            //
// =========================== //
// #TODO this return only one clip
BOOL TWLPClips(const char *data, const BOOL is_just_created, FTWLStreamClip *clips) {

	const char *p = data;

	if(is_just_created == TRUE) {

		// {"data":[{"id":
		p += 15;
		TWLPGetValue(&p, clips->clip_id);

		// "edit_url":
		p += 12;
		TWLPGetValue(&p, clips->edit_url);

	} else {

		if(strcmp(data, "{\"data\":[],\"pagination\":{}}") == 0) {
			printf(YELLOW("WARNING: Cannot find any clips!\n"));
			return FALSE;
		}

		// {"data":[{"id":
		p += 15;
		TWLPGetValue(&p, clips->clip_id);

		// "url":
		p += 7;
		TWLPGetValue(&p, clips->clip_url);

		// "embed_url":
		p += 13;
		TWLPGetValue(&p, clips->embed_url);

		// "broadcaster_id":
		p += 18;
		TWLPGetValue(&p, clips->broadcaster_id);

		// "broadcaster_name":
		p += 20;
		TWLPGetValue(&p, clips->broadcaster_name);

		// "creator_id":
		p += 14;
		TWLPGetValue(&p, clips->creator_id);

		// "creator_name":
		p += 16;
		TWLPGetValue(&p, clips->creator_name);

		// "video_id":
		p += 12;
		TWLPGetValue(&p, clips->video_id);

		// "game_id":
		p += 11;
		TWLPGetValue(&p, clips->game_id);

		// "language":
		p += 12;
		TWLPGetValue(&p, clips->language);

		// "title":
		p += 9;
		TWLPGetValue(&p, clips->title);

		// "view_count":
		char tmp[8] = {};
		p += 14;
		TWLPGetValue(&p, tmp);
		clips->view_count = atoi(tmp);

		// "created_at":
		p += 14;
		TWLPGetValue(&p, clips->created_at);

		// "thumbnail_url":
		p += 17;
		TWLPGetValue(&p, clips->thumbnail_url);

		// "duration":
		memset(tmp, '\0', sizeof(tmp));
		p += 12;
		TWLPGetValue(&p, tmp);
		clips->duration = (float)atof(tmp);

		// "vod_offset":
		memset(tmp, '\0', sizeof(tmp));
		p += 14;
		TWLPGetValue(&p, tmp);
		clips->vod_offset = atoi(tmp);

		// "is_featured":
		memset(tmp, '\0', sizeof(tmp));
		p += 15;
		TWLPGetValue(&p, tmp);
		clips->is_featured = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;
	}

	free((void*)data);
	return TRUE;
}


// ========================== //
//            CCLs            //
// ========================== //
// #TODO this return only one ccls
BOOL TWLPContentClassificationLabels(const char *data, FTWLCCLabel *ccls) {
	
	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Cannot find the stream key!\n"));
		return FALSE;
	}

	const char *p = data;
	
	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, ccls->id);

	// "description":
	p += 15;
	TWLPGetValue(&p, ccls->description);

	// "name":
	p += 8;
	TWLPGetValue(&p, ccls->name);

	free((void*)data);
	return TRUE;
}
// ========================== //

// ============================== //
//            EVENTSUB            //
// ============================== //

BOOL TWLPEventSubscription(const char *data, FTWLEventSub *event_sub) {
	const char *p = data;
	
	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, event_sub->id);
	
	// "status":
	p += 10;
	TWLPGetValue(&p, event_sub->status);

	// "type":
	p += 8;
	TWLPGetValue(&p, event_sub->type);

	// "version":
	p += 11;
	TWLPGetValue(&p, event_sub->version);

	// "condition":{
	p += 13;
	size_t i = 0;
	while(*p != '}') {
		event_sub->condition[i] =  *p;
		p++;
		i++;
	}
	event_sub->condition[i] =  *p;
	p++;

	// "created_at":
	p += 14;
	TWLPGetValue(&p, event_sub->created_at);

	// "transport":{
	p += 13;
	i = 0;
	while(*p != '}') {
		event_sub->transport[i] =  *p;
		p++;
		i++;
	}
	event_sub->transport[i] =  *p;
	p++;

	char tmp[8] = {};
	// "cost":
	p += 8;
	TWLPGetValue(&p, tmp);
	event_sub->cost = atoi(tmp);

	// }]
	p += 2;

	memset(tmp, '\0', sizeof(tmp));
	// "total":
	p += 9;
	TWLPGetValue(&p, tmp);
	event_sub->total = atoi(tmp);

	memset(tmp, '\0', sizeof(tmp));
	// "max_total_cost":
	p += 18;
	TWLPGetValue(&p, tmp);
	event_sub->max_total_cost = atoi(tmp);

	memset(tmp, '\0', sizeof(tmp));
	// "total_cost":
	p += 14;
	TWLPGetValue(&p, tmp);
	event_sub->total_cost = atoi(tmp);
	
	free((void*)data);

	return TRUE;
}
// ============================== //

// =========================== //
//            GAMES            //
// =========================== //

BOOL TWLPGames(const char *data, FTWLGame *top_games) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Cannot find any game!\n"));
		return FALSE;
	}

	const char *p = data;

	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, top_games->game_id);

	// "name":
	p += 8;
	TWLPGetValue(&p, top_games->name);

	// "box_art_url":
	p += 15;
	TWLPGetValue(&p, top_games->box_art_url);

	// "igdb_id":
	p += 11;
	TWLPGetValue(&p, top_games->igdb_id);

	free((void*)data);
	return TRUE;
}


// =========================== //
//            GOALS            //
// =========================== //

BOOL TWLPGoals(const char *data, FTWLGoal *goals) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Cannot find any goals!\n"));
		return FALSE;
	}

	const char *p = data;

	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, goals->goal_id);

	// "broadcaster_id":
	p += 18;
	TWLPGetValue(&p, goals->broadcaster_id);

	// "broadcaster_name":
	p += 20;
	TWLPGetValue(&p, goals->broadcaster_name);

	// "broadcaster_login":
	p += 21;
	TWLPGetValue(&p, goals->broadcaster_login);

	// "type":
	p += 8;
	TWLPGetValue(&p, goals->type);

	// "description":
	p += 15;
	TWLPGetValue(&p, goals->description);

	// "current_amount":
	char tmp[8] = {};
	p += 18;
	TWLPGetValue(&p, tmp);
	goals->current_amount = atoi(tmp);

	// "target_amount":
	memset(tmp, '\0', sizeof(tmp));
	p += 17;
	TWLPGetValue(&p, tmp);
	goals->target_amount = atoi(tmp);

	// "created_at":
	p += 14;
	TWLPGetValue(&p, goals->created_at);

	free((void*)data);
	return TRUE;
}


// ================================ //
//            HYPE TRAIN            //
// ================================ //
// #TODO test when i will able to do hype tains and manage the arrays, right now it returns
// only one event and there is a high probability that it breaks on top_contributions
BOOL TWLPHypeTrainEvents(const char *data, FTWLHypeTrainEvent *hype_train_events) {
	
	if(strcmp(data, "{\"data\":[],\"pagination\":{}}") == 0) {
		printf(YELLOW("WARNING: Cannot find any Hype Train Events!\n"));
		return FALSE;
	}

	const char *p = data;

	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, hype_train_events->event_id);

	// "event_type":
	p += 14;
	TWLPGetValue(&p, hype_train_events->event_type);

	// "event_timestamp":
	p += 19;
	TWLPGetValue(&p, hype_train_events->event_timestamp);

	// "version":
	p += 11;
	TWLPGetValue(&p, hype_train_events->version);

	// "event_data":{"broadcaster_id":
	p += 32;
	TWLPGetValue(&p, hype_train_events->broadcaster_id);

	// "cooldown_end_time":
	p += 21;
	TWLPGetValue(&p, hype_train_events->cooldown_end_time);

	// "expires_at":
	p += 14;
	TWLPGetValue(&p, hype_train_events->expires_at);

	// "goal":
	char tmp[8];
	p += 8;
	TWLPGetValue(&p, tmp);
	hype_train_events->goal = atoi(tmp);

	// "id":
	p += 6;
	TWLPGetValue(&p, hype_train_events->hype_train_id);

	// "last_contribution":{"total":
	memset(tmp, '\0', sizeof(tmp));
	p += 30;
	TWLPGetValue(&p, tmp);
	hype_train_events->last_contribution.total = atoi(tmp);

	// "type":
	p += 8;
	TWLPGetValue(&p, hype_train_events->last_contribution.type);

	// "user":"134247454"},
	p += 8;
	TWLPGetValue(&p, hype_train_events->last_contribution.user);

	// "level":
	memset(tmp, '\0', sizeof(tmp));
	// the field above ends on a } not on a , so we add one
	p += 10;
	TWLPGetValue(&p, tmp);
	hype_train_events->level = atoi(tmp);

	// "started_at":
	p += 14;
	TWLPGetValue(&p, hype_train_events->started_at);

	// #TODO array three items, iterate with a for loop untile we reach ]
	// "top_contributions":[{"total":
	memset(tmp, '\0', sizeof(tmp));
	p += 31;
	TWLPGetValue(&p, tmp);
	hype_train_events->top_contributions[0].total = atoi(tmp);

	// "type":
	p += 8;
	TWLPGetValue(&p, hype_train_events->top_contributions[0].type);

	// "user":247449"}],
	p += 8;
	TWLPGetValue(&p, hype_train_events->top_contributions[0].user);

	// "total":
	memset(tmp, '\0', sizeof(tmp));
	p += 9;
	TWLPGetValue(&p, tmp);
	hype_train_events->total = atoi(tmp);

	free((void*)data);
	return TRUE;
}


// ============================ //
//            SEARCH            //
// ============================ //

BOOL TWLPSearchCategories(const char *data, FTWLGame *matches) {

	if(strcmp(data, "{\"data\":[],\"pagination\":{}}") == 0) {
		printf(YELLOW("WARNING: Cannot find any matches!\n"));
		return FALSE;
	}

	const char *p = data;

	// {"data":[{"box_art_url":
	p += 24;
	TWLPGetValue(&p, matches->box_art_url);
	
	// "id":
	p += 6;
	TWLPGetValue(&p, matches->game_id);
	
	// "name":
	p += 8;
	TWLPGetValue(&p, matches->name);
	
	
	free((void*)data);
	return TRUE;
}


BOOL TWLPSearchChannels(const char *data, FTWLChannelInfo *matches) {
	
	if(strcmp(data, "{\"data\":[],\"pagination\":{}}") == 0) {
		printf(YELLOW("WARNING: Cannot find any matches!\n"));
		return FALSE;
	}
	
	const char *p = data;
	
	// {"data":[{"broadcaster_language":
	p += 33;
	TWLPGetValue(&p, matches->broadcaster_language);

	// "broadcaster_login":
	p += 21;
	TWLPGetValue(&p, matches->broadcaster_login);

	// "display_name":
	p += 16;
	TWLPGetValue(&p, matches->broadcaster_name);

	// "game_id":
	p += 11;
	TWLPGetValue(&p, matches->game_id);

	// "game_name":
	p += 13;
	TWLPGetValue(&p, matches->game_name);

	// "id":
	p += 6;
	TWLPGetValue(&p, matches->broadcaster_id);

	// "is_live":
	char tmp[8] = {};
	p += 11;
	TWLPGetValue(&p, tmp);
	matches->is_live = (strcmp(tmp, "true") == 0) ? TRUE : FALSE;

	// "tag_ids":[]
	p += 13;
	// DEPRECATED
	
	// "tags":[
	p += 8;
	TWLPGetValues(&p, matches->tags);
	
	// "thumbnail_url":
	p += 17;
	TWLPGetValue(&p, matches->thumbnail_url);

	// "title":
	p += 9;
	TWLPGetValue(&p, matches->title);

	// "started_at":
	p += 14;
	TWLPGetValue(&p, matches->broadcaster_language);


	free((void*)data);
	return TRUE;
}

// ============================= //
//            STREAMS            //
// ============================= //

BOOL TWLPStreamKey(const char *data, FTWLStreamKey *stream_key) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Cannot find the stream key!\n"));
		return FALSE;
	}

	const char *p = data;

	p += 23;
	TWLPGetValue(&p, stream_key->strem_key);

	free((void*)data);
	return TRUE;
}

// #TODO this return only one stream
BOOL TWLPStreams(const char *data, FTWLStream *streams) {

	// Maybe Twitch is down or something, in this case i suppose the list will be empty
	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: Cannot find any stream!\n"));
		return FALSE;
	}

	const char *p = data;

	// {"data":[{"id":
	p += 15;
	TWLPGetValue(&p, streams->id);
	
	// "user_id":
	p += 11;
	TWLPGetValue(&p, streams->user_id);

	// "user_login":
	p += 14;
	TWLPGetValue(&p, streams->user_login);

	// "user_name":
	p += 14;
	TWLPGetValue(&p, streams->user_name);

	// "game_id":
	p += 11;
	TWLPGetValue(&p, streams->game_id);

	// "game_name":
	p += 13;
	TWLPGetValue(&p, streams->game_name);

	// "type":
	p += 8;
	TWLPGetValue(&p, streams->type);

	// "title":
	p += 9;
	TWLPGetValue(&p, streams->title);

	// "viewer_count":
	char tmp[8] = {};
	p += 16;
	TWLPGetValue(&p, tmp);
	streams->viever_count = atoi(tmp);

	// "started_at":
	p += 14;
	TWLPGetValue(&p, streams->started_at);

	// "language":
	p += 12;
	TWLPGetValue(&p, streams->language);

	// "thumbnail_url":
	p += 17;
	TWLPGetValue(&p, streams->thumbnail_url);

	// "tag_ids":[],
	p += 13;
	//DEPRECATED

	// "tags":[
	p += 8;
	TWLPGetValues(&p, streams->tags);

	// "is_mature":
	memset(tmp, '\0', sizeof(tmp));
	p += 13;
	TWLPGetValue(&p, tmp);
	streams->is_mature = (strcmp(tmp, "true") == 0) ?  TRUE : FALSE;

	free((void*)data);
	return TRUE;
}


BOOL TWLPFollowedStreams(const char *data, FTWLStream *followed_streams) {
	return TWLPStreams(data, followed_streams);
}

// #TODO This get only the first one/
BOOL TWLPStreamMarkers(const char *data, const BOOL is_just_created, FTWLStreamMarker *stream_markers) {
	
	if(strcmp(data, "{\"data\":[],\"pagination\":{}}") == 0) {
		printf(YELLOW("WARNING: Cannot find any stream markers!\n"));
		return FALSE;
	}

	const char *p = data;

	if(is_just_created == TRUE) {
	
		// {"data":[{"id":
		p += 15;
		TWLPGetValue(&p, stream_markers->marker_id);

		// "created_at":
		p += 14;
		TWLPGetValue(&p, stream_markers->created_at);

		// "position_seconds":
		p += 20;
		TWLPGetValue(&p, stream_markers->position_seconds);

		// "description":
		p += 15;
		TWLPGetValue(&p, stream_markers->description);

	} else {

		// {"data":[{"user_id":
		p += 20;
		TWLPGetValue(&p, stream_markers->user_id);
		
		// "user_name":
		p += 13;
		TWLPGetValue(&p, stream_markers->user_name);

		// "user_login":
		p += 14;
		TWLPGetValue(&p, stream_markers->user_login);
		
		// "videos":[{"video_id":
		p += 23;
		TWLPGetValue(&p, stream_markers->video_id);
		
		// "markers":[{"id":
		p += 18;
		TWLPGetValue(&p, stream_markers->marker_id);
		
		// "created_at":
		p += 14;
		TWLPGetValue(&p, stream_markers->created_at);
		
		// "description":
		p += 15;
		TWLPGetValue(&p, stream_markers->description);
		
		// "position_seconds":
		p += 20;
		TWLPGetValue(&p, stream_markers->position_seconds);
		
		// "URL":
		p += 7;
		TWLPGetValue(&p, stream_markers->url);
	}

	free((void*)data);
	return TRUE;
}


// =================================== //
//            SUBSCRIPTIONS            //
// =================================== //

BOOL TWLPBroadcasterSubscriptions(const char *data, FTWLSubscription *subscriptions) {
	
	if(strcmp(data, "{\"data\":[],\"pagination\":{},\"points\":0,\"total\":0}") == 0) {
		printf(YELLOW("WARNING: There are no users subscribed to this broadcaster!\n"));
		return FALSE;
	}
	
	const char *p = data;

	// {"data":[{"broadcaster_id":
	p += 27;
	TWLPGetValue(&p, subscriptions->broadcaster_id);

	// "broadcaster_login":
	p += 21;
	TWLPGetValue(&p, subscriptions->broadcaster_login);

	// "broadcaster_name":
	p += 20;
	TWLPGetValue(&p, subscriptions->broadcaster_name);

	// "gifter_id":
	p += 13;
	TWLPGetValue(&p, subscriptions->gifter_id);

	// "gifter_login":
	p += 16;
	TWLPGetValue(&p, subscriptions->gifter_login);

	// "gifter_name":
	p += 15;
	TWLPGetValue(&p, subscriptions->gifter_name);

	// "is_gift":
	char tmp[8] = {};
	p += 11;
	TWLPGetValue(&p, tmp);
	subscriptions->is_gift = (strcmp(tmp, "true") == 0) ? TRUE : FALSE;

	// "tier":
	p += 8;
	TWLPGetValue(&p, subscriptions->tier);

	// "plan_name":
	p += 13;
	TWLPGetValue(&p, subscriptions->plan_name);

	// "user_id":
	p += 11;
	TWLPGetValue(&p, subscriptions->user_id);

	// "user_name":
	p += 13;
	TWLPGetValue(&p, subscriptions->user_name);

	// "user_login":
	p += 14;
	TWLPGetValue(&p, subscriptions->user_login);

	// "pagination":{"cursor":"xxxx"},
	p += 24;
	while(*p != ',') p++;

	// "total":13,
	memset(tmp, '\0', sizeof(tmp));
	p += 9;
	TWLPGetValue(&p, tmp);
	subscriptions->total = atoi(tmp);

	// "points":13}
	memset(tmp, '\0', sizeof(tmp));
	p += 10;
	TWLPGetValue(&p, tmp);
	subscriptions->points = atoi(tmp);
	

	free((void*)data);
	return TRUE;
}


BOOL TWLPUserSubscription(const char *data, FTWLSubscription *subscription) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: The user has no subscription to this broadcaster!\n"));
		return FALSE;
	}
	
	const char *p = data;

	// {"data":[{"broadcaster_id":
	p += 27;
	TWLPGetValue(&p, subscription->broadcaster_id);

	// "broadcaster_name":
	p += 20;
	TWLPGetValue(&p, subscription->broadcaster_name);

	// "broadcaster_login":
	p += 21;
	TWLPGetValue(&p, subscription->broadcaster_login);
	
	// "is_gift":
	char tmp[8] = {};
	p += 11;
	TWLPGetValue(&p, tmp);
	subscription->is_gift = (strcmp(tmp, "true") == 0) ? TRUE : FALSE;

	// "tier":
	p += 8;
	TWLPGetValue(&p, subscription->tier);

	free((void*)data);
	return TRUE;
}

// =========================== //		
//            USERS            //		
// =========================== //		
BOOL TWLPUserInfo(const char *data, FTWLUserInfo *user_info) {

	if(strcmp(data, "{\"data\":[]}") == 0) {
		printf(YELLOW("WARNING: The user you are looking for doesn't exist!\n"));
		return FALSE;
	}

	const char *p = data;

	// skip the whole {"data":[{"id":" part
	p += 15;
	TWLPGetValue(&p, user_info->id);

	p += 9;
	TWLPGetValue(&p, user_info->login);

	p += 16;
	TWLPGetValue(&p, user_info->display_name);

	p += 8;
	char tmp[16] = {};
	TWLPGetValue(&p, tmp);
	
	if(strcmp(tmp, "") == 0) {
		user_info->type = TWL_UT_NORMAL;
	} else if(strcmp(tmp, "admin") == 0) {
		user_info->type = TWL_UT_ADMIN;
	} else if(strcmp(tmp, "global_mod") == 0) {
		user_info->type = TWL_UT_GLOBAL_MOD;
	} else if(strcmp(tmp, "staff") == 0) {
		user_info->type = TWL_UT_STAFF;
	} else {
		user_info->type = TWL_UT_NONE;
	}

	p += 20;
	memset(tmp, 0x00, 16);
	TWLPGetValue(&p, tmp);
	
	if(strcmp(tmp, "") == 0) {
		user_info->broadcaster_type = TWL_BT_NORMAL;
	} else if(strcmp(tmp, "affiliate") == 0) {
		user_info->broadcaster_type = TWL_BT_AFFILIATE;
	} else if(strcmp(tmp, "partner") == 0) {
		user_info->broadcaster_type = TWL_BT_PARTNER;
	} else {
		user_info->broadcaster_type = TWL_BT_NONE;
	}
	
	p += 15;
	TWLPGetValue(&p, user_info->description);
	
	p += 21;
	TWLPGetValue(&p, user_info->profile_image_url);
	
	p += 21;
	TWLPGetValue(&p, user_info->offline_image_url);
	
	// #WARNING view_count is deprecated
	p += 14;
	memset(tmp, 0x00, 16);
	TWLPGetValue(&p, tmp);
	user_info->view_count = atoi(tmp);
	
	// WARNING: require user:read:email scope
	if(*(p+2) == 'e') {
		p += 9;
		TWLPGetValue(&p, user_info->email);
	}

	p += 14;
	TWLPGetValue(&p, user_info->created_at);

	free((void*)data);
	return TRUE;
}


// =========================== //		
//          UTILITIES          //
// =========================== //		
void TWLPrintUserInfo(const FTWLUserInfo *user_info) {

	char type[16] = {};
	switch(user_info->type) {
		case TWL_UT_NORMAL:
			strcpy(type, "normal");
			break;

		case TWL_UT_ADMIN:
			strcpy(type, "admin");
			break;

		case TWL_UT_GLOBAL_MOD:
			strcpy(type, "global mod");
			break;

		case TWL_UT_STAFF:
			strcpy(type, "staff");
			break;

		case TWL_UT_NONE:
			strcpy(type, "undefined");
			break;
	}

	char broadcaster_type[16] = {};
	switch(user_info->broadcaster_type) {
		case TWL_BT_NORMAL:
			strcpy(broadcaster_type, "normal");
			break;

		case TWL_BT_AFFILIATE:
			strcpy(broadcaster_type, "affiliate");
			break;

		case TWL_BT_PARTNER:
			strcpy(broadcaster_type, "partner");
			break;

		case TWL_BT_NONE:
			strcpy(broadcaster_type, "undefined");
			break;
	}

	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Login: ", user_info->login);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Display name: ", user_info->display_name);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "ID: ", user_info->id);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "User type: ", type);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Broadcast type: ", broadcaster_type);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Description: ", user_info->description);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Profile image: ", user_info->profile_image_url);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Offline image: ", user_info->offline_image_url);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%i\n", "View count: ", user_info->view_count);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Email: ", user_info->email);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Created at: ", user_info->created_at);
	printf("\x1B[0m");

}


void TWLPrintChannelInfo(const FTWLChannelInfo *channel_info) {

	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Login: ", channel_info->broadcaster_login);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Display name: ", channel_info->broadcaster_name);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "ID: ", channel_info->broadcaster_id);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Language: ", channel_info->broadcaster_language);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Game ID: ", channel_info->game_id);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Game name: ", channel_info->game_name);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%s\n", "Title: ", channel_info->title);
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%i\n", "Delay: ", channel_info->delay);

	printf("\x1B[94m" "%-16s" "\x1B[92m", "Tags: ");
	for(size_t i=0; i<16; i++) {
		if(strcmp(channel_info->tags[i], "") == 0) break;
		if(strcmp(channel_info->tags[i+1], "") != 0) {
			printf("%s, ", channel_info->tags[i]);
		} else {
			printf("%s", channel_info->tags[i]);
		}
	}
	printf("\n");

	printf("\x1B[94m" "%-16s" "\x1B[92m", "CCLs: ");
#if 0 // #TODO
	for(size_t i=0; i<16; i++) {
		if(strcmp(channel_info->ccl[i], "") == 0) break;
		if(strcmp(channel_info->ccl[i+1], "") != 0) {
			printf("%s, ", channel_info->ccl[i]);
		} else {
			printf("%s", channel_info->ccl[i]);
		}
	}
#endif
	printf("\n");
	
	printf("\x1B[94m" "%-16s" "\x1B[92m" "%i\n", "Is branded content: ", channel_info->is_branded_content);
	printf("\x1B[0m");
}


// ============================== //
//           INTERNALS            //
// ============================== //
void TWLPGetValue(const char **p, char *value) {
	
	// in general we move from the first '"' to the first ',' after the last '"'
	// edge cases like non-string value apply.

#if TWLP_DEBUG_GET_VALUE
	printf("\x1B[93m\n%c<-%c->%c\n\x1B[0m", *(*p-1), **p, *(*p+1));
#endif

	// edge case where the value don't have "" (like view_count or is_branded_content)
	if(**p != '"') {

		for(size_t i=0; **p != ','; i++) {
			if(**p == '}') break;	// end of json object
			if(**p == '"') { (*p)++; i--; continue; }
			value[i] = *(*p)++;
		}

	} else {
		
		for(size_t i=0, deep=2; deep > 0; i++) {
			if(**p == '"') { (*p)++; i--; deep--; continue; }
			value[i] = *(*p)++;
		}
	}
	
#if TWLP_DEBUG_GET_VALUE
	printf("\x1B[94m%s\x1B[0m", value);
	printf("\x1B[92m\n%c<-%c->%c\n\x1B[0m", *(*p-1), **p, *(*p+1));
#endif
}


void TWLPGetValues(const char **p, char values[][64]) {

#if TWLP_DEBUG_GET_VALUES
	printf("\x1B[93m\n%c<-%c->%c\n\x1B[0m", *(*p-1), **p, *(*p+1));
#endif

	// in general we move from after the first '[' to the first ',' after the last ']'
	// land after [
	(*p)++;
	size_t i = 0;
	while(**p != ']') {
		for(size_t j=0, deep=2; j<32 && deep>0; j++) {
			if(**p == ',') {(*p)++; j--; continue; }
			if(**p == '"') {(*p)++; j--; deep--; continue; }
			values[i][j] = *(*p)++;
		}
		i++;
	}
	// land after ]
	(*p)++;

#if TWLP_DEBUG_GET_VALUES
	printf("\x1B[94m%s\x1B[0m", values[0]);
	printf("\x1B[92m\n%c<-%c->%c\n\x1B[0m", *(*p-1), **p, *(*p+1));
#endif
}


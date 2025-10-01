#pragma once

#ifdef _WIN32
#ifdef TWL_PARSER_EXPORTS
#define TWL_PARSER __declspec(dllexport)
#else
#define TWL_PARSER __declspec(dllimport)
#endif
#else
#define TWL_PARSER
#endif

#include <stdint.h>
#include "twl_enums.h"
#include "twl_structs.h"

#ifdef __cplusplus
extern "C" {
#endif


	// ========== TWITCH LIBRARY PARSER ========== //
	// =========================================== //
	
	// ========== ADS ========== //
	
	/**
	 */
	TWL_PARSER BOOL TWLPGetAdSchedule(const char *data, FTWLAdSchedule *ad_schedule);

	// ========================= //
	
	// ========== ANALYTICS ========== //
	
	/**
	 */
	TWL_PARSER BOOL TWLPGetExtensionAnalytics(const char *data, FTWLExtensionAnalytics *reports);
	
	/**
	 */
	TWL_PARSER BOOL TWLPGetGameAnalytics(const char *data, FTWLGameAnalytics *reports);

	// =============================== //

	// ========== BITS ========== //
	
	/**
	 */
	TWL_PARSER BOOL TWLPGetBitsLeaderboard(const char *data, FTWLBitsLeaderboard *leaderboard);

	/**
	 */
	TWL_PARSER BOOL TWLPGetCheermotes(const char *data, FTWLCheermoteSet *cheermotes);

	/**
	 */
	TWL_PARSER BOOL TWLPGetExtensionTransactions(const char *data, void *transactions);

	// ========================== //
	
	// ========== CHANNELS ========== //
	
	/**
	 * Get channel information (only one channel)
	 * 
	 * @param data				The data to parse.
	 
	 * @param channel_info		Pointer to a FTWLChannelInfo struct
	 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChannelInfo(const char *data, FTWLChannelInfo *channel_info);

	/**
	 * Get the broascaster's list editors.
	 * 
	 * @param data				The data to parse.
	 * 
	 * @param channel_editors	Array that's gonna gets the broadcaster's list editors
	 * 							MAX SIZES 64
	 * 
	 * @return					TRUE if parsed successfully, FALSE if there are no editors.
	 *
	 */
	TWL_PARSER BOOL TWLPChannelEditors(const char *data, FTWLChannelEditor *channel_editors);
	
	/**
	 * Gets a list of broadcasters that the specified user follows.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param followed			The list of followed channels, it can be empty.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPFollowedChannels(const char *data, FTWLFollowedChannel *followed_channels);
	
	/**
	 * Gets a list of users that follow the specified broadcaster.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param followers			The list of followers, it can be empty.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChannelFollowers(const char *data, FTWLChannelFollower *followers);

	// ============================== //
	
	// ========== CHANNEL POINTS ========== //
	// ==================================== //
	
	// ========== CHARITY ========== //
	// ============================= //

	// ========== CHAT ========== //
	
	/**
	 * Gets the list of users that are connected to the broadcaster's chat session.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param chatters			The list of followers, it can be empty.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChatters(const char *data, FTWLChatter *chatters);

	/**
	 * Gets the broadcaster's list of custom emotes.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param emotes			The list of emotes.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChannelEmotes(const char *data, FTWLEmote *emotes);

	/**
	 * Gets the list of global emotes.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param emotes			The list of emotes.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPGlobalEmotes(const char *data, FTWLEmote *emotes);

	/**
	 * Gets emotes for one or more specified emote sets.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param emote_sets		The list of emote sets.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPEmoteSets(const char *data, FTWLEmoteSet *emote_sets);

	/**
	 * Gets the broadcaster's list of custom chat badges.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param badges			The list of chat badges.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChannelChatBadges(const char *data, FTWLBadge *badges);

	/**
	 * Gets Twitch's list if chat badges, which users may use in any channels char room.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param badges			The list of chat badges.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPGlobalChatBadges(const char *data, FTWLBadge *badges);

	/**
	 * Gets the broadcaster's chat setting.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param chat_settings		The chat settings.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPChatSettings(const char *data, FTWLChatSettings *chat_settings);

	/**
	 * Retrevies the active shared chat session for a channel.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param chat_session		The shared chat session.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPSharedChatSession(const char *data, FTWLChatSession *chat_session);

	/**
	 * Retrievies emotes available to the user across all channels.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param emotes			The user emotes.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPUserEmotes(const char *data, FTWLUserEmote *emotes);

	/**
	 * Gets the color used for the user's name in chat.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param color				The user color.	#TODO single user or array of users?
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPUserChatColor(const char *data, FTWLUserChatColor *color);
	// ========================== //
	
	// ========== CLIPS ========== //
	
	/**
	 * Gets one or more video clips that were captured from streams.
	 *
	 * @param data				The data to parse.
	 * 
	 * @param is_just_created	Set it to TRUE if you call it shortly after TWLCreateClip
	 * 							to gather the clip id and edit_url.
	 * 							Set it to FALSE if you are looking for the list of clips.
	 * 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPClips(const char *data, const BOOL is_just_created, FTWLStreamClip *clips);
	// =========================== //

	// ========== CONDUITS ========== //
	// ============================== //

	// ========== CCLs ========== //
	
	TWL_PARSER BOOL TWLPContentClassificationLabels(const char *data, FTWLCCLabel *ccls);
	// ========================== //
  
	// ========== ENTITLEMENTS ========== //
	// ================================== //

	// ========== EXTENSIONS ========== //
	// ================================ //
	
	// ========== EVENTSUB ========== //
	
	/**
	 * Parse the created EventSub subscription.
	 *
	 * @param data				The data to parse.
	 *
	 * @param event_sub			The EventSub subscription.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPEventSubscription(const char *data, FTWLEventSub *event_sub);
	// ============================== //
	
	// ========== GAMES ========== //
	
	TWL_PARSER BOOL TWLPGames(const char *data, FTWLGame *top_games);
	// =========================== //

	// ========== GOALS ========== //
	
	TWL_PARSER BOOL TWLPGoals(const char *data, FTWLGoal *goals);
	// =========================== //

	// ========== GUEST STAR ========== //
	// ================================ //

	// ========== HYPE TRAIN ========== //
	
	TWL_PARSER BOOL TWLPHypeTrainEvents(const char *data, FTWLHypeTrainEvent *hype_train_events);
	// ================================ //

	// ========== MODERATION ========== //
	// ================================ //

	// ========== POLLS ========== //
	// =========================== //

	// ========== PREDICTIONS ========== //
	// ================================= //

	// ========== RAIDS ========== //
	// =========================== //

	// ========== SCHEDULE ========== //
	// ============================== //

	// ========== SEARCH ========== //
	
	/**
	 * Gets the games or categories that match the specified query.
	 *
	 * @param data				The data to parse.
	 *
	 * @param matches			The list of games or categories that match the query.
	 * 							The list is empty if there are no matches.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPSearchCategories(const char *data, FTWLGame *matches);

	/**
	 * Gets the channels that match the specified query and have streamed content the past 6 months.
	 *
	 * @param data				The data to parse.
	 *
	 * @param matches			The list of channels that match the query.
	 * 							The list is empty if there are no matches.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPSearchChannels(const char *data, FTWLChannelInfo *matches);
	// ============================ //

	// ========== STREAMS ========== //
	
	/**
	 * Gets the channel's stream key.
	 *
	 * @param data				The data to parse.
	 
	 * @param stream_key		The stream key.
	 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPStreamKey(const char *data, FTWLStreamKey *stream_key);
	
	/**
	 * Gets a list of all streams.
	 *
	 * @param data				The data to parse.
	 
	 * @param streams			The streams.
	 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPStreams(const char *data, FTWLStream *streams);

	/**
	 * Gets the list of broadcasters that the user follows and that are stremaing live.
	 *
	 * @param data				The data to parse.
	 
	 * @param followed_streams	The followed streams.
	 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPFollowedStreams(const char *data, FTWLStream *followed_streams);

	/**
	 * Gets the list of stream markers.
	 *
	 * @param data				The data to parse.
	 *
	 * @param is_just_created	If TRUE parse the data for one just created marker,
	 * 							if FALSE parse the data for all existent markers.
	 *
	 * @param stream_markers	The list of stream markers.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPStreamMarkers(const char *data, const BOOL is_just_created, FTWLStreamMarker *stream_markers);
	// ============================= //
	
	// ========== SUBSCRIPTIONS ========== //
	
	/**
	 * Gets a list of user that subscribe to the specified broadcaster,
	 *
	 * @param data				The data to parse.
	 *
	 * @param subscriptions		The list of users that subscribe to the broadcaster.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPBroadcasterSubscriptions(const char *data, FTWLSubscription *subscriptions);

	/**
	 * Checks whether the user subscribes to the broadcater's channel.
	 *
	 * @param data				The data to parse.
	 *
	 * @param subscription		The information about user's subscription.
	 *
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPUserSubscription(const char *data, FTWLSubscription *subscription);
	// =================================== //
	
	// ========== TAGS ========== //
	// DEPRECATED
	// ========================== //

	// ========== TEAMS ========== //
	// =========================== //
	
	// ========== USERS ========== //		
	
	/**
	 * Gets user information (only one user)
	 *
	 * @param data				The data to parse.
	 
	 * @param emotes			The user info.
	 
	 * @return					TRUE if parsed successfully, FALSE otherwise.
	 *
	 */
	TWL_PARSER BOOL TWLPUserInfo(const char *data, FTWLUserInfo *user_info);
	
	// =========================== //		

	// ========== VIDEOS ========== //
	// ============================ //

	// ========== WHISPERS ========== //
	// ============================== //

	

	// ========== LIBRARY UTILITIES ========== //
	
	/**
	 * Print the User Info retrevied with TWLGetUserInfo
	 *
	 * @param user_info User information struct
	 *
	 */
	TWL_PARSER void TWLPrintUserInfo(const FTWLUserInfo *user_info);
	
	/**
	 * Print the Channel Info retrevied with TWLGetChannelInfo
	 *
	 * @param user_info User information struct
	 *
	 */
	TWL_PARSER void TWLPrintChannelInfo(const FTWLChannelInfo *channel_info);
	// ======================================= //
	
	// ========== LIBRARY INTERNALS ========== //
	
	/*
	 *
	 * #TODO rename this function and decide if expose it to the library
	 */
	void TWLPGetValue(const char **p, char *value);

	/*
	 *
	 * #TODO rename this function and decide if expose it to the library
	 */
	void TWLPGetValues(const char **p, char values[][64]);
	// ======================================= //

#ifdef __cplusplus
}
#endif

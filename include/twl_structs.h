#pragma once

#ifdef _WIN32
#ifdef TWL_API_EXPORTS
#define TWL_API __declspec(dllexport)
#else
#define TWL_API __declspec(dllimport)
#endif
#else
#define TWL_API
#endif

#include <stdint.h>
#include <windows.h>
#include <winhttp.h>
#include "twl_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

// ======================================== //
//            TWITCH LIBRARY API            //
// ======================================== //

	typedef struct FTWLSessionHandles {
		HINTERNET hSession;
		HINTERNET hConnection;
	} FTWLSessionHandles;

	typedef struct FTWLUserCredentials {
		wchar_t client_id[100];						// Twitch app client ID
		wchar_t oauth2_token[100];					// Twitch app OAuth token
	} FTWLUserCredentials;

// ========================= //
//            ADS            //
// ========================= //

	typedef struct FTWLCommercialStatus {
		int32_t length;								// Clamped to 180
		char message[64];
		int32_t retry_after;
	} FTWLCommercialStatus;

	typedef struct FTWLAdSchedule {
		int32_t snooze_count;
		char snooze_refresh_at[32];					// RFC3339
		char next_ad_at[32];						// RFC3339
		int32_t duration;
		char last_ad_at[32];						// RFC3339
		int32_t preroll_free_time;
	} FTWLAdSchedule;

	typedef struct FTWLSnoozes {
		int32_t snooze_count;
		char snooze_refresh_at[32];					// RFC3339
		char next_ad_at[32];						// RFC3339
	} FTWLSnoozes;

// =============================== //
//            ANALYTICS            //
// =============================== //

	typedef struct FTWLAnalytics {
		union {
			char extension_id[64];
			char game_id[64];
		};
		char url[256];
		char type[16];
		char started_at[32];						// RFC3339
		char ended_at[32];							// RFC3339
	} FTWLExtensionAnalytics, FTWLGameAnalytics;

// ========================== //
//            BITS            //
// ========================== //

	typedef struct FTWLBitsLeader {
		char user_id[64];
		char user_login[32];
		char user_name[32];
		int32_t rank;								// The user's position on the leaderboard
		int32_t score;								// The number of Bits the user has cheered
	} FTWLBitsLeader;

	typedef struct FTWLBitsLeaderboard {
		FTWLBitsLeader leaders[10];					// min 1 max 100 default 10
		char started_at[32];						// RFC3339
		char ended_at[32];							// RFC3339
		int32_t total;
	} FTWLBitsLeaderboard;


	typedef struct FTWLCheermoteImage {
		char dark_animated[5][256];
		char dark_static[5][256];
		char light_animated[5][256];
		char light_static[5][256];
	} FTWLCheermoteImage;

	typedef struct FTWLCheermoteTier {
		int32_t min_bits;
		int32_t id;
		char color[8];
		FTWLCheermoteImage images;
		BOOL can_cheer;
		BOOL show_in_bits_card;

	} FTWLCheermoteTier;

	typedef struct FTWLCheermoteSet {
		char prefix[16];
		FTWLCheermoteTier tiers[7];
		char type[20];
		int32_t order;
		char last_updated[32];						// RFC3339
		BOOL is_charitable;
	} FTWLCheermoteSet;

// ========================== //
//            CCLs            //
// ========================== //

	typedef struct FTWLCCLabel {
		char id[31];								// ID of the Content Classification Lable.
													// Can be one of the following values:
													// 	* DebatedSocialIssuesAndPolitics
													// 	* DrugsIntoxication
													// 	* SexualThemes
													// 	* ViolentGraphic
													//	* Gambling
													//	* ProfanityVulgarity

		BOOL is_enabled;							// Flag indicating whether the label should
													// be enabled (TRUE) or disabled for the channel.
	
		char description[256];						// Localized description of the CCL.

		char name[64];								// Localized name of the CCL.
	} FTWLCCLabel;

// ============================== //
//            CHANNELS            //
// ============================== //

	typedef struct FTWLChannelInfo {
		char thumbnail_url[256];
		char game_name[256];
		char title[256];							// Title of the stream (140 characters)
		char broadcaster_id[64];
		char game_id[64];
		char started_at[64];
		char broadcaster_login[32];
		char broadcaster_name[32];
		char broadcaster_language[8];				// ISO 639-1 two letter langage code, set to "other" if not supported
		FTWLCCLabel ccl[6];							// Content Classification Label
		char tags[16][64];							// [10][25] but we use 64 for the tags so we can use one function for both this and CCLs
		uint32_t delay;
		BOOL is_branded_content;
		BOOL is_live;
	} FTWLChannelInfo;

	typedef struct FTWLChannel {
		union {
			char created_at[64];
			char followed_at[64];
		};
		union {
			char user_id[64];
			char broadcaster_id[64];
		};
		union {
			char user_name[64];
			char broadcaster_name[64];
		};
		union {
			char user_login[32];
			char broadcaster_login[32];
		};
	} FTWLChannelEditor, FTWLChannelFollower, FTWLFollowedChannel;

// ==================================== //
//            CHANNEL POINTS            //
// ==================================== //
	typedef struct FTWLReward {
	
		char title[46];								// The custom reward's title.
	  												// The title may contain a maximum of 45 characters and
	  												// it must be unique amongst all of the broadcaster's
	  												// custom rewards.
	
		int32_t cost;								// The cost of the reward, in Channel Points.
													// The minimum is 1 point.

		char prompt[201];							// The prompt shown to the viewer when they redeem the reward.
													// Specify a prompt if is_user_input_required is TRUE.
													// The prompt is limited to a maximum of 200 characters.

		BOOL is_enabled;							// Determines whether the reward is enabled.
													// Viewers see only enabled rewards.
													// The default is TRUE.

		char background_color[8];					// The background color to use for the reward.
													// Specify the color using Hex format (#9147FF)

		BOOL is_user_input_required;				// Determines whether the user needs to enter information
													// when redeeming the reward, see the 'prompt' field.
													// The default is FALSE.

		BOOL is_max_per_stream_enabled;				// Determines whether to limit the maximum number of redemptions
													// allowed per live stream, see max_per_stream.
													// The default is FALSE.
		
		int32_t max_per_stream;						// The maximum number of redemptions allowed per live stream.
													// Applied only if is_max_per_stream_enabled is TRUE.
													// The minimum value is 1.
		
		BOOL is_max_per_user_per_stream_enabled;	// Deterimines whether to limit the maximum number of redemptions
													// allowed per user per stream, see max_per_user_per_stream.
													// The default is FALSE.
		
		int32_t max_per_user_per_stream;			// The maximum nuber of redemptions allowed per user per stream.
													// Applied only if is_max_per_user_per_stream_enabled is TRUE;
													// The minimum value is 1.

		BOOL is_global_cooldown_enabled;			// Determines whether to apply a cooldown period between
													// redemptions, see global_cooldown_secons for the duration
													// of the cooldown period.
													// The default is FALSE.
		
		int32_t global_cooldown_seconds;			// The cooldown perios, in seconds.
													// Applied only if is_global_cooldown_enabled is TRUE.
													// The minimum value is 1, however, the m,inimum value is 60
													// for it to be shown in the Twitch UX.

		BOOL is_paused;								// Determines whether to pause the reward.
													// Set to TRUE to pause the reward.
													// Viewers can't redeem paused rewards.

		BOOL should_redemptions_skip_request_queue;	// Determines whether redemptions should be set to FULFILLED
													// status immediately when a reward is redeemed.
													// If FALSE, status is set to UNFULFILLED and follows the
													// normal request queue process.
													// The default is FALSE.

	} FTWLReward;


// ========================== //
//            CHAT            //
// ========================== //
	typedef struct FTWLChatter {
		char user_id[64];
		char user_name[32];
		char user_login[32];
	} FTWLChatter;

	typedef struct FTWLEmote {
		char id[64];
		char name[64];
		char images[3][128];						// #TODO maybe ignore those three URLs and use the template as suggested in the docs?
		char tier[32];
		char emote_type[16];						// bitstier, follower, subscriptions #TODO what about use enums?
		char emote_set_id[64];
		char format[2][64];							// static, animated #TODO what about use enums?
		char scale[3][64];							// 1.0, 2.0, 3.0 (small 28px, medium 56px, large 112px)
		char theme_mode[2][64];						// light, dark
		char owner_id[64];
	} FTWLEmote, FTWLEmoteSet, FTWLUserEmote;


	// #TODO fix versions (maybe dinamyc allocation)
	typedef struct FTWLBadge {
		char set_id[64];
		char versions[128][128];
	} FTWLBadge;

	typedef struct FTWLChatSettings {

		char broadcaster_id[64];

		BOOL is_slow_mode;							// Determines whether the broadcaster limits how often users in the 
													// chat room are allowed to send messages.
													// Set to TRUE if the broadcaster applies a wait period between
													// messages, otherwise FALSE.
													// The default is FALSE.
													// To specify the delay, see the slow_mode_wait_time field.
		
		int32_t slow_mode_wait_time;				// The amount of time, in seconds, that users must wait between
													// sending messages.
													// Set only if slow mode is TRUE.
													// Possible values are: 3 (3 second delay) through 120 (2 mionute delay).
													// The default is 30 seconds.

		BOOL is_follower_mode;						// Determines whether the broadcaster restricts the chat room to followers only.
													// Set to TRUE if the broadcaster restricts the chat room to followers only, otherwise FALSE.
													// The default is TRUE.
													// To specify how long users must follow the broadcaster before being able to partecipate
													// in the chat room, see the follower_mode_duration field.
		
		int32_t follower_mode_duration;				// The length of time, in minutes, that users must follow the broadcaster before
													// being able to participate in the chat room.
													// Set only if follower_mode is TRUE.
													// Possible values are 0 (no restriction) through 129600 (3 months).
													// The default is 0.
		
		BOOL is_subscriber_mode;					// Determines whether only users that subscribe to the broadcaster's
													// channel may talk in the chat room.
													// Set to TRUE if the broadcaster restricts the chat room to subscribers only,
													// otherwise FALSE.
													// The default is FALSE.
		
		BOOL is_emote_mode;							// Determines whether chat messagees must contain only emotes.
													// Set to TRUE if only emotes are allowed, otherwise FALSE.
													// The default is FALSE.

		BOOL is_unique_chat_mode;					// Determines whether the broadcaster requires users to post only unique
													// messages in the chat room.
													// Set to TRUE if the broadcaster allows only unique messages, otherwise FALSE.
													// The default is FALSE.
		
		BOOL is_non_moderator_chat_delay;			// Determines whether the broadcaster adds a short delay before
													// chat messages appear in the chat room.
													// This gives chat moderators and bots a chance to remove them
													// before viewers can see the message.
													// Set to TRUE if the broadcaster applies a delay, otherwise FALSE.
													// The default is FALSE.
													// To specify the length of the delay, see the non_moderator_chat_delay_duration field.

		int32_t non_moderator_chat_delay_duration;	// The amount of time, in seconds, that messages are delayed before appearing in chat.
													// Set only if non_moderator_chat_delay is TRUE.
													// Possible values are:
													// 	* 2 - 2 second delay (recommended)
													// 	* 4 - 4 second delay
													// 	* 6 - 6 second delay
	} FTWLChatSettings;

	// #TODO how many participants?
	typedef struct FTWLChatSession {
		char session_id[64];
		char host_broadcaster_id[64];
		char participants[8][64];
		char created_at[64];
		char updated_at[64];
	} FTWLChatSession;

	typedef struct FTWLUserChatColor {
		char user_id[64];
		char user_name[32];
		char user_login[32];
		char color[8];
	} FTWLUserChatColor;


// =========================== //
//            CLIPS            //
// =========================== //
	typedef struct FTWLStreamClip {
		char edit_url[256];
		char clip_url[256];
		char embed_url[256];
		char thumbnail_url[256];
		char clip_id[64];
		char game_id[64];
		char video_id[64];
		char broadcaster_id[64];
		char broadcaster_name[64];
		char creator_id[64];
		char creator_name[64];
		char title[64];
		char created_at[64];
		char language[8];
		int32_t view_count;
		int32_t vod_offset;
		float duration;
		BOOL is_featured;
	} FTWLStreamClip;


// ============================== //
//            CONDUITS            //
// ============================== //
	typedef struct FTWLShard {
		char callback[256];							// only for webhook
		char id[64];
		char method[64];							// webhook or websocket
		char secret[64];							// only for webhook
		char session_id[64];						// only for websocket
	} FTWLShard;


// ========================== //
//            CCLs            //
// ========================== //
// MOVED UP BECAUSE REASONS

// =========================== //
//            GAMES            //
// =========================== //
	typedef struct FTWLGame {
		char box_art_url[256];
		char game_id[64];
		char igdb_id[64];
		char name[64];
	} FTWLGame;


// =========================== //
//            GOALS            //
// =========================== //
	typedef struct FTWLGoal {
		char goal_id[64];
		char broadcaster_id[64];
		char broadcaster_name[32];
		char broadcaster_login[32];
		char type[64];
		char description[64];
		char created_at[64];
		int32_t current_amount;
		int32_t target_amount;
	} FTWLGoal;


// ================================ //
//            HYPE TRAIN            //
// ================================ //

	typedef struct FTWLHTContribution {
		int32_t total;								// If type is BITS -> amount of bits. If type is SUBS -> 500, 1000, 2500 -> tier 1, 2 or 3 sub
		char type[8];								// BITS, SUBS, OTHER
		char user[64];
	} FTWLHTContribution;

	typedef struct FTWLHypeTrainEvent {
		char event_id[64];
		char event_type[64];						// This string is in the form hypetrain.{event_name}
		char event_timestamp[64];
		char version[8];
		// event_data
		char broadcaster_id[64];
		char cooldown_end_time[64];
		char expires_at[64];
		int32_t goal;
		char hype_train_id[64];
		FTWLHTContribution last_contribution;
		int32_t level;
		char started_at[64];
		FTWLHTContribution top_contributions[3];	// The top contributors for each contribution type.	
		int32_t total;								// The current total amount raised.
		// end of event_data
	} FTWLHypeTrainEvent;

// ================================ //
//            EXTENSIONS            //
// ================================ //
	typedef struct FTWLProductCost {
		int32_t amount;								// The product's price.
		char type[8];								// The type of currency
													// Possible values are:
													// 	* bits - The minimum price is 1 and the maximum is 10000.
	} FTWLProductCost;

// ============================== //
//            EVENTSUB            //
// ============================== //
	typedef struct FTWLEventSubSession {
		HINTERNET hSession;
		HINTERNET hConnection;
		HINTERNET hWebsocket;
		char session_id[64];
	} FTWLEventSubSession;

	typedef struct FTWLEventType {
		char condition[256];
		char transport[256];
		char type[256];
		char version[256];
	} FTWLEventType;

	typedef struct FTWLEventSub {
		char id[64];								// An ID that identifies the subscription.
		char status[38];							// Possible values are: enabled, webhook_callback_verification_pending.
		char type[64];
		char version[8];
		char condition[256];						// The subscriptions's parameter values. Is a string-encoded JSON object determined by the sub.
		char created_at[64];						// RFC3339
		char transport[256];						// The transport details used to send the notifications.
		int32_t cost;								// The amount that the subscription counts against your limit.
		int32_t total;								// The total number of subs you've created.
		int32_t max_total_cost;						// The maximum total cost that you're allowed to incur for all subs you create.
		int32_t total_cost;							// The sum of all of your subs costs.
	} FTWLEventSub;

// ================================ //
//            MODERATION            //
// ================================ //
	typedef struct FTWLAutoModSettings {
		int32_t aggresion;							// The Automod level for hostility involving aggression.
		int32_t bullying;							// The Automod level for hostility involving name calling or insults.
		int32_t disability;							// The Automod level for discrimination against disability.
		int32_t misogyny;							// The Automod level for discrimination against women..
		int32_t overall_level;						// The default Automod level for the broadcaster.
		int32_t race_ethnicity_or_religion;			// The Automod level for racial discrimination.
		int32_t sex_based_terms;					// The Automod level for sexual content.
		int32_t sexuality_sex_or_gender;			// The Automod level for discrimination based on sexuality, sex, or gender.
		int32_t swearing;							// The Automod level for profanity.
	} FTWLAutoModSettings;

	typedef struct FTWLAutoModMessage {				// Used with TWLCheckAutoModStatus
		char msg_id[64];							// A caller-defined ID used to correlate this message with the
													// same message in the response.
		char msg_text[500];							// The message to check.aaaaa
	} FTWLAutoModMessage;


// ============================= //
//            STREAMS            //
// ============================= //
	typedef struct FTWLStreamKey {
		char strem_key[64];
	} FTWLStreamKey;

	typedef struct FTWLStream {
		char id[64];
		char user_id[64];
		char user_login[32];
		char user_name[32];
		char game_id[64];
		char game_name[256];
		char type[8];								// This is always set to "live", empty if an error occurs.
		char title[256];
		char tags[16][64];
		int32_t viever_count;
		char started_at[64];
		char language[8];
		char thumbnail_url[256];					// Replace {width}x{height} with the size of the image you want.
		char tag_ids;								// DEPRECATED: returns only an empty array.
		BOOL is_mature;
	} FTWLStream;

	typedef struct FTWLStreamMarker {
		char user_id[64];
		char user_name[32];
		char user_login[32];
		char video_id[64];
		char marker_id[64];
		char created_at[64];
		char description[256];						// Max 140 characters!
		char position_seconds[64];
		char url[256];
	} FTWLStreamMarker;


// =================================== //
//            SUBSCRIPTIONS            //
// =================================== //
	typedef struct FTWLSubscription {
		char broadcaster_id[64];
		char broadcaster_login[32];
		char broadcaster_name[32];
		char gifter_id[64];							// Empty if is_gift is FALSE
		char gifter_login[64];						// Empty if is_gift is FALSE
		char gifter_name[64];						// Empty if is_gift is FALSE
		BOOL is_gift;
		char tier[8];								// 1000 - Tier1 | 2000 - Tier 2 | 3000 - Tier 3
		char plan_name[64];
		char user_id[64];
		char user_login[32];
		char user_name[32];
		int32_t total;								// Total number of subscriptions
		int32_t points;								// Sum of sub points (Tier 1 is 1pt, Tier 2 is 2 pt, Tier 2 is 6pt
	} FTWLSubscription;

// =========================== //
//            USERS            //
// =========================== //
	typedef struct FTWLUserInfo {
		char profile_image_url[4096];	
		char offline_image_url[4096];
		char description[512];						// max 300 characters
		char email[512];							// RFC5321 (max 320 bytes)
		char created_at[64];						// UTC RFC3339 (max 35 bytes)
		char id[64];
		char display_name[32];						// 4->25 characters
		char login[32];								// 4->25 characters		
		ETWLBroadcasterType broadcaster_type;		// affiliate, partner, "" (normal broadcaster)
		ETWLUserType type;							// admin, global_mod, staff, "" (normal user)
		int32_t view_count;							// DEPRECATED
	} FTWLUserInfo;
	// ============================= //

#ifdef __cplusplus
}
#endif

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
#include "twl_structs.h"

#ifdef __cplusplus
extern "C" {
#endif


	// ========== TWITCH LIBRARY API ========== //
	
	/**
	 * Connect to Twitch API server.
	 * 
	 * @param user_credentials 		Twitch client ID and OAuth token
	 * 
	 * @return 						TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLConnect(FTWLUserCredentials *user_credentials);

	/**
	 * Reset the client id and the oauth token.
	 * Shutdown the connection and the session.
	 *
	 * @param user_credentials 		Twitch client ID and OAuth token
	 * 
	 * @return 						TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLShutdown(FTWLUserCredentials *user_credentials);

	/**
	 * Set the current API user, call it before any function that you want to be
	 * executed with different user credentials.
	 *
	 * @param user_credentials		Twitch client ID and OAuth token we want to use
	 * 								in the next API calls.
	 *
	 * @return						TRUE if user credentials are valid, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSetCurrentUser(const FTWLUserCredentials *user_credentials);

	// ======================================== //		

	// ========== ADS ========== //
	
	/**
	 * Starts a commercial on the specified channel.
	 *
	 * NOTE:	Only partners and affiliates may run commercials and they must be streaming 
	 * 			live at the time.
	 *
	 * NOTE:	Only the draodcaster may start a commercial, the broadcaster's editors and
	 * 			moderators may not start commercials on behalf of the broadcaster.
	 *
	 * Requires a user access token that includes tha channel:edit:commercial scope.
	 *
	 * @param braodcaster_id		The ID of the partner or affiliate braodcaster that want
	 * 								to run the commercial.
	 * 								This ID must match the user ID found in the OAuth token.
	 *
	 * @param length				The length of the commercial to run, in seconds.
	 * 								Twitch tries to serve a commercial that's the requested length,
	 * 								but it may be shorter ot longer.
	 * 								The maximum length you should request is 180 seconds.
	 *
	 * @param request_status		The status of your start commercial request.
	 *
	 * @return						TRUE if we successfully started the commercial, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLStartCommercial(const char *broadcaster_id, int32_t length, char **request_status);

	/**
	 * Returns ad schedule related information, including snooze, when the last ad
	 * was run, when the next ad is scheduled, and if the channel is currently in pre-roll free time.
	 * Note that a new ad cannot be run until 8 minutes after running previous ad.
	 *
	 * Requires a user access token that incldues the channel:read:ads scope.
	 * The user_id in the user access token must match the broadcaster_id.
	 *
	 * @param broadcaster_id		Must match the auth token user id.
	 *
	 * @param schedule				Information related to the channel's schedule.
	 *
	 * @return						TRUE if returns the ad schedule information for the channel,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetAdSchedule(const char *broadcaster_id, char **schedule);

	/**
	 * If available, pushes back the timestamp of the upcoming automatic mid-roll ad by 5 minutes.
	 * This function duplicates the snooze functionality in the creator dashboard's Ads Manager.
	 *
	 * Requires a user access token that includes the channel:manage:ads scope.
	 * The user_id in the user access token must match the broadcaster_id.
	 *
	 * @param broadcaster_id		Must match the auth token user id.
	 *
	 * @param snoozes				A list that contains information about the channel's snoozes
	 * 								and next upcoming ad after successfully snoozing.
	 *
	 * @return						TRUE if user's next ad is successfully snoozed.
	 * 								Their snooze_count is decremented and snooze_refresh_time
	 * 								and next_ad_at are both updated.
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSnoozeNextAd(const char *broadcaster_id, char **snoozes);
	// ========================= //
	
	// ========== ANALYTICS ========== //
	
	/**
	 * Gets an analytics report for one or more extensions.
	 * The response contains the URLs used to download the report (CSV files).
	 *
	 * Requires a user access token that includes tha analytics:read:extensions scope.
	 *
	 * @aparam extension_id			The extension's clientID.
	 * 								If specified, the response contains a report for the specified
	 * 								extension, if not specified, the response includes a report
	 * 								for each extension that the authenticated user owns.
	 * 								It can be NULL or an empty string.
	 *
	 * @param type					The type of the analytics report to get.
	 * 								Possible values are:
	 * 									* overview_v2
	 * 								It can be NULL or an empty string.
	 *
	 * @param started_at			The reporting window's start date, in RFC3339 format.
	 * 								Set the time portion to zeroes (2021-10-22T00:00:00Z).
	 *
	 * 								The start date must be on or after January 31, 2018.
	 * 								If you specify an earlier date, the API ignores it 
	 * 								and uses January 31, 2018.
	 * 								If you specify a start date, you must specify the end date.
	 * 								If you don't specify a start and end date, the report
	 * 								includes all available data since January 31, 2018.
	 *
	 * 								The report contains one row of data for each day 
	 * 								in the reporting window.
	 * 								It can be NULL or an empty string.
	 *
	 * @param ended_at				The reporting window's end date, in RFC3339 format.
	 * 								Set the time portion to zeroes (2021-10-22T00:00:00Z).
	 * 								The report is inclusive of the end date.
	 *
	 * 								Specify an end date only if you provide a start date.
	 * 								Because it can take up to two days for the data to be available,
	 * 								you must specify an end date that's earlier than today minus
	 * 								one to two days.
	 * 								If not, the API ignores your end date and uses an end date that is today minus one to two days.
	 * 								It can be NULL or an empty string.
	 *
	 * @param reports				A list of reports.
	 * 								The reports are returned in no particular order, however, the data
	 * 								whithin each report is in ascending order by date (newest first).
	 * 								The report contains one row of data per day of the reporting window,
	 * 								the report contains rows for only those days that the extension was used.
	 * 								The array is empty if there are no reports.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's analytics, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionAnalytics(
			const char *extension_id,
			const char *type,
			const char *started_at,
			const char *ended_at,
			char **reports);

	/**
	 * Gets an analytics report for one or more games.
	 * The response contains the URLs used to download the reports (CSV files).
	 *
	 * Requires a user access token that includes the analytics:read:games scope.
	 *
	 * @param game_id				The game's client ID.
	 * 								If specified, the response contains a report for the specified game.
	 * 								If not specified, the response includes a report for each of the
	 * 								authenticated user's games.
	 * 								It can be NULL or an empty string.
	 *
	 * @param type					The type of analytics report to get.
	 * 								Possible values are:
	 * 									* overview_v2
	 * 								It can be NULL or an empty string.
	 *
	 * @param started_at			The reporting window's start date, in RFC3339 format.
	 * 								Set the time portion to zeroes (2021-12-22T00:00:00Z).
	 * 								If you specify a start date, you must specify an end date.
	 *
	 * 								The start date must be within one year of today's date.
	 * 								if you specify an earlier date, the API ignores it and uses a date
	 * 								that's oen yer prior to today's date.
	 * 								If you don't specify a start and end date, the report includes
	 * 								all available data for the last 365 days from today.
	 *
	 * 								The report contains one row of data for each day in the
	 * 								reporting window.
	 * 								It can be NULL or an empty string.
	 *
	 * @param ended_at				The reporting window's end date, in RFC3339 format.
	 * 								Set the time portion to zeroes (2021-12-22T00:00:00Z).
	 * 								The report is inclusive of the enhd date.
	 *
	 *								Specify an end date only if you provide a start date.
	 *								Because it can take up to two days for the data to be available,
	 *								you must specify an end date that's earlier than today minus one
	 *								to two days.
	 *								If not, the API ignores your end date and uses an end date that
	 *								is today minus one to two days.
	 *								It can be NULL or an empty string.
	 *
	 * @param reports				A list of reports.
	 * 								The reports are returned in no particular order, however, the data
	 * 								whithin each report is in ascending order by date (newest first).
	 * 								The report contains one row of data per day of the reporting window,
	 * 								the report contains rows for only those days that the game was used.
	 * 								A report is available only if the game was broadcast for at least 5 hours
	 * 								over the reporting perios.
	 * 								The array is empty if there are no reports.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's analytics reports,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGameAnalytics(const char *game_id, const char *type, const char *started_at, const char *ended_at, char **reports);
	// =============================== //

	// ========== BITS ========== //
	
	/**
	 * Gets the Bits leaderboard for the authenticated broadcaster.
	 *
	 * Requires a user access token that includes the bits:read scope.
	 *
	 * @param count					The number of results to return;
	 * 								The minimum count is 1 and the maximum is 100.
	 * 								The default is 10.
	 * 								If set to 0 will be on default.
	 *
	 * @param period				The time over which data is aggregated (uses the PST time zone).
	 * 								Possible values are:
	 * 									* day - A day spans from 00:00:00 on the day specified in 
	 * 											'started_at' and runs through 00:00:00 of the next day.
	 *
	 * 									* week - A week spans from 00:00:00 on ther Monday of the week
	 * 											 specified in 'started_at' and runs through 00:00:00
	 * 											 of yhe next Monday.
	 *
	 * 									* month - A month spans from 00:00:00 on the first day of the month
	 * 											  specified in 'started_at' and runs through 00:00:00 of 
	 * 											  the first day of the next month.
	 *
	 * 									* year - A year spans from 00:00:00 on the first day of the year
	 * 											 specified in 'started_at' and runs through 00:00:00 of
	 * 											 the first day of the next year.
	 *
	 * 									* all - Default. The lifetime of the broadcaster channel.
	 *
	 * 								It can be NULL or an empty string.
	 *
	 * @param started_at			The start date, in RFC3339 format, used for determining 
	 * 								the aggregation period.
	 * 								Specify this parameter only if you specify the 'period' parameter.
	 * 								The start date is ignored if 'period' is 'all'.
	 *
	 * 								Note that the date is converted to PST before being used, so if
	 * 								you set the start time to 2022-01-01T00:00:00.0Z and 'period' 
	 * 								to 'month', the actual reporting period is December 2021,
	 * 								not January 2022.
	 * 								If you want the reporting period to be January 2022, you must 
	 * 								set the start time to 2022-01-01T00:00:00.0Z or 2022-01-01T00:00:00.0-08:00.
	 *
	 * 								If your start date uses the '+' offset operator (for example
	 * 								2022-01-01T00:00:00.0+05:00), you must URL encode the start date.
	 *
	 * 								It can be NULL or an empty string.
	 *
	 * @param user_id				An ID that identifies a user that cheered bits in the channel.
	 * 								If count is greater than 1, the response may include users ranked
	 *								above and below the specified user.
	 *								To get the leaderboard's top leaders, don't specify a user ID.
	 *								It can be NUL or an empty string.
	 * 								
	 * @param leaderboard			A list if leaderboard leaders.
	 * 								The leaders are returned in rank order by how much they've cheered.
	 * 								The array is empty if nobody has cheered bits.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's Bits leaderboard,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetBitsLeaderboard(int32_t count, const char *period, const char *started_at, const char *user_id, char **leaderboard);

	/**
	 * Gets a list of Cheermotes that users can use to cheer Bits in any Bits-enabled chat room.
	 * Cheermotes are animated emotes that viewers can assign Bits to.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose custom Cheermotes you want to get.
	 *								Specify the braodcaster's ID if you want to include the broadcaster's
	 *								Cheermotes in the response (not all the broadcasters upload Cheermotes).
	 *								If not specified, the response contains only global Cheermotes.
	 *								If the broadcaster uploaded Cheermotes, the "type" field in 
	 *								the response is set to channel_custom.
	 *								It can be NULL or an empty string.
	 *
	 * @param cheermotes			The list of Cheermotes.
	 *
	 * @return						TRUE if successful retrieved the Cheermotes, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCheermotes(const char *broadcaster_id, char **cheermotes);

	/**
	 * Gets an extension's list of transactions.
	 * A transaction records the exchange of a currency (for example, Bits) for a digital product.
	 *
	 * Requires an app access token.
	 *
	 * @param extension_id			The ID of the extension whose list of transactions you want to get.
	 *
	 * @param transaction_id		A transaction ID used to filter the list of transactions.
	 * 								To specify more than one ID, include this parameter for each
	 * 								transaction you want to get separated by |, for example 1234|5678.
	 *								You may specify a maximum of 100 IDs.
	 *								It can be NULL or an empty string.
	 *
	 * @param transactions			The list of transactions.
	 *
	 * @return						TRUE if successfully retrieved the list of transactions,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionTransactions(const char *extension_id, const char *transaction_id, char **transactions);
	// ========================== //
	
	// ========== CHANNELS ========== //
	
	/**
	 * Get information about one or more channels.
	 * 
	 * @param broadcaster_id		The ID of the broadcaster whose channel you want to get.
	 * 								To specify more that one ID, include this parameter for
	 * 								each broadcaster you want to get seprated by |, for
	 * 								example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								The API ignores duplicate IDs and IDs that are not found.			
	 *
	 * @param channel_info			A list that contains information about the specified channels.
	 * 								The list is empty if the specified channels weren't found.
	 * 
	 * @return						TRUE if successfully retrieved the list of channels,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelInfo(const char *broadcaster_id, char **channel_info);

	/**
	 * Update a channel's properties/
	 * Requires a user access token that includes the channel:manage:broadcast scope
	 * 
	 * @param broadcaster_id		The ID of the broadcaster whose channel you want to update.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param game_id				The ID of the game that the user plays.
	 * 								The game is not updated if the ID isn't a game ID that
	 * 								Twitch recognizes.
	 * 								To unset this field, use "0" or an empty string.
	 * 								It can be NULL.
	 *
	 * @param language				The user's preferred language.
	 * 								Set the value to an ISO 639-1 two-letter language code (for
	 * 								example, en for English), set to "other" if the user's
	 * 								preferred language is not a Twitch supported language.
	 * 								The language isn't updated if the language code isn't
	 * 								a Twitch supported language.
	 * 								It can be NULL or an empty string.
	 *
	 * @param title					The title of the user's stream.
	 * 								It can be NULL or an empty string.
	 * 								NOTE: Official docs say that you may not set this field to
	 * 									  an empty string, but usually in this library we use
	 * 									  NULL and empty strings for optional fields if not
	 * 									  specified.
	 *
	 * @param delay					The number of seconds you want your broadcast buffered
	 * 								before streaming is live.
	 * 								The delay helps ensure fairness during competitive play.
	 * 								Only users with Partner status may set this field.
	 * 								The maximum delay is 900 seconds (15 minutes).
	 * 								If set to a negative number it will be ignored.
	 *
	 * @param tags					A list of channel-defined tags to apply to the channel.
	 * 								To remove all tags from the channel, set tags to an empty array.
	 * 								Tags help identify the content that the channel streams.
	 *
	 * 								A channel may specify a maximum of 10 tags.
	 * 								Each tag is limited to a maximum of 25 characters and may not
	 * 								be an empty string or contain spaces or special characters.
	 * 								Tags are case insensitive.
	 * 								For readability, consider using camelCasing  or PascalCasing.
	 * 								It can be NULL.
	 *
	 * @param CCLs					List of labels that should be set as the Channel's CCLs.
	 * 								It can be NULL.
	 *
	 * @param is_branded_content	Flag indicating if the channel has branded content.
	 * 
	 * @return						TRUE if successfully updated the channel's properties, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLModifyChannelInformation(
			const char *broadcaster_id,
			const char *game_id,
			const char *language,
			const char *title,
			int32_t delay,
			const char tags[10][26],
			const FTWLCCLabel CCLs[6],
			BOOL is_branded_content);
	
	/**
	 * Get the broascaster's list editors.
	 * Requires a user access token that includes the channel:read:editors scope
	 * 
	 * @param broadcaster_id		The ID of the broadcaster that owns the channel.
	 * 								This ID must match the user ID in the access token.
	 * 
	 * @param channel_editors		A list of users that are editors for the specified broadcaster.
	 * 								The list is empty if the broadcaster doesn’t have editors.
	 * 
	 * @return						TRUE if successfully retrieved the broadcaster's
	 * 								list of editors, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLGetChannelEditors(const char *broadcaster_id, char **channel_editors);
	
	/**
	 * Gets a list of broadcasters that the specified user follows.
	 * You can also use this function to see whether a user follows a specific broadcaster.
	 * Requires a user access token that includes the user:read:follows scope
	 *
	 * @param user_id				A user's ID.
	 * 								This ID must match the user ID in the user OAuth token
	 * 
	 * @param broadcaster_id		A broadcaster's ID.
	 * 								Use this parameter to see whether the user follows this broadcaster.
	 * 								If specified, the response contains this broadcaster if the user follows them.
	 * 								If NULL, the response contains all broadcasters that the user follows.
	 * 								It can be NULL or an empty string.
	 * 
	 * @param followed				The list of broadcasters that the user follows.
	 * 								The list is in descending order by followed_at (with the most recently followed broadcaster first).
	 * 								The list is empty if the user doesn’t follow anyone.
	 * 
	 * @return						TRUE if successfully retrieved the broadcaster's list
	 * 								of followers, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetFollowedChannels(const char *user_id, const char *broadcaster_id, char **followed);
	
	/**
	 * Gets a list of users that follow the specified broadcaster.
	 * You can also use this function to see whether a specific user follows the broadcaster.
	 * - Requires a user access token that includes the moderator:read:followers scope
	 * - The ID in the broadcaster_id query parameter must match the user ID in the access token 
	 *   or the user ID in the access token must be a moderator for the specified broadcaster.
	 * This function will return specific follower information only if both of the above are true,
	 * otherwise only the total follower count will be included in the response.
	 *
	 * @param broadcaster_id		The broadcaster's ID
	 * 
	 * @param user_id				A user's ID.
	 * 								Use this parameter to see whether the user follows this broadcaster.
	 * 								If specified, the response contains this user if they follow the broadcaster.
	 * 								If NULL, the response contains all users that follow the broadcaster.
	 * 								It can be NULL or an empty string.
	 * 
	 * @param followers				The list of users that follow the specified broadcaster.
	 * 								The list is in descending order by followed_at (with the most recent follower first).
	 * 								The list is empty if nobody follows the broadcaster, the specified 
	 * 								user_id isn’t in the follower list, the user access token is missing 
	 * 								the moderator:read:followers scope, or the user isn’t the broadcaster 
	 * 								or moderator for the channel.
	 * 
	 * @return						TRUE if successfully retrieved the broadcaster's list of followers, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelFollowers(const char *broadcaster_id, const char *user_id, char **followers);
	// ============================== //
	
	// ========== CHANNEL POINTS ========== //
	
	/**
	 * Creates a Custom Rewards in the broadcaster's channel.
	 * The maximum number of custom rewards per channel is 50,
	 * which includes both enabled and disabled rewards.
	 *
	 * Requires a user access token that includes the channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster to add the custom reward to.
	 * 								This ID must match the user ID found in the OAuth token.
	 *
	 * @param title					The custom reward's title.
	 * 								The title may contain a maximum of 45 characters and
	 * 								it must be unique amongst all of the broadcaster's
	 * 								custom rewards.
	 *
	 * @param cost					The cost of the reward, in Channel Points.
	 * 								The minimum is 1 point.
	 *
	 * @param options				Struct with the remaining optional parameters.
	 * 								It can be NULL.
	 *
	 * @param reward				The custom reward you created.
	 *
	 * @return						TRUE if we successfully create a reward, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreateCustomRewards(
			const char *broadcaster_id, 
			const char title[46], 
			const int64_t cost, 
			const FTWLReward *reward_options, 
			char **reward);

	/**
	 * Deletes a custom reward that the broadcaster created.
	 * The app used to create the reward is the only app that may delete it.
	 * If the reward's redemption status is UNFULFILLED at the time the reward is deleted,
	 * its redemption status is marked as FULFILLED.
	 *
	 * Requires a user access token that includes the channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that created the custom reward.
	 * 								This ID must match the user ID found in the OAuth token.
	 *
	 * @param id					The ID of the custom reward to delete.
	 *
	 * @return						TRUE if we successfully delete the custom reward, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteCustomReward(const char *broadcaster_id, const char *id);
	
	/**
	 * Gets a list of custom rewards that the specified broadcaster created.
	 * NOTE:	A channel may offer a maximum of 50 rewards, which includes
	 * 			both enabled and disabled rewards.
	 *
	 * Requires a user access token that includes the channel:read:redemptions or
	 * channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose custom rewards you want to get.
	 * 								This ID must match the user ID found in the OAuth token.
	 *
	 * @param id					A list of IDs to filter the rewards by.
	 * 								To specify more than one ID, include this parameter for each
	 * 								reward you want to get separated by |, for example 1234|56768.
	 * 								You may specify a maximum of 50 IDs.
	 *
	 * 								Duplicates ID are ignored.
	 * 								The response contains only he IDs that were found, id none
	 * 								of the IDs were found, the response is 404 Not Found and 
	 * 								the function return FALSE.
	 * 								It can be NULL or an empty string.
	 *
	 * @param only_manageable_rewards	A boolean value that determines whether the response contains
	 * 									only the custom rewards that the app may manage (the app is 
	 * 									identified by the ID in the Client-id header).
	 * 									Set to TRUE to get only the custom rewards that the app
	 * 									may manage.
	 *
	 * @param custom_rewards		A list of custom rewards.
	 * 								The list is in ascending order by id.
	 *								If the broadcaster hasn't created custom rewards, the list is empty.
	 *
	 * @return						TRUE if we successfully retrevied the broadcaster's list of suctom rewards
	 * 								(even if empty), FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCustomReward(const char *broadcaster_id, const char *id, BOOL only_manageable_rewards, char **custom_rewards);
	
	/**
	 * Gets a list of redemptions for the specified custom reward.
	 * The app used to create the reward is the only app that may get the redemptions.
	 *
	 * Requires a user access token that includes the channel:read:redemptions or 
	 * channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the custom reward.
	 * 								This ID must match the user ID found in the user OAuth token.
	 *
	 * @param reward_id				The ID that identifies the custom reward whose redemptions you
	 * 								want to get.
	 *
	 * @param status				The status of the redemptions to return.
	 * 								The possible case-sensitive values are:
	 * 									* CANCELED
	 * 									* FULFILLED
	 * 									* UNFULFILLED
	 * 								
	 * 								NOTE:	This field is required only if you don't specify
	 * 										the 'id' query parameter (look below).
	 *
	 * 								NOTE: 	Canceled and fulfilled redemptions are returned
	 * 										for only a few days after they're canceled or fulfilled.
	 *
	 * @param id					A list of IDs to filter the redemptions by.
	 * 								To specify more than one ID, include this parameter for each
	 * 								redemption you want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 50 IDs.
	 * 								
	 * 								Duplicate IDs are ignored.
	 * 								The response contains only the IDs that were found, if none of
	 * 								the IDs were found, the response is 404 Not Found.
	 * 								It can be NULL or an empty string.
	 *
	 * @param sort					The order to sort redemptions by.
	 * 								The possible case-sensitive values are:
	 * 									* OLDEST
	 * 									* NEWEST
	 * 								The default is OLDEST.
	 * 								It can be NULL or an empty string.
	 *
	 * @param redemptions			The list of redemptions for the specified reward.
	 * 								The list is empty if there are no redemptions that 
	 * 								match the redemption criteria.
	 *
	 * @return						TRUE if we successfully retrevied the list of redeemed custom
	 * 								rewards (even if empty), FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCustomRewardRedemption(
			const char *broadcaster_id,
			const char *reward_id,
			const char *status,
			const char *id,
			const char *sort,
			char **redemptions);

	/**
	 * Updates a custom reward.
	 * The app used to create the reward is the only app that may update the reward.
	 *
	 * Requires a user access token that includes the channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's updating the reward.
	 * 								This ID must match the user ID found in the OAuth token.
	 *
	 * @param id					The ID of the reward to update.
	 *
	 * @param reward				Struct containing the fields you are updating.
	 * 								WARNING: the function send all the fields so you must update
	 * 										 all of them, even if they are not changed.
	 *
	 * @param updated_reward		The reward that you updated.
	 *
	 * @return						TRUE if we successfully update the reward, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateCustomReward(const char *broadcaster_id, const char *id, const FTWLReward *reward, char **updated_reward);
	
	/**
	 * Updates a redemption's status.
	 * You may update a redemption only if its status is UNFULFILLED.
	 * The app used to create the reward is the only app that may update the redemption.
	 *
	 * Requires a user acces token that includes the channel:manage:redemptions scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's updating the redemption.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param reward_id				The ID that identifies the reward that's been redeemed.
	 *
	 * @param id					A list of IDs that identify the redemptions to update.
	 * 								To specify more than one ID, include this parameter for each redemption
	 * 								you want to update separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 50 IDs.
	 *
	 * @param status				The status to set the redemption to.
	 * 								Possible values are:
	 * 									* CANCELED
	 * 									* FULFILLED
	 *
	 * 								Setting the status to CANCELED refunds the user's channel points.
	 *
	 * @param updated_redemption	The redemption that you updated.
	 *
	 * @return						TRUE if we successfully update the redemption status, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateRedemptionStatus(
			const char *broadcaster_id,
			const char *reward_id,
			const char *id,
			const char *status,
			char **updated_redemption);
	// ==================================== //

	// ========== CHARITY ========== //
	
	/**
	 * Gets information about the charity campaign that a broadcaster is running.
	 * For example, the campaign's fundraising goal and the current amount of donations.
	 * To receive events when progress is made towards the campaign's goal or the broadcaster 
	 * changes the fundraising goalm subscribe to channel.charity_campaign.progress
	 * subscription type.
	 *
	 * Requires a user access token that includes the channel:read:charity scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's currently running
	 * 								a charity campaign.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param campaign				The charity campaign that the broadcaster is currently running.
	 * 								The list is empty if the boradcaster is not running a charity
	 * 								campaign, the campaign information is not available after
	 * 								the campaign ends.
	 *
	 * @return						TRUE if we successfully retrevied information about the
	 * 								broadcaster's active charity campaign, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCharityCampaign(const char *broadcaster_id, char **campaign);
	
	/**
	 * Gets the list of donations that users have made to the broadcaster's active charity campaign.
	 * To receive events as donation occur, subscribe to the channel.charity_campaing.donate
	 * subscription type.
	 *
	 * Requires a user access token that includes the channel:read:charity scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's currently running a
	 * 								charity campaign.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param donations				A list of the donations that users have made to the
	 * 								broadcaster's charity campaign.
	 * 								The list is empty if the broadcaster is not currently
	 * 								running a charity campaign, the donation information is
	 * 								not available after the campaign ends.
	 *
	 * @return						TRUE if we successfully get a list of donations (even if empty),
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCharityCampaignDonations(const char *broadcaster_id, char **donations);
	// ============================= //

	// ========== CHAT ========== //
	
	/**
	 * Gets the list of users that are connected to the broadcaster's chat session.
	 * Requires a user access token that includes the moderator:read:chatters scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose list of chatters you want to get.
	 * 
	 * @param moderator_id			The ID of the broadcaster or one of the broadcaster's moderators.
	 * 								This ID must match the user ID in the user access token.
	 * 
	 * @param chatters				The list of users that are connected to the broadcaster's chat room.
	 * 								The list is empty if no users are connected to the chat room.
	 *
	 * @return						TRUE if successfully retrieved the broaddcater's list of chatters, 
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChatters(const char *broadcaster_id, const char *moderator_id, char **chatters);

	/**
	 * Gets the broadcaster's list of custom emotes.
	 * Broadcasters create these custom emotes for users who subscribe to or follow
	 * the channel or cheers Bits in the channel's chat window.
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		An ID that identifies the broadcaster whose emotes you want to get.
	 * 
	 * @param emotes				The lsit if the emotes that the specified braodcaster created.
	 * 								if the broadcaster hasn't created custom emotes, the list is empty.
	 *
	 * @return						TRUE if successfully retrieved broadcaster's list if custom emotes,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelEmotes(const char *broadcaster_id, char **emotes);

	/**
	 * Gets the list of global emotes.
	 * Global emotes are Twitch created emotes that users can use in any Twitch chat.
	 * Requires an app access token or user access token.
	 *
	 * @param emotes				The list of global emotes.
	 *
	 * @return						TRUE if successfully retrieved Twitch's list of global emotes,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGlobalEmotes(char **emotes);

	/**
	 * Gets emotes for one or more specified emote sets.
	 * An emote set groups emotes that have a similar context. For example, Twitch places
	 * all the subscriber emotes that a broadcaster uploads for their channel in the same meote set.
	 * Requires an app access token or user access token.
	 *
	 * @param emote_set_id			An ID that identifies the emote set to get.
	 * 								Include this parameter for each emote set you want
	 * 								to get separated by |.
	 * 								For example 12345|6789|93726, you may specify a
	 * 								maximum of 25 IDs.
	 * 								The response contains only the IDs that were found
	 * 								and ignores duplicate IDs.
	 *
	 * 								To get emote set IDs, use the TWLGetChannelEmotes function.
	 *
	 * @param emote_sets			The list of emotes found in the specified emote sets. 
	 * 								The list is empty if none of the IDs were found. 
	 * 								The list is in the same order as the set IDs specified in the request. 
	 * 								Each set contains one or more emoticons.
	 *
	 * @return						TRUE if successfully retrieved the emotes for the specified emote sets,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetEmoteSets(const char *emote_set_id, char **emote_sets);

	/**
	 * Gets the broadcaster's list of custom chat badges.
	 * The list is empty if the broadcaster hasn't created custom chat badges.
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat badges you want to get.
	 * 
	 * @param badges				The list of chat badges.
	 * 								The list is sorted in ascending order by set_id, and within
	 * 								a set, the list is sorted in ascending order by id.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's custom chat badges,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelChatBadges(const char *broadcaster_id, char **badges);

	/**
	 * Gets Twitch's list if chat badges, which users may use in any channels char room.
	 * Requires an app access token or user access token
	 *
	 * @param badges				The list of chat badges.
	 * 								The list is sorted in ascending order by set_is, and within
	 * 								a set, the list is sorted in ascending order by id.
	 *
	 * @return						TRUE if successfully retrieved the list of global chat badges, 
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGlobalChatBadges(char **badges);

	/**
	 * Gets the broadcaster's chat setting.
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat settings you want to get.
	 * 
	 * @param moderator_id			The ID of the broadcaster or one of the broadcaster's moderators.
	 * 								
	 * 								This field is required only if you want to include the 
	 * 								non_moderator_chat_delay and non_moderator_chat_delay_duration
	 * 								settings in the response.
	 *
	 * 								If you specify this field, this ID must match the user ID
	 * 								in the user access token.
	 * 								It can be NULL or an empty string.
	 *
	 * @param chat_settings			The list of chat settings.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's chat settings,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChatSettings(const char *broadcaster_id, const char *moderator_id, char **chat_settings);

	/**
	 * Retrevies the active shared chat session for a channel.
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The User ID of the channel broadcaster.
	 * 
	 * @param chat_session			The shared chat session.
	 *
	 * @return						TRUE if successfully retrieved the shared chat session.
	 * 								Returns an empty array if the broadcaster_id in the request
	 * 								isn't in a shared session, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetSharedChatSession(const char *broadcaster_id, char **chat_session);

	/**
	 * Retrievies emotes available to the user across all channels.
	 * 
	 * Requires a user access token tha includes the user:read:emotes scope.
	 * Parameter user_id must match the user_id in the user access token.
	 *
	 * @param user_id				The ID of the user.
	 * 
	 * @param broadcaster_id		The User ID of a broadcaster you wish to get follower emotes of.
	 * 								Using this parameter will guarantee inclusion of the broadcaster's
	 * 								follower emotes in the response body.
	 *
	 * 								NOTE: 	If the user specified in user_id is subscribed to the
	 * 										broadcaster specified, their follower emotes will appear
	 * 										in the response body regardless if this parameter is used.
	 * 								It can be NULL or an empty string.
	 *
	 * @param emotes				The list of emotes.
	 *
	 * @return						TRUE if successfully retrieved the emotes, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUserEmotes(const char *user_id, const char *broadcaster_id, char **emotes);

	/**
	 * Updates the broadcaster's chat settings.
	 * 
	 * Requires a user access token that includes the moderator:manage:chat_settings scope.
	 *
	 * To set the slow_mode_wait_time, follower_mode_duration, or non_moderator_chat_delay_duration 
	 * field’s value, you must set the corresponding slow_mode, follower_mode, 
	 * or non_moderator_chat_delay field to TRUE.
	 *
	 * To set the slow_mode_wait_time or follower_mode_duration field to its default value, 
	 * set the corresponding slow_mode or follower_mode field to TRUE 
	 * (and the slow_mode_wait_time or follower_mode_duration field to a NEGATIVE value).
	 *
	 * To remove the slow_mode_wait_time, follower_mode_duration, or non_moderator_chat_delay_duration 
	 * field’s value, set the corresponding slow_mode, follower_mode, or non_moderator_chat_delay field 
	 * to FALSE.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat settings you want to update.
	 * 
	 * @param moderator_id			The ID of a user that has permission to moderate the broadcaster's
	 * 								chat room, or the broadcaster's ID if they're making the update.
	 * 								This ID must match the user ID in the user access token.
	 * 
	 * @param settings				Struct containing the fields you are updating.
	 * 								WARNING: the function send all the fields so you must update
	 * 										 all of them, even if they are not changed.
	 *
	 * @param updated_settings		The list of chat settings.
	 * 								The list contain a single object with all the settings.
	 *
	 * @return						TRUE if successfully updated the broadcaster's chat settings, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateChatSettings(const char *broadcaster_id, const char *moderator_id, const FTWLChatSettings *settings, char **updated_settings);

	/**
	 * Sends an announcement to the broadcaster's chat room.
	 * 
	 * RATE LIMITS: One announcement may be sent every 2 seconds.
	 * 
	 * Requires a user access toekn that includes the moderator:manage:announcements scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the chat room to send the announcement to.
	 * 
	 * @param moderator_id			The ID of a user who has permission to moderate the broadcaster's chat room,
	 * 								or the broadcaster's ID if they're sending the announcement. 
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param message				The announcement to make in the broadcaster's chat room.
	 * 								Announcements are limited to a maximum of 500 characters,
	 * 								longer than 500 characters are truncated.
	 *
	 * @param color					The color used to highlight the announcement.
	 * 								Possible case-sensitive values are:
	 * 									* blue
	 * 									* green
	 * 									* orange
	 * 									* purple
	 * 									* primary (default)
	 *
	 * 								If color is set to 'primary' or is not set, the channel's
	 * 								accent color is used to highlight the announcement.
	 * 								It can be NULL or an empty string.
	 *
	 * @return 						TRUE if successfully sent the announcement, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendChatAnnouncement(
			const char *broadcaster_id,
			const char *moderator_id,
			const char announcement[501],
			const char *color);

	/**
	 * Sends a Shoutout to the specified broadcaster.
	 * Tipically, you send Shoutouts when you or one of your moderators notice another broadcaster in your chat,
	 * the other broadcaster is coming up in conversation, or after they raid your broadcast.
	 *
	 * Twitch's Shoutout feature is a great way for you to show support for other broadcasters and help them grow.
	 * Viewers who do not follow the other broadcaster will see a pop=up Follow button in your chat that
	 * they can click to follow the other broadcaster.
	 *
	 * RATE LIMITS: The broadcaster may send a Shoutout once every 2 minutes. They may send the same
	 * 				broadcaster a Shoutout once every 60 minutes.
	 *
	 * Requires a user access token that includes the moderator:manage:shoutouts scope.
	 *
	 * @param from_broadcaster_id	The ID of the broadcaster that's sending the Shoutout.
	 * 
	 * @param to_broadcaster_id		The ID of the broadcaster that's receiving the Shoutout.
	 * 
	 * @param moderator_id			The ID of the broadcaster or a user that is one of the broadcaster's moderators.
	 * 								This ID must match the user ID in the access token.
	 * 
	 * @return						TRUE if successfully sent the specified broadcaster a Shoutout, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendShoutout(const char *from_broadcaster_id, const char *to_broadcaster_id, const char *moderator_id);

	/**
	 * Sends a message to the broadcaster's chat room.
	 * 
	 * NOTE: 	When sending messages to a Shared Chat session, behaviors differ depending on your
	 * 			authentication token type:
	 * 		
	 * 			- When using an app access token, messages will only be sent to the source channel
	 * 			  (defined by the broadcaster_id parameter) by default.
	 * 			  Messages can be sent to all channels by using the for_source_only parameter and
	 * 			  setting it to FALSE.
	 *
	 * 			- When using an user acces token, messages will be sent to all channels in the shared
	 * 			  chat session, including the source channel.
	 * 			  This behavior CANNOT BE CHANGED with this token type.
	 *        
	 * Requires an app access token or user access token that includes the user:write:chat scope.
	 * If app access token used, then additionally requires user:bot scope from chatting user, and either
	 * channel:bot scope from broadcaster or moderator status.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat room the message will be sent to.
	 * 
	 * @param sender_id				The ID of the user sending the message.
	 * 								This ID must match the user ID in the user access token.
	 * 
	 * @param message				The message to send.
	 * 								The message is limited to a maximum of 500 characters. Chat messages can also
	 * 								inlude emoticons, use the name of the emote.
	 * 								The names are case sensitive.
	 * 								Don't include colons around the name (e.g.,:bleedPurple:).
	 * 								If Twitch recognizes the name, Twitch converts the name to the emote before
	 * 								writing the chat message to the chat room.
	 *
	 * @param reply_parent_message_id	The ID of the chat message being replied to.
	 *
	 * @param for_source_only		NOTE: This parameter can only be set when utilizing an App Access Token.
	 * 								It cannot be specified when a User Access Token is used, and will
	 * 								instead result in an HTTP 400 error and function return FALSE.
	 *
	 * 								If you are using a user access token set it to 0.
	 *
	 * 								Determines if the chat message is sent only to the source channel
	 * 								(define ny the broadcaster_id) during shared chat session.
	 * 								This has no effect if the message is sent during a shared chat session.
	 *
	 * 								If this parameter is not set, the default value when using an 
	 * 								App Access Token is FALSE.
	 * 								On May 19, 2025 the default value for this parameter will be updated
	 * 								to TRUE, and chat messages sent using an App Access Token will only be
	 * 								shared with the source channel by default.
	 * 								If you prefer to send a chat message to both channels in a shared 
	 * 								chat session, make sure this parameter is explicity set to FALSE
	 * 								before May 19.
	 * 								
	 * 								IMPORTANT: we assume the seguent values meaning:
	 * 									1 = TRUE
	 * 									0 = ignored because we are using an user access token
	 * 								   -1 = FALSE
	 *
	 * @param sent_message			The message we just sent.
	 *
	 * @return 						TRUE if successfully sent the specified broadcaster a message, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendChatMessage(
			const char *broadcaster_id,
			const char *sender_id,
			const char message[501],
			const char *reply_parent_message_id,
			int32_t for_source_only,
			char **sent_message);

	/**
	 * Gets the color used for the user's name in chat.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param user_id				The ID of the user whose username color you want to get.
	 * 								To specify more than one user, include the user id parameter
	 * 								for each user to get separated by |, for example 1234|3445|5334.
	 * 								The maximum number of IDs that you may specify is 100.
	 * 								The API ignores duplicate IDs and IDs that weren't found.
	 * 
	 * @param color					The list of users and the color code they use for their name.
	 *
	 * @return						TRUE if successfully retrieved the chat color used
	 * 								by the specified users, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUserChatColor(const char *user_id, char **color);

	/**
	 * Updates the color used for the user's name in chat.
	 * 
	 * Requires a user access token that includes the user:manage:chat_color scope.
	 *
	 * @param user_id				The ID of the user whose chat color you want to update.
	 * 								This ID must match the user ID in the access token.
	 * 
	 * @param color					The color to use for the user's name in chat.
	 * 								All users may specify one of the following named color values:
	 * 								blue, vlue_violet, cadet_blue, chocolate, coral, dodger_blue, firebrick,
	 * 								golden_rod, green, hot_pink, orange_red, red, sea_green, spring_green, yellow_green.
	 * 								Turbo and Prime users may specify a named color or a Hex color code like #9146FF.
	 * 								If you use a Hex color code, remember to URL encode it.
	 * 
	 * @return						TRUE if successfully updated the user's chat color, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateUserChatColor(const char *user_id, const char *color);
	// ========================== //
	
	// ========== CLIPS ========== //
	
	/**
	 * Creates a clip from the boradcaster's stream.
	 * This API captures up to 90 seconds of the broadcaster’s stream.
	 * The 90 seconds spans the point in the stream from when you called the API.
	 * For example, if you call the API at the 4:00 minute mark, the API captures from 
	 * approximately the 3:35 mark to approximately the 4:05 minute mark. 
	 * Twitch tries its best to capture 90 seconds of the stream, but the actual length may be less. 
	 * This may occur if you begin capturing the clip near the beginning or end of the stream.
	 *
	 * By default, Twitch publishes up to the last 30 seconds of the 90 seconds window and provides a default title for the clip.
	 * To specify the title and the portion of the 90 seconds window that’s used for the clip, use the URL in the response’s edit_url field.
	 * You can specify a clip that’s from 5 seconds to 60 seconds in length.
	 * The URL is valid for up to 24 hours or until the clip is published, whichever comes first.
	 *
	 * Creating a clip is an asynchronous process that can take a short amount of time to complete.
	 * To determine whether the clip was successfully created, call TWLGetClips using the clip ID that this request returned.
	 * If TWLGetClips returns the clip, the clip was successfully created.
	 * If after 15 seconds Get Clips hasn’t returned the clip, assume it failed.
	 *
	 * Requires a user access token that includes the clips:edit scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose stream you want to create a clip from.
	 * 
	 * @param has_delay				A boolean value that determines whether the API captures the clip at
	 * 								the moment the viewwer requests it or after a delay.
	 * 								If FALSE, Twitch captures the clip at the moment the viewer requests it,
	 * 								this is the same as the clip experience as the Twitch UX.
	 * 
	 * @param clip					The new created clip (id and edit url).
	 * 
	 * @return						TRUE if successfully started the clip process, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreateClip(const char *broadcaster_id, BOOL has_delay, char **clip);

	/**
	 * Gets one or more video clips that were captured from streams.
	 * 
	 * When useing pagination for clips, note that the maximum number of results returned over multiple requests
	 * will be approximately 1000.
	 * If additional results are necessary, paginate over different query paramters such as multiple "started_at"
	 * and "ended_at" timeframes to refine the search.
	 *
	 * Requires an app access token or user access token.
	 *
	 * The broadcaster_id, game_id and clip_id params are mutually exclusive.
	 *
	 * @param broadcaster_id		An ID that indentifies the broadcaster whose video clips you want to get.
	 * 								Use this parameter to get clips that were captured from the broadcaster's streams.
	 * 
	 * @param game_id				An ID that identifies the game whose clips you want to get.
	 * 								Use this parameter to get clips that were captured from streams that were
	 * 								playing this game.
	 * 
	 * @param clip_id				An ID that identifies the clip to get.
	 * 								To specify more than one ID, include this paramter for each clip you want to get
	 * 								separated by |, for example foo|bar.
	 * 								You may specify a maximum of 100 IDs.
	 * 								The API ignores duplicate IDs and IDs that aren't found.
	 *
	 * @param started_at			The start date used to filter the clips.
	 * 								The API returns only clips within the start and end date window.
	 * 								Specify the date and time in RFC3339 format.
	 * 								It can be NULL or an empty string.
	 *
	 * @param ended_at				The end date used to filter clips.
	 * 								If not specified, the time window is the start date plus one week.
	 * 								Specify the date and time in RFC3339 format.
	 * 								It can be NULL or an empty string.
	 *
	 * @param is_featured			A boolean value that determines whether the response includes featured clips.
	 * 								If > 0, returns only clips that are featured.
	 * 								If < 0, returns only clips that aren't featured.
	 * 								If 0 All clips are returned.
	 * 
	 * @param clips					The list of video clips.
	 * 								For clips returned by game_id or broadcaster_id, the list is in
	 * 								descending order by view count.
	 * 								For lists returned by clip_id, the list is in the same order as the input IDs.
	 * 
	 * @return						TRUE if successfully retrieved the list of video clips, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetClips(
			const char *broadcaster_id,
			const char *game_id,
			const char *clip_id,
			const char *started_at,
			const char *ended_at,
			int32_t is_featured,
			char **clips);
	// =========================== //

	// ========== CONDUITS ========== //
	
	/**
	 * Gets the conduits for a client ID.
	 *
	 * Requires an app access token.
	 *
	 * @param conduits				List of information about clinet's conduits.
	 *
	 * @return						TRUE if we successfully retrevied conduits, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetConduits(char **conduits);

	/**
	 * Creates a new conduit.
	 *
	 * Requires an app access token.
	 *
	 * @param shard_count			The number of shards to create for this conduit.
	 *
	 * @param conduit				Information about the client's conduit.
	 *
	 * @return						TRUE if the conduit has been created, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreateConduits(int32_t shard_count, char **conduit);

	/**
	 * Update a conduit's shard count.
	 * To delete shards, update the count to a lower number, and the shards above the count will be deleted.
	 * For example, if the existing shard count is 100, by resetting shard count to 50, shards 50-99 are disabled.
	 *
	 * Requires an app access token.
	 *
	 * @param id					Conduit ID.
	 *
	 * @param shard_count			The number of shards for this conduit.
	 *
	 * @param updated_conduit		Information about the updated conduit.
	 *
	 * @return						TRUE if successfully updated, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateConduits(const char *id, int32_t shard_count, char **updated_conduit);

	/**
	 * Deletes a specified conduit.
	 * Note that it may take some time for Eventsub subscriptions on a deleted
	 * conduit to show as disabled when calling TWLGetEventsubSubscriptions.
	 *
	 * Requires an app access token.
	 *
	 * @param id					The ID of the conduit we want to delete.
	 *
	 * @return						TRUE if successfully deleted the conduit, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteConduit(const char *id);

	/**
	 * Gets a lists of all shards for a conduit.
	 *
	 * Requires an app access token.
	 *
	 * @param conduit_id			Conduit ID.
	 *
	 * @param status				Status to filter by.
	 * 								It can be NULL or an empty string.
	 *
	 * @param shards				List of conduit's shards.
	 *
	 * @return						TRUE if successfully retrieved shards, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetConduitShards(const char *conduit_id, const char *status, char **shards);
	
	/**
	 * Updates shard(s) for a conduit.
	 *
	 * NOTE:	Shard IDs are indexed starting at 0, so a conduit with a shard_count of 5
	 * 			will have shards with IDs 0 through 4.
	 *
	 * Requires an app access toekn.
	 *
	 * @param conduit_id			Conduit ID.
	 *
	 * @param shards				List of shards to update, as per documentation the
	 * 								max number of shards per conduit is 20k.
	 * 								https://dev.twitch.tv/docs/eventsub/handling-conduit-events/
	 *
	 * @param updated_shards		List of successfully update shards.
	 *
	 * @return						TRUE if successfully updated shards, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateConduitShards(const char *conduit_id, const FTWLShard *shards, char **updated_shards);
	// ============================== //

	// ========== CCLs ========== //
	
	/**
	 * Gets information about Twitch content classification labels.
	 *
	 * Require an app access token or user access token.
	 * 
	 * @param locale				Locale for the Content Classification Labels.
	 * 								You may specify a maximum of 1 locale, default is "en-US".
	 * 								Supported locales:
	 * 								"bg-BG", "cs-CZ", "da-DK", "da-DK", "de-DE", "el-GR",
	 * 								"en-GB", "en-US", "es-ES", "es-MX", "fi-FI", "fr-FR",
	 * 								"hu-HU", "it-IT", "ja-JP", "ko-KR", "nl-NL", "no-NO",
	 * 								"pl-PL", "pt-BT", "pt-PT", "ro-RO", "ru-RU", "sk-SK",
	 * 								"sv-SE", "th-TH", "tr-TR", "vi-VN",	"zh-CN", "zh-TW"
	 * 								It can be NULL or an empty string, in this case 
	 * 								it will be set to "en-US."
	 * 
	 * @param ccls					A list that contains information about the available content
	 * 								classification labels.
	 * 
	 * @return						TRUE if we successfully get the list of CCLs, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetContentClassificationLabels(const char *locale, char **ccls);
	// ========================== //

	// ========== ENTITLEMENTS ========== //
	
	/**
	 * Gets am organization's list of entitlements that have been granted to a game, a user or both.
	 *
	 * NOTE:	Entitlements returned in the response body data are not guaranteed to
	 * 			be sorted by any field returned by the API.
	 * 			To retrieve CLAIMED or FULFILLED entitlements, use the fulfillment_status
	 * 			query parameter to filter results.
	 * 			To retrieve entitlements for a specific game, use the game_id query parameter
	 * 			to filter results
	 *
	 * 			The following table identifies the request parameters that you may specify
	 * 			based on the type of access token used.
	 *
	 * 			ACCESS TOKEN TYPE	PARAMETER	DESCRIPTION
	 * 			App					None		If you don't specify request parameters, the request
	 * 											returns all entitlements that your organization owns.
	 *
	 * 			App					user_id		The request returns all entitlements for any game that
	 * 											the organization granted to the specified user.
	 *
	 * 			App					user_id,	The request returns all entitlements that the specified
	 * 								game_id		game granted to the specified user.
	 *
	 * 			App					game_id		The request returns all entitlements that the specified
	 * 											game granted to all entitled users.
	 *
	 * 			User				None		If you don't specify request paramters, the request
	 * 											returns all entitlements for any game that the organization
	 * 											granted to the user identified in the access token.
	 *
	 * 			User				user_id		Invalid.
	 *
	 * 			User				user_id,	Invalid.
	 * 								game_id
	 *
	 * 			User				game_id		The request returns all the entitlements that the specified game
	 * 											granted to the user identified in the access token.
	 *
	 * Requires an app access token or user access token.
	 * The Client ID associated with the access token must be owned by a user who
	 * is a member of the organization that holds ownership of the game.
	 *
	 * @param id					An ID that identifies the entitlement to get.
	 * 								Include this parameter for each entitlement you
	 * 								want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param user_id				An ID that identifies a user that was granted entitlements.
	 * 								It can be NULL or an empty string.
	 *
	 * @param game_id				An ID that identifies a game that offered entitlements.
	 * 								It can be NULL or an empty string.
	 *
	 * @param fulfillment_status	The entitlement's fulfillment status.
	 * 								Used to filter the list to only those with the specified status.
	 * 								Possible values are:
	 * 									* CLAIMED
	 * 									* FULFILLED
	 * 								It can be NULL or an empty string.
	 *
	 * @param entitlements			The list of entitlements.
	 *
	 * @return						TRUE if successfully retrieved the entitlements, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetDropsEntitlements(
			const char *id,
			const char *user_id,
			const char *game_id,
			const char *fulfillment_status,
			char **entitlements);

	/**
	 * Updates the Drop entitlement's fulfillment status.
	 *
	 * The following table identifies which entitlements are updated based
	 * on the type of access token used.
	 *
	 * 		ACCESS TOKEN TYPE	DATA THAT'S UPDATED
	 * 		App					Updates all entitlements with benefits owned by the organization
	 * 							int the access token.
	 *
	 * 		User				Updates all entitlements owned by the user in the access token
	 * 							and where the benefits are owned by the organization 
	 * 							in the access token.
	 *
	 * Requires an app access token or user access token.
	 * The Client ID associated with the access token must be owned by a user
	 * who is a member of the organization that holds ownership of the game.
	 *
	 * @param entitlement_ids		A list of IDs that identify the entitlements to update.
	 * 								You may specify a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param fulfillment_status	The fulfillment status to set the entitlements to.
	 * 								Possible values are:
	 * 									* CLAIMED 	- The user claimed the benefit.
	 * 									* FULFILLED	- The developer granted the benefit
	 * 												  that the user claimed.
	 * 								It can be NULL or an empty string.
	 *
	 * @param entitlements			A list that indicates which entitlements were successfully
	 * 								updated and those that weren't.
	 *
	 * @return						TRUE if successfully requested the updates. 
	 * 								Check the response to determine which updates succeeded,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateDropsEntitlements(const char *entitlement_ids, const char *fulfillment_status, char **entitlements);
	// ================================== //

	// ========== EXTENSIONS ========== //
	
	/**
	 * Gets the specified configuration segment from the specified extension.
	 *
	 * RATE LIMITS: You may retrieve each segment a maximum of 20 times per minute.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param extension_id			The ID of the extension that contains the configuration segment
	 * 								you want to get.
	 *
	 * @param segment				The type of configuration segment to get.
	 * 								Possible case-sensitive values are:
	 * 									* broadcaster
	 * 									* developer
	 * 									* global
	 *
	 * 								You may specify one or more segments.
	 * 								To specify multiple segments, include this parameter for each
	 * 								segment to get separated by |, for example "broadcaster|developer".
	 * 								Ignores duplicate segments.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that installed the extension.
	 * 								This parameter is required if you set the 'segmen' parameter
	 * 								to 'broadcaster' or 'developer'.
	 * 								Do not specify this parameter if you set 'segment' to 'global',
	 * 								in this case it can be NULL or an empty string.
	 *
	 * @param segments				The list of requested configuration segments.
	 * 								The list is returned in the same order that you specified
	 * 								the list of segments in the request.
	 *
	 * @return						TRUE if successfully retrevied the configurations, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionConfigurationSegment(
			const char *extension_id,
			const char *segment,
			const char *broadcaster_id,
			char **configuration_segments);

	/**
	 * Updates a configuration segment.
	 * The segment is limited to 5 KB.
	 * Extensions that are active on a channel do not receive the updated configuration.
	 *
	 * RATE LIMITS: You may update the configuration a maximum of 20 times per minute.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param extension_id			The ID of the extension to update.
	 *
	 * @param segment				The configuration segment to update.
	 * 								Possible case-sensitive values are:
	 * 									* broadcaster
	 * 									* developer
	 * 									* global
	 *
	 * @param braodcaster_id		The ID of the braodcaster that installed the extension.
	 * 								Include this field only if the 'segment' is set to 
	 * 								developer or braodcaster.
	 * 								It can be NULL or an empty string.
	 *
	 * @param content				The contents of the segment.
	 * 								The string may be a plain-text string or a string-encoded
	 * 								JSON object.
	 * 								It can be NULL or an empty string.
	 *
	 * @param version				The version number that identifies this definition of the
	 * 								segment's data.
	 * 								If not specified, the latest definition is updated.
	 * 								It can be NULL or an empty string.
	 *
	 * @return						TRUE if successfully updated the configuration, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSetExtensionConfigurationSegment(
			const char *extension_id,
			const char *segment,
			const char *broadcaster_id,
			const char *content,
			const char *version);

	/**
	 * Update the extension's required_configuration string.
	 * Use this function if your extension requires the broadcaster to configure the
	 * extension before activating it (to require configuration, you must select
	 * Custom/My Own Service in Extension Capabilities).
	 * For more information see:
	 * https://dev.twitch.tv/docs/extensions/life-cycle/#capabilities
	 * https://dev.twitch.tv/docs/extensions/building/#required-configurations
	 * https://dev.twitch.tv/docs/extensions/building/#setting-required-configuration-with-the-configuration-service-optional
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external' and the 'user_id' field to the ID of the user that owns the extension.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that installed the
	 * 								extension on their channel.
	 *
	 * @param extension_id			The ID of the extension to update.
	 *
	 * @param extension_version		The version of the extension to update.
	 *
	 * @param required_configuration The required_configuration string to user with the extension.
	 *
	 * @return						TRUE if successfully updated the extension's
	 * 								required_configuration string, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSetExtensionRequiredConfiguration(
			const char *broadcaster_id,
			const char *extension_id,
			const char *extension_version,
			const char *required_configuration);

	/**
	 * PUBSUB IS DECOMMISIONED
	 */
	TWL_API BOOL TWLSendExtensionPubSubMessage();

	/**
	 * Gets a list of broadcasters that are streaming live and have installed 
	 * or activated the extension.
	 *
	 * It may take a few minutes for the list to include or remove broadcasters
	 * that have recently gone live or stopped broadcasting.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param extension_id			The ID of the extension to get.
	 *
	 * @param channels				The list of broadcasters that are live and that
	 * 								have installed or activated this extension.
	 *
	 * @return						TRUE if successfully retrieved the list of
	 * 								broadcasters, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionLiveChannels(const char *extension_id, char **channels);

	/**
	 * Gets an extension's list of shared secrets.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param extension_id			The ID of the extension whose shared secrets you want to get.
	 *
	 * @param shared_secrets		The list of shared secrets that the extension created.
	 *
	 * @return						TRUE if successfully retrieved the list of secrets, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionSecrets(const char *extension_id, char **shared_secrets);

	/**
	 * Creates a shared secret used to sign and verify JWT tokens.
	 * Creating a new secret removes the current secrets from service.
	 * Use this function only when you are ready to use the new secret it returns.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param extension_id			The ID of the extension to apply the shared secret to.
	 *
	 * @param delay					The amount of time, in seconds, to delay activating the secret.
	 * 								The delay should provide enough time for instances of the extension
	 * 								to gracefully switch over the new secret.
	 * 								The minimum (and default) delay is 300 seconds (5 minutes), the value
	 * 								will be clamped to 300 if it is less then 300.
	 * 								
	 * @param new_secrets			A list that contains the newly added secrets.
	 *
	 * @return						TRUE if successfully created the new secret, FALSE otherwise.
	 * 								
	 */
	TWL_API BOOL TWLCreateExtensionSecret(const char *extension_id, int32_t delay, char **new_secrets);

	/**
	 * Sends a message to the specifiedbroadcaster's chat room.
	 * The extension's name is used as the username for the message in the chat room.
	 * To send a chat message, your extension must enable Chat Capabilities (under your
	 * extension's Capabilities tab).
	 *
	 * RATE LIMITS: You may send a maximum of 12 messages per minute per channel.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that has activated the extension.
	 *
	 * @param extension_id			The ID of the extension that's sending the chat message.
	 *
	 * @param extension_version		The extension's version number.
	 *
	 * @param text					The message, it may contain a maximum of 280 characters.
	 *
	 * @return						TRUE if successfully sent the chat message, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendExtensionChatMessage(
			const char *broadcaster_id,
			const char *extension_id,
			const char *extension_version,
			const char text[281]);

	/**
	 * Gets information about an extension.
	 *
	 * Requires a signed JSON Web Token (JWT) created by an Extension Backend Service (EBS).
	 * For signing requirements, see https://dev.twitch.tv/docs/extensions/building/#signing-the-jwt
	 * The signed JWT must include the 'role', 'user_id' and 'exp' fields (see JWT Schema
	 * https://dev.twitch.tv/docs/extensions/reference/#jwt-schema).
	 * The 'role' field must be set to 'external'.
	 *
	 * @param extension_id			The ID of the extension to get.
	 *
	 * @param extension_version		The version of the extension to get.
	 * 								If not specified, it returns the latest, released version.
	 * 								If you don't have a released version, you must
	 * 								specify a version, otherwise, the list is empty.
	 * 								It can be NULL or an empty string.
	 *
	 * @param extension				A list that contains the specified extension.
	 *
	 * @return						TRUE if successfully retrieved the list of extensions,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensions(const char *extension_id, const char *extension_version, char **extension);

	/**
	 * Gets information about a released extension.
	 * Returns the extension if its 'state' is 'Released'.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param extension_id			The ID of the extension to get.
	 *
	 * @param extension_version		The version of the extension to get.
	 * 								If not specified, it returns the latest version.
	 * 								It can be NULL or an empty string.
	 *
	 * @param released_extension	The released extension we are looking for.
	 *
	 * @return						TRUE if successfully retrieved the extension,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetReleasedExtensions(const char *extension_id, const char *extension_version, char **released_extension);

	/**
	 * Gets the list of Bits products that belongs to the extension.
	 *
	 * Requires an app access token.
	 * The client ID in the app access token must be the extension's client ID.
	 *
	 * @param should_include_all	A boolean value that determines whether to include disabled
	 * 								or expired Bits products in the response.
	 * 								Default behavior is FALSE.
	 *
	 * @param bits_products			List of Bits products that the extension created.
	 * 								The list is in ascending SKU order.
	 * 								Empty if the extension hasn't created any products or
	 * 								they're all expired or disabled.
	 *
	 * @return						TRUE if successfully retrieved the list of products,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetExtensionBitsProducts(BOOL should_include_all, char **bits_products);

	/**
	 * Adds or updates a Bits product that the extension created.
	 * If the SKU doesn't exist, the product is added.
	 * You may update all fields except the 'sku' field.
	 *
	 * Requires an app access token.
	 * The client ID in the app access token must match the extension's client ID.
	 *
	 * @param sku					The product's SKU.
	 * 								The SKU must be unique within an extension.
	 * 								The product's SKU cannot be changed.
	 * 								The SKU may contain only alphanumeric characters,
	 * 								dashes (-), underscores (_) and periods (.) and
	 * 								is limited to a maximum of 255 characters.
	 * 								No spaces.
	 *
	 * @param cost					The product's cost information.
	 *
	 * @param display_name			The product's name as displayed in the extension.
	 * 								The maximum length is 255 characters.
	 *
	 * @param in_development		A boolean value that indicates whether the product
	 * 								is in development.
	 * 								Set to TRUE if the product is in development and not
	 * 								available for public use, FALSE otherwise.
	 *
	 * @param expiration			The date and time, in RFC3339 format, when the product expires.
	 * 								If not set, the product does not expire.
	 * 								To disable the product, set the expiration date to a date in the past.
	 * 								It can be NULL or an empty string.
	 *
	 * @param is_broadcast			A boolean value that determines whether Bits product purchase
	 * 								events are broadcast to all instances of the extension on a channel.
	 * 								The events are broadcast via the onTransactionComplete helper callback (?).
	 * 								
	 * @param bits_products			A list of Bits products that the extension created/updated.
	 * 								The list is in ascending SKU order.
	 * 								The list is empty if the extension hasn't created/updated any products or
	 * 								they're all expired or disabled.
	 *
	 * @return						TRUE if successfully created/updated the product, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateExtensionBitsProduct(
			const char *sku,
			const FTWLProductCost *cost,
			const char display_name[256],
			BOOL in_development,
			const char *expiration,
			BOOL is_broadcast,
			char **bits_products);
	// ================================ //
	

	// ========== EVENTSUB ========== //

	/**
	 * Create an EventSub connection and gather the session id
	 *
	 * @param event_sub 			Pointer to the FTWLEventSub struct.
	 * 
	 * @return						TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLEventSub(FTWLEventSubSession *event_sub);

	/**
	 * Close the websocket, connection and session handles
	 *
	 * @param event_sub 			Pointer to the EventSub connection
	 * 
	 * @return						TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLEventSubShutdown(const FTWLEventSubSession *event_sub);

	/**
	 * Create an event subscription
	 *
	 * @param event_type 			The event type we are going to subscribe
	 *
	 * @param 						The subscription that we created
	 * 
	 * @return			 			TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLCreateEventSubscription(const FTWLEventType *event_type, char **event);

	/**
	 * Delete an event subscription
	 *
	 * @param subscription_id 		The id of the subscription we want to delete
	 * 
	 * @return				  		TRUE if successful, FALSE otherwise
	 *
	 */
	TWL_API BOOL TWLDeleteEventSubscription(const char *subscription_id);

	/**
	 * Get a list of EventSub subscriptions that the client in the access token created.
	 *
	 * If you use Webhooks or Conduits to receive events, the request must 
	 * specify an app access token.
	 * The request will fail if you use a user access token.
	 *
	 * If you use WebSockets to receive events, the request must specify a user access token.
	 * The request will fail if you use an app access token.
	 * The token may include any scopes.
	 *
	 * Use the "status", "type", "user_id" and "subscription_id" para,eters to filter
	 * the list of subscriptions that are returned.
	 * The filters are mutually exclusive, the request fails if you specify more than one filter.
	 *
	 * @param status				Filter subscriptions by its status.
	 * 								Possible values are:
	 * 									* enabled 
	 * 									  - The subscription is enabled
	 * 									
	 * 									* webhook_callback_verification_pending
	 * 									  - The subscription is pending verification
	 * 									    of the specified callback URL
	 * 									
	 * 									* webhook_callback_verification_failed
	 * 									  - The specified callback URL verification failed.
	 * 									
	 * 									* notification_failures_exceeded
	 * 									  - The notification delivery failure rate was too high.
	 *
	 * 									* authorization_revoked
	 * 									  - The authorization was revoked for one or more users
	 * 									    specified in the Condition object.
	 *
	 * 									* moderator_removed
	 * 									  - The moderator that authorized the subscription is no
	 * 									    longer one of the broadcaster's moderators.
	 *
	 * 									* user_removed
	 * 									  - One of the users specified in the Condition object was removed.
	 *
	 * 									* chat_user_banned
	 * 									  - The user specified in the Condition object was banned from
	 * 									    the broadcaster's chat.
	 *
	 * 									* version_removed
	 * 									  - The subscription to subscription type and version is 
	 * 									    no longer supported.
	 *
	 * 									* beta_maintenance
	 * 									  - The subscription to the beta subscription type was
	 * 									    removed due to maintenance.
	 *
	 * 									* websocket_disconnected
	 * 									  - The client closed the connection.
	 *
	 * 									* websocket_failed_ping_pong
	 * 									  - The client failed to respond to a ping message.
	 *
	 * 									* websocket_received_inbound_traffic
	 * 									  - The client sent a non-pong message, clients may only
	 * 									    send pong messages (and only in response to a ping message).
	 *
	 * 									* websocket_connection_unused
	 * 									  - The client failed to subscribe to events within the required time.
	 *
	 * 									* websocket_internal_error
	 * 									  - The Twitch WebSocket server experienced an unexpected error.
	 *
	 * 									* websocket_network_timeout
	 * 									  - The Twitch WebSocket server timed out writing the message to the client.
	 *
	 * 									* websocket_network_error
	 * 									  - The Twitch WebSocket server experienced a network error writing
	 * 									    the message to the client.
	 *
	 * 									* websocket_failed_to_reconnect
	 * 									  - The client failed to reconnect to the Twitch WebSocket server within
	 * 									    the required time after a Reconnect Message.
	 *
	 * 									* It can be NULL or an empty string.
	 *
	 * @param type					Filter subscriptions by subscription type.
	 * 								For a list of subscription types, see Subscription Types
	 * 								https://dev.twitch.tv/docs/eventsub/eventsub-subscription-types/#subscription-types
	 * 								It can be NULL or an empty string.
	 *
	 * @param user_id				Filter subscriptions by user ID.
	 * 								The response contains subscriptions where this ID matches a user ID
	 * 								that you specified in the Condition object when you created the subscription.
	 * 								It can be NULL or an empty string.
	 *
	 * @param subscription_id		Returns an array with the subscription matching the ID (as long as
	 * 								it is owned by the client making the request), or an empty
	 * 								array if there is no matching subscription.
	 * 								It can be NULL or an empty string.
	 * 
	 * @param subscriptions			The list of subscriptions.
	 * 								The list is ordered by the oldest subscription first.
	 * 								The list is empty if the client hasn't created subscriptions or there
	 * 								are no subscriptions that match the specified filter criteria.
	 *
	 * @return						TRUE if successfully retrieved the subscriptions, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetEventSubSubscriptions(const char *status, const char *type, const char *user_id, const char *subscription_id, char **subscriptions);	

	/**
	 * Read the received data from all the subscribed events
	 *
	 * @param event_sub				Pointer to the EventSub connection
	 *
	 */
	TWL_API BOOL TWLReadEventData(const FTWLEventSubSession *event_sub, char **data);

	// ============================== //
	
	// ========== GAMES ========== //
	
	/**
	 * Gets information about all broadcasts on Twitch.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param top_games				The list of broadcasts.
	 * 								The broadcasts are sorted by the number of viewers,
	 * 								with the most popular first.
	 * 
	 * @return						TRUE if successfully retrieved the list of broadcasts,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetTopGames(char **top_games);

	/**
	 * Gets information about specified categories or games.
	 *
	 * You may get up to 100 categories or games by specifying their ID or name.
	 * You may specify all IDs, all names, or a combination of IDs and names.
	 * If you specify a combination of IDs and names, the total number of IDs and names
	 * must not exeed 100.
	 *
	 * Requires an app access toekn or user access token
	 *
	 * @param id					The ID of the category or game to get.
	 * 								Include this parameter for each category or game you want to get
	 * 								separated by |, for example, 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								The endpoint ignores duplicate and invalid IDs or IDs that weren't found.
	 * 
	 * @param name					The name of the category or game to get.
	 * 								The name must exactly match the category's or game's title.
	 * 								Include this parameter for each category or game you want to get
	 * 								separated by |, for example, foo|bar.
	 * 								You may specify a maximum of 100 names.
	 * 								The endpoint ignores duplicate names and names that weren't found.
	 * 
	 * @param igdb_id				The IGDB ID of the game to get.
	 * 								Include this parameter for each game you want to get
	 * 								separated by |, for example, 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								The endpoint ignores duplicate and invalid IDs or IDs that weren't found.
	 * 
	 * @param games					The list of categories and games.
	 * 								The list is empty if the specified categories and games weren't found.
	 * 
	 * @return						TRUE if successfully retrieved the specified games, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGames(const char *id, const char *name, const char *igdb_id, char **games);
	// =========================== //

	// ========== GOALS ========== //
	
	/**
	 * Gets the broadcaster's list of active goals.
	 * Use this endpoint to get the current progress of each goal.
	 * Instead of polling for the progress of a goal, consider subscribing to receive notifications
	 * when a goal makes progress using the channel.goal.progress subscryption type.
	 *
	 * Requires a user access token that includes the channel:read:goals scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that created the goal.
	 * 								This ID must match the user ID in the user access token.
	 * 
	 * @param goals					The list of goals.
	 * 								The list is empty if the broadcaster hasn't created goals.
	 * 
	 * @return						TRUE if successfully retrieved the broadcaster's goals, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetCreatorGoals(const char *broadcaster_id, char **goals);

	// =========================== //

	// ========== GUEST STAR ========== //
	
	/**
	 * Gets the channel settings for configuration of the Guest Start feature for a particular host.
	 *
	 * Requires a user access token that includes the channel:read:guest_star, channel:manage:guest_star,
	 * moderator:read:guest_star or moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster you want to get guest star settings for.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to moderate
	 * 								the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param settings				The guest star settings.
	 *
	 * @return						TRUE if successfully retrieve the settings, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelGuestStarSettings(const char *broadcaster_id, const char *moderator_id, char **settings);

	/**
	 * Mutates the channel settings for configuration of the Guest Star feature for a partucular host.
	 *
	 * Requires a user access token that includes the channel:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the nroadcaster you want to update
	 * 								Guest Star settings for.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param is_moderator_send_live_enabled	Flag determining if Gueat Star moderators have
	 * 											access to control whether a guest is live once
	 * 											assigned to a slot.
	 *
	 * @param slot_count			Number of slots the Guest Star call interface will allow
	 * 								the host to add to a call.
	 * 								Required to be between 1 and 6.
	 * 								Set it to 0 to don't set it (yep).
	 *
	 * @param is_browser_source_audio_enabled	Flag determining if Browser Sources subscribed
	 * 											to sessions on this channel should output audio.
	 *
	 * @param group_layout			This setting determines how the guests within a session 
	 * 								should be laid out within the browser source.
	 * 								Can be one of the following values:
	 * 									
	 * 									* TILED_LAYOUT: 
	 * 										All live guests are tiled within the 
	 * 										browser source with the same size.
	 *
	 * 									* SCREENSHARE_LAYOUT:
	 * 										All live guests are tiled within the
	 * 										browser source with the same size.
	 * 										If there is an active screen share,
	 * 										it is sized larger than the other guests.
	 *
	 * 									* HORIZONTAL_LAYOUT:
	 * 										All live guests are arranged in a horizontal
	 * 										bar within the browser source.
	 * 										
	 * 									* VERTICAL_LAYOUT:
	 * 										All live guests are arranged in a vertical
	 * 										bar within the browser source.
	 *
	 * 								It can be NULL or an empty string.
	 *
	 * @param regenerate_browser_sources	Flag determining if Guest Star should regenerate
	 * 										the auth token associated with the channel's
	 * 										browser sources.
	 * 										Providing a TRUE value for this will immediately
	 * 										invalidate all browser sources previously configured
	 * 										in your streaming software.
	 *
	 * @return								TRUE if successfully updated channel settings, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateChannelGuestStarSettings(
			const char *broadcaster_id,
			BOOL is_moderator_send_live_enabled,
			int32_t slot_count,
			BOOL is_browser_source_audio_enabled,
			const char *group_layout,
			BOOL regenerate_browser_sources);

	/**
	 * Gets information about an ongoing Guest Star session for a particular channel.
	 *
	 * Requires a user access token that includes channel:read:guest_star, channel:manage:guest_star,
	 * moderator:read:guest_star or moderator:manage:guest_star scope.
	 * Guests must be either invited or assigned a slot within the session.
	 *
	 * @param broadcaster_id		The ID for the user hosting the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to 
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param session				Summary of the session details.
	 *
	 * @return						TRUE if successfully retrieved the session, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGuestStarSession(const char *broadcaster_id, const char *moderator_id, char **session);

	/**
	 * Programmatically creates a Guest Star session on behalf of the broadcaster.
	 * Requires the broadcaster to be present in the call interface,
	 * or the call will be ended automatically.
	 *
	 * Requires a user access token that includes channel:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster you want to create
	 * 								a Guest Star session for.
	 * 								Provided ID must match the user id in the auth token.
	 *
	 * @param session				Summary of the session details.
	 *
	 * @return						TRUE if sucessfully created a Guest Star session,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreateGuestStarSession(const char *broadcaster_id, char **session);

	/**
	 * Programmatically ends a Guest Star session on behalf of the broadcaster.
	 * Performs the same action as if the host clicked the "End Call" button
	 * in the Guest Star UI.
	 *
	 * Requires a user access token that includes channel:manageLguest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster you want to end a
	 * 								Guest Star session for.
	 * 								Provided ID must match the user id in the auth token.
	 *
	 * @param session_id			The ID for the session to end on behalf of the broadcaster.
	 *
	 * @param session				Summary of the session details when the session was ended.
	 *
	 * @return						TRUE if successfully ended the Guest Star session, FALSE
	 * 								otherwise.
	 *
	 */
	TWL_API BOOL TWLEndGuestStarSession(const char *broadcaster_id, const char *session_id, char **session);

	/**
	 * Provides the caller with a list of pending invites to a Guest Star session,
	 * including the invitee's ready status while joining the waiting room.
	 *
	 * Requires a user access token that includes channel:read:guest_star, channel:manage:guest_star,
	 * moderator:read:guest_star or moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								The ID must match the user ID in the user access token.
	 *
	 * @param session_id			The session ID to query for invite status.
	 *
	 * @param invites				A list of invites describing the invited user as well as
	 * 								their ready status.
	 *
	 * @return						TRUE if successfully retrieved a list of invites (even if empty),
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetGuestStarInvites(const char *broadcaster_id, const char *moderator_id, const char *session_id, char **invites);

	/**
	 * Sends an invite to a specified guest on behalf of the broadcaster
	 * for a Guest Star session in progress.
	 *
	 * Requires a user access token that includes channel:manage:guest_star or
	 * moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user id in the user access token.
	 *
	 * @param session_id			The session ID for the invite to be sent on behalf of the broadcaster.
	 *
	 * @param guest_id				Twitch User ID for the guest to invite to the Guest Star session.
	 *
	 * @return						TRUE if successfully invited a guest, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendGuestStarInvite(const char *broadcaster_id, const char *moderator_id, const char *session_id, const char *guest_id);
	
	/**
	 * Revokes a previously sent invite for a Guest Star session.
	 *
	 * Requires a user access token that includes channel:manage:guest_star or
	 * moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user id in the user access token.
	 *
	 * @param session_id			The ID of the session for the invite to be revoked on behalf of the broadcaster.
	 *
	 * @param guest_id				Twitch User ID for the guest to revoke to the Guest Star invite from.
	 *
	 * @return						TRUE if successfully revoked the invite, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteGuestStarInvite(const char *broadcaster_id, const char *moderator_id, const char *session_id, const char *guest_id);

	/**
	 * Allows a previously invited user to be assigned a slot within the active Guest Star session,
	 * once that guest has indicated they are ready to join.
	 *
	 * Requires a user access token that includes channel:manage:guest_star or moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user id in the user access toekn.
	 *
	 * @param session_id			The ID of the Guest Star session in which to assign the slot.
	 *
	 * @param guest_id				The Twitch User ID corresponding to the guest to assign a
	 * 								slot in the session.
	 * 								This user must already have an invite to this session, and
	 * 								have indicated that are ready to join.
	 *
	 * @param slot_id				The slot assignment to give to the user.
	 * 								Must be a numeric identifier between "1" and "N"
	 * 								where N is the max number of slots for the session.
	 * 								Max number of slots allowed for the session is reported
	 * 								by TWLGetChannelGuestStarSettings function.
	 *
	 * @return						TRUE if successfully assigned guest to slot, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLAssignGuestStarSlot(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *session_id,
			const char *guest_id,
			const char *slot_id);
	
	/**
	 * Allows a user to update the assigned slot for a particular user within the
	 * active Guest Star session.
	 *
	 * Requires a use access token that includes channel:manage:guest_star
	 * or moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user is in the user access token.
	 *
	 * @param session_id			The ID of the Guest Star session in which to update slot settings.
	 *
	 * @param source_slot_id		The slot assignment previously assigned to a user.
	 *
	 * @param destination_slot_id	The slot to move this user assignment to.
	 * 								If the destination slot is occupied, the user assigned
	 * 								will be swapped into source_slot_id.
	 *
	 * @return						TRUE if successfully updated slot(s), FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateGuestStarSlot(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *session_id,
			const char *source_slot_id,
			const char *destination_slot_id);
	
	/**
	 * Allows a caller to remove a slot assignment from a user participating in an active Guest Star session.
	 * This revokes their access to the session immediately and disables their access to publish or
	 * subscribes to media within the session.
	 *
	 * Requires a user access token that includes channel:manage:guest_star or moderator:manage:guest_star scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permisison to
	 * 								moderate the broadcaster's char room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param session_id			The ID of the Guest Star session int which to remove
	 * 								the slot assignment.
	 *
	 * @param guest_id				The Twitch User ID corresponding to the guest
	 * 								to remove from the session.
	 *
	 * @param slot_id				The slot ID representing the slot assignment to
	 * 								remove from the session.
	 *
	 * @param should_reinvite_guest Flag signaling that the guest should be reinvited
	 * 								to the session, sending them back to the invite queue.
	 *
	 * @return						TRUE if successfully remove the slot, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteGuestStarSlot(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *session_id,
			const char *guest_id,
			const char *slot_id,
			BOOL should_reinvite_guest);

	/**
	 * Allows a user to update slot settings for a particular guest within a Guet Star session,
	 * such as allowing the user to share audio or video within the call as a host.
	 * These settings will be broadcasted to all subscribers which control their view 
	 * of the guest in that slot.
	 * One or more of the optional parameters to this API can be specified
	 * at any time.
	 *
	 * Requires a user access token that includes channel:manage:guest_star or moderator:manage:guest_star.
	 *
	 * @param broadcaster_id		The ID of the broadcaster running the Guest Star session.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user id in the user access toekn.
	 *
	 * @param session_id			The ID of the Guest Star session in which to update a slot's settings.
	 *
	 * @param slot_id				The slot assignment that has previously been assigned to a user.
	 *
	 * @param is_audio_enabled		Flag indicating whether the slot is allowed to share their audio
	 * 								with the rest of the session.
	 * 								If FALSE, the slot will be muted in any views containing the slot.
	 *
	 * @param is_video_enabled		Flag indicating whether the slot is allowed to share their video
	 * 								with the rest of the session.
	 * 								If FALSE, the slot will have no video shared in any views
	 * 								containing the slot.
	 *
	 * @param is_live				Flag indicating whether the user assigned to this slot is
	 * 								visible/can be heard from any public subscriptions.
	 * 								Generally, this determines whether or not the slot is enabled
	 * 								in any broadcasting software integrations.
	 *
	 * @param volume				Value from 0-100 that controls the audio volume for
	 * 								shared views containing the slot.
	 *
	 * @return						TRUE if successfully updated the slot settings, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateGuestStarSlotSettings(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *session_id,
			const char *slot_id,
			BOOL is_audio_enabled,
			BOOL is_video_enabled,
			BOOL is_live,
			int32_t volume);
	// ================================ //

	// ========== HYPE TRAIN ========== //
	
	/**
	 * Gets information about the broadcaster's current or most recent Hype Train event.
	 * Instead of polling for events, consider subscribing to Hype Train events (Begin, Progress, End).
	 *
	 * Requires a user access token that includes the channel:read:hype_train scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's running the Hype Train.
	 * 								This ID must match the User ID in the user access token.
	 * 
	 * @param hype_train_events		The list of hype train events.
	 * 								This list is empty if the broadcaster hasn't run a Hype Train
	 * 								within the last 5 days.
	 * 
	 * @return						TRUE if successfully retrieved the broadcaster's Hype train events,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetHypeTrainEvents(const char *broadcaster_id, char **hype_train_events);

	/**
	 * Get the status of a Hype Train for the specified broadcaster.
	 *
	 * Requires a user access token that includes the channel:read:hype_train scope.
	 *
	 * @param broadcaster_id		The User ID of the channel broadcaster.
	 * 								This ID must match the User ID in the user access token.
	 *
	 * @param hype_train_status		A list that contains information related to the channel's Hype Train.
	 *
	 * @return						TRUE if successfully retrieved the status object.
	 *
	 */
	TWL_API BOOL TWLGetHypeTrainStatus(const char *broadcaster_id, char **hype_train_status);
	// ================================ //

	// ========== MODERATION ========== //
	
	/**
	 * Checks whether AutoMod would flag the specified message for review.
	 *
	 * AutoMod is a moderation tool that holds inappropriate or harassing chat messages
	 * for moderators to review.
	 * Moderators approve or deny the messages that AutoMod flags, only approved messages
	 * are released to chat.
	 * AutoMod detects misspellings and evasive language automatically.
	 *
	 * RATE LIMITS:	Rates are limited per channel based on the account type
	 * 				rather than per access token.
	 *
	 * 				ACCOUNT TYPE	LIMIT PER MINUTE	LIMIT PER HOUR
	 * 				Normal			5					50
	 * 				Affiliate		10					100
	 * 				Partner			30					300
	 *
	 * The above limits are in addition to the standard Twitch API rate limits.
	 * The rate limit headers in the response represent the Twitch rate limits and not
	 * the above limits.
	 *
	 * Requires a user access token that includes the moderation:read scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose AutoMod settings and list of
	 * 								blocked terms are used to check the message.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param messages_to_check		The list of messages to check.
	 * 								The list must contain at least one message and may contain
	 * 								up to a maximum of 100 messages.
	 *
	 * @param checked_messages		The list of messages and whether Twitch would approve them for chat.
	 *
	 * @result						TRUE if successfully checked the messages, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCheckAutomodStatus(const char *broadcast_id, FTWLAutoModMessage messages_to_check[100], char **checked_messages);

	/**
	 * Allow or deny the message that AutoMod flagged for review.
	 *
	 * WARNING - PUBSUB IS DEPRECATED
	 * To get messages that AutoMod is holding for review, subscribe to the 
	 * automod-queue.<moderator_id>.<channel_id> topic using PubSub.
	 * PubSub sends a notification to your app when AutoMod holds a message for review.
	 * WARNING - PUBSUB IS DEPRECATED
	 *
	 * Requires a user access token that includes the moderator:manage:automod scope.
	 *
	 * @param user_id				The moderator who is approving or denying the held message.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param msg_id				The ID of the message to allow or deny.
	 *
	 * @param action				The action to take for the message.
	 * 								Possible values are:
	 * 									* ALLOW
	 * 									* DENY
	 *
	 * @return						TRUE if successfully approved or denied the message,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLManageHeldAutoModMessages(const char *user_id, const char *msg_id, const char *action);

	/**
	 * Gets the broadcaster's AutoMod settings.
	 * The Settings are used to automatically block inappropriate
	 * or harassing messages from appearing in the broadcaster's chat room.
	 *
	 * Requires a user access token that includes the moderator:read:automod_settings scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose AutoMod settings you want to get.
	 *
	 * @param moderator_id			The ID of the braodcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param settings				The list of AutoMod settings.
	 * 								The list contains a single object that contains all
	 * 								the AutoMod settings.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's AutoMod settings,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetAutoModSettings(const char *broadcaster_id, const char *moderator_id, char **settings);

	/**
	 * Updates the broadcaster's AutoMod settings.
	 * The settings are used to automatically block inappropriate or harassing
	 * messages from appearing in the broadcaster's chat room.
	 *
	 * Requires a user access token that includes the moderator:manage:automod_settings scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose AutoMod settings
	 * 								you want to update.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param new_settings			Because this is an overwrite operation, you must include all the
	 * 								fields that you want set after the operation completes.
	 * 								Typically, you'll call TWLGetAutoModSettings, update the fields
	 * 								you want to change, and pass that struct to TWLUpdateAutoModSettings.
	 *
	 * 								You may set either 'overall_level' or the individual settings
	 * 								like 'aggression', but not both.
	 *
	 * 								Setting 'overall_level' applies default values to the individual settings.
	 * 								However, settings 'overall_level' to 4 does not necessarily mean that it
	 * 								applies 4 to all the individual settings.
	 * 								Instead, if applies a set of recommended defaults to the rest of the settings.
	 * 								For example, if you set 'overall_level' to 2, Twitch provides some filtering on
	 * 								discrimination and sexual content, but more filtering ob hostility.
	 *
	 * 								If 'overall_level' is currently set and you update 'swearing' to 3, 'overall_level'
	 * 								will be set to null and all settings other than 'swearing' will be set to 0.
	 * 								The same is true if individual settings are set and you update 'overall_level' to 3,
	 * 								all the individual settings are updated to reflect the default level.
	 *
	 * 								Note that if you set all the individual settings to values that match what 'overall_level'
	 * 								would have set them to, Twitch changes AutoMod to use the default AutoMod level
	 * 								instead of using the individual settings.
	 *
	 * 								Valid values for all levels are from 0 (no filtering) to 4 (most aggressive filtering).
	 * 								These levels affect how aggressively AutoMod holds black messages for moderators
	 * 								to review before they appear in chat or are denied (not shown).
	 *
	 * 								TL;DR; 	Setting 'overall_level' struct member >= 0 will exclude all the other
	 * 										struct members to be used.
	 *
	 * @param updated_settings		The updated AutoMod settings.
	 *
	 * @return						TRUE if successfully update the broadcaster's AutoMod the settings, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateAutoModSettings(const char *broadcaster_id, const char *moderator_id, const FTWLAutoModSettings *new_settings, char **updated_settings);

	/**
	 * Gets all users that the braodcaster banned or put in a timeout.
	 *
	 * Requires a user access token that includes the moderation:read or moderation:manage:banned_users scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose list of banned users you want to get.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				A list of user IDs used to filter the results.
	 * 								To specify more than one ID, include this parameter for each
	 * 								user you want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param banned_users			The list of users that were banned or put in a timeout.
	 *
	 * @return						TRUE if successfully retrieved the list of banned users, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetBannedUsers(const char *broadcaster_id, const char *user_id, char **banned_users);

	/**
	 * Bans a user from partecipating in the specified broadcaster's chat room or puts them in a timeout.
	 * 
	 * If the user is currently in a timeout, you can call this function to change the duration of
	 * the timeout or ban them altogether.
	 * If the user is currently banned, you cannot call this function to put them in a timeout instead.
	 * 
	 * To remove a ban or end a timeout, use TWLUnbanUser function.
	 *
	 * Requires a user access token that includes the moderator:manage:banned_users scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat room 
	 * 								the user is being banned from.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param user_id				The ID of the user to ban or put in a timeout.
	 *
	 * @param duration				To ban a user indefinitely, set this parameter to 0.
	 * 								
	 * 								To put a user in a timeout, use this parameter and specify
	 * 								the timeout period, in seconds,
	 * 								The minimum timeout is 1 second and the maximum is 1,209,600
	 * 								seconds (two weeks).
	 *
	 * 								To end a user's timeout early, set this paramter to 1, or
	 * 								use the TWLUnbanUser function.
	 *
	 * @param reason				The reason the you're banning the user or putting them in a timeout.
	 * 								The text is user defined and is limited to a maximum of 500 characters.
	 * 								It can be NULL or an empty string.
	 *
	 * @param banned_user			The user you successfully banned or put in a timeout.
	 *
	 * @return						TRUE if successfully banned the user or placed them in a timeout,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLBanUser(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *user_id,
			int32_t duration,
			const char reason[501],
			char **banned_user);
	
	/**
	 * Removes the ban or timeout that was placed on the specified user.
	 *
	 * Requires a user access token that includes the moderation:manage:banned_users scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose chat room the user is banned
	 * 								from chatting in.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param user_id				The ID of the user to remove the ban or timeout from.
	 *
	 * @return						TRUE we successfully removed the ban or timeout,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUnbanUser(const char *broadcaster_id, const char *moderator_id, const char *user_id);

	/**
	 * Gets a list of unban requests for a broadcaster's channel.
	 *
	 * Requires a user access token that includes the moderator:read:unban_requests or
	 * moderator:manage:unban_requests scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose channel is receiving unban requests.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission to moderate
	 * 								the broadcaster's unban requests.
	 * 								The ID must match the user ID in the user access token.
	 *
	 * @param status				Filter by a status.
	 * 									* pending
	 * 									* approved
	 * 									* denied
	 * 									* acknowledged
	 * 									* canceled
	 *
	 * @param user_id				The ID used to filter what unban requests returned.
	 * 								It can be NULL or an empty string.
	 *
	 * @param unban_requests		The channel's unban requests.
	 *
	 * @return						TRUE if successfully retrieved the list of unban requests, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUnbanRequest(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *status,
			const char *user_id,
			char **unban_requests);

	/**
	 * Resolves an unban request by approving or denying it.
	 *
	 * Requires a user access token that includes the moderator:manage:unban_requests scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose channel is approving
	 * 								or denying the unban request.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's unban requests.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param unban_request_id		The ID of the unban request.
	 *
	 * @param status				Resolution status:
	 * 									* approved
	 * 									* denied
	 *
	 * @param resolution_text		Message supplied by the unban request resolver.
	 * 								The message is limited to a maximum of 500 characters.
	 * 								It can be NULL or an empty string.
	 *
	 * @param unban_resolution		The unban resolution.
	 *
	 * @return						TRUE if successfully resolved the unban request,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLResolveUnbanRequests(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *unban_request_id,
			const char *status,
			const char resolution_text[501],
			char **unban_resolution);

	/**
	 * Gets the broadcaster's list of non-private, blocked words and phrases.
	 * These are the terms that the broadcaster or moderator added manually or
	 * that were denied by AutoMod.
	 *
	 * Requires a user access token that includes the moderator:read:blocked_terms or
	 * moderator:manage:blocked_terms scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose blocked terms you're getting.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission 
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param blocked_terms			The list of blocked terms in descending order of when
	 * 								they were created (see the created_at timestamp).
	 *
	 * @return						TRUE if successfully retrieved the list of blocked terms, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetBlockedTerms(const char *broadcaster_id, const char *moderator_id, char **blocked_terms);
	
	/**
	 * Adds a word or phrase to the broadcaster's list of blocked terms.
	 * These are the terms that the broadcaster doesn't want used in their chat room.
	 *
	 * Requires a user access token that includes the moderator:manage:blocked_terms scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the list of blocked terms.
	 *
	 * @param moderator_id			The ID of hte broadcaster or a user that has permission to
	 * 								moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param text					The sword or phrase to block from being used in the
	 * 								broadcaster's chat room.
	 * 								The term must contain a minimum of 2 characters and may contain
	 * 								up to a maximum of 500 characters.
	 *
	 * 								Terms may include a wildcard character (*).
	 * 								The wildcard character must appear at the beginning or end
	 * 								of a word or set of characters.
	 * 								For example "*foo" or "foo*".
	 *
	 * 								If the blocked term already exists, the response contains
	 * 								the existing blocked term.
	 *
	 * @param blocked_term			The blocked term we just added or the already existing one.
	 *
	 * @return						TRUE if successfully retrieved the list of blocked terms, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLAddBlockedTerm(const char *broadcaster_id, const char *moderator_id, const char text[501], char **blocked_term);
	
	/**
	 * Removes the word or phrase from the broadcaster's list of blocked terms.
	 *
	 * Requires a user access token that includes the moderator:manage:blocked_terms scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the list of blocked terms.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param id					The ID of the blocked term to remove from the broadcaster's
	 * 								list of blocked terms.
	 *
	 * @return						TRUE if successfully removed the blocked term, also
	 * 								returned if the ID is not found, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLRemoveBlockedTerm(const char *broadcaster_id, const char *moderator_id, const char *id);
	
	/**
	 * Removes a single chat message or all chat messages from the broadcaster's chat room.
	 *
	 * Requires a user access token that includes the moderator:manage:chat_messages scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the chat room
	 * 								to remove messages from.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that has permission
	 * 								to moderate the broadcaster's chat room.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param message_id			The ID of the message to remove.
	 * 								Restrictions:
	 * 									* The message must have been created within the last 6 hours.
	 * 									* The message must not belong to the broadcaster.
	 * 									* The message must not belong to another moderator.
	 *
	 * 								If NULL or empty string, the request removes all messages
	 * 								in the broadcaster's chat room.
	 *
	 * @return						TRUE if successfully removed the specified messages,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteChatMessages(const char *broadcaster_id, const char *moderator_id, const char *message_id);

	/**
	 * Gets a list of channels that the specified user has moderator priviliges in.
	 *
	 * Requires a user access token that includes the user:read:moderated_channels scope.
	 *
	 * @param user_id				The ID of the user that has moderator priviliges.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param moderated_channels	The list of channels that the user has moderator privileges in.
	 *
	 * @return						TRUE if we successfully get a list of moderated channels, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetModeratedChannels(const char *user_id, char **moderated_channels);
	
	/**
	 * Gets all users allowed to moderate the broadcaster's chat room.
	 *
	 * Requires a user access token that includes the moderation:read scope.
	 * If your app also adds and removes moderators, you can use the channel:manage:moderators scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose list of moderators you want to get.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				A list of user IDs used to filter the results.
	 * 								To specify more than one ID, include this parameter for each
	 * 								user you want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * 								The returned list includes only the users from the list
	 * 								who are moderators in the broadcaster's channel.
	 * 								The list is returned in the same order as you
	 * 								specifid the IDs.
	 *
	 * @param moderators			The list of moderators.
	 *
	 * @return						TRUE if successfully retrieved the list of moderators, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetModerators(const char *broadcaster_id, const char *user_id, char **moderators);
	
	/**
	 * Adds a moderator to the broadcaster's chat room.
	 *
	 * RATE LIMITS:	The broadcaster may add a maximum of 10 moderators within a 10-second window.
	 *
	 * Requires a user access token that includes the channel:manage:moderators scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the chat room.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				The ID of the user to add as a moderator in the
	 * 								broadcaster's char room.
	 *
	 * @return						TRUE if successfully added the moderator, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLAddChannelModerator(const char *broadcaster_id, const char *user_id);
	
	/**
	 * Removes a moderator from the broadcaster's chat room.
	 *
	 * RATE LIMITS:	The broadcaster may remove a maximum of 10 moderators within a 10-second window.
	 *
	 * Requires a user access token that includes the channel:manage:moderators scope.
	 *
	 * @pram broadcaster_id			The ID of the broadcaster that owns the chat room.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				The ID of the user to remove as a moderator from
	 * 								the broadcaster's chat room.
	 *
	 * @return						TRUE if successfully removed the moderator, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLRemoveChannelModerator(const char *broadcaster_id, const char *user_id);
	
	/**
	 * Gets a list of broadcaster's VIPs.
	 *
	 * Requires a user access token that includes the channel:read:vips scope.
	 * If your app also adds and removes VIP status, you can use the channel:manage:vips scope instead.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose list of VIPs you want to get.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				Filters the list for specific VIPs.
	 * 								To specify more than one ID, include this parameter for each
	 * 								user you want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								Ignores the ID of those users in the list that aren't VIPs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param VIPs					The list of VIPs, is empty if broadcaster doesn't have VIPs.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's list of VIPs,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetVIPs(const char *broadcaster_id, const char *user_id, char **VIPs);
	 
	/**
	 * Adds the specified user as a VIP in the broadcaster's channel.
	 *
	 * RATE LIMITS:	The broadcaster may add a maximum of 10 VIPs within a 10-second window.
	 *
	 * Requires a user access token that includes the channel:manage:vips scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster taht's adding the user as a VIP.
	 * 								The ID must match the user ID in the access token.
	 *
	 * @param user_id				The ID of the user to give VIP status to.
	 *
	 * @return						TRUE if successfully added the VIP, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLAddChannelVIP(const char *broadcaster_id, const char *user_id);
	
	/**
	 * Removes the specified user as a VIP in the broadcaster's channel.
	 *
	 * If the broadcaster is removing the user's VIP status, the ID in the
	 * broadcaster_id paramter must match the user ID in the access token,
	 * otherwise, if the user is removing their VIP status themselves,
	 * the ID in the user_id paramter must match the user ID in the access token.
	 *
	 * RATE LIMITS:	The broadcaster may remove a maximum of 10 VIPs within a 10 second window.
	 *
	 * Requires a user access token that includes the channel:manage:vips scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster who owns the channel
	 * 								where the user has VIP status.
	 *
	 * @param user_id				The ID of the user to remove VIP status from.
	 *
	 * @return						TRUE if successfully removed the VIP status
	 * 								from the user, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLRemoveChannelVIP(const char *broadcaster_id, const char *user_id);
	
	/**
	 * Activates or deactivates the broadcaster's Shield Mode.
	 *
	 * Twitch's Shield Mode feature is like a panic button that broadcasters can push to
	 * protect themselves from chat abuse coming from one or more accounts.
	 * When activated, Shield Mode applies the overrides that the braodcaster configured
	 * in the Twitch UX.
	 * If the broadcaster hasn't configured Shield Mode, it applies default overrides.
	 *
	 * Requires a user access token that includes the moderator:manage:shield_mode scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose Shield Mode you want
	 * 								to activate or deactivate.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that is one
	 * 								of the broadcaster's moderators.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param is_active				A boolean value that determines whether to activate
	 * 								Shield Mode.
	 * 								Set to TRUE to activate Shield Mode, otherwise FALSE
	 * 								to deactivate Shield Mode.
	 *
	 * @param shield_mode_status	The broadcaster's updated Shield Mode status.
	 *
	 * @return						TRUE if successfully updated the Shield Mode status, FALSE otherwise,
	 *
	 */
	TWL_API BOOL TWLUpdateShieldModeStatus(const char *broadcaster_id, const char *moderator_id, BOOL is_active, char **shield_mode_status);

	/**
	 * Gets the broadcaster's Shield Mode activation status.
	 *
	 * To receive notification when the broadcaster activates and deactivates Shield Mode,
	 * subscribe to the channel.shield_mode.begin and channel.shield_mode.end subscription types.
	 *
	 * Requires a user access token that includes the moderator:read:shield_mode or
	 * moderator:manage:shield_mode scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose Shield Mode activation
	 * 								status you want to get.
	 *
	 * @param moderator_id			The ID of the broadcaster or a user that is one of the
	 * 								broadcaster's moderators.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param shield_mode_status	The broadcaster's Shield Mode status.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's Shield Mode
	 * 								activation status, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetShieldModeStatus(const char *broadcaster_id, const char *moderator_id, char **shield_mode_status);
	
	/**
	 * Warns a user in the specified broadcaster's chat room, preventing them from
	 * chat interaction until the warning is acknowledged.
	 * New warnings can be issued to a user when they already have a warning
	 * in the channel (new warning will replace old warning).
	 *
	 * Requires a user access token that includes the moderator:manage:warnings scope.
	 *
	 * @param broadcaster_id		The ID of the channel in which the warning will take effect.
	 *
	 * @param moderator_id			The ID of the twitch user who requested the warning.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				The ID of the Twitch user to be warned.
	 *
	 * @param reason				A custom reason for the warning.
	 * 								The message is limited to a maximum of 500 characters.
	 *
	 * @param warning_info			Informationa bout the warning.
	 *
	 * @return						TRUE if successffully warn a user, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLWarnChatUser(
			const char *broadcaster_id,
			const char *moderator_id,
			const char *user_id,
			const char reason[501],
			char **warning_info);
	// ================================ //


	// ========== POLLS ========== //
	
	/**
	 * Gets a list of pools that the broadcaster created.
	 * Pools are available for 90 days after they've created.
	 *
	 * Requires a user access token that includes the channel:read:polls or channel:manage:polls scope.
	 *
	 * @param broadcaster_id		The ID of the breoadcaster that created the polls.
	 * 								This IS must match the user ID in the user access token.
	 *
	 * @param id					A list of IDs that identify the poll to return.
	 * 								To specify morethan one ID, include this parameter for each poll
	 * 								you want to get separated by |, for example, 1234|5678.
	 * 								You may specify a maximum of 20 IDs.
	 * 								Specify this parameter only if you want to filter the list the
	 * 								request returns.
	 * 								The endpoint ignores duplicate IDs and those not owned by this broadcaster.
	 * 								It can be NULL or an empty string.
	 *
	 * @param polls					A list of polls.
	 * 								The polls are returned in descending order of start
	 * 								time unless you specify IDs in the request, in which case they're
	 * 								returned in the same order as you passed them in the request.
	 * 								The list is empty if the broadcster hasn't created polls.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's polls, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetPolls(const char *broadcster_id, const char *id, char **polls);

	/**
	 * Creates a poll that viewers in the boradcaster's channel can vote on.
	 * The poll begins as soon as it's created. You may run only one poll at a time.
	 *
	 * Requires a user access token that includes the channel:manage:polls scope.
	 * The broadcaster need to be a partner or affiliate.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's running the poll.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param title					The question that viewers will vote on.
	 * 								For example, "What game should i play next?".
	 * 								The question may contain a maximum of 60 characters.
	 *
	 * @param choices				A list of choices that viewers may choose from.
	 * 								The list must contain a minimum of 2 choices and up to a
	 * 								maximum of 5 choices, the choice may contain a maximum 
	 * 								of 25 characters.
	 *
	 * @param duration				The length of time (in seconds) that the poll will run for.
	 * 								The minimum is 15 seconds and the maximum is 1800 seconds (30 minutes).
	 *
	 * @param channel_points_voting_enabled	
	 * 								
	 * 								A Boolean value that indicates whether viewers may cast additional votes using Channel Points.
	 * 								If TRUE, the viewer may cast more than one vote but each additional vote costs the
	 * 								number of Channel Points specified in channel_points_per_vote.
	 * 								The default behavior is FALSE (viewers may cast only one vote).
	 * 								
	 * @param channel_points_per_vote
	 * 								
	 * 								The number of points that the viewer must spend to cast one additional vote.
	 * 								The minimum is 1 and the maximum is 1000000.
	 * 								Set only if channel_points_voting_enabled is TRUE.
	 *
	 * @param poll					The poll that you created.
	 *
	 * @return						TRUE if successfully created the poll, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreatePoll(
			const char *broadcaster_id, 
			const char title[61],
			const char choices[5][26],
			int32_t duration,
			BOOL channel_points_voting_enabled,
			int32_t channel_points_per_vote,
			char **poll);

	/**
	 * Ends an active poll.
	 * You have the option to end it or end it and archive it.
	 *
	 * Requires a user access token that includes the channel:manage:polls scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's running the poll.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param id					The ID of the poll to update.
	 *
	 * @param status				The status to set the poll to.
	 * 								Possible case-sensitive values are:
	 * 								* TERMINATED
	 * 									Ends the poll before the poll is scheduled to end.
	 * 									The poll remains publicly visble.
	 *
	 * 								* ARCHIVED
	 * 									Ends the poll before the poll is scheduled to end,
	 * 									and then archives it so it's no longer publicy visible.
	 *
	 * @param poll					The poll that you ended.
	 *
	 * @return						TRUE if successfully ended the poll, FALSE otherwise.
	 * 										
	 */
	TWL_API BOOL TWLEndPoll(const char *broadcaster_id, const char *id, const char *status, char **poll);
	// =========================== //

	// ========== PREDICTIONS ========== //
	
	/**
	 * Gets a list of Channel Points Predictions that the broadcaster created.
	 *
	 * Requires a user access token that includes the channel:read:predictions or channel:manage:predictions scope.
	 *
	 * @param broadcaster_id 		The ID of the broadcaster whose predictions you want to get.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param id					The ID of the prediction to get.
	 * 								To specify more than one ID, include this parameters for each
	 * 								prediciton you want to get separated by |, for example 1234|5678,
	 * 								you may specify a maximum of 25 IDs.
	 * 								The endpoint ignores duplicate IDs and those not owned by the broadcaster.
	 * 								It can be NULL or an empty string.
	 *
	 * @param predictions			The broadcaster's list of Channel Points Predictions.
	 * 								The list is sorted in descending ordered by when the prediction began,
	 * 								the most recent prediction is first.
	 *
	 * @return						TRUE if successfully retrieved the list of predictions, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetPredictions(const char *broadcaster_id, const char *id, char **predictions);

	/**
	 * Creates a Channel Points Prediction.
	 * With a Channel Points Predictions, the broadcaster poses a question 
	 * and viewers try to predict the outcome.
	 * The prediction runs as soon as it's created.
	 * The broadcaster may run only one prediction at a time.
	 *
	 * Requires a user access token that includes the channel:manage:predictions scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's running the prediction.
	 * 								The ID must match the user ID in the user access token.
	 *
	 * @param title					The question that the broadcaster is asking.
	 * 								For example, "Will i finish the entire pizza?"
	 * 								The title is limited to a maximum of 45 characters.
	 *
	 * @param outcomes				The list of possible outcomes that the viewers may choose from.
	 * 								The list must contain a minimum of 2 choices and up to 
	 * 								a maximum of 10 choices, the title is limited to 25 characters.
	 *
	 * @param prediction_window		The length of time (in seconds) that the prediction will run for.
	 * 								The minimum is 30 seconds and the maximum is 1800 seconds (30 minutes).
	 *
	 * @param prediction			The prediction that you just created.
	 *
	 * @return						TRUE if successfully created the Channel Points Prediction, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreatePrediction(
			const char *broadcaster_id,
			const char title[46],
			const char outcomes[10][26],
			int32_t prediction_window,
			char **prediction);

	/**
	 * Locks, resolves, or cancels a Channel Points Prediction.
	 *
	 * Requires a user access token that includes the channel:manage:predictions scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that's running the prediction.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param id					The ID of the prediction to update.
	 *
	 * @param status				The status to set the prediction to.
	 * 								Possible case-sensitive values are:
	 * 									* RESOLVED
	 * 										The winning outcome is determined and the
	 * 										Channel Points are distributed to the viewers
	 * 										who predicted the correct outcome.
	 *
	 * 									* CANCELED
	 * 										The broadcastr is cancelling the prediction and
	 * 										sending refunds to the partecipants.
	 *
	 * 									* LOCKED
	 * 										The broadcaster is locking the predictions,
	 * 										which means viewers may no longer make predictions.
	 *
	 * 								The Broadcaster can update an active prediction to LOCKED,
	 * 								RESOLVED or CANCELED and update a locked prediction
	 * 								to RESOLVED or CANCELED.
	 *
	 * 								The broadcaster has up to 24 hours after the prediction window
	 * 								closes to resolve the prediction, if not Twitch sets the status to
	 * 								CANCELED and returns the points.
	 *
	 * @param winning_outcome_id	The ID of the winning outcome.
	 * 								You must set this parameter if you set the status to RESOLVED.
	 *
	 * @param prediction			The prediction that you updated.
	 *
	 * @return						TRUE if successfully ended the prediction, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLEndPrediction(const char *broadcaster_id, const char *id, const char *status, const char *winning_outcome_id, char **prediction);
	// ================================= //

	// ========== RAIDS ========== //
	
	/**
	 * Raid another channel by sending the broadcaster's viewers to the targeted channel.
	 *
	 * When you call the API from a chat bot or extension, the Twitch UX pops up a window at the
	 * top of the chat room that identifies the number of viewers in the raid.
	 * The raid occurs when the broadcaster clicks Raid Now or after the 90-seconds countdown expires.
	 * To determine whether the raid successfully occurred, you must subscribe to the Channel Raid event.
	 * To cancel a pending raid, use the TWLCancelRaid function.
	 *
	 * RATE LIMIT: The limit is 10 requests within a 10-minute window.
	 *
	 * Requires a user access token that includes the channel:manage:raids scope.
	 *
	 * @param from_broadcaster_id	The ID of the broadcaster that's sending the raiding party.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param to_broadcaster_id		The ID of the broadcaster to raid.
	 *
	 * @param data					Information about the pending raid.
	 *
	 * @return						TRUE if we successfully requested to start a raid FALSE otherwise.
	 * 								To determine whether the raid successfully occurred (that is, the
	 * 								broadcaster clicked Raid Now or the countdown expired), you must
	 * 								subscribe to the Channel Raid event.
	 *
	 */
	TWL_API BOOL TWLStartRaid(const char *from_broadcaster, const char *to_broadcaster_id, char **data);

	/**
	 * Cancel a pending raid.
	 *
	 * You can cancel a raid at any pint up until the broadcaster clicks Raid Now 
	 * in the Twitch UX or the 90-second countdown expires.
	 *
	 * RATE LIMIT: The limit is 10 requests within a 10-minute window.
	 *
	 * Requires a user access token that includes the channel:manage:raids scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that initiated the raid.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @return						TRUE if the pending raid was successfully canceled,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCancelRaid(const char *broadcaster_id);
	// =========================== //

	// ========== SCHEDULE ========== //
	
	/**
	 * Gets the broadcaster's streaming schedule.
	 * You can get the entire schedule of specific segnebts of the schedule.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the streming schedule
	 * 								you want to get.
	 *
	 * @param id					The ID of the scheduled segment to return.
	 * 								To specify more than one segment, include the ID of each
	 * 								segment you want to get separated by |, for example 1234|5678.
	 * 								You may specify a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param start_time			The UTC date and time that identifies when in the broadcaster's
	 * 								schedule to start returning segments.
	 * 								If not specified, the request returns segments starting after
	 * 								the current UTC date and time.
	 * 								Specify the date and time inRFC3339 format, for example
	 * 								2025-09-01T00:00:00Z.
	 * 								It can be NULL or an empty string.
	 *
	 * @param schedule				The broadcaster streaming schedule.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's streaming schedule,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelStreamSchedule(const char *broadcaster_id, const char *id, const char *start_time, char **schedule);

	/**
	 * Gets the broadcaster's streaming schedule as an iCalendar.
	 *
	 * The Client-ID and Authorization headers are not required.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the streaming schedule
	 * 								you want to get.
	 *
	 * @param iCalendar				The iCalendar data, the Content-Type response header
	 * 								is set to text/calendar.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's
	 * 								schedule as an iCalendar, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChanneliCalendar(const char *broadcaster_id, char **iCalendar);

	/**
	 * Updates the broadcaster's schedule settings, such as scheduling a vacation.
	 *
	 * Requires a user access token that includes the channel:manage:schedule scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster whose schedule settings you want to update.
	 * 								The ID must match the user ID in the user access token.
	 *
	 * @param is_vacation_enabled	A boolean value that indicates whether the broadcaster 
	 * 								has scheduled a vacation.
	 * 								Set to TRUE to enable Vacation Mode and add vacation dates,
	 * 								or FALSE to cancel a previously scheduled vacation.
	 *
	 * @param vacation_start_time	The UTC date and time of when the broadcaster's vacation starts.
	 * 								Specify the date and time in RFC3339 format.
	 * 								Required if is_vacation_enabled is TRUE.
	 *
	 * @param vacation_end_time		The UTC date and time of when the broadcaster's vacation ends.
	 * 								Specify the date and time in RFC3339 format.
	 * 								Required if is_vacation_enabled is TRUE.
	 * 
	 * @param timezone				The time zone that the broadcaster broadcasts from.
	 * 								Specify the time zone using IANA time zone database format,
	 * 								for example America/New_York.
	 * 								Required if is_vacation_enabled is TRUE.
	 *
	 * @return						TRUE if successfully updated the broadcaster's schedule settings,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateChannelStreamSchedule(
			const char *broadcaster_id,
			const BOOL is_vacation_enabled,
			const char *vacation_start_time,
			const char *vacation_end_time,
			const char *timezone);
	
	/**
	 * Adds a single or recurring broadcast to the broadcaster's streaming schedule.
	 * 
	 * Requires a user access token that includes the channel:manage:schedule scope.
	 *
	 * @param broadcaster_id		The ID of the broadcaster that owns the schedule to add
	 * 								the broadcast segment to.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param start_time			The date and time that the boradcast segment starts.
	 * 								Specify the date and time in RFC3339 format, for example
	 * 								2025-07-01T18:00:00Z.
	 *
	 * @param timezone				The time zone where the broadcast takes place.
	 * 								Specify the time zone using IANA time zone database format,
	 * 								for example America/New_York.
	 *
	 * @param duration				The length of time, in minutes, that the broadcast is scheduled to run,
	 * 								The duration must be in range 30 through 1380 (23 hours).
	 * 
	 * @param is_recurring			A boolean vale that determines whether the broadcast recurs weekly.
	 * 								Is TRUE if the broadcast recurs weekly.
	 * 								Only partners and affiliates may add non-recurring broadcast.
	 *
	 * @param category_id			The ID of the category that best represents the broadcast's content.
	 * 								To get the category ID, use the function TWLSearchCategories.
	 * 								It can be NULL or an empty string.
	 *
	 * @param title					The broadcast's title.
	 * 								The title may contain a maximum of 140 characters.
	 * 								It can be NULL or an empty string.
	 *
	 * @param scheduled_stream		The broadcaster's streaming scheduled.
	 *
	 * @return						TRUE if successfully added the broadcast segment,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCreateChannelStreamScheduleSegment(
			const char *broadcaster_id,
			const char *start_time,
			const char *timezone,
			const char *duration,
			const BOOL is_recurring,
			const char *category_id,
			const char *title,
			char **scheduled_stream);

	/**
	 * Updates a scheduled broadcast segment.
	 * For recurring segments, updating a segment's title, category, duration and timezone,
	 * changes all segments in the recurring schedule, not just the specified segment.
	 *
	 * Requires a user access token that includes the channel:manage:schedule scope.
	 *
	 * @param braodcaster_id		The ID of the broadcaster who owns the broadcast segment to update.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param id					The ID of the broadcast segment to update.
	 *
	 * @param start_time			The date and time that the broadcast segment starts.
	 * 								Specify the date and time in RFC3339 format, for example
	 * 								2026-11-10T13:30:00Z.
	 * 								NOTE:	Only partners and affiliates may update a broadcast's
	 * 										start time and only for non-recurring segments.
	 * 								It can be NULL or an empty string.
	 *
	 * @param duration				The length of time, in minutes, that the braodcast is
	 * 								cheduled to run.
	 * 								The duration must be in the range 30 through 1380 (23 hours).
	 * 								It can be NULL or an empty string.
	 *
	 * @param category_id			The ID of the category that best represents the broadcast's content.
	 * 								To get the category ID, use the TWLSearchCategories function.
	 * 								It can be NULL or an empty string.
	 *
	 * @param title					The broadcast's title.
	 * 								The title may contain a maximum of 140 characters.
	 * 								It can be NULL or an empty string.
	 *
	 * @param is_canceled			A boolean value that indicates whether the broadcast is canceled.
	 * 								Set to TRUE to cancel the segment.
	 * 								NOTE:	for recurring segments, the API cancels the first segment
	 * 										after the current UTC date and time and not the specified segment
	 * 										(unless the specified segment is the next segment after the current
	 * 										UTC date and time).
	 *
	 * @param timezone				The timezone where the broadcst takes place.
	 * 								Specify the time zone using IANA time zone database format,
	 * 								for example, America/New_York.
	 *
	 * @param scheduled_stream		The braodcaster's streaming scheduled.
	 *
	 * @return						TRUE if successfully updated the broadcast segment, 
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateChannelStreamScheduleSegment(
			const char *broadcaster_id,
			const char *id,
			const char *start_time,
			const char *duration,
			const char *category_id,
			const char *title,
			const BOOL is_canceled,
			const char *timezone,
			char **scheduled_stream);

	/**
	 * Removes a broadcast segment from the broadcaster's streaming schedule.
	 * NOTE: 	For recurring segments, removing a segment removes all segments
	 * 			in the recurring schedule.
	 *
	 * Requires a user access token that includes the channel:mahange:schedule scope.
	 *
	 * @param braodcaster_id		The ID of the broadcaster that owns he streaming schedule.
	 * 								This iD must match the user ID in the user access token.
	 *
	 * @param id					The ID of the broadcast segment to remove.
	 *
	 * @return						TRUE if successfully removed the broadcast segment,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteChannelStreamScheduleSegment(const char *broadcaster_id, const char *id);
	// ============================== //

	// ========== SEARCH ========== //
	
	/**
	 * Gets the games or categories that match the specified query.
	 *
	 * To match, the category's name must contain all parts of the query string.
	 * For example, if the query string is 42, the response includes any category
	 * name that contains 42 in the title.
	 * If the query string is a phrase like "love computer", the response includes
	 * any category name that contains the words "love" and "computer" anywhere in the name.
	 * The comparision is case insensitive.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param query					The URI-encoded search string.
	 * 								For example, encode #archery as %23archery and search
	 * 								strings like "angel of death" as angel%20of%20death.
	 *
	 * @param matches				The list of games or categories that match the query,
	 * 								is empty if there are no matches.
	 *
	 * @return						TRUE if successfully retrieved the list of category names
	 * 								that matched the specified query string, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSearchCategories(const char *query, char **matches);

	/**
	 * Gets the channels that match the specified query and have streamed content within the past 6 months.
	 * The fields that the API uses for comparison depends in the value that the 
	 * live_only query parameter is set to.
	 * If live_only is FALSE, the API matches on the broadcaster's login name. However, if live_only
	 * is TRUE, the API matches on the broadcaster's name and category name.
	 * To match, the beginning of the broadcaster's name or category must match the query string.
	 * The comparison is case insensitive. If the query string is angle_of_death, it matches all names
	 * that begin with angel_of_death.
	 * However, if the query string is a phrase like "angel of death", it matches to names starting with
	 * "angelofdeath" or names starting with angel_of_death.
	 * 
	 * Requires an app access token or user access token.
	 *
	 * @param query					The URI-encoded search string.
	 * 								For example, encode search string like "angel of death"
	 * 								as "angel%20of%20death".
	 *
	 * @param live_only				A boolean value that determines whether the response includes only
	 * 								channels that are currently streaming live.
	 * 								Set to TRUE to get only channels that are streaming live, otherwise,
	 * 								FALSE to get live and offline channels.
	 *
	 * @param matches				The list of channels that match the query, empty if there are no matches.
	 *
	 * @return						TRUE if successfully retrieved the list of category names
	 * 								that matched the specified query string, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSearchChannels(const char *query, const BOOL live_only, char **matches);
	// ============================ //

	// ========== STREAMS ========== //
	
	/**
	 * Gets the channel's stream key.
	 * 
	 * Requires a user access token that includes the channel:read:stream_key scope.
	 *
	 * @return						TRUE if successfully retrieved the stream's key, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetStreamKey(const char *broadcaster_id, char **stream_key);

	/**
	 * Gets a list of all streams.
	 * The list is in descending order by the number of viewers watching the stream.
	 * Because viewers come and go during a stream, it's possible to find duplicate
	 * or missing streams in the list as you page through the results.
	 *
	 * Requires am app access token or user access token.
	 *
	 * @param user_id				A user ID used to filter the list of streams.
	 * 								Returns only ther streams of those users that are broadcasting.
	 * 								To specify multiple IDs, include the user_id parameter for each
	 * 								user separeted by |, for example 1234|348489.
	 * 								You may specify  a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @aparam user_login			A user login name used to filter the list of streams.
	 * 								Returns only ther streams of those users that are broadcasting.
	 * 								To specify multiple names, include the user_login parameter for each
	 * 								user separeted by |, for example foo|bar.
	 * 								You may specify  a maximum of 100 login names.
	 * 								It can be NULL or an empty string.
	 * 
	 * @aparam game_id				A game (category) ID used to filter the list of streams.
	 * 								Returns only ther streams that are broadcasting the game (category).
	 * 								To specify multiple names, include the user_login parameter for each
	 * 								user separeted by |, for example foo|bar.
	 * 								You may specify  a maximum of 100 IDs.
	 * 								It can be NULL or an empty string.
	 *
	 * @param type					The type of the stream to filter the list of streams by.
	 * 								Possible values are:
	 * 									* all
	 * 									* live
	 * 								The default is 'all'.
	 * 								It can be NULL or an empty string.
	 *
	 * @param language				A language code used to filter the list of streams.
	 * 								Returns only stream that broadcast in the specific language.
	 * 								Specify the language using an ISO 639-1 two-letter language code
	 * 								or 'other' if the broadcast uses a language not in the list  of
	 * 								supported stream languages.
	 *
	 * 								To specify multiple languages, include the language parameter
	 * 								for each language separated by |, for example de|fr.
	 * 								You may specify a maximum of 100 language codes.
	 * 								It can be NULL or an empty string.
	 * 
	 * @return						TRUE if successfully retrieved the list of stream, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetStreams(
			const char *user_id,
			const char *user_login,
			const char *game_id,
			const char *type,
			const char *language,
			char **streams);

	/**
	 * Gets the list of broadcasters that the user follows and that are streaming live.
	 *
	 * Requires a user access token that includes the user:read:follows scope.
	 *
	 * @param user_id				The ID of the user whose list of followed streams you want to get.
	 * 								This ID must match the user ID in the access token.
	 * 
	 * @return						TRUE if successfully retrieved the list of broadcasters that
	 * 								the user follows and that are streaming live, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetFollowedStreams(const char *user_id, char **followed_streams);

	/**
	 * Adds a marker to a live stream.
	 * A marker is an arbitrary point in a live stream that the broadcaster or editor wants
	 * to mark, so they can return to that spot later to create video highlights (see Video Producer,
	 * Highlights in the Twitch UX).
	 *
	 * You may not add markers:
	 * 	* if the stream is not live
	 * 	* if the stream has not enabled video on demand (VOD)
	 * 	* if hte stream is a premire (a live, first-viewing event that combines uploaded videos with live chat)
	 * 	* if the stream is a rerun of a past broadcast, including past premieres
	 *
	 * Requires a user access token that includes the channel:manage:broadcast scope.
	 *
	 * @param user_id				The ID of the broadcaster that's streaming content.
	 * 								This ID must match the user ID in the access token or the user in the
	 * 								access token must be one of the broadcaster's editors.
	 * 
	 * @param description			A short description of the marker to help the user remember why they marked
	 * 								the location, it can be NULL or an empty string.
	 * 								The maximum length of the description is 140 characters.
	 * 
	 * @param marker				The marker we created.
	 * 
	 * @return						TRUE if successfully created the marker, FALSE otherwise.								
	 *
	 */
	TWL_API BOOL TWLCreateStreamMarker(const char *user_id, const char description[141], char **marker);

	/**
	 * Gets a list of markers from the user's most recent stream or from the specified VOD/video.
	 * A marker is an arbitrary point in a live stream that the broadcaster or editor marked, so they
	 * can return to that spot later to create video highlights (see Video Producer, Highlights in the Twitch UX).
	 *
	 * Requires a user access token that includes the user:read:broadcast or channel:manage:broadcast or channel_editor scope.
	 *
	 * @param user_id				A user ID.
	 * 								The request returns the markers from this user's most recent video.
	 * 								This ID must match the user ID in the access token or the user in the access
	 * 								token must be one of the broadcaster's editors.
	 * 								This parameter and the video_id parameter are mutually exclusive.
	 * 
	 * @param video_id				A video on demand (VOD)/video ID.
	 * 								The request returns the markers from this VOD/video.
	 * 								The user in the access token must own the video or the user must be one
	 * 								of the broadcaster's editors.
	 * 								This parameter and the user_id parameter are mutually exclusive.
	 * 
	 * @param markers				The list of markers grouped by the user that created the marks.
	 * 
	 * @return						TRUE if successfully retrieved the list of markers, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetStreamMarkers(const char *user_id, const char *video_id, char **markers);
	// ============================= //
	
	// ========== SUBSCRIPTIONS ========== //
	
	/**
	 * Gets a list of users that subscribe to the specified broadcaster.
	 *
	 * Requires a user access token that includes the channel:read:subscriptions scope.
	 * A Twitch exstensions may use an app access token if the broadcaster has granted the
	 * channel:read:subscriptions scope from within the Twitch Extensions manager.
	 *
	 * @param broadcaster_id		The boradcaster ID.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param user_id				Filters the list to include only the specified subscribers.
	 * 								To specify more than one subscriber, include this parameter for
	 * 								each subscriber separated by |, for example, 1234|5678.
	 * 								You may specify a maximum of 100 subscribers.
	 * 								It can be NULL or an empty string.
	 *
	 * @param subscribers			The list of users subscribed to the broadcaster.
	 * 								The list is empty if the broadcaster has no subscribers.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's list of subscribers,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetBroadcasterSubscriptions(const char *broadcaster_id, const char *user_id, char **subscribers);

	/**
	 * Checks whether te user subscribes to the broadcaster's channel.
	 *
	 * Requires a user access token that includes the user:read:subscriptions scope.
	 * A twitch extensions may use an app access token if the broadcaster has granted the
	 * user:read:subscriptions scope from whithin the Twitch Extensions manager.
	 *
	 * @param broadcaster_id		The ID of a partner or affiliate broadcaster.
	 *
	 * @param user_id				The ID of the user that you're checking to see whether they subscribe to
	 * 								the broadcaster in broadcaster_id.
	 * 								This ID must match the user ID in the access token.
	 *
	 * @param subscription			The user's subscription.
	 *
	 * @return						TRUE if successfully get the user subscribes to the broadcaster,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLCheckUserSubscription(const char *broadcaster_id, const char *user_id, char **subscription);
	// =================================== //
	
	// ========== TAGS ========== //
	// DEPRECATED
	// ========================== //

	// ========== TEAMS ========== //
	
	/**
	 * Gets the list of Twitch teams that the broadcaster is a member of.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param broadcaster_id		The ID og the broadcaster whose teams you want to get.
	 *
	 * @param teams					The list of teams that the broadcaster is a member of.
	 * 								Return an empty array if the broadcaster is not a member
	 * 								of a team.
	 *
	 * @return						TRUE if successfully retrieved the list of teams,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetChannelTeams(const char *broadcaster_id, char **teams);

	/**
	 * Gets information about the specified Twitch team.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param name					The name of the team to get.
	 * 								This parameter and the "id" parameter are mutually exclusive,
	 * 								you must specify the team's name or ID but not both.
	 *
	 * @param id					The ID of the team to get.
	 * 								This parameter and the "name" parameter are mutually exclusive,
	 * 								you must specify the team's name or ID but not both.
	 *
	 * @param team					The team that you requested.
	 *
	 * @return						TRUE if successfully retrieved the team's information, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetTeams(const char *name, const char *id, char **team);
	// =========================== //
	
	// ========== USERS ========== //		
	
	/**
	 * Get user information about one or more users.
	 * You may look up users using their user ID, login name, or noth but the sum total of the number of
	 * users you may look up is 100.
	 * For example, you may specify 50 IDs and 50 names or 100IDs or names, but you cannot
	 * specify 100 IDs and 100 names.
	 * If you don't spacify IDs or login names, the request returns information about the user
	 * in the access token if you specify a user access token.
	 * To include the user's verified email address in the response, you must use a user access token
	 * that includes the user:read:email scope.
	 * 
	 * Requires an app access token or user access token,
	 *
	 * @param id					The ID of the user to get.
	 * 								To specify more than one user, include the id parameter for each
	 * 								user to get separated by |, for example 1234|5678.
	 * 								The maximum number of IDs you may specify is 100.
	 * 								It can be NULL or an empty string.
	 * 
	 * @param login					The login name of the user to get.
	 * 								To specify more than one user, include the login parameter for each
	 * 								user to get separated by |, for example foo|bar.
	 * 								The maximum number of login mnames you may specify is 100.
	 * 								It can be NULL or an empty string.
	 * 
	 * @param users					The list of users.
	 * 
	 * @return						TRUE if successfully retrieved the specified user's information,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUsers(const char *id, const char *login, char **users);
	
	/**
	 * Update the specified user's information.
	 * The user ID in the OAuth token identifies the user whose information you want to update.
	 * To include the user's verified email in the response, the user access token must
	 * include the user:read:email scope.
	 *
	 * Requires a user access token that includes the user:edit scope.
	 *
	 * @param description			The string to update the channel's description to.
	 * 								The description is limited to a maximum of 300 characters.
	 * 								To remove the description, specify this parameter but don't set it's
	 * 								value (for example ?description=, in our case means an empty string "").
	 *
	 * @param user					The user that you updated.
	 *
	 * @return						TRUE if successfully updated the specified user's informastion,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateUser(const char description[301], char **user);

	/**
	 * Gets the list of users that the broadcaster has blocked.
	 *
	 * Requires a user access token that includes the user:read:blocked_users scope.
	 *
	 * @param broadcaster_id		The ID of he broadcaster whose list of blocked
	 * 								users you want to get.
	 *
	 * @param blocked_users			The list of blocked users.
	 * 								The list is in descending order by when the user was blocked.
	 *
	 * @return						TRUE if successfully retrieved the broadcaster's list of
	 * 								blocked users, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUserBlockList(const char *broadcaster_id, char **blocked_users);

	/**
	 * Blocks the specified user from interacting with or having contact with the broadcaster.
	 * The user ID in the OAuth token identifies the broadcaster who is blocking the user.
	 *
	 * Requires a user access token that includes the user:manage:blocked_users scope.
	 *
	 * @param target_user_id		The ID of the user to block.
	 * 								The API ignores the request if the broadcaster
	 * 								has already blocked the user.
	 *
	 * @param source_context		The location where the harassment took place that
	 * 								is causing the broadcaster to block the user (can be NULL).
	 * 								Possible values are:
	 * 									* chat
	 * 									* whisper
	 *
	 * @param reason				The reason that the broadcaster is blocking the user (can be NULL).
	 * 								Possible values are:
	 * 									* harassment
	 * 									* spam
	 * 									* other
	 *
	 * @return						TRUE if successfully blocked the user, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLBlockUser(const char *target_user_id, const char *source_context, const char *reason);

	/**
	 * Removes the user from the broadcaster's list of blocked users.
	 * The user ID in the OAuth token identifies the broadcaster who's removing the block.
	 *
	 * Requires a user access token that includes the user:manage:blocked_users scope.
	 *
	 * @param target_user_id		The ID of the user to remove from the 
	 * 								broadcaster's list of blocked users.
	 * 								The API ignores the request if the broadcaster
	 * 								hasn't blocked the user.
	 *
	 * @return						TRUE if successfully removed the block, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUnblockUser(const char *target_user_id);

	/**
	 * Gets a list of all extensions (both active and inactive) that the broadcaster has installed.
	 * The user ID in the access token identifies the broadcaster.
	 *
	 * Requires a user access token that includes the user:read:broadcast or user:edit:broadcast scope.
	 * To include inactive extensions, you must include the user:edit:broadcast scope.
	 *
	 * @param extensions			The list of extensions that the user has installed.
	 *
	 * @return						TRUE if successfully retrieved the user's installed extensions,
	 * 								FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUserExtensions(char **extensions);

	/**
	 * Gets the active extensions that the broadcaster has installed for each donfiguration.
	 * To include extensions that you have under development, you must specify a
	 * user access token that includes the user:read:broadcast or user:edit:broadcast scope.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param user_id				The ID of the broadcaster whose active extensions you want to get.
	 * 								This parameter is required if you specify an app access token and optional
	 * 								if you specify a user access token.
	 * 								If you specify a user access token and don't specify this parameter, the API uses
	 * 								the user ID from the access token.
	 * 								It can be NULL or an empty string.
	 *
	 * @param active_extensions		The active extensions that the broadcaster has installed.
	 *
	 * @return						TRUE if successfully retrieved the user's active extensions, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetUserActiveExtensions(const char *user_id, char **active_extensions);

	/**
	 * Updates an installed extension's information.
	 * You can update the extension's activation state, ID and version number.
	 * The user ID in the access token identifies the broadcaster whose extensions you're updating.
	 * 
	 * NOTE:	If you try to activate an extension under multiple extensions types, the last write wins
	 * 			(and there is no guarantee of write order).
	 *
	 * Requires a user access token that includes the user:edit:broadcast scope.
	 *
	 * @param data					The extensions to update.
	 * 								The data is a dictionary of extensions types.
	 * 								The dictionary's possible keys are: 
	 * 									panel
	 * 									overlay
	 * 									component
	 * 								The key's value is a dictionary of extensions.
	 *
	 * 								For the extension's dictionary, the key is a sequential number 
	 * 								beginning with 1.
	 * 								
	 * 								For panel and overlay extensions, the key's value is an object that
	 * 								contains the following fields:
	 * 									active (true/false)
	 * 									id (the extension's ID)
	 * 									version (the extension's version)
	 *
	 * 								For component extensions, the key's value includes the above fields
	 * 								plus the 'x' and 'y' fields, which identify the coordinate where the
	 * 								extension is placed.
	 *
	 * 								NOTE: You must pass the whole json object, for example:
	 * 								"data": {
     *									"panel": {
     *									  "1": {
     *									    "active": true,
     *									    "id": "rh6jq1q334hqc2rr1qlzqbvwlfl3x0",
     *									    "version": "1.1.0"
     *									  },
     *									  "2": {
     *									    "active": true,
     *									    "id": "wi08ebtatdc7oj83wtl9uxwz807l8b",
     *									    "version": "1.1.8"
     *									  },
     *									  "3": {
     *									    "active": true,
     *									    "id": "naty2zwfp7vecaivuve8ef1hohh6bo",
     *									    "version": "1.0.9"
     *									  }
     *									},
     *									"overlay": {
     *									  "1": {
     *									    "active": true,
     *									    "id": "zfh2irvx2jb4s60f02jq0ajm8vwgka",
     *									    "version": "1.0.19"
     *									  }
     *									},
     *									"component": {
     *									  "1": {
     *									    "active": true,
     *									    "id": "lqnf3zxk0rv0g7gq92mtmnirjz2cjj",
     *									    "version": "0.0.1",
     *									    "x": 0,
     *									    "y": 0
     *									  },
     *									  "2": {
     *									    "active": false
     *									  }
     *									}
  	 *								}
	 *
	 * @param updated_extensions	The extensions that the broadcaster updated.
	 *
	 * @return						TRUE if successfully updated the active extensions, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLUpdateUserExtensions(const char *data, char **updated_extensions);
	// =========================== //		

	// ========== VIDEOS ========== //
	
	/**
	 * Gets information about one or more published videos.
	 * You may get videos by ID, by user, or by game/category.
	 * You may apply several filters to get a subset of the videos, the filters are
	 * applied as an AND operation to each video.
	 * For example, if "language" is set to 'de' and "game_id" is set to 21779, the response includes
	 * only videos that show playing League of Legends by users that stream in German.
	 * The filters apply only if you get videos by user ID or game ID.
	 *
	 * IMPORTANT: The id, user_id and game_id parameters are mutually exclusive.
	 *
	 * Requires an app access token or user access token.
	 *
	 * @param id					A list of IDs that identify the videos you want to get.
	 * 								To get more than one video, include this parameter for each video 
	 * 								you want to get separated by |. For example, 1234|5678.
	 * 								You may specify a maximum of 100 IDs, the endpoint ignores
	 * 								duplicate IDs and IDs that weren't found (if there's at least one valid ID).
	 *
	 * @param user_id				The ID of the user whose list of videos you want to get.
	 *
	 * @param game_id				A category or game ID.
	 * 								The response contains a maximum of 500 videos that show this content.
	 * 								To get category/game IDs, use the TWLSearchCategories function.
	 *
	 * @param language				A filter used to filter the list of videos by the language that
	 * 								the video owner broadcast in.
	 * 								For example, to get videos that were broadcast in German, set
	 * 								this parameter to ISO  639-1 two letter code for German (DE).
	 * 								For a list of supported languages, see 
	 * 								https://help.twitch.tv/s/article/languages-on-twitch?language=en_US#streamlang
	 * 								If the language is not supported, use 'other'.
	 * 								
	 * 								Specify this paramter only if you specify the 'game_id' parameter.
	 * 								It can be NULL or an empty string.
	 *
	 * @param period				A filter used to filter the list of videos by when they were published.
	 * 								For example, videos published in the last week.
	 * 								Possible values are:
	 * 									* all
	 * 									* day
	 * 									* month
	 * 									* week
	 *
	 * 								The default is 'all', which returns videos published in all periods.
	 *
	 * 								Specify this parameter only if you specify the 'game_id' or
	 * 								'user_id' parameter.
	 * 								It can be NULL or an empty string.
	 *
	 * @param sort					The order to sort the returned videos in.
	 * 								Possible values are:
	 * 									* time - Sort the results in descending order by when they were created.
	 * 									* trending - Sort the results in descending order by biggest gains in viewership.
	 * 									* views - Sort the results in descending order by most views.
	 *
	 * 								The default is 'time'.
	 * 								Specify this parameter only if you specify the 'game_id' or 'user_id' parameter.
	 * 								It can be NULL or an empty string.
	 *
	 * @param type					A filter used to filter the list of videos by the video's type.
	 * 								Possible case-sensitive values are:
	 * 									* all
	 * 									* archive - On-demand videos (VODs) of past streams.
	 * 									* highlight - Highlight reels of past streams.
	 * 									* upload - External videos that the broadcaster uploaded
	 * 												using the Video Producer.
	 *
	 * 								The default is 'all', which returns all video types.
	 *
	 * 								Specify this paramter only if you specify the 'game_id' or 'user_id' parameter.
	* 								It can be NULL or an empty string.
	 *
	 * @param videos				The list of published videos that match the filter criteria.
	 *
	 * @return						TRUE if successfully retrieved the list of videos, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLGetVideos(
			const char *id, 
			const char *user_id, 
			const char *game_id, 
			const char *language,
			const char *period,
			const char *sort,
			const char *type,
			char **videos);

	/**
	 * Deletes one or more videos.
	 * You may delete past broadcasts, highlights or uploads.
	 *
	 * Requires a user access token that includes the channel:manage:videos scope.
	 *
	 * @param id					The list of videos to delete.
	 * 								To specify more than one video, include the id parameter for each
	 * 								video to delete separated by |, for example 1234|5678.
	 * 								You can delete a maximum of 5 videos per request.
	 * 								Ignores invalid video IDs.
	 * 								If the user doesn't have permission to delete one of the videos in the list,
	 * 								none of the videos are deleted.
	 *
	 * @param deleted_videos		The list of IDs of the videos that were deleted.
	 *
	 * @return						TRUE if successfully deleted the videos, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLDeleteVideos(const char *id, char **deleted_videos);
	// ============================ //

	// ========== WHISPERS ========== //
	
	/**
	 * Sends a whisper message to the specified user.
	 *
	 * NOTE:		The user sending the whisper must have a verified phone number.
	 *
	 * NOTE:		The API may silently drop whispers that it suspects of violating
	 * 				Twitch policies. (The API does not indicate that it dropped the
	 * 				whispers, it returns a 204 status code as if it succeeded.)
	 *
	 * RATE LIMITS:	You may whisper to a maximum of 40 unique recipients per day.
	 * 				Within the per day limit, you may whisper a maximum of 3 whispers
	 * 				per second and a maximum of 100 whispers per minute.
	 *
	 * Requires a user access token that includes the user:manage:whispers scope.
	 *
	 * @param from_user_id			The ID of the user sending the whisper.
	 * 								This user must have a verified phone number.
	 * 								This ID must match the user ID in the user access token.
	 *
	 * @param to_user_id			The ID of the user to receive the whisper.
	 *
	 * @param message				The whisper message to send.
	 * 								The message must not be empty.
	 * 								The maximum message lengths are:
	 * 									* 500 characters if the user you're sending the message
	 * 									  to hasn't whispered you before.
	 *
	 * 									* 10000 characters if the user you're sending the message
	 * 									  to has whispered you before,
	 *
	 * 								Messages that exceed the maximum length are truncated.
	 *
	 * @return						TRUE if successfully sent the whisper message or
	 * 								the message was silently dropped, FALSE otherwise.
	 *
	 */
	TWL_API BOOL TWLSendWhisper(const char *from_user_id, const char *to_user_id, const char *message);
	// ============================== //


	// ========== LIBRARY UTILITIES ========== //
	
	/**
	 * Free a pointer to the data internally allocated by any TWLGetX functions.
	 * 
	 * @param						Pointer to the data we want to free.
	 *
	 */
	TWL_API void TWLFree(void *data);
	// ======================================= //
	

	// ========== LIBRARY INTERNALS ========== //
	
	/*
	 * Make a request to the twitch api server.
	 *
	 * @param verb					Type of request (GET, POST, DELETE ..).

	 * @param request_object		The header of the request.
	 
	 * @param request_body			The body of the request (can be "" or NULL).
	 
	 * @param status_code			The server response status code (200, 400 ..).
	 
	 * @return						The request handle if sucessful, NULL otherwise.
	 *
	 */
	HINTERNET TWLMakeRequest(const wchar_t *verb, const wchar_t *request_query, const char *request_body, int *status_code);
	
	/**
	 * Read data, allocate memory and close the handle, call this function
	 * after TWLMakeRequest.
	 *
	 * IMPORTANT:	The returned data pointer can be freed with a call to TWLFree.
	 * IMPORTANT: 	if you pass the returned data pointer to a parser function, the allocated memory will
	 * 				be freed by the parser function ONLY in case the function is successful.
	 *
	 * @param						Handle to a request.
	 * 
	 * @return						A pointer to the readed data, NULL if something went wrong.
	 * 
	 */
	char *TWLReadData(const HINTERNET hRequest);

	/**
	 * Read and print the Headers of the request.
	 *
	 * @param						Handle to a request.
	 *
	 */
	void TWLDebugQueryHeaders(const HINTERNET hRequest);
	// ======================================= //

#ifdef __cplusplus
}
#endif

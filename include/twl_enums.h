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

#ifdef __cplusplus
extern "C" {
#endif

	// ========== ENUMS ========== //
	typedef enum ETWLBroadcasterType  {
		TWL_BT_NONE = 0,
		TWL_BT_AFFILIATE,
		TWL_BT_PARTNER,
		TWL_BT_NORMAL,
	} ETWLBroadcasterType;

	typedef enum ETWLUserType {
		TWL_UT_NONE = 0,
		TWL_UT_ADMIN,
		TWL_UT_GLOBAL_MOD,
		TWL_UT_STAFF,
		TWL_UT_NORMAL,
	} ETWLUserType;

	typedef enum ETWLCCLabel {
		TWL_CCL_DEBATE = 0,			// DebatedSocialIssuesAndPolitics
		TWL_CCL_DRUGS,				// DrugsIntoxication
		TWL_CCL_SEX_THEMES,			// SexualThemes
		TWL_CCL_VIOLENT_GRAPHIC,	// ViolentGraphic
		TLW_CCL_GAMBLING,			// Gambling
		TWL_CCL_PROFANITY,			// ProfanityVulgarity
	} ETWLCCLabel;

	typedef enum ETWLChannelUpdateMask {
		TWL_CU_SET_NONE 		= 0x00,
		TWL_CU_SET_GAME_ID		= 0x01,		
		TWL_CU_SET_LANGUAGE		= 0x02,
		TWL_CU_SET_TITLE		= 0x04,
		TWL_CU_SET_DELAY		= 0x08,
		TWL_CU_SET_TAGS			= 0x10,
		TWL_CU_SET_CCL			= 0x20,
		TWL_CU_SET_BRANDED		= 0x40,
		TWL_CU_SET_RESERVED		= 0x80,
		TWL_CU_SET_ALL			= 0xFF,
	} ETWLChannelUpdateMask;
	// =========================== //
	
#ifdef __cplusplus
}
#endif

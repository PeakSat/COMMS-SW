#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpsabi" // Suppress: parameter passing for argument of type 'Time::DefaultCUC' {aka 'TimeStamp<4, 0, 1, 10>'} changed in GCC 7.1

#include "Helpers/Parameter.hpp"

namespace PeakSatParameters {
    /**
     * ID enumeration of OBDH subsystem's specific parameters.
     */
    enum ParameterID : uint16_t {
        init = 0,
        OBCUseRTT = 8,
        OBCUseUART = 9,
        OBCUseCAN = 10,

        /* OBDH Parameters */
        OBCPCBTemperature1 = 5000,
        OBCPCBTemperature2 = 5001,
        OBCMCUTemperature = 5002,
        OBCMCUInputVoltage = 5003,
        OBCMCUBootCounter = 5004,
        OBCFlashInt = 5005,
        OBCSRAMInt = 5006,
        OBCAvailableMRAM = 5007,
        OBCAvailableNAND = 5008,
        OBCSpacecraftTimeRef = 5009, ///< which subsystem holds the correct time
        OBCOnBoardTime = 5010,
        OBCOperationalMode = 5011,
        OBCMemoryPartition = 5012,
        OBCReconfigurationTimer = 5013, ///< timer responsible to reset the spacecraft to a known working state unless it has been
                                        ///< reset by a ground station pass.
        OBCLastFailedEvent = 5014,
        OBCMCUSystick = 5015,
        OBCCANBUSLoad1 = 5016, ///< the CAN bus load is based on the used capacity (bandwidth) divided by maximum capacity
        OBCCANBUSLoad2 = 5017,
        OBCCANBUSActive = 5018,
        OBCMCUFDIR = 5019,
        OBCMCURestartSafeModeThreshold = 5020,
        OBCNANDFLASHLCLThreshold = 5021,
        OBCMRAMLCLThreshold = 5022,
        OBCNANDFLASHON = 5023,
        OBCMRAMON = 5024,
        OBCNANDFLASHScrubbingFrequency = 5025,
        OBCRAMScrubbingFrequency = 5026,
        OBCProgramFlashScrubbingFrequency = 5027,

        /* EPS Parameters */
        EPS_MODE = 3000,
        EPS_CONF = 3001,
        EPS_RESET_CAUSE = 3002,
        EPS_UPTIME = 3003,
        EPS_ERROR = 3004,
        EPS_RC_CNT_PWRON = 3005,
        EPS_RC_CNT_WDG = 3006,
        EPS_RC_CNT_CMD = 3007,
        EPS_RC_CNT_MCU = 3008,
        EPS_RC_CNT_EMLOPO = 3009,
        EPS_PREVCMD_ELAPSED = 3010,
        EPS_UNIX_TIME = 3011,
        EPS_UNIX_YEAR = 3012,
        EPS_UNIX_MONTH = 3013,
        EPS_UNIX_DAY = 3014,
        EPS_UNIX_HOUR = 3015,
        EPS_UNIX_MINUTE = 3016,
        EPS_UNIX_SECOND = 3017,
        EPS_STAT_CH_ON = 3018,
        EPS_STAT_CH_EXT_ON = 3019,
        EPS_STAT_CH_OCF = 3020,
        EPS_STAT_CH_EXT_OCF = 3021,
        EPS_OCF_CNT_CH00 = 3022,
        EPS_OCF_CNT_CH01 = 3023,
        EPS_OCF_CNT_CH02 = 3024,
        EPS_OCF_CNT_CH03 = 3025,
        EPS_OCF_CNT_CH04 = 3026,
        EPS_OCF_CNT_CH05 = 3027,
        EPS_OCF_CNT_CH06 = 3028,
        EPS_OCF_CNT_CH07 = 3029,
        EPS_OCF_CNT_CH08 = 3030,
        EPS_OCF_CNT_CH09 = 3031,
        EPS_OCF_CNT_CH10 = 3032,
        EPS_OCF_CNT_CH11 = 3033,
        EPS_OCF_CNT_CH12 = 3034,
        EPS_OCF_CNT_CH13 = 3035,
        EPS_OCF_CNT_CH14 = 3036,
        EPS_OCF_CNT_CH15 = 3037,
        EPS_ABF_PLACED_0 = 3038,
        EPS_ABF_PLACED_1 = 3039,
        EPS_VOLT_BRDSUP_RAW = 3040,
        EPS_TEMP_MCU_RAW = 3041,
        EPS_VIP_VOLT_VD0_RAW = 3042,
        EPS_VIP_CURR_VD0_RAW = 3043,
        EPS_VIP_POWE_VD0_RAW = 3044,
        EPS_VIP_VOLT_VD1_RAW = 3045,
        EPS_VIP_CURR_VD1_RAW = 3046,
        EPS_VIP_POWE_VD1_RAW = 3047,
        EPS_VIP_VOLT_VD2_RAW = 3048,
        EPS_VIP_CURR_VD2_RAW = 3049,
        EPS_VIP_POWE_VD2_RAW = 3050,
        EPS_VIP_VOLT_VD3_RAW = 3051,
        EPS_VIP_CURR_VD3_RAW = 3052,
        EPS_VIP_POWE_VD3_RAW = 3053,
        EPS_VIP_VOLT_VD4_RAW = 3054,
        EPS_VIP_CURR_VD4_RAW = 3055,
        EPS_VIP_POWE_VD4_RAW = 3056,
        EPS_VIP_VOLT_VD5_RAW = 3057,
        EPS_VIP_CURR_VD5_RAW = 3058,
        EPS_VIP_POWE_VD5_RAW = 3059,
        EPS_VIP_VOLT_VD6_RAW = 3060,
        EPS_VIP_CURR_VD6_RAW = 3061,
        EPS_VIP_POWE_VD6_RAW = 3062,
        EPS_VOLT_BRDSUP_ENG = 3063,
        EPS_TEMP_MCU_ENG = 3064,
        EPS_VIP_VOLT_INPUT_ENG = 3065,
        EPS_VIP_CURR_INPUT_ENG = 3066,
        EPS_VIP_POWE_INPUT_ENG = 3067,
        EPS_VIP_VOLT_DIST_INPUT_ENG = 3068,
        EPS_VIP_CURR_DIST_INPUT_ENG = 3069,
        EPS_VIP_POWE_DIST_INPUT_ENG = 3070,
        EPS_VIP_VOLT_BAT_INPUT_ENG = 3071,
        EPS_VIP_CURR_BAT_INPUT_ENG = 3072,
        EPS_VIP_POWE_BAT_INPUT_ENG = 3073,
        EPS_VIP_VOLT_VD0_ENG = 3074,
        EPS_VIP_CURR_VD0_ENG = 3075,
        EPS_VIP_POWE_VD0_ENG = 3076,
        EPS_VIP_VOLT_VD1_ENG = 3077,
        EPS_VIP_CURR_VD1_ENG = 3078,
        EPS_VIP_POWE_VD1_ENG = 3079,
        EPS_VIP_VOLT_VD2_ENG = 3080,
        EPS_VIP_CURR_VD2_ENG = 3081,
        EPS_VIP_POWE_VD2_ENG = 3082,
        EPS_VIP_VOLT_VD3_ENG = 3083,
        EPS_VIP_CURR_VD3_ENG = 3084,
        EPS_VIP_POWE_VD3_ENG = 3085,
        EPS_VIP_VOLT_VD4_ENG = 3086,
        EPS_VIP_CURR_VD4_ENG = 3087,
        EPS_VIP_POWE_VD4_ENG = 3088,
        EPS_VIP_VOLT_VD5_ENG = 3089,
        EPS_VIP_CURR_VD5_ENG = 3090,
        EPS_VIP_POWE_VD5_ENG = 3091,
        EPS_VIP_VOLT_VD6_ENG = 3092,
        EPS_VIP_CURR_VD6_ENG = 3093,
        EPS_VIP_POWE_VD6_ENG = 3094,
        EPS_VIP_CH00_VOLT_RAW = 3095,
        EPS_VIP_CH00_CURR_RAW = 3096,
        EPS_VIP_CH00_POWE_RAW = 3097,
        EPS_VIP_CH01_VOLT_RAW = 3098,
        EPS_VIP_CH01_CURR_RAW = 3099,
        EPS_VIP_CH01_POWE_RAW = 3100,
        EPS_VIP_CH02_VOLT_RAW = 3101,
        EPS_VIP_CH02_CURR_RAW = 3102,
        EPS_VIP_CH02_POWE_RAW = 3103,
        EPS_VIP_CH03_VOLT_RAW = 3104,
        EPS_VIP_CH03_CURR_RAW = 3105,
        EPS_VIP_CH03_POWE_RAW = 3106,
        EPS_VIP_CH04_VOLT_RAW = 3107,
        EPS_VIP_CH04_CURR_RAW = 3108,
        EPS_VIP_CH04_POWE_RAW = 3109,
        EPS_VIP_CH05_VOLT_RAW = 3110,
        EPS_VIP_CH05_CURR_RAW = 3111,
        EPS_VIP_CH05_POWE_RAW = 3112,
        EPS_VIP_CH06_VOLT_RAW = 3113,
        EPS_VIP_CH06_CURR_RAW = 3114,
        EPS_VIP_CH06_POWE_RAW = 3115,
        EPS_VIP_CH07_VOLT_RAW = 3116,
        EPS_VIP_CH07_CURR_RAW = 3117,
        EPS_VIP_CH07_POWE_RAW = 3118,
        EPS_VIP_CH08_VOLT_RAW = 3119,
        EPS_VIP_CH08_CURR_RAW = 3120,
        EPS_VIP_CH08_POWE_RAW = 3121,
        EPS_VIP_CH09_VOLT_RAW = 3122,
        EPS_VIP_CH09_CURR_RAW = 3123,
        EPS_VIP_CH09_POWE_RAW = 3124,
        EPS_VIP_CH10_VOLT_RAW = 3125,
        EPS_VIP_CH10_CURR_RAW = 3126,
        EPS_VIP_CH10_POWE_RAW = 3127,
        EPS_VIP_CH11_VOLT_RAW = 3128,
        EPS_VIP_CH11_CURR_RAW = 3129,
        EPS_VIP_CH11_POWE_RAW = 3130,
        EPS_VIP_CH12_VOLT_RAW = 3131,
        EPS_VIP_CH12_CURR_RAW = 3132,
        EPS_VIP_CH12_POWE_RAW = 3133,
        EPS_VIP_CH13_VOLT_RAW = 3134,
        EPS_VIP_CH13_CURR_RAW = 3135,
        EPS_VIP_CH13_POWE_RAW = 3136,
        EPS_VIP_CH14_VOLT_RAW = 3137,
        EPS_VIP_CH14_CURR_RAW = 3138,
        EPS_VIP_CH14_POWE_RAW = 3139,
        EPS_VIP_CH00_VOLT_ENG = 3140,
        EPS_VIP_CH00_CURR_ENG = 3141,
        EPS_VIP_CH00_POWE_ENG = 3142,
        EPS_VIP_CH01_VOLT_ENG = 3143,
        EPS_VIP_CH01_CURR_ENG = 3144,
        EPS_VIP_CH01_POWE_ENG = 3145,
        EPS_VIP_CH02_VOLT_ENG = 3146,
        EPS_VIP_CH02_CURR_ENG = 3147,
        EPS_VIP_CH02_POWE_ENG = 3148,
        EPS_VIP_CH03_VOLT_ENG = 3149,
        EPS_VIP_CH03_CURR_ENG = 3150,
        EPS_VIP_CH03_POWE_ENG = 3151,
        EPS_VIP_CH04_VOLT_ENG = 3152,
        EPS_VIP_CH04_CURR_ENG = 3153,
        EPS_VIP_CH04_POWE_ENG = 3154,
        EPS_VIP_CH05_VOLT_ENG = 3155,
        EPS_VIP_CH05_CURR_ENG = 3156,
        EPS_VIP_CH05_POWE_ENG = 3157,
        EPS_VIP_CH06_VOLT_ENG = 3158,
        EPS_VIP_CH06_CURR_ENG = 3159,
        EPS_VIP_CH06_POWE_ENG = 3160,
        EPS_VIP_CH07_VOLT_ENG = 3161,
        EPS_VIP_CH07_CURR_ENG = 3162,
        EPS_VIP_CH07_POWE_ENG = 3163,
        EPS_VIP_CH08_VOLT_ENG = 3164,
        EPS_VIP_CH08_CURR_ENG = 3165,
        EPS_VIP_CH08_POWE_ENG = 3166,
        EPS_VIP_CH09_VOLT_ENG = 3167,
        EPS_VIP_CH09_CURR_ENG = 3168,
        EPS_VIP_CH09_POWE_ENG = 3169,
        EPS_VIP_CH10_VOLT_ENG = 3170,
        EPS_VIP_CH10_CURR_ENG = 3171,
        EPS_VIP_CH10_POWE_ENG = 3172,
        EPS_VIP_CH11_VOLT_ENG = 3173,
        EPS_VIP_CH11_CURR_ENG = 3174,
        EPS_VIP_CH11_POWE_ENG = 3175,
        EPS_VIP_CH12_VOLT_ENG = 3176,
        EPS_VIP_CH12_CURR_ENG = 3177,
        EPS_VIP_CH12_POWE_ENG = 3178,
        EPS_VIP_CH13_VOLT_ENG = 3179,
        EPS_VIP_CH13_CURR_ENG = 3180,
        EPS_VIP_CH13_POWE_ENG = 3181,
        EPS_VIP_CH14_VOLT_ENG = 3182,
        EPS_VIP_CH14_CURR_ENG = 3183,
        EPS_VIP_CH14_POWE_ENG = 3184,
        EPS_VIP_CH00_VOLT_RA = 3185,
        EPS_VIP_CH00_CURR_RA = 3186,
        EPS_VIP_CH00_POWE_RA = 3187,
        EPS_VIP_CH01_VOLT_RA = 3188,
        EPS_VIP_CH01_CURR_RA = 3189,
        EPS_VIP_CH01_POWE_RA = 3190,
        EPS_VIP_CH02_VOLT_RA = 3191,
        EPS_VIP_CH02_CURR_RA = 3192,
        EPS_VIP_CH02_POWE_RA = 3193,
        EPS_VIP_CH03_VOLT_RA = 3194,
        EPS_VIP_CH03_CURR_RA = 3195,
        EPS_VIP_CH03_POWE_RA = 3196,
        EPS_VIP_CH04_VOLT_RA = 3197,
        EPS_VIP_CH04_CURR_RA = 3198,
        EPS_VIP_CH04_POWE_RA = 3199,
        EPS_VIP_CH05_VOLT_RA = 3200,
        EPS_VIP_CH05_CURR_RA = 3201,
        EPS_VIP_CH05_POWE_RA = 3202,
        EPS_VIP_CH06_VOLT_RA = 3203,
        EPS_VIP_CH06_CURR_RA = 3204,
        EPS_VIP_CH06_POWE_RA = 3205,
        EPS_VIP_CH07_VOLT_RA = 3206,
        EPS_VIP_CH07_CURR_RA = 3207,
        EPS_VIP_CH07_POWE_RA = 3208,
        EPS_VIP_CH08_VOLT_RA = 3209,
        EPS_VIP_CH08_CURR_RA = 3210,
        EPS_VIP_CH08_POWE_RA = 3211,
        EPS_VIP_CH09_VOLT_RA = 3212,
        EPS_VIP_CH09_CURR_RA = 3213,
        EPS_VIP_CH09_POWE_RA = 3214,
        EPS_VIP_CH10_VOLT_RA = 3215,
        EPS_VIP_CH10_CURR_RA = 3216,
        EPS_VIP_CH10_POWE_RA = 3217,
        EPS_VIP_CH11_VOLT_RA = 3218,
        EPS_VIP_CH11_CURR_RA = 3219,
        EPS_VIP_CH11_POWE_RA = 3220,
        EPS_VIP_CH12_VOLT_RA = 3221,
        EPS_VIP_CH12_CURR_RA = 3222,
        EPS_VIP_CH12_POWE_RA = 3223,
        EPS_VIP_CH13_VOLT_RA = 3224,
        EPS_VIP_CH13_CURR_RA = 3225,
        EPS_VIP_CH13_POWE_RA = 3226,
        EPS_VIP_CH14_VOLT_RA = 3227,
        EPS_VIP_CH14_CURR_RA = 3228,
        EPS_VIP_CH14_POWE_RA = 3229,
        EPS_VOLT_BRDSUP_RA = 3230,
        EPS_TEMP_MCU_RA = 3231,
        EPS_VIP_VOLT_INPUT_RA = 3232,
        EPS_VIP_CURR_INPUT_RA = 3233,
        EPS_VIP_POWE_INPUT_RA = 3234,
        EPS_VIP_VOLT_DIST_INPUT_RA = 3235,
        EPS_VIP_CURR_DIST_INPUT_RA = 3236,
        EPS_VIP_POWE_DIST_INPUT_RA = 3237,
        EPS_VIP_VOLT_BAT_INPUT_RA = 3238,
        EPS_VIP_CURR_BAT_INPUT_RA = 3239,
        EPS_VIP_POWE_BAT_INPUT_RA = 3240,
        EPS_VIP_VOLT_VD0_RA = 3241,
        EPS_VIP_CURR_VD0_RA = 3242,
        EPS_VIP_POWE_VD0_RA = 3243,
        EPS_VIP_VOLT_VD1_RA = 3244,
        EPS_VIP_CURR_VD1_RA = 3245,
        EPS_VIP_POWE_VD1_RA = 3246,
        EPS_VIP_VOLT_VD2_RA = 3247,
        EPS_VIP_CURR_VD2_RA = 3248,
        EPS_VIP_POWE_VD2_RA = 3249,
        EPS_VIP_VOLT_VD3_RA = 3250,
        EPS_VIP_CURR_VD3_RA = 3251,
        EPS_VIP_POWE_VD3_RA = 3252,
        EPS_VIP_VOLT_VD4_RA = 3253,
        EPS_VIP_CURR_VD4_RA = 3254,
        EPS_VIP_POWE_VD4_RA = 3255,
        EPS_VIP_VOLT_VD5_RA = 3256,
        EPS_VIP_CURR_VD5_RA = 3257,
        EPS_VIP_POWE_VD5_RA = 3258,
        EPS_VIP_VOLT_VD6_RA = 3259,
        EPS_VIP_CURR_VD6_RA = 3260,
        EPS_VIP_POWE_VD6_RA = 3261,
        EPS_STAT_BU = 3262,
        EPS_VOLT_BP1_INPUT_RAW = 3263,
        EPS_CURR_BP1_INPUT_RAW = 3264,
        EPS_POWE_BP1_INPUT_RAW = 3265,
        EPS_STAT_BP1_RAW = 3266,
        EPS_VOLT_BP1_CELL1_RAW = 3267,
        EPS_VOLT_BP1_CELL2_RAW = 3268,
        EPS_VOLT_BP1_CELL3_RAW = 3269,
        EPS_VOLT_BP1_CELL4_RAW = 3270,
        EPS_BAT_TEMP1_BP1_RAW = 3271,
        EPS_BAT_TEMP2_BP1_RAW = 3272,
        EPS_BAT_TEMP3_BP1_RAW = 3273,
        EPS_VOLT_BP2_INPUT_RAW = 3274,
        EPS_CURR_BP2_INPUT_RAW = 3275,
        EPS_POWE_BP2_INPUT_RAW = 3276,
        EPS_STAT_BP2_RAW = 3277,
        EPS_VOLT_BP2_CELL1_RAW = 3278,
        EPS_VOLT_BP2_CELL2_RAW = 3279,
        EPS_VOLT_BP2_CELL3_RAW = 3280,
        EPS_VOLT_BP2_CELL4_RAW = 3281,
        EPS_BAT_TEMP1_BP2_RAW = 3282,
        EPS_BAT_TEMP2_BP2_RAW = 3283,
        EPS_BAT_TEMP3_BP2_RAW = 3284,
        EPS_VOLT_BP3_INPUT_RAW = 3285,
        EPS_CURR_BP3_INPUT_RAW = 3286,
        EPS_POWE_BP3_INPUT_RAW = 3287,
        EPS_STAT_BP3_RAW = 3288,
        EPS_VOLT_BP3_CELL1_RAW = 3289,
        EPS_VOLT_BP3_CELL2_RAW = 3290,
        EPS_VOLT_BP3_CELL3_RAW = 3291,
        EPS_VOLT_BP3_CELL4_RAW = 3292,
        EPS_BAT_TEMP1_BP3_RAW = 3293,
        EPS_BAT_TEMP2_BP3_RAW = 3294,
        EPS_BAT_TEMP3_BP3_RAW = 3295,
        EPS_VOLT_BP1_INPUT_ENG = 3296,
        EPS_CURR_BP1_INPUT_ENG = 3297,
        EPS_POWE_BP1_INPUT_ENG = 3298,
        EPS_STAT_BP1_ENG = 3299,
        EPS_VOLT_BP1_CELL1_ENG = 3300,
        EPS_VOLT_BP1_CELL2_ENG = 3301,
        EPS_VOLT_BP1_CELL3_ENG = 3302,
        EPS_VOLT_BP1_CELL4_ENG = 3303,
        EPS_BAT_TEMP1_BP1_ENG = 3304,
        EPS_BAT_TEMP2_BP1_ENG = 3305,
        EPS_BAT_TEMP3_BP1_ENG = 3306,
        EPS_VOLT_BP2_INPUT_ENG = 3307,
        EPS_CURR_BP2_INPUT_ENG = 3308,
        EPS_POWE_BP2_INPUT_ENG = 3309,
        EPS_STAT_BP2_ENG = 3310,
        EPS_VOLT_BP2_CELL1_ENG = 3311,
        EPS_VOLT_BP2_CELL2_ENG = 3312,
        EPS_VOLT_BP2_CELL3_ENG = 3313,
        EPS_VOLT_BP2_CELL4_ENG = 3314,
        EPS_BAT_TEMP1_BP2_ENG = 3315,
        EPS_BAT_TEMP2_BP2_ENG = 3316,
        EPS_BAT_TEMP3_BP2_ENG = 3317,
        EPS_VOLT_BP3_INPUT_ENG = 3318,
        EPS_CURR_BP3_INPUT_ENG = 3319,
        EPS_POWE_BP3_INPUT_ENG = 3320,
        EPS_STAT_BP3_ENG = 3321,
        EPS_VOLT_BP3_CELL1_ENG = 3322,
        EPS_VOLT_BP3_CELL2_ENG = 3323,
        EPS_VOLT_BP3_CELL3_ENG = 3324,
        EPS_VOLT_BP3_CELL4_ENG = 3325,
        EPS_BAT_TEMP1_BP3_ENG = 3326,
        EPS_BAT_TEMP2_BP3_ENG = 3327,
        EPS_BAT_TEMP3_BP3_ENG = 3328,
        EPS_VOLT_BP1_INPUT_RA = 3329,
        EPS_CURR_BP1_INPUT_RA = 3330,
        EPS_POWE_BP1_INPUT_RA = 3331,
        EPS_STAT_BP1_RA = 3332,
        EPS_VOLT_BP1_CELL1_RA = 3333,
        EPS_VOLT_BP1_CELL2_RA = 3334,
        EPS_VOLT_BP1_CELL3_RA = 3335,
        EPS_VOLT_BP1_CELL4_RA = 3336,
        EPS_BAT_TEMP1_BP1_RA = 3337,
        EPS_BAT_TEMP2_BP1_RA = 3338,
        EPS_BAT_TEMP3_BP1_RA = 3339,
        EPS_VOLT_BP2_INPUT_RA = 3340,
        EPS_CURR_BP2_INPUT_RA = 3341,
        EPS_POWE_BP2_INPUT_RA = 3342,
        EPS_STAT_BP2_RA = 3343,
        EPS_VOLT_BP2_CELL1_RA = 3344,
        EPS_VOLT_BP2_CELL2_RA = 3345,
        EPS_VOLT_BP2_CELL3_RA = 3346,
        EPS_VOLT_BP2_CELL4_RA = 3347,
        EPS_BAT_TEMP1_BP2_RA = 3348,
        EPS_BAT_TEMP2_BP2_RA = 3349,
        EPS_BAT_TEMP3_BP2_RA = 3350,
        EPS_VOLT_BP3_INPUT_RA = 3351,
        EPS_CURR_BP3_INPUT_RA = 3352,
        EPS_POWE_BP3_INPUT_RA = 3353,
        EPS_STAT_BP3_RA = 3354,
        EPS_VOLT_BP3_CELL1_RA = 3355,
        EPS_VOLT_BP3_CELL2_RA = 3356,
        EPS_VOLT_BP3_CELL3_RA = 3357,
        EPS_VOLT_BP3_CELL4_RA = 3358,
        EPS_BAT_TEMP1_BP3_RA = 3359,
        EPS_BAT_TEMP2_BP3_RA = 3360,
        EPS_BAT_TEMP3_BP3_RA = 3361,
        EPS_VOLT_VD0_RAW = 3362,
        EPS_VOLT_VD1_RAW = 3363,
        EPS_VOLT_VD2_RAW = 3364,
        EPS_VOLT_VD0_ENG = 3365,
        EPS_VOLT_VD1_ENG = 3366,
        EPS_VOLT_VD2_ENG = 3367,
        EPS_VOLT_VD0_RA = 3368,
        EPS_VOLT_VD1_RA = 3369,
        EPS_VOLT_VD2_RA = 3370,
        EPS_BAT_STAT = 3371,
        EPS_BAT_TEMP2_RAW = 3372,
        EPS_BAT_TEMP3_RAW = 3373,
        EPS_BAT_TEMP2_ENG = 3374,
        EPS_BAT_TEMP3_ENG = 3375,
        EPS_BAT_TEMP2_RA = 3376,
        EPS_BAT_TEMP3_RA = 3377,
        EPS_CC1_VOLT_IN_MPPT_RAW = 3378,
        EPS_CC2_VOLT_IN_MPPT_RAW = 3379,
        EPS_CC3_VOLT_IN_MPPT_RAW = 3380,
        EPS_CC4_VOLT_IN_MPPT_RAW = 3381,
        EPS_CC5_VOLT_IN_MPPT_RAW = 3382,
        EPS_CC1_CURR_IN_MPPT_RAW = 3383,
        EPS_CC2_CURR_IN_MPPT_RAW = 3384,
        EPS_CC3_CURR_IN_MPPT_RAW = 3385,
        EPS_CC4_CURR_IN_MPPT_RAW = 3386,
        EPS_CC5_CURR_IN_MPPT_RAW = 3387,
        EPS_CC1_VOLT_OU_MPPT_RAW = 3388,
        EPS_CC2_VOLT_OU_MPPT_RAW = 3389,
        EPS_CC3_VOLT_OU_MPPT_RAW = 3390,
        EPS_CC4_VOLT_OU_MPPT_RAW = 3391,
        EPS_CC5_VOLT_OU_MPPT_RAW = 3392,
        EPS_CC1_CURR_OU_MPPT_RAW = 3393,
        EPS_CC2_CURR_OU_MPPT_RAW = 3394,
        EPS_CC3_CURR_OU_MPPT_RAW = 3395,
        EPS_CC4_CURR_OU_MPPT_RAW = 3396,
        EPS_CC5_CURR_OU_MPPT_RAW = 3397,
        EPS_CC1_VOLT_IN_MPPT_ENG = 3398,
        EPS_CC2_VOLT_IN_MPPT_ENG = 3399,
        EPS_CC3_VOLT_IN_MPPT_ENG = 3400,
        EPS_CC4_VOLT_IN_MPPT_ENG = 3401,
        EPS_CC5_VOLT_IN_MPPT_ENG = 3402,
        EPS_CC1_CURR_IN_MPPT_ENG = 3403,
        EPS_CC2_CURR_IN_MPPT_ENG = 3404,
        EPS_CC3_CURR_IN_MPPT_ENG = 3405,
        EPS_CC4_CURR_IN_MPPT_ENG = 3406,
        EPS_CC5_CURR_IN_MPPT_ENG = 3407,
        EPS_CC1_VOLT_OU_MPPT_ENG = 3408,
        EPS_CC2_VOLT_OU_MPPT_ENG = 3409,
        EPS_CC3_VOLT_OU_MPPT_ENG = 3410,
        EPS_CC4_VOLT_OU_MPPT_ENG = 3411,
        EPS_CC5_VOLT_OU_MPPT_ENG = 3412,
        EPS_CC1_CURR_OU_MPPT_ENG = 3413,
        EPS_CC2_CURR_OU_MPPT_ENG = 3414,
        EPS_CC3_CURR_OU_MPPT_ENG = 3415,
        EPS_CC4_CURR_OU_MPPT_ENG = 3416,
        EPS_CC5_CURR_OU_MPPT_ENG = 3417,
        EPS_CC1_VOLT_IN_MPPT_RA = 3418,
        EPS_CC2_VOLT_IN_MPPT_RA = 3419,
        EPS_CC3_VOLT_IN_MPPT_RA = 3420,
        EPS_CC4_VOLT_IN_MPPT_RA = 3421,
        EPS_CC5_VOLT_IN_MPPT_RA = 3422,
        EPS_CC1_CURR_IN_MPPT_RA = 3423,
        EPS_CC2_CURR_IN_MPPT_RA = 3424,
        EPS_CC3_CURR_IN_MPPT_RA = 3425,
        EPS_CC4_CURR_IN_MPPT_RA = 3426,
        EPS_CC5_CURR_IN_MPPT_RA = 3427,
        EPS_CC1_VOLT_OU_MPPT_RA = 3428,
        EPS_CC2_VOLT_OU_MPPT_RA = 3429,
        EPS_CC3_VOLT_OU_MPPT_RA = 3430,
        EPS_CC4_VOLT_OU_MPPT_RA = 3431,
        EPS_CC5_VOLT_OU_MPPT_RA = 3432,
        EPS_CC1_CURR_OU_MPPT_RA = 3433,
        EPS_CC2_CURR_OU_MPPT_RA = 3434,
        EPS_CC3_CURR_OU_MPPT_RA = 3435,
        EPS_CC4_CURR_OU_MPPT_RA = 3436,
        EPS_CC5_CURR_OU_MPPT_RA = 3437,
        EPS_CH_STARTUP_ENA_BF = 3438,
        EPS_CH_STARTUP_KEY = 3439,
        EPS_CH_LATCHOFF_ENA_BF = 3440,
        EPS_CH_LATCHOFF_KEY = 3441,
        EPS_TTC_WDG_TIMEOUT = 3442,
        EPS_TTC_WDG_TIMEOUT_KEY = 3443,
        EPS_CH_STARTUP_DELAY_CH1 = 3444,
        EPS_CH_LATCHOFF_DELAY_CH1 = 3445,
        EPS_SAFETY_VOLT_LOTHR = 3446,
        EPS_SAFETY_VOLT_HITHR = 3447,
        EPS_LOTHR_BP1_HEATER = 3448,
        EPS_LOTHR_BP2_HEATER = 3449,
        EPS_LOTHR_BP3_HEATER = 3450,
        EPS_HITHR_BP1_HEATER = 3451,
        EPS_HITHR_BP2_HEATER = 3452,
        EPS_HITHR_BP3_HEATER = 3453,
        EPS_LOTHR_BP1_UNBAL = 3454,
        EPS_LOTHR_BP2_UNBAL = 3455,
        EPS_LOTHR_BP3_UNBAL = 3456,
        EPS_HITHR_BP1_UNBAL = 3457,
        EPS_HITHR_BP2_UNBAL = 3458,
        EPS_HITHR_BP3_UNBAL = 3459,
        EPS_MCU_TEMP_BIAS = 3460,
        EPS_MCU_TEMP_PREMUL = 3461,
        EPS_MCU_TEMP_POSDIV = 3462,
        EPS_BP1_TEMP1_BIAS = 3463,
        EPS_BP1_TEMP2_BIAS = 3464,
        EPS_BP1_TEMP3_BIAS = 3465,
        EPS_BP2_TEMP1_BIAS = 3466,
        EPS_BP2_TEMP2_BIAS = 3467,
        EPS_BP2_TEMP3_BIAS = 3468,
        EPS_BP3_TEMP1_BIAS = 3469,
        EPS_BP3_TEMP2_BIAS = 3470,
        EPS_BP3_TEMP3_BIAS = 3471,
        EPS_BP1_TEMP1_PREMUL = 3472,
        EPS_BP1_TEMP2_PREMUL = 3473,
        EPS_BP1_TEMP3_PREMUL = 3474,
        EPS_BP2_TEMP1_PREMUL = 3475,
        EPS_BP2_TEMP2_PREMUL = 3476,
        EPS_BP2_TEMP3_PREMUL = 3477,
        EPS_BP3_TEMP1_PREMUL = 3478,
        EPS_BP3_TEMP2_PREMUL = 3479,
        EPS_BP3_TEMP3_PREMUL = 3480,
        EPS_BP1_TEMP1_POSDIV = 3481,
        EPS_BP1_TEMP2_POSDIV = 3482,
        EPS_BP1_TEMP3_POSDIV = 3483,
        EPS_BP2_TEMP1_POSDIV = 3484,
        EPS_BP2_TEMP2_POSDIV = 3485,
        EPS_BP2_TEMP3_POSDIV = 3486,
        EPS_BP3_TEMP1_POSDIV = 3487,
        EPS_BP3_TEMP2_POSDIV = 3488,
        EPS_BP3_TEMP3_POSDIV = 3489,
        EPS_BOARD_IDENTIFIER = 3490,
        EPS_BOARD_IDENTIFIER_KEY = 3491,
        EPS_RAVG_STRENGTH_P2 = 3492,
        EPS_AUTO_HEAT_ENA_BP1 = 3493,
        EPS_AUTO_HEAT_ENA_BP2 = 3494,
        EPS_AUTO_HEAT_ENA_BP3 = 3495,
        EPS_AUTO_BAL_ENA_BP1 = 3496,
        EPS_AUTO_BAL_ENA_BP2 = 3497,
        EPS_AUTO_BAL_ENA_BP3 = 3498,
        EPS_VD1_ALWAYS_ENA = 3499,
        EPS_VD1_ALWAYS_DISA = 3500,
        EPS_CH_FORCE_ENA_USE_BF = 3501,
        EPS_CH_STARTUP_ENA_USE_BF = 3502,
        EPS_CH_LATCHOFF_ENA_USE_BF = 3503,
        EPS_VD1_ALLOC_CH_BF = 3504,
        EPS_SWCI_CH_CMD_ENA_BF = 3505,
        EPS_SWCI_CH_CMD_DISA_BF = 3506,
        EPS_TTC_I2C_SLAVE_ADDR = 3507,
        EPS_CONF_NVM_SAVE_CNTR = 3508,
        EPS_CONF_NVM_SAVE_CHKS = 3509,
        EPS_RST_CAUSE = 3510,
        EPS_RST_CNTR_PWRON = 3511,
        EPS_RST_CNTR_WDG = 3512,
        EPS_RST_CNTR_CMD = 3513,
        EPS_RST_CNTR_MCU = 3514,
        EPS_RST_CNTR_EMLOPO = 3515,
        EPS_RST_CODE_MCU_RAW = 3516,
        EPS_EMLOPO_VOLT_LOTHR = 3517,
        EPS_EMLOPO_VOLT_HITHR = 3518,
        EPS_EMLOPO_PERIOD = 3519,
        EPS_SAFETY_VOLT_LOTHR_USED = 3520,
        EPS_SAFETY_VOLT_HITHR_USED = 3521,
        EPS_SAFETY_LINGER = 3522,
        EPS_TTC_WDG_TIMOUT_USED = 3523,
        EPS_TTC_PREVCMD_ELAPSED = 3524,
        EPS_STID = 3525,
        EPS_IVID = 3526,
        EPS_BID_USED = 3527,
        EPS_BOOT_RESUME_SHORT = 3528,
        EPS_CONF_PARAM_CHANGED = 3529,

        EPS_VIP_VOLT_INPUT_RAW = 3530,
        EPS_VIP_CURR_INPUT_RAW = 3531,
        EPS_VIP_POWE_INPUT_RAW = 3532,

        EPS_VIP_CC1_OUTPUT_VOLT_RAW = 3533,
        EPS_VIP_CC1_OUTPUT_CURR_RAW = 3534,
        EPS_VIP_CC1_OUTPUT_POWE_RAW = 3535,
        EPS_VIP_CC2_OUTPUT_VOLT_RAW = 3536,
        EPS_VIP_CC2_OUTPUT_CURR_RAW = 3537,
        EPS_VIP_CC2_OUTPUT_POWE_RAW = 3538,
        EPS_VIP_CC3_OUTPUT_VOLT_RAW = 3539,
        EPS_VIP_CC3_OUTPUT_CURR_RAW = 3560,
        EPS_VIP_CC3_OUTPUT_POWE_RAW = 3561,
        EPS_VIP_CC4_OUTPUT_VOLT_RAW = 3562,
        EPS_VIP_CC4_OUTPUT_CURR_RAW = 3563,
        EPS_VIP_CC4_OUTPUT_POWE_RAW = 3564,
        EPS_VIP_CC5_OUTPUT_VOLT_RAW = 3565,
        EPS_VIP_CC5_OUTPUT_CURR_RAW = 3566,
        EPS_VIP_CC5_OUTPUT_POWE_RAW = 3567,

        EPS_VIP_CC1_OUTPUT_VOLT_ENG = 3568,
        EPS_VIP_CC1_OUTPUT_CURR_ENG = 3569,
        EPS_VIP_CC1_OUTPUT_POWE_ENG = 3570,
        EPS_VIP_CC2_OUTPUT_VOLT_ENG = 3571,
        EPS_VIP_CC2_OUTPUT_CURR_ENG = 3572,
        EPS_VIP_CC2_OUTPUT_POWE_ENG = 3573,
        EPS_VIP_CC3_OUTPUT_VOLT_ENG = 3574,
        EPS_VIP_CC3_OUTPUT_CURR_ENG = 3575,
        EPS_VIP_CC3_OUTPUT_POWE_ENG = 3576,
        EPS_VIP_CC4_OUTPUT_VOLT_ENG = 3577,
        EPS_VIP_CC4_OUTPUT_CURR_ENG = 3578,
        EPS_VIP_CC4_OUTPUT_POWE_ENG = 3579,
        EPS_VIP_CC5_OUTPUT_VOLT_ENG = 3580,
        EPS_VIP_CC5_OUTPUT_CURR_ENG = 3581,
        EPS_VIP_CC5_OUTPUT_POWE_ENG = 3582,

        EPS_VIP_CC1_OUTPUT_VOLT_RA = 3583,
        EPS_VIP_CC1_OUTPUT_CURR_RA = 3584,
        EPS_VIP_CC1_OUTPUT_POWE_RA = 3585,
        EPS_VIP_CC2_OUTPUT_VOLT_RA = 3586,
        EPS_VIP_CC2_OUTPUT_CURR_RA = 3587,
        EPS_VIP_CC2_OUTPUT_POWE_RA = 3588,
        EPS_VIP_CC3_OUTPUT_VOLT_RA = 3589,
        EPS_VIP_CC3_OUTPUT_CURR_RA = 3590,
        EPS_VIP_CC3_OUTPUT_POWE_RA = 3591,
        EPS_VIP_CC4_OUTPUT_VOLT_RA = 3592,
        EPS_VIP_CC4_OUTPUT_CURR_RA = 3593,
        EPS_VIP_CC4_OUTPUT_POWE_RA = 3594,
        EPS_VIP_CC5_OUTPUT_VOLT_RA = 3595,
        EPS_VIP_CC5_OUTPUT_CURR_RA = 3596,
        EPS_VIP_CC5_OUTPUT_POWE_RA = 3597,

        EPS_VIP_VOLT_DIST_INPUT_RAW = 3598,
        EPS_VIP_CURR_DIST_INPUT_RAW = 3599,
        EPS_VIP_POWE_DIST_INPUT_RAW = 3600,
        EPS_VIP_VOLT_BAT_INPUT_RAW = 3601,
        EPS_VIP_CURR_BAT_INPUT_RAW = 3602,
        EPS_VIP_POWE_BAT_INPUT_RAW = 3603,

        EPS_VIP_OUTPUT_VOLT_RAW = 3604,
        EPS_VIP_OUTPUT_CURR_RAW = 3605,
        EPS_VIP_OUTPUT_POWE_RAW = 3606,
        EPS_VIP_OUTPUT_VOLT_ENG = 3607,
        EPS_VIP_OUTPUT_CURR_ENG = 3608,
        EPS_VIP_OUTPUT_POWE_ENG = 3609,
        EPS_VIP_OUTPUT_VOLT_RA = 3610,
        EPS_VIP_OUTPUT_CURR_RA = 3611,
        EPS_VIP_OUTPUT_POWE_RA = 3612,
        EPS_ADC_MCU_TEMP_V25T30 = 3613,
        EPS_ADC_MCU_TEMP_V25T85 = 3614,

        /* COMMS Parameters */
        COMMS_UHFBandPATemperatureID = 10010,
        COMMS_PCBTemperatureID = 10020,
        COMMS_GNSSTemperatureID = 10030,
        Antenna_Deployment_StatusID = 10050,

        /* PAYLOAD Parameters */
        PAY_xID = 15010,
        PAY_yID = 15020,
        PAY_zID = 15030,
        PAY_uptimeID = 15040,
        PAY_timeID = 15050,
        PAY_psu_12vID = 15060,
        PAY_psu_5vID = 15070,
        PAY_psu_33vID = 15080,
        PAY_mcu_die_temperatureID = 15090,
        PAY_main_board_temperatureID = 15100,
        PAY_seed_ld_output_powerID = 15110,
        PAY_fsm_chamber_temperatureID = 15120,
        PAY_fsm_chamber_pressureID = 15130,
        PAY_fsmd_voltage_converter_temperatureID = 15140,
        PAY_fsmd_drivers_temperatureID = 15150,
        PAY_camera_pcb_temperatureID = 15160,
        PAY_camera_sensor_temperatureID = 15170,
        PAY_fso_aux_temperature_1ID = 15171,
        PAY_fso_aux_temperature_2ID = 15172,
        PAY_oad_temperatureID = 15180,
        PAY_optical_amplifier_combiner_heater_temperatureID = 15181,
        PAY_optical_amplifier_fiber_mirror_temperatureID = 15182,
        PAY_optical_amplifier_circulator_heater_temperatureID = 15183,
        PAY_ldd_temperatureID = 15190,
        PAY_ld_temperatureID = 15200,
        PAY_ldd_12v_currentID = 15210,
        PAY_ldd_psu_12vID = 15211,
        PAY_ldd_converter_input_currentID = 15212,
        PAY_ldd_ld_vID = 15213,
        PAY_ldd_ld_dac_set_vID = 15214,
        PAY_fsm_driver_12v_currentID = 15220,
        PAY_flashes_33v_currentID = 15230,
        PAY_fpga_5v_currentID = 15240,
        PAY_sdd_33v_currentID = 15250,
        PAY_pump_ld_powerID = 15260,
        PAY_mcu_33v_currentID = 15270,
        PAY_fpga_die_temperatureID = 15280,
        PAY_fpga_vdd1_voltageID = 15290,
        PAY_fpga_vdd18_voltageID = 15300,
        PAY_fpga_vdd25_voltageID = 15310,
        PAY_seed_ld_incoming_powerID = 15320,
        PAY_amplifier_output_powerID = 15321,
        PAY_amplifier_output_reflected_powerID = 15322,
        PAY_seed_diode_bias_currentID = 15323,
        PAY_tec_currentID = 15330,
        PAY_ldd_output_currentID = 15340,
        PAY_sd_temperature_violationsID = 15341,
        PAY_end_uptimeID = 15350,
        PAY_responseID = 15360,
        PAY_deviceID = 15390,
        PAY_firmwareID = 15400,
        PAY_firmware_is_confirmedID = 15410,
        PAY_bitstreamID = 15420,
        PAY_softcpu_1_firmwareID = 15430,
        PAY_softcpu_2_firmwareID = 15440,
        PAY_softcpu_3_firmwareID = 15450,
        PAY_softcpu_4_firmwareID = 15460,
        PAY_softcpu_5_firmwareID = 15470,
        PAY_softcpu_6_firmwareID = 15480,
        PAY_softcpu_7_firmwareID = 15490,
        PAY_softcpu_8_firmwareID = 15500,
        PAY_boot_countID = 15510,
        PAY_transmission_countID = 15520,
        PAY_hw_detID = 15530,
        PAY_storage0ID = 15540,
        PAY_storage1ID = 15550,
        PAY_storage2ID = 15560,
        PAY_storage3ID = 15570,
        PAY_ldd_faultID = 15580,
        PAY_fsm_faultID = 15590,
        PAY_fpga_faultID = 15600,
        PAY_v_cam_faultID = 15620,
        PAY_sdd_faultID = 15630
    };

    /******************* OBDH ENUMS *******************/
    enum SpacecraftTimeRef : uint8_t {
        Spacecraft = 0,
        GroundStation = 1
    };

    enum OperationalMode : uint8_t {
        CommissioningMode = 0,
        NominalMode = 1,
        ScienceMode = 2,
        SafeMode = 3
    };

    enum MemoryPartition : uint8_t {
        First = 0,
        Second = 1
    };

    enum CANBUSActive : uint8_t {
        Main = 0,
        Reductant = 1
    };

    enum MCUFDIR : uint8_t {
        OBC = 0,
        ADCS = 1
    };

    /******************* COMMS ENUMS *******************/
    enum Antenna_Deployment_Status_enum : uint8_t {
        Closed = 0,
        OneDoorOpen = 1,
        TwoDoorOpen = 2,
        ThreeDoorOpen = 3,
        FullyDeployed = 4
    };

    enum SampleRateUHFTX : uint8_t {
        Rate = 0
    };

    enum AntennaGains : uint8_t {
        Gain = 0
    };

    /******************* OBDH PARAMETERS *******************/
    inline Parameter<bool> obcUseRTT(true);
    inline Parameter<bool> obcUseUART(true);
    inline Parameter<bool> obcUseCAN(false);

    inline Parameter<float> obcPCBTemperature1(0);
    inline Parameter<float> obcPCBTemperature2(0);
    inline Parameter<float> obcMCUTemperature(0);
    inline Parameter<float> obcMCUInputVoltage(0);

    inline Parameter<uint16_t> obcMCUBootCounter(0);
    inline Parameter<uint32_t> obcFlashInt(0);
    inline Parameter<uint32_t> obcSRAMInt(0);
    inline Parameter<uint32_t> obcAvailableMRAM(0);
    inline Parameter<uint32_t> obcAvailableNAND(0);

    inline Parameter<Time::DefaultCUC> obcOnBoardTime(Time::DefaultCUC(0));

    inline Parameter<SpacecraftTimeRef> obcSpacecraftTimeRef(Spacecraft); // enum
    inline Parameter<OperationalMode> obcOperationalMode(NominalMode);    // enum
    inline Parameter<MemoryPartition> obcMemoryPartition(First);          // enum

    inline Parameter<uint32_t> obcReconfigurationTimer(0);
    inline Parameter<uint16_t> obcLastFailedEvent(0);
    inline Parameter<uint32_t> obcMCUSystick(0);

    inline Parameter<float> obcCANBUSLoad1(0);
    inline Parameter<float> obcCANBUSLoad2(0);

    // inline Parameter<CAN::Driver::ActiveBus> obcCANBUSActive(CAN::Driver::Main); // enum
    inline Parameter<MCUFDIR> obcMCUFDIR(OBC);                                   // enum

    inline Parameter<uint8_t> obcMCURestartSafeModeThreshold(0);
    inline Parameter<float> obcNANDFLASHLCLThreshold(0);
    inline Parameter<float> obcMRAMLCLThreshold(0);
    inline Parameter<float> obcNANDFLASHON(0);
    inline Parameter<float> obcNANDFlashScrubbingFrequency(0);
    inline Parameter<float> obcMRAMON(0);
    inline Parameter<float> obcRAMScrubbingFrequency(0);
    inline Parameter<float> obcProgramFlashScrubbingFrequency(0);

    /****************** EPS PARAMETERS *******************/
    inline Parameter<uint8_t> epsMODE(0);
    inline Parameter<uint8_t> epsCONF(0);
    inline Parameter<uint8_t> epsRESET_CAUSE(0);
    inline Parameter<uint32_t> epsUPTIME(0);
    inline Parameter<uint16_t > epsERROR(0);
    inline Parameter<uint16_t > epsRC_CNT_PWRON(0);
    inline Parameter<uint16_t> epsRC_CNT_WDG(0);
    inline Parameter<uint16_t> epsRC_CNT_CMD(0);
    inline Parameter<uint16_t> epsRC_CNT_MCU(0);
    inline Parameter<uint16_t> epsRC_CNT_EMLOPO(0);
    inline Parameter<uint16_t> epsPREVCMD_ELAPSED(0);
    inline Parameter<uint32_t > epsUNIX_TIME(0);
    inline Parameter<uint8_t> epsUNIX_YEAR(0);
    inline Parameter<uint8_t> epsUNIX_MONTH(0);
    inline Parameter<uint8_t > epsUNIX_DAY(0);
    inline Parameter<uint8_t> epsUNIX_HOUR(0);
    inline Parameter<uint8_t> epsUNIX_MINUTE(0);
    inline Parameter<uint8_t > epsUNIX_SECOND(0);

    inline Parameter<uint16_t> epsSTAT_CH_ON(0);
    inline Parameter<uint16_t> epsSTAT_CH_EXT_ON(0);
    inline Parameter<uint16_t> epsSTAT_CH_OCF(0);
    inline Parameter<uint16_t> epsSTAT_CH_EXT_OCF(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH00(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH01(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH02(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH03(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH04(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH05(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH06(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH07(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH08(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH09(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH10(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH11(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH12(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH13(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH14(0);
    inline Parameter<uint16_t> epsOCF_CNT_CH15(0);

    inline Parameter<uint8_t> epsABF_PLACED_0(0);
    inline Parameter<uint8_t> epsABF_PLACED_1(0);

    inline Parameter<uint16_t> epsVOLT_BRDSUP_RAW(0);
    inline Parameter<uint16_t> epsTEMP_MCU_RAW(0);
    inline Parameter<int16_t> epsVOLT_BRDSUP_ENG(0);
    inline Parameter<int16_t> epsTEMP_MCU_ENG(0);
    inline Parameter<int16_t> epsVOLT_BRDSUP_RA(0);
    inline Parameter<int16_t> epsTEMP_MCU_RA(0);

    inline Parameter<int16_t> epsVIP_VOLT_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_INPUT_RAW(0);

    inline Parameter<int16_t> epsDIST_VOLT_INPUT_RAW(0);
    inline Parameter<int16_t> epsDIST_CURR_INPUT_RAW(0);
    inline Parameter<int16_t> epsDIST_POWE_INPUT_RAW(0);

    inline Parameter<int16_t> epsVIP_VOLT_BAT_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_BAT_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_BAT_INPUT_RAW(0);

    inline Parameter<int16_t> epsVIP_VOLT_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_INPUT_ENG(0);

    inline Parameter<int16_t> epsDIST_VOLT_INPUT_ENG(0);
    inline Parameter<int16_t> epsDIST_CURR_INPUT_ENG(0);
    inline Parameter<int16_t> epsDIST_POWE_INPUT_ENG(0);

    inline Parameter<int16_t> epsVIP_VOLT_BAT_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_BAT_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_BAT_INPUT_ENG(0);

    inline Parameter<int16_t> epsVIP_VOLT_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_INPUT_RA(0);

    inline Parameter<int16_t> epsVIP_VOLT_DIST_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_DIST_INPUT_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_DIST_INPUT_RAW(0);

    inline Parameter<int16_t> epsVIP_VOLT_DIST_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_DIST_INPUT_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_DIST_INPUT_ENG(0);

    inline Parameter<int16_t> epsVIP_VOLT_DIST_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_DIST_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_DIST_INPUT_RA(0);

    inline Parameter<int16_t> epsVIP_VOLT_BAT_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_BAT_INPUT_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_BAT_INPUT_RA(0);


    inline Parameter<int16_t> epsVIP_VOLT_VD0_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD0_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD0_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD1_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD1_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD1_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD2_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD2_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD2_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD3_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD3_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD3_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD4_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD4_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD4_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD5_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD5_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD5_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD6_RAW(0);
    inline Parameter<int16_t> epsVIP_CURR_VD6_RAW(0);
    inline Parameter<int16_t> epsVIP_POWE_VD6_RAW(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD0_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD0_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD0_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD1_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD1_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD1_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD2_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD2_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD2_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD3_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD3_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD3_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD4_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD4_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD4_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD5_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD5_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD5_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD6_ENG(0);
    inline Parameter<int16_t> epsVIP_CURR_VD6_ENG(0);
    inline Parameter<int16_t> epsVIP_POWE_VD6_ENG(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD0_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD0_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD0_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD1_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD1_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD1_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD2_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD2_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD2_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD3_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD3_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD3_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD4_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD4_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD4_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD5_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD5_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD5_RA(0);
    inline Parameter<int16_t> epsVIP_VOLT_VD6_RA(0);
    inline Parameter<int16_t> epsVIP_CURR_VD6_RA(0);
    inline Parameter<int16_t> epsVIP_POWE_VD6_RA(0);


    inline Parameter<uint16_t> epsVIP_CH00_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH00_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH00_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH01_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH01_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH01_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH02_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH02_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH02_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH03_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH03_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH03_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH04_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH04_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH04_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH05_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH05_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH05_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH06_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH06_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH06_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH07_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH07_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH07_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH08_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH08_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH08_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH09_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH09_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH09_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH10_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH10_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH10_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH11_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH11_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH11_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH12_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH12_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH12_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH13_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH13_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH13_POWE_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH14_VOLT_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH14_CURR_RAW(0);
    inline Parameter<uint16_t> epsVIP_CH14_POWE_RAW(0);
    inline Parameter<int16_t> epsVIP_CH00_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH00_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH00_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH01_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH01_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH01_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH02_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH02_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH02_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH03_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH03_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH03_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH04_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH04_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH04_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH05_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH05_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH05_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH06_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH06_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH06_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH07_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH07_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH07_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH08_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH08_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH08_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH09_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH09_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH09_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH10_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH10_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH10_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH11_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH11_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH11_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH12_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH12_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH12_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH13_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH13_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH13_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_CH14_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CH14_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CH14_POWE_ENG(0);

    inline Parameter<int16_t> epsVIP_CH00_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH00_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH00_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH01_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH01_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH01_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH02_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH02_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH02_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH03_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH03_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH03_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH04_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH04_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH04_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH05_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH05_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH05_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH06_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH06_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH06_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH07_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH07_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH07_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH08_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH08_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH08_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH09_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH09_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH09_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH10_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH10_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH10_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH11_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH11_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH11_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH12_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH12_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH12_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH13_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH13_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH13_POWE_RA(0);
    inline Parameter<int16_t> epsVIP_CH14_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CH14_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CH14_POWE_RA(0);

    inline Parameter<int16_t> epsSTAT_BU(0);
    inline Parameter<int16_t> epsVOLT_BP1_INPUT_RAW(0);
    inline Parameter<int16_t> epsCURR_BP1_INPUT_RAW(0);
    inline Parameter<int16_t> epsPOWE_BP1_INPUT_RAW(0);

    inline Parameter<uint16_t> epsSTAT_BP1_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP1_CELL1_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP1_CELL2_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP1_CELL3_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP1_CELL4_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP1_BP1_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP2_BP1_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP3_BP1_RAW(0);

    inline Parameter<int16_t> epsVOLT_BP2_INPUT_RAW(0);
    inline Parameter<int16_t> epsCURR_BP2_INPUT_RAW(0);
    inline Parameter<int16_t> epsPOWE_BP2_INPUT_RAW(0);

    inline Parameter<uint16_t> epsSTAT_BP2_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP2_CELL1_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP2_CELL2_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP2_CELL3_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP2_CELL4_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP1_BP2_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP2_BP2_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP3_BP2_RAW(0);

    inline Parameter<int16_t> epsVOLT_BP3_INPUT_RAW(0);
    inline Parameter<int16_t> epsCURR_BP3_INPUT_RAW(0);
    inline Parameter<int16_t> epsPOWE_BP3_INPUT_RAW(0);

    inline Parameter<uint16_t> epsSTAT_BP3_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP3_CELL1_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP3_CELL2_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP3_CELL3_RAW(0);
    inline Parameter<uint16_t> epsVOLT_BP3_CELL4_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP1_BP3_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP2_BP3_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP3_BP3_RAW(0);

    inline Parameter<int16_t> epsVOLT_BP1_INPUT_ENG(0);
    inline Parameter<int16_t> epsCURR_BP1_INPUT_ENG(0);
    inline Parameter<int16_t> epsPOWE_BP1_INPUT_ENG(0);

    inline Parameter<int16_t> epsSTAT_BP1_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL1_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL2_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL3_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL4_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP1_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP1_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP1_ENG(0);

    inline Parameter<int16_t> epsVOLT_BP2_INPUT_ENG(0);
    inline Parameter<int16_t> epsCURR_BP2_INPUT_ENG(0);
    inline Parameter<int16_t> epsPOWE_BP2_INPUT_ENG(0);

    inline Parameter<int16_t> epsSTAT_BP2_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL1_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL2_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL3_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL4_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP2_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP2_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP2_ENG(0);

    inline Parameter<int16_t> epsVOLT_BP3_INPUT_ENG(0);
    inline Parameter<int16_t> epsCURR_BP3_INPUT_ENG(0);
    inline Parameter<int16_t> epsPOWE_BP3_INPUT_ENG(0);

    inline Parameter<int16_t> epsSTAT_BP3_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL1_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL2_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL3_ENG(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL4_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP3_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP3_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP3_ENG(0);

    inline Parameter<int16_t> epsVOLT_BP1_INPUT_RA(0);
    inline Parameter<int16_t> epsCURR_BP1_INPUT_RA(0);
    inline Parameter<int16_t> epsPOWE_BP1_INPUT_RA(0);

    inline Parameter<int16_t> epsSTAT_BP1_RA(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL1_RA(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL2_RA(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL3_RA(0);
    inline Parameter<int16_t> epsVOLT_BP1_CELL4_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP1_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP1_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP1_RA(0);

    inline Parameter<int16_t> epsVOLT_BP2_INPUT_RA(0);
    inline Parameter<int16_t> epsCURR_BP2_INPUT_RA(0);
    inline Parameter<int16_t> epsPOWE_BP2_INPUT_RA(0);

    inline Parameter<int16_t> epsSTAT_BP2_RA(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL1_RA(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL2_RA(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL3_RA(0);
    inline Parameter<int16_t> epsVOLT_BP2_CELL4_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP2_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP2_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP2_RA(0);

    inline Parameter<int16_t> epsVOLT_BP3_INPUT_RA(0);
    inline Parameter<int16_t> epsCURR_BP3_INPUT_RA(0);
    inline Parameter<int16_t> epsPOWE_BP3_INPUT_RA(0);

    inline Parameter<int16_t> epsSTAT_BP3_RA(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL1_RA(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL2_RA(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL3_RA(0);
    inline Parameter<int16_t> epsVOLT_BP3_CELL4_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP1_BP3_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP2_BP3_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP3_BP3_RA(0);


    inline Parameter<int16_t> epsVIP_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_POWE_RAW(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_POWE_ENG(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_OUTPUT_POWE_RA(0);
    inline Parameter<uint16_t> epsVIP_CC1_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_POWE_RAW(0);

    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_POWE_RAW(0);

    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_POWE_RAW(0);

    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_POWE_RAW(0);

    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_VOLT_RAW(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_CURR_RAW(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_POWE_RAW(0);

    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_POWE_ENG(0);

    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_POWE_ENG(0);

    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_POWE_ENG(0);

    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_POWE_ENG(0);

    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_VOLT_ENG(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_CURR_ENG(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_POWE_ENG(0);

    // RA Parameters
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CC1_OUTPUT_POWE_RA(0);

    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CC2_OUTPUT_POWE_RA(0);

    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CC3_OUTPUT_POWE_RA(0);

    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CC4_OUTPUT_POWE_RA(0);

    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_VOLT_RA(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_CURR_RA(0);
    inline Parameter<int16_t> epsVIP_CC5_OUTPUT_POWE_RA(0);

    inline Parameter<uint16_t> epsCC1_VOLT_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC2_VOLT_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC3_VOLT_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC4_VOLT_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC5_VOLT_IN_MPPT_RAW(0);

    inline Parameter<uint16_t> epsCC1_CURR_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC2_CURR_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC3_CURR_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC4_CURR_IN_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC5_CURR_IN_MPPT_RAW(0);

    inline Parameter<uint16_t> epsCC1_VOLT_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC2_VOLT_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC3_VOLT_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC4_VOLT_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC5_VOLT_OU_MPPT_RAW(0);

    inline Parameter<uint16_t> epsCC1_CURR_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC2_CURR_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC3_CURR_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC4_CURR_OU_MPPT_RAW(0);
    inline Parameter<uint16_t> epsCC5_CURR_OU_MPPT_RAW(0);

    inline Parameter<int16_t> epsCC1_VOLT_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC2_VOLT_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC3_VOLT_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC4_VOLT_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC5_VOLT_IN_MPPT_ENG(0);

    inline Parameter<int16_t> epsCC1_CURR_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC2_CURR_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC3_CURR_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC4_CURR_IN_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC5_CURR_IN_MPPT_ENG(0);

    inline Parameter<int16_t> epsCC1_VOLT_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC2_VOLT_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC3_VOLT_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC4_VOLT_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC5_VOLT_OU_MPPT_ENG(0);

    inline Parameter<int16_t> epsCC1_CURR_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC2_CURR_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC3_CURR_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC4_CURR_OU_MPPT_ENG(0);
    inline Parameter<int16_t> epsCC5_CURR_OU_MPPT_ENG(0);

    // RA Parameters
    inline Parameter<int16_t> epsCC1_VOLT_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC2_VOLT_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC3_VOLT_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC4_VOLT_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC5_VOLT_IN_MPPT_RA(0);

    inline Parameter<int16_t> epsCC1_CURR_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC2_CURR_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC3_CURR_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC4_CURR_IN_MPPT_RA(0);
    inline Parameter<int16_t> epsCC5_CURR_IN_MPPT_RA(0);

    inline Parameter<int16_t> epsCC1_VOLT_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC2_VOLT_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC3_VOLT_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC4_VOLT_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC5_VOLT_OU_MPPT_RA(0);

    inline Parameter<int16_t> epsCC1_CURR_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC2_CURR_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC3_CURR_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC4_CURR_OU_MPPT_RA(0);
    inline Parameter<int16_t> epsCC5_CURR_OU_MPPT_RA(0);



    inline Parameter<int16_t> epsBAT_STAT(0);
    inline Parameter<uint16_t> epsBAT_TEMP2_RAW(0);
    inline Parameter<uint16_t> epsBAT_TEMP3_RAW(0);
    inline Parameter<int16_t> epsBAT_TEMP2_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP3_ENG(0);
    inline Parameter<int16_t> epsBAT_TEMP2_RA(0);
    inline Parameter<int16_t> epsBAT_TEMP3_RA(0);

    inline Parameter<uint16_t> epsVOLT_VD0_RAW(0);
    inline Parameter<uint16_t> epsVOLT_VD1_RAW(0);
    inline Parameter<uint16_t> epsVOLT_VD2_RAW(0);
    inline Parameter<int16_t> epsVOLT_VD0_ENG(0);
    inline Parameter<int16_t> epsVOLT_VD1_ENG(0);
    inline Parameter<int16_t> epsVOLT_VD2_ENG(0);
    inline Parameter<int16_t> epsVOLT_VD0_RA(0);
    inline Parameter<int16_t> epsVOLT_VD1_RA(0);
    inline Parameter<int16_t> epsVOLT_VD2_RA(0);

    inline Parameter<uint32_t> epsCH_STARTUP_ENA_BF(0);
    inline Parameter<uint32_t> epsCH_STARTUP_KEY(0);
    inline Parameter<uint32_t> epsCH_LATCHOFF_ENA_BF(0);
    inline Parameter<uint32_t> epsCH_LATCHOFF_KEY(0);

    inline Parameter<uint16_t> epsTTC_WDG_TIMEOUT(0);
    inline Parameter<uint16_t> epsTTC_WDG_TIMEOUT_KEY(0);
    inline Parameter<uint16_t> epsCH_STARTUP_DELAY_CH1(0);
    inline Parameter<uint16_t> epsCH_LATCHOFF_DELAY_CH1(0);
    inline Parameter<uint16_t> epsSAFETY_VOLT_LOTHR(0);
    inline Parameter<uint16_t> epsSAFETY_VOLT_HITHR(0);

    inline Parameter<int16_t> epsLOTHR_BP1_HEATER(0);
    inline Parameter<int16_t> epsLOTHR_BP2_HEATER(0);
    inline Parameter<int16_t> epsLOTHR_BP3_HEATER(0);
    inline Parameter<int16_t> epsHITHR_BP1_HEATER(0);
    inline Parameter<int16_t> epsHITHR_BP2_HEATER(0);
    inline Parameter<int16_t> epsHITHR_BP3_HEATER(0);
    inline Parameter<int16_t> epsLOTHR_BP1_UNBAL(0);
    inline Parameter<int16_t> epsLOTHR_BP2_UNBAL(0);
    inline Parameter<int16_t> epsLOTHR_BP3_UNBAL(0);
    inline Parameter<int16_t> epsHITHR_BP1_UNBAL(0);
    inline Parameter<int16_t> epsHITHR_BP2_UNBAL(0);
    inline Parameter<int16_t> epsHITHR_BP3_UNBAL(0);
    inline Parameter<int16_t> epsMCU_TEMP_BIAS(0);
    inline Parameter<int16_t> epsMCU_TEMP_PREMUL(0);
    inline Parameter<int16_t> epsMCU_TEMP_POSDIV(0);
    inline Parameter<int16_t> epsBP1_TEMP1_BIAS(0);
    inline Parameter<int16_t> epsBP1_TEMP2_BIAS(0);
    inline Parameter<int16_t> epsBP1_TEMP3_BIAS(0);
    inline Parameter<int16_t> epsBP2_TEMP1_BIAS(0);
    inline Parameter<int16_t> epsBP2_TEMP2_BIAS(0);
    inline Parameter<int16_t> epsBP2_TEMP3_BIAS(0);
    inline Parameter<int16_t> epsBP3_TEMP1_BIAS(0);
    inline Parameter<int16_t> epsBP3_TEMP2_BIAS(0);
    inline Parameter<int16_t> epsBP3_TEMP3_BIAS(0);
    inline Parameter<int16_t> epsBP1_TEMP1_PREMUL(0);
    inline Parameter<int16_t> epsBP1_TEMP2_PREMUL(0);
    inline Parameter<int16_t> epsBP1_TEMP3_PREMUL(0);
    inline Parameter<int16_t> epsBP2_TEMP1_PREMUL(0);
    inline Parameter<int16_t> epsBP2_TEMP2_PREMUL(0);
    inline Parameter<int16_t> epsBP2_TEMP3_PREMUL(0);
    inline Parameter<int16_t> epsBP3_TEMP1_PREMUL(0);
    inline Parameter<int16_t> epsBP3_TEMP2_PREMUL(0);
    inline Parameter<int16_t> epsBP3_TEMP3_PREMUL(0);
    inline Parameter<int16_t> epsBP1_TEMP1_POSDIV(0);
    inline Parameter<int16_t> epsBP1_TEMP2_POSDIV(0);
    inline Parameter<int16_t> epsBP1_TEMP3_POSDIV(0);
    inline Parameter<int16_t> epsBP2_TEMP1_POSDIV(0);
    inline Parameter<int16_t> epsBP2_TEMP2_POSDIV(0);
    inline Parameter<int16_t> epsBP2_TEMP3_POSDIV(0);
    inline Parameter<int16_t> epsBP3_TEMP1_POSDIV(0);
    inline Parameter<int16_t> epsBP3_TEMP2_POSDIV(0);
    inline Parameter<int16_t> epsBP3_TEMP3_POSDIV(0);

    inline Parameter<uint8_t> epsBOARD_IDENTIFIER(0);
    inline Parameter<uint8_t> epsBOARD_IDENTIFIER_KEY(0);
    inline Parameter<uint8_t> epsRAVG_STRENGTH_P2(0);

    inline Parameter<int8_t> epsAUTO_HEAT_ENA_BP1(0);
    inline Parameter<int8_t> epsAUTO_HEAT_ENA_BP2(0);
    inline Parameter<int8_t> epsAUTO_HEAT_ENA_BP3(0);
    inline Parameter<int8_t> epsAUTO_BAL_ENA_BP1(0);
    inline Parameter<int8_t> epsAUTO_BAL_ENA_BP2(0);
    inline Parameter<int8_t> epsAUTO_BAL_ENA_BP3(0);
    inline Parameter<int8_t> epsVD1_ALWAYS_ENA(0);
    inline Parameter<int8_t> epsVD1_ALWAYS_DISA(0);

    inline Parameter<uint32_t> epsCH_FORCE_ENA_USE_BF(0);
    inline Parameter<uint32_t> epsCH_STARTUP_ENA_USE_BF(0);
    inline Parameter<uint32_t> epsCH_LATCHOFF_ENA_USE_BF(0);
    inline Parameter<uint32_t> epsVD1_ALLOC_CH_BF(0); // VD1 default, add the offset for other channels, range: (0x680C –0x6812)
    inline Parameter<uint32_t> epsSWCI_CH_CMD_ENA_BF(0);
    inline Parameter<uint32_t> epsSWCI_CH_CMD_DISA_BF(0);

    inline Parameter<uint16_t> epsTTC_I2C_SLAVE_ADDR(0);
    inline Parameter<uint16_t> epsCONF_NVM_SAVE_CNTR(0);
    inline Parameter<uint16_t> epsCONF_NVM_SAVE_CHKS(0);
    inline Parameter<uint16_t> epsRST_CAUSE(0);
    inline Parameter<uint16_t> epsRST_CNTR_PWRON(0);
    inline Parameter<uint16_t> epsRST_CNTR_WDG(0);
    inline Parameter<uint16_t> epsRST_CNTR_CMD(0);
    inline Parameter<uint16_t> epsRST_CNTR_MCU(0);
    inline Parameter<uint16_t> epsRST_CNTR_EMLOPO(0);
    inline Parameter<uint16_t> epsRST_CODE_MCU_RAW(0);
    inline Parameter<uint16_t> epsEMLOPO_VOLT_LOTHR(0);
    inline Parameter<uint16_t> epsEMLOPO_VOLT_HITHR(0);
    inline Parameter<uint16_t> epsEMLOPO_PERIOD(0);
    inline Parameter<uint16_t> epsSAFETY_VOLT_LOTHR_USED(0);
    inline Parameter<uint16_t> epsSAFETY_VOLT_HITHR_USED(0);
    inline Parameter<uint16_t> epsSAFETY_LINGER(0);
    inline Parameter<uint16_t> epsTTC_WDG_TIMOUT_USED(0);
    inline Parameter<uint16_t> epsTTC_PREVCMD_ELAPSED(0);

    inline Parameter<int16_t> epsADC_MCU_TEMP_V25T30(0);
    inline Parameter<int16_t> epsADC_MCU_TEMP_V25T85(0);

    inline Parameter<uint8_t> epsSTID(0);
    inline Parameter<uint8_t> epsIVID(0);
    inline Parameter<uint8_t> epsBID_USED(0);
    inline Parameter<uint8_t> epsBOOT_RESUME_SHORT(0);

    inline Parameter<int8_t> epsCONF_PARAM_CHANGED(0);

    /****************** COMMS PARAMETERS *******************/
    inline Parameter<float> commsUHFBandPATemperature(0);
    inline Parameter<float> commsPCBTemperature(0);
    inline Parameter<float> commsGNSSTemperature(0);
    inline Parameter<Antenna_Deployment_Status_enum> Antenna_Deployment_Status(Closed);

    /****************** PAYLOAD PARAMETERS *******************/
    inline Parameter<int32_t> pay_x(0);
    inline Parameter<int32_t> pay_y(0);
    inline Parameter<int32_t> pay_z(0);
    inline Parameter<int32_t> pay_uptime(0);
    inline Parameter<int32_t> pay_time(0);
    inline Parameter<int32_t> pay_psu_12v(0);
    inline Parameter<int32_t> pay_psu_5v(0);
    inline Parameter<int32_t> pay_psu_33v(0);
    inline Parameter<int32_t> pay_mcu_die_temperature(0);
    inline Parameter<int32_t> pay_main_board_temperature(0);
    inline Parameter<int32_t> pay_seed_ld_output_power(0);
    inline Parameter<int32_t> pay_fsm_chamber_temperature(0);
    inline Parameter<int32_t> pay_fsm_chamber_pressure(0);
    inline Parameter<int32_t> pay_fsmd_voltage_converter_temperature(0);
    inline Parameter<int32_t> pay_fsmd_drivers_temperature(0);
    inline Parameter<int32_t> pay_camera_pcb_temperature(0);
    inline Parameter<int32_t> pay_camera_sensor_temperature(0);
    inline Parameter<int32_t> pay_fso_aux_temperature_1(0);
    inline Parameter<int32_t> pay_fso_aux_temperature_2(0);
    inline Parameter<int32_t> pay_oad_temperature(0);
    inline Parameter<int32_t> pay_optical_amplifier_combiner_heater_temperature(0);
    inline Parameter<int32_t> pay_optical_amplifier_fiber_mirror_temperature(0);
    inline Parameter<int32_t> pay_optical_amplifier_circulator_heater_temperature(0);
    inline Parameter<int32_t> pay_ldd_temperature(0);
    inline Parameter<int32_t> pay_ld_temperature(0);
    inline Parameter<int32_t> pay_ldd_12v_current(0);
    inline Parameter<int32_t> pay_ldd_psu_12v(0);
    inline Parameter<int32_t> pay_ldd_converter_input_current(0);
    inline Parameter<int32_t> pay_ldd_ld_v(0);
    inline Parameter<int32_t> pay_ldd_ld_dac_set_v(0);
    inline Parameter<int32_t> pay_fsm_driver_12v_current(0);
    inline Parameter<int32_t> pay_flashes_33v_current(0);
    inline Parameter<int32_t> pay_fpga_5v_current(0);
    inline Parameter<int32_t> pay_sdd_33v_current(0);
    inline Parameter<int32_t> pay_pump_ld_power(0);
    inline Parameter<int32_t> pay_mcu_33v_current(0);
    inline Parameter<int32_t> pay_fpga_die_temperature(0);
    inline Parameter<int32_t> pay_fpga_vdd1_voltage(0);
    inline Parameter<int32_t> pay_fpga_vdd18_voltage(0);
    inline Parameter<int32_t> pay_fpga_vdd25_voltage(0);
    inline Parameter<int32_t> pay_seed_ld_incoming_power(0);
    inline Parameter<int32_t> pay_amplifier_output_power(0);
    inline Parameter<int32_t> pay_amplifier_output_reflected_power(0);
    inline Parameter<int32_t> pay_seed_diode_bias_current(0);
    inline Parameter<int32_t> pay_tec_current(0);
    inline Parameter<int32_t> pay_ldd_output_current(0);
    inline Parameter<int32_t> pay_sd_temperature_violations(0);
    inline Parameter<int32_t> pay_end_uptime(0);
    inline Parameter<int32_t> pay_response(0);
    inline Parameter<int32_t> pay_device(0);
    inline Parameter<int32_t> pay_firmware(0);
    inline Parameter<int32_t> pay_firmware_is_confirmed(0);
    inline Parameter<int32_t> pay_bitstream(0);
    inline Parameter<int32_t> pay_softcpu_1_firmware(0);
    inline Parameter<int32_t> pay_softcpu_2_firmware(0);
    inline Parameter<int32_t> pay_softcpu_3_firmware(0);
    inline Parameter<int32_t> pay_softcpu_4_firmware(0);
    inline Parameter<int32_t> pay_softcpu_5_firmware(0);
    inline Parameter<int32_t> pay_softcpu_6_firmware(0);
    inline Parameter<int32_t> pay_softcpu_7_firmware(0);
    inline Parameter<int32_t> pay_softcpu_8_firmware(0);
    inline Parameter<int32_t> pay_boot_count(0);
    inline Parameter<int32_t> pay_transmission_count(0);
    inline Parameter<int32_t> pay_hw_det(0);
    inline Parameter<int32_t> pay_storage0(0);
    inline Parameter<int32_t> pay_storage1(0);
    inline Parameter<int32_t> pay_storage2(0);
    inline Parameter<int32_t> pay_storage3(0);
    inline Parameter<int32_t> pay_ldd_fault(0);
    inline Parameter<int32_t> pay_fsm_fault(0);
    inline Parameter<int32_t> pay_fpga_fault(0);
    inline Parameter<int32_t> pay_v_cam_fault(0);
    inline Parameter<int32_t> pay_sdd_fault(0);
} // namespace PeakSatParameters

#pragma GCC diagnostic pop
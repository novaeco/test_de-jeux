#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1
#define LV_COLOR_SCREEN_TRANSP 0
#define LV_COLOR_MIX_ROUND_OFS 128

/*====================
   MEMORY SETTINGS
 *====================*/
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE (128 * 1024U)          /*128KB pour écran 7"*/
    #define LV_MEM_ADR 0
#endif
#define LV_MEM_BUF_MAX_NUM 16
#define LV_MEMCPY_MEMSET_STD 1

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DISP_DEF_REFR_PERIOD 33      /*30 FPS*/
#define LV_INDEV_DEF_READ_PERIOD 10     /*100 Hz touch*/

#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (esp_timer_get_time() / 1000)
#endif

#define LV_DPI_DEF 130

/*====================
   FEATURE CONFIGURATION
 *====================*/
#define LV_DRAW_COMPLEX 1
#if LV_DRAW_COMPLEX != 0
    #define LV_SHADOW_CACHE_SIZE 8
    #define LV_CIRCLE_CACHE_SIZE 8
#endif

#define LV_IMG_CACHE_DEF_SIZE 4
#define LV_GRADIENT_MAX_STOPS 4
#define LV_USE_USER_DATA 1

/*==================
 *   FONT USAGE
 *===================*/
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*=================
 *  TEXT SETTINGS
 *=================*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS " ,.;:-_"
#define LV_USE_BIDI 0

/*==================
 *  WIDGET USAGE
 *==================*/
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1

/*==================
 * EXTRA COMPONENTS
 *==================*/
#define LV_USE_ANIMIMG    1
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     1
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_MENU       1
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    1
#define LV_USE_TILEVIEW   1
#define LV_USE_WIN        1

/*==================
 *  THEME USAGE
 *==================*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    #define LV_THEME_DEFAULT_DARK 1
    #define LV_THEME_DEFAULT_GROW 1
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif

/*==================
 *  LAYOUTS
 *==================*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*==================
 * OTHERS
 *==================*/
#define LV_USE_SNAPSHOT 1
#define LV_USE_PERF_MONITOR 1
#define LV_USE_MEM_MONITOR 1
#define LV_USE_ASSERT_STYLE 0 /*Check if the styles are properly initialized. (Fast)*/

/* 1: Print the parameter passed to LV_LOG together with level, file and line number */
#define LV_LOG_PRINTF 1

/* Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs */
#define LV_LOG_TRACE_MEM        0
#define LV_LOG_TRACE_TIMER      0
#define LV_LOG_TRACE_INDEV      0
#define LV_LOG_TRACE_DISP_REFR  0
#define LV_LOG_TRACE_EVENT      0
#define LV_LOG_TRACE_OBJ_CREATE 0
#define LV_LOG_TRACE_LAYOUT     0
#define LV_LOG_TRACE_ANIM       0

#endif /*LV_CONF_H*/
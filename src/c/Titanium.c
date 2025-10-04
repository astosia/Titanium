#include <pebble.h>
#include "Titanium.h"
#include "utils/weekday.h"
#include "utils/MathUtils.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>

// Constants for improved readability
//#define SECONDS_VISIBLE_TIME_ALWAYS_ON 135
#define SECONDS_TICK_INTERVAL_MS 1000

// Main window and layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_dial_layer;
static Layer *s_dial_digits_layer;
static Layer *s_date_battery_logo_layer;
static Layer *s_canvas_second_hand;
static Layer *s_canvas_bt_icon;
static Layer *s_canvas_qt_icon;
static Layer *s_canvas_battery;
static GRect bounds;
static GRect bounds_seconds;
// Fonts
static GFont /*FontDigits,  FontLogo, FontBattery,*/
    #ifdef PBL_BW
    FontDate,
    FontBattery,
    FontLogo,
    #endif
    FontBTQTIcons;

FFont* Digits_Font;
FFont* Date_Font;
// Time and date variables
static struct tm *prv_tick_time;
static int current_date;
static int s_weekday;
static int minutes;
static int hours;
static int seconds;
static ClaySettings settings;
static bool showSeconds;

// Date position struct for different platforms
typedef struct {
  int BottomXPosition;
  int BottomYPosition;
  int LeftxPosition;
  int xOffset;
  int yOffset;
  int xOffsetFctxWeekday;
  int xOffsetFctxDate;
  int yOffsetFctx;
  int xyMaskOffset;
  int xWeekdayOffset;
  int yWeekdayDateOffset;
  int xDateOffset;
  int ShadowAndMaskWidth;
  int ShadowAndMaskHeight;
  int WeekdayWidth;
  int DateWidth;
  int WeekdayDateHeight;
  int BTIconYOffset;
  int QTIconYOffset;
  int BatteryYOffset;
  int BatteryLineYOffset;
  int LogoYOffset;
} UIConfig;

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
.BottomXPosition = 46,
.BottomYPosition = 168,
.LeftxPosition = -5,
.xOffset = 22,
.yOffset = -8,
.xOffsetFctxWeekday = 38/2  -1,
.xOffsetFctxDate = 22/2 + 1,
.yOffsetFctx = 3,
.xyMaskOffset = 6,
.xWeekdayOffset = 2,
.yWeekdayDateOffset = 1,
.xDateOffset = 41,
.ShadowAndMaskWidth = 64,
.ShadowAndMaskHeight = 17,
.WeekdayWidth = 38,
.DateWidth = 22,
.WeekdayDateHeight = 13,
.BTIconYOffset = -21,
.QTIconYOffset = 21,
.BatteryYOffset = 53-4 +3-1,
.BatteryLineYOffset = 66,
.LogoYOffset = 69+1
};
#elif defined(PBL_BW)
static const UIConfig config = {
.BottomXPosition = 38,
.BottomYPosition = 126,
.LeftxPosition = 6,
.xOffset = 10,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 ,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 3,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryLineYOffset = 49,
.LogoYOffset = 52+1
};
#elif defined(PBL_ROUND)
static const UIConfig config = {
.BottomXPosition = 52,
.BottomYPosition = 132,
.LeftxPosition = 16,
.xOffset = 14,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 -1,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryLineYOffset = 49,
.LogoYOffset = 52+1
};
#else // Default for other platforms
static const UIConfig config = {
.BottomXPosition = 38,
.BottomYPosition = 126,
.LeftxPosition = 7,
.xOffset = 10,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 -1,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryLineYOffset = 49,
.LogoYOffset = 52+1
};
#endif

bool connected = true;
bool ignore_next_tap = false;

//function prototypes

static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void bg_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_dial_digits_mask(Layer *layer, GContext * ctx);
static void update_logo_date_battery_fctx_layer(Layer *layer, GContext * ctx);
static void layer_update_proc_battery_line(Layer *layer, GContext * ctx);
static void layer_update_proc_seconds_hand(Layer *layer, GContext * ctx);
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_qt(Layer *layer, GContext *ctx);
static void layer_update_proc_bt(Layer *layer, GContext *ctx);
static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static void draw_fancy_hand_min(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color);
static void draw_seconds_center(GContext *ctx, GColor minutes_color, GColor seconds_color);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);

// Save settings to persistent storage
static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}


// Set default settings
static void prv_default_settings(void) {
  settings.EnableSecondsHand = true;
  settings.SecondsVisibleTime = 15;
  settings.EnableDate = true;
  settings.EnableBattery = true;
  settings.EnableBatteryLine = true;
  settings.EnableLogo = true;
  snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "TITANIUM");
  settings.BackgroundColor1 = GColorWhite;
  settings.BackgroundColor2 = GColorLightGray ;
  settings.TextColor1 = GColorWhite;
  settings.TextColor2 = GColorBlack;
  settings.TextColor3 = GColorBlack;
  settings.DateColor = GColorBlack;
  settings.HoursHandColor = GColorWhite;
  settings.HoursHandBorderColor = GColorBlack;
  settings.MinutesHandColor = GColorWhite;
  settings.MinutesHandBorderColor = GColorBlack;
  settings.SecondsHandColor = GColorOrange;
  settings.BatteryLineColor = GColorOrange;
  settings.BWDateColor = GColorBlack;
  settings.BWBackgroundColor1 = GColorWhite;
  settings.BWBackgroundColor2 = GColorDarkGray;
  settings.BWTextColor1 = GColorWhite;
  settings.BWTextColor2 = GColorBlack;
  settings.BWTextColor3 = GColorBlack;
  settings.BTQTColor = GColorBlack;
  settings.BWBTQTColor = GColorBlack;
  settings.BWThemeSelect = "wh";
  settings.BWShadowOn = true;
  settings.Font = 1;
  settings.VibeOn = false;
}

// Quiet time icon handler
static void quiet_time_icon () {
    layer_set_hidden(s_canvas_qt_icon, !quiet_time_is_active());
}


static AppTimer *s_timeout_timer;


static void timeout_handler(void *context) {
  showSeconds = false;

  // Unsubscribe from second ticks to save power
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  layer_mark_dirty(s_canvas_second_hand);
  s_timeout_timer = NULL; // Set the handle to NULL after the timer expires

  APP_LOG(APP_LOG_LEVEL_DEBUG, "timeout event");

}

// Accelerometer tap handler for seconds hand visibility
static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
   if (ignore_next_tap) {
       ignore_next_tap = false; // Reset the flag for the next tap
       return;
     }

  // Only handle if the seconds hand setting is enabled and not already always on
  if (settings.EnableSecondsHand && settings.SecondsVisibleTime < 135) {
      showSeconds = true;

    // If a timer is already running, cancel it
    if (s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }

    // Subscribe to second ticks
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    // Register a new timer to hide the seconds hand
    s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
    layer_mark_dirty(s_canvas_second_hand);
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "tap event, visible seconds is %d",settings.SecondsVisibleTime);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "tap event show seconds is %d",showSeconds);

}

static void bluetooth_vibe_icon (bool connected) {


   layer_set_hidden(s_canvas_bt_icon, connected);

  if((!connected && !quiet_time_is_active()) ||(!connected && quiet_time_is_active() && settings.VibeOn)) {
    if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
      // Unsubscribe from accel_tap before the vibe
      accel_tap_service_unsubscribe();
      showSeconds = false;
    }

    // Issue a vibrating alert
    #ifdef PBL_PLATFORM_DIORITE
    vibes_short_pulse();
    ignore_next_tap = true;

    #else
    vibes_double_pulse();
    // Set the flag to ignore the next tap
    ignore_next_tap = false;
    #endif

    if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
      accel_tap_service_subscribe(accel_tap_handler);
    }
  }


}

// Load settings from persistent storage
static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// AppMessage inbox handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message");
#endif

  bool settings_changed = false;
  bool theme_settings_changed = false;

  Tuple *vibe_t = dict_find(iter, MESSAGE_KEY_VibeOn);
  Tuple *enable_seconds_t = dict_find(iter, MESSAGE_KEY_EnableSecondsHand);
  Tuple *enable_secondsvisible_t = dict_find(iter, MESSAGE_KEY_SecondsVisibleTime);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_battery_t = dict_find(iter, MESSAGE_KEY_EnableBattery);
  Tuple *enable_battery_line_t = dict_find(iter, MESSAGE_KEY_EnableBatteryLine);
  Tuple *enable_logo_t = dict_find(iter, MESSAGE_KEY_EnableLogo);
  Tuple *logotext_t = dict_find(iter, MESSAGE_KEY_LogoText);
  Tuple *bwthemeselect_t = dict_find(iter, MESSAGE_KEY_BWThemeSelect);
  Tuple *bg_color1_t = dict_find(iter, MESSAGE_KEY_BackgroundColor1);
  Tuple *bg_color2_t = dict_find(iter, MESSAGE_KEY_BackgroundColor2);
  Tuple *text_color1_t = dict_find(iter, MESSAGE_KEY_TextColor1);
  Tuple *text_color2_t = dict_find(iter, MESSAGE_KEY_TextColor2);
  Tuple *text_color3_t = dict_find(iter, MESSAGE_KEY_TextColor3);
  Tuple *date_color_t = dict_find(iter, MESSAGE_KEY_DateColor);
  Tuple *bwdate_color_t = dict_find(iter, MESSAGE_KEY_BWDateColor);
  Tuple *hours_color_t = dict_find(iter, MESSAGE_KEY_HoursHandColor);
  Tuple *hours_border_t = dict_find(iter, MESSAGE_KEY_HoursHandBorderColor);
  Tuple *minutes_color_t = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  Tuple *minutes_border_t = dict_find(iter, MESSAGE_KEY_MinutesHandBorderColor);
  Tuple *seconds_color_t = dict_find(iter, MESSAGE_KEY_SecondsHandColor);
  Tuple *battery_line_color_t = dict_find(iter, MESSAGE_KEY_BatteryLineColor);
  Tuple *bwbg_color1_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor1);
  Tuple *bwbg_color2_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor2);
  Tuple *bwtext_color1_t = dict_find(iter, MESSAGE_KEY_BWTextColor1);
  Tuple *bwtext_color2_t = dict_find(iter, MESSAGE_KEY_BWTextColor2);
  Tuple *bwtext_color3_t = dict_find(iter, MESSAGE_KEY_BWTextColor3);
  Tuple *btqt_color_t = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *bwbtqt_color_t = dict_find(iter, MESSAGE_KEY_BWBTQTColor);
  Tuple *bwshadowon_t = dict_find(iter, MESSAGE_KEY_BWShadowOn);

  if (vibe_t){
    if (vibe_t -> value -> int32 == 0){
      settings.VibeOn = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe off");
    } else {
      settings.VibeOn = true;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe on");
    }
    layer_mark_dirty(s_canvas_bt_icon);
  }

  if (enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
    // Unsubscribe from any existing tick services
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    // Always subscribe to MINUTE_UNIT by default for efficiency
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    layer_mark_dirty(s_canvas_second_hand);
  }

  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (enable_logo_t) {
    settings.EnableLogo = enable_logo_t->value->int32 == 1;

    // Check if the logo is enabled and the custom text string is not empty
    if (settings.EnableLogo && logotext_t && strlen(logotext_t->value->cstring) > 0) {
      // If the custom text field is not blank, use the user's text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", logotext_t->value->cstring);
    } else if (settings.EnableLogo && strlen(logotext_t->value->cstring) == 0) {
      // If the custom text field is blank but the logo is enabled, use the default text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "TITANIUM");
    }
    else {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "");
    }

    layer_mark_dirty(s_date_battery_logo_layer);

  }



  if (enable_battery_t) {
    settings.EnableBattery = enable_battery_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_battery);
    layer_mark_dirty(s_date_battery_logo_layer);
    }

  if (enable_battery_line_t) {
    settings.EnableBatteryLine = enable_battery_line_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_battery);
  }

  if (enable_secondsvisible_t) {
    settings.SecondsVisibleTime = (int) enable_secondsvisible_t->value->int32;
    // Cancel and re-register timer if it was running
    if (s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }

    // Handle "Always On" vs. "Timeout" behavior for the seconds hand
    if (settings.SecondsVisibleTime == 135) {
      // "Always On" logic: show seconds, and don't register a timer
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        // Unsubscribe from accel_tap_service as it's not needed
        accel_tap_service_unsubscribe();
      }
    } else if (settings.SecondsVisibleTime > 0) {
      // "Timeout" logic: start with seconds shown, register a timer
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
        // Subscribe to accel_tap_service to reset the timer
        accel_tap_service_subscribe(accel_tap_handler);
      }
    } else {
      // "Disabled" logic: don't show seconds, ensure we're on minute ticks
      showSeconds = false;
      tick_timer_service_unsubscribe();
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      // Unsubscribe from accel_tap_service
      accel_tap_service_unsubscribe();
    }
    layer_mark_dirty(s_canvas_second_hand);
  }

  if (bwthemeselect_t) {
          // Compare the string value received from the phone
          if (strcmp(bwthemeselect_t->value->cstring, "wh") == 0) {
              // Set the theme and other settings for "wh"
                    settings.BWDateColor = GColorBlack;
                    if (bwshadowon_t) {
                      settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
                    }
                        if(settings.BWShadowOn){
                          settings.BWBackgroundColor2 = GColorDarkGray;
                        }
                        else {
                        settings.BWBackgroundColor2 = GColorWhite;
                        }
                    settings.BWBackgroundColor1 = GColorWhite;
                    settings.BWTextColor1 = GColorWhite;
                    settings.BWTextColor2 = GColorBlack;
                    settings.BWTextColor3 = GColorBlack;
                    settings.BWBTQTColor = GColorBlack;
                      theme_settings_changed = true;
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "bl") == 0) {
              // Set the theme and other settings for "bl"
                    settings.BWDateColor = GColorWhite;
                    settings.BWBackgroundColor1 = GColorBlack;
                    if (bwshadowon_t) {
                      settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
                    }
                        if(settings.BWShadowOn){
                          settings.BWBackgroundColor2 = GColorDarkGray;
                        }
                        else {
                        settings.BWBackgroundColor2 = GColorBlack;
                        }
                    settings.BWTextColor1 = GColorBlack;
                    settings.BWTextColor2 = GColorWhite;
                    settings.BWTextColor3 = GColorWhite;
                    settings.BWBTQTColor = GColorWhite;
                      theme_settings_changed = true;
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "cu") == 0) {
              // Set the theme for "cu" and handle custom colors
              settings.BWDateColor = GColorFromHEX(bwdate_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);

                  if (bwbg_color1_t) {
                    settings.BWBackgroundColor1 = GColorFromHEX(bwbg_color1_t->value->int32);
                    settings_changed = true;
                  }

                  if (bwshadowon_t) {
                    settings.BWShadowOn = bwshadowon_t->value->int32 == 1;

                      if(settings.BWShadowOn){
                        if (bwbg_color2_t) {
                          settings.BWBackgroundColor2 = GColorFromHEX(bwbg_color2_t->value->int32);
                          settings_changed = true;
                        }
                      }
                      else {
                      settings.BWBackgroundColor2 = settings.BWBackgroundColor1;
                      }
                  }

                  if (bwtext_color1_t) {
                    settings.BWTextColor1 = GColorFromHEX(bwtext_color1_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwtext_color2_t) {
                    settings.BWTextColor2 = GColorFromHEX(bwtext_color2_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwtext_color3_t) {
                    settings.BWTextColor3 = GColorFromHEX(bwtext_color3_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwbtqt_color_t) {
                    settings.BWBTQTColor = GColorFromHEX(bwbtqt_color_t->value->int32);
                    layer_mark_dirty(s_canvas_bt_icon);
                    layer_mark_dirty(s_canvas_qt_icon);
                  }
                  theme_settings_changed = true;
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }

  if (bg_color1_t) {
    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    layer_mark_dirty(s_canvas_second_hand);
  }
  if (bg_color2_t) {
    settings.BackgroundColor2 = GColorFromHEX(bg_color2_t->value->int32);
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
  }
  if (text_color1_t) {
    settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }
  if (text_color2_t) {
    settings.TextColor2 = GColorFromHEX(text_color2_t->value->int32);
    layer_mark_dirty(s_bg_layer);
  }
  if (text_color3_t) {
    settings.TextColor3 = GColorFromHEX(text_color3_t->value->int32);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_dial_digits_layer);
    //layer_mark_dirty(s_date_battery_logo_layer);
  }
  if (date_color_t) {
    settings.DateColor = GColorFromHEX(date_color_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (hours_color_t) {
    settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
  }
  if (hours_border_t) {
    settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
  }
  if (minutes_color_t) {
    settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
  }
  if (minutes_border_t) {
    settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
  }
  if (seconds_color_t) {
    settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32);
    layer_mark_dirty(s_canvas_second_hand);
  }
  if (battery_line_color_t) {
    settings.BatteryLineColor = GColorFromHEX(battery_line_color_t->value->int32);
    layer_mark_dirty(s_canvas_battery);
  }
  if (btqt_color_t) {
    settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_canvas_qt_icon);
  }

  if (settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_battery);
  }

  if (theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_qt_icon);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_canvas_battery);
  }

  prv_save_settings();

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);

  // Update hour and minute hands and the date on minute change
  if (units_changed & MINUTE_UNIT) {
    minutes = tick_time->tm_min;
    hours = tick_time->tm_hour % 12;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    if (settings.EnableDate && tick_time->tm_mday != current_date) {
      current_date = tick_time->tm_mday;
      s_weekday = tick_time->tm_wday;
    }
  }

  // Update seconds hand on second change, but only if it's visible
  if (showSeconds && (units_changed & SECOND_UNIT)) {
    seconds = tick_time->tm_sec;
    layer_mark_dirty(s_canvas_second_hand);
  }

  // hide or show the seconds hand layer
  layer_set_hidden(s_canvas_second_hand, !(showSeconds && settings.EnableSecondsHand));


}

//draw hour hand
static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    int p1l = 18;
    int p2l = length;
    GPoint p1 = polar_to_point_offset(origin, angle, p1l);
    GPoint p2 = polar_to_point_offset(origin, angle, p2l);

    // Define shadow color
    GColor shadow_color = PBL_IF_BW_ELSE(settings.BWBackgroundColor1,settings.BackgroundColor2);

    // Draw the shadow first, with an offset
    GPoint shadow_offset = GPoint(1, 1);
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, shadow_color);
    graphics_context_set_stroke_width(ctx, 11);
    graphics_draw_line(ctx, GPoint(origin.x + shadow_offset.x, origin.y + shadow_offset.y), GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));

    // Now draw the main hand on top
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 11);
    graphics_draw_line(ctx, origin, p1);
    graphics_draw_line(ctx, p1, p2);
    graphics_context_set_stroke_color(ctx, fill_color);
    graphics_context_set_stroke_width(ctx, 5);
    graphics_draw_line(ctx, p1, p2);

    graphics_context_set_fill_color(ctx, border_color);
    graphics_fill_circle(ctx, origin, 7);
}

static void draw_fancy_hand_min(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    int p1l = 18;
    int p2l = length;
    GPoint p1 = polar_to_point_offset(origin, angle, p1l);
    GPoint p2 = polar_to_point_offset(origin, angle, p2l);

    // Define shadow color
    GColor shadow_color = PBL_IF_BW_ELSE(settings.BWBackgroundColor1,settings.BackgroundColor2);


    // Draw the shadow first, with an offset
    GPoint shadow_offset = PBL_IF_BW_ELSE(GPoint(1,1),GPoint(2, 2));
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, shadow_color);
    graphics_context_set_stroke_width(ctx, 9);
    graphics_draw_line(ctx, GPoint(origin.x + shadow_offset.x, origin.y + shadow_offset.y), GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y));
    graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y), GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));

    // Now draw the main hand on top
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 9);
    graphics_draw_line(ctx, origin, p1);
    graphics_draw_line(ctx, p1, p2);
    graphics_context_set_stroke_color(ctx, fill_color);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, p1, p2);
}

///second hand
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset(origin, angle + 180, back_length);
  GPoint p2 = polar_to_point_offset(origin, angle, length);

  // Define shadow color
  GColor shadow_color = PBL_IF_BW_ELSE(settings.BWBackgroundColor1,settings.BackgroundColor2);


  // Set the antialiasing
  graphics_context_set_antialiased(ctx, true);

  // Draw the shadow first, with a small offset
  graphics_context_set_stroke_color(ctx, shadow_color);
  graphics_context_set_stroke_width(ctx, 1); // Same width as the hand
  graphics_draw_line(ctx, GPoint(p1.x + 2, p1.y + 2), GPoint(p2.x + 2, p2.y + 2));

  // Now draw the main second hand on top
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, p1, p2);

#ifdef PBL_BW
  // Special handling for black-and-white displays, if needed
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, p1, p2);
#endif
}

static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, minutes_color);
  graphics_fill_circle(ctx, origin, 3); //started as 2
}

static void draw_seconds_center(GContext *ctx, GColor minutes_color, GColor seconds_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, seconds_color);
  graphics_fill_circle(ctx, origin, 5); //started as 4

  graphics_context_set_fill_color(ctx, minutes_color);
  graphics_fill_circle(ctx, origin, 3); //started as 2

}

static void draw_major_tick_shadow (GContext *ctx, int angle, int length, GColor fill_color, GColor color2) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);


    // The tick starts away from the center
    GPoint p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 20);
    // The tick ends at a fixed length from p1
    GPoint p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 20 + length);

    // Calculate a point that is 20% of the way from p1 to p2
    GPoint shadow_endpoint = GPoint(
        p1.x + (p2.x - p1.x) * 0.5,
        p1.y + (p2.y - p1.y) * 0.5
    );

    // Define a shadow color and offset
    GColor shadow_color = PBL_IF_BW_ELSE(settings.BWBackgroundColor1,settings.BackgroundColor2);
    GPoint shadow_offset = GPoint(3, 3);

    // Draw the shadow first. It only goes from the start to the calculated endpoint.
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, shadow_color);
    graphics_context_set_stroke_width(ctx, 7);
    graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y), GPoint(shadow_endpoint.x + shadow_offset.x, shadow_endpoint.y + shadow_offset.y));


}

static void draw_major_tick_mask (GContext *ctx, int radius, GColor tick_mask_color) {
    GPoint origin = GPoint((bounds.size.w / 2) , (bounds.size.h / 2) );

    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, tick_mask_color);
    graphics_context_set_stroke_width(ctx, 24);
    graphics_draw_circle(ctx, origin, radius);
}


static void draw_major_tick (GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);

    // The tick starts away from the center
    GPoint p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 20);
    // The tick ends at a fixed length from p1
    GPoint p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 20 + length);

    // Now, draw the main tick on top. It covers the full length.
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 7);
    graphics_draw_line(ctx, p1, p2);

    graphics_context_set_stroke_color(ctx, fill_color);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, p1, p2);
}

static void draw_minor_tick(GContext *ctx, int angle, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  // The tick starts away from the center of the watch face.
  GPoint p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 15);
  // The tick ends closer to the edge.
  GPoint p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 10);

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 1); // A thin line for minor ticks
  graphics_draw_line(ctx, p1, p2);
}

static void draw_circle_shadow(GContext *ctx, GPoint center, int radius, GColor circle_color, GColor shadow_color) {
    // Draw the shadow first, offset by a few pixels
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_fill_color(ctx, shadow_color);
    graphics_fill_circle(ctx, GPoint(center.x , center.y ), radius);

    // Draw the main circle on top of the shadow
    graphics_context_set_fill_color(ctx, circle_color);
    graphics_fill_circle(ctx, GPoint(center.x + 4, center.y + 4), radius);
}


// Update procedures for the dial layer (the digits)
static void update_dial_digits_fctx_layer (Layer *layer, GContext *ctx) {
  //use FCTX to antialise the digits better

  GRect bounds = layer_get_bounds(s_dial_digits_layer);

  FContext fctx;

  #ifdef PBL_COLOR
    fctx_enable_aa(true);
  #endif

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);

  #ifdef PBL_PLATFORM_EMERY
    int font_size_digits = 36;
  #else
    int font_size_digits = 34;
  #endif

  fctx_set_fill_color(&fctx, PBL_IF_BW_ELSE(settings.BWTextColor3, settings.TextColor3));

  FPoint six_pos;

  fctx_begin_fill(&fctx);
  fctx_set_text_em_height(&fctx, Digits_Font, font_size_digits);

  #ifdef PBL_PLATFORM_EMERY
    six_pos.x = INT_TO_FIXED((bounds.size.w / 2) + 2);
    six_pos.y = INT_TO_FIXED(bounds.size.h - 40 + 9);
  #else
    six_pos.x = INT_TO_FIXED((bounds.size.w / 2) + 2);
    six_pos.y = INT_TO_FIXED(bounds.size.h - 38 + 9);
  #endif

  char sixdraw [2];
  snprintf(sixdraw, sizeof(sixdraw), "%s", "6");

  fctx_set_offset(&fctx, six_pos);

  fctx_draw_string(&fctx, sixdraw, Digits_Font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(&fctx);

  FPoint twelve_pos;

  fctx_begin_fill(&fctx);
  fctx_set_text_em_height(&fctx, Digits_Font, font_size_digits);

  #ifdef PBL_PLATFORM_EMERY
    //GRect((bounds.size.w / 2) - 16, bounds.size.h - 40, 36, 40)
    twelve_pos.x = INT_TO_FIXED((bounds.size.w / 2) - 2 + 1);
    twelve_pos.y = INT_TO_FIXED(-3);
  #elif defined(PBL_BW)
    twelve_pos.x = INT_TO_FIXED((bounds.size.w / 2) - 2 + 1);
    twelve_pos.y = INT_TO_FIXED(-2);
  #else
    twelve_pos.x = INT_TO_FIXED((bounds.size.w / 2) - 2 );
    twelve_pos.y = INT_TO_FIXED(-2);
  #endif

  char twelvedraw [3];
  snprintf(twelvedraw, sizeof(twelvedraw), "%s", "12");

  fctx_set_offset(&fctx, twelve_pos);

  fctx_draw_string(&fctx, twelvedraw, Digits_Font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(&fctx);

  fctx_deinit_context(&fctx);

}
#ifdef PBL_BW
static void update_logo_date_battery_fctx_layer (Layer *layer, GContext *ctx) {
  //DON'T use FCTX on Aplite

  GRect bounds = layer_get_bounds(s_date_battery_logo_layer);

  //draw battery value
  if(settings.EnableBattery ){

    int s_battery_level = battery_state_service_peek().charge_percent;
    char BatterytoDraw[6];
          snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

    if (settings.EnableBatteryLine) {
      GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset, 36, 40);
      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    }
    else{
      GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset + 4, 36, 40);
      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    }

    }

  //draw weekday and date text
  if (settings.EnableDate ) {


    minutes = prv_tick_time->tm_min;
    hours = prv_tick_time->tm_hour % 12;


    ///use below for testing and for screenshots
    //  int seconds = 8;
      // int minutes = 30;
      // int hours = 9;


    int xPosition;
    int yPosition;
    int xOffset = config.xOffset;
    int yOffset = config.yOffset;
    ///int yOffsetFctx = config.yOffsetFctx;
    int xWeekdayOffset = config.xWeekdayOffset;
    int yWeekdayDateOffset = config.yWeekdayDateOffset;
    int xDateOffset = config.xDateOffset;
    int xyMaskOffset = config.xyMaskOffset;
    int ShadowAndMaskWidth = config.ShadowAndMaskWidth;
    int ShadowAndMaskHeight = config.ShadowAndMaskHeight;
    int WeekdayWidth = config.WeekdayWidth;
    int DateWidth = config.DateWidth;
    int WeekdayDateHeight = config.WeekdayDateHeight;

    if (((hours > 7 && hours < 11) && (minutes > 7 && minutes < 23))||
    ((hours <5 && hours >1) && (minutes > 37 && minutes < 53))){
      //bottom position
      xPosition = config.BottomXPosition;
      yPosition = config.BottomYPosition;
     }
     else if ((minutes > 7 && minutes < 23)||
     ((hours <5 && hours >1) && (minutes > 52 || minutes < 37))){
//       // Left position
      xPosition = config.LeftxPosition;
      yPosition = bounds.size.h/2;
     }
     else{
       // Right position
     xPosition = bounds.size.w/2;
     yPosition = bounds.size.h/2;
     }
    GRect DateShadowRect =
     //  (0, offsetdate, bounds3.size.w, bounds1.size.h/4);
       GRect(xPosition + xOffset, yPosition + yOffset, ShadowAndMaskWidth, ShadowAndMaskHeight);

     GRect DateShadowMaskRect =
        GRect(xPosition + xOffset + xyMaskOffset, yPosition + yOffset + xyMaskOffset, ShadowAndMaskWidth-xyMaskOffset, ShadowAndMaskHeight-xyMaskOffset);

    graphics_context_set_antialiased(ctx, true);

     graphics_context_set_fill_color(ctx, settings.BWBackgroundColor2);
     graphics_fill_rect(ctx, DateShadowRect ,0, GCornersAll);

     graphics_context_set_fill_color(ctx, settings.BWBackgroundColor1);
     graphics_fill_rect(ctx, DateShadowMaskRect ,0, GCornersAll);

    const char * sys_locale = i18n_get_system_locale();
    char weekday[5];
    fetchwday(s_weekday, sys_locale, weekday);

    char weekdaydraw[10];
    snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

    char daynow[5];
    snprintf(daynow, sizeof(daynow), "%d", current_date);

    GRect WeekdayRect =
           //  (0, offsetdate, bounds3.size.w, bounds1.size.h/4);
         GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

     GRect DateRect =
         GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);

     graphics_context_set_text_color(ctx, settings.BWDateColor);
     graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
     graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
 }


     if (settings.EnableLogo) {
       //draw logo text
      GRect LogoRect = GRect((bounds.size.w / 2) - 50, config.LogoYOffset - 1, 100, 40);

      char logodraw [12];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, logodraw, FontLogo, LogoRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
     }

}
#else
static void update_logo_date_battery_fctx_layer (Layer *layer, GContext *ctx) {
  //use FCTX to antialise the digits better on all watches except APLITE

  GRect bounds = layer_get_bounds(s_date_battery_logo_layer);

  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  #ifdef PBL_COLOR
   fctx_enable_aa(true);
  #endif

  //draw battery value


  if(settings.EnableBattery ){
    fctx_set_fill_color(&fctx, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));
    FPoint battery_pos;
    #ifdef PBL_PLATFORM_EMERY
      int font_size_battery = 12;
    #elif defined(PBL_BW)
      int font_size_battery = 10;
    #else
      int font_size_battery = 10;
    #endif

    int s_battery_level = battery_state_service_peek().charge_percent;
    fctx_begin_fill(&fctx);
    fctx_set_text_em_height(&fctx, Date_Font, font_size_battery);

        // Place the code that formats and draws the battery percentage text here.
      if (settings.EnableBatteryLine) {
          #ifdef PBL_PLATFORM_EMERY
            //GRect((bounds.size.w / 2) - 16, bounds.size.h - 40, 36, 40)
            battery_pos.x = INT_TO_FIXED((bounds.size.w / 2));
            battery_pos.y = INT_TO_FIXED(config.BatteryYOffset + config.yOffsetFctx);
          #else
            battery_pos.x = INT_TO_FIXED((bounds.size.w / 2) );
            battery_pos.y = INT_TO_FIXED(config.BatteryYOffset + config.yOffsetFctx);
          #endif
          //int rect_y_pos = config.BatteryYOffset;
          char BatterytoDraw[6];
          snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

          fctx_set_offset(&fctx, battery_pos);
          fctx_draw_string(&fctx, BatterytoDraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
          fctx_end_fill(&fctx);
        }
        else {
          #ifdef PBL_PLATFORM_EMERY
            //GRect((bounds.size.w / 2) - 16, bounds.size.h - 40, 36, 40)
            battery_pos.x = INT_TO_FIXED((bounds.size.w / 2));
            battery_pos.y = INT_TO_FIXED(config.BatteryYOffset   + config.yOffsetFctx + 4);
          #else
            battery_pos.x = INT_TO_FIXED((bounds.size.w / 2) );
            battery_pos.y = INT_TO_FIXED(config.BatteryYOffset + config.yOffsetFctx + 4);
          #endif
          //int rect_y_pos = config.BatteryYOffset + 4;
          char BatterytoDraw[6];
          snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

          fctx_set_offset(&fctx, battery_pos);
          fctx_draw_string(&fctx, BatterytoDraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
          fctx_end_fill(&fctx);
        }
    }


  //draw weekday and date text
  if (settings.EnableDate ) {
    fctx_set_fill_color(&fctx, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));

    minutes = prv_tick_time->tm_min;
    hours = prv_tick_time->tm_hour % 12;


    ///use below for testing and for screenshots
    //  int seconds = 8;
      // int minutes = 30;
      // int hours = 9;

    FPoint weekday_pos;
    FPoint date_pos;


    int weekday_loc_x = 0;
    int weekday_loc_y = 0;

    #ifdef PBL_PLATFORM_EMERY
          int font_size_date = 12;
    #else
          int font_size_date = 9;
    #endif


    int xPosition;
    int yPosition;
    int xOffset = config.xOffset;
    int yOffset = config.yOffset;
    int yOffsetFctx = config.yOffsetFctx;
    int xWeekdayOffset = config.xWeekdayOffset;
    int yWeekdayDateOffset = config.yWeekdayDateOffset;
    int xDateOffset = config.xDateOffset;
    int xyMaskOffset = config.xyMaskOffset;
    int ShadowAndMaskWidth = config.ShadowAndMaskWidth;
    int ShadowAndMaskHeight = config.ShadowAndMaskHeight;
    int xOffsetFctxWeekday = config.xOffsetFctxWeekday;
    int xOffsetFctxDate = config.xOffsetFctxDate;

    if (((hours > 7 && hours < 11) && (minutes > 7 && minutes < 23))||
    ((hours <5 && hours >1) && (minutes > 37 && minutes < 53))){
      // Bottom position
      xPosition = config.BottomXPosition;
      yPosition = config.BottomYPosition;

      weekday_pos.x = INT_TO_FIXED(config.BottomXPosition + xOffset + xOffsetFctxWeekday + xWeekdayOffset);
      weekday_pos.y = INT_TO_FIXED(config.BottomYPosition + yOffset + yOffsetFctx + yWeekdayDateOffset);
      date_pos.x = INT_TO_FIXED(config.BottomXPosition + xOffset + xOffsetFctxDate + xDateOffset);
      date_pos.y = INT_TO_FIXED(config.BottomYPosition + yOffset + yOffsetFctx + yWeekdayDateOffset);
       weekday_loc_x = FIXED_TO_INT(weekday_pos.x);
       weekday_loc_y = FIXED_TO_INT(weekday_pos.y);
        APP_LOG(APP_LOG_LEVEL_DEBUG,"weekday pos is bottom position ");

    }
    else if ((minutes > 7 && minutes < 23)||
    ((hours <5 && hours >1) && (minutes > 52 || minutes < 37))){
      // Left position
      xPosition = config.LeftxPosition;
      yPosition = bounds.size.h/2;

      weekday_pos.x = INT_TO_FIXED(config.LeftxPosition + xOffset + xOffsetFctxWeekday + xWeekdayOffset);
      weekday_pos.y = INT_TO_FIXED(bounds.size.h/2 + yOffset + yOffsetFctx+ yWeekdayDateOffset);
      date_pos.x = INT_TO_FIXED(config.LeftxPosition + xOffset + xOffsetFctxDate + xDateOffset);
      date_pos.y = INT_TO_FIXED(bounds.size.h/2 + yOffset + yOffsetFctx + yWeekdayDateOffset);
      weekday_loc_x = FIXED_TO_INT(weekday_pos.x);
      weekday_loc_y = FIXED_TO_INT(weekday_pos.y);
        APP_LOG(APP_LOG_LEVEL_DEBUG,"weekday pos is left position");

    }
    else{
      // Right position
      xPosition = bounds.size.w/2;
      yPosition = bounds.size.h/2;

      weekday_pos.x = INT_TO_FIXED(bounds.size.w/2 + xOffset + xOffsetFctxWeekday + xWeekdayOffset);
      weekday_pos.y = INT_TO_FIXED(bounds.size.h/2 + yOffset + yOffsetFctx + yWeekdayDateOffset);
      date_pos.x = INT_TO_FIXED(bounds.size.w/2 + xOffset + xOffsetFctxDate + xDateOffset);
      date_pos.y = INT_TO_FIXED(bounds.size.h/2 + yOffset + yOffsetFctx + yWeekdayDateOffset);
        weekday_loc_x = FIXED_TO_INT(weekday_pos.x);
        weekday_loc_y = FIXED_TO_INT(weekday_pos.y);
        APP_LOG(APP_LOG_LEVEL_DEBUG,"weekday pos is right position");

    }
    APP_LOG(APP_LOG_LEVEL_DEBUG,"weekday pos is %d, %d", weekday_loc_x, weekday_loc_y);
    GRect DateShadowRect =
     //  (0, offsetdate, bounds3.size.w, bounds1.size.h/4);
       GRect(xPosition + xOffset, yPosition + yOffset, ShadowAndMaskWidth, ShadowAndMaskHeight);

     GRect DateShadowMaskRect =
        GRect(xPosition + xOffset + xyMaskOffset, yPosition + yOffset + xyMaskOffset, ShadowAndMaskWidth-xyMaskOffset, ShadowAndMaskHeight-xyMaskOffset);

     graphics_context_set_antialiased(ctx, true);

     #ifdef PBL_BW
     graphics_context_set_fill_color(ctx, settings.BWBackgroundColor2);
     graphics_fill_rect(ctx, DateShadowRect ,0, GCornersAll);

     graphics_context_set_fill_color(ctx, settings.BWBackgroundColor1);
     graphics_fill_rect(ctx, DateShadowMaskRect ,0, GCornersAll);
     #else
     graphics_context_set_fill_color(ctx, settings.BackgroundColor2);
     graphics_fill_rect(ctx, DateShadowRect ,0, GCornersAll);

     graphics_context_set_fill_color(ctx, settings.BackgroundColor1);
     graphics_fill_rect(ctx, DateShadowMaskRect ,0, GCornersAll);
     #endif

    fctx_begin_fill(&fctx);
    fctx_set_text_em_height(&fctx, Date_Font, font_size_date);

    const char * sys_locale = i18n_get_system_locale();
    char weekday[5];
    fetchwday(s_weekday, sys_locale, weekday);

    char weekdaydraw[10];
    snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);
  //  snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", "WED");

    fctx_set_offset(&fctx, weekday_pos);
    fctx_draw_string(&fctx, weekdaydraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
    fctx_end_fill(&fctx);



    fctx_begin_fill(&fctx);
    fctx_set_text_em_height(&fctx, Date_Font, font_size_date);

    char daynow[5];
    snprintf(daynow, sizeof(daynow), "%d", current_date);
  //  snprintf(daynow, sizeof(daynow), "%s", "30");

    fctx_set_offset(&fctx, date_pos);
    fctx_draw_string(&fctx, daynow, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
    fctx_end_fill(&fctx);


    }


    if (settings.EnableLogo) {  //draw logo text
    fctx_set_fill_color(&fctx, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));
    FPoint logo_pos;

    #ifdef PBL_PLATFORM_EMERY
      //GRect((bounds.size.w / 2) - 16, bounds.size.h - 40, 36, 40)
      logo_pos.x = INT_TO_FIXED((bounds.size.w / 2));
      logo_pos.y = INT_TO_FIXED(config.LogoYOffset);
    #else
      logo_pos.x = INT_TO_FIXED((bounds.size.w / 2));
      logo_pos.y = INT_TO_FIXED(config.LogoYOffset);
    #endif

    #ifdef PBL_PLATFORM_EMERY
      int font_size_logo = 8;
    #else
      int font_size_logo = 8;
    #endif

      fctx_begin_fill(&fctx);
      fctx_set_text_em_height(&fctx, Date_Font, font_size_logo);

      char logodraw [12];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      fctx_set_offset(&fctx, logo_pos);

      fctx_draw_string(&fctx, logodraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
      fctx_end_fill(&fctx);
    }

  fctx_deinit_context(&fctx);

}
#endif

static void layer_update_proc_dial_digits_mask(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(s_dial_layer);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWBackgroundColor2, settings.BackgroundColor2));

#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 15, 23, 39, 7), 2, GCornersAll);
#else
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 14, 22, 36, 7), 2, GCornersAll);
#endif

  //graphics_context_set_text_color(ctx, PBL_IF_BW_ELSE(settings.BWTextColor2, settings.TextColor3));
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));

#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 19, 0, 39, 27), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 19, bounds.size.h - 19, 31, 27), 1, GCornersAll);
#else
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 18, 0, 36, 26), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 13, bounds.size.h - 26, 28, 26), 1, GCornersAll);
#endif

}

// Update procedure for the seconds hand layer
static void layer_update_proc_seconds_hand(Layer *layer, GContext *ctx) {
  if (!showSeconds || !prv_tick_time) {
      // Do not draw the second hand if it should be hidden or if time data is not yet available
      return;
    }

  GRect bounds = layer_get_bounds(layer);
  seconds = prv_tick_time->tm_sec;
  //for test & screenshots
  //int seconds = 8;

  if (!settings.EnableSecondsHand || !showSeconds) {
    seconds = 0;
  }

  int seconds_angle = ((double)seconds / 60 * 360) - 90;

#ifdef PBL_PLATFORM_EMERY
  draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 20, 22, settings.SecondsHandColor);
#elif defined(PBL_COLOR)
  draw_line_hand(ctx, seconds_angle, PBL_IF_ROUND_ELSE(bounds.size.w / 2 - 35, bounds.size.w / 2 - 20), 22, settings.SecondsHandColor);
#else
  draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 20, 22, settings.BWTextColor2);
#endif

  draw_seconds_center(ctx, PBL_IF_BW_ELSE(settings.BWTextColor1, settings.HoursHandColor), PBL_IF_BW_ELSE(settings.BWTextColor2, settings.SecondsHandColor));
}



static void layer_update_proc_battery_line(Layer *layer, GContext *ctx) {
    // 1. EXIT EARLY: If neither element is enabled, stop.
    if (!settings.EnableBattery && !settings.EnableBatteryLine) {
        return;
    }

    int s_battery_level = battery_state_service_peek().charge_percent;

    // 2. DRAW THE BATTERY LINE (Independent Visibility Check)
    if (settings.EnableBatteryLine) {
        //  the code that draws the battery line
        int width_rect = (s_battery_level * 51) / 100;
        int rect_x_pos = (bounds.size.w/2) - (width_rect/2);

        GRect BatteryLineRect = GRect(rect_x_pos,config.BatteryLineYOffset,width_rect, 2);
        graphics_context_set_antialiased(ctx, true);
        graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWTextColor2, settings.BatteryLineColor));
        graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);

    }


}

//Update procedure for the BT Icon layer
static void layer_update_proc_bt(Layer * layer, GContext * ctx){
  GRect bounds = layer_get_bounds(layer);
  // int minutes = prv_tick_time->tm_min;
  // int hours = prv_tick_time->tm_hour % 12;

//use this for testing
  // int minutes = 30;
  // int hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;
      int BTIconYOffset;

      if (((hours > 7 && hours < 11) && (minutes > 7 && minutes < 23))||
      ((hours <5 && hours >1) && (minutes > 37 && minutes < 53))){
        // Bottom position
            if(quiet_time_is_active()){
              xPosition = config.BottomXPosition + 2;
              yPosition = config.BottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth/2;
              BTIconYOffset = config.BTIconYOffset;
            }
            else{
              xPosition = config.BottomXPosition;
              yPosition = config.BottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth;
              BTIconYOffset = config.BTIconYOffset;
            }
      }
      else if ((minutes > 7 && minutes < 23)||
      ((hours <5 && hours >1) && (minutes > 52 || minutes < 37))){
        // Left position
        xPosition = config.LeftxPosition;
        yPosition = bounds.size.h/2;
        textboxwidth = config.ShadowAndMaskWidth;
        BTIconYOffset = config.BTIconYOffset;
      }
      else{
        // Right position
        xPosition = bounds.size.w/2;
        yPosition = bounds.size.h/2;
        textboxwidth = config.ShadowAndMaskWidth;
        BTIconYOffset = config.BTIconYOffset;
      }

  GRect BTIconRect =
    GRect(xPosition + config.xOffset, yPosition + config.yOffset + BTIconYOffset, textboxwidth, 20);


#ifdef PBL_COLOR
 graphics_context_set_text_color(ctx, settings.BTQTColor);
 #else
  graphics_context_set_text_color(ctx, settings.BWBTQTColor);
 #endif

 //bluetooth_callback(connection_service_peek_pebble_app_connection());
 graphics_context_set_antialiased(ctx, true);
 graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);


}

//Update procedure for the QT Icon layer
static void layer_update_proc_qt(Layer * layer, GContext * ctx){

  GRect bounds = layer_get_bounds(layer);
  // int minutes = prv_tick_time->tm_min;
  // int hours = prv_tick_time->tm_hour % 12;

//use this for testing
  // int minutes = 30;
  // int hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;
      int QTIconYOffset;

      if (((hours > 7 && hours < 11) && (minutes > 7 && minutes < 23))||
      ((hours <5 && hours >1) && (minutes > 37 && minutes < 53))){
        // Bottom position
      if(connection_service_peek_pebble_app_connection()){
        xPosition = config.BottomXPosition;
        yPosition = config.BottomYPosition;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = 0 - config.QTIconYOffset;
      }
      else{
        xPosition = config.BottomXPosition + config.ShadowAndMaskWidth/2 - 2;
        yPosition = config.BottomYPosition -1 ;
        textboxwidth = config.ShadowAndMaskWidth/2;
        QTIconYOffset = 0 - config.QTIconYOffset;
      }
      }
      else if ((minutes > 7 && minutes < 23)||
      ((hours <5 && hours >1) && (minutes > 52 || minutes < 37))){
        // Left position
        xPosition = config.LeftxPosition;
        yPosition = bounds.size.h/2;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = config.QTIconYOffset;
      }
      else{
        // Right position
        xPosition = bounds.size.w/2;
        yPosition = bounds.size.h/2;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = config.QTIconYOffset;
      }

  GRect QTIconRect =
    GRect(xPosition + config.xOffset, yPosition + config.yOffset + QTIconYOffset, textboxwidth, 20);

 quiet_time_icon();


 #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, settings.BTQTColor);
  #else
   graphics_context_set_text_color(ctx, settings.BWBTQTColor);
  #endif
  graphics_context_set_antialiased(ctx, true);
  graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);


}

// Update procedure for the main canvas layer (hour & minute hands)
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx) {

 GRect bounds = layer_get_bounds(layer);

//use these for live version

  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour % 12;
  //int seconds = prv_tick_time->tm_sec;

  ///use below for testing and for screenshots
  //  int seconds = 8;
    // int minutes = 30;
    // int hours = 9;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "minutes in hour_min_hands_canvas_update_proc is %d", minutes);

  // #ifdef SECOND
  //   seconds = SECOND;
  // #endif

  #ifdef HOUR
    hours = HOUR;
  #endif

  #ifdef MINUTE
    minutes = MINUTE;
  #endif

   int hours_angle = ((double)hours / 12 * 360) + ((double)minutes / 60 * 360 / 12) + /*((double)seconds / 60 * 360 / 60 / 12)*/  - 90;

 #ifdef PBL_PLATFORM_EMERY
   draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - 50, settings.HoursHandColor, settings.HoursHandBorderColor);
 #elif defined (PBL_COLOR)
   draw_fancy_hand_hour(ctx, hours_angle, PBL_IF_ROUND_ELSE(bounds.size.w / 2 - 50,bounds.size.w / 2 - 35), settings.HoursHandColor, settings.HoursHandBorderColor);
 #else
   draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - 35, settings.BWTextColor1, settings.BWTextColor2);
 #endif

   int minutes_angle = ((double)minutes / 60 * 360) + /*((double)seconds / 60 * 360 / 60)*/ - 90;
 #ifdef PBL_PLATFORM_EMERY
   draw_fancy_hand_min(ctx, minutes_angle, bounds.size.w / 2 - 20, settings.MinutesHandColor, settings.MinutesHandBorderColor);

 #elif defined (PBL_COLOR)
   draw_fancy_hand_min(ctx, minutes_angle, PBL_IF_ROUND_ELSE(bounds.size.w / 2 - 34,bounds.size.w / 2 - 22), settings.MinutesHandColor, settings.MinutesHandBorderColor);
 #else
   draw_fancy_hand_min(ctx, minutes_angle, bounds.size.w / 2 - 22, settings.BWTextColor1, settings.BWTextColor2);
 #endif



 #ifdef PBL_COLOR
   draw_center(ctx, settings.HoursHandBorderColor, settings.HoursHandColor);
 #else
   draw_center(ctx, settings.BWTextColor2, settings.BWTextColor1);
 #endif
}

///update procedure for background
static void bg_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);

  GRect Background =
       GRect(0, 0, bounds.size.w, bounds.size.h);

   graphics_context_set_fill_color(ctx,PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1) );
   graphics_fill_rect(ctx, Background,0,GCornersAll);

   // Draw the large circle with a shadow
      GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
     #ifdef PBL_ROUND
      int circle_radius = (bounds.size.h / 2) - 17; // Adjust this value as needed
      int tick_mask_radius = circle_radius +12;
     #else
       int circle_radius = (bounds.size.h / 2) - 18; // Adjust this value as needed
       int tick_mask_radius = circle_radius +12;
     #endif

     draw_circle_shadow(ctx, center, circle_radius, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1), PBL_IF_BW_ELSE(settings.BWBackgroundColor2, settings.BackgroundColor2));

      for (int i = 0; i < 12; i++) {
        if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick_shadow(ctx, angle, 15, PBL_IF_BW_ELSE(settings.BWTextColor1, settings.HoursHandColor), PBL_IF_BW_ELSE(settings.BWTextColor3, settings.BackgroundColor2));
      }

      draw_major_tick_mask(ctx, tick_mask_radius, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));

      for (int i = 0; i < 12; i++) {
        if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1), PBL_IF_BW_ELSE(settings.BWTextColor3, settings.TextColor2));
      }

      for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        int angle = i * 6;
        draw_minor_tick(ctx, angle, PBL_IF_BW_ELSE(settings.BWTextColor3, settings.TextColor2));
      }


}


static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
  current_date = prv_tick_time->tm_mday;
  s_weekday = prv_tick_time->tm_wday;
  minutes = prv_tick_time->tm_min;
  hours = prv_tick_time->tm_hour % 12;

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  Layer *seconds_root_layer = window_get_root_layer(window);
  bounds_seconds = layer_get_bounds(seconds_root_layer);

  // Load fonts here, so they are available for the layers
    Digits_Font =  ffont_create_from_resource(RESOURCE_ID_FONT_DIGITS_FCTX);
    Date_Font =  ffont_create_from_resource(RESOURCE_ID_FONT_DATE_FCTX);


  // #ifdef PBL_PLATFORM_EMERY
  //   FontDigits = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITS_38));
  //   FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_12));
  //   FontBattery = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_12));
  //
  // #else
  //   FontDigits = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITS_34));
  //   FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_9));
  //   FontBattery = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_10));
  // #endif
//    FontLogo = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_8));
    FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));
    #ifdef PBL_BW
    FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_9));
    FontBattery = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_10));
    FontLogo = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_8));

    #endif
  // Subscribe to the connection service to get Bluetooth status updates.
  // Add the following code in prv_window_load
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });


  // Subscribe to the correct tick service based on settings
    if (settings.EnableSecondsHand) {
        if (settings.SecondsVisibleTime == 135) {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        } else {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(1000*settings.SecondsVisibleTime, timeout_handler,NULL);
        accel_tap_service_subscribe(accel_tap_handler);
        }
    }
    else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
    showSeconds = settings.EnableSecondsHand;



  s_bg_layer = layer_create(bounds);
  s_dial_layer = layer_create(bounds);
  s_canvas_qt_icon = layer_create(bounds);
     quiet_time_icon();
  s_canvas_bt_icon = layer_create(bounds);
    bool is_connected = connection_service_peek_pebble_app_connection();
    layer_set_hidden(s_canvas_bt_icon, is_connected);
  s_canvas_battery = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_canvas_second_hand = layer_create(bounds_seconds);
  s_dial_digits_layer = layer_create(bounds);
  s_date_battery_logo_layer = layer_create(bounds);

  // Change the order here
  layer_add_child(window_layer, s_bg_layer); //backforound, circles, major tick shoadow &tickmask
  layer_add_child(window_layer, s_dial_layer); //12 and 6 digit mask
  layer_add_child(window_layer, s_dial_digits_layer); //12 and 6 text fctx
  layer_add_child(window_layer, s_canvas_bt_icon);
  layer_add_child(window_layer, s_canvas_qt_icon);
  layer_add_child(window_layer, s_date_battery_logo_layer); //fctx version of text
  layer_add_child(window_layer, s_canvas_battery); //battery line
  layer_add_child(window_layer, s_canvas_layer);  //hour and minute hands
  layer_add_child(seconds_root_layer, s_canvas_second_hand);  //second hand

  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_dial_layer, layer_update_proc_dial_digits_mask);
  layer_set_update_proc(s_dial_digits_layer, update_dial_digits_fctx_layer);
  layer_set_update_proc(s_canvas_bt_icon, layer_update_proc_bt);
  layer_set_update_proc(s_canvas_qt_icon, layer_update_proc_qt);
  layer_set_update_proc(s_date_battery_logo_layer, update_logo_date_battery_fctx_layer);
  layer_set_update_proc(s_canvas_battery, layer_update_proc_battery_line);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);
  layer_set_update_proc(s_canvas_second_hand, layer_update_proc_seconds_hand);


}


static void prv_window_unload(Window *window) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_dial_layer);
  layer_destroy(s_canvas_second_hand);
  layer_destroy(s_canvas_battery);
  layer_destroy(s_canvas_bt_icon);
  layer_destroy(s_canvas_qt_icon);
  layer_destroy(s_dial_digits_layer);
  layer_destroy(s_date_battery_logo_layer);
  ffont_destroy(Digits_Font);
  ffont_destroy(Date_Font);
//  fonts_unload_custom_font(FontDigits);
//  fonts_unload_custom_font(FontLogo);
//  fonts_unload_custom_font(FontBattery);
  #ifdef PBL_BW
  fonts_unload_custom_font(FontDate);
  fonts_unload_custom_font(FontBattery);
  fonts_unload_custom_font(FontLogo);
  #endif
  fonts_unload_custom_font(FontBTQTIcons);
}

static void prv_init(void) {
  prv_load_settings();

  // Open AppMessage and set the message handler
  app_message_open(256, 256);
  app_message_register_inbox_received(prv_inbox_received_handler);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

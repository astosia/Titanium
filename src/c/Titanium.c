#include <pebble.h>
#include "Titanium.h"
#include "utils/weekday.h"
#include "utils/MathUtils.h"

// Constants for improved readability
//#define SECONDS_VISIBLE_TIME_ALWAYS_ON 135
#define SECONDS_TICK_INTERVAL_MS 1000

// Main window and layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_dial_layer;
static Layer *s_canvas_second_hand;
static Layer *s_canvas_bt_icon;
static Layer *s_canvas_qt_icon;
static Layer *s_canvas_battery_hand;
static GRect bounds;
// Fonts
static GFont FontDigits, FontDate, FontBTQTIcons;
// Time and date variables
static struct tm *prv_tick_time;
static int current_date;
static int s_weekday;
static ClaySettings settings;
static bool showSeconds;

// Date position struct for different platforms
typedef struct {
  int BottomXPosition;
  int BottomYPosition;
  int LeftxPosition;
  int xOffset;
  int yOffset;
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
} UIConfig;

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
.BottomXPosition = 46,
.BottomYPosition = 168,
.LeftxPosition = -5,
.xOffset = 22,
.yOffset = -8,
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
.QTIconYOffset = 21
};
#elif defined(PBL_BW)
static const UIConfig config = {
.BottomXPosition = 38,
.BottomYPosition = 126,
.LeftxPosition = 6,
.xOffset = 10,
.yOffset = -7,
.xyMaskOffset = 3,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 17,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18
};
#elif defined(PBL_ROUND)
static const UIConfig config = {
.BottomXPosition = 52,
.BottomYPosition = 132,
.LeftxPosition = 16,
.xOffset = 14,
.yOffset = -7,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 17,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18
};
#else // Default for other platforms
static const UIConfig config = {
.BottomXPosition = 38,
.BottomYPosition = 126,
.LeftxPosition = 7,
.xOffset = 10,
.yOffset = -7,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 17,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18
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
//static void bluetooth_vibe_icon (bool connected);
static void bg_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_dial(Layer *layer, GContext * ctx);
static void layer_update_proc_battery(Layer *layer, GContext * ctx);
static void layer_update_proc_seconds(Layer *layer, GContext * ctx);
static void canvas_update_proc(Layer *layer, GContext *ctx);
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
  settings.SecondsVisibleTime = 135;
  settings.EnableDate = true;
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
  settings.BWDateColor = GColorBlack;
  settings.BWBackgroundColor1 = GColorWhite;
  settings.BWBackgroundColor2 = GColorDarkGray;
  settings.BWTextColor1 = GColorWhite;
  settings.BWTextColor2 = GColorBlack;
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

//bool showSeconds = false;
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

// static void  start_seconds (void *context) {
//   if (settings.EnableSecondsHand && settings.SecondsVisibleTime == 135) {
//    showSeconds = true;
//   }
//  else if (settings.EnableSecondsHand && settings.SecondsVisibleTime < 135){
//    showSeconds = true;
//    s_timeout_timer = app_timer_register(1000*settings.SecondsVisibleTime, timeout_handler,NULL);
//  }
//  else{
//    showSeconds = false;
//   }
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "start event, visible seconds is %d",settings.SecondsVisibleTime);
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "start event show seconds is %d",showSeconds);
//
// }

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

// static void bluetooth_vibe_icon(bool connected) {
//   // Check if we are subscribed to the accel_tap service
//   if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
//     // Unsubscribe from accel_tap before the vibe
//     accel_tap_service_unsubscribe();
//   }
//
//   if (connected) {
//     layer_set_hidden(s_canvas_bt_icon, true);
//     layer_mark_dirty(s_canvas_bt_icon);
//     APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth is connected.");
//   } else {
//     layer_set_hidden(s_canvas_bt_icon, false);
//     layer_mark_dirty(s_canvas_bt_icon);
//     APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth is disconnected.");
//     if(settings.VibeOn){
//       // Vibrate if a vibe is wanted
//       vibes_double_pulse();
//     }
//   }
//
//   // Re-subscribe to accel_tap after the vibe
//   if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
//     accel_tap_service_subscribe(accel_tap_handler);
//   }
// }

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

// static void bluetooth_vibe_icon (bool connected) {
//   layer_set_hidden(s_canvas_bt_icon, connected);
//
//   if((!connected && !quiet_time_is_active()) ||(!connected && quiet_time_is_active() && settings.VibeOn)) {
//     if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
//       // Unsubscribe from accel_tap before the vibe
//       accel_tap_service_unsubscribe();
//       // Set the seconds hand state to hidden
//       showSeconds = false;
//       // Stop the seconds hand from updating
//       tick_timer_service_unsubscribe();
//       tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
//       layer_mark_dirty(s_canvas_second_hand);
//     }
//     // Issue a vibrating alert
//     vibes_double_pulse();
//
//     if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
//       // Re-subscribe to the tap service to re-enable tap-to-show
//       accel_tap_service_subscribe(accel_tap_handler);
//     }
//   }
// }

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
  Tuple *bwbg_color1_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor1);
  Tuple *bwbg_color2_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor2);
  Tuple *bwtext_color1_t = dict_find(iter, MESSAGE_KEY_BWTextColor1);
  Tuple *bwtext_color2_t = dict_find(iter, MESSAGE_KEY_BWTextColor2);
  Tuple *btqt_color_t = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *bwbtqt_color_t = dict_find(iter, MESSAGE_KEY_BWBTQTColor);
  Tuple *bwshadowon_t = dict_find(iter, MESSAGE_KEY_BWShadowOn);
  Tuple *font_t = dict_find(iter, MESSAGE_KEY_Font);


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
    settings_changed = true;
  }
  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_layer);
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
                  if (bwbtqt_color_t) {
                    settings.BWBTQTColor = GColorFromHEX(bwbtqt_color_t->value->int32);
                    layer_mark_dirty(s_canvas_bt_icon);
                    layer_mark_dirty(s_canvas_qt_icon);
                  }
                  theme_settings_changed = true;
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }


  // if (bwthemeselect_t) {
  //
  //   snprintf(settings.BWThemeSelect, sizeof(settings.BWThemeSelect), "%s", bwthemeselect_t -> value -> cstring);
  //
  //
  //   //settings.BWThemeSelect = (int) bwthemeselect_t->value->int32;
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "BW Theme is %s", settings.BWThemeSelect);
  //
  // //  if (settings.BWThemeSelect == 656670769 || settings.BWThemeSelect == 1) { // White background theme
  //   if (settings.BWThemeSelect == "wh") {
  //         settings.BWDateColor = GColorBlack;
  //       if (bwshadowon_t) {
  //         settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
  //       }
  //           if(settings.BWShadowOn){
  //             settings.BWBackgroundColor2 = GColorDarkGray;
  //           }
  //           else {
  //           settings.BWBackgroundColor2 = GColorWhite;
  //           }
  //       settings.BWBackgroundColor1 = GColorWhite;
  //       settings.BWTextColor1 = GColorWhite;
  //       settings.BWTextColor2 = GColorBlack;
  //       settings.BWBTQTColor = GColorBlack;
  //         theme_settings_changed = true;
  //           APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
  //   }
  //   //else if (settings.BWThemeSelect == 656670770 || settings.BWThemeSelect == 2) { // Black background theme
  //     else if (settings.BWThemeSelect == "bl") {
  //       settings.BWDateColor = GColorWhite;
  //       settings.BWBackgroundColor1 = GColorBlack;
  //       if (bwshadowon_t) {
  //         settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
  //       }
  //           if(settings.BWShadowOn){
  //             settings.BWBackgroundColor2 = GColorDarkGray;
  //           }
  //           else {
  //           settings.BWBackgroundColor2 = GColorBlack;
  //           }
  //       settings.BWTextColor1 = GColorBlack;
  //       settings.BWTextColor2 = GColorWhite;
  //       settings.BWBTQTColor = GColorWhite;
  //         theme_settings_changed = true;
  //           APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
  //   }
  //    else {
  //     if (bwdate_color_t) {
  //       settings.BWDateColor = GColorFromHEX(bwdate_color_t->value->int32);
  //       layer_mark_dirty(s_canvas_layer);
  //     }
  //     if (bwbg_color1_t) {
  //       settings.BWBackgroundColor1 = GColorFromHEX(bwbg_color1_t->value->int32);
  //       settings_changed = true;
  //     }
  //
  //     if (bwshadowon_t) {
  //       settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
  //
  //         if(settings.BWShadowOn){
  //           if (bwbg_color2_t) {
  //             settings.BWBackgroundColor2 = GColorFromHEX(bwbg_color2_t->value->int32);
  //             settings_changed = true;
  //           }
  //         }
  //         else {
  //         settings.BWBackgroundColor2 = settings.BWBackgroundColor1;
  //         }
  //     }
  //
  //     if (bwtext_color1_t) {
  //       settings.BWTextColor1 = GColorFromHEX(bwtext_color1_t->value->int32);
  //       settings_changed = true;
  //     }
  //     if (bwtext_color2_t) {
  //       settings.BWTextColor2 = GColorFromHEX(bwtext_color2_t->value->int32);
  //       settings_changed = true;
  //     }
  //     if (bwbtqt_color_t) {
  //       settings.BWBTQTColor = GColorFromHEX(bwbtqt_color_t->value->int32);
  //       layer_mark_dirty(s_canvas_bt_icon);
  //       layer_mark_dirty(s_canvas_qt_icon);
  //     }
  //     theme_settings_changed = true;
  //       APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
  //   }
  //
  //   }

    // Apply the selected theme colors directly to settings

    // The 'else' case for custom theme is already handled by the individual color tuples below
    // No need for a separate block here.



  if (bg_color1_t) {
    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
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
  }
  if (text_color2_t) {
    settings.TextColor2 = GColorFromHEX(text_color2_t->value->int32);
    layer_mark_dirty(s_bg_layer);
    //layer_mark_dirty(s_dial_layer);
    //layer_mark_dirty(s_canvas_layer);
  }
  if (text_color3_t) {
    settings.TextColor3 = GColorFromHEX(text_color3_t->value->int32);
    layer_mark_dirty(s_dial_layer);
  }
  if (date_color_t) {
    settings.DateColor = GColorFromHEX(date_color_t->value->int32);
    layer_mark_dirty(s_canvas_layer);
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
  if (btqt_color_t) {
    settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_canvas_qt_icon);
  }


  if (font_t) {
    settings.Font = font_t->value->int32;
    settings_changed = true;
  }

  if (settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_canvas_second_hand);
  }

  if (theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_qt_icon);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_canvas_battery_hand);
  }

  prv_save_settings();
  //start_seconds(NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);

// #ifdef LOGTIME
//   int seconds = tick_time->tm_sec;
//   int minutes = tick_time->tm_min;
//   int hours = tick_time->tm_hour;
//   APP_LOG(APP_LOG_LEVEL_INFO, "Time: %d:%d:%d", hours, minutes, seconds);
// #endif

  // Update hour and minute hands and the date on minute change
  if (units_changed & MINUTE_UNIT) {
    layer_mark_dirty(s_canvas_layer);
    if (settings.EnableDate && tick_time->tm_mday != current_date) {
      current_date = tick_time->tm_mday;
      s_weekday = tick_time->tm_wday;
    }
  }

  // Update seconds hand on second change, but only if it's visible
  if (showSeconds && (units_changed & SECOND_UNIT)) {
    layer_mark_dirty(s_canvas_second_hand);
  }

  // Add this line to hide or show the seconds hand layer
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
  //  graphics_context_set_fill_color(ctx, border_color);
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

    //  graphics_context_set_stroke_width(ctx, 3);
    //graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y), GPoint(shadow_endpoint.x + shadow_offset.x, shadow_endpoint.y + shadow_offset.y));

    // Now, draw the main tick on top. It covers the full length.
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

  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 1); // A thin line for minor ticks
  graphics_draw_line(ctx, p1, p2);
}

static void draw_circle_shadow(GContext *ctx, GPoint center, int radius, GColor circle_color, GColor shadow_color) {
    // Draw the shadow first, offset by a few pixels
    graphics_context_set_fill_color(ctx, shadow_color);
    graphics_fill_circle(ctx, GPoint(center.x , center.y ), radius);

    // Draw the main circle on top of the shadow
    graphics_context_set_fill_color(ctx, circle_color);
    graphics_fill_circle(ctx, GPoint(center.x + 4, center.y + 4), radius);
}


// Update procedure for the dial layer (the digits)
static void layer_update_proc_dial(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(s_dial_layer);

  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWBackgroundColor2, settings.BackgroundColor2));

#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 15, 23, 39, 7), 2, GCornersAll);
#else
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 14, 22, 36, 7), 2, GCornersAll);
#endif

  graphics_context_set_text_color(ctx, PBL_IF_BW_ELSE(settings.BWTextColor2, settings.TextColor3));
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));

#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 19, 0, 39, 27), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 19, bounds.size.h - 19, 31, 27), 1, GCornersAll);
#else
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 18, 0, 36, 26), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect((bounds.size.w / 2) - 13, bounds.size.h - 26, 28, 26), 1, GCornersAll);
#endif

  graphics_context_set_text_color(ctx, PBL_IF_BW_ELSE(settings.BWTextColor2, settings.TextColor3));

#ifdef PBL_PLATFORM_EMERY
  graphics_draw_text(ctx, "12", FontDigits, GRect((bounds.size.w / 2) - 27, -9, 52, 18), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, "6", FontDigits, GRect((bounds.size.w / 2) - 16, bounds.size.h - 40, 36, 40), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#else
  graphics_draw_text(ctx, "12", FontDigits, GRect((bounds.size.w / 2) - 23, -9, 44, 18), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, "6", FontDigits, GRect((bounds.size.w / 2) - 16, bounds.size.h - 38, 36, 40), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#endif

}



// Update procedure for the seconds hand layer
static void layer_update_proc_seconds(Layer *layer, GContext *ctx) {
  if (!showSeconds || !prv_tick_time) {
      // Do not draw the second hand if it should be hidden or if time data is not yet available
      return;
    }

  GRect bounds = layer_get_bounds(layer);
  int seconds = prv_tick_time->tm_sec;
  if (!settings.EnableSecondsHand || !showSeconds) {
    seconds = 0;
  }

  int seconds_angle = ((double)seconds / 60 * 360) - 90;

#ifdef PBL_PLATFORM_EMERY
  draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 20, 22, settings.SecondsHandColor);
#elif defined (PBL_COLOR)
  draw_line_hand(ctx, seconds_angle, PBL_IF_ROUND_ELSE(bounds.size.w / 2 - 35, bounds.size.w / 2 - 20), 22, settings.SecondsHandColor);
#else
  draw_line_hand(ctx, seconds_angle, bounds.size.w / 2 - 20, 22, settings.BWTextColor2);
#endif

  draw_seconds_center(ctx, PBL_IF_BW_ELSE(settings.BWTextColor1, settings.HoursHandColor), PBL_IF_BW_ELSE(settings.BWTextColor2, settings.SecondsHandColor));
}

static void layer_update_proc_battery(Layer *layer, GContext *ctx) {
  ///placeholder

// int s_battery_level = battery_state_service_peek().charge_percent;
//
// GRect BatteryRect =
//     (PBL_IF_ROUND_ELSE(
//       GRect(0,50+75-2-35,180,2),
//       GRect(0,68,144,2)));
//
//       #ifdef PBL_ROUND
//         int width_round = (s_battery_level * 180) / 100;
//       #else
//         int width_rect = (s_battery_level * 144) / 100;
//       #endif
//
//
//
//       GRect BatteryFillRect =
//           (PBL_IF_ROUND_ELSE(
//             GRect(0,50+75-2-35,width_round,2),
//             GRect(0,68,width_rect,2)));
//
//         char battperc[6];
//         snprintf(battperc, sizeof(battperc), "%d", s_battery_level);
//         strcat(battperc, "%");
//
// graphics_context_set_fill_color(ctx, ColorSelect(settings.FrameColor2,settings.FrameColor2N));// GColorBlack);
// graphics_fill_rect(ctx, BatteryRect, 0, GCornerNone);
//
// // Draw the battery bar
// graphics_context_set_fill_color(ctx, ColorSelect(settings.Text6Color, settings.Text6ColorN));
// graphics_fill_rect(ctx,BatteryFillRect, 0, GCornerNone);

}

//Update procedure for the BT Icon layer
static void layer_update_proc_bt(Layer * layer, GContext * ctx){
  GRect bounds = layer_get_bounds(layer);
  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour % 12;

//use this for testing
  // int minutes = 15;
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

// #ifdef PBL_PLATFORM_EMERY
//   GRect BTIconRect = GRect(93,36,20,20);
// #else
//   GRect BTIconRect =
//     (PBL_IF_ROUND_ELSE(
//       GRect(83,34,20,20),
//       GRect(65,34,20,20)));
//
// #endif

#ifdef PBL_COLOR
 graphics_context_set_text_color(ctx, settings.BTQTColor);
 #else
  graphics_context_set_text_color(ctx, settings.BWBTQTColor);
 #endif

 //bluetooth_callback(connection_service_peek_pebble_app_connection());

 graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);


}

//Update procedure for the QT Icon layer
static void layer_update_proc_qt(Layer * layer, GContext * ctx){

  GRect bounds = layer_get_bounds(layer);
  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour % 12;

//use this for testing
  // int minutes = 15;
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


  // #ifdef PBL_PLATFORM_EMERY
  //    GRect QTIconRect = GRect(92,57,20,20);
  // #else
  // GRect QTIconRect =
  //   (PBL_IF_ROUND_ELSE(
  //     GRect(82,55,20,20),
  //     GRect(64,55,20,20)));
  // #endif





 quiet_time_icon();


 #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, settings.BTQTColor);
  #else
   graphics_context_set_text_color(ctx, settings.BWBTQTColor);
  #endif

  graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);


}

// Update procedure for the main canvas layer (hour & minute hands and date)
static void canvas_update_proc(Layer *layer, GContext *ctx) {

 GRect bounds = layer_get_bounds(layer);

//use these for live version

  int minutes = prv_tick_time->tm_min;
  int hours = prv_tick_time->tm_hour % 12;
  int seconds = prv_tick_time->tm_sec;

  ///use below for testing and for screenshots
    // int seconds = 35;
    // int minutes = 15;
    // int hours = 9;

//   #ifdef SECOND
//     seconds = SECOND;
//   #endif
//
//
// #ifdef HOUR
//   hours = HOUR;
// #endif
//
// #ifdef MINUTE
//   minutes = MINUTE;
// #endif


  if (settings.EnableDate) {


          const char * sys_locale = i18n_get_system_locale();
          char weekday[10];
          fetchwday(s_weekday, sys_locale, weekday);

          char weekdaydraw[10];
          snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

          int date_to_display = current_date;
          char daynow[8];
          snprintf(daynow, sizeof(daynow), "%d", current_date);

          ///date position adjustments
          int xPosition;
          int yPosition;

          if (((hours > 7 && hours < 11) && (minutes > 7 && minutes < 23))||
          ((hours <5 && hours >1) && (minutes > 37 && minutes < 53))){
            // Bottom position
            xPosition = config.BottomXPosition;
            yPosition = config.BottomYPosition;
          }
          else if ((minutes > 7 && minutes < 23)||
          ((hours <5 && hours >1) && (minutes > 52 || minutes < 37))){
            // Left position
            xPosition = config.LeftxPosition;
            yPosition = bounds.size.h/2;
          }
          else{
            // Right position
            xPosition = bounds.size.w/2;
            yPosition = bounds.size.h/2;
          }




///main date featre position
          int xOffset = config.xOffset;
          int yOffset = config.yOffset;
          int xyMaskOffset = config.xyMaskOffset;
          int xWeekdayOffset = config.xWeekdayOffset;
          int yWeekdayDateOffset = config.yWeekdayDateOffset;
          int xDateOffset = config.xDateOffset;
          int ShadowAndMaskWidth = config.ShadowAndMaskWidth;
          int ShadowAndMaskHeight = config.ShadowAndMaskHeight;
          int WeekdayWidth = config.WeekdayWidth;
          int DateWidth = config.DateWidth;
          int WeekdayDateHeight = config.WeekdayDateHeight;
///draw date shadow
        //  #ifdef PBL_PLATFORM_EMERY

          GRect DateShadowRect =
           //  (0, offsetdate, bounds3.size.w, bounds1.size.h/4);
             GRect(xPosition + xOffset, yPosition + yOffset, ShadowAndMaskWidth, ShadowAndMaskHeight);

           GRect DateShadowMaskRect =
              GRect(xPosition + xOffset + xyMaskOffset, yPosition + yOffset + xyMaskOffset, ShadowAndMaskWidth, ShadowAndMaskHeight);


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

           //draw date complication



          GRect WeekdayRect =
           //  (0, offsetdate, bounds3.size.w, bounds1.size.h/4);
             GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

           GRect DateRect =
              GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);



             #ifdef PBL_BW
             graphics_context_set_text_color(ctx, settings.BWDateColor);
             #else
             graphics_context_set_text_color(ctx, settings.DateColor);
             #endif
             graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
             graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            }

   int hours_angle = ((double)hours / 12 * 360) + ((double)minutes / 60 * 360 / 12) + ((double)seconds / 60 * 360 / 60 / 12)  - 90;

 #ifdef PBL_PLATFORM_EMERY
   draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - 50, settings.HoursHandColor, settings.HoursHandBorderColor);
 #elif defined (PBL_COLOR)
   draw_fancy_hand_hour(ctx, hours_angle, PBL_IF_ROUND_ELSE(bounds.size.w / 2 - 50,bounds.size.w / 2 - 35), settings.HoursHandColor, settings.HoursHandBorderColor);
 #else
   draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - 35, settings.BWTextColor1, settings.BWTextColor2);
 #endif

   int minutes_angle = ((double)minutes / 60 * 360) + ((double)seconds / 60 * 360 / 60) - 90;
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
       int circle_radius = (bounds.size.h / 2) - 18; // Adjust this value as neededh
       int tick_mask_radius = circle_radius +12;
     #endif

     draw_circle_shadow(ctx, center, circle_radius, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1), PBL_IF_BW_ELSE(settings.BWBackgroundColor2, settings.BackgroundColor2));

      for (int i = 0; i < 12; i++) {
        if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick_shadow(ctx, angle, 15, PBL_IF_BW_ELSE(settings.BWTextColor1, settings.HoursHandColor), PBL_IF_BW_ELSE(settings.BWTextColor2, settings.BackgroundColor2));
      }

      draw_major_tick_mask(ctx, tick_mask_radius, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));

      for (int i = 0; i < 12; i++) {
        if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1), PBL_IF_BW_ELSE(settings.BWTextColor2, settings.TextColor2));
      }

      for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        int angle = i * 6;
        draw_minor_tick(ctx, angle, PBL_IF_BW_ELSE(settings.BWTextColor2, settings.TextColor2));
      }

}


static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
  current_date = prv_tick_time->tm_mday;
  s_weekday = prv_tick_time->tm_wday;
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  // Load fonts here, so they are available for the layers


  #ifdef PBL_PLATFORM_EMERY
    FontDigits = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITS_38));
    FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_12));
  #else
    FontDigits = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITS_34));
    FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_9));
  #endif
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));

  // Subscribe to the connection service to get Bluetooth status updates.
  // Add the following code in prv_window_load
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });


  // if (settings.EnableSecondsHand) {
  //     accel_tap_service_subscribe(accel_tap_handler);
  //   }

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
  s_canvas_battery_hand = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_canvas_second_hand = layer_create(bounds);

  // Change the order here
  layer_add_child(window_layer, s_bg_layer);
  layer_add_child(window_layer, s_dial_layer);
  layer_add_child(window_layer, s_canvas_bt_icon);
  layer_add_child(window_layer, s_canvas_qt_icon);
  layer_add_child(window_layer, s_canvas_battery_hand);
  layer_add_child(window_layer, s_canvas_layer);
  layer_add_child(window_layer, s_canvas_second_hand);


  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_dial_layer, layer_update_proc_dial);
  layer_set_update_proc(s_canvas_bt_icon, layer_update_proc_bt);
  layer_set_update_proc(s_canvas_qt_icon, layer_update_proc_qt);
  layer_set_update_proc(s_canvas_battery_hand, layer_update_proc_battery);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_update_proc(s_canvas_second_hand, layer_update_proc_seconds);
}


static void prv_window_unload(Window *window) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_dial_layer);
  layer_destroy(s_canvas_second_hand);
  layer_destroy(s_canvas_battery_hand);
  layer_destroy(s_canvas_bt_icon);
  layer_destroy(s_canvas_qt_icon);
  fonts_unload_custom_font(FontDigits);
  ///fonts_unload_custom_font(FontDigits2);
  fonts_unload_custom_font(FontDate);
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

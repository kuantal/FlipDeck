/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Types, Constants & Data Structures
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>

/* ──────────────────────────────────────────────────────────────────
   HID Modifier Constants
   ────────────────────────────────────────────────────────────────── */
#define MOD_CTRL  KEY_MOD_LEFT_CTRL
#define MOD_SHIFT KEY_MOD_LEFT_SHIFT
#define MOD_ALT   KEY_MOD_LEFT_ALT
#define MOD_WIN   KEY_MOD_LEFT_GUI

/* Common keyboard shortcuts */
#define HID_CTRL_S          (MOD_CTRL  | HID_KEYBOARD_S)
#define HID_CTRL_Z          (MOD_CTRL  | HID_KEYBOARD_Z)
#define HID_CTRL_Y          (MOD_CTRL  | HID_KEYBOARD_Y)
#define HID_CTRL_T          (MOD_CTRL  | HID_KEYBOARD_T)
#define HID_CTRL_W          (MOD_CTRL  | HID_KEYBOARD_W)
#define HID_CTRL_R          (MOD_CTRL  | HID_KEYBOARD_R)
#define HID_CTRL_SHIFT_ESC  (MOD_CTRL  | MOD_SHIFT | HID_KEYBOARD_ESCAPE)
#define HID_CTRL_SHIFT_R    (MOD_CTRL  | MOD_SHIFT | HID_KEYBOARD_R)
#define HID_CTRL_SHIFT_S    (MOD_CTRL  | MOD_SHIFT | HID_KEYBOARD_S)
#define HID_CTRL_SHIFT_M    (MOD_CTRL  | MOD_SHIFT | HID_KEYBOARD_M)
#define HID_CTRL_SHIFT_V    (MOD_CTRL  | MOD_SHIFT | HID_KEYBOARD_V)
#define HID_ALT_LEFT        (MOD_ALT   | HID_KEYBOARD_LEFT_ARROW)
#define HID_ALT_RIGHT       (MOD_ALT   | HID_KEYBOARD_RIGHT_ARROW)
#define HID_ALT_UP          (MOD_ALT   | HID_KEYBOARD_UP_ARROW)
#define HID_ALT_DOWN        (MOD_ALT   | HID_KEYBOARD_DOWN_ARROW)
#define HID_WIN_D           (MOD_WIN   | HID_KEYBOARD_D)
#define HID_WIN_L           (MOD_WIN   | HID_KEYBOARD_L)
#define HID_WIN_SHIFT_S     (MOD_WIN   | MOD_SHIFT | HID_KEYBOARD_S)
#define HID_CTRL_LEFT_KEY   (MOD_CTRL  | HID_KEYBOARD_LEFT_ARROW)
#define HID_CTRL_RIGHT_KEY  (MOD_CTRL  | HID_KEYBOARD_RIGHT_ARROW)
#define HID_WIN_UP          (MOD_WIN   | HID_KEYBOARD_UP_ARROW)
#define HID_WIN_DOWN        (MOD_WIN   | HID_KEYBOARD_DOWN_ARROW)
#define HID_WIN_LEFT_KEY    (MOD_WIN   | HID_KEYBOARD_LEFT_ARROW)
#define HID_WIN_RIGHT_KEY   (MOD_WIN   | HID_KEYBOARD_RIGHT_ARROW)
#define HID_CTRL_WIN_RIGHT  (MOD_CTRL  | MOD_WIN | HID_KEYBOARD_RIGHT_ARROW)
#define HID_CTRL_WIN_LEFT   (MOD_CTRL  | MOD_WIN | HID_KEYBOARD_LEFT_ARROW)
#define HID_ALT_ENTER       (MOD_ALT   | HID_KEYBOARD_RETURN)
#define HID_SHIFT_TAB       (MOD_SHIFT | HID_KEYBOARD_TAB)
#define HID_CTRL_PLUS       (MOD_CTRL  | HID_KEYBOARD_EQUAL_SIGN)
#define HID_CTRL_MINUS_KEY  (MOD_CTRL  | HID_KEYBOARD_MINUS)
#define HID_CTRL_ALT_Z      (MOD_CTRL  | MOD_ALT  | HID_KEYBOARD_Z)

/* ──────────────────────────────────────────────────────────────────
   Constants
   ────────────────────────────────────────────────────────────────── */
#define MACRO_COUNT      5
#define MACRO_LEN       64
#define MACRO_PAGE_IDX   5
#define PROFILE_COUNT    3
#define LABEL_LEN       22
#define HELP_LINES       4
#define HELP_SCREENS     6
#define HELP_LINE_LEN   22

/* ──────────────────────────────────────────────────────────────────
   Action Model
   ────────────────────────────────────────────────────────────────── */
typedef enum {
    ActionNone = 0,
    ActionConsumer,  /* Media / Consumer key  */
    ActionKey,       /* Keyboard shortcut     */
    ActionTypeText,  /* Type text char-by-char */
} ActionType;

typedef struct {
    ActionType  type;
    uint16_t    hid_code;
    const char* text;   /* text to type (ActionTypeText) */
    const char* label;  /* short label shown on screen    */
} Action;

typedef struct {
    const char* name;
    Action      up, down, left, right, ok;
} Page;

/* ──────────────────────────────────────────────────────────────────
   Application State
   ────────────────────────────────────────────────────────────────── */
typedef enum {
    AppScreenMenu = 0,
    AppScreenPage,
    AppScreenHelp,
    AppScreenMacroEdit,
} AppScreen;

typedef struct {
    AppScreen   screen;
    uint8_t     page_idx;
    uint8_t     help_screen;
    const char* last_label;     /* label of last executed action    */
    uint32_t    last_tick;      /* furi_get_tick() when it was run  */
    uint8_t     profile_idx;    /* active profile 0..PROFILE_COUNT-1 */
    uint8_t     edit_macro_idx; /* macro slot being edited (0-4)    */
    char        edit_buf[MACRO_LEN]; /* working copy in editor      */
    TextInput*  text_input;     /* Flipper's built-in text input    */
    ViewDispatcher* view_dispatcher; /* For text input modal        */
    ViewPort*   viewport;       /* Main viewport                    */
    Gui*        gui;            /* GUI instance                     */
    bool        text_input_active; /* Is text input currently shown */
} AppState;

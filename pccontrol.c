/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Flipper Zero PC Controller (Loupedeck-style)
 *  Control your PC over USB HID: media, system, browser,
 *  VS Code, OBS, and text macros.
 * ╚══════════════════════════════════════════════════════════════╝
 *  Strings: see lang_en.json
 */

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb_hid.h>
#include <furi_hal_usb.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <string.h>

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
   Page Definitions  (6 pages x 5 actions = 30 shortcuts)
   Labels: Left/Right ≤ 6 chars, OK ≤ 8 chars, Up/Down ≤ 21 chars
   ────────────────────────────────────────────────────────────────── */
static const Page PAGES[] = {

    /* 0 -- MEDIA */
    { "MEDIA",
      {ActionConsumer, HID_CONSUMER_VOLUME_INCREMENT,    NULL, "Vol+"       },
      {ActionConsumer, HID_CONSUMER_VOLUME_DECREMENT,    NULL, "Vol-"       },
      {ActionConsumer, HID_CONSUMER_SCAN_PREVIOUS_TRACK, NULL, "Prev"       },
      {ActionConsumer, HID_CONSUMER_SCAN_NEXT_TRACK,     NULL, "Next"       },
      {ActionConsumer, HID_CONSUMER_PLAY_PAUSE,          NULL, "Play/Pause" },
    },

    /* 1 -- SYSTEM */
    { "SYSTEM",
      {ActionConsumer, HID_CONSUMER_BRIGHTNESS_INCREMENT, NULL, "Bright+"  },
      {ActionKey,      HID_WIN_D,                          NULL, "Desktop"  },
      {ActionKey,      HID_WIN_L,                          NULL, "Lock"     },
      {ActionKey,      HID_WIN_SHIFT_S,                    NULL, "Snip"     },
      {ActionKey,      HID_CTRL_SHIFT_ESC,                 NULL, "Task Mgr" },
    },

    /* 2 -- BROWSER */
    { "BROWSER",
      {ActionKey, HID_KEYBOARD_F5,       NULL, "Reload"  },
      {ActionKey, HID_KEYBOARD_F11,      NULL, "Fullscr" },
      {ActionKey, HID_ALT_LEFT,          NULL, "Back"    },
      {ActionKey, HID_ALT_RIGHT,         NULL, "Fwd"     },
      {ActionKey, HID_CTRL_T,            NULL, "New Tab" },
    },

    /* 3 -- VS CODE */
    { "VS CODE",
      {ActionKey, HID_ALT_UP,   NULL, "Line Up" },
      {ActionKey, HID_ALT_DOWN, NULL, "Line Dn" },
      {ActionKey, HID_CTRL_Z,   NULL, "Undo"    },
      {ActionKey, HID_CTRL_Y,   NULL, "Redo"    },
      {ActionKey, HID_CTRL_S,   NULL, "Save"    },
    },

    /* 4 -- OBS  (configure shortcuts in OBS > Settings > Hotkeys) */
    { "OBS",
      {ActionKey, HID_CTRL_SHIFT_M,   NULL, "Mute"   },
      {ActionKey, HID_CTRL_SHIFT_V,   NULL, "Camera" },
      {ActionKey, HID_CTRL_LEFT_KEY,  NULL, "<Scene" },
      {ActionKey, HID_CTRL_RIGHT_KEY, NULL, "Scene>" },
      {ActionKey, HID_CTRL_SHIFT_R,   NULL, "Record" },
    },

    /* 5 -- MACRO (edit text strings to customise) */
    { "MACRO",
      {ActionTypeText, 0, "Hello,\nHow are you?\n",  "Hello"   },
      {ActionTypeText, 0, "Best regards,\n",          "Sign Off" },
      {ActionTypeText, 0, "admin@example.com",         "Email"   },
      {ActionTypeText, 0, "+1 555 000 0000",            "Phone"   },
      {ActionTypeText, 0, "https://github.com/",        "GitHub"  },
    },
};

#define PAGE_COUNT ((uint8_t)(sizeof(PAGES) / sizeof(PAGES[0])))

/* ──────────────────────────────────────────────────────────────────
   Help Screen Content  (strings from lang_en.json)
   Max 21 chars per line to fit 128 px at FontSecondary
   ────────────────────────────────────────────────────────────────── */
#define HELP_LINES      4
#define HELP_SCREENS    5
#define HELP_LINE_LEN  22  /* 21 chars + null terminator */

static const char HELP[HELP_SCREENS][HELP_LINES][HELP_LINE_LEN] = {
    /* 0 - General */
    {
        "=GENERAL=",
        "USB-C: connect to PC",
        "Up/Dn: select page",
        "OK: open page",
    },
    /* 1 - Inside a page */
    {
        "=IN PAGE=",
        "5 dirs = 5 actions",
        "Long L/R: next page",
        "Back: return to menu",
    },
    /* 2 - Pages list */
    {
        "=PAGES=",
        "0:Media  1:System",
        "2:Browser 3:VSCode",
        "4:OBS    5:Macro",
    },
    /* 3 - Macro & OBS note */
    {
        "=MACRO & OBS=",
        "Macro: types text",
        "OBS: set in OBS app",
        "Ctrl+Shift+R/S/M/V",
    },
    /* 4 - Tips */
    {
        "=TIPS=",
        "Vibration feedback",
        "Long OK: open help",
        "Back in menu: exit",
    },
};

/* ──────────────────────────────────────────────────────────────────
   Application State
   ────────────────────────────────────────────────────────────────── */
typedef enum {
    AppScreenMenu = 0,
    AppScreenPage,
    AppScreenHelp,
} AppScreen;

typedef struct {
    AppScreen screen;
    uint8_t   page_idx;
    uint8_t   help_screen;
} AppState;

/* ──────────────────────────────────────────────────────────────────
   HID Helpers
   ────────────────────────────────────────────────────────────────── */
static void hid_tap_key(uint16_t key) {
    furi_hal_hid_kb_press(key);
    furi_delay_ms(20);
    furi_hal_hid_kb_release(key);
    furi_delay_ms(10);
}

static void hid_tap_consumer(uint16_t key) {
    furi_hal_hid_consumer_key_press(key);
    furi_delay_ms(20);
    furi_hal_hid_consumer_key_release(key);
    furi_delay_ms(10);
}

static void hid_type_text(const char* text) {
    while(*text) {
        char c = *text++;
        if(c == '\n') { hid_tap_key(HID_KEYBOARD_RETURN); continue; }

        bool need_shift = false;
        uint16_t key    = 0;

        if(c >= 'a' && c <= 'z')      { key = HID_KEYBOARD_A + (c - 'a'); }
        else if(c >= 'A' && c <= 'Z') { key = HID_KEYBOARD_A + (c - 'A'); need_shift = true; }
        else if(c >= '1' && c <= '9') { key = HID_KEYBOARD_1 + (c - '1'); }
        else if(c == '0')             { key = HID_KEYBOARD_0; }
        else if(c == ' ')             { key = HID_KEYBOARD_SPACEBAR; }
        else if(c == '.')             { key = HID_KEYBOARD_DOT; }
        else if(c == ',')             { key = HID_KEYBOARD_COMMA; }
        else if(c == '@')             { key = HID_KEYBOARD_2;          need_shift = true; }
        else if(c == '+')             { key = HID_KEYBOARD_EQUAL_SIGN; need_shift = true; }
        else if(c == ':')             { key = HID_KEYBOARD_SEMICOLON;  need_shift = true; }
        else if(c == '/')             { key = HID_KEYBOARD_SLASH; }
        else if(c == '-')             { key = HID_KEYBOARD_MINUS; }

        if(key) hid_tap_key(need_shift ? (MOD_SHIFT | key) : key);
        furi_delay_ms(30);
    }
}

static void execute_action(const Action* action) {
    if(!action) return;
    switch(action->type) {
    case ActionConsumer:  hid_tap_consumer(action->hid_code);           break;
    case ActionKey:       hid_tap_key(action->hid_code);                break;
    case ActionTypeText:  if(action->text) hid_type_text(action->text); break;
    default: break;
    }
}

/* ──────────────────────────────────────────────────────────────────
   Draw Callback
   ────────────────────────────────────────────────────────────────── */
static void draw_callback(Canvas* canvas, void* ctx) {
    const AppState* st = (const AppState*)ctx;
    canvas_clear(canvas);

    if(st->screen == AppScreenMenu) {
        /* ── ANA MENU ── */
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "FlipDeck");
        canvas_draw_line(canvas, 0, 13, 127, 13);

        int8_t start = (int8_t)st->page_idx - 1;
        if(start < 0) start = 0;
        if(start > (int8_t)(PAGE_COUNT - 3)) start = (int8_t)(PAGE_COUNT - 3);
        if(start < 0) start = 0;

        for(uint8_t i = 0; i < 3 && (start + (int8_t)i) < (int8_t)PAGE_COUNT; i++) {
            uint8_t idx = (uint8_t)(start + (int8_t)i);
            int y = 15 + (int)(i * 12);
            bool selected = (idx == st->page_idx);

            if(selected) {
                canvas_draw_rbox(canvas, 0, y, 127, 11, 2);
                canvas_set_color(canvas, ColorWhite);
            }
            canvas_set_font(canvas, FontSecondary);
            char row[22];
            snprintf(row, sizeof(row), "%d. %s", idx + 1, PAGES[idx].name);
            canvas_draw_str(canvas, 4, y + 8, row);
            if(selected) canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_line(canvas, 0, 52, 127, 52);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 30, 63, AlignCenter, AlignBottom, "OK:Open");
        canvas_draw_str_aligned(canvas, 96, 63, AlignCenter, AlignBottom, "LongOK:Help");

    } else if(st->screen == AppScreenPage) {
        /* ── PAGE VIEW ── */
        const Page* p = &PAGES[st->page_idx];

        char title[22];
        snprintf(title, sizeof(title), "[%d/%d] %s",
                 st->page_idx + 1, PAGE_COUNT, p->name);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, title);
        canvas_draw_line(canvas, 0, 12, 127, 12);

        canvas_set_font(canvas, FontSecondary);

        /* UP */
        canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignTop, p->up.label);
        /* LEFT */
        canvas_draw_str(canvas, 2, 36, p->left.label);
        /* OK — merkez kutu */
        canvas_draw_rframe(canvas, 39, 25, 50, 15, 3);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, p->ok.label);
        /* RIGHT */
        canvas_draw_str_aligned(canvas, 126, 36, AlignRight, AlignCenter, p->right.label);
        /* DOWN */
        canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignBottom, p->down.label);

        canvas_draw_line(canvas, 0, 52, 127, 52);
        canvas_set_font(canvas, FontSecondary);
        /* Split footer so it never overflows the 128 px width */
        canvas_draw_str_aligned(canvas,   2, 63, AlignLeft,  AlignBottom, "Back:Menu");
        canvas_draw_str_aligned(canvas, 126, 63, AlignRight, AlignBottom, "LongL/R:Pg");

    } else {
        /* ── HELP SCREEN ── */
        uint8_t hs = st->help_screen;

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "HELP");
        canvas_draw_line(canvas, 0, 13, 127, 13);

        canvas_set_font(canvas, FontSecondary);
        for(uint8_t i = 0; i < HELP_LINES; i++) {
            canvas_draw_str(canvas, 2, 16 + i * 9, HELP[hs][i]);
        }

        canvas_draw_line(canvas, 0, 52, 127, 52);
        char pg[24];
        snprintf(pg, sizeof(pg), "< %d/%d > Back:Close", hs + 1, HELP_SCREENS);
        canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, pg);
    }
}

/* ──────────────────────────────────────────────────────────────────
   Input Callback
   ────────────────────────────────────────────────────────────────── */
static void input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = (FuriMessageQueue*)ctx;
    furi_message_queue_put(queue, event, FuriWaitForever);
}

/* ──────────────────────────────────────────────────────────────────
   Application Entry Point
   ────────────────────────────────────────────────────────────────── */
int32_t flip_deck_main(void* p) {
    UNUSED(p);

    /* Enable USB HID mode */
    FuriHalUsbInterface* usb_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    if(!furi_hal_usb_set_config(&usb_hid, NULL)) {
        return -1;
    }
    furi_delay_ms(500);

    AppState* st = malloc(sizeof(AppState));
    furi_assert(st);
    st->screen      = AppScreenMenu;
    st->page_idx    = 0;
    st->help_screen = 0;

    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, draw_callback, st);
    view_port_input_callback_set(vp, input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(queue, &event, 100) != FuriStatusOk) {
            view_port_update(vp);
            continue;
        }

        bool needs_update = true;

        if(st->screen == AppScreenMenu) {
            if(event.type == InputTypeShort) {
                switch(event.key) {
                case InputKeyUp:
                    if(st->page_idx > 0) st->page_idx--;
                    break;
                case InputKeyDown:
                    if(st->page_idx < PAGE_COUNT - 1) st->page_idx++;
                    break;
                case InputKeyOk:
                    st->screen = AppScreenPage;
                    break;
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    needs_update = false;
                    break;
                }
            } else if(event.type == InputTypeLong && event.key == InputKeyOk) {
                st->screen      = AppScreenHelp;
                st->help_screen = 0;
            } else {
                needs_update = false;
            }

        } else if(st->screen == AppScreenPage) {
            const Page* pg = &PAGES[st->page_idx];
            const Action* action = NULL;

            if(event.type == InputTypeLong) {
                if(event.key == InputKeyLeft) {
                    if(st->page_idx > 0) st->page_idx--;
                } else if(event.key == InputKeyRight) {
                    if(st->page_idx < PAGE_COUNT - 1) st->page_idx++;
                } else if(event.key == InputKeyOk) {
                    st->screen      = AppScreenHelp;
                    st->help_screen = 0;
                } else {
                    needs_update = false;
                }
            } else if(event.type == InputTypeShort) {
                switch(event.key) {
                case InputKeyUp:    action = &pg->up;    break;
                case InputKeyDown:  action = &pg->down;  break;
                case InputKeyLeft:  action = &pg->left;  break;
                case InputKeyRight: action = &pg->right; break;
                case InputKeyOk:    action = &pg->ok;    break;
                case InputKeyBack:
                    st->screen = AppScreenMenu;
                    break;
                default:
                    needs_update = false;
                    break;
                }
                if(action && action->type != ActionNone) {
                    execute_action(action);
                    notification_message(notif, &sequence_single_vibro);
                }
            } else {
                needs_update = false;
            }

        } else {
            /* HELP */
            if(event.type == InputTypeShort) {
                switch(event.key) {
                case InputKeyRight:
                    if(st->help_screen < HELP_SCREENS - 1) st->help_screen++;
                    break;
                case InputKeyLeft:
                    if(st->help_screen > 0) st->help_screen--;
                    break;
                case InputKeyBack:
                case InputKeyOk:
                    st->screen = AppScreenMenu;
                    break;
                default:
                    needs_update = false;
                    break;
                }
            } else {
                needs_update = false;
            }
        }

        if(needs_update) view_port_update(vp);
    }

    /* Cleanup */
    view_port_enabled_set(vp, false);
    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    free(st);

    furi_hal_usb_unlock();
    furi_hal_usb_set_config(usb_prev, NULL);

    return 0;
}

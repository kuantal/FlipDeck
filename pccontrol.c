/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Flipper Zero PC Controller (Loupedeck-style)
 *  USB HID + BLE HID auto-fallback: media, system, browser,
 *  VS Code, OBS, gaming, Photoshop, WM and text macros.
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
#include <storage/storage.h>
#include <furi_hal_power.h>

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
/* v2: Window management, gaming, photo shortcuts */
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
   Macro SD Storage — runtime text buffers
   ────────────────────────────────────────────────────────────────── */
#define MACRO_COUNT      5
#define MACRO_LEN       64
#define MACRO_PAGE_IDX   5
#define PROFILE_COUNT    3
#define DATA_DIR        "/ext/apps_data/flip_deck"
#define STATE_FILE      "/ext/apps_data/flip_deck/state.bin"

static char s_macro_path[64];
static void build_macro_path(uint8_t pidx) {
    snprintf(s_macro_path, sizeof(s_macro_path),
             "/ext/apps_data/flip_deck/macros_%u.txt", (unsigned)pidx);
}

/* ──────────────────────────────────────────────────────────────────
   Transport Layer — USB HID with live plug/unplug detection
   Checked every 2 s via VBUS voltage (> 4.0 V = connected).
   When USB is removed a full-screen overlay blocks HID sends
   until the cable is re-inserted (auto-resume, single vibro).
   ────────────────────────────────────────────────────────────────── */
typedef enum {
    TransportNone = 0,
    TransportUsb,
    TransportWaitUsb,  /* USB was once active but cable was removed */
} TransportMode;

static TransportMode        g_transport  = TransportNone;
static FuriHalUsbInterface* g_usb_prev   = NULL;

static bool transport_activate_usb(void) {
    if(g_usb_prev == NULL)
        g_usb_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    if(!furi_hal_usb_set_config(&usb_hid, NULL)) return false;
    furi_delay_ms(300);
    g_transport = TransportUsb;
    return true;
}

/* Returns true if the transport state changed this call */
static bool transport_check(NotificationApp* notif) {
    bool vbus = (furi_hal_power_get_usb_voltage() > 4.0f);
    if(vbus && g_transport != TransportUsb) {
        TransportMode prev = g_transport;
        if(transport_activate_usb()) {
            if(prev != TransportNone)  /* skip vibro on first init */
                notification_message(notif, &sequence_single_vibro);
            return true;
        }
    } else if(!vbus && g_transport == TransportUsb) {
        g_transport = TransportWaitUsb;
        notification_message(notif, &sequence_single_vibro);
        return true;
    }
    return false;
}

/* Default texts; overwritten if macros.txt exists on SD card */
static char macro_buf[MACRO_COUNT][MACRO_LEN] = {
    "Hello,\nHow are you?\n",
    "Best regards,\n",
    "admin@example.com",
    "+1 555 000 0000",
    "https://github.com/",
};

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
   Page Definitions  (9 pages x 5 actions = 45 shortcuts)
   Labels: Left/Right ≤ 6 chars, OK ≤ 8 chars, Up/Down ≤ 21 chars
   Non-const: macro text pointers are patched at runtime.
   ────────────────────────────────────────────────────────────────── */
static Page PAGES[] = {

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

    /* 5 -- MACRO (text pointers patched from macro_buf[] at runtime) */
    { "MACRO",
      {ActionTypeText, 0, NULL, "Hello"  },
      {ActionTypeText, 0, NULL, "SignOff"},
      {ActionTypeText, 0, NULL, "Email"  },
      {ActionTypeText, 0, NULL, "Phone"  },
      {ActionTypeText, 0, NULL, "GitHub" },
    },

    /* 6 -- GAMING */
    { "GAMING",
      {ActionKey, HID_KEYBOARD_T,           NULL, "PTT"     },
      {ActionKey, HID_CTRL_SHIFT_M,         NULL, "Mute"    },
      {ActionKey, HID_KEYBOARD_F12,         NULL, "Scrnshot"},
      {ActionKey, HID_SHIFT_TAB,            NULL, "Overlay" },
      {ActionKey, HID_ALT_ENTER,            NULL, "Fullscr" },
    },

    /* 7 -- PHOTOSHOP */
    { "PHOTOSHP",
      {ActionKey, HID_CTRL_PLUS,    NULL, "Zoom+"  },
      {ActionKey, HID_CTRL_MINUS_KEY, NULL, "Zoom-" },
      {ActionKey, HID_CTRL_Z,       NULL, "Undo"   },
      {ActionKey, HID_CTRL_ALT_Z,   NULL, "History"},
      {ActionKey, HID_CTRL_SHIFT_S, NULL, "Save As"},
    },

    /* 8 -- WINDOWS WM */
    { "WIN WM",
      {ActionKey, HID_WIN_UP,        NULL, "Maximze"},
      {ActionKey, HID_WIN_DOWN,      NULL, "Minimize"},
      {ActionKey, HID_WIN_LEFT_KEY,  NULL, "<Snap"  },
      {ActionKey, HID_WIN_RIGHT_KEY, NULL, "Snap>"  },
      {ActionKey, HID_CTRL_WIN_RIGHT,NULL, "NxDesk" },
    },

    /* 9 -- NUMPAD */
    { "NUMPAD",
      {ActionTypeText, 0, "123", "123"  },
      {ActionTypeText, 0, "456", "456"  },
      {ActionTypeText, 0, "789", "789"  },
      {ActionTypeText, 0, "0",   "0"    },
      {ActionTypeText, 0, ".",   "Dot"  },
    },
};

#define PAGE_COUNT ((uint8_t)(sizeof(PAGES) / sizeof(PAGES[0])))

/* ──────────────────────────────────────────────────────────────────
   Help Screen Content  (strings from lang_en.json)
   Max 21 chars per line to fit 128 px at FontSecondary
   ────────────────────────────────────────────────────────────────── */
#define HELP_LINES      4
#define HELP_SCREENS    6
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
    /* 2 - Pages 1-6 */
    {
        "=PAGES 1-6=",
        "1:Media  2:System",
        "3:Browser 4:VSCode",
        "5:OBS    6:Macro",
    },
    /* 3 - Pages 7-10 */
    {
        "=PAGES 7-10=",
        "7:Gaming 8:Photshp",
        "9:Win WM",
        "10:Numpad",
    },
    /* 4 - Macro editor */
    {
        "=MACRO EDITOR=",
        "LongUp/Dn: edit slot",
        "U/D:chr R:add L:del",
        "OK:save Bk:cancel",
    },
    /* 5 - Tips */
    {
        "=TIPS=",
        "MenuLU/D:switch prof",
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
    uint8_t     edit_char_idx;  /* index into EDITOR_CHARS          */
    char        edit_buf[MACRO_LEN]; /* working copy in editor      */
} AppState;

/* ──────────────────────────────────────────────────────────────────
   Storage Helpers — macro load + state persist
   ────────────────────────────────────────────────────────────────── */
static void macros_load_sd(uint8_t pidx) {
    build_macro_path(pidx);
    Storage* store = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, s_macro_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint8_t mx = 0, ci = 0;
        char line[MACRO_LEN];
        char c;
        while(mx < MACRO_COUNT && storage_file_read(f, &c, 1) > 0) {
            if(c == '\r') continue;
            if(c == '\n' || ci >= MACRO_LEN - 1) {
                line[ci] = '\0';
                if(ci > 0) {
                    strncpy(macro_buf[mx], line, MACRO_LEN - 1);
                    macro_buf[mx][MACRO_LEN - 1] = '\0';
                    mx++; ci = 0;
                }
            } else { line[ci++] = c; }
        }
        if(ci > 0 && mx < MACRO_COUNT) {
            line[ci] = '\0';
            strncpy(macro_buf[mx], line, MACRO_LEN - 1);
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
    /* Patch MACRO page text pointers */
    PAGES[MACRO_PAGE_IDX].up.text    = macro_buf[0];
    PAGES[MACRO_PAGE_IDX].down.text  = macro_buf[1];
    PAGES[MACRO_PAGE_IDX].left.text  = macro_buf[2];
    PAGES[MACRO_PAGE_IDX].right.text = macro_buf[3];
    PAGES[MACRO_PAGE_IDX].ok.text    = macro_buf[4];
}

static void macro_save_sd(uint8_t pidx) {
    build_macro_path(pidx);
    Storage* store = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(store, DATA_DIR);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, s_macro_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        for(uint8_t i = 0; i < MACRO_COUNT; i++) {
            storage_file_write(f, macro_buf[i], strlen(macro_buf[i]));
            storage_file_write(f, "\n", 1);
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}

static void state_save(const AppState* st) {
    Storage* store = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(store, DATA_DIR);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, STATE_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint8_t buf[2] = {st->page_idx, st->profile_idx};
        storage_file_write(f, buf, 2);
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}

static void state_load(AppState* st) {
    Storage* store = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(store);
    if(storage_file_open(f, STATE_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint8_t buf[2] = {0, 0};
        if(storage_file_read(f, buf, 2) > 0) {
            if(buf[0] < PAGE_COUNT)    st->page_idx    = buf[0];
            if(buf[1] < PROFILE_COUNT) st->profile_idx = buf[1];
        }
        storage_file_close(f);
    }
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
}

/* ──────────────────────────────────────────────────────────────────
   HID Helpers
   ────────────────────────────────────────────────────────────────── */
static void hid_tap_key(uint16_t key) {
    if(g_transport != TransportUsb) return;  /* block when no USB */
    furi_hal_hid_kb_press(key);
    furi_delay_ms(20);
    furi_hal_hid_kb_release(key);
    furi_delay_ms(10);
}

static void hid_tap_consumer(uint16_t key) {
    if(g_transport != TransportUsb) return;  /* block when no USB */
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
        else if(c == ':')             { key = HID_KEYBOARD_SEMICOLON;   need_shift = true; }
        else if(c == '/')             { key = HID_KEYBOARD_SLASH; }
        else if(c == '-')             { key = HID_KEYBOARD_MINUS; }
        else if(c == '_')             { key = HID_KEYBOARD_MINUS;        need_shift = true; }
        else if(c == '!')             { key = HID_KEYBOARD_1;            need_shift = true; }
        else if(c == '#')             { key = HID_KEYBOARD_3;            need_shift = true; }
        else if(c == '$')             { key = HID_KEYBOARD_4;            need_shift = true; }
        else if(c == '%')             { key = HID_KEYBOARD_5;            need_shift = true; }
        else if(c == '^')             { key = HID_KEYBOARD_6;            need_shift = true; }
        else if(c == '&')             { key = HID_KEYBOARD_7;            need_shift = true; }
        else if(c == '*')             { key = HID_KEYBOARD_8;            need_shift = true; }
        else if(c == '(')             { key = HID_KEYBOARD_9;            need_shift = true; }
        else if(c == ')')             { key = HID_KEYBOARD_0;            need_shift = true; }
        else if(c == '=')             { key = HID_KEYBOARD_EQUAL_SIGN; }
        else if(c == '?')             { key = HID_KEYBOARD_SLASH;        need_shift = true; }
        else if(c == ';')             { key = HID_KEYBOARD_SEMICOLON; }
        else if(c == '\'')            { key = HID_KEYBOARD_APOSTROPHE; }
        else if(c == '"')             { key = HID_KEYBOARD_APOSTROPHE;  need_shift = true; }
        else if(c == '\t')            { key = HID_KEYBOARD_TAB; }

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
   Macro Editor — printable character set (space..~ plus newline)
   ────────────────────────────────────────────────────────────────── */
static const char EDITOR_CHARS[] =
    " !\"#$%&'()*+,-./:;<=>?@"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~\n";
#define EDITOR_CHARS_LEN ((uint8_t)(sizeof(EDITOR_CHARS) - 1))

/* ──────────────────────────────────────────────────────────────────
   Draw Callback
   ────────────────────────────────────────────────────────────────── */
static void draw_callback(Canvas* canvas, void* ctx) {
    const AppState* st = (const AppState*)ctx;
    canvas_clear(canvas);

    if(st->screen == AppScreenMenu) {
        /* ── MAIN MENU ── */
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 50, 2, AlignCenter, AlignTop, "FlipDeck");
        /* Profile badge top-right */
        char prof_lbl[6];
        snprintf(prof_lbl, sizeof(prof_lbl), "P%u", (unsigned)(st->profile_idx + 1));
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_rbox(canvas, 105, 1, 20, 10, 2);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str_aligned(canvas, 115, 6, AlignCenter, AlignCenter, prof_lbl);
        canvas_set_color(canvas, ColorBlack);
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
        canvas_draw_str_aligned(canvas, 20, 63, AlignCenter, AlignBottom, "OK:Open");
        canvas_draw_str_aligned(canvas, 75, 63, AlignCenter, AlignBottom, "LongOK:Help");
        canvas_draw_str_aligned(canvas, 126, 63, AlignRight,  AlignBottom, "LU/LD:Prof");

    } else if(st->screen == AppScreenPage) {
        /* ── PAGE VIEW ── */
        const Page* p = &PAGES[st->page_idx];

        char title[22];
        snprintf(title, sizeof(title), "[%d/%d] %s",
                 st->page_idx + 1, PAGE_COUNT, p->name);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, title);
        /* Transport badge top-right: filled=USB connected, outline=disconnected */
        canvas_set_font(canvas, FontSecondary);
        if(g_transport == TransportUsb) {
            canvas_draw_rbox(canvas, 104, 1, 22, 9, 2);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str_aligned(canvas, 115, 6, AlignCenter, AlignCenter, "USB");
            canvas_set_color(canvas, ColorBlack);
        } else {
            canvas_draw_rframe(canvas, 104, 1, 22, 9, 2);
            canvas_draw_str_aligned(canvas, 115, 6, AlignCenter, AlignCenter, "- -");
        }
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

        /* Last action flash — shown for 1.5 s in a pill at top-right */
        if(st->last_label && (furi_get_tick() - st->last_tick) < 1500) {
            canvas_draw_rbox(canvas, 72, 2, 54, 9, 2);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str_aligned(canvas, 99, 6, AlignCenter, AlignCenter, st->last_label);
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_draw_line(canvas, 0, 52, 127, 52);
        canvas_set_font(canvas, FontSecondary);
        /* Split footer so it never overflows the 128 px width */
        canvas_draw_str_aligned(canvas,   2, 63, AlignLeft,  AlignBottom, "Back:Menu");
        canvas_draw_str_aligned(canvas, 126, 63, AlignRight, AlignBottom, "LongL/R:Pg");

    } else if(st->screen == AppScreenMacroEdit) {
        /* ── MACRO EDITOR ── */
        static const char* const SLOT[5] = {"Up","Down","Left","Rght","OK"};
        canvas_set_font(canvas, FontPrimary);
        char etitle[22];
        snprintf(etitle, sizeof(etitle), "Edit P%u: %s",
                 st->profile_idx + 1, SLOT[st->edit_macro_idx]);
        canvas_draw_str(canvas, 2, 10, etitle);
        canvas_draw_line(canvas, 0, 12, 127, 12);
        canvas_set_font(canvas, FontSecondary);

        /* Buffer — show last 20 chars + cursor marker */
        uint8_t blen = (uint8_t)strlen(st->edit_buf);
        char dbuf[23];
        uint8_t si = (blen > 20) ? (uint8_t)(blen - 20) : 0;
        uint8_t di = 0;
        for(uint8_t bi = si; bi < blen && di < 20; bi++, di++)
            dbuf[di] = (st->edit_buf[bi] == '\n') ? '\x7E' : st->edit_buf[bi];
        dbuf[di++] = '_'; dbuf[di] = '\0';
        canvas_draw_str(canvas, 2, 24, dbuf);
        canvas_draw_line(canvas, 2, 26, (int)(di * 6), 26);

        /* Char selector */
        char cc = EDITOR_CHARS[st->edit_char_idx];
        char sel[20];
        if(cc == '\n')
            snprintf(sel, sizeof(sel), "[NL] %u/%u",
                     st->edit_char_idx + 1, EDITOR_CHARS_LEN);
        else
            snprintf(sel, sizeof(sel), "[ %c ] %u/%u",
                     cc == ' ' ? '_' : cc,
                     st->edit_char_idx + 1, EDITOR_CHARS_LEN);
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter, sel);

        canvas_draw_line(canvas, 0, 52, 127, 52);
        canvas_draw_str_aligned(canvas,   2, 63, AlignLeft,  AlignBottom, "U/D:Chr R:Add");
        canvas_draw_str_aligned(canvas, 126, 63, AlignRight, AlignBottom, "L:Del OK:Save");

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

    /* Global USB-disconnected overlay — drawn last so it covers all screens */
    if(g_transport == TransportWaitUsb) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_rbox(canvas, 8, 16, 112, 30, 4);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str_aligned(canvas, 64, 27, AlignCenter, AlignCenter,
                                "USB Disconnected");
        canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignCenter,
                                "Plug in USB-C cable");
        canvas_set_color(canvas, ColorBlack);
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

    AppState* st = malloc(sizeof(AppState));
    furi_assert(st);
    st->screen         = AppScreenMenu;
    st->page_idx       = 0;
    st->help_screen    = 0;
    st->last_label     = NULL;
    st->last_tick      = 0;
    st->profile_idx    = 0;
    st->edit_macro_idx = 0;
    st->edit_char_idx  = 0;
    st->edit_buf[0]    = '\0';

    /* Load last page + profile, then load macros for that profile */
    state_load(st);
    macros_load_sd(st->profile_idx);

    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);

    /* Auto-select transport: USB when VBUS present, else wait */
    transport_check(notif);
    if(g_transport == TransportNone)
        g_transport = TransportWaitUsb;  /* show overlay until USB plugged */
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, draw_callback, st);
    view_port_input_callback_set(vp, input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;
    uint8_t ttick = 0;  /* transport check tick counter (100 ms each) */

    while(running) {
        if(furi_message_queue_get(queue, &event, 100) != FuriStatusOk) {
            /* Check transport every ~2 s (20 x 100 ms) */
            if(++ttick >= 20) {
                ttick = 0;
                transport_check(notif);
            }
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
            } else if(event.type == InputTypeLong && event.key == InputKeyUp) {
                /* Previous profile */
                st->profile_idx = (st->profile_idx == 0)
                                   ? (uint8_t)(PROFILE_COUNT - 1)
                                   : (uint8_t)(st->profile_idx - 1);
                macros_load_sd(st->profile_idx);
            } else if(event.type == InputTypeLong && event.key == InputKeyDown) {
                /* Next profile */
                st->profile_idx = (uint8_t)((st->profile_idx + 1) % PROFILE_COUNT);
                macros_load_sd(st->profile_idx);
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
                } else if(st->page_idx == MACRO_PAGE_IDX &&
                          (event.key == InputKeyUp || event.key == InputKeyDown)) {
                    /* Open editor for Up(0) or Down(1) macro slot */
                    uint8_t slot = (event.key == InputKeyUp) ? 0 : 1;
                    st->edit_macro_idx = slot;
                    st->edit_char_idx  = 0;
                    strncpy(st->edit_buf, macro_buf[slot], MACRO_LEN - 1);
                    st->edit_buf[MACRO_LEN - 1] = '\0';
                    st->screen = AppScreenMacroEdit;
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
                    st->last_label = action->label;
                    st->last_tick  = furi_get_tick();
                    notification_message(notif, &sequence_single_vibro);
                }
            } else if(event.type == InputTypeRepeat) {
                /* Hold-to-repeat: Up and Down repeat continuously */
                const Action* rep = NULL;
                if(event.key == InputKeyUp)   rep = &pg->up;
                if(event.key == InputKeyDown) rep = &pg->down;
                if(rep && rep->type != ActionNone) {
                    execute_action(rep);
                    st->last_label = rep->label;
                    st->last_tick  = furi_get_tick();
                    /* No vibro on repeat to avoid continuous buzz */
                }
            } else {
                needs_update = false;
            }

        } else if(st->screen == AppScreenMacroEdit) {
            /* ── EDITOR INPUT ── */
            uint8_t blen = (uint8_t)strlen(st->edit_buf);
            if(event.type == InputTypeShort || event.type == InputTypeRepeat) {
                switch(event.key) {
                case InputKeyUp:
                    st->edit_char_idx =
                        (uint8_t)((st->edit_char_idx + 1) % EDITOR_CHARS_LEN);
                    break;
                case InputKeyDown:
                    st->edit_char_idx = (st->edit_char_idx == 0)
                        ? (uint8_t)(EDITOR_CHARS_LEN - 1)
                        : (uint8_t)(st->edit_char_idx - 1);
                    break;
                case InputKeyRight:
                    if(event.type == InputTypeShort && blen < MACRO_LEN - 1) {
                        st->edit_buf[blen]     = EDITOR_CHARS[st->edit_char_idx];
                        st->edit_buf[blen + 1] = '\0';
                    }
                    break;
                case InputKeyLeft:
                    if(event.type == InputTypeShort && blen > 0)
                        st->edit_buf[blen - 1] = '\0';
                    break;
                case InputKeyOk:
                    if(event.type == InputTypeShort) {
                        strncpy(macro_buf[st->edit_macro_idx],
                                st->edit_buf, MACRO_LEN - 1);
                        macro_buf[st->edit_macro_idx][MACRO_LEN - 1] = '\0';
                        PAGES[MACRO_PAGE_IDX].up.text    = macro_buf[0];
                        PAGES[MACRO_PAGE_IDX].down.text  = macro_buf[1];
                        PAGES[MACRO_PAGE_IDX].left.text  = macro_buf[2];
                        PAGES[MACRO_PAGE_IDX].right.text = macro_buf[3];
                        PAGES[MACRO_PAGE_IDX].ok.text    = macro_buf[4];
                        macro_save_sd(st->profile_idx);
                        notification_message(notif, &sequence_single_vibro);
                        st->screen = AppScreenPage;
                    }
                    break;
                case InputKeyBack:
                    if(event.type == InputTypeShort)
                        st->screen = AppScreenPage;
                    break;
                default:
                    needs_update = false;
                    break;
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

    /* Save last page to SD before exit */
    state_save(st);

    /* Cleanup */
    view_port_enabled_set(vp, false);
    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    free(st);

    /* Tear down transport */
    if(g_transport != TransportNone && g_usb_prev) {
        furi_hal_usb_unlock();
        furi_hal_usb_set_config(g_usb_prev, NULL);
    }
    g_transport = TransportNone;

    return 0;
}

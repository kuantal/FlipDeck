/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Page Definitions & Help Screens Implementation
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_pages.h"
#include "flip_deck_storage.h"

/* Page definitions (9 pages x 5 actions = 45 shortcuts) */
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

    /* 4 -- OBS */
    { "OBS",
      {ActionKey, HID_CTRL_SHIFT_M,   NULL, "Mute"   },
      {ActionKey, HID_CTRL_SHIFT_V,   NULL, "Camera" },
      {ActionKey, HID_CTRL_LEFT_KEY,  NULL, "<Scene" },
      {ActionKey, HID_CTRL_RIGHT_KEY, NULL, "Scene>" },
      {ActionKey, HID_CTRL_SHIFT_R,   NULL, "Record" },
    },

    /* 5 -- MACRO */
    { "MACRO",
      {ActionTypeText, 0, NULL, NULL},
      {ActionTypeText, 0, NULL, NULL},
      {ActionTypeText, 0, NULL, NULL},
      {ActionTypeText, 0, NULL, NULL},
      {ActionTypeText, 0, NULL, NULL},
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
      {ActionKey, HID_CTRL_PLUS,      NULL, "Zoom+"  },
      {ActionKey, HID_CTRL_MINUS_KEY, NULL, "Zoom-"  },
      {ActionKey, HID_CTRL_Z,         NULL, "Undo"   },
      {ActionKey, HID_CTRL_ALT_Z,     NULL, "History"},
      {ActionKey, HID_CTRL_SHIFT_S,   NULL, "Save As"},
    },

    /* 8 -- WINDOWS WM */
    { "WIN WM",
      {ActionKey, HID_WIN_UP,         NULL, "Maximze" },
      {ActionKey, HID_WIN_DOWN,       NULL, "Minimize"},
      {ActionKey, HID_WIN_LEFT_KEY,   NULL, "<Snap"   },
      {ActionKey, HID_WIN_RIGHT_KEY,  NULL, "Snap>"   },
      {ActionKey, HID_CTRL_WIN_RIGHT, NULL, "NxDesk"  },
    },
};

/* Help screen content */
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
    /* 3 - Pages 7-9 */
    {
        "=PAGES 7-9=",
        "7:Gaming",
        "8:Photoshop",
        "9:Win WM",
    },
    /* 4 - Macro editor */
    {
        "=MACRO EDITOR=",
        "LongUp/Dn: edit slot",
        "Flipper keyboard",
        "label|text format",
    },
    /* 5 - Tips */
    {
        "=TIPS=",
        "MenuLU/D:switch prof",
        "Long OK: open help",
        "Back in menu: exit",
    },
};

Page* pages_get_all(void) {
    return PAGES;
}

uint8_t pages_get_count(void) {
    return (uint8_t)(sizeof(PAGES) / sizeof(PAGES[0]));
}

const Page* pages_get(uint8_t idx) {
    if(idx >= pages_get_count()) return NULL;
    return &PAGES[idx];
}

void pages_update_macro_pointers(void) {
    PAGES[MACRO_PAGE_IDX].up.text    = macro_buf[0];
    PAGES[MACRO_PAGE_IDX].down.text  = macro_buf[1];
    PAGES[MACRO_PAGE_IDX].left.text  = macro_buf[2];
    PAGES[MACRO_PAGE_IDX].right.text = macro_buf[3];
    PAGES[MACRO_PAGE_IDX].ok.text    = macro_buf[4];
    PAGES[MACRO_PAGE_IDX].up.label    = macro_label_buf[0];
    PAGES[MACRO_PAGE_IDX].down.label  = macro_label_buf[1];
    PAGES[MACRO_PAGE_IDX].left.label  = macro_label_buf[2];
    PAGES[MACRO_PAGE_IDX].right.label = macro_label_buf[3];
    PAGES[MACRO_PAGE_IDX].ok.label    = macro_label_buf[4];
}

const char* help_get_line(uint8_t screen, uint8_t line) {
    if(screen >= HELP_SCREENS || line >= HELP_LINES) return "";
    return HELP[screen][line];
}

/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — HID Layer Implementation
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_hid.h"
#include "flip_deck_transport.h"
#include <furi_hal_usb_hid.h>

void hid_tap_key(uint16_t key) {
    if(!transport_is_connected()) return;  /* block when no USB */
    furi_hal_hid_kb_press(key);
    furi_delay_ms(20);
    furi_hal_hid_kb_release(key);
    furi_delay_ms(10);
}

void hid_tap_consumer(uint16_t key) {
    if(!transport_is_connected()) return;  /* block when no USB */
    furi_hal_hid_consumer_key_press(key);
    furi_delay_ms(20);
    furi_hal_hid_consumer_key_release(key);
    furi_delay_ms(10);
}

void hid_type_text(const char* text) {
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

void execute_action(const Action* action) {
    if(!action) return;
    switch(action->type) {
    case ActionConsumer:  hid_tap_consumer(action->hid_code);           break;
    case ActionKey:       hid_tap_key(action->hid_code);                break;
    case ActionTypeText:  if(action->text) hid_type_text(action->text); break;
    default: break;
    }
}

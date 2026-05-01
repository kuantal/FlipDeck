/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — HID Layer (USB HID wrappers)
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include "flip_deck_types.h"

/* HID wrappers - check transport before sending */
void hid_tap_key(uint16_t key);
void hid_tap_consumer(uint16_t key);
void hid_type_text(const char* text);
void execute_action(const Action* action);

/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Page Definitions & Help Screens
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include "flip_deck_types.h"

/* Page data access */
Page* pages_get_all(void);
uint8_t pages_get_count(void);
const Page* pages_get(uint8_t idx);
void pages_update_macro_pointers(void);

/* Help screen data */
const char* help_get_line(uint8_t screen, uint8_t line);

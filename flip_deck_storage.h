/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — SD Card Storage Layer
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include "flip_deck_types.h"

/* Storage paths */
#define DATA_DIR    "/ext/apps_data/flip_deck"
#define STATE_FILE  "/ext/apps_data/flip_deck/state.bin"

/* Macro buffers - accessed by pages module */
extern char macro_buf[MACRO_COUNT][MACRO_LEN];
extern char macro_label_buf[MACRO_COUNT][LABEL_LEN];

/* Storage functions */
void macros_load_sd(uint8_t pidx);
void macro_save_sd(uint8_t pidx);
void state_save(const AppState* st);
void state_load(AppState* st);

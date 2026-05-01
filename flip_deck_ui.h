/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — UI Layer (Draw + Input + Text Editor)
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include "flip_deck_types.h"
#include <notification/notification_messages.h>

/* UI callbacks */
void ui_draw_callback(Canvas* canvas, void* ctx);
void ui_input_callback(InputEvent* event, void* ctx);

/* Text editor modal */
void ui_open_text_editor(AppState* st, uint8_t slot, NotificationApp* notif);

/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — UI Layer (Drawing + Text Editor)
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_ui.h"
#include "flip_deck_types.h"
#include "flip_deck_transport.h"
#include "flip_deck_pages.h"
#include "flip_deck_storage.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/text_input.h>
#include <gui/view_dispatcher.h>
#include <notification/notification_messages.h>

#define TEXT_INPUT_VIEW_ID 0

/* Text input callback prototypes */
static void text_input_callback(void* ctx);
static uint32_t text_input_back_callback(void* ctx);

/* ──────────────────────────────────────────────────────────────────
   Draw Callback
   ────────────────────────────────────────────────────────────────── */
void ui_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    AppState* st = (AppState*)ctx;

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
        if(start > (int8_t)(pages_get_count() - 3)) start = (int8_t)(pages_get_count() - 3);
        if(start < 0) start = 0;

        for(uint8_t i = 0; i < 3 && (start + (int8_t)i) < (int8_t)pages_get_count(); i++) {
            uint8_t idx = (uint8_t)(start + (int8_t)i);
            int y = 15 + (int)(i * 12);
            bool selected = (idx == st->page_idx);

            if(selected) {
                canvas_draw_rbox(canvas, 0, y, 127, 11, 2);
                canvas_set_color(canvas, ColorWhite);
            }
            canvas_set_font(canvas, FontSecondary);
            char row[22];
            const Page* pg = pages_get(idx);
            snprintf(row, sizeof(row), "%d. %s", idx + 1, pg->name);
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
        const Page* p = pages_get(st->page_idx);

        char title[22];
        snprintf(title, sizeof(title), "[%d/%d] %s",
                 st->page_idx + 1, pages_get_count(), p->name);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 10, title);
        /* Transport badge top-right: filled=USB connected, outline=disconnected */
        canvas_set_font(canvas, FontSecondary);
        if(transport_get_mode() == TransportUsb) {
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
        /* OK — center box */
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
        /* Macro editor uses Flipper's built-in text input (shown separately) */
        /* This screen is not rendered as text_input widget takes over display */

    } else {
        /* ── HELP SCREEN ── */
        uint8_t hs = st->help_screen;

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "HELP");
        canvas_draw_line(canvas, 0, 13, 127, 13);

        canvas_set_font(canvas, FontSecondary);
        for(uint8_t i = 0; i < HELP_LINES; i++) {
            canvas_draw_str(canvas, 2, 16 + i * 9, help_get_line(hs, i));
        }

        canvas_draw_line(canvas, 0, 52, 127, 52);
        char pg[24];
        snprintf(pg, sizeof(pg), "< %d/%d > Back:Close", hs + 1, HELP_SCREENS);
        canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, pg);
    }

    /* Global USB-disconnected overlay — drawn last so it covers all screens */
    if(transport_get_mode() == TransportWaitUsb) {
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
   Text Input Callbacks (for macro editor)
   ────────────────────────────────────────────────────────────────── */
static void text_input_callback(void* ctx) {
    AppState* st = (AppState*)ctx;
    
    /* Parse format: label|text (if no pipe, entire input is text) */
    char* pipe = strchr(st->edit_buf, '|');
    if(pipe != NULL) {
        /* Split label and text */
        *pipe = '\0';  /* terminate label */
        strncpy(macro_label_buf[st->edit_macro_idx], st->edit_buf, LABEL_LEN - 1);
        macro_label_buf[st->edit_macro_idx][LABEL_LEN - 1] = '\0';
        strncpy(macro_buf[st->edit_macro_idx], pipe + 1, MACRO_LEN - 1);
        macro_buf[st->edit_macro_idx][MACRO_LEN - 1] = '\0';
    } else {
        /* No pipe - entire input is text, keep existing label */
        strncpy(macro_buf[st->edit_macro_idx], st->edit_buf, MACRO_LEN - 1);
        macro_buf[st->edit_macro_idx][MACRO_LEN - 1] = '\0';
    }
    
    /* Update MACRO page pointers */
    pages_update_macro_pointers();
    
    /* Save to SD card */
    macro_save_sd(st->profile_idx);
    
    /* Stop view dispatcher to return to main loop */
    view_dispatcher_stop(st->view_dispatcher);
    st->text_input_active = false;
    st->screen = AppScreenPage;
}

static uint32_t text_input_back_callback(void* ctx) {
    AppState* st = (AppState*)ctx;
    /* Cancel without saving */
    view_dispatcher_stop(st->view_dispatcher);
    st->text_input_active = false;
    st->screen = AppScreenPage;
    return VIEW_IGNORE;  /* or VIEW_NONE to consume event */
}

/* ──────────────────────────────────────────────────────────────────
   Text Editor Launcher
   ────────────────────────────────────────────────────────────────── */
void ui_open_text_editor(AppState* st, uint8_t macro_slot, NotificationApp* notif) {
    st->edit_macro_idx = macro_slot;
    st->screen = AppScreenMacroEdit;

    /* Create text input widget if needed */
    if(!st->text_input) {
        st->text_input = text_input_alloc();
    }

    /* Create view dispatcher if needed */
    if(!st->view_dispatcher) {
        st->view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_add_view(st->view_dispatcher, TEXT_INPUT_VIEW_ID,
                                 text_input_get_view(st->text_input));
    }

    /* Build initial content: label|text */
    snprintf(st->edit_buf, sizeof(st->edit_buf), "%s|%s",
             macro_label_buf[macro_slot], macro_buf[macro_slot]);

    /* Configure text input */
    text_input_reset(st->text_input);
    text_input_set_header_text(st->text_input, "Edit: label|text");
    text_input_set_result_callback(st->text_input, text_input_callback, st,
                                    st->edit_buf, sizeof(st->edit_buf), true);

    /* Set navigation callback for Back button */
    view_set_previous_callback(text_input_get_view(st->text_input),
                                text_input_back_callback);

    /* Remove main viewport temporarily */
    gui_remove_view_port(st->gui, st->viewport);

    /* Attach view dispatcher */
    view_dispatcher_attach_to_gui(st->view_dispatcher, st->gui,
                                   ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(st->view_dispatcher, TEXT_INPUT_VIEW_ID);

    /* Vibrate feedback */
    notification_message(notif, &sequence_single_vibro);

    /* Run blocking event loop (returns when user confirms or cancels) */
    st->text_input_active = true;
    view_dispatcher_run(st->view_dispatcher);

    /* Cleanup after text input closes */
    view_dispatcher_remove_view(st->view_dispatcher, TEXT_INPUT_VIEW_ID);
    view_dispatcher_free(st->view_dispatcher);
    st->view_dispatcher = NULL;
    text_input_free(st->text_input);
    st->text_input = NULL;

    /* Restore main viewport */
    gui_add_view_port(st->gui, st->viewport, GuiLayerFullscreen);
    view_port_update(st->viewport);
}

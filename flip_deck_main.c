/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — Main Entry Point
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_types.h"
#include "flip_deck_hid.h"
#include "flip_deck_transport.h"
#include "flip_deck_storage.h"
#include "flip_deck_pages.h"
#include "flip_deck_ui.h"
#include <notification/notification_messages.h>
#include <input/input.h>

static void input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = (FuriMessageQueue*)ctx;
    furi_message_queue_put(queue, event, FuriWaitForever);
}

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
    st->edit_buf[0]    = '\0';
    st->text_input_active = false;
    st->text_input = NULL;
    st->view_dispatcher = NULL;

    /* Load last page + profile, then load macros for that profile */
    state_load(st);
    macros_load_sd(st->profile_idx);

    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);

    /* Initialize transport */
    transport_init();
    transport_check(notif);
    if(transport_get_mode() == TransportNone) {
        /* Show overlay until USB plugged */
    }
    
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* vp = view_port_alloc();
    st->viewport = vp;
    view_port_draw_callback_set(vp, ui_draw_callback, st);
    view_port_input_callback_set(vp, input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    st->gui = gui;
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;
    uint8_t ttick = 0;  /* transport check tick counter */

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
                    if(st->page_idx < pages_get_count() - 1) st->page_idx++;
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
            const Page* pg = pages_get(st->page_idx);
            const Action* action = NULL;

            if(event.type == InputTypeLong) {
                if(event.key == InputKeyLeft) {
                    if(st->page_idx > 0) st->page_idx--;
                } else if(event.key == InputKeyRight) {
                    if(st->page_idx < pages_get_count() - 1) st->page_idx++;
                } else if(event.key == InputKeyOk) {
                    st->screen      = AppScreenHelp;
                    st->help_screen = 0;
                } else if(st->page_idx == MACRO_PAGE_IDX &&
                          (event.key == InputKeyUp || event.key == InputKeyDown)) {
                    /* Open text editor */
                    uint8_t slot = (event.key == InputKeyUp) ? 0 : 1;
                    ui_open_text_editor(st, slot, notif);
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
                /* Hold-to-repeat */
                const Action* rep = NULL;
                if(event.key == InputKeyUp)   rep = &pg->up;
                if(event.key == InputKeyDown) rep = &pg->down;
                if(rep && rep->type != ActionNone) {
                    execute_action(rep);
                    st->last_label = rep->label;
                    st->last_tick  = furi_get_tick();
                }
            } else {
                needs_update = false;
            }

        } else if(st->screen == AppScreenMacroEdit) {
            /* Text input handles its own events */
            needs_update = false;

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

    /* Save state before exit */
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
    transport_cleanup();

    return 0;
}

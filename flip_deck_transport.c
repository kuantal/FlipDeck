/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — USB Transport Implementation
 *  VBUS voltage-based detection (>4.0V = USB connected)
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include "flip_deck_transport.h"
#include <furi_hal_usb.h>
#include <furi_hal_power.h>
#include <furi_hal_usb_hid.h>

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

void transport_init(void) {
    g_transport = TransportNone;
    g_usb_prev = NULL;
}

bool transport_check(NotificationApp* notif) {
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

bool transport_is_connected(void) {
    return g_transport == TransportUsb;
}

TransportMode transport_get_mode(void) {
    return g_transport;
}

void transport_cleanup(void) {
    if(g_transport != TransportNone && g_usb_prev) {
        furi_hal_usb_unlock();
        furi_hal_usb_set_config(g_usb_prev, NULL);
    }
    g_transport = TransportNone;
}

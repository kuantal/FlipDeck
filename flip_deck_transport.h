/*
 * ╔══════════════════════════════════════════════════════════════╗
 *  FlipDeck — USB Transport Layer
 * ╚══════════════════════════════════════════════════════════════╝
 */

#pragma once

#include <notification/notification_messages.h>

/* Transport mode enumeration */
typedef enum {
    TransportNone = 0,
    TransportUsb,
    TransportWaitUsb,  /* USB was once active but cable was removed */
} TransportMode;

/* Transport functions */
void transport_init(void);
bool transport_check(NotificationApp* notif);
bool transport_is_connected(void);
TransportMode transport_get_mode(void);
void transport_cleanup(void);

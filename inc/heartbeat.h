#ifndef _HEARTBEAT_H
#define _HEARTBEAT_H

typedef enum hbeats {
    HB_MI,
    HB_DI,
    HB_UI,
    HB_WVS,
    HB_COUNT
} HBEATS_T

static const char * const hbeat_names[HB_COUNT] = {
    "HB_MI",
    "HB_DI",
    "HB_UI",
    "HB_WVS"
}

typedef enum hb_status {
    bool enabled
    uint32_t time_stamp
}

type


#endif

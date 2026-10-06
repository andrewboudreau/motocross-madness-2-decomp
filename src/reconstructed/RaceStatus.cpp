#include "RaceStatus.h"

#include "DebugAlloc.h"
#include "TrackGame.h"

int g_UnknownGlobal689c6c;

// 0x004e5c70
int UnknownFunction4e5c70(UnknownEventRacerPart* list, UnknownEventRacer* racer) {
    for (; list; list = list->field_0x50) {
        if (list->field_0x04 == racer)
            return 1;
    }
    return 0;
}

// 0x004e5ca0
int UnknownFunction4e5ca0(UnknownEventRacerPart** list) {
    while (list && *list) {
        if (!(*list)->field_0x04->field_0x25_bit0) {
            (*list)->field_0x04->field_0x744 = 0;
            UnknownEventRacerPart* status = *list;
            *list = status->field_0x50;
            operator delete(status, __FILE__, 193);
        } else {
            list = &(*list)->field_0x50;
        }
    }
    return 1;
}

// 0x004e5f10
int UnknownFunction4e5f10(UnknownEventRacerPart* list) {
    while (list) {
        UnknownEventRacerPart* status = list;
        list = list->field_0x50;
        operator delete(status, __FILE__, 285);
    }
    return 0;
}

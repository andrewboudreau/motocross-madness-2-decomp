// Near misses for MSZoneInterface.cpp (canonical file:
// src/reconstructed/MSZoneInterface.cpp, included below for its types).
//
// 0x004aa360 (384 bytes) and 0x004aa4e0 (393 bytes): GETPROPERTY requests
// for the preset / rank lobby property. Everything up to the response
// checks matches (frame, request build, 15 s receive loop). Remaining
// difference: retail lays the failure block (delete at line 118 / 198,
// return 0) directly after the response checks, falling through from them
// (`jbe` to the copy), and puts the memcpy/log block after it; VC6 SP3 sinks
// the failure block to the end of the function for this source and for the
// nested-if, if/else and `||`-combined variants tried (293/384 and 297/393
// bytes; the `||` form also loses the threaded second type test). The
// success free (line 114) precedes the failure free (line 118) in the
// source; `if (result >= 0) { ... return 1; } failed: ...`, a trailing
// `failed:` label, `do { ... break; } while (0)` and `while (1)` forms all
// keep VC6's layout.
// Check: compile this file and compare ?UnknownFunction4aa360@... and
// ?UnknownFunction4aa4e0@... with src/reconstructed/MSZoneInterface.bindings.json.

#include "../../src/reconstructed/MSZoneInterface.cpp"

// 0x004aa360: sends a GETPROPERTY request for the preset and waits up to
// 15 seconds for the answer. Returns 0 on failure; 1 otherwise (also when
// another message arrives first).
int UnknownTrackGameObject3410::UnknownFunction4aa360(char* buffer, unsigned int size) {
    UnknownLobbyGetPropertyResponse* response;
    UnknownLobbyGetProperty request;
    unsigned long flags = 0;
    unsigned long responseSize = 0;
    long result;
    unsigned int start;

    response = (UnknownLobbyGetPropertyResponse*)DebugMalloc(0x1000, __FILE__, 57);
    responseSize = 4;
    request.type = 7;
    request.requestId = 1;
    request.player = g_UnknownGuid556db0;
    request.property = g_UnknownGuid556dd0;
    result = field_0x04->SendLobbyMessage(2, 0, &request, sizeof(request));
    start = UnknownFunction4bfa80();
    while (UnknownFunction4bfa80() - start < 15000) {
        result = field_0x04->ReceiveLobbyMessage(0, 0, &flags, response, &responseSize);
        if (result >= 0)
            break;
        Sleep(100);
    }
    if (result < 0)
        goto failed;
    if (response->type == 8) {
        if (response->result != 0 || response->requestId != 1 ||
            memcmp(&response->property, &g_UnknownGuid556dd0, sizeof(GUID)) != 0 || response->dataSize > size) {
        failed:
            DebugFree(response, __FILE__, 118);
            return 0;
        }
        memcpy(buffer, response->data, response->dataSize);
        SendDebugMessage("\nGot Preset from The Zone\n%s\n", response->data);
    }
    DebugFree(response, __FILE__, 114);
    return 1;
}

// 0x004aa4e0: the same for the rank property.
int UnknownTrackGameObject3410::UnknownFunction4aa4e0(char* buffer, unsigned int size) {
    UnknownLobbyGetPropertyResponse* response;
    UnknownLobbyGetProperty request;
    unsigned long flags = 0;
    unsigned long responseSize = 0;
    long result;
    unsigned int start;

    response = (UnknownLobbyGetPropertyResponse*)DebugMalloc(0x1000, __FILE__, 135);
    responseSize = 4;
    request.type = 7;
    request.requestId = 2;
    request.player = g_UnknownGuid556db0;
    request.property = g_UnknownGuid556de0;
    result = field_0x04->SendLobbyMessage(2, 0, &request, sizeof(request));
    start = UnknownFunction4bfa80();
    while (UnknownFunction4bfa80() - start < 15000) {
        result = field_0x04->ReceiveLobbyMessage(0, 0, &flags, response, &responseSize);
        if (result >= 0)
            break;
        Sleep(100);
    }
    if (result < 0)
        goto failed;
    if (response->type == 8) {
        if (response->result != 0 || response->requestId != 2 ||
            memcmp(&response->property, &g_UnknownGuid556de0, sizeof(GUID)) != 0 || response->dataSize > size) {
        failed:
            DebugFree(response, __FILE__, 198);
            return 0;
        }
        memcpy(buffer, response->data, response->dataSize);
        SendDebugMessage("\nGot Rank# from The Zone\n%s\n", response->data);
    }
    DebugFree(response, __FILE__, 194);
    return 1;
}

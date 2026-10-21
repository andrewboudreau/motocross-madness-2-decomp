// Startup list constructors identified through their actual callers.
// Original TU ownership and the Unknown* class names remain provisional.
#include "Parser.h"
#include "DirectoryList.h"

// 0x004aae20: the parser member at +4; no arguments.
UnknownParserList::UnknownParserList() {
    head = 0;
}

// 0x004676a0: the eight-byte drive-list allocation in TrackGameMode.
UnknownDriveList::UnknownDriveList() {
    drives = 0;
    count = 0;
}

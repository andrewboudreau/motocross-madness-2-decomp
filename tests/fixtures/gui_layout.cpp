#include <stddef.h>
#include "../../src/reconstructed/GUIManager.h"
#include "../../src/reconstructed/KrustyUI.h"
#define CHECK(name, expression) typedef char name[(expression) ? 1 : -1]
CHECK(manager_extent, sizeof(GUIManager) == 0x3e0);
CHECK(manager_users, offsetof(GUIManager, users) == 0x330);
CHECK(manager_keyboard, offsetof(GUIManager, keyboardDevice) == 0x30c);
CHECK(manager_font_offset, offsetof(GUIManager, fontSizeOffset) == 0x3d0);
CHECK(user_extent, sizeof(GUIUser) == 0x1f4);
CHECK(user_cursor, offsetof(GUIUser, userCursor) == 0x30);
CHECK(user_tooltip, offsetof(GUIUser, userToolTip) == 0xc0);
CHECK(tooltip_enabled, offsetof(ToolTip, enabled) == 0x5c);
CHECK(ui_gui, offsetof(KrustyUI, gui) == 0x2c);

#pragma once

enum class WndId {
    MemoryView,
    CopperDbgWnd,
    Disassembly,
    Registers,
    Console,
    Screen,
    Colors,
    MemoryGraph,
    CustomRegsWnd,
    BlitterWnd,

    ImGuiDemo,
    MostCommonCount,
};

namespace UiDrawEvent {
enum Type : uint8_t {
    MainMenu_File,
    MainMenu_Emul,
    MainMenu_Debug,

    MenuItemStateChecked,
    Count,
    Undef = 0xff,
};
};  // namespace UiDrawEvent

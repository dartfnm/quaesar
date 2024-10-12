#pragma once
#include <src/generic/base.h>


FORWARD_DECLARATION_2(qd, Shortcut);

namespace qd::shortcut {

#define SHORTCUT(name, setup_func)

//
// Shortcuts list + Enum class shortcut::EId declaration
// 'qd::shortcut::EId::DebugTraceStart'
//
#define SHORTCUT_LIST(SHORTCUT)                                                                                     \
    SHORTCUT(DebugTraceStepInto, [](Shortcut& self) { self.addKey(ImGuiKey_F11).setRepeat(); })                     \
    SHORTCUT(DebugTraceStepOut, [](Shortcut& self) { self.addKey(ImGuiKey_F10).setRepeat(); })                      \
    SHORTCUT(DebugTraceStart, [](Shortcut& self) { self.addKey(ImGuiKey_F12); })                                    \
    SHORTCUT(DebugTraceContinue, [](Shortcut& self) { self.addKey(ImGuiKey_F5); })                                  \
    SHORTCUT(DebugWaitScanLines, [](Shortcut& self) {})                                                             \
    SHORTCUT(DisasmToggleBreakpoint, [](Shortcut& self) { self.addKey(ImGuiKey_F9); })                              \
    SHORTCUT(CopperToggleBreakpoint,                                                                                \
             [](Shortcut& self) { self.addKey(ImGuiKey_F9).addKey(ImGuiMod_Shift).setRepeat(); })                   \
    SHORTCUT(CopperTraceStep, [](Shortcut& self) { self.addKey(ImGuiKey_F11).addKey(ImGuiMod_Shift).setRepeat(); }) \
    SHORTCUT(ToggleTurboEmulation, [](Shortcut& self) { self.addKey(ImGuiKey_NumLock); })                           \
    SHORTCUT(ResetAmigaEmu, [](Shortcut& self) {})                                                                  \
    SHORTCUT(AlwaysOnTopEmu, [](Shortcut& self) { self.addKey(ImGuiKey_T).addKey(ImGuiMod_Ctrl); })                 \
    /* END OF SHORTCUTS LIST */
//////////////////////////////////////////////////////////////////////////


enum class EId {
    UNDEF = -1,
#undef SHORTCUT
#define SHORTCUT(name, setup_func) name,
    SHORTCUT_LIST(SHORTCUT)
#undef SHORTCUT
        MAX_COUNT
};  // enum


extern qd::Shortcut* makeInstance(qd::shortcut::EId id);

};  // namespace qd::shortcut

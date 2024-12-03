#pragma once
#include <src/generic/base.h>


FORWARD_DECLARATION_2(qd, Shortcut);


namespace qd {
namespace shortcut {

#define SHORTCUT(name, setup_func)

//
// Shortcuts list + Enum class shortcut::EId declaration
// 'qd::shortcut::EId::DebugTraceStart'
//
#define SHORTCUT_LIST(SHORTCUT)                                                                                     \
    SHORTCUT(DebugTraceStepInto, [](Shortcut& s) { s.addKey(ImGuiKey_F11).setRepeat(); })                           \
    SHORTCUT(DebugTraceStepOut, [](Shortcut& s) { s.addKey(ImGuiKey_F10).setRepeat(); })                            \
    SHORTCUT(DebugTraceStart, [](Shortcut& s) { s.addKey(ImGuiKey_F12); })                                          \
    SHORTCUT(DebugTraceContinue, [](Shortcut& s) { s.addKey(ImGuiKey_F5); })                                        \
    SHORTCUT(DebugWaitScanLines, [](Shortcut&) {})                                                                  \
    SHORTCUT(DisasmToggleBreakpoint, [](Shortcut& s) { s.addKey(ImGuiKey_F9); })                                    \
    SHORTCUT(CopperToggleBreakpoint, [](Shortcut& s) { s.addKey(ImGuiKey_F9).addKey(ImGuiMod_Shift).setRepeat(); }) \
    SHORTCUT(CopperTraceStep, [](Shortcut& s) { s.addKey(ImGuiKey_F11).addKey(ImGuiMod_Shift).setRepeat(); })       \
    SHORTCUT(ToggleTurboEmulation, [](Shortcut& s) { s.addKey(ImGuiKey_NumLock); })                                 \
    SHORTCUT(ResetAmigaEmu, [](Shortcut&) {})                                                                       \
    SHORTCUT(AlwaysOnTopEmu, [](Shortcut& s) { s.addKey(ImGuiKey_T).addKey(ImGuiMod_Ctrl); })                       \
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

};  // namespace shortcut
};  // namespace qd

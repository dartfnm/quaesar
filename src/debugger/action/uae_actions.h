#pragma once
// clang-format off
#include <src/sysconfig.h>
#include "sysdeps.h"
#include "options.h"
#include "keyboard.h"
#include "inputdevice.h"
#include "inputrecord.h"
#include "keybuf.h"
#include "custom.h"
#include "xwin.h"
#include "drawing.h"
#include <uae_src/include/savestate.h>
#include <uae_src/include/uae.h>
// clang-format on
#include <SDL.h>
#include <debugger/action/comps.h>
#include <debugger/action_mgr.h>
#include <debugger/debugger.h>
#include <debugger/msg_list.h>
#include <imgui_eastl.h>
#include <quaesar.h>
#include <src/shortcut/shortcut_list.h>
#include <uae_src/include/debug.h>


//////////////////////////////////////////////////////////////////////////
// These are private UAE actions and in a good case they should not be called directly by name,
// instead they are should triggered via messages (dbg->applyActionMsg(m)) or shortcuts.
//

namespace qd {
namespace action {
namespace uae {

//////////////////////////////////////////////////////////////////////////
static constexpr int ACTIONS_AUTO_ID_START = (__COUNTER__ - 1);
#define AUTO_ID (__COUNTER__ - ACTIONS_AUTO_ID_START)

#define REG(ClassName) \
    ;                  \
    QDB_ACTION_REGISTER(ClassName, AUTO_ID)
//////////////////////////////////////////////////////////////////////////


struct DebugDmaOption : public Action {
    void setup() {
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
    }

    virtual void onDrawMainMenuItem(UiDrawEvent::Type event, void*) override;

} REG(DebugDmaOption);
//////////////////////////////////////////////////////////////////////////


struct DisasmTraceStep : public Action {
    void setup() {
        mName = "Step Into";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DebugTraceStepInto);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID: {
                getDbg()->setDebugMode(DebuggerMode_Break);
                getDbg()->execConsoleCmd("t");
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DisasmTraceStep);
//////////////////////////////////////////////////////////////////////////


struct DisasmTraceStepOut : public Action {
    void setup() {
        mName = "Step Out";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DebugTraceStepOut);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID: {
                getDbg()->execConsoleCmd("z");
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DisasmTraceStepOut);
//////////////////////////////////////////////////////////////////////////


struct DebugTraceStart : public Action {
    void setup() {
        mName = "Debug Trace Mode";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DebugTraceStart);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID: {
                getDbg()->setDebugMode(DebuggerMode_Break);
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DebugTraceStart);
//////////////////////////////////////////////////////////////////////////


struct DebugTraceContinue : public Action {
    void setup() {
        mName = "Continue";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DebugTraceContinue);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID:
            case action::msg::DoDebugTraceContinue::ID: {
                getDbg()->execConsoleCmd("g");
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DebugTraceContinue);
//////////////////////////////////////////////////////////////////////////


struct DebugWaitScanLines : public Action {
    void setup() {
        mName = "Wait N scanlines";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DebugWaitScanLines);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID:
            case action::msg::DoDebugTraceContinue::ID: {
                eastl::string cmd;
                cmd.append_sprintf("fs %i", getDbg()->getWaitScanLines());
                getDbg()->execConsoleCmd(eastl::move(cmd));
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DebugWaitScanLines);
//////////////////////////////////////////////////////////////////////////


struct DisasmToggleBreakpoint : public Action {
    void setup() {
        mName = "Disasm breakpoint";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::DisasmToggleBreakpoint);
    }

    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DisasmToggleBreakpoint::ID: {
                auto p = msg->cast<action::msg::DisasmToggleBreakpoint>();
                eastl::string cmd;
                cmd.sprintf("f %08x", (uint32_t)p->address);
                if (p->nBreakpoint >= 0)
                    cmd.append_sprintf(" %i", p->nBreakpoint);
                getDbg()->execConsoleCmd(eastl::move(cmd));
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(DisasmToggleBreakpoint);
//////////////////////////////////////////////////////////////////////////


struct CopperTraceStep : public Action {
    void setup() {
        mName = "Copper Trace Step";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::CopperTraceStep);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::CopperTraceStep::ID:
            case action::msg::DoAction::ID: {
                getDbg()->execConsoleCmd("ot");
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(CopperTraceStep);
//////////////////////////////////////////////////////////////////////////


struct CopperToggleBreakpoint : public Action {
    void setup() {
        mName = "Copper breakpoint";
        supportMtd.set(UiDrawEvent::MainMenu_Debug);
        addShortcut(qd::shortcut::EId::CopperToggleBreakpoint);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::CopperToggleBreakpoint::ID: {
                auto p = msg->cast<action::msg::CopperToggleBreakpoint>();
                eastl::string cmd;
                cmd.sprintf("ob %08x", (uint32_t)p->address);
                getDbg()->execConsoleCmd(eastl::move(cmd));
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(CopperToggleBreakpoint);
//////////////////////////////////////////////////////////////////////////


struct ToggleTurboEmulation : public Action {
    void setup() {
        mName = "Turbo Emulation";
        supportMtd.set(UiDrawEvent::MainMenu_Emul);
        addShortcut(qd::shortcut::EId::ToggleTurboEmulation);
    }
    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case action::msg::DoAction::ID: {
                if (currprefs.turbo_emulation != 0) {
                    // off
                    warpmode(0);
                } else {
                    // on
                    warpmode(2);
                }
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(ToggleTurboEmulation);
//////////////////////////////////////////////////////////////////////////


struct UaeResetAmiga : public Action {
    void setup() {
        mName = "Reset Amiga";
        supportMtd.set(UiDrawEvent::MainMenu_Emul);
        addShortcut(qd::shortcut::EId::ResetAmigaEmu);
    }

    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case msg::DoAction::ID: {
                ::uae_reset(1, 1);
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(UaeResetAmiga);
//////////////////////////////////////////////////////////////////////////


struct UaeWndAlwaysOnTop : public Action {
    void setup() {
        mName = "Always on Top";
        supportMtd.set(UiDrawEvent::MainMenu_Emul).set(UiDrawEvent::MenuItemStateChecked);
        addShortcut(qd::shortcut::EId::AlwaysOnTopEmu);
    }

    virtual EFlow applyMsgProc(action::msg::Base* msg) override {
        switch (msg->mId) {
            case msg::DoAction::ID: {
                Uint32 flags = SDL_GetWindowFlags(app->mUaeWindow);
                bool setOnTop = (flags & SDL_WINDOW_ALWAYS_ON_TOP) != 0;
                SDL_SetWindowAlwaysOnTop(app->mUaeWindow, (SDL_bool)(!setOnTop));
                return EFlow::SUCCESS;
            } break;
            case msg::MenuItemStateGet::ID: {
                auto p = static_cast<msg::MenuItemStateGet*>(msg);
                Uint32 flags = SDL_GetWindowFlags(app->mUaeWindow);
                p->checked = (flags & SDL_WINDOW_ALWAYS_ON_TOP) ? 1 : 0;
                return EFlow::SUCCESS;
            } break;
            default:
                return Action::applyMsgProc(msg);
        }
    }
} REG(UaeWndAlwaysOnTop);
//////////////////////////////////////////////////////////////////////////


#undef REG
#undef AUTO_ID
#undef ACTIONS_AUTO_ID_START
};  // namespace uae
};  // namespace action
};  // namespace qd

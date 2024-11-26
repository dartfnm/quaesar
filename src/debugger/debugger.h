#pragma once

#include <EASTL/fixed_set.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/string.h>
#include <debugger/vm/memory.h>
#include <debugger/vm/vm.h>
#include <src/generic/base.h>

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;
typedef size_t csh;

FORWARD_DECLARATION_4S(qd, action, msg, Base);

//////////////////////////////////////////////////////////////////////////
namespace qd {
class GuiManager;
class VM;
namespace action {
class ActionManager;
};  // namespace action

constexpr static int BREAKPOINTS_MAX = 20;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum DebuggerMode {
    DebuggerMode_Live,
    DebuggerMode_Break,
};
//////////////////////////////////////////////////////////////////////////

class Breakpoint {
public:
    AddrRef addr1 = {};
    AddrRef addr2 = {};
    bool enabled = false;
    EReg reg;
};  // class Breakpoint
//////////////////////////////////////////////////////////////////////////


class BreakpointsSortedList {
    eastl::fixed_vector<Breakpoint, qd::BREAKPOINTS_MAX, false> breakpoints;

    struct OneAddrBp {
        AddrRef addr;
        int bpIdx;

        bool operator<(const OneAddrBp& rh) const {
            return addr < rh.addr;
        }
    };
    eastl::fixed_set<OneAddrBp, qd::BREAKPOINTS_MAX, false> oneAddrBps;

public:
    void create();
    const qd::Breakpoint* getBpByAddr(AddrRef addr, EReg reg) const;
};  // BreakpointsSortedList
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
class Debugger {
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;

public:
    csh* capstone = nullptr;
    VM* vm = nullptr;
    GuiManager* gui = nullptr;
    action::ActionManager* actions = nullptr;

    SDL_Renderer* getRenderer() const {
        return mRenderer;
    }

private:
    int mWaitScanLines = 1;

public:
    void init();
    void destroy();
    void update();
    void render();
    bool isVisible() const;
    void toggleWndVisible(DebuggerMode mode);
    void sdlEventProc(SDL_Event* event);

    qd::VM* getVm() const {
        return vm;
    }

    action::ActionManager* getActions() const {
        return actions;
    }

    static bool isDebugActivated();
    static bool isDebugActivatedFull();
    void setDebugMode(DebuggerMode debug_mode);

    EFlow applyActionMsg(qd::action::msg::Base* p_msg) const;

    void execConsoleCmd(eastl::string&& cmd);

    void applyImmediateConsoleCmd(eastl::string&& cmd);

    int waitConsoleCmd(char* out, int maxlen);

    int getWaitScanLines() const {
        return mWaitScanLines;
    }
    void setWaitScanLines(int waitScanLines) {
        mWaitScanLines = waitScanLines;
    }

    static Debugger* get() {
        static Debugger instance;
        return &instance;
    }

    BreakpointsSortedList getBreakpointsSorted() const {
        BreakpointsSortedList bp;
        bp.create();
        return bp;
    }

private:
    void createRenderWindow();
    void initImGui();
    Debugger() = default;
    ~Debugger() = default;
};  // class Debugger
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


};  // namespace qd

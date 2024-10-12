#pragma once
#include <debugger/action_mgr.h>
#include <debugger/vm/memory.h>
#include <debugger/vm/vm.h>
#include <src/generic/types.h>


namespace qd::action::msg {

static constexpr int AUTO_ID_START = (__COUNTER__ - 1);
#define AUTO_ID (__COUNTER__ - AUTO_ID_START)


template <int TClassId>
struct Base_ : public Base {
    static constexpr int ID = TClassId;
    Base_() : Base(TClassId) {
    }
};  // struct Base_


struct MenuItemStateGet : Base_<AUTO_ID> {
    UiDrawEvent::Type menuType = UiDrawEvent::Undef;
    int checked = -1;
};


struct DoAction : Base_<AUTO_ID> {};


struct DoDebugTraceContinue : Base_<AUTO_ID> {};


struct DisasmToggleBreakpoint : Base_<AUTO_ID> {
    AddrRef address = {};
    EReg reg = EReg::PC;
    int setBreakpoint = -1;
    int nBreakpoint = -1;
};


struct CopperToggleBreakpoint : Base_<AUTO_ID> {
    AddrRef address = {};
};

struct CopperTraceStep : Base_<AUTO_ID> {};


#undef AUTO_ID
#undef AUTO_ID_START
};  // namespace qd::action::msg

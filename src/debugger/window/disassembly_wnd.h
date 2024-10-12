#pragma once
#include <EASTL/optional.h>
#include <debugger/vm/memory.h>
#include <src/ui/ui_view.h>

namespace qd::window {

class DisassemblyView : public UiWindow {
    QDB_CLASS_ID(WndId::Disassembly);
    eastl::string addrInputStr;
    eastl::optional<AddrRef> mDisasmAddr;

public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "Disassembly";
    }

    virtual void drawContent() override;

};  // class DisassemblyView

};  // namespace qd::window

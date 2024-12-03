#pragma once
#include <imgui.h>
#include "src/ui/ui_view.h"

namespace qd {
namespace window {


class CustomRegsWnd : public UiWindow {
    QDB_CLASS_ID(WndId::CustomRegsWnd);
    ImGuiTextFilter mRegsFilter;

public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "Custom regs";
    }

    virtual void drawContent() override;

} QDB_WINDOW_REGISTER(CustomRegsWnd);
//////////////////////////////////////////////////////////////////////////


};  // namespace window
};  // namespace qd

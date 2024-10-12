#pragma once
#include "src/ui/ui_view.h"

namespace qd::window {

class ScreenWnd : public UiWindow {
    QDB_CLASS_ID(WndId::Screen);

    ImTextureID mTextureId = 0;

public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "Screen";
    }

    virtual void drawContent() override;

};  // class

};  // namespace qd::window

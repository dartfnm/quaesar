#pragma once
#include "src/ui/ui_view.h"

namespace qd {
namespace window {

class ColorsWnd : public UiWindow {
    QDB_CLASS_ID(WndId::Colors);

public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "Palette";
    }

    virtual void drawContent() override;

} QDB_WINDOW_REGISTER(ColorsWnd);
//////////////////////////////////////////////////////////////////////////

};  // namespace window
};  // namespace qd

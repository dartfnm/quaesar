#pragma once
#include <src/ui/ui_view.h>

namespace qd::window {

class ConsoleLogWriter;

class ConsoleWnd : public UiWindow {
    QDB_CLASS_ID(WndId::Console);
    eastl::string inputStr;
    ConsoleLogWriter* mpConsoleWriter = nullptr;

public:
    virtual void onCreate(UiViewCreate* cp) override;

    virtual void drawContent() override;

    virtual void destroy() override;

} QDB_WINDOW_REGISTER(ConsoleWnd);
//////////////////////////////////////////////////////////////////////////

};  // namespace qd::window

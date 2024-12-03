#include "uae_actions.h"


namespace qd {
namespace action {
namespace uae {

void DebugDmaOption::onDrawMainMenuItem(UiDrawEvent::Type event, void*) {
    switch (event) {
        case UiDrawEvent::MainMenu_Debug: {
            const char* options =
                "off\0"
                "mode 2\0"
                "mode 3\0"
                "mode 4\0"
                "\0";
            int n = ::debug_dma > 0 ? ::debug_dma - 1 : 0;
            if (ImGui::Combo("Debug DMA", &n, options)) {
                eastl::string buf(eastl::string::CtorSprintf(), "v -%d", n + 1);
                getDbg()->applyImmediateConsoleCmd(eastl::move(buf));
            }
        } break;
        default:
            break;
    }
}

};  // namespace uae
};  // namespace action
};  // namespace qd

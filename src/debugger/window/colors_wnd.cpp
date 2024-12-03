#include "colors_wnd.h"
#include <EASTL/fixed_vector.h>
#include <debugger/debugger.h>
#include <debugger/vm/vm.h>
#include <imgui_eastl.h>
#include <src/generic/color.h>

namespace qd {
namespace window {

void ColorsWnd::drawContent() {
    VM* vm = getDbg()->getVm();

    eastl::fixed_vector<Color, 256> palette;

    vm->custom->fetch();

    for (int i = 0; i < 32; i++) {
        uint16_t ecsCol = vm->custom->getRegVal(CustReg::COLOR00 + i);
        Color cc;

        cc.r = (ecsCol >> 8) & 15;
        cc.r |= (cc.r) << 4;
        cc.g = (ecsCol >> 4) & 15;
        cc.g |= (cc.g) << 4;
        cc.b = (ecsCol >> 0) & 15;
        cc.b |= (cc.b) << 4;
        cc.a = 255;

        palette.push_back(cc);
    }

    bool colorChanged = false;
    for (size_t n = 0; n < palette.size(); n++) {
        ImGui::PushID((int)n);
        if ((n % 8) != 0)
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

        Color& palCol = palette[n];
        ImVec4 imcolor;
        palCol.toColorF(imcolor.x, imcolor.y, imcolor.z, imcolor.w);
        ImGui::SetNextItemWidth(20);
        if (ImGui::ColorEdit4(
                "color##3", &imcolor.x,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder)) {
            palette[n] = Color::makeFromF(imcolor.x, imcolor.y, imcolor.z, imcolor.w);
            colorChanged = true;
        }
        ImGui::PopID();
    }

    if (colorChanged) {
        for (size_t i = 0; i < palette.size(); ++i) {
            uint16_t ecsCol = 0;
            Color cc = palette[i];
            ecsCol = ((cc.r & 0xf0) >> 4) << 8 | ((cc.g & 0xf0) >> 4) << 4 | ((cc.b & 0xf0) >> 4) << 0;
            vm->custom->setRegVal(CustReg::COLOR00 + i, ecsCol);
        }
        vm->custom->commit();
    }
}

};  // namespace window
};  // namespace qd

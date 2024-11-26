#pragma once
#include <EASTL/array.h>
#include <imgui.h>
#include "src/generic/color.h"

namespace qd {

#define IT(name, color)
#define UiColorsList(IT)                             \
    IT(DisasmWnd_PcCursor, Color(0, 10, 160))        \
    IT(DisasmWnd_UserCursor, Color(160, 160, 0))     \
    IT(DisasmWnd_OpCodeBytes, Color(128, 128, 128))  \
    IT(DisasmWnd_Addr, Color(192, 192, 192))         \
    IT(RegistersWnd_RegName, Color(164, 164, 164))   \
    IT(RegistersWnd_RegValue, Color(255, 255, 255))  \
    IT(CustomRegsWnd_RegName, Color(165, 164, 164))  \
    IT(CustomRegsWnd_RegValue, Color(255, 255, 255)) \
    /* UI COLOR LIST */

#undef IT


//////////////////////////////////////////////////////////////////////////
struct UiStyle {
public:
#define IT(name, color) name,
    enum EColor : uint32_t { UiColorsList(IT) COUNT };  // enum
#undef IT
    struct ColorRec {
        qd::Color colorU32;
        ImVec4 colorF;
    };
    eastl::array<ColorRec, COUNT> mColors = {};

public:
    UiStyle();
    void applyColors();
    void applyImGuiDarkStyle();

    [[nodiscard]] inline const qd::Color& getColorU(UiStyle::EColor col) const {
        const ColorRec& colorRef = mColors[col];
        return colorRef.colorU32;
    };
    [[nodiscard]] inline const ImVec4& getColorF(UiStyle::EColor col) const {
        const ColorRec& colorRef = mColors[col];
        return colorRef.colorF;
    };

    void setColorU(UiStyle::EColor col, const Color& c) {
        UiStyle::ColorRec& rec = mColors[col];
        rec.colorU32 = c;
        rec.colorF.x = (float)c.r / 255.f;
        rec.colorF.y = (float)c.g / 255.f;
        rec.colorF.z = (float)c.b / 255.f;
        rec.colorF.w = (float)c.a / 255.f;
    }

public:
    [[nodiscard]] static UiStyle* get() {
        static UiStyle instance;
        return &instance;
    }
};  // class UiColors
//////////////////////////////////////////////////////////////////////////
extern UiStyle* gUiColors;


//////////////////////////////////////////////////////////////////////////
[[nodiscard]] inline const Color& uiGetColorU(UiStyle::EColor col) {
    return gUiColors->getColorU(col);
};
[[nodiscard]] inline const ImVec4& uiGetColorF(UiStyle::EColor col) {
    return gUiColors->getColorF(col);
};


};  // namespace qd

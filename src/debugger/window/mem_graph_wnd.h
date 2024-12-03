#pragma once
#include <src/generic/types.h>
#include <src/ui/ui_view.h>

namespace qd {
namespace window {

class MemoryGraphWnd : public UiWindow {
    QDB_CLASS_ID(WndId::MemoryGraph);

    ImTextureID mTextureId = 0;
    Int2 mTextureSize = {-1, -1};
    Int2 mNewTextureSize = {640, 320};
    float mLastTextureCreateTime = FLT_MIN;
    int mCurBank = MemBank::CHIP;
    int mBankOffset = 0x0;
    int mTextureMod = 0;
    int mStartDragBankOffset = 0;


public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "Memory graph";
    }

    virtual void drawContent() override;

};  // class

};  // namespace window
};  // namespace qd

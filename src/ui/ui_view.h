#pragma once
#include <debugger/class_reg.h>
#include <debugger/ui_defs.h>
#include <imgui_eastl.h>
#include <src/generic/color.h>

namespace qd {

class GuiManager;
class Debugger;

struct UiViewCreate {
    GuiManager* gui;
    bool visible = true;

    UiViewCreate(GuiManager* _ui) : gui(_ui) {
    }
};  // struct CreateUiViewParams

#define QDB_CLASS_ID(wnd_id)                           \
public:                                                \
    static const uint32_t CLASS_ID = (uint32_t)wnd_id; \
                                                       \
private:                                               \
    typedef UiView TSuper;

//////////////////////////////////////////////////////////////////////////
//
// Base class of all ui
//
class UiView {
public:
    eastl::string mTitle;
    bool mVisible = true;
    GuiManager* ui = nullptr;
    uint32_t mClassId = 0;

public:
    UiView() = default;

    virtual void onCreate(UiViewCreate* cp) {
        mVisible = cp->visible;
        ui = cp->gui;
    }

protected:
    virtual void updateBeforeDraw() {
    }
    virtual void drawContent() {
    }

public:
    virtual ~UiView() {
    }

    virtual void destroy() {
    }

    virtual void draw() {
        drawContent();
    }

    Debugger* getDbg() const;

};  // class UiView
//////////////////////////////////////////////////////////////////////////

class UiWindow : public UiView {
public:
    // QDB_CLASS_ID();
    UiWindow() = default;

    virtual void onCreate(UiViewCreate* cp) override {
        UiView::onCreate(cp);
    }
    virtual void draw() override;

};  // class UiWindow
//////////////////////////////////////////////////////////////////////////

namespace window {
class ImGuiDemoWindow : public UiWindow {
    QDB_CLASS_ID(WndId::ImGuiDemo);

public:
    virtual void onCreate(UiViewCreate* cp) override {
        UiWindow::onCreate(cp);
        mTitle = "ImGui Demo";
        mVisible = false;
    }

    virtual void draw() override;

};  // class
//////////////////////////////////////////////////////////////////////////

};  // namespace window
//////////////////////////////////////////////////////////////////////////

using UiViewClassRegistry = ClassInfoRegistry_<UiView>;

template <class TClass>
struct UiViewClassRegistrator_ {
    inline UiViewClassRegistrator_() {
        UiViewClassRegistry::MetaInfo metaInfo;
        metaInfo.classId = TClass::CLASS_ID;
        metaInfo.createCallback = (void*)&createClassCb;
        metaInfo.registerClass();
    }

    static UiView* createClassCb(const UiViewClassRegistry::MetaInfo& meta, UiViewCreate* cp) {
        TClass* newInst = new TClass();
        newInst->mClassId = meta.classId;
        newInst->onCreate(cp);
        return newInst;
    }
};  // struct

#define QDB_WINDOW_REGISTER(className) \
    ;                                  \
    static UiViewClassRegistrator_<className> EA_PREPROCESSOR_JOIN(reg, __COUNTER__);

};  // namespace qd

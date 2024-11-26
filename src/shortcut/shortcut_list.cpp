#include "shortcut_list.h"
#include <src/shortcut/shortcut_mgr.h>


namespace qd::shortcut {


typedef void (*ShortcutSetupFunc)(qd::Shortcut&);

static constexpr ShortcutSetupFunc shortcuts_list[] = {
#define SHORTCUT(name, setup_func) setup_func,
    SHORTCUT_LIST(SHORTCUT)
#undef SHORTCUT
};  // ShortcutList


qd::Shortcut* makeInstance(shortcut::EId id) {
    ShortcutSetupFunc setupFunc = shortcuts_list[(int)id];
    qd::Shortcut* pInst = new qd::Shortcut();
    pInst->mId = id;
    setupFunc(*pInst);
    return pInst;
};

};  // namespace qd::shortcut

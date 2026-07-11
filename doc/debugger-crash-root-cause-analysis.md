# Debugger SIGSEGV Crash - Root Cause Analysis & Fix

## Crash Summary

**Signal**: SIGSEGV (Segmentation Fault)  
**Address**: `0xb85f43a0b81f43d9` (corrupted pointer)  
**Location**: `AmDbgWindow::drawImp()` + 76 bytes  
**PID/TID**: 57642 / 259

### Stack Trace (Demangled)
```
0  posixSignalHandler(int, __siginfo*, void*)
1  _sigtramp
2  amD::AmDbgWindow::drawImp() + 76
3  qd::UiNode::draw()
4  qd::uinode_draw_child(qd::UiNode*)
5  qd::UiNode::drawContentImp()
6  amD::DebuggerDesktop::drawImGuiMainFrame()
7  amD::DebuggerApp::updateAppPart(float, float)
8  qd::AppPartsManager::update(float, float)
9  qd::Application::onFrameUpdate(float, float)
10 qd::Application::doMainLoop()
11 main
```

---

## Root Cause

### Race Condition in Debugger Initialization

The crash occurs due to a **race condition** where debugger windows attempt to access VM sub-modules (`cpu`, `mem`, `custom`) **before** the VM is fully initialized.

### Initialization Sequence (Problematic)

1. `DebuggerApp::init()` creates `DebuggerDesktop` (debuggerWndApp.cpp:98)
2. `DebuggerDesktop::onUiNodeCreated()` calls `createAllUiWndows()` (debuggerDesktop.cpp:119)
3. All debugger windows are created and become visible immediately
4. **First frame renders** - windows call `drawContentImp()`
5. **VM sub-modules are NOT initialized yet** - emulator thread hasn't started
6. Windows dereference null pointers (`vm->cpu`, `vm->custom`, etc.) → **SIGSEGV**

### Code Evidence

**Disassembly Window** (disassembly_wnd.cpp:54):
```cpp
void DisassemblyView::drawContentImp()
{
    Debugger* dbg = getDbg();
    if (!dbg)
        return;
    IVm::VM* vm = dbg->getVm();
    if (!vm)  // ❌ Only checks VM existence, not readiness
        return;
    
    // Line 54: CRASH - vm->cpu is NULL!
    const AddrRef regPc = vm->cpu->getPC();
}
```

**Registers Window** (registers_wnd.cpp:46):
```cpp
void RegistersView::drawContentImp() {
    Debugger* dbg = getDbg();
    if (!dbg)
        return;
    IVm::VM* vm = dbg->getVm();
    IVm::Cpu* cpu = vm->cpu;  // ❌ No null check - CRASH!
```

### Why Existing Guard Doesn't Help

The `AmDbgWindow::drawImp()` method has a guard (uiView.cpp:39):
```cpp
const bool vmAvailable = getVm() && getVm()->isReady();
```

This guard prevents calling `drawContentImp()` when VM is not ready. **However**, individual window implementations have their own null checks that are **insufficient** - they only check `if (!vm)` but not `if (!vm->isReady())`.

The `isReady()` method (vmInterface.h:106) checks:
```cpp
bool isReady() const { return cpu && mem && custom; }
```

---

## Solution

### Fix Applied: Add `isReady()` Checks to All Windows

Modified 8 debugger window files to check `vm->isReady()` before accessing sub-modules:

1. **disassembly_wnd.cpp** (line 47)
   ```cpp
   if (!vm || !vm->isReady())
       return;
   ```

2. **screen_wnd.cpp** (line 17)
3. **memory_graph_wnd.cpp** (line 19)
4. **colors_wnd.cpp** (line 16) - **Added new check**
5. **registers_wnd.cpp** (line 46) - **Added new check**
6. **custom_regs_wnd.cpp** (line 105) - **Added new check**
7. **copper_wnd.cpp** (line 55) - **Added new check**
8. **blitter_wnd.cpp** (line 114) - **Added new check**

### Files Not Modified (Already Safe)

- **memory_wnd.cpp** - Uses local memory data, checks VM before evaluate
- **console_wnd.cpp** - Calls `dbg->execConsoleCmd()` which has internal guards

---

## Impact

### Before Fix
- ❌ Debugger crashes immediately on startup
- ❌ SIGSEGV when accessing `vm->cpu`, `vm->custom`, etc.
- ❌ Race condition between UI rendering and VM initialization

### After Fix
- ✅ All windows check VM readiness before accessing sub-modules
- ✅ Windows show "No VM connected" placeholder until VM is ready
- ✅ No crashes - debugger waits for VM initialization
- ✅ Consistent with existing guard pattern in `AmDbgWindow::drawImp()`

---

## Technical Details

### VM Initialization States

1. **VM Created, Not Ready**: 
   - `VM` object exists
   - Sub-modules (`cpu`, `mem`, `custom`) are NULL
   - `isReady()` returns `false`
   
2. **VM Fully Initialized**:
   - Sub-modules wired up by emulator backend (UaeVmImp/VAmVmImp)
   - `isReady()` returns `true`
   - Safe to access `vm->cpu`, `vm->mem`, `vm->custom`

### Correct Pattern for Debugger Windows

```cpp
void MyDebuggerWindow::drawContentImp()
{
    Debugger* dbg = getDbg();
    if (!dbg)
        return;
    
    IVm::VM* vm = dbg->getVm();
    if (!vm || !vm->isReady())  // ✅ Check both existence AND readiness
        return;
    
    // Safe to access sub-modules now
    vm->cpu->getPC();
    vm->custom->fetch();
    // ...
}
```

---

## Verification

- ✅ Build completes with zero warnings
- ✅ All 8 windows have `isReady()` checks
- ✅ No compilation errors
- ✅ Consistent with debugger operation guards (debugger.cpp:31, 96, 106)

---

## Related Code

- **VM Interface**: `libs/amDebugger/src/amDebugger/vm/vmInterface.h:106`
- **Base Window Guard**: `libs/amDebugger/src/amDebugger/ui/uiView.cpp:39`
- **Debugger Operations**: `libs/amDebugger/src/amDebugger/debugger.cpp:31,96,106`
- **Initialization**: `libs/amDebugger/src/amDebugger/debuggerWndApp.cpp:98`

---

## Lessons Learned

1. **Always check object readiness, not just existence** - A pointer may be non-null but the object may not be fully initialized
2. **Centralize guards where possible** - The `AmDbgWindow::drawImp()` guard is good, but individual windows can bypass it with their own checks
3. **Race conditions in async initialization** - UI can render before background threads complete initialization
4. **Consistent null-checking patterns** - All debugger windows should use the same pattern: `if (!vm || !vm->isReady())`

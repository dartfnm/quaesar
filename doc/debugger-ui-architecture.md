# Debugger UI Architecture

Deep analysis of how the Quaesar debugger UI works: component hierarchy, widget inventory, emulator binding mechanism, data flow, and operation dispatch.

---

## 1. Component Hierarchy

The debugger UI lives in its **own OS window** (separate from the emulator display) and is built on a layered architecture:

```mermaid
graph TB
    App[QuaesarApplication] --> DebuggerApp[DebuggerApp<br/>ApplicationPart]
    App --> EmuWnd[QsrMainClientWndApp<br/>Emulator Window]

    DebuggerApp --> SDLWnd[SDL_Window + SDL_Renderer<br/>Dedicated debugger window]
    DebuggerApp --> ImGuiCtx[QImGuiContext<br/>Separate ImGui instance]
    DebuggerApp --> Debugger[Debugger<br/>Engine + VM bridge]
    DebuggerApp --> Desktop[DebuggerDesktop<br/>UI root + dockspace]

    Desktop --> MenuBar[Main Menu Bar]
    Desktop --> ToolBar[Toolbar]
    Desktop --> DockSpace[ImGui DockSpace]
    Desktop --> Operations[OperationsRegistry]
    Desktop --> Shortcuts[ShortcutsMgr]

    DockSpace --> Disasm[DisassemblyView]
    DockSpace --> Mem[MemoryHexViewWnd]
    DockSpace --> Regs[RegistersView]
    DockSpace --> CustReg[CustomRegsWnd]
    DockSpace --> Copper[CopperWnd]
    DockSpace --> Blitter[BlitterWnd]
    DockSpace --> Console[ConsoleWnd]
    DockSpace --> Screen[ScreenWnd]
    DockSpace --> Colors[ColorsWnd]
    DockSpace --> MemGraph[MemoryGraphWnd]
    DockSpace --> ImGuiDemo[ImGuiDemoWindow]
```

**Source files for the hierarchy:**

| Component | Header | Implementation |
|-----------|--------|----------------|
| DebuggerApp | `libs/amDebugger/src/amDebugger/debuggerWndApp.h` | `debuggerWndApp.cpp` |
| Debugger (engine) | `libs/amDebugger/src/amDebugger/debugger.h` | `debugger.cpp` |
| DebuggerDesktop | `libs/amDebugger/src/amDebugger/ui/debuggerDesktop.h` | `ui/debuggerDesktop.cpp` |
| AmDbgWindow (base) | `libs/amDebugger/src/amDebugger/ui/uiView.h` | `ui/uiView.cpp` |
| VM abstraction | `libs/amDebugger/src/amDebugger/vm/vmInterface.h` | `vm/vmInterface.cpp` |
| Connection bridge | `libs/amDebugger/src/amDebugger/dbgConnection.h` | `dbgConnection.cpp` |

---

## 2. Lifecycle and Initialization Sequence

```mermaid
sequenceDiagram
    participant App as QuaesarApplication
    participant DbgApp as DebuggerApp
    participant Dbg as Debugger
    participant Desktop as DebuggerDesktop
    participant TypeReg as TypeRegistry
    participant OpReg as OperationsRegistry
    participant ShortMgr as ShortcutsMgr

    App->>DbgApp: new DebuggerApp()
    Note over DbgApp: setPartActive(true), setPartRenderable(true)
    Note over DbgApp: new Debugger(this)

    App->>DbgApp: onPartCreate()
    App->>DbgApp: init()
    DbgApp->>DbgApp: createRenderWindow() [SDL_WINDOW_HIDDEN]
    DbgApp->>DbgApp: initImGui() [new QImGuiContext, DockingEnable]
    DbgApp->>Dbg: setDbgServiceBridge(create_dummy_connection())
    Dbg->>Dbg: connection->getClientVm() -> creates IVm::VM

    DbgApp->>Desktop: mk.make_<DebuggerDesktop>(dbgApp, debugger)
    Desktop->>Desktop: onUiNodeCreated()
    Desktop->>OpReg: OperationsRegistry::get()
    Desktop->>ShortMgr: ShortcutsMgr::get()
    ShortMgr->>ShortMgr: createPredefinedShortcuts(g_shortcuts_list)

    Desktop->>TypeReg: findAllDerivedFromTypesCached_<AmDbgWindow>()
    loop For each registered window type
        TypeReg-->>Desktop: TypeInfo with CreateClassCb
        Desktop->>Desktop: pCreateAttr->makeInstance_<AmDbgWindow>(ctx)
        Note over Desktop: new WindowClass() -> onCreate(ctx)
        Desktop->>Desktop: addChild(window)
    end

    Desktop->>OpReg: createOperations(&operationCreate)
    OpReg->>OpReg: For each DECLARE_OPERATION_1 auto-registration
    Note over OpReg: Creates UiOperation instances, binds shortcuts
```

**Key source:** [debuggerWndApp.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/debuggerWndApp.cpp) lines 42-55, [debuggerDesktop.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/debuggerDesktop.cpp) lines 113-129.

---

## 3. Per-Frame Rendering Loop

Every frame, `DebuggerApp::updateAppPart()` drives the entire render pipeline:

```mermaid
sequenceDiagram
    participant Parts as AppPartsManager
    participant DbgApp as DebuggerApp
    participant ImGui as QImGuiContext
    participant Dbg as Debugger
    participant Desktop as DebuggerDesktop
    participant VM as IVm::VM

    Parts->>DbgApp: updateAppPart(dt, time)
    alt Window visible
        DbgApp->>ImGui: newFrame()
        DbgApp->>Dbg: fetchVmState()
        Dbg->>VM: fetchStateFromEmu()
        loop For each IModule (mem, cpu, custom, copper, blitter, floppy, emu)
            VM->>VM: module->fetch()
        end
        DbgApp->>Desktop: drawImGuiMainFrame()
        Desktop->>Desktop: _drawMainMenuBar()
        Desktop->>Desktop: _drawToolBar()
        Desktop->>Desktop: ImGui::DockSpace()
        loop For each child AmDbgWindow
            Desktop->>Desktop: child->drawImp() -> drawContentImp()
        end
        Desktop->>Desktop: testOperationsShortcuts_<>(this)
        DbgApp->>ImGui: endFrame()
    else Window hidden
        DbgApp->>ImGui: skipFrame()
    end

    Parts->>DbgApp: renderAppPart()
    DbgApp->>ImGui: render(bgColor)
```

**Source:** [debuggerWndApp.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/debuggerWndApp.cpp) lines 132-150.

---


## 4. Emulator Binding: The VM Connection Bridge

The debugger never touches emulator internals directly. Instead, it goes through a layered bridge:

```mermaid
graph LR
    subgraph "Debugger Window (main thread)"
        Wnd[AmDbgWindow subclass]
        Dbg[Debugger engine]
    end

    subgraph "Connection Layer"
        Bridge[IVmDbgServiceBridge]
    end

    subgraph "VM Abstraction (IVm)"
        VM[IVm::VM]
        CPU[IVm::Cpu]
        MEM[IVm::Memory]
        CR[IVm::CustomRegs]
        COP[IVm::Copper]
        BLT[IVm::Blitter]
        EMU[IVm::Emu]
        FLP[IVm::Floppy x4]
    end

    subgraph "Backend Implementation"
        UAE[UaeVmImp<br/>reads UAE globals]
        VAM[VAmVmImp<br/>reads vAmiga API]
    end

    subgraph "Emulator Core (separate thread)"
        UAECORE[WinUAE/FS-UAE<br/>m_pAmigaBuffer + regs]
        VAMCORE[vAmiga<br/>VAmiga::Amiga]
    end

    Wnd -->|"getDbg() → getVm()"| Dbg
    Dbg -->|m_pConnection| Bridge
    Bridge -->|"getClientVm"| VM
    VM --> CPU
    VM --> MEM
    VM --> CR
    VM --> COP
    VM --> BLT
    VM --> EMU
    VM --> FLP

    CPU -.->|virtual dispatch| UAE
    MEM -.->|virtual dispatch| UAE
    CR -.->|virtual dispatch| UAE
    UAE -->|direct memory read| UAECORE

    CPU -.->|virtual dispatch| VAM
    VAM -->|public API| VAMCORE
```

### Connection Types

| Connection Class | File | Description |
|------------------|------|-------------|
| `DummyVmDbgServiceBridge` | `libs/amDebugger/src/amDebugger/dbgConnection.cpp:9-23` | Creates a standalone `IVm::VM` with default-constructed modules. Used during initial setup before any emulator is active. Client and server VMs are the same object. |
| `UaeSharedConnectionImpl` | `src/quasar_app/uae_imp/uae_server_app_part.cpp:21-35` | Wraps the UAE-backed `UaeVmImp` instance. Shared: same VM object for both client (debugger reads) and server (debugger writes). Direct pointer to UAE globals. |

### Binding Sequence

```mermaid
sequenceDiagram
    participant Factory as UaeServerProviderFactory
    participant Selector as VmPlayersSelector
    participant DbgApp as DebuggerApp
    participant Dbg as Debugger
    participant Bridge as UaeSharedConnectionImpl
    participant VM as IVm::VM (UaeVmImp)
    participant Thread as UaeServerThread

    Factory->>Factory: setup() [id="uae", guiName="UAEmu"]
    Selector->>Factory: createServerAppPart()
    Factory->>Thread: new UaeServerThread() -> initialize()
    Note over Thread: UAE core starts in SDL thread

    Selector->>Factory: createVmDebuggerConnection()
    Factory->>VM: m_pUaeAppPart->getVm()
    VM-->>Factory: UaeVmImp instance (lives in UaeServerThread)
    Factory->>Bridge: new UaeSharedConnectionImpl(vm)

    Selector->>DbgApp: (sets connection on debugger)
    DbgApp->>Dbg: setDbgServiceBridge(bridge)
    Dbg->>Bridge: getClientVm()
    Bridge-->>Dbg: IVm::VM pointer
    Note over Dbg: m_pVm now points to UaeVmImp

    Note over Dbg,Thread: Every frame: Dbg->fetchVmState() -> vm->fetchStateFromEmu()<br/>UaeVmImp::Cpu::fetch() reads UAE 'regs' global<br/>UaeVmImp::Memory::fetch() reads UAE 'addrmap[]'
```

**Source:** [uae_server_app_part.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/src/quasar_app/uae_imp/uae_server_app_part.cpp) lines 56-77, [dbgConnection.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/dbgConnection.cpp) lines 9-29.

---

## 5. Window Widget Inventory

All debugger windows inherit from `AmDbgWindow` (which inherits `qd::UiWindow` + `IOperationEnvironment`). Windows are auto-discovered at startup via the type reflection system.

### Window Registration Mechanism

Each window uses the `QDB_WINDOW_REGISTER` macro:

```cpp
// In disassembly_wnd.h:
class DisassemblyView : public amD::AmDbgWindow {
    QDB_WINDOW_REGISTER(WndId::Disassembly, amD::window::DisassemblyView, amD::AmDbgWindow);
    // ...
};
```

This expands to:
1. `TS_BEGIN_REFLECT_CLASS` -- registers the type with `TypeRegistry`
2. `TS_ATTRIBUTE(CustomClassId32(WndId::Disassembly))` -- binds enum ID
3. `TS_ATTRIBUTE(CreateClassCb(&createWindowCb_<DisassemblyView>))` -- registers factory function

At startup, `DebuggerDesktop::createAllUiWndows()` queries `TypeRegistry::findAllDerivedFromTypesCached_<AmDbgWindow>()`, iterates all registered types, calls the factory for each, and adds the resulting window as a child.

**Source:** [uiView.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/uiView.h) lines 73-88, [debuggerDesktop.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/debuggerDesktop.cpp) lines 132-150.

### Complete Window Catalog

| WndId | Class | Header | Impl | UI Widgets | VM Data Read | VM Data Written |
|-------|-------|--------|------|------------|-------------|-----------------|
| `Disassembly` | `DisassemblyView` | `window/disassembly_wnd.h` | 239 lines | InputText (address expr), Button (PC), 4-column Table (breakpoint dot, addr hex, opcode bytes, mnemonic), mouse wheel scroll | `cpu->getPC()`, `memory->getU16/U32()` via Capstone | Breakpoints via `DisasmToggleBreakpoint` op |
| `MemoryView` | `MemoryHexViewWnd` | `window/memory_wnd.h` | 850 lines | InputText (addr expr), hex grid Table, ASCII column, data preview (bin/dec/hex), bank selector, goto dialog | `memory->getRealAddr()`, bank structure | `memory->setU16/setU32()` (cell editing) |
| `Registers` | `RegistersView` | `window/registers_wnd.h` | 129 lines | 4-column Table (A0-A7 + D0-D7 side-by-side), PC row, IMASK row, flag rows (Z/C/N/V/X) | `cpu->getRegA(i)`, `cpu->getRegD(i)`, `cpu->getPC()`, `cpu->getFlg()`, `cpu->getIntMask()` | Console command `r <reg> <val>` |
| `CustomRegsWnd` | `CustomRegsWnd` | `window/custom_regs_wnd.h` | 143 lines | TextFilter input, 4-column Table (reg name + value, two per row), tooltip with flag bitfields | `custom->getRegVal(reg)`, `custom->fetch()` (extra refresh) | InputText (not yet connected) |
| `CopperDbgWnd` | (CopperWnd) | (no separate .h) | 169 lines | Copper list Table (addr, hex words, decoded instruction), breakpoint column | `copper->getCopperAddr()`, `memory->getU16/U32()` | `CopperToggleBreakpoint` op |
| `BlitterWnd` | (BlitterWnd) | (no separate .h) | 209 lines | Register table (BLTCON0/1, BLTAPT/BLTBPT/etc, BLTCDATA/CDATB/CDATC/CDATD), status indicator | `custom->getRegVal()` for blitter regs | None |
| `Console` | `ConsoleWnd` | `window/console_wnd.h` | 115 lines | Scrolling log output (custom `ConsoleLogWriter`), InputText with Enter-to-execute | None (reads log) | `Debugger::execConsoleCmd()` |
| `Screen` | (ScreenWnd) | (no separate .h) | 120 lines | ImGui::Image of emulator framebuffer | `blitter->getScreenPixBuf()` | None |
| `Colors` | (ColorsWnd) | (no separate .h) | 64 lines | 32 color swatches (ImGui::ColorButton) with hex values | `custom->getRegVal()` for color registers | None |
| `MemoryGraph` | `MemoryGraphWnd` | `window/memory_graph_wnd.h` | 204 lines | SDL_Texture rendered as ImGui::Image (byte value = grayscale pixel), bank selector, scrollable | `memory->getRealAddr()`, bank data | None |
| `ImGuiDemo` | `ImGuiDemoWindow` | `ui/uiView.h:93` | -- | Standard ImGui demo (hidden by default) | None | None |

### How Each Window Accesses VM State

Every window follows the same pattern:

```mermaid
graph LR
    Wnd[AmDbgWindow subclass] -->|"getDbg()"| Dbg[Debugger]
    Dbg -->|"getVm()"| VM[IVm::VM]
    VM -->|".cpu"| CPU[IVm::Cpu]
    VM -->|"memory"| MEM[IVm::Memory]
    VM -->|".custom"| CR[IVm::CustomRegs]
    VM -->|".copper"| COP[IVm::Copper]
    VM -->|".blitter"| BLT[IVm::Blitter]
    VM -->|".emu"| EMU[IVm::Emu]
    VM -->|"floppy 0-3"| FLP[IVm::Floppy]
```

**Implementation chain** (example from `RegistersView::drawContentImp()`):

```cpp
Debugger* dbg = getDbg();         // returns Desktop->m_pDbg
IVm::VM* vm = dbg->getVm();      // returns m_pVm (the UaeVmImp)
IVm::Cpu* cpu = vm->cpu;         // UaeVmImp::Cpu*
cpu->getRegA(i);                 // reads UAE regs.regs[8+i]
cpu->getRegD(i);                 // reads UAE regs.regs[i]
cpu->getPC();                    // reads UAE regs.pc
```

---



## 6. Operation Dispatch System

The debugger uses a **chain-of-responsibility** pattern for operations. Each `IOperationEnvironment` can handle an operation or pass it to its parent.

### Operation Class Hierarchy

```mermaid
graph TB
    BaseOp["qd::operation::BaseOpArgs<br/>getTypeInfo, clone, cast_"]

    OpsArgs["amD::operation::OperationArgs<br/>empty setup"]

    Reset[VmEmuReset<br/>F12 - Reset Amiga]
    Exec[ExecConsoleCmd<br/>cmd: string]
    Dma[DebugDmaOption<br/>dmaMode: int]
    TraceStart[DebugTraceStart<br/>F12 - enter debug mode]
    TraceCont[DebugTraceContinue<br/>F5 - resume emulation]
    StepIn[DisasmTraceStepInto<br/>F11 - step one instruction]
    StepOut[DisasmTraceStepOut<br/>F10 - step over]
    BpToggle[DisasmToggleBreakpoint<br/>F9 - toggle BP at address]
    CopStep[CopperTraceStep<br/>Shift+F11]
    CopBp[CopperToggleBreakpoint<br/>Shift+F9]
    Turbo[ToggleTurboEmulation<br/>NumLock]
    WaitSL[DebugWaitScanLines<br/>wait N scanlines]
    AlwaysTop[VmPlayerWndAlwaysOnTop<br/>Ctrl+T]

    BaseOp --> OpsArgs
    OpsArgs --> Reset
    OpsArgs --> Exec
    OpsArgs --> Dma
    OpsArgs --> TraceStart
    OpsArgs --> TraceCont
    OpsArgs --> StepIn
    OpsArgs --> StepOut
    OpsArgs --> BpToggle
    OpsArgs --> CopStep
    OpsArgs --> CopBp
    OpsArgs --> Turbo
    OpsArgs --> WaitSL
    OpsArgs --> AlwaysTop
```

### Dispatch Chain

Operations flow through the `IOperationEnvironment` chain from UI to VM:

```mermaid
graph TB
    subgraph "Trigger Sources"
        Shortcut["Keyboard Shortcut<br/>(ShortcutsMgr)"]
        MenuClick["Menu Item Click<br/>(menuItemOperation)"]
        ToolbarBtn["Toolbar Button<br/>(ImGui::Button)"]
        WindowAction["Window Action<br/>(e.g. breakpoint click)"]
        ConsoleInput["Console Input<br/>(Enter key)"]
    end

    subgraph "Dispatch Chain (IOperationEnvironment)"
        Desktop["DebuggerDesktop<br/>applyOperationMsgProcImp()"]
        Dbg["Debugger<br/>applyOperationMsgProcImp()"]
        VM["IVm::VM<br/>applyOperationMsgProcImp()"]
    end

    subgraph "UAE Backend"
        UAE["UaeVmImp<br/>operation handlers"]
        UAECore["UAE core<br/>activate_debugger(), etc."]
    end

    Shortcut --> Desktop
    MenuClick --> Desktop
    ToolbarBtn --> Desktop
    WindowAction --> Desktop
    ConsoleInput -->|execConsoleCmd| Dbg

    Desktop -->|"getOpEnvParent = Dbg"| Dbg
    Dbg -->|"vm.applyOperationMsgProcImp()"| VM
    VM -->|"virtual dispatch"| UAE
    UAE --> UAECore
```

### Dispatch Examples

**Step Into (F11):**
```mermaid
sequenceDiagram
    participant SM as ShortcutsMgr
    participant Desktop as DebuggerDesktop
    participant Dbg as Debugger
    participant VM as IVm::VM

    SM->>Desktop: testOperationsShortcuts -> F11 pressed
    Desktop->>Desktop: new DisasmTraceStepInto()
    Desktop->>Desktop: setupDefaultOperationArgs(&opArgs)
    Desktop->>Desktop: applyOperationMsgProcImp(&opArgs)
    Note over Desktop: Not a DisasmToggleBreakpoint, passes to parent
    Desktop->>Dbg: m_pDbg->applyOperationMsgProcImp(&opArgs)
    Dbg->>VM: m_pVm->applyOperationMsgProcImp(&opArgs)
    Note over VM: UaeVmImp handles: sets UAE debug trace mode
```

**Toggle Breakpoint (F9):**
```mermaid
sequenceDiagram
    participant User as User click in DisasmView
    participant Disasm as DisassemblyView
    participant Desktop as DebuggerDesktop
    participant Dbg as Debugger
    participant VM as IVm::VM

    User->>Disasm: Click breakpoint column at address X
    Disasm->>Disasm: new DisasmToggleBreakpoint()
    Note over Disasm: p.address = curAddr, p.reg = EReg::PC
    Disasm->>Dbg: dbg->applyOperationMsgProcImp(&p)
    Dbg->>VM: m_pVm->applyOperationMsgProcImp(&p)
    Note over VM: UaeVmImp::Emu handles breakpoint add/remove
```

**Source:** [debuggerDesktop.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/debuggerDesktop.cpp) lines 31-41, [debugger.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/debugger.cpp) lines 25-29.

---

## 7. Code Analysis Engine (Disassembly Backend)

The `DisassemblyView` window delegates actual M68K instruction decoding to the `M68CodeDisassembler` singleton, which provides a page-cached disassembly service:

```mermaid
graph TB
    Disasm[DisassemblyView] -->|"requestM68DisasmLines"| CDA[cda::M68CodeDisassembler<br/>Singleton]

    CDA --> Pool["CodeChunk Pool<br/>64 pages x 64 bytes"]
    CDA --> Tree["QuadTreeAddrMap<br/>addr -> chunk index"]
    CDA --> LRU["intrusive_list<br/>LRU eviction order"]
    CDA --> Capstone["Capstone csh<br/>CS_ARCH_M68K"]

    Pool --> Items["cda::CodeItem[]<br/>disassembly results"]
    Items --> Text["m_text: 'MOVE.L (A0),D0'"]
    Items --> Bytes["m_bytesString: '20 10'"]
    Items --> Addr["m_addr: $0023A4"]

    Disasm -->|getVm| VM[IVm::VM]
    VM -->|"memory.getU16/U32()"| Backend[UaeVmImp::Memory]
    Backend -->|direct pointer| UAE[UAE memory banks]
```

### Disassembly Cache Flow

```mermaid
sequenceDiagram
    participant DV as DisassemblyView
    participant CDA as M68CodeDisassembler
    participant QT as QuadTreeAddrMap
    participant VM as IVm::VM
    participant CS as Capstone

    DV->>CDA: requestM68DisasmLines(vm, startAddr, nLines, &outItems)
    CDA->>QT: querySingle(addr) -> chunk index?

    alt Cache hit and bytes unchanged
        QT-->>CDA: chunkIdx found
        CDA->>CDA: Compare cached bytes with vm->memory
        Note over CDA: bytes match -> reuse cached CodeItems
        CDA-->>DV: Return cached items
    else Cache miss or bytes changed
        CDA->>CDA: getOrCreateCodePage(addr)
        alt Pool has free page
            CDA->>CDA: Claim free CodeChunk
        else Pool full
            CDA->>CDA: Evict LRU tail chunk
            CDA->>QT: remove(old chunk addr)
        end
        CDA->>VM: memory->getU16/U32() for 64 bytes
        CDA->>CS: cs_disasm() for page bytes
        CS-->>CDA: Capstone instructions
        CDA->>CDA: Create CodeItem for each instruction
        CDA->>QT: insert(addr, chunkIdx)
        CDA->>CDA: Move chunk to LRU head
        CDA-->>DV: Return new items
    end
```

**Source:** [cdaServer.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/codeAnalyzer/cdaServer.h), [cdaServer.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/codeAnalyzer/cdaServer.cpp).

---


## 8. Menu Bar and Toolbar

### Main Menu Structure

Drawn by `DebuggerDesktop::_drawMainMenuBar()`:

```mermaid
graph LR
    MenuBar[Main Menu Bar]

    MenuBar --> File["File"]
    File --> Empty1["(empty)"]

    MenuBar --> Emulator["Emulator"]
    Emulator --> AlwaysOn["Always on Top (Ctrl+T)"]
    Emulator --> Reset["Reset Amiga"]

    MenuBar --> Debug["Debug"]
    Debug --> Continue["Continue (F5)"]
    Debug --> TraceMode["Debug Trace Mode (F12)"]
    Debug --> Sep1["---"]
    Debug --> StepInto["Step Into (F11)"]
    Debug --> StepOut["Step Out (F10)"]
    Debug --> BrkPt["Disasm breakpoint (F9)"]
    Debug --> Sep2["---"]
    Debug --> CopStep["Copper Trace Step (Shift+F11)"]
    Debug --> CopBrk["Copper breakpoint (Shift+F9)"]
    Debug --> Sep3["---"]
    Debug --> DMA["Debug DMA (combo)"]

    MenuBar --> Window["Window"]
    Window --> AllWnd["(one checkbox per AmDbgWindow child)"]
```

The **Window** menu is dynamically populated by iterating `getChild(i)` for each registered debugger window. Each window's title appears as a checkable menu item toggling its visibility.

**Source:** [debuggerDesktop.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/debuggerDesktop.cpp) lines 50-110.

### Toolbar

Drawn by `DebuggerDesktop::_drawToolBar()` as a horizontal `ImGui::BeginChild("ToolBar")`:

| Widget | Type | Operation | Source |
|--------|------|-----------|--------|
| "Trace" checkbox | `ImGui::Checkbox` | Toggles between `EVmDebugMode::Break` and `Live` | `dbg->setDebugMode()` |
| Step Into button | `ImGui::ImageButton` | `DisasmTraceStepInto` | `doOperation_<>()` |
| Scanlines input | `ImGui::InputInt` | Sets `dbg->setWaitScanLines(n)` | Direct setter |
| "Wait Scanlines" button | `ImGui::Button` | `DebugWaitScanLines` | `doOperation_<>()` |

**Source:** [debuggerDesktop.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/debuggerDesktop.cpp) lines 202-267.

---

## 9. Keyboard Shortcut System

Shortcuts are defined via X-macro in `shortcutsList.h`, which generates both the `amD::shortcut::EId` enum and the `g_shortcuts_list[]` initialization array:

```mermaid
graph TB
    Define["SHORTCUT_LIST X-macro<br/>shortcutsList.h"]
    Define --> Enum["amD::shortcut::EId enum<br/>(DisasmTraceStepInto, etc.)"]
    Define --> Array["g_shortcuts_list[] array<br/>of ShortcutInitItem"]

    Array -->|createPredefinedShortcuts| SMgr["ShortcutsMgr<br/>singleton"]
    SMgr -->|binds to OpDesc| OReg["OperationsRegistry"]

    OReg -->|testOperationsShortcuts| Desktop["DebuggerDesktop<br/>every frame"]
    Desktop -->|matches pressed keys| Dispatch["Operation dispatch"]
```

### Complete Shortcut Table

| Shortcut ID | Key Binding | Operation | Context |
|-------------|-------------|-----------|---------|
| `DisasmTraceStepInto` | F11 (repeat) | Step one M68K instruction | Debug menu, toolbar |
| `DisasmTraceStepOut` | F10 (repeat) | Step over instruction | Debug menu |
| `DebugTraceStart` | F12 | Enter debug/break mode | Debug menu |
| `DebugTraceContinue` | F5 | Resume emulation (Live mode) | Debug menu |
| `DisasmToggleBreakpoint` | F9 | Toggle breakpoint at cursor | Debug menu |
| `CopperToggleBreakpoint` | Shift+F9 (repeat) | Toggle copper breakpoint | Debug menu |
| `CopperTraceStep` | Shift+F11 (repeat) | Step copper instruction | Debug menu |
| `ToggleTurboEmulation` | NumLock | Toggle turbo CPU speed | (no menu) |
| `ResetAmigaEmu` | (none) | Reset Amiga | Emulator menu |
| `AlwaysOnTopEmu` | Ctrl+T | Emulator window always-on-top | Emulator menu |
| `ShowDebuggerWnd` | Shift+F12 | Show/hide debugger window | App level |
| `ShowUaeOptionsWnd` | Ctrl+P | Open UAE options dialog | App level |

**Source:** [shortcutsList.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/shortcutsList.h).

---

## 10. Expression Evaluator Bridge

Several windows (`DisassemblyView`, `MemoryHexViewWnd`, `MemoryGraphWnd`) accept **expression-based address input** through the `ExprValStr` bridge:

```mermaid
graph LR
    Input["ImGui::InputText '$C000' or 'd0+32'"] -->|"Enter pressed"| EVS["ExprValStr setStrVal"]
    EVS -->|"Expr.parse"| Parser["ParserOop::Expr<br/>AST tree"]
    Parser -->|"IdentifierResolver"| Resolver["ExprResolver<br/>resolves 'd0' to vm register"]
    Parser -->|"evaluate()"| Evaluator["ExprEvaluator<br/>reads vm.cpu.getRegD(0)"]
    Evaluator -->|Var16 result| Addr["AddrRef<br/>address to display"]
```

**Source:** [exprValue.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/exprValue.h), [exprValue.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/exprValue.cpp), [parser_oop.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/exprParser/parser/parser_oop.h), [resolve_oop.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/exprParser/parser/resolve_oop.h).

---

## 11. Color Theme System

All debugger window colors are managed by the `UiStyle` singleton, which uses an X-macro to define a named color palette:

```mermaid
graph TB
    Theme["UiStyle singleton<br/>extends ImColorsTab"]
    Theme --> PC["DisasmWnd_PcCursor<br/>RGB(0,10,160) blue"]
    Theme --> UC["DisasmWnd_UserCursor<br/>RGB(160,160,0) yellow"]
    Theme --> OB["DisasmWnd_OpCodeBytes<br/>RGB(128,128,128) gray"]
    Theme --> DA["DisasmWnd_Addr<br/>RGB(192,192,192) silver"]
    Theme --> RN["RegistersWnd_RegName<br/>RGB(164,164,164)"]
    Theme --> RV["RegistersWnd_RegValue<br/>RGB(255,255,255) white"]
    Theme --> CN["CustomRegsWnd_RegName<br/>RGB(165,164,164)"]
    Theme --> CV["CustomRegsWnd_RegValue<br/>RGB(255,255,255) white"]
```

Windows access colors via the free functions `uiGetColorU(id)` (returns `qd::Color`) and `uiGetColorF(id)` (returns `ImVec4`), used in `ImGui::TextColored()`, `ImGui::PushStyleColor()`, and `ImGui::TableSetBgColor()`.

**Source:** [uiStyle.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/libs/amDebugger/src/amDebugger/ui/uiStyle.h).

---

## 12. Threading Model

The debugger UI and the emulator run on **separate threads** with mutex-protected data exchange:

```mermaid
graph TB
    subgraph "Main Thread"
        AppMgr["AppPartsManager<br/>doMainLoop()"]
        DbgApp["DebuggerApp<br/>updateAppPart()"]
        DbgFetch["Debugger::fetchVmState()"]
        Draw["ImGui rendering"]
    end

    subgraph "Emulator Thread (SDL)"
        UAEThread["UaeServerThread<br/>onUaeThreadMain()"]
        UAERun["UAE emulation loop"]
        ScreenBuf["m_pAmigaBuffer<br/>screen pixels"]
        ScrMutex["m_UaeScrTextureMutex"]
    end

    AppMgr --> DbgApp
    DbgApp --> DbgFetch
    DbgFetch -->|"IVm module.fetch()"| Backend["UaeVmImp modules"]
    Backend -->|"reads UAE globals<br/>(regs, custom_regs, addrmap)"| UAERun
    DbgApp --> Draw

    UAEThread --> UAERun
    UAERun --> ScreenBuf
    ScreenBuf -->|"lock/unlock mutex"| ScrMutex

    ScrMutex -.->|qsr_lockUaeScreenTexBuf| MainThread["Main thread<br/>texture blit"]
```

Key insight: `Debugger::fetchVmState()` reads UAE globals (like `regs`, `custom_regs`) **without a mutex**. This works because:
1. The UAE globals are read atomically (single 32-bit values)
2. The debugger reads snapshots that may be slightly stale
3. The emulator thread runs continuously; the debugger just samples state

Screen buffer exchange **does** use `m_UaeScrTextureMutex` because the pixel buffer is large and requires coordinated access.

**Source:** [uae_server_thread.h](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/src/quasar_app/uae_imp/uae_server_thread.h), [uae_server_thread.cpp](file:///Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/src/quasar_app/uae_imp/uae_server_thread.cpp).

---

## 13. Complete Data Flow Summary

End-to-end trace from user action to emulator state change:

```mermaid
sequenceDiagram
    participant User
    participant ImGui as ImGui Widgets
    participant Window as AmDbgWindow
    participant Desktop as DebuggerDesktop
    participant Dbg as Debugger
    participant VM as IVm::VM
    participant Imp as UaeVmImp
    participant UAE as UAE Core

    Note over User,UAE: EXAMPLE: User presses F11 (Step Into)

    User->>ImGui: Key event: F11
    ImGui->>Desktop: testOperationsShortcuts_ matches F11
    Desktop->>Desktop: new DisasmTraceStepInto()
    Desktop->>Desktop: setupDefaultOperationArgs(&op)
    Desktop->>Desktop: applyOperationMsgProcImp(&op)
    Note over Desktop: Desktop doesn't handle it, passes to parent
    Desktop->>Dbg: m_pDbg->applyOperationMsgProcImp(&op)
    Note over Dbg: Debugger doesn't handle it either
    Dbg->>VM: m_pVm->applyOperationMsgProcImp(&op)
    VM->>Imp: Virtual dispatch to UaeVmImp
    Imp->>UAE: activate_debugger() / debug trace step
    UAE-->>Imp: returns
    Imp-->>VM: EFlow::DONE
    VM-->>Dbg: EFlow::DONE

    Note over User,UAE: NEXT FRAME: State refresh

    Dbg->>VM: fetchVmState()
    VM->>Imp: cpu->fetch()
    Imp->>UAE: reads regs global
    UAE-->>Imp: D0-D7, A0-A7, PC, SR
    VM->>Imp: memory->fetch()
    Imp->>UAE: reads addrmap[], chipmem ptrs
    VM->>Imp: custom->fetch()
    Imp->>UAE: reads custom_regs array

    Note over User,UAE: RENDER: UI updates

    Desktop->>Window: drawContentImp() for each window
    Window->>Dbg: getDbg()->getVm()
    Window->>VM: vm->cpu->getPC() -> new PC value
    Window->>ImGui: Renders with new state
    ImGui-->>User: Updated display
```

# Quaesar RE Research: Internal Architecture

This document is a reverse-engineering research note covering the internal mechanics of the Quaesar codebase. It traces data flows, documents design patterns, and maps the class hierarchy for developers contributing to or analyzing the project.

## Class Hierarchy Map

The following shows the primary inheritance tree, from root base classes to concrete implementations:

```
qd::RefCounted                         (Reference-counted object base)
  +-- IVm::VM                          (VM snapshot container)
  |     +-- IVm::imp::UaeVmImp         (UAE backend, final)
  |     +-- IVm::imp::VAmVmImp         (vAmiga backend, final)
  +-- amD::IVmDbgServiceBridge         (Debug connection interface)
  +-- amD::IVmConnectionBuilder        (Connection factory)
  +-- amD::Debugger                    (Debugger engine)

qd::IOperationEnvironment              (Operation message handler)
  +-- IVm::VM                          (via multiple inheritance)
  +-- amD::Debugger                    (via multiple inheritance with RefCounted)
  +-- amD::DebuggerDesktop             (UI operation routing)
  +-- amD::AmDbgWindow                 (Window-level operation handling)
  +-- qsr::QsrVmClientPlayerGuiDesktop (Emulator window operation handling)

qd::Node                               (Tree node base)
  +-- qd::ApplicationPart              (Modular application component)
  |     +-- qsr::BaseVmServerAppPart   (Abstract VM server)
  |     |     +-- UaeServerAppPart     (UAE server app part)
  |     |     +-- VAmServerAppPart     (vAmiga server app part)
  |     +-- QsrMainClientWndApp        (Emulator display window)
  |     +-- amD::DebuggerApp           (Debugger window app)
  |     +-- BarmanProfileViewerAppPart (Placeholder)
  |
  +-- qd::UiNode                       (UI tree node)
        +-- qd::UiDesktop              (Root UI container)
        |     +-- amD::DebuggerDesktop (Debugger menu + toolbar + windows)
        |     +-- QsrVmClientPlayerGuiDesktop (Emulator overlay UI)
        +-- qd::UiWindow               (ImGui window wrapper)
        |     +-- amD::AmDbgWindow     (Base debugger window)
        |           +-- DisassemblyView
        |           +-- MemoryWnd
        |           +-- MemoryGraphWnd
        |           +-- RegistersWnd
        |           +-- CustomRegsWnd
        |           +-- CopperWnd
        |           +-- BlitterWnd
        |           +-- ColorsWnd
        |           +-- ScreenWnd
        |           +-- ConsoleWnd
        |           +-- ImGuiDemoWindow
        +-- qd::UiDialog               (Modal/popup dialog)
              +-- qsr::BaseOptionsDlg   (Generic options browser)
                    +-- qsr::UaeOptionsDlg (UAE-specific options)

IVm::IModule                           (VM module base)
  +-- IVm::Memory                      (Memory read/write)
  +-- IVm::Cpu                         (CPU register access)
  +-- IVm::CustomRegs                  (Amiga custom registers)
  +-- IVm::Copper                      (Copper coprocessor)
  +-- IVm::Blitter                     (Blitter + screen buffer)
  +-- IVm::Emu                         (Emulation control)
  +-- IVm::Floppy                      (Floppy drive)

qsr::IVmClientPlayer                   (Thread interface for VM)
  +-- UaeServerThread                  (UAE emulator thread)
  +-- VAmServerThread                  (vAmiga emulator thread)

IAppPartServerProviderFactory          (Plugin factory interface)
  +-- UaeAppPartServerProviderFactory  (UAE backend registration)
  +-- VAmAppPartServerProviderFactory  (vAmiga backend registration)
```


## Operation Message Flow: Step Into (F11)

This section traces the complete path of a "Step Into" debug operation from keyboard press to emulator execution.

### Step 1: Key Press Detection

```
SDL sends ImGuiKey_F11 key event
  -> qd::ShortcutsMgr checks bound shortcuts
  -> Finds amD::shortcut::EId::DisasmTraceStepInto bound to F11
```

### Step 2: Operation Creation

```
amD::shortcut::DisasmTraceStepInto fires
  -> OperationsRegistry creates DisasmTraceStepInto args struct
  -> setupDefaultOperationArgs() fills defaults (no params needed)
```

### Step 3: Operation Dispatch Chain

```
AmDbgWindow::applyOperationMsgProcImp(args)
  -> Checks if args type is DisasmTraceStepInto
  -> Forwards to parent environment:

DebuggerDesktop::applyOperationMsgProcImp(args)
  -> Checks if args type is DisasmTraceStepInto
  -> Calls Debugger::setDebugMode(EVmDebugMode::Break)
  -> Forwards to parent environment:

Debugger::applyOperationMsgProcImp(args)
  -> Handles step-into logic:
     1. Sets VM debug mode to Break
     2. Calls setVmDebugMode(EVmDebugMode::Break) on IVm::VM
  -> Forwards to parent:

IVm::VM::applyOperationMsgProcImp(args)
  -> Routes to emulator thread via:
     IVmClientPlayer::pushOperationMsg(unique_ptr<BaseOpArgs>(args.clone()))
```

### Step 4: Emulator Thread Processing

```
UaeServerThread (SDL thread)
  -> onUaeHandleEvents() polls m_pClientOpsStack
  -> Finds DisasmTraceStepInto operation
  -> Applies to UAE:
     - Sets UAE's debug flag (activates single-step mode)
     - UAE CPU executes one instruction
     - UAE hits debug breakpoint after instruction
  -> Calls m_pVm->fetchStateFromEmu()
```

### Step 5: State Snapshot

```
IVm::VM::fetchStateFromEmu()
  -> For each module in order (MS_MEMORY, MS_CPU, MS_CUSTOM_REGS, ...):
     module->fetch()

UaeVmImp::Cpu::fetch()
  -> Reads UAE regs.regs[] into cached state

UaeVmImp::CustomRegs::fetch()
  -> Bulk-copies UAE custom register array to local regsData[]

UaeVmImp::Copper::fetch()
  -> Reads copper state (cop1lc, cop2lc) from UAE
```

### Step 6: UI Update

```
Main thread (next frame):
  -> DebuggerApp::updateAppPart() called
  -> DebuggerDesktop::drawImGuiMainFrame()
  -> Each AmDbgWindow::drawContentImp() reads VM state:
     DisassemblyView reads IVm::Cpu::getPC() for cursor
     DisassemblyView calls cda::M68CodeDisassembler for disasm lines
     RegistersWnd reads D0-D7, A0-A7, PC, flags
     CustomRegsWnd reads register values from CustomRegs
```

## Data Flow: Screen Rendering

How the Amiga framebuffer gets from the emulator to the SDL texture displayed in the main window.

### Emulator Thread Side

```
UAE/vAmiga renders a frame:
  -> Emulator fills internal pixel buffer (native Amiga resolution)
  -> For UAE: custom.cpp draws to internal framebuffer
  -> For vAmiga: Denise component renders to pixel buffer
  -> Increments SDL_atomic_t m_scrFrameNo
```

### Shared Buffer Protocol

```
Emulator thread:                          Main thread:
                                          QsrMainClientWndApp::renderAppPart()
                                            -> Checks getScrFrameNo() against m_renderedFrameNo
                                            -> If new frame available:
                                               lockDisplayTexBuf(&w, &h, &pixels)
                                                 |
  m_UaeScrTextureMutex.lock()  <-----------+
  Copy amiga buffer to m_pAmigaBuffer       |
  m_UaeScrTextureMutex.unlock()   --------->+
                                                 |
                                               memcpy pixels to SDL_Texture
                                               unlockDisplayTexBuf()
                                               SDL_RenderCopy()
                                               SDL_RenderPresent()
                                               m_renderedFrameNo = frameNo
```

### Screen Buffer Details

- **Resolution**: 754x576 (default, matches PAL Amiga with border)
- **Format**: 32-bit ARGB (`uint32_t*` pixel buffer)
- **Synchronization**: Mutex `m_UaeScrTextureMutex` protects buffer access
- **Frame skipping**: Main thread compares atomic frame counter to avoid redundant copies

For vAmiga, `VAmServerThread::fetchScreenBufferToTexture()` reads from vAmiga's ` Denise` component output, converting to the same 32-bit ARGB format.


## Data Flow: Debug State Fetch

How `fetchStateFromEmu()` pulls the complete machine state from the emulator through the IVm abstraction layer.

### Fetch Sequence

```
Debugger::fetchVmState()
  -> IVm::VM::fetchStateFromEmu()
       |
       +-- 1. IVm::Cpu::fetch()
       |      UAE: reads regs.regs[0..15] (D0-D7, A0-A7)
       |           reads regs.pc, regs.sr
       |      vAmiga: calls m_pVAmiga->cpu.getInfo()
       |           returns CPUInfo{ d[8], a[8], pc0, flags }
       |
       +-- 2. IVm::Memory::fetch() [base: no-op, banks are static]
       |
       +-- 3. IVm::CustomRegs::fetch()
       |      UAE: bulk-copies from UAE's custom_regs[] array
       |            ~200 registers in single memcpy
       |      vAmiga: reads from vAmiga's Agnus/Denise/Paula inspectables
       |
       +-- 4. IVm::Copper::fetch()
       |      UAE: reads cop1lc, cop2lc from UAE copper state
       |      vAmiga: reads from vAmiga's Agnus copper component
       |
       +-- 5. IVm::Blitter::fetch() [base: no-op, state read on demand]
       |
       +-- 6. IVm::Floppy::fetch() [base: no-op, config read on demand]
```

### Memory Bank Population (init time only)

```
UaeVmImp::Memory::init(vm)
  -> Queries UAE's memory map:
     - Reads UAE's addrmap[], allocated_chipmem, etc.
     - Creates MemBank for each region:
       {CHIP, startAddr=0x000000, size=512KB, realAddr=<UAE pointer>}
       {FAST, startAddr=0x200000, size=512KB, realAddr=<UAE pointer>}
       {ROM,  startAddr=0xFC0000, size=512KB, realAddr=<UAE pointer>}
       {CUSTOM, startAddr=0xDFF000, size=512, realAddr=<UAE pointer>}
       ... etc
  -> Stores in m_banks[EMemSrc::MAX_COUNT] array
```

After init, memory reads go directly through the bank's `m_realAddr` pointer -- no virtual dispatch for hot-path reads.

## Singleton Pattern Usage

The codebase makes extensive use of singletons. Here is a complete inventory:

### Global Application Singletons

| Singleton | Location | Access Pattern |
|-----------|----------|----------------|
| `QuaesarApplication` | `qsr_application.h` | `g_pApp` (global raw pointer) + `QuaesarApplication::get()` (static) |
| `qd::Application` | `qd/app/application.h` | `qd::Application::g_pInstance` (inline static) |

### Per-Thread Singletons

| Singleton | Location | Notes |
|-----------|----------|-------|
| `UaeServerThread` | `uae_server_thread.h` | `UaeServerThread::get()` (inline static, set in emulator thread) |
| `VAmServerThread` | `va_server_thread.h` | `VAmServerThread::get()` (inline static, set in emulator thread) |

### Library Singletons

| Singleton | Location | Purpose |
|-----------|----------|---------|
| `M68CodeDisassembler` | `cdaServer.h` | `QD_SINGLETON_DECLARE`, Capstone wrapper + page cache |
| `AppPartServerFactoryListMgr` | `vm_player_selector.cpp` | `QD_SINGLETON_DECLARE`, plugin registry |

### Config Singletons (Meyers Singleton)

| Config | Location | Access |
|--------|----------|--------|
| `CfgQsrStartup` | `qsr_config.h` | `g_cfg_startup` (inline static ref) |
| `CfgQsrMain` | `qsr_main_wnd_client_app.h` | `g_cfg_vm_wnd` (inline static ref) |
| `CfgVmPrefs` | `amDebugger/config.h` | `g_cfg_vm_prefs` (inline static ref) |
| `DbgConfig` | `amDebugger/debuggerConfig.h` | `g_dbg_cfg` (inline static pointer) |
| `amD::UiStyle` | `amDebugger/ui/uiStyle.h` | `UiStyle::get()` + `g_imColors` (inline static ref) |

### Debug Project Options

| Global | Location | Notes |
|--------|----------|-------|
| `amD::DbgProjOptinons g_opt` | `debugger.h` | Trace wait scanlines config |

## EASTL Usage Patterns

The project uses EA's Standard Template Library (EASTL) for performance-critical container operations, wrapped in `qtd::` namespace aliases.

### Container Selection Rationale

| Container | EASTL Type | Use Case | Why EASTL |
|-----------|-----------|----------|-----------|
| `qtd::fixed_vector<T, N>` | `eastl::fixed_vector` | Breakpoints (max 20), register arrays | Stack allocation for small N, no heap |
| `eastl::fixed_set<T, N>` | `eastl::fixed_set` | Breakpoint address lookup | Sorted set with inline storage |
| `eastl::intrusive_list<T>` | `eastl::intrusive_list` | CodeChunk LRU cache | No allocation, node embedded in object |
| `qtd::vector<T>` | `eastl::vector` | Disassembly items, general lists | EASTL's aligned allocator |
| `qtd::string` | `eastl::string` | All string storage | SSO, consistent with EASTL allocators |
| `qtd::array<T, N>` | `eastl::array` | MemBank arrays, register data | Fixed-size, stack-allocated |
| `qtd::span<T>` | `eastl::span` | Memory bank views, string views | Non-owning, zero-cost |
| `qtd::optional<T>` | `std::optional` equivalent | Disassembly view base address | Value semantics, no heap |

### Fixed Containers for Real-Time Use

The most critical EASTL pattern is `fixed_vector` / `fixed_set` with inline storage:

```cpp
// Breakpoints: max 20, never heap-allocates
qtd::fixed_vector<Breakpoint, amD::BREAKPOINTS_MAX, false> mBreakpoints;

// Breakpoint address index: max 20 entries inline
eastl::fixed_set<OneAddrBp, amD::BREAKPOINTS_MAX, false> mOneAddrBps;

// Custom register data: exactly _COUNT_ + 2 entries
eastl::array<uint16_t, CustReg::_COUNT_ + 2> regsData;
```

The `false` template parameter means "do not fall back to heap allocation" -- this guarantees deterministic memory behavior for real-time debugger operations.

### Intrusive List for LRU Cache

The `CodeChunk` LRU cache uses EASTL's intrusive list:

```cpp
struct CodeChunk : public eastl::intrusive_list_node { ... };
eastl::intrusive_list<CodeChunk> m_chunkUseHistory;
```

Benefits:
- Zero allocation for list operations (node pointers embedded in CodeChunk)
- O(1) splice/reorder (just pointer manipulation)
- Cache-friendly when combined with the fixed `m_disasmChunkStorage` array

## Key Abstractions Worth Noting

### ref_ptr\<T\>

A custom reference-counted smart pointer used throughout:

```cpp
ref_ptr<IVm::VM> m_pVm;
ref_ptr<amD::IVmDbgServiceBridge> m_pConnection;
```

Used for:
- VM instances shared between debugger and emulator thread
- Connection bridges with unclear ownership
- Any object that needs shared ownership

`IVm::VM` inherits from `qd::RefCounted` to support this.

### AddrRef

A 32-bit address type used throughout the codebase:

```cpp
using AddrRef = uint32_t;  // (declared in baseTypes.h)
```

Represents an address in the Amiga's 24-bit or 32-bit address space. Used consistently instead of raw `uint32_t` for type safety and documentation purposes.

### qd::InlineString

A small-string-optimized string used in performance-critical paths:

```cpp
qd::InlineString strInsn;   // Copper instruction name (MOVE/WAIT/SKIP)
qd::InlineString comment;   // Copper comment text
```

Avoids heap allocation for short strings like register names and instruction text.

### qd::EFlow

A return type used for operation processing chains:

```cpp
enum class EFlow {
    UNDEF,       // No result / not handled
    CONSUMED,    // Operation was handled, stop propagation
    // ... other flow control values
};
```

Returned by `applyOperationMsgProc()` to indicate whether the operation was consumed or should continue propagating up the chain.

### qd::Var16

A 16-bit variant type (`qd::variant16.h`) used for expression evaluation results:

```cpp
class Var16 {
    uint16_t value;
    // Can display as signed, unsigned, or hex
    // Used by expression evaluator to show register values
};
```

### QD_SINGLETON_DECLARE

Macro for declaring singleton classes:

```cpp
class M68CodeDisassembler {
    QD_SINGLETON_DECLARE(M68CodeDisassembler);
    // Generates: static M68CodeDisassembler& get()
};
```

Provides a Meyers singleton pattern with `get()` static method.

### ECpuFlg

CPU flag enumeration matching the M68K status register:

```cpp
enum ECpuFlg {
    C = 0,  // Carry
    V = 1,  // Overflow
    Z = 2,  // Zero
    N = 3,  // Negative
    X = 4,  // Extended
    I0-I2 = 8-10,  // Interrupt mask
    M = 12, // Master/Interrupt switch
    S = 13, // Supervisor mode
    T0 = 14, T1 = 15,  // Trace bits
    STOPPED,  // CPU is stopped (STOP instruction)
};
```

These map directly to the M68K Status Register (SR) bit layout, with the addition of `STOPPED` for emulator-specific state tracking.

### ECopperStates

Complete enumeration of Copper coprocessor internal states:

```cpp
enum ECopperStates {
    COP_stop, COP_waitforever,     // Idle states
    COP_read1, COP_read2,          // Reading instruction words
    COP_bltwait, COP_bltwait2,     // Waiting for blitter
    COP_wait_in2, COP_skip_in2,    // Processing WAIT/SKIP
    COP_wait1, COP_wait, COP_skip, COP_skip1,  // Beam wait states
    COP_strobe_delay1..5,          // COPJMP strobe delays
    COP_strobe_delay1x, COP_strobe_delay2x,
    COP_strobe_extra, COP_start_delay
};
```

These match the real Amiga copper state machine, used internally by UAE's copper emulation. The debugger can display the current copper state for diagnostic purposes.


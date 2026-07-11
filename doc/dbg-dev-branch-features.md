# dbg-dev Branch: Features and Progress

## Overview

The `dbg-dev` branch is a significant development branch containing ~120 commits ahead of `main`, adding approximately 1.9 million lines of code. It introduces a comprehensive debugger system and dual-emulator architecture.

**Author**: Anton "Dart" Nikolaev

> The goal is to create development and debugging tools, something like an IDE. Started refactoring the debugger (originally UAE-only) to support an abstract VM. This allows future replay recording/playback. Added support for 2 emulators simultaneously (but still needs work). The architecture is client-server style, similar to GDB with GUI.

## Major Features

### 1. Dual Emulator Architecture

**Status**: In Progress

The branch introduces support for two emulation backends with a unified abstraction layer:

| Backend | Integration | Debugger Support | VM Interface | Notes |
|---------|-------------|------------------|--------------|-------|
| WinUAE (uae_lib) | Complete | Working | Fully implemented | Primary backend |
| vAmiga | Partial | Partial | Stubs present | Secondary backend |

**Key files**:
- `src/quasar_app/vm_player_selector.*` -- Engine selector (plugin factory)
- `libs/vAmiga_imp_lib/src/qvAmigaImp/` -- vAmiga integration
- `libs/vAmiga/Core/` -- vAmiga engine (~55k lines)

**vAmiga Integration Status**:
- VM interface implemented but memory accessors are stubbed out (return 0)
- Thread architecture in place (`va_server_thread.*`)
- Floppy drive interface partial
- Custom registers interface partial
- Screen buffer fetch implemented (`fetchScreenBufferToTexture`)
- Message queue processing via `vAmigaMsgQueueProc()`

### 2. Abstract VM Interface (IVm)

**Status**: Mostly Complete

Clean abstraction layer allowing debugger to work with any backend:

```
IVm::VM          -- Main VM container (owns all modules)
IVm::Cpu         -- CPU state (D0-D7, A0-A7, PC, SR flags)
IVm::Memory      -- Memory read/write with bank system
IVm::CustomRegs  -- Amiga chipset registers (fetch/commit)
IVm::Copper      -- Copper coprocessor (cop1lc, cop2lc)
IVm::Blitter     -- Blitter state + screen pixel buffer
IVm::Emu         -- Emulation control (DMA debug, breakpoints)
IVm::Floppy      -- Floppy drive control (4 drives)
```

Each module follows the `IModule` interface with `init(VM*)` and `fetch()` lifecycle methods.

**Implemented for UAE**: Full (all methods delegate to UAE global functions)
**Implemented for vAmiga**: Partial (CPU reads via `CPUInfo`, memory accessors return 0)



### 3. VM Implementation Comparison

Detailed comparison of `UaeVmImp` vs `VAmVmImp` for each IVm module:

#### CPU

| Method | UaeVmImp::Cpu | VAmVmImp::Cpu |
|--------|---------------|---------------|
| `getRegA(i)` | Delegates to UAE `regs.regs[i+8]` | Reads `m_pCpuInfo->a[i]` |
| `getRegD(i)` | Delegates to UAE `regs.regs[i]` | Reads `m_pCpuInfo->d[i]` |
| `getPC()` | Reads UAE PC register | Reads `m_pCpuInfo->pc0` |
| `getFlg(f)` | Reads UAE flag registers | Implemented (partial) |
| `getIntMask()` | Reads UAE interrupt mask | Returns 0 (stub) |
| `fetch()` | N/A (reads on demand) | Calls `m_pVAmiga->cpu.getInfo()` |

#### Memory

| Method | UaeVmImp::Memory | VAmVmImp::Memory |
|--------|------------------|-------------------|
| `init(vm)` | Populates banks from UAE memory map | Partial |
| `getRealAddr(addr)` | Returns UAE memory pointer | Not implemented |
| `getU16(addr, out)` | Reads from UAE memory | Returns `false` (stub) |
| `getU16(addr)` | Reads from UAE memory | Returns 0 (stub) |
| `setU16(addr, v)` | Writes to UAE memory | No-op (stub) |
| `getU32(addr)` | Reads from UAE memory | Returns 0 (stub) |
| `setU32(addr, v)` | Writes to UAE memory | No-op (stub) |

#### CustomRegs

| Method | UaeVmImp::CustomRegs | VAmVmImp::CustomRegs |
|--------|---------------------|----------------------|
| `fetch()` | Bulk-reads from UAE custom registers | Partial |
| `commit()` | Bulk-writes to UAE custom registers | Partial |
| `getRegVal(reg)` | Array lookup (local copy) | Array lookup (local copy) |
| `setRegVal(reg, v)` | Array write (local copy) | Array write (local copy) |

#### Blitter

| Method | UaeVmImp::Blitter | VAmVmImp::Blitter |
|--------|-------------------|-------------------|
| `isBlitterActive()` | Reads UAE blitter state | Implemented |
| `getScreenPixBuf()` | Returns UAE pixel buffer | Implemented |

#### Copper

| Method | UaeVmImp::Copper | VAmVmImp::Copper |
|--------|------------------|-------------------|
| `fetch()` | Reads copper state from UAE | Partial |
| `getCopperAddr(copno)` | Reads cop1lc/cop2lc | Partial |

#### Emu

| Method | UaeVmImp::Emu | VAmVmImp::Emu |
|--------|---------------|---------------|
| `getDebugDmaMode()` | Reads UAE DMA debug mode | Partial |
| `setDebugDmaMode(mode)` | Sets UAE DMA debug mode | Partial |
| `initBreakPoints(bpList)` | Initializes UAE breakpoints | Not implemented |

#### Floppy

| Method | UaeVmImp::Floppy | VAmVmImp::Floppy |
|--------|------------------|-------------------|
| `getEnabled()` | Reads UAE config | Partial |
| `getAdfPath()` | Reads UAE disk path | Partial |
| `getWriteProtect()` | Reads UAE setting | Returns `false` (stub) |

### 4. Debugger Application (amDebugger)

**Status**: Functional for UAE

Complete debugger library with:

| Component | Status | File | Description |
|-----------|--------|------|-------------|
| Disassembly View | Working | `disassembly_wnd.cpp` (8 KB) | M68K disassembly via Capstone with expression-based goto |
| Memory View | Working | `memory_wnd.cpp` (36 KB) | Hex viewer/editor with search, largest debugger window |
| Memory Graph | Working | `memory_graph_wnd.cpp` (8 KB) | Visual memory map rendered as texture |
| Register View | Working | `registers_wnd.cpp` (4 KB) | D0-D7, A0-A7, PC, SR flags |
| Custom Registers | Working | `custom_regs_wnd.cpp` (5 KB) | Amiga chipset registers with bitfield display |
| Copper View | Working | `copper_wnd.cpp` (5.5 KB) | Copper list disassembly (MOVE/WAIT/SKIP) |
| Blitter View | Working | `blitter_wnd.cpp` (6.5 KB) | Blitter state display |
| Color Palette | Working | `colors_wnd.cpp` (1.8 KB) | 32-color palette display |
| Screen Capture | Working | `screen_wnd.cpp` (3.7 KB) | Emulator framebuffer snapshot |
| Console | Working | `console_wnd.cpp` (2.9 KB) | Debug console (WinUAE-style commands) |
| Breakpoints | Working | `debugger.h` | Up to 20 breakpoints, sorted by address |

### 5. Client-Server Architecture

**Status**: Foundation Complete

Infrastructure for GDB-like remote debugging:

| Component | File | Description |
|-----------|------|-------------|
| `IVmDbgServiceBridge` | `dbgConnection.h` | Connection interface (`getClientVm`/`getServerVm`) |
| `IVmConnectionBuilder` | `debuggerServer.h` | Connection factory (also `IOperationEnvironment`) |
| `IVmConnectionsManager` | `debuggerWndApp.h` | Multi-connection manager interface |
| `IAppPartServerProviderFactory` | `vm_player_selector.h` | Plugin factory for backends |
| `RegOnLoadAppPartServerFactory` | `vm_player_selector.cpp` | Static auto-registration |

**Implemented Connections** (shared memory, same-process):
- `UaeSharedConnectionImpl` (`src/quasar_app/uae_imp/uae_server_app_part.cpp`) -- UAE bridge; same VM for client & server
  - Created via `UaeServerProviderFactory::createVmDebuggerConnection()` (id=`"uae"`)
- `VAmSharedConnectionImpl` (`libs/vAmiga_imp_lib/src/qvAmigaImp/va_server_app_part.cpp`) -- vAmiga bridge; same VM for client & server
  - Created via `VAmigaServerProviderFactory::createVmDebuggerConnection()` (id=`"vamiga"`)

**Declared but not implemented** (stubs in `dbgConnection.h`):
- `create_uae_shared_connection(const char* name)` -- Intended standalone factory function
- `create_dummy_connection()` -- Intended test placeholder

**Commented-out code** (both backends have a `*ConnImpl` struct inheriting `IVmConnectionBuilder`, currently unused):

**Not yet implemented**:
- Network remote debugging
- Replay recording/playback (planned)

### 6. Debugger Operations Complete Registry

All operations defined in `debuggerOps.h` with their keyboard shortcuts:

| Operation ID | Name | Shortcut | Description |
|-------------|------|----------|-------------|
| `DisasmTraceStepInto` | Step Into | F11 | Execute one instruction, follow subroutines |
| `DisasmTraceStepOut` | Step Out | F10 | Run until current subroutine returns |
| `DebugTraceStart` | Debug Trace Mode | F12 | Enter debug/break mode |
| `DebugTraceContinue` | Continue | F5 | Resume normal execution |
| `DebugWaitScanLines` | Wait N scanlines | -- | Break after N scanlines |
| `DisasmToggleBreakpoint` | Disasm breakpoint | F9 | Toggle breakpoint at disassembly cursor |
| `CopperToggleBreakpoint` | Copper breakpoint | Shift+F9 | Toggle copper breakpoint |
| `CopperTraceStep` | Copper Trace Step | Shift+F11 | Single-step copper execution |
| `ToggleTurboEmulation` | Turbo Emulation | NumLock | Toggle maximum speed |
| `ResetAmigaEmu` | Reset Amiga | -- | Hard reset the emulated machine |
| `AlwaysOnTopEmu` | Always on Top | Ctrl+T | Pin emulator window above debugger |
| `ShowDebuggerWnd` | Show Debugger | Shift+F12 | Toggle debugger window visibility |
| `ShowUaeOptionsWnd` | Show Options | Ctrl+P | Open UAE options dialog |

Shortcuts are defined via X-macro pattern in `shortcutsList.h`, generating both the enum and the initialization array simultaneously.


### 7. Custom Registers Coverage

The custom registers database (`customRegsList.h`) is a 31 KB X-macro file that defines every Amiga hardware register:

```cpp
#define QDB_CUSTOM_REGS_LIST(REG)                                     \
    REG(register_id, hardware_address, special_flags, bitmasks, "description") \
    REG(BLTCON0,  0x040, ..., ..., "Blitter control register 0")     \
    REG(DMACON,   0x096, ..., ..., "DMA control")                    \
    REG(BPLCON0,  0x100, ..., ..., "Bitplane control register 0")    \
    // ... hundreds of entries
```

Each entry provides:
- **Enum identifier** -- Becomes a `CustReg::Type` enum value
- **Hardware address** -- Physical register address (e.g., `0xDFF040`)
- **Special flags** -- Handling hints for the UI
- **Bitmasks** -- Three mask values for read/write/display
- **Description** -- Human-readable register name

The `CustReg` struct provides:
- `toString()` -- Returns register name as string_view
- `getRegByAddr()` -- Static lookup of register by hardware address
- `getFlagDesc()` -- Returns bitfield description (e.g., DMACON flags)

**Flag descriptor examples**:

`DMAC` (DMACON register):
- AUD0EN-AUD3EN (bits 0-3) -- Audio channel DMA enable
- DSKEN (bit 4) -- Disk DMA enable
- BLTEN (bit 6) -- Blitter DMA enable
- COPEN (bit 7) -- Copper DMA enable
- BPLEN (bit 8) -- Bitplane DMA enable
- DMAEN (bit 9) -- Master DMA enable
- SETCLR (bit 15) -- Set/clear bit for write operations

`BC0F` (BLTCON0 register):
- ABC through NANBNC (bits 0-7) -- Minterm selection for blitter logic op
- DEST/SRCC/SRCB/SRCA (bits 8-11) -- Channel DMA enable

### 8. UI Framework (qd library)

**Status**: Complete

Custom application framework providing:
- `qd::Application` -- SDL-based application lifecycle
- `qd::ApplicationPart` -- Modular app components with update/render phases
- `qd::OperationsRegistry` -- Command/operation system with type-safe args
- `qd::ShortcutsMgr` -- Keyboard shortcut management
- `qd::QImGuiContext` -- ImGui integration with SDL2
- Type reflection system (`TS_REFLECT_CLASS` macros)
- Node hierarchy for UI tree management
- `qd::ImColorsTab` -- Named color table for theming

### 9. Expression Parser

**Status**: Complete

`libs/exprParser/` -- Expression evaluation for debugger commands. Based on Drunk Fly's MIT-licensed parser.

Components:
- `lexer.*` -- Tokenizer (operators, numbers, identifiers)
- `parser_oop.*` -- Recursive descent parser producing AST
- `resolve_oop.h` -- `ExprResolver` / `ExprEvaluator` interfaces
- `common.h` -- Value types (`ExprValue` = int), error handling

Bridging via `ExprValStr` (`exprValue.h`):
- Stores string input + parsed `Expr*` AST
- Re-parses on string change
- Evaluates with VM context to produce `qd::Var16` result

Supports: hex (`$2000`, `0x2000`), registers (`D0`, `A7`, `PC`), arithmetic, parentheses.

### 10. Options Dialog Architecture

**Status**: Working for UAE

The options dialog (`uae_options_wnd.*`) implements a category-based settings browser:

```
BaseOptionsDlg (qd::UiDialog)
  +-- Category tree (left panel)
  |     +-- Root
  |     |     +-- Quickstart
  |     |     +-- Sound
  |     |     +-- Hardware
  |     |     +-- CPU
  |     |     +-- Host
  |     |     +-- Floppy drives
  |     |     +-- Advanced
  +-- Option content (right panel)
        +-- UOption with draw callbacks
```

Key types:
- `EOptionCat` -- Category enum (Quickstart, Sound, Hardware, CPU, Host, Floppy, Advanced)
- `UCategory` -- Tree node with children and options
- `UOption` -- Individual option with `TDrawOptionCb` (fixed_function callback)
- `BaseOptionsDlg` -- Generic tree browser
- `UaeOptionsDlg` -- Populates with UAE-specific options

The `fixed_function<2*sizeof(void*), void(OptionDrawCtx*)>` pattern avoids heap allocation for option draw callbacks.

### 11. Bartman Profile Viewer

**Status**: Placeholder

`src/quasar_app/bartman_profile_viewer/` contains a skeleton `BarmanProfileViewerAppPart` class. The `profile_viewer_emu.h` is empty. This is intended for future Amiga profiling integration (cycle counting, function timing) using Bartman's profiling format.

### 12. Windows Build Improvements

**Status**: Complete

- CMake presets for Windows (Visual Studio 2019/2022)
- Clang-format enforcement via CI (`format_check.yml`)
- Static CRT linking option (`/MT`, `/MTd`)
- `bin/win/cmake/` ships CMake 3.28 and clang-format for reproducibility
- `bin/win/clang-format.exe` for consistent code formatting
- Edit-and-continue support (`add_option_edit_and_continue`)


## Key Architectural Patterns

### Plugin Factory Pattern

Backends register themselves at load time via static initialization:

```cpp
// Each backend declares a factory:
class UaeAppPartServerProviderFactory : public IAppPartServerProviderFactory {
    std::string id = "uae";
    std::string guiName = "WinUAE";
    // createServerAppPart(), createVmDebuggerConnection()
};

// Static registration happens before main():
static qsr::plugin_api::RegOnLoadAppPartServerFactory s_regUae(
    new UaeAppPartServerProviderFactory());
```

This allows adding new emulator backends without modifying the core application -- just link a new library that registers its factory.

### Type Reflection Macros

The codebase uses a custom RTTI-like system extensively:

```cpp
// Class declaration:
TS_BEGIN_REFLECT_CLASS(MyClass, ParentClass);
    TS_ATTRIBUTE(qd::tsAttr::Name("My Class"));
    TS_ATTRIBUTE(qd::tsAttr::CustomClassId32(42));
    TS_ATTRIBUTE(qd::tsAttr::CreateClassCb(&myFactory));
TS_END();

// Usage:
const qd::TypeInfo& type = MyClass::getStaticTypeInfo();
bool isDerived = type.isDerivedFrom(ParentClass::getStaticTypeInfo());
```

This enables:
- Runtime type checking without C++ RTTI
- Generic factory creation
- Type-indexed registries
- Serialization-ready metadata

### X-Macro Pattern

X-macros are used throughout to generate enum + data tables from a single definition:

- `SHORTCUT_LIST` in `shortcutsList.h` -- generates enum + shortcut array
- `QDB_CUSTOM_REGS_LIST` in `customRegsList.h` -- generates enum + register database
- `OPTIONS_LIST` in `uae_options_wnd.h` -- generates option category enum + names

### Configuration Singleton Pattern

```cpp
struct CfgQsrStartup : public CfgBase {
    CFG_DECLARE(CfgQsrStartup);
    std::string kickRomPath;
    std::string input;
};
inline static CfgQsrStartup& g_cfg_startup = CfgQsrStartup::get();
```

The `CFG_DECLARE` macro creates a `get()` static method returning a Meyers singleton. The `inline static` global reference provides zero-cost access.

### Operation Chain Pattern

Operations bubble up through the `IOperationEnvironment` hierarchy:

```
AmDbgWindow (window-specific handling)
  -> DebuggerDesktop (global UI handling)
    -> Debugger (debug logic)
      -> IVm::VM (VM-level handling)
        -> Emulator thread (execution control)
```

Each level can handle, modify, or forward the operation. `setupDefaultOperationArgs()` flows downward to fill in defaults (e.g., current PC address for a breakpoint operation).

## Commits Summary

Key commit categories:
- **vAmiga Integration**: ~10 commits (7e41fce7, d125cc8f, 81fc8afb, 4c2187dd)
- **Debugger Development**: ~40 commits
- **Architecture Refactoring**: ~30 commits (namespace, folder structure)
- **Build System**: ~15 commits
- **Bug Fixes**: ~25 commits

## What's Not Ready

### vAmiga Backend

```cpp
// va_vm_imp.cpp - Many stubs like:
virtual uint16_t getU16(AddrRef addr) override {
    return 0;  // Not implemented
}
virtual uint32_t getU32(AddrRef addr) override {
    return 0;  // Not implemented
}
virtual void setU16(AddrRef addr, uint16_t v) override {
    // Not implemented
}
```

The vAmiga CPU reads work via `vamiga::VAmiga::cpu.getInfo()` which returns a `CPUInfo` struct with register values. However, memory access, custom register commits, and breakpoint initialization are still stubbed.

Missing vAmiga pieces:
- `getRealAddr()` -- needs hook into vAmiga's memory subsystem
- `initBreakPoints()` -- vAmiga has `GuardList` infrastructure but not connected
- `commit()` for CustomRegs -- writing back register values to vAmiga
- Floppy write-protect, enabled state management

### Replay System
- Mentioned in design goals but no implementation
- Would allow recording and playback of emulation state
- The IVm snapshot-based design is partially intended to support this

### Remote Debugging
- Client-server infrastructure exists (`IVmDbgServiceBridge`)
- No network transport implemented yet
- The abstraction is designed to support out-of-process debugging

### Bartman Profile Viewer
- Skeleton `ApplicationPart` only
- No profiling data parsing or display

## Recommended Next Steps

1. **Complete vAmiga Memory Interface**
   - Implement `getRealAddr()` by hooking into vAmiga's `Memory` component
   - Implement `getU16()`, `getU32()`, `setU16()`, `setU32()` using vAmiga's memory API
   - Populate `MemBank` array from vAmiga's memory configuration

2. **vAmiga Custom Registers**
   - Map vAmiga chipset state (`Agnis`, `Denise`, `Paula`) to `IVm::CustomRegs`
   - Implement `fetch()` to read from vAmiga's inspectable properties
   - Implement `commit()` to write register values back

3. **vAmiga Breakpoints**
   - Implement `initBreakPoints()` using vAmiga's `GuardList` infrastructure
   - Connect `VAmServerThread` breakpoint handling to debugger operations

4. **Testing**
   - Side-by-side comparison between UAE and vAmiga
   - Verify debugger accuracy (register values, memory contents, disassembly)
   - Test dual-backend switching at runtime

## File Change Statistics

```
Total changes: ~1.9 million lines added

Key additions by library:
  libs/vAmiga/              -- vAmiga emulator core (~55k lines)
    Core/Components/        -- Agnus, CPU, Denise, Paula, CIA, Memory, Zorro
    Core/Infrastructure/    -- Threading, config, serialization (~50 files)
    Core/FileSystems/       -- OFS/FFS filesystem drivers (~22 files)
    Core/Media/             -- Disk, ROM handling (~15 files)
  libs/vAmiga_imp_lib/      -- vAmiga integration layer (~6 files, 25k)
  libs/amDebugger/          -- Complete debugger library (~21 files)
    vm/                     -- VM abstraction interfaces (8 files)
    window/                 -- Debug UI windows (18 files)
    codeAnalyzer/           -- Disassembly engine (6 files)
    ui/                     -- Debugger desktop framework (6 files)
  libs/qd/                  -- Application framework (~130 files)
    app/                    -- Application, parts, modules (10 files)
    base/                   -- Base types, color, variant (23 files)
    typeSystem/             -- Type reflection system (13 files)
    qui/                    -- UI operations, shortcuts (13 files)
    stl/                    -- EASTL wrappers (26 files)
    thread/                 -- Threading primitives (5 files)
  libs/exprParser/          -- Expression parser (~7 files)
  libs/uae_lib/             -- WinUAE core (~1.4M lines, ported)
  src/quasar_app/           -- Main application (~20 files)
  src/uae_lib_imp/          -- UAE platform hooks (~16 files)
```

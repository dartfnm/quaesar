# Quaesar Architecture Documentation

## Overview

Quaesar is a cross-platform Amiga emulator targeting demosceners and developers. The project is designed around a **client-server architecture** with an abstraction layer that allows plugging in different emulation backends (UAE, vAmiga) while sharing a unified debugger UI.

**Author**: Anton "Dart" Nikolaev
**Goal**: Create development and debugging tools for the Amiga, functioning like an IDE -- with debugger, memory inspector, disassembler, and custom register views.

## High-Level Architecture

```
+---------------------------------------------------------------------+
|                      QuaesarApplication                             |
|  (qsr::QuaesarApplication - main app orchestrator)                  |
+---------------------------+-----------------------------------------+
|   QsrMainClientWndApp     |            DebuggerApp                  |
|   (Emulator Display)      |         (Debugger Windows)              |
+---------------------------+-----------------------------------------+
|                    VmPlayersSelector                                |
|           (VM Provider Factory & Selection)                         |
+---------------------------------------------------------------------+
|                 IVm::VM Abstraction Layer                           |
|    +------------------+                    +------------------+     |
|    |   UaeVmImp       |                    |   VAmVmImp       |     |
|    | (WinUAE Core)    |                    | (vAmiga Core)    |     |
|    +------------------+                    +------------------+     |
+---------------------------------------------------------------------+
|                     Emulation Engines                               |
|    +----------------------+      +---------------------+            |
|    |       uae_lib        |      |       vAmiga        |            |
|    |  (WinUAE ported)     |      |  (Modern C++20)     |            |
|    |  ~1.4M lines C/C++   |      |                     |            |
|    +----------------------+      +---------------------+            |
+---------------------------------------------------------------------+
```

## Application Lifecycle

The application starts in `src/quasar_app/qsr_main.cpp` and follows this sequence:

1. **CLI Parsing** -- Uses CLI11 to parse command-line arguments:
   - `input` -- executable or disk image (ADF, DMS)
   - `-k, --kickstart` -- path to Kickstart ROM
   - `--serial_port` -- serial port path (e.g. `/tmp/virtual-serial-port`)
   - `-s` -- pass-through WinUAE-style configuration commands

2. **SDL Initialization** -- `SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)`

3. **Application Construction** -- `QuaesarApplication` is created and `onConstruct()` is called, which initializes the `AppPartsManager` and `ModuleManager`.

4. **NFD Init** -- Native File Dialog library initialized for ADF file selection.

5. **`QuaesarApplication::initialize()`** -- This is where the real setup happens:
   - Creates `QsrMainClientWndApp` (the emulator display window)
   - Creates `DebuggerApp` (the debugger window)
   - Activates a VM player via `VmPlayersSelector::activateVmPlayerByIdStr()`
   - The VM player is selected by config string (default: `"uae"`, alternative: `"vamiga"`)

6. **Main Loop** -- `qd::Application::doMainLoop()` runs frame-by-frame:
   - `onFrameUpdate(dt, time)` -- updates all `ApplicationPart`s
   - `onFrameRender()` -- renders all renderable parts
   - SDL event processing via `onSdlEventProc()`

7. **Shutdown** -- `destroy()` cleans up in reverse order, then NFD and SDL quit.


## Directory Structure

```
quaesar-ng/
+-- src/
|   +-- quasar_app/           # Main application
|   |   +-- qsr_main.cpp          # Entry point (SDL_main)
|   |   +-- qsr_application.*     # Core app class
|   |   +-- qsr_main_wnd_*        # Main emulator window + SDL renderer
|   |   +-- qsr_config.*          # Startup configuration (CLI args)
|   |   +-- qsr_operations.*      # Quaesar-specific operations (stub)
|   |   +-- qsr_debug.*           # Debug utilities
|   |   +-- vm_player_selector.*  # VM engine selector (plugin factory)
|   |   +-- uae_imp/              # UAE engine integration
|   |   |   +-- uae_server_thread.*   # UAE execution thread
|   |   |   +-- uae_server_app_part.* # App part wrapping UAE server
|   |   |   +-- uae_vm_imp.*          # IVm implementation for UAE
|   |   |   +-- qsr_imp_proxy.*       # Import proxy for UAE symbols
|   |   +-- ui/                   # UI components
|   |   |   +-- uae_wnd_desktop.*     # Emulator window desktop (ImGui)
|   |   |   +-- uae_options_wnd.*     # Options dialog (categories/tabs)
|   |   +-- bartman_profile_viewer/   # Bartman profile viewer (placeholder)
|   +-- uae_lib_imp/            # UAE platform-specific code
|       +-- machdep/               # M68K, memory access, timing
|       +-- sounddep/              # Sound output stubs
|       +-- threaddep/             # Threading primitives
|       +-- sysconfig.h            # UAE build configuration
|       +-- target.h               # Platform target definitions
|       +-- gfx.cpp / input.cpp / gui.cpp  # Platform hooks
|
+-- libs/
|   +-- uae_lib/               # WinUAE core (ported, ~1.4M lines)
|   +-- vAmiga/                # vAmiga emulator core
|   |   +-- Core/
|   |       +-- Components/        # Agnus, Paula, Denise, CPU, CIA, Memory, Zorro
|   |       +-- Infrastructure/    # Threading, config, errors, serialization
|   |       +-- Media/             # Disk, ROM handling
|   |       +-- FileSystems/       # ADF, filesystem drivers
|   |       +-- Peripherals/       # Joystick, mouse
|   |       +-- Ports/             # Serial, parallel
|   |       +-- Utilities/         # Checksums, compression
|   |       +-- ThirdParty/        # Embedded third-party code
|   +-- vAmiga_imp_lib/        # vAmiga integration layer
|   |   +-- src/qvAmigaImp/
|   |       +-- va_server_thread.*    # vAmiga execution thread
|   |       +-- va_server_app_part.*  # App part wrapping vAmiga server
|   |       +-- va_vm_imp.*          # IVm implementation for vAmiga
|   +-- amDebugger/             # Amiga debugger library
|   |   +-- src/amDebugger/
|   |       +-- debugger.*          # Debugger engine (client)
|   |       +-- debuggerWndApp.*    # Debugger window application
|   |       +-- debuggerServer.*    # Server-side connection builder
|   |       +-- dbgConnection.*     # Service bridge interface
|   |       +-- debuggerOps.*       # Operation definitions + shortcuts
|   |       +-- exprValue.*         # Expression evaluator bridge
|   |       +-- shortcutsList.h     # Keyboard shortcut registry
|   |       +-- config.h            # VM model config (A500/A1200, RAM sizes)
|   |       +-- vm/                 # VM abstraction interfaces
|   |       |   +-- vmInterface.*      # IVm::VM, IModule base
|   |       |   +-- memory.*           # MemBank, EMemSrc, EReg, ECpuFlg
|   |       |   +-- customRegs.*       # CustReg enum, flag descriptors
|   |       |   +-- customRegsList.h   # X-macro: all Amiga register definitions
|   |       |   +-- emuDefs.h          # EVmDebugMode, copper states, enums
|   |       +-- window/             # Debugger UI windows
|   |       |   +-- disassembly_wnd.*  # M68K disassembly view
|   |       |   +-- memory_wnd.*       # Hex memory viewer/editor
|   |       |   +-- memory_graph_wnd.* # Visual memory map (texture)
|   |       |   +-- registers_wnd.*    # CPU register view (D0-D7, A0-A7, PC, SR)
|   |       |   +-- custom_regs_wnd.*  # Amiga chipset register view
|   |       |   +-- copper_wnd.*       # Copper list disassembly
|   |       |   +-- blitter_wnd.*      # Blitter state display
|   |       |   +-- colors_wnd.*       # 32-color palette display
|   |       |   +-- screen_wnd.*       # Emulator framebuffer capture
|   |       |   +-- console_wnd.*      # Debug console (WinUAE commands)
|   |       +-- codeAnalyzer/       # Code analysis engine
|   |       |   +-- cdaServer.*        # M68CodeDisassembler + page cache
|   |       |   +-- cdaTypes.*         # Item/CodeItem/DataInfo types
|   |       |   +-- quadTreeAddrMap.*  # Quad-tree address -> page lookup
|   |       |   +-- copperDisasm.h     # Copper instruction decoder
|   |       +-- ui/                  # Debugger UI framework
|   |           +-- debuggerDesktop.*  # Main debugger desktop (menubar, toolbar)
|   |           +-- uiView.*           # AmDbgWindow base class
|   |           +-- uiStyle.*          # Color theme system
|   |           +-- uiDefs.h           # Window ID enums
|   +-- qd/                    # Custom application framework
|   |   +-- app/                  # Application, ApplicationPart, ModuleManager
|   |   +-- base/                 # Base types: EFlow, Color, Variant16, Guid, etc.
|   |   +-- typeSystem/           # RTTI-like type reflection (TS_REFLECT_CLASS)
|   |   +-- qui/                  # UI operation system, shortcuts, controls
|   |   +-- imGui/                # ImGui integration + styling
|   |   +-- thread/               # Mutex, ThreadEvent, thread abstraction
|   |   +-- stl/                  # EASTL wrappers: string, vector, unique_ptr, etc.
|   |   +-- enum/                 # Enum helpers, flags, to_string
|   |   +-- math/                 # Point, Size, Rect
|   |   +-- mem/                  # Memory utilities
|   |   +-- file/                 # File I/O
|   |   +-- log/                  # Logging
|   |   +-- debug/                # Assert macros
|   |   +-- node/                 # Node base class (tree hierarchy)
|   |   +-- txtNodes/             # Text node helpers
|   +-- exprParser/             # Expression parser (based on Drunk Fly's parser)
|       +-- parser/
|           +-- common.h            # ExprValue, ExprError, callback types
|           +-- lexer.*             # Tokenizer
|           +-- parser_oop.*        # Recursive descent parser
|           +-- resolve_oop.h       # ExprResolver, ExprEvaluator interfaces
|
+-- external/
|   +-- dear_imgui/            # Dear ImGui UI library
|   +-- sdl2/                  # SDL2 (prebuilt for Windows, system for macOS/Linux)
|   +-- capstone/              # Capstone disassembly engine (M68K support)
|   +-- EASTL/                 # EA Standard Template Library
|   +-- zlib/                  # Zlib compression
|   +-- ADFlib/                # Amiga Disk File library
|   +-- nativefiledialog-extended/  # Native file dialog
|   +-- cli11/                 # CLI11 command-line parser
|   +-- nlohmann/              # JSON library
|
+-- data/static/               # Source Code Pro font files (TTF)
+-- resources/                 # Default layout INI
+-- scripts/cmake/             # Build helpers (bin2c, compile options, format)
+-- bin/                       # Tools (cmake.exe, clang-format.exe for Windows)
```


## Emulation Engines

### 1. WinUAE (libs/uae_lib)

- **Origin**: Port of [WinUAE](https://github.com/tonioni/WinUAE), 30+ years of development
- **Size**: ~1.4 million lines of C/C++ code (187 files at top level, plus subdirectories)
- **Accuracy**: Most accurate Amiga emulation available
- **Features**: Full chipset emulation, cycle-accurate modes, JIT compiler
- **Integration**: `UaeServerThread` runs emulation in separate thread
- **Status**: Primary, fully functional backend

Key source files in `libs/uae_lib/`:

| File | Size | Description |
|------|------|-------------|
| `cpuemu_11.cpp` | 4219 KB | CPU emulation (68000) |
| `cpuemu_13.cpp` | 3672 KB | CPU emulation (68010) |
| `cpuemu_40.cpp` | 2004 KB | CPU emulation (68040) |
| `cpustbl.cpp` | 3882 KB | CPU instruction table |
| `custom.cpp` | 426 KB | Custom chipset (Agnus, Denise, Paula) |
| `newcpu.cpp` | 255 KB | CPU core logic |
| `cfgfile.cpp` | 311 KB | Configuration file parser |
| `filesys.cpp` | 285 KB | Filesystem emulation |
| `inputdevice.cpp` | 280 KB | Input device handling |
| `linetoscr.cpp` | 662 KB | Scanline-to-screen rendering |
| `memory.cpp` | 114 KB | Memory subsystem |
| `blitter.cpp` | 62 KB | Blitter coprocessor |
| `audio.cpp` | 77 KB | Paula audio |
| `debug.cpp` | 208 KB | Built-in debugger (WinUAE native) |
| `drawing.cpp` | 164 KB | Video rendering pipeline |
| `disk.cpp` | 162 KB | Floppy disk emulation |
| `aros.rom.cpp` | 3278 KB | AROS Kickstart replacement ROM |

The UAE codebase is massive legacy C/C++ with extensive use of macros, global state, and preprocessor conditionals. The Quaesar port provides platform-specific implementations in `src/uae_lib_imp/` that replace the original Windows/SDL platform layer.

### 2. vAmiga (libs/vAmiga)

- **Origin**: Modern Amiga emulator by Dirk W. Hoffmann
- **Language**: Modern C++20, modular component-based architecture
- **Accuracy**: Some timing inaccuracies compared to WinUAE
- **Structure**: Clean component hierarchy under `Core/`
- **Integration**: `VAmServerThread` runs emulation in separate thread
- **Status**: Secondary backend, integration in progress

Component layout under `libs/vAmiga/Core/`:

| Directory | Contents |
|-----------|----------|
| `Components/Agnus/` | Agnus chip (DMA, copper, blitter scheduling) |
| `Components/CPU/` | M68K CPU emulation |
| `Components/Denise/` | Denise chip (video, bitplanes, sprites) |
| `Components/Paula/` | Paula chip (audio, floppy, interrupts) |
| `Components/CIA/` | Complex Interface Adapter (IO, timers) |
| `Components/Memory/` | Memory management, bank switching |
| `Components/Zorro/` | Zorro bus expansion boards |
| `Components/RTC/` | Real-time clock |
| `Infrastructure/` | Threading, config, serialization, error handling, inspectable properties |
| `Media/` | Disk images, ROM files |
| `FileSystems/` | OFS/FFS filesystem implementations |
| `Peripherals/` | Joystick, mouse |
| `Ports/` | Serial, parallel port emulation |
| `Utilities/` | CRC, checksums, compression helpers |

The vAmiga core exposes a clean C++ API via `VAmiga.h` and `Amiga.h`, with a message queue (`MsgQueue`) for thread-safe communication and an inspection system (`Inspectable`, `CPUInfo`) for reading emulator state.



## VM Abstraction Layer (IVm)

The `IVm` namespace in `libs/amDebugger/src/amDebugger/vm/vmInterface.h` provides a unified interface for both emulation backends. This is the central architectural abstraction that makes the dual-engine design possible.

### Design Philosophy

The IVm layer treats the VM as a **snapshot of the machine at any given moment**. Each backend implements the same interfaces, and the debugger reads state through these interfaces without knowing which engine is running underneath.

### Module System

All VM subsystems inherit from `IVm::IModule`:

```cpp
class IModule {
public:
    virtual void init(IVm::VM*) {}   // Called once at startup
    virtual void fetch() {}           // Called each frame to pull state from emulator
};
```

The `VM` class owns instances of all modules and orchestrates the `init()` / `fetch()` lifecycle:

```cpp
class VM : public qd::RefCounted, public qd::IOperationEnvironment {
    IVm::Memory*    mem     = nullptr;
    IVm::Cpu*       cpu     = nullptr;
    IVm::CustomRegs* custom = nullptr;
    IVm::Copper*    copper  = nullptr;
    IVm::Blitter*   blitter = nullptr;
    IVm::Floppy*    floppy0..3 = nullptr;
    IVm::Emu*       emu     = nullptr;
};
```

Module state ordering is defined by `EModuleState`:

| Value | Module | Description |
|-------|--------|-------------|
| `MS_MEMORY` | Memory | RAM/ROM access |
| `MS_CPU` | Cpu | Registers (D0-D7, A0-A7, PC, SR) |
| `MS_CUSTOM_REGS` | CustomRegs | Amiga chipset registers |
| `MS_COPPER` | Copper | Copper coprocessor state |
| `MS_BLITTER` | Blitter | Blitter state |
| `MS_FLOPPY` | Floppy | Floppy drive configuration |

### Interface Reference

#### IVm::Memory

```cpp
class Memory : public IModule {
    qtd::array<IVm::MemBank, EMemSrc::MAX_COUNT> m_banks;
    virtual uint8_t*  getRealAddr(AddrRef ptr) = 0;
    virtual bool      getU16(AddrRef addr, uint16_t* out) = 0;
    virtual uint16_t  getU16(AddrRef addr) = 0;
    virtual void      setU16(AddrRef addr, uint16_t v) = 0;
    virtual uint32_t  getU32(AddrRef addr) = 0;
    virtual void      setU32(AddrRef addr, uint32_t v) = 0;
};
```

Memory is organized into banks (see Memory Subsystem Architecture below). The `getRealAddr()` method returns a direct pointer into emulator memory for bulk reads.

#### IVm::Cpu

```cpp
class Cpu : public IModule {
    virtual uint32_t getRegA(int i) const = 0;   // Address registers A0-A7
    virtual uint32_t getRegD(int i) const = 0;   // Data registers D0-D7
    virtual AddrRef  getPC() const = 0;           // Program Counter
    virtual bool     getFlg(ECpuFlg_ f) const = 0; // CPU flags (Z/C/V/N/X)
    virtual int      getIntMask() const = 0;       // Interrupt priority mask
};
```

#### IVm::CustomRegs

```cpp
class CustomRegs : public IModule {
    virtual void     fetch() override = 0;         // Pull all registers from emulator
    virtual void     commit() = 0;                  // Write modified registers back
    virtual uint16_t getRegVal(CustReg reg) = 0;   // Read by register enum
    virtual void     setRegVal(CustReg reg, uint16_t new_val) = 0;
};
```

Custom registers are defined by the X-macro `QDB_CUSTOM_REGS_LIST` in `customRegsList.h` (31 KB, covering every Amiga hardware register with address, bitmask, and description).

#### IVm::Copper

```cpp
class Copper : public IModule {
    virtual void     fetch() override = 0;
    virtual AddrRef  getCopperAddr(ECopperAddr_ copno) = 0; // cop1lc or cop2lc
};
```

#### IVm::Blitter

```cpp
class Blitter : public IModule {
    virtual bool   isBlitterActive() const = 0;
    virtual void*  getScreenPixBuf(int mon_id, int* w, int* h, int* pitch) = 0;
};
```

#### IVm::Emu

```cpp
class Emu : public IModule {
    virtual int  getDebugDmaMode() { return 0; }
    virtual void setDebugDmaMode(int mode) {}
    virtual void getScreenSize(int* w, int* h) const;
    virtual void initBreakPoints(amD::BreakpointsSortedList& bpList) {}
};
```

#### IVm::Floppy

```cpp
class Floppy : public IModule {
    int m_nFloppy = 0;                          // Drive number (0-3)
    virtual bool      getEnabled() = 0;
    virtual void      setEnabled(bool v) = 0;
    virtual bool      getWriteProtect() = 0;
    virtual void      setWriteProtect(bool v) = 0;
    virtual qtd::string getAdfPath() = 0;
    virtual void      setAdfPath(const qtd::string& v) = 0;
};
```

### Factory Pattern

VM instances are created through a type-erased factory:

```cpp
void* impFactoryCreateInstance(const std::type_info& type);
template<typename T> T* createByFactory_() {
    return static_cast<T*>(impFactoryCreateInstance(typeid(T)));
}
```

Each backend library provides its own implementation of `impFactoryCreateInstance`, allowing the debugger to create the correct VM implementation without knowing which backend is active.

### Backend Implementations

| Backend | Class | Location |
|---------|-------|----------|
| UAE | `IVm::imp::UaeVmImp` | `src/quasar_app/uae_imp/uae_vm_imp.*` |
| vAmiga | `IVm::imp::VAmVmImp` | `libs/vAmiga_imp_lib/src/qvAmigaImp/va_vm_imp.*` |



## Client-Server Architecture

The design follows a client-server model similar to GDB. The debugger UI is a client that communicates with an emulator server through a bridge interface.

```
+------------------+         +----------------------------+
|  Debugger UI     | <-----> |  IVmDbgServiceBridge       |
|  (Client)        |         |  (Connection Interface)    |
+------------------+         +-------------+--------------+
                                           |
                               +-----------+-----------+
                               |                       |
                       +-------v--------+      +-------v--------+
                       | UaeServerThread |      |VAmServerThread |
                       |  (UAE Server)   |      |(vAmiga Server) |
                       +-----------------+      +----------------+
```

### Connection Interfaces

**`IVmDbgServiceBridge`** (`dbgConnection.h`) -- The core connection abstraction:

```cpp
class IVmDbgServiceBridge : public qd::RefCounted {
    virtual ref_ptr<IVm::VM> getClientVm() = 0;  // VM for debugger reads
    virtual ref_ptr<IVm::VM> getServerVm() = 0;  // VM for emulator writes
};
```

Both backends follow the same pattern. Each provides a concrete `IVmDbgServiceBridge` that wraps a shared VM pointer, created via the plugin factory's `createVmDebuggerConnection()` override.

- `UaeSharedConnectionImpl` (`src/quasar_app/uae_imp/uae_server_app_part.cpp`) -- UAE shared memory bridge
- `VAmSharedConnectionImpl` (`libs/vAmiga_imp_lib/src/qvAmigaImp/va_server_app_part.cpp`) -- vAmiga shared memory bridge

**Declared but unimplemented** factory functions (stubs in `dbgConnection.h`):
- `create_uae_shared_connection(const char* name)` -- Intended standalone factory
- `create_dummy_connection()` -- Intended test placeholder

**`IVmConnectionBuilder`** (`debuggerServer.h`) -- Abstract factory also implementing `IOperationEnvironment`:

```cpp
class IVmConnectionBuilder : public qd::RefCounted, public qd::IOperationEnvironment {
protected:
    IVm::VM* vm = nullptr;
public:
    void init();
    IVm::VM* getVm() const { return vm; }
    virtual ref_ptr<IVmDbgServiceBridge> createConnection() const = 0;
};
```

**`IVmConnectionsManager`** (`debuggerWndApp.h`) -- Interface for managing multiple debugger connections:

```cpp
class IVmConnectionsManager {
    virtual uint32_t getNumConnections() = 0;
    virtual ref_ptr<IVmDbgServiceBridge> createVmProvider(const char* conn_id) = 0;
};
```

### Server Components

Each emulator backend provides:

| Component | UAE | vAmiga |
|-----------|-----|--------|
| Server Thread | `UaeServerThread` | `VAmServerThread` |
| Server App Part | `UaeServerAppPart` | `VAmServerAppPart` |
| VM Implementation | `UaeVmImp` | `VAmVmImp` |
| Bridge Impl | `UaeSharedConnectionImpl` | `VAmSharedConnectionImpl` |
| Plugin Factory | `UaeServerProviderFactory` (id=`"uae"`) | `VAmigaServerProviderFactory` (id=`"vamiga"`) |

Both server threads implement `IVmClientPlayer`:

```cpp
class IVmClientPlayer {
    virtual IVm::VM* getVm() const = 0;
    virtual int  getScrFrameNo() = 0;
    virtual void pushSdlEvent(const SDL_Event&) = 0;
    virtual void pushOperationMsg(qtd::unique_ptr<qd::operation::BaseOpArgs>) = 0;
    virtual bool lockDisplayTexBuf(int* w, int* h, uint32_t** pixels) = 0;
    virtual void unlockDisplayTexBuf() = 0;
};
```

### Plugin Factory System

Backend registration uses a static plugin pattern. Each backend creates a factory that registers itself at load time:

```cpp
// UAE backend (src/quasar_app/uae_imp/uae_server_app_part.cpp):
class UaeServerProviderFactory : public qsr::IAppPartServerProviderFactory {
    // id = "uae", guiName = "UAEmu"
    virtual ref_ptr<amD::IVmDbgServiceBridge> createVmDebuggerConnection() override;
};
static qsr::plugin_api::RegOnLoadAppPartServerFactory reg_me(new UaeServerProviderFactory());

// vAmiga backend (libs/vAmiga_imp_lib/src/qvAmigaImp/va_server_app_part.cpp):
class VAmigaServerProviderFactory : public qsr::IAppPartServerProviderFactory {
    // id = "vamiga", guiName = "vAmiga emulator"
    virtual ref_ptr<amD::IVmDbgServiceBridge> createVmDebuggerConnection() override;
};
static qsr::plugin_api::RegOnLoadAppPartServerFactory reg_me(new VAmigaServerProviderFactory());
```

The `VmPlayersSelector` looks up factories by ID string and creates the server app part at runtime:

```cpp
int VmPlayersSelector::activateVmPlayerByIdStr(QuaesarApplication* pApp, const char* vmProviderId) {
    // "uae" or "vamiga"
    IAppPartServerProviderFactory* pFactory = findFactoryByIdStr(vmProviderId);
    ServerAppPartCreateCtx ctx;
    pFactory->createServerAppPart(ctx);  // Creates thread, VM, etc.
    pPart->onPartCreate(prm);
    pApp->getAppParts()->addPart(ctx.outPartPtr);
}
```



## Threading Model

Each emulator runs in its own dedicated SDL thread, completely isolated from the main UI thread.

### Thread Architecture

```
Main Thread                          Emulator Thread
+------------------------+           +------------------------+
| SDL Event Loop         |           | UAE / vAmiga Main Loop |
| ImGui Rendering        |           | CPU Execution          |
| DebuggerApp::update    |           | Chipset Emulation      |
| QsrMainWndApp::render  |           | Audio Mixing           |
|                        |           | Disk I/O               |
+-----------+------------+           +-----------+------------+
            |                                    |
            |  +-- Mutexes + Queues --+          |
            +--+ m_eventMutex         +----------+
               | m_UaeScrTextureMutex  |
               | m_pClientOpsStack     |
               | m_sdlEventsQueue      |
               +-----------------------+
```

### Thread Synchronization Points

1. **SDL Events** -- Main thread pushes keyboard/mouse events into `m_sdlEventsQueue` (protected by `m_eventMutex`). The emulator thread polls this queue during `onUaeHandleEvents()`.

2. **Screen Buffer** -- The emulator writes to `m_pAmigaBuffer` (raw 32-bit ARGB). The main thread calls `lockDisplayTexBuf()` which acquires `m_UaeScrTextureMutex`, copies the buffer to an SDL texture, then releases via `unlockDisplayTexBuf()`.

3. **Operations** -- UI operations (step, continue, break) are pushed via `pushOperationMsg()` into `m_pClientOpsStack`. The emulator thread processes these at safe points.

4. **Console Commands** -- A dedicated console queue (`m_pConsoleQueue`) bridges console input from the debugger to the emulator's debug console.

### Initialization Sequence

Both threads use a `ThreadEvent` (`m_onUaeInitialized`) to synchronize startup:
1. Main thread creates the server thread
2. Server thread initializes the emulator
3. Server thread signals `m_onUaeInitialized`
4. Main thread waits (or continues) and begins rendering

### Frame Counter

`m_scrFrameNo` (SDL_atomic_t) is incremented by the emulator thread each time a new frame is rendered. The main thread compares this against its last rendered frame number to avoid unnecessary texture updates.

## Memory Subsystem Architecture

### Amiga Memory Map

The Amiga memory space is represented by `EMemSrc` enum values, each corresponding to a physical memory region:

| Region | Address Range | Description |
|--------|---------------|-------------|
| `CHIP` | 0x000000-0x1FFFFF | Chip RAM (shared with chipset, up to 2MB) |
| `CHIP_MIRROR` | mirrors | Mirror of chip RAM |
| `SLOW` | 0xC00000-0xC7FFFF | Slow RAM / Bogo RAM (CPU-only, 512KB) |
| `FAST` | 0x200000-0x9FFFFF | Fast RAM (CPU-only, separate bus) |
| `CIA` | 0xBFE001 / 0xBFD000 | CIA-A and CIA-B registers |
| `CUSTOM` | 0xDFF000-0xDFF1FF | Custom chipset registers |
| `AUTOCONF` | 0xE80000 | Auto-config space for expansion boards |
| `ROM` | 0xFC0000-0xFFFFFF | Kickstart ROM (512KB) |
| `WOM` | varies | Write-Once Memory (A1000 only) |
| `EXT` | 0xE00000 | Extended ROM (AROS) |
| `RTC` | 0x00DC00 | Real-time clock registers |
| `ZOR` | varies | Zorro expansion board space |

### MemBank Class

Each memory bank is represented by:

```cpp
class MemBank {
    EMemSrc    m_id;         // Bank type identifier
    uint32_t   m_size;       // Size in bytes
    uint32_t   m_mask;       // Address mask
    qtd::string m_name;      // Display name
    qtd::string m_label;     // Short label
    AddrRef    m_startAddr;  // Base address
    uint8_t*   m_realAddr;   // Direct pointer to emulator memory
    bool       m_bEnabled;   // Whether this bank is active
};
```

The `m_realAddr` field provides zero-copy access to the emulator's address space. When the debugger needs to read memory, it first finds the correct bank via `findBankByAddr()`, then uses `getRealAddr()` or the bank's `getU8()`/`getSpan()` methods.

### Address Type

`AddrRef` is used throughout the codebase as a 32-bit address reference into the Amiga's address space. It corresponds to the M68K's 24-bit or 32-bit address bus depending on the CPU model.



## Debugger Components (amDebugger)

### Debugger Engine

The core debugger engine (`amD::Debugger`) is a client that holds a reference to an `IVmDbgServiceBridge` and the VM. It implements `IOperationEnvironment` to receive and process debug operations.

Key capabilities:
- **Debug Mode Control** -- Switch between `Live` (running), `Break` (paused)
- **Console Commands** -- `execConsoleCmd()` sends commands to the emulator's built-in console
- **State Fetching** -- `fetchVmState()` calls `fetch()` on all IVm modules to snapshot emulator state
- **Breakpoint Management** -- Up to 20 breakpoints (`BREAKPOINTS_MAX`), stored in `BreakpointsSortedList`

#### Breakpoint System

```cpp
class Breakpoint {
    AddrRef addr1;      // Primary address
    AddrRef addr2;      // Secondary address (range)
    bool enabled;       // Active flag
    IVm::EReg reg;      // Register to watch (PC, A0-A7, D0-D7, etc.)
};
```

Breakpoints are sorted by address for fast lookup via `fixed_set<OneAddrBp>`. The `EReg` enum supports 37 register types including D0-D7, A0-A7, PC, USP, MSP, ISP, VBR, SR, and more.

### Debugger App Part

`DebuggerApp` is an `ApplicationPart` that creates its own SDL window and ImGui context for the debugger UI. It:
- Creates a dedicated `SDL_Window` and `SDL_Renderer`
- Initializes ImGui with the `QImGuiContext` wrapper
- Owns the `Debugger` engine instance
- Owns the `DebuggerDesktop` (root UI node)
- Manages the `OperationsRegistry` for debug operations

### Debug Modes

```cpp
enum class EVmDebugMode {
    UNDEF,   // Not yet determined
    Live,    // Normal execution (emulator running)
    Break,   // Execution paused (breakpoint hit or manual break)
};
```

### Debugger Operations

All debugger actions are defined as operation argument structs in `debuggerOps.h`:

| Operation | Description | Default Shortcut |
|-----------|-------------|-----------------|
| `VmEmuReset` | Reset the Amiga | -- |
| `ExecConsoleCmd` | Execute a console command | -- |
| `DebugDmaOption` | Toggle DMA debug mode (off/mode 2/3/4) | -- |
| `DebugTraceStart` | Enter trace/debug mode | F12 |
| `DebugTraceContinue` | Continue execution | F5 |
| `DisasmTraceStepInto` | Single step (into subroutine) | F11 |
| `DisasmTraceStepOut` | Step out of current subroutine | F10 |
| `CopperTraceStep` | Step copper execution | Shift+F11 |
| `DisasmToggleBreakpoint` | Toggle breakpoint at cursor | F9 |
| `CopperToggleBreakpoint` | Toggle copper breakpoint | Shift+F9 |
| `ToggleTurboEmulation` | Toggle turbo speed | NumLock |
| `DebugWaitScanLines` | Wait N scanlines before breaking | -- |
| `VmPlayerWndAlwaysOnTop` | Pin emulator window on top | Ctrl+T |

Operations flow through the `IOperationEnvironment` chain:
1. Window receives shortcut -> creates operation args
2. `AmDbgWindow::applyOperationMsgProcImp()` handles or forwards
3. `DebuggerDesktop` processes or forwards to `Debugger`
4. `Debugger` pushes to emulator thread via `IVmClientPlayer::pushOperationMsg()`

## Code Analysis Engine (cda)

The code analysis engine (`amD::cda` namespace) provides M68K disassembly with intelligent page caching.

### M68CodeDisassembler

A singleton that wraps the Capstone disassembly engine (`csh* m_pCapstone`) with a page-based caching system:

```cpp
class M68CodeDisassembler {
    csh* m_pCapstone;
    qtd::array<CodeChunk, 64> m_disasmChunkStorage;    // Fixed pool of 64 pages
    eastl::intrusive_list<CodeChunk> m_chunkUseHistory;  // LRU eviction order
    QuadTreeAddrMap<uint16_t, 13> m_chunksQuadTree;      // Address -> page lookup
};
```

### CodeChunk (Page Cache)

Each `CodeChunk` covers 64 bytes (`g_chunkSize = 1 << 6`) of Amiga address space:

```cpp
struct CodeChunk : public eastl::intrusive_list_node {
    AddrRef m_addr;                                        // Base address
    qtd::array<uint8_t, 64> m_bytes;                       // Copy of memory bytes
    qtd::array<CodeItem*, 32> m_codeItems;                 // Disassembled items (max 32 = 64/2)
    uint16_t m_idx;                                        // Pool index
    bool m_bAddrValid : 1;                                 // Address is set
    bool m_bCodeValid : 1;                                 // Disassembly is current
    bool m_bBytesValid : 1;                                // Byte copy is current
};
```

Page lifecycle:
1. `requestCodeChunk(vm, addr)` -- Find or create a page for the address
2. If found in quad-tree, move to front of LRU list
3. If not found, evict least-recently-used page, reassign
4. Copy bytes from VM memory, disassemble via Capstone
5. Store `CodeItem` pointers for each instruction

### QuadTreeAddrMap

A custom quad-tree (4-ary tree) for fast address-to-page mapping:

```cpp
template<class TItem, int TMAX_DEPTH = 16>
class QuadTreeAddrMap {
    // Each node has 4 children (2 bits of address per level)
    // Depth = (32 - 6) / 2 = 13 levels for 64-byte granularity
    // insert/query/remove operations
};
```

The tree uses 2 bits per level (4 children), giving `O(log_4(N))` lookup. For the 32-bit Amiga address space with 64-byte pages, this requires 13 levels maximum.

### cda::Item Type Hierarchy

```
cda::Item (base)
  +-- cda::CodeItem    (disassembled instruction with text)
  +-- cda::DataInfo    (data bytes)
  +-- Labels, Comments, Function descriptions (planned)
```

Each item has an `EItemType` discriminant:
- `Label` -- Named code/data location
- `Code` -- Disassembled M68K instruction
- `Data` -- Raw data byte(s)
- `CommentBlock` / `CommentLine` -- User annotations
- `FunctionDescLine` -- Function signature info

## Copper Disassembler

The `DecodedCopperList` struct (`copperDisasm.h`) decodes Amiga Copper instructions from memory.

### Copper Instruction Format

Each Copper instruction is two 16-bit words (4 bytes):

| Instruction | Bit Pattern | Description |
|-------------|-------------|-------------|
| `MOVE` | `xx xxxx xxx 0 0000, vvvv vvvv vvvv vvvv` | Write value to register |
| `WAIT` | `vvvv vvvv hhhh hh0 0, vvvv eeee hhhh eeee 1` | Wait for beam position |
| `SKIP` | `vvvv vvvv hhhh hh0 1, vvvv eeee hhhh eeee 1` | Skip next instruction |

### Decoding Process

```cpp
void decodeInstr(Entry& ent) {
    uint32_t insn = (w1 << 16) | w2;
    uint32_t insn_type = insn & 0x00010001;
    switch (insn_type) {
        case 0x00010000:  // WAIT
        case 0x00010001:  // SKIP
        case 0x00000000:  // MOVE
        case 0x00000001:  // MOVE (variant)
    }
}
```

For `WAIT`/`SKIP`, the disassembler extracts:
- `vp` -- Vertical position (bits 31-24)
- `hp` -- Horizontal position (bits 23-17)
- `ve` -- Vertical enable/mask (bits 14-8)
- `he` -- Horizontal enable/mask (bits 7-0)
- `bfd` -- Blitter finished disable (bit 15)

For `MOVE`, the target register is identified via `CustReg::getRegByAddr()` using the `customRegsList.h` database, producing human-readable output like `0x0123 -> BPLCON0`.



## Expression Parser Integration

The expression parser (`libs/exprParser/`) is based on Drunk Fly's MIT-licensed parser, adapted for Amiga debugger use.

### Architecture

```
User Input String
      |
      v
  ExprValStr::setStrVal()
      |
      v
  ParserOop::Expr::parse()    <-- Lexer tokenizes, Parser builds AST
      |
      v
  Expr* (AST root)
      |
      v
  ExprValStr::evaluate(vm)    <-- Evaluates with VM context
      |
      v
  qd::Var16 (result)
```

### Key Types

- **`ExprValStr`** (`exprValue.h`) -- Bridge between the parser and the debugger. Stores the input string and parsed expression. Re-parses when the string changes.
- **`ExprValue`** -- Integer result type (`typedef int`)
- **`ExprResolver`** -- Interface for resolving symbols (register names, memory addresses) during parsing
- **`ExprEvaluator`** -- Interface for evaluating expressions with VM context
- **`Var16`** (`qd::variant16.h`) -- 16-bit variant that can hold signed/unsigned/hex display of the result

### Usage in Debugger Windows

The expression evaluator is used in:
- **Disassembly Window** -- Address expression input (`m_addrInputStr`) for "go to address"
- **Memory Window** -- Address expressions for memory view navigation
- **Memory Graph Window** -- Address expressions for graph offset
- **Console** -- Evaluation of debugger commands like `d $2000` or `m D0+100`

The parser supports:
- Hex literals (`$2000`, `0x2000`)
- Register references (`D0`, `A7`, `PC`)
- Arithmetic operators (`+`, `-`, `*`, `/`)
- Parenthesized sub-expressions
- Custom symbols resolved via the VM context

## UI Framework Architecture

### Window Hierarchy

```
qd::Node
  +-- qd::ApplicationPart
  |     +-- qsr::BaseVmServerAppPart
  |     |     +-- UaeServerAppPart
  |     |     +-- VAmServerAppPart
  |     +-- QsrMainClientWndApp
  |     +-- amD::DebuggerApp
  |     +-- BarmanProfileViewerAppPart (placeholder)
  |
  +-- qd::UiNode
        +-- qd::UiDesktop
        |     +-- amD::DebuggerDesktop
        |     +-- QsrVmClientPlayerGuiDesktop
        +-- qd::UiWindow
              +-- amD::AmDbgWindow (base for all debug windows)
                    +-- DisassemblyView
                    +-- MemoryWnd
                    +-- MemoryGraphWnd
                    +-- RegistersWnd
                    +-- CustomRegsWnd
                    +-- CopperWnd
                    +-- BlitterWnd
                    +-- ColorsWnd
                    +-- ScreenWnd
                    +-- ConsoleWnd
                    +-- ImGuiDemoWindow
```

### DebuggerDesktop

The `DebuggerDesktop` is the root UI node for the debugger. It:
- Owns the menu bar (`_drawMainMenuBar()`)
- Owns the toolbar (`_drawToolBar()`)
- Manages all debug windows via `createAllUiWndows()`
- Processes operations via `IOperationEnvironment` chain
- Owns the `ShortcutsMgr` for keyboard bindings

### Window Registration

Windows are registered using macros that combine type reflection with factory callbacks:

```cpp
#define QDB_WINDOW_REGISTER(enumId, ClassName, BaseClass)          \
    TS_BEGIN_REFLECT_CLASS(ClassName, BaseClass);                   \
    TS_ATTRIBUTE(qd::tsAttr::CustomClassId32(enumId));              \
    TS_ATTRIBUTE(qd::tsAttr::CreateClassCb(&createWindowCb_<ClassName>)); \
    TS_END()
```

Each window gets a unique `WndId` enum value and is automatically instantiated by the desktop.

### Color Theme System

`UiStyle` (`uiStyle.h`) defines debugger-specific colors as an enum-driven color table:

| Color Key | Value | Usage |
|-----------|-------|-------|
| `DisasmWnd_PcCursor` | (0, 10, 160) | PC highlight in disassembly |
| `DisasmWnd_UserCursor` | (160, 160, 0) | User cursor position |
| `DisasmWnd_OpCodeBytes` | (128, 128, 128) | Opcode byte display |
| `DisasmWnd_Addr` | (192, 192, 192) | Address labels |
| `RegistersWnd_RegName` | (164, 164, 164) | Register names |
| `RegistersWnd_RegValue` | (255, 255, 255) | Register values |
| `CustomRegsWnd_RegName` | (165, 164, 164) | Custom register names |
| `CustomRegsWnd_RegValue` | (255, 255, 255) | Custom register values |

Colors are accessed via `uiGetColorU()` / `uiGetColorF()` for integer/float ImGui formats.

### Options Dialog

The UAE options dialog (`uae_options_wnd.*`) implements a tree-structured category browser:

```
Root
 +-- Quickstart
 +-- Sound
 +-- Hardware
 +-- CPU
 +-- Host
 +-- Floppy drives
 +-- Advanced
```

Each category (`UCategory`) contains options (`UOption`) with draw callbacks. The `BaseOptionsDlg` provides the tree navigation, and `UaeOptionsDlg` populates it with UAE-specific settings.

## Configuration System

### Config Singletons

Configuration follows a singleton pattern with a macro:

```cpp
#define CFG_DECLARE(TCfgClass)            \
    TS_REFLECT_CLASS(TCfgClass, CfgBase);  \
    static TCfgClass& get() {              \
        static TCfgClass instance;         \
        return instance;                   \
    }
```

### Configuration Instances

| Config | Location | Purpose |
|--------|----------|---------|
| `CfgQsrStartup` | `qsr_config.h` | CLI arguments: kickstart path, input file, serial port, UAE args |
| `CfgQsrMain` | `qsr_main_wnd_client_app.h` | Main window: size, ESC-quit, VM player selection (`"uae"` or `"vamiga"`) |
| `CfgVmPrefs` | `amDebugger/config.h` | VM model: A500/A1200, chip/fast RAM sizes |
| `DbgConfig` | `amDebugger/debuggerConfig.h` | Debugger options: VH pops lines display |

### VM Model Configuration

```cpp
struct EVmModel {
    A500,    // Motorola 68000, OCS chipset
    A1200,   // Motorola 68020, AGA chipset
};

struct EVmModelCfg {
    Chip512Kb = 1 << 10,   // 512 KB chip RAM
    Chip1Mb   = 1 << 11,   // 1 MB chip RAM
    Chip2Mb   = 1 << 12,   // 2 MB chip RAM
    Fast512Kb = 1 << 20,   // 512 KB fast RAM
    Fast1Mb   = 1 << 21,   // 1 MB fast RAM
    A500_DEF  = Chip512Kb | Fast512Kb,  // Default A500 config
};
```

## Custom Registers Database

The file `customRegsList.h` (31 KB) is an X-macro that defines every Amiga custom chip register:

```cpp
#define QDB_CUSTOM_REGS_LIST(REG) \
    REG(BLTCON0,  0x040, specials, mask, "Blitter control register 0") \
    REG(BLTCON1,  0x042, specials, mask, "Blitter control register 1") \
    REG(DMACON,   0x096, specials, mask, "DMA control") \
    REG(INTENA,   0x09A, specials, mask, "Interrupt enable") \
    REG(BPLCON0,  0x100, specials, mask, "Bitplane control register 0") \
    REG(COP1LCH,  0x080, specials, mask, "Copper 1 location high") \
    // ... hundreds more
```

Each entry provides: enum ID, hardware address, special handling flags, bitmask, and description.

Flag descriptors (`CustomFlagsDesc`) provide bit-level documentation for registers like `DMACON` (AUD0EN through SETCLR) and `BLTCON0` (ABC through SRCA opcodes).


## qd Framework Deep Dive

The `qd` library is Quaesar's custom application framework, providing the foundation for both the emulator display and the debugger.

### Type Reflection System

The qd framework implements a lightweight RTTI-like system using macros:

```cpp
// Declare a reflected class
TS_BEGIN_REFLECT_CLASS(ClassName, ParentClass);
    TS_ATTRIBUTE(qd::tsAttr::Name("Display Name"));
    TS_ATTRIBUTE(qd::tsAttr::CustomClassId32(42));
    TS_ATTRIBUTE(qd::tsAttr::CreateClassCb(&factoryCallback));
TS_END();
```

This generates:
- Static `getStaticTypeInfo()` method
- Virtual `getTypeInfo()` method
- `isDerivedFrom()` checking
- Integration with the `TypeRegistry` singleton

The `TypeInfo` class stores:
- Class name hash
- Parent type info pointer
- Attributes (name, ID, factory callback)

### Node Hierarchy

`qd::Node` is the base of all tree-structured objects:

```
qd::Node
  +-- qd::ApplicationPart     (application modules)
  +-- qd::UiNode              (UI elements)
       +-- qd::UiDesktop       (root container)
       +-- qd::UiWindow        (ImGui window)
       +-- qd::UiDialog        (modal/popup dialog)
```

Nodes support:
- Parent-child relationships
- Named child lookup
- Lifecycle management (create/destroy)

### Application Parts

`qd::ApplicationPart` is a modular application component:

```cpp
class ApplicationPart : public qd::Node {
    qtd::string m_PartName;
    EAppPartMtd m_Methods;  // UPDATE and/or RENDER flags
    float m_ZOrder;          // Rendering order
    qd::Application* m_pApp;

    virtual void updateAppPart(float dt, float time);
    virtual void renderAppPart();
    virtual void onPartCreate(OnCreate_t& prm);
    virtual void destroyImp();
    virtual qd::EFlow onSdlEventProc(SDL_Event& event);
};
```

The `AppPartsManager` maintains a sorted list of active parts and calls `updateAppPart()` / `renderAppPart()` on each frame.

### Operation System

The operation system provides a typed message-passing mechanism:

```
IOperationEnvironment (interface)
  +-- applyOperationMsgProc(BaseOpArgs*)     -- handle operation
  +-- setupDefaultOperationArgs(BaseOpArgs*)  -- fill defaults
  +-- getOpEnvParent()                        -- chain to parent
```

Operations are defined as structs inheriting from `BaseOpArgs`:

```cpp
struct MyOperation : public BaseOpArgs {
    DECLARE_OPERATION_1(MyOperation);
    int myParam = 0;
    static void setup(OpDesc& d) {
        d.m_name = "My Operation";
        d.addShortcut(shortcut::EId::MyShortcut);
    }
};
```

The `OperationsRegistry` maintains a registry of all known operation types, and `UiOperationCreator` provides factory creation.

### Shortcut Management

Shortcuts are defined via X-macro in `shortcutsList.h`:

```cpp
#define SHORTCUT_LIST(SHORTCUT)                                    \
    SHORTCUT(DisasmTraceStepInto, [](qd::Shortcut& s) {            \
        s.addKey(ImGuiKey_F11).setRepeat(); })                     \
    SHORTCUT(DebugTraceStart, [](qd::Shortcut& s) {                \
        s.addKey(ImGuiKey_F12); })                                  \
    // ...
```

This generates:
- `amD::shortcut::EId` enum with all shortcut IDs
- `g_shortcuts_list[]` array with setup callbacks
- Each `qd::Shortcut` binds ImGui key + modifier combinations

### ImGui Integration

The `QImGuiContext` class wraps ImGui initialization:
- Creates ImGui context
- Sets up SDL2 backend (platform) + SDL2 renderer backend
- Handles font loading (Source Code Pro from `data/static/`)
- Manages new-frame / render lifecycle
- Applies custom style via `qd::ImColorsTab`

## Build System

### CMake Structure

```
CMakeLists.txt               -- Root: project setup, links everything
 +-- libs/uae_lib/CMakeLists.txt     -- UAE core (conditionals for FSUAE)
 +-- libs/vAmiga/Core/CMakeLists.txt -- vAmiga core
 +-- libs/vAmiga_imp_lib/CMakeLists.txt -- vAmiga integration
 +-- libs/amDebugger/CMakeLists.txt  -- Debugger library
 +-- libs/qd/CMakeLists.txt          -- Framework (uses QD_USE define)
 +-- libs/exprParser/CMakeLists.txt  -- Expression parser
 +-- external/*/CMakeLists.txt       -- All external dependencies
 +-- VAmigaLib.cmake                 -- Conditional vAmiga inclusion
 +-- scripts/cmake/*.cmake           -- Build helpers
```

### Key Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `VAMIGA` | ON | Enable vAmiga backend |
| `USE_STATIC_MSVC_RUNTIME` | ON | Static CRT linking on MSVC (/MT) |
| `ZLIB_BUILD_EXAMPLES` | OFF | Disable zlib examples |
| `CAPSTONE_M68K_SUPPORT` | ON | Enable M68K in Capstone |

### Resource Embedding

Resources (`resources/*`) are converted to C byte arrays at configure time using `scripts/cmake/bin2c.cmake`, producing `resources_inc.cpp` and `resources_inc.h`. This embeds the default layout INI and application icon directly into the binary.

### Platform Support

| Platform | SDL2 | Compiler | Notes |
|----------|------|----------|-------|
| Windows | Prebuilt .lib | MSVC 2019/2022 | Static CRT, WinMain subsystem |
| macOS | System/framework | Clang | `-Wno-macro-redefined` for UAE compat |
| Linux | pkg-config | GCC/Clang | Links `-ldl` |

### C++20 Features Used

- Concepts (in type system)
- `consteval` / `constexpr`
- Structured bindings
- `std::unique_ptr` with custom deleters
- EASTL as STL replacement for performance-critical paths

### EASTL Integration

The `qd` library uses EASTL (EA Standard Template Library) as its container library:

- `qtd::string` -- EASTL string wrapper
- `qtd::vector` -- EASTL vector
- `qtd::array` -- EASTL array (compile-time fixed size)
- `qtd::fixed_vector` -- EASTL fixed_vector (inline storage, no heap for small sizes)
- `qtd::span` -- Non-owning view
- `qtd::unique_ptr` -- std::unique_ptr compatible
- `qtd::ref_ptr` -- Reference-counted smart pointer (custom)
- `qtd::optional` -- std::optional compatible
- `eastl::fixed_set` -- Fixed-capacity sorted set
- `eastl::intrusive_list` -- Intrusive linked list (used for LRU cache)

The `QD_USE` macro controls which backends are enabled: `EASTL;SDL2;IMGUI`.


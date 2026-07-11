# Quaesar Source File Reference

Complete inventory of every source file in the Quaesar project, organized by subsystem. Each entry includes the file path, line count, and a description of its purpose and key contents.

---

## Root Files

| File | Lines | Description |
|------|------:|-------------|
| `CMakeLists.txt` | 220 | Root CMake build script. Sets C++20 standard, conditionally links static CRT on MSVC, includes all subdirectory CMakeLists, runs bin2c resource embedding at configure time, creates the `quaesar` executable target, links all libraries (adf, zlibstatic, SDL2, EASTL, imgui, nfd, qd, uae_lib, amDebugger). Platform-specific: Windows links Ws2_32/Winmm/etc, macOS adds `-Wno-macro-redefined`, Linux links `-ldl`. |
| `CMakePresets.json` | -- | CMake presets for Visual Studio 2019/2022, configuration types (RelWithDebInfo, Debug, Release, MinSizeRel). |
| `VAmigaLib.cmake` | 10 | Conditional inclusion of vAmiga backend: adds `libs/vAmiga` and `libs/vAmiga_imp_lib` subdirectories, links `VAmigaImpLib`. |
| `.gitignore` | -- | Git ignore rules. |

---

## `src/quasar_app/` -- Main Application

The primary application code that ties together all libraries.

### Entry Point

| File | Lines | Description |
|------|------:|-------------|
| `qsr_main.cpp` | 73 | **Application entry point** (`SDL_main`). Parses CLI via CLI11 (`input`, `-k/--kickstart`, `--serial_port`, `-s` UAE ext args). Initializes SDL video+audio. Creates `QuaesarApplication`, calls `onConstruct()`, `initialize()`, `doMainLoop()`, `destroy()`. Initializes NFD for native file dialogs. |

### Application Core

| File | Lines | Description |
|------|------:|-------------|
| `qsr_application.h` | 94 | **`qsr::QuaesarApplication`** class. Inherits `qd::Application`. Owns `DebuggerApp*` and `QsrMainClientWndApp*`. Declares `BaseVmServerAppPart` (abstract, inherits `qd::ApplicationPart`, provides `getVmPlayer()`). Also declares `IVmClientPlayer` interface for cross-thread communication. |
| `qsr_application.cpp` | 73 | Implements `QuaesarApplication`: constructor, `onConstruct()` (initializes app parts manager), `initialize()` (creates emulator display window, debugger app, activates VM player), `destroyImp()`, `onSdlEventProc()`, `getInterface()`. |
| `quaesar.h` | 11 | Precompiled header. Forward declares `QuaesarApplication` and `QuaesarOptions`, declares global `g_pApp` pointer. |
| `qsr_app_interfaces.h` | 25 | **`qsr::IVmClientPlayer`** interface. Defines cross-thread contract: `getVm()`, `getScrFrameNo()`, `pushSdlEvent()`, `pushOperationMsg()`, `lockDisplayTexBuf()`/`unlockDisplayTexBuf()`. Bridge between main thread and emulator thread. |
| `qsr_config.h` | 21 | **`CfgQsrStartup`** config singleton. CLI arguments: `kickRomPath`, `input` (ADF/DMS), `serialPort`, `uaeExtArgs`. Accessed via `g_cfg_startup`. |
| `qsr_config.cpp` | 1 | Empty (config is header-only via `CFG_DECLARE` macro). |
| `qsr_operations.h` | 36 | **`qsr::operations` namespace.** Defines app-level operations: `ShowDebuggerWnd` (Shift+F12), `ShowUaeOptionsWnd` (Ctrl+P), `QuitQuasarApp`. |
| `qsr_operations.cpp` | 1 | Empty. |
| `qsr_debug.h` | 17 | Debug utilities. `TRACE()` macro (disabled by default, logs unimplemented function names). Declares `debug()` printf-style function. |
| `qsr_debug.cpp` | 11 | Implements `debug()` -- formats message with function name, logs via `SDL_LogMessageV`. |

### Main Emulator Window

| File | Lines | Description |
|------|------:|-------------|
| `qsr_main_wnd_client_app.h` | 104 | **`QsrMainClientWndApp`** class. Inherits `ApplicationPart` + `IVmOperationsHandler`. Owns SDL_Window, SDL_Renderer, SDL_Texture for emulator display. Contains `VmPlayersSelector`, manages `IVmClientPlayer*`. Config: `CfgQsrMain` (window size, quit-by-esc, VM player ID defaulting to `"uae"`). |
| `qsr_main_wnd_client_app.cpp` | 307 | Implements the main emulator window: creates OS window, initializes ImGui context, renders emulator framebuffer as SDL texture, handles SDL events, processes debug operations, manages VM player lifecycle. Contains the main rendering pipeline that blits the Amiga screen. |

### VM Player Selector (Plugin System)

| File | Lines | Description |
|------|------:|-------------|
| `vm_player_selector.h` | 60 | **`VmPlayersSelector`** class. Contains `ProviderItem` (server app part + id + title). `activateVmPlayerByIdStr()` looks up factory and creates server app part. Declares **`IAppPartServerProviderFactory`** interface (`id`, `guiName`, `setup()`, `createServerAppPart()`, `createVmDebuggerConnection()`). Also `plugin_api::RegOnLoadAppPartServerFactory` for static registration. |
| `vm_player_selector.cpp` | 76 | Implements **`AppPartServerFactoryListMgr`** singleton (holds all registered factories). `activateVmPlayerByIdStr()` finds factory by ID, creates server app part, registers with app parts manager. Factories are registered at load time via `RegOnLoadAppPartServerFactory` constructor. |


### UAE Backend Integration (`uae_imp/`)

| File | Lines | Description |
|------|------:|-------------|
| `uae_imp/uae_vm_imp.h` | 139 | **`IVm::imp::UaeVmImp`** class (final). Full IVm implementation for UAE. Contains nested classes: `Cpu` (reads UAE `regs.regs[]`), `Memory` (populates MemBank from UAE memory map), `Blitter` (wraps UAE blitter state), `CustomRegs` (bulk-copies UAE custom register array), `Copper` (reads cop1lc/cop2lc), `Emu` (DMA debug mode, breakpoints), `Floppy` (4 drives, reads UAE config). Holds `UaeServerThread*` pointer. |
| `uae_imp/uae_vm_imp.cpp` | 418 | Implements all UaeVmImp methods. Memory init reads UAE's `addrmap[]`, `allocated_chipmem`, etc. CPU reads from UAE `regs` global. Custom register fetch/commit uses UAE's `custom_regs` array. Emu methods call UAE debug API (`debug_dma_*`, `activate_debugger`). |
| `uae_imp/uae_server_thread.h` | 75 | **`UaeServerThread`** class. Inherits `IVmClientPlayer`. Runs UAE in a separate SDL thread. Manages: `m_pAmigaBuffer` (screen pixels), `m_UaeScrTextureMutex` (buffer synchronization), `m_sdlEventsQueue` (input forwarding), `m_pClientOpsStack` (debug operations), `m_pConsoleQueue` (console bridge). Owns `ref_ptr<UaeVmImp>`. |
| `uae_imp/uae_server_thread.cpp` | 296 | Implements UAE thread lifecycle: `initialize()` starts SDL thread, `_lockUaeScreenTexBuf()`/`_unlockUaeScreenTexBuf()` for frame exchange, `onUaeHandleEvents()` polls event queue and processes ops, `execConsoleCmd()`/`uaeWaitConsoleCmdImpl()` bridge UAE console. Thread runs UAE main loop. |
| `uae_imp/uae_server_app_part.h` | 46 | **`UaeServerAppPart`** class. Inherits `BaseVmServerAppPart`. Wraps UAE server as an ApplicationPart. Creates `UaeServerThread`, provides `getVmPlayer()`, handles `updateAppPart()`. |
| `uae_imp/uae_server_app_part.cpp` | 133 | Implements UAE server app part: `createUaeThread()`, `destroyImp()`, `updateAppPart()` (polls VM state), `getVm()`. Creates debugger connection via `IVmConnectionBuilder`. |
| `uae_imp/qsr_imp_proxy.h` | 13 | **C linkage proxy functions** called by UAE core (which is C): `qsr_setUaeInitiized()`, `qsr_lockUaeScreenTexBuf()`, `qsr_unlockUaeScreenTexBuf()`, `qsr_onUaeHandleEvents()`, `qsr_waitConsoleCmd()`. Bridges UAE's C callbacks to C++ `UaeServerThread` methods. |
| `uae_imp/qsr_imp_proxy.cpp` | 32 | Implements proxy functions. Maintains `g_pUaeThread` global, delegates all calls to `UaeServerThread::get()` singleton. |

### UI Components (`ui/`)

| File | Lines | Description |
|------|------:|-------------|
| `ui/uae_wnd_desktop.h` | 43 | **`QsrVmClientPlayerGuiDesktop`** class. Inherits `qd::UiDesktop` + `IOperationEnvironment`. Root UI node for the emulator display window. Declares **`IVmOperationsHandler`** (combines `IOperationEnvironment` + `IVm::IVmHandler`). |
| `ui/uae_wnd_desktop.cpp` | 111 | Implements emulator desktop: `init()` creates UI nodes, `drawContentImp()` renders ImGui overlay on emulator display, `applyOperationMsgProcImp()` forwards operations to VM. |
| `ui/uae_options_wnd.h` | 139 | **Options dialog system.** `EOptionCat` enum (Quickstart, Sound, Hardware, CPU, Host, Floppy, Advanced). `UCategory` (tree node with children/options), `UOption` (with `fixed_function` draw callback). `BaseOptionsDlg` (generic tree browser). `UaeOptionsDlg` (UAE-specific). Declares `open_file_dlg_select_adf()` for ADF file selection. |
| `ui/uae_options_wnd.cpp` | 227 | Implements options dialog: category tree navigation, option drawing callbacks, NFD-based ADF file picker integration. `UaeOptionsDlg::onUiNodeCreated()` populates categories and options. |

### Bartman Profile Viewer (`bartman_profile_viewer/`)

| File | Lines | Description |
|------|------:|-------------|
| `bartman_profile_viewer/profile_viewer_app.h` | 16 | **`BarmanProfileViewerAppPart`** skeleton. Empty `onPartCreate()`. Placeholder for future Amiga profiling integration. |
| `bartman_profile_viewer/profile_viewer_app.cpp` | 1 | Empty. |
| `bartman_profile_viewer/profile_viewer_emu.h` | 0 | Empty placeholder. |
| `bartman_profile_viewer/profile_viewer_emu.cpp` | 0 | Empty placeholder. |

---

## `src/uae_lib_imp/` -- UAE Platform Hooks

Platform-specific implementations that replace WinUAE's original Windows/AmigaOS layer. These files provide the glue between the UAE core (which expects certain platform functions) and Quaesar's SDL2-based framework.

| File | Lines | Description |
|------|------:|-------------|
| `sysconfig.h` | 658 | **UAE build configuration.** Defines `FSUAE`, `SDL`, `HAVE_*` feature macros, `uae_*` type definitions, memory model settings, endianness. This is the master config that controls UAE's conditional compilation. |
| `target.h` | 22 | UAE target identification. Defines `TARGET_NAME`, `NO_MAIN_IN_MAIN_C` (prevents UAE from defining its own main), `OPTIONSFILENAME`. |
| `winuae_compat.h` | 85 | Compatibility shim for WinUAE APIs. Provides stubs/typedefs for Windows-only constructs used by UAE but not available on other platforms. |
| `machdep/m68k.h` | 128 | M68K-specific declarations. UAE CPU state structure mapping, 68000/68020/68040 register access macros. |
| `machdep/maccess.h` | 154 | **Memory access functions.** `get_long()`, `get_word()`, `get_byte()`, `put_long()`, `put_word()`, `put_byte()` -- the core memory accessors UAE uses. Maps to Quaesar's IVm::Memory interface. |
| `machdep/m68k.cpp` | 125 | M68K FPU emulation hooks, interrupt handling stubs. |
| `machdep/rpt.h` | 1 | Empty. RPT (real-time period timer) declarations. |
| `sounddep/sound.h` | 98 | **Sound output interface.** Audio driver stubs for UAE. Defines `audio_info`, `sound_*` functions. SDL2 audio output hooks. |
| `sounddep/sound.cpp` | 1009 | **Sound driver implementation.** SDL2 audio device initialization, buffer management, audio callback. Handles UAE's 16-bit stereo audio output, resampling, volume control. |
| `threaddep/thread.h` | 25 | Threading primitives for UAE. Maps UAE's `uae_sem_*` to SDL/POSIX semaphores. |
| `thread.cpp` | 164 | UAE thread implementation. `uae_start_thread()`, `uae_wait_thread()`, semaphore create/wait/signal/destroy using SDL2 primitives. |
| `gfx.cpp` | 55 | Graphics output hooks. Minimal stubs -- Quaesar handles rendering via its own screen buffer pipeline rather than UAE's native gfx. |
| `gui.cpp` | 53 | GUI hooks. Stubbed -- Quaesar provides its own UI via ImGui, not UAE's native GUI. |
| `input.cpp` | 10 | Input stubs. Keyboard/mouse input is handled by Quaesar's SDL event forwarding. |
| `keyboard_input_imp.cpp` | 251 | Keyboard input mapping. Translates SDL keycodes to UAE's internal key codes for Amiga keyboard emulation. |
| `file_system.cpp` | 1321 | **Filesystem implementation.** Maps UAE's filesystem calls to host OS. Hardfile handling, directory mapping, file I/O for emulated Amiga filesystem. |
| `filepaths.cpp` | 72 | UAE file path resolution. Maps Amiga paths to host filesystem paths. |
| `hardfile_host.cpp` | 797 | **Hardfile host implementation.** Low-level disk image I/O for UAE hardfile emulation. Read/write sectors, geometry management. |
| `adf.cpp` | 200 | ADF (Amiga Disk File) handling for UAE. Disk image loading, writing, sector access. |
| `adf.h` | 5 | ADF declarations. |
| `mman.cpp` | 183 | Memory mapping implementation. `uae_mmap()`, `uae_munmap()` using POSIX/SDL memory allocation. |
| `time.cpp` | 46 | Timing functions for UAE. `target_usleep()`, `target_sleep()`, read timer using SDL_GetTicks/SDL_GetPerformanceCounter. |
| `unicode.cpp` | 377 | Unicode conversion functions. UTF-8/UTF-16/ISO-8859-1 conversion for Amiga text handling. |
| `dummy.cpp` | 2066 | **Large stub file.** Provides empty implementations for UAE functions not needed in Quaesar's port. ~2000 lines of `void some_uae_function() {}` stubs. |
| `ahidsound.h` | 8 | AHI (Amiga High-level Audio) sound declarations. |
| `ahidsound_new.h` | 3 | New AHI sound declarations. |
| `avioutput.h` | 41 | AVI video capture output stubs. |
| `rp.h` | 60 | RetroPlatform compatibility declarations. |


---

## `libs/amDebugger/` -- Amiga Debugger Library

The complete debugger system: VM abstraction interfaces, code analysis engine, debugger UI windows, and operation system.

### Build

| File | Lines | Description |
|------|------:|-------------|
| `CMakeLists.txt` | 38 | Builds `amDebugger` static library. Sources all .cpp files in `src/amDebugger/`, links `qd` and `capstone`. |

### VM Abstraction Layer (`vm/`)

| File | Lines | Description |
|------|------:|-------------|
| `vm/vmInterface.h` | 218 | **Core VM abstraction.** `IVm` namespace. Declares `IVmHandler`, `IModule` (init/fetch lifecycle), `VM` (owns all modules, inherits RefCounted + IOperationEnvironment), `Floppy`, `Emu`, `Memory`, `Cpu`, `CustomRegs`, `Copper`, `Blitter`. Factory template `createByFactory_<T>()`. |
| `vm/vmInterface.cpp` | 61 | Implements `VM()` constructor, `~VM()`, `init()` (calls `init()` on all modules), `fetchStateFromEmu()` (calls `fetch()` on all modules in order), `applyOperationMsgProcImp()`. |
| `vm/memory.h` | 176 | **Memory types.** `EMemSrc` enum (NONE, CHIP, CHIP_MIRROR, SLOW, FAST, CIA, RTC, CUSTOM, AUTOCONF, ZOR, ROM, WOM, EXT, MAX_COUNT). `MemBank` class (id, size, mask, name, startAddr, realAddr, enabled). `EReg` enum (Dx=0, Ax=8, PC=16, USP, MSP, ISP, VBR, SR, CCR, CACR, etc. up to MAX_COUNT=37). `ECpuFlg` enum (C, V, Z, N, X, I0-I2, M, S, T0, T1, STOPPED). |
| `vm/memory.cpp` | 89 | Implements `MemBank::findBankByAddr()`, `Memory::getBankByInd()`. |
| `vm/customRegs.h` | 133 | **Custom register types.** `CustReg` enum (generated by X-macro from `customRegsList.h`). `CustReg::Data` struct (name, addr, special, mask, desc). `CustomFlagsDesc` (bitfield descriptions, `addBit()` builder). `BC0F` (BLTCON0 flags), `BC1F` (BLTCON1 flags), `DMAC` (DMACON flags with getFlagDesc). |
| `vm/customRegs.cpp` | 97 | Implements `CustReg::getRegByAddr()` (linear search through register database), `CustReg::getFlagDesc()`, `DMAC::getFlagDesc()`. |
| `vm/customRegsList.h` | 276 | **X-macro register database.** `QDB_CUSTOM_REGS_LIST(REG)` macro defining every Amiga hardware register with ID, address, specials, mask, and description. ~276 entries. 31 KB of register definitions. |
| `vm/emuDefs.h` | 65 | **Emulator definitions.** `breakpoint_reg_end` constant. `ECopperAddr_` enum (null, cop1lc, cop2lc, vblankip, ip). `ECpuFlg_` enum. `ECopperStates` enum (18 states matching real copper state machine). `EVmDebugMode` (UNDEF, Live, Break). |

### Debugger Engine

| File | Lines | Description |
|------|------:|-------------|
| `debugger.h` | 101 | **`amD::Debugger`** class. Inherits RefCounted + IOperationEnvironment. Owns `IVmDbgServiceBridge` and `IVm::VM`. Manages debug mode, console commands, state fetching, breakpoints. `Breakpoint` struct (addr1, addr2, enabled, reg). `BreakpointsSortedList` (fixed_vector of 20 breakpoints + fixed_set for address lookup). `DbgProjOptinons` (traceWaitScanLines). |
| `debugger.cpp` | 99 | Implements Debugger: `setDbgServiceBridge()`, `fetchVmState()` (calls VM fetch), `setDebugMode()`, `execConsoleCmd()`, breakpoint initialization. |
| `debuggerOps.h` | 208 | **All debugger operations.** 13 operation structs: `VmEmuReset`, `ExecConsoleCmd`, `DebugDmaOption`, `DebugTraceStart`, `DebugTraceContinue`, `DisasmTraceStepInto`, `DisasmTraceStepOut`, `CopperTraceStep`, `DisasmToggleBreakpoint`, `CopperToggleBreakpoint`, `ToggleTurboEmulation`, `DebugWaitScanLines`, `VmPlayerWndAlwaysOnTop`. Each has `DECLARE_OPERATION_1` and `setup()` with name/shortcut. |
| `debuggerOps.cpp` | 8 | Empty (operations are header-only structs). |
| `shortcutsList.h` | 63 | **Keyboard shortcut registry.** X-macro `SHORTCUT_LIST` defines all shortcuts with ImGui key bindings. Generates `amD::shortcut::EId` enum and `g_shortcuts_list[]` array. Shortcuts: F5-F12, Shift+F9/F11/F12, Ctrl+T/P, NumLock. |
| `shortcutsList.cpp` | 11 | Empty (shortcuts are header-only). |

### Server-Side Connection

| File | Lines | Description |
|------|------:|-------------|
| `dbgConnection.h` | 27 | **`IVmDbgServiceBridge`** interface. Inherits RefCounted. Methods: `getClientVm()`, `getServerVm()`. Factory functions: `create_dummy_connection()`, `create_uae_shared_connection()`. |
| `dbgConnection.cpp` | 32 | Implements connection factories. Shared connection returns same VM for both client and server (same-process, direct pointer sharing). |
| `debuggerServer.h` | 23 | **`IVmConnectionBuilder`** class. Inherits RefCounted + IOperationEnvironment. Holds `IVm::VM*`, creates connections via `createConnection()`. |
| `debuggerServer.cpp` | 16 | Implements `IVmConnectionBuilder::init()`. |

### Debugger App Window

| File | Lines | Description |
|------|------:|-------------|
| `debuggerWndApp.h` | 89 | **`DebuggerApp`** class. Inherits ApplicationPart + IOperationEnvironment. Owns SDL_Window, SDL_Renderer, QImGuiContext. Owns `Debugger*`, `DebuggerDesktop*`, `OperationsRegistry*`. Also declares `IVmConnectionsManager` interface. |
| `debuggerWndApp.cpp` | 220 | Implements DebuggerApp: creates its own SDL window (separate from emulator), initializes ImGui, manages debugger lifecycle, handles visibility toggle, processes SDL events for debugger window. |

### Expression Value

| File | Lines | Description |
|------|------:|-------------|
| `exprValue.h` | 36 | **`ExprValStr`** class. Bridges expression parser to debugger. Stores string input + parsed `ParserOop::Expr*`. Methods: `setStrVal()` (re-parses), `evaluate(vm, Var16&)`. |
| `exprValue.cpp` | 108 | Implements parsing (calls `Expr::parse()` with VM-aware resolver) and evaluation (calls `Expr::evaluate()` with VM evaluator). Converts result to `Var16`. |

### Configuration

| File | Lines | Description |
|------|------:|-------------|
| `config.h` | 61 | Config singletons: `CfgBase`, `EVmModel` (A500, A1200), `EVmModelCfg` (chip/fast RAM sizes as flags), `CfgVmPrefs` (model + RAM config). |
| `debuggerConfig.h` | 10 | `DbgConfig` singleton. `showVHPopsLines` option for displaying vertical/horizontal position popups. |

### Code Analysis Engine (`codeAnalyzer/`)

| File | Lines | Description |
|------|------:|-------------|
| `codeAnalyzer/cdaServer.h` | 92 | **`M68CodeDisassembler`** singleton. Wraps Capstone (`csh*`). Fixed pool of 64 `CodeChunk` pages. LRU via `eastl::intrusive_list`. Address lookup via `QuadTreeAddrMap`. Method: `requestM68DisasmLines()`. |
| `codeAnalyzer/cdaServer.cpp` | 323 | Implements disassembler: page management (find/evict/create), Capstone disassembly per page, byte comparison for change detection, LRU ordering. |
| `codeAnalyzer/cdaTypes.h` | 89 | **`cda::Item` type hierarchy.** `EItemType` enum (Label, Code, Data, CommentBlock, CommentLine, FunctionDescLine). `Item` base class (addr, bytesCount, bytesString). `CodeItem` (disassembly text, operand type). `DataInfo`. |
| `codeAnalyzer/cdaPage.h` | 1 | Empty pragma-once (placeholder). |
| `codeAnalyzer/quadTreeAddrMap.h` | 147 | **`QuadTreeAddrMap<TItem, MAX_DEPTH>`** template. 4-ary tree for address-to-item lookup. Uses 2 bits per level. `insert()`, `querySingle()`, `remove()`, `clear()`. Node allocation with free-list recycling. |
| `codeAnalyzer/copperDisasm.h` | 137 | **`DecodedCopperList`** struct. Decodes Copper instructions: `MOVE` (register write), `WAIT` (beam position wait), `SKIP` (conditional skip). `decodeInstr()` extracts VP/HP/VE/HE/BFD bits. `decodeLines()` reads N instructions from VM memory. |

### UI Framework (`ui/`)

| File | Lines | Description |
|------|------:|-------------|
| `ui/debuggerDesktop.h` | 53 | **`DebuggerDesktop`** class. Inherits `qd::UiDesktop` + `IOperationEnvironment`. Owns `DebuggerApp*`, `Debugger*`, `OperationsRegistry*`, `ShortcutsMgr*`. Methods: `drawImGuiMainFrame()`, `createAllUiWndows()`, `_drawMainMenuBar()`, `_drawToolBar()`. |
| `ui/debuggerDesktop.cpp` | 272 | Implements desktop: creates all debugger windows, draws main menu (Debug, View menus), toolbar (step/continue/break buttons), processes operations, manages window visibility. |
| `ui/uiView.h` | 114 | **`AmDbgWindow`** base class for all debugger windows. Inherits `qd::UiWindow` + `IOperationEnvironment`. Contains `DebuggerDesktop*` pointer. `QDB_WINDOW_REGISTER` macro for type registration + factory. `createWindowCb_<T>()` template. Declares `ImGuiDemoWindow`. |
| `ui/uiView.cpp` | 35 | Implements `AmDbgWindow::getDbg()`, `AmDbgWindow::getVm()`, window creation callback. |
| `ui/uiStyle.h` | 65 | **`UiStyle`** color theme. `EColor` enum (DisasmWnd_PcCursor, UserCursor, OpCodeBytes, Addr, RegName, RegValue, CustomRegName, CustomRegValue). `applyColors()`, `uiGetColorU()`/`uiGetColorF()` accessors. |
| `ui/uiDefs.h` | 20 | **`WndId`** enum. Window identifiers: MemoryView, CopperDbgWnd, Disassembly, Registers, Console, Screen, Colors, MemoryGraph, CustomRegsWnd, BlitterWnd, ImGuiDemo, MostCommonCount. |

### Debugger Windows (`window/`)

| File | Lines | Description |
|------|------:|-------------|
| `window/disassembly_wnd.h` | 44 | **`DisassemblyView`** class. Expression-based address input (`m_addrInputStr`). Tracks view base address, cursor position, PC snap state. Uses `cda::Item*` vector for disasm lines. |
| `window/disassembly_wnd.cpp` | 239 | Renders M68K disassembly: requests lines from `M68CodeDisassembler`, highlights PC cursor (blue) and user cursor (yellow), shows opcode bytes, handles step-into/breakpoint toggle operations. |
| `window/memory_wnd.h` | 166 | **`MemoryHexViewWnd`** class. Based on ImGui club's memory editor (v0.50). Hex viewer with: columns, ASCII view, data preview (bin/dec/hex), address input via expression, highlight support, read-only mode, bank tracking. ~850 lines of rendering code. |
| `window/memory_wnd.cpp` | 850 | Full hex memory editor implementation: hex/ASCII grid, cell editing, data type preview, goto address, bank selection, search functionality. Largest single file in the debugger. |
| `window/memory_graph_wnd.h` | 36 | **`MemoryGraphWnd`** class. Renders memory as an SDL texture (pixel = byte value). Bank selection, scrollable view, expression-based address. |
| `window/memory_graph_wnd.cpp` | 204 | Creates/manages texture, reads memory bank bytes, maps byte values to grayscale pixels, handles mouse drag for scrolling. |
| `window/registers_wnd.h` | 20 | **`RegistersWnd`** class declaration. |
| `window/registers_wnd.cpp` | 129 | Displays CPU registers: D0-D7 (data), A0-A7 (address), PC, SR flags (C/V/Z/N/X, interrupt mask, supervisor/trace). Color-coded names/values. |
| `window/custom_regs_wnd.h` | 25 | **`CustomRegsWnd`** class declaration. |
| `window/custom_regs_wnd.cpp` | 143 | Displays Amiga custom chip registers from `CustReg` database. Shows register name, value, and bitfield flags. Uses `CustomFlagsDesc` for bit-level display. |
| `window/copper_wnd.cpp` | 169 | Displays Copper list disassembly using `DecodedCopperList`. Shows MOVE/WAIT/SKIP instructions with register name resolution. Supports copper breakpoints and trace stepping. |
| `window/blitter_wnd.cpp` | 209 | Displays blitter state: BLTCON0/1 registers, source/destination addresses, data registers, control flags (minterms, channel enables). Shows blitter activity status. |
| `window/colors_wnd.cpp` | 64 | Displays the 32-color Amiga palette. Reads color registers, renders color swatches with RGB values. |
| `window/screen_wnd.cpp` | 120 | Captures and displays the emulator's framebuffer. Reads pixel buffer from `IVm::Blitter::getScreenPixBuf()`, renders as ImGui image. |
| `window/console_wnd.cpp` | 115 | Debug console window. Text input for WinUAE-style commands, command history, output display. Sends commands via `Debugger::execConsoleCmd()`. |

### Base

| File | Lines | Description |
|------|------:|-------------|
| `base.h` | 6 | Defines `amD::AddrRef` as `uint32_t` and imports it into global namespace. |


---

## `libs/vAmiga_imp_lib/` -- vAmiga Integration Layer

Integration code that adapts the vAmiga emulator to Quaesar's IVm abstraction.

| File | Lines | Description |
|------|------:|-------------|
| `CMakeLists.txt` | 48 | Builds `VAmigaImpLib` static library. Sources `src/qvAmigaImp/*.cpp`, links `VAmiga` (core), `qd`, `amDebugger`. |
| `src/qvAmigaImp/va_vm_imp.h` | 176 | **`IVm::imp::VAmVmImp`** class (final). Parallel to UaeVmImp but for vAmiga. Holds `VAmServerThread*`, `vamiga::VAmiga*`, `vamiga::Amiga*`, `QuaesarVAmigaInjectAccess*`. Nested classes: `Cpu` (reads `CPUInfo` from vAmiga), `Memory` (**stubs** -- returns 0 for getU16/getU32, no-op for set*), `Blitter`, `CustomRegs`, `Copper`, `Emu` (no `initBreakPoints`), `Floppy` (write-protect stubs). |
| `src/qvAmigaImp/va_vm_imp.cpp` | 475 | Implements VAmVmImp. CPU fetches via `m_pVAmiga->cpu.getInfo()`. Memory methods are stubbed. Screen buffer access through vAmiga's Denise component. Floppy uses vAmiga's disk API. |
| `src/qvAmigaImp/va_server_thread.h` | 85 | **`VAmServerThread`** class. Inherits `IVmClientPlayer`. Runs vAmiga in separate SDL thread. Manages: `vamiga::VAmiga*` instance, `m_VAmScrTextureMutex`, event queue, ops stack, message queue processor (`vAmigaMsgQueueProc()`), `fetchScreenBufferToTexture()`. |
| `src/qvAmigaImp/va_server_thread.cpp` | 402 | Implements vAmiga thread: creates `vamiga::VAmiga` instance, runs emulation loop (`onVAmigaThreadMain()`), processes vAmiga messages, fetches screen buffer, handles console commands, SDL event forwarding. |
| `src/qvAmigaImp/va_server_app_part.h` | 40 | **`VAmServerAppPart`** class. Inherits `BaseVmServerAppPart`. Parallel to `UaeServerAppPart`. Creates `VAmServerThread`. |
| `src/qvAmigaImp/va_server_app_part.cpp` | 106 | Implements vAmiga server app part: creates thread, manages lifecycle, provides `getVmPlayer()`. |

---

## `libs/exprParser/` -- Expression Parser

Based on Drunk Fly's MIT-licensed expression parser, adapted for Amiga debugger use.

| File | Lines | Description |
|------|------:|-------------|
| `CMakeLists.txt` | 19 | Builds `exprParser` static library. |
| `parser/common.h` | 54 | Core types: `ExprValue` (int), `ExprUValue` (unsigned int), `ExprError` (error message with 2KB buffer), `ExprValuePtr` (read callback + pointer + size), `ExprCallback0-3` (function pointer types, max 3 args). |
| `parser/common.cpp` | 33 | Implements `ExprError` constructor (printf-style formatting). |
| `parser/lexer.h` | 82 | `Lexer` class. Tokenizes input strings into tokens: numbers, identifiers, operators, parentheses. |
| `parser/lexer.cpp` | 422 | Full lexer implementation. Handles hex (`$`, `0x`), decimal, identifiers, operators (+, -, *, /, %, &, |, ^, ~, <<, >>), parentheses, commas. |
| `parser/parser_oop.h` | 42 | **`ParserOop::Expr`** abstract base class. `evaluate(ExprEvaluator&)` returns `ExprValue`. Static `parse()` factory method. |
| `parser/parser_oop.cpp` | 996 | Full recursive descent parser. Builds AST with expression nodes: binary ops, unary ops, number literals, identifier references, function calls. Operator precedence handling. |
| `parser/resolve_oop.h` | 54 | **`ExprResolver`** interface: resolves identifiers (register names, symbols) during parsing. **`ExprEvaluator`** interface: evaluates resolved symbols during evaluation. These are the extension points for the debugger's VM context. |
| `tests/test.cpp` | 330 | Expression parser unit tests. |
| `tests/common.h` | 50 | Test helper declarations. |
| `tests/common.cpp` | 129 | Test helper implementations. |

---

## `libs/qd/` -- Application Framework

Custom C++ application framework providing the foundation for both the emulator and debugger. ~155 source files.

### Build

| File | Lines | Description |
|------|------:|-------------|
| `CMakeLists.txt` | 107 | Builds `qd` static library. Controlled by `QD_USE` define (values: `EASTL;SDL2;IMGUI`). Sources all subdirectories. |
| `qd.natvis` | 95 | Visual Studio debugger visualization for qd types. |
| `va_stdafx.h` | 16 | Precompiled header for vAmiga integration. |
| `qtdDefines.h.in` | 13 | Template for generating build-specific defines. |

### Application Framework (`app/`)

| File | Lines | Description |
|------|------:|-------------|
| `app/application.h` | 68 | **`qd::Application`** base class. Owns `ModuleManager*`, `AppPartsManager*`. Main loop: `doMainLoop()` -> `onFrameUpdate()` -> `onFrameRender()`. SDL event processing. App quit management. |
| `app/application.cpp` | 126 | Implements main loop, frame timing, event dispatch, app lifecycle. |
| `app/applicationPart.h` | 116 | **`qd::ApplicationPart`** class. Inherits `qd::Node`. Modular app component with update/render phases (`EAppPartMtd` flags). Z-order, active state, part name. Lifecycle: `onPartCreate()`, `updateAppPart()`, `renderAppPart()`, `destroyImp()`. |
| `app/applicationPart.cpp` | 98 | Implements part lifecycle management, activate/deactivate, Z-order sorting. |
| `app/appPartsMgr.h` | 127 | **`qd::AppPartsManager`** class. Maintains sorted list of active ApplicationParts. Calls update/render on each part every frame. |
| `app/appPartsMgr.cpp` | 251 | Implements parts manager: add/remove parts, sorted iteration, update/render dispatch. |
| `app/moduleBase.h` | 106 | **`qd::ModuleBase`** -- base class for loadable modules with init/destroy lifecycle. |
| `app/moduleManager.h` | 343 | **`qd::ModuleManager`** class. Manages module loading/unloading, initialization order. |
| `app/moduleManager.cpp` | 347 | Implements module manager: registration, init ordering, dependency resolution. |
| `app/appMessages.h` | 59 | Application message types for inter-part communication. |

### Type Reflection System (`typeSystem/`)

| File | Lines | Description |
|------|------:|-------------|
| `typeSystem/typeDeclare.h` | 145 | Core macros: `TS_BEGIN_REFLECT_CLASS`, `TS_END`, `TS_ATTRIBUTE`, `TS_REFLECT_CLASS`. Generates type info, static type info, parent chain. `DECLARE_OPERATION_1` macro for auto-registering operations. |
| `typeSystem/typeInfo.h` | 83 | **`qd::TypeInfo`** class. Stores class name, hash, parent pointer, attributes. `isDerivedFrom()` checking. |
| `typeSystem/typeInfo.cpp` | 54 | Implements TypeInfo methods. |
| `typeSystem/typeInfoBase.h` | 93 | Base type info with attribute storage. |
| `typeSystem/typeInfoBase.cpp` | 81 | Implements attribute lookup, parent chain traversal. |
| `typeSystem/typeInfoBuilder.h` | 81 | Builder pattern for constructing TypeInfo at registration time. |
| `typeSystem/typeInfoBuilder.cpp` | 106 | Implements builder: add attributes, set parent, register with TypeRegistry. |
| `typeSystem/typeRegistry.h` | 133 | **`qd::TypeRegistry`** singleton. Global registry of all reflected types. Lookup by name hash. |
| `typeSystem/typeRegistry.cpp` | 160 | Implements type registration, lookup, iteration. |
| `typeSystem/attributesCommon.h` | 125 | Standard attributes: `Name`, `CustomClassId32`, `CreateClassCb`. Used with `TS_ATTRIBUTE()`. |
| `typeSystem/typeInfoAttrBase.h` | 32 | Base class for type info attributes. |
| `typeSystem/typeId.h` | 51 | Type ID representation (hash-based). |
| `typeSystem/stdTypeId.h` | 59 | Standard type ID helpers for built-in types. |

### UI Operation System (`qui/`)

| File | Lines | Description |
|------|------:|-------------|
| `qui/uiOperation.h` | 164 | **`IOperationEnvironment`** interface. `applyOperationMsgProc()`, `setupDefaultOperationArgs()`, `getOpEnvParent()`. `BaseOpArgs` (typed operation arguments with clone/cast). `UiOperationCreator`. Macros: `QD_REG_OPERATION`, `DECLARE_OPERATION`. |
| `qui/uiOperation.cpp` | 67 | Implements operation dispatch chain, argument casting. |
| `qui/operationsRegistry.h` | 124 | **`qd::OperationsRegistry`** singleton. Maps type IDs to operation descriptors and factory callbacks. `regOperationDesc_<T>()` for auto-registration. |
| `qui/operationsRegistry.cpp` | 117 | Implements registry: registration, creation, lookup. |
| `qui/uiNode.h` | 350 | **`qd::UiNode`** class. Base for all UI elements. Inherits `qd::Node`. Manages visibility, title, ImGui integration. Lifecycle: `onUiNodeCreated()`, `drawImp()`, `drawContentImp()`. Message handling: `onUiNodeMessageProc()`. |
| `qui/uiNode.cpp` | 299 | Implements UI node: ImGui Begin/End, visibility, child management, message routing. |
| `qui/uiMessages.h` | 19 | UI message types. |
| `qui/shortcut.h` | 77 | **`qd::Shortcut`** class. Binds ImGui key + modifiers. `addKey()`, `addModifier()`, `setRepeat()`. |
| `qui/shortcutMgr.h` | 64 | **`qd::ShortcutsMgr`** class. Manages shortcut registry, checks key bindings. |
| `qui/shortcutMgr.cpp` | 176 | Implements shortcut checking: compares current ImGui key state against registered shortcuts. |
| `qui/shortcutHnd.h` | 49 | Shortcut handler callback type. |
| `qui/controls/desktop.h` | 32 | **`qd::UiDesktop`** class. Root UI container, owns all child windows. |
| `qui/controls/desktop.cpp` | 32 | Implements desktop: creates ImGui dockspace, manages child rendering. |
| `qui/controls/window.h` | 37 | **`qd::UiWindow`** class. ImGui window wrapper with title, visibility, position. |
| `qui/controls/window.cpp` | 58 | Implements window: ImGui Begin/End, scrolling, positioning. |
| `qui/controls/dialog.h` | 21 | **`qd::UiDialog`** class. Modal/popup dialog base. |
| `qui/controls/dialog.cpp` | 13 | Implements dialog: ImGui popup modal. |
| `qui/controls/lambda.h` | 28 | Lambda-based UI control helper. |
| `qui/controls/menuItemOperation.h` | 40 | Menu item that triggers an operation when clicked. |
| `qui/comps/uiOperationMgrComp.h` | 21 | Operation manager UI component. |
| `qui/comps/uiShortcutMgrComp.h` | 25 | Shortcut manager UI component. |

### Node System (`node/`)

| File | Lines | Description |
|------|------:|-------------|
| `node/node.h` | 245 | **`qd::Node`** base class. Tree node with parent/children, named child lookup, lifecycle management. Foundation for both ApplicationPart and UiNode hierarchies. |
| `node/node.cpp` | 211 | Implements node: add/remove children, find by name, destroy. |
| `node/nodeIterator.h` | 43 | Iterator for traversing node children. |

### Threading (`thread/`)

| File | Lines | Description |
|------|------:|-------------|
| `thread/thread.h` | 156 | **`qd::Thread`** class. SDL/POSIX thread wrapper. `qd::ThreadEvent` for synchronization. `QD_THREAD_FUNC` macro. |
| `thread/thread.cpp` | 4 | Dispatches to platform implementation. |
| `thread/threadImplSdl2.cpp` | 250 | SDL2 thread implementation: create, join, set name, ThreadEvent (SDL condvar). |
| `thread/threadImplPosix.cpp` | 193 | POSIX thread implementation: pthread_create/join, condvar for ThreadEvent. |
| `thread/mutex.h` | 136 | **`qd::Mutex`** class. SDL mutex wrapper with RAII lock/unlock. |

### STL Wrappers (`stl/`)

Thin wrappers around EASTL providing `qtd::` namespace aliases. Key files:

| File | Lines | Description |
|------|------:|-------------|
| `stl/ref_ptr.h` | 957 | **`qtd::ref_ptr<T>`** -- custom reference-counted smart pointer. Largest single file in qd. |
| `stl/string.h` | 205 | `qtd::string` -- EASTL string with format helpers. |
| `stl/fixed_vector.h` | 26 | `qtd::fixed_vector` -- inline-storage vector (no heap for small N). |
| `stl/fixed_function.h` | 20 | `qtd::fixed_function` -- inline-storage function wrapper. |
| `stl/span.h` | 39 | `qtd::span` -- non-owning view. |
| `stl/optional.h` | 30 | `qtd::optional` -- std::optional equivalent. |
| `stl/array.h` | 17 | `qtd::array` -- compile-time fixed array. |
| `stl/vector.h` | 18 | `qtd::vector` -- EASTL vector. |
| `stl/unique_ptr.h` | 28 | `qtd::unique_ptr` -- std::unique_ptr equivalent. |
| `stl/eastl.h` | 10 | EASTL include bridge. |
| `stl/eastl.cpp` | 87 | EASTL initialization, allocator configuration. |

(Plus 16 more wrapper headers: `algorithm.h`, `atomic.h`, `bitvector.h`, `deque.h`, `functional.h`, `list.h`, `map.h`, `memory.h`, `set.h`, `stlUtils.h`, `unordered_map.h`, `unordered_set.h`, `utility.h`, `vector_map.h`, `vector_set.h`)

### Base Types (`base/`)

Core utility types (23 header files). Key files:

| File | Lines | Description |
|------|------:|-------------|
| `base/base.h` | 81 | Foundation: `QD_SINGLETON_DECLARE`, `FORWARD_DECLARATION_*` macros, `c_def()` (not-null check), `ref_ptr` forward. |
| `base/baseTypes.h` | 34 | `AddrRef` typedef, basic type aliases. |
| `base/variant16.h` | 297 | `qd::Var16` -- 16-bit variant (signed/unsigned/hex display). |
| `base/Color.h` | 184 | `qd::Color` -- RGB color class with float/int access. |
| `base/compiler.h` | 165 | Compiler detection, platform macros, attribute helpers. |
| `base/eFlow.h` | 42 | `qd::EFlow` enum -- operation processing return type. |
| `base/ptr.h` | 167 | Pointer utilities, checked casts. |
| `base/endian.h` | 89 | Endianness detection and byte swapping. |
| `base/Guid.h` | 227 | UUID/Guid type. |
| `base/stringId.h` | 109 | String interning / hashed string IDs. |
| `base/classPrimeId.h` | 104 | Prime-number-based class ID system. |
| `base/primesArray.h` | 70 | Precomputed prime number table. |
| `base/classIdCC.h` | 60 | Four-character code class ID (`_MAKE4C`). |
| `base/classInfoReg.h` | 76 | Class info registration helper. |

(Plus `Color4.h`, `IRandom.h`, `InitOnDemand.h`, `RandomAlgos.h`, `Tribool.h`, `defForEach.h`, `eTrisult.h`)

### Other qd Subdirectories

| Directory | Files | Description |
|-----------|------:|-------------|
| `imGui/` | 11 | ImGui integration: `QImGuiContext`, SDL2/Win32/DX11/OpenGL backends, style system (`styleDark.cpp`), helper classes. |
| `enum/` | 2 | `enumBase.h` (enum declaration macros with flags support), `enumToString.h` (enum-to-string utilities). |
| `math/` | 9 | Point2, Point3, Point4, BBox2, TMatrix, fixedPoint, easingFuncs, mathBase. |
| `mem/` | 6 | fnvHash, memAlloc, memBuffer, ptrMath. Memory allocation and buffer management. |
| `file/` | 11 | File I/O: fileBase, memFile, archiveBase (1.3K lines), archiveSerializer, INodeElement. Cross-platform file access. |
| `log/` | 2 | Logging system with priority levels. |
| `debug/` | 4 | Assert macros, exception handling. |
| `txtNodes/` | 1 | TinyXML2 integration for text nodes. |


---

## `libs/uae_lib/` -- UAE Core (Git Submodule)

The WinUAE/FS-UAE emulator core, included as a git submodule. Contains the full M68K CPU emulation, Agnus/Denise/Paula custom chip emulation, floppy/hardfile subsystem, and all Amiga hardware modeling. Not documented file-by-file here as it is a third-party codebase.

Key integration points (Quaesar accesses these):
- `regs` global -- CPU register state (D0-D7, A0-A7, PC, SR, etc.)
- `custom_regs` array -- Amiga custom chip registers
- `addrmap[]` -- Memory map descriptor
- `allocated_chipmem` / `allocated_fastmem` / `allocated_bogomem` -- RAM sizes
- `activate_debugger()` -- Break into debugger
- `debug_dma_*` -- DMA debug trace API
- `changedaddr[]` -- Change detection for disassembly caching

---

## `libs/vAmiga/` -- vAmiga Core (Git Submodule)

The vAmiga C++ emulator core by Dirk W. Hoffmann, included as a git submodule. Modern C++ codebase with public API for all Amiga subsystems. Not documented file-by-file.

Key integration points (Quaesar accesses these):
- `vamiga::VAmiga` -- Root emulator instance
- `vamiga::Amiga` -- Amiga model
- `vamiga::CPUInfo` -- CPU state snapshot
- `vamiga::Denise` -- Screen buffer access
- `vamiga::FloppyDisk` -- Disk image handling

---

## External Dependencies (`external/`)

Third-party libraries included as source, not git submodules.

| Directory | Description |
|-----------|-------------|
| `EASTL/` | **EA STL** -- Electronic Arts' standard template library. Provides `fixed_vector`, `fixed_set`, `intrusive_list`, `span`, etc. Used throughout qd framework. |
| `capstone/` | **Capstone** disassembly framework. Provides M68K instruction decoding for the debugger's disassembly engine. Headers + prebuilt libraries. |
| `cli11/` | **CLI11** command-line parser (header-only). Used in `qsr_main.cpp` for argument parsing. |
| `dear_imgui/` | **Dear ImGui** immediate-mode GUI toolkit. Core rendering for all debugger windows and emulator overlays. |
| `nlohmann/` | **nlohmann/json** JSON parser (header-only). Used for configuration serialization. |
| `nativefiledialog-extended/` | **Native File Dialog Extended**. OS-native file open/save dialogs for ADF selection. |
| `sdl2/` | **SDL2** headers. Cross-platform window, input, audio, threading. Core platform abstraction. |
| `zlib/` | **zlib** compression library. Used by ADFlib for DMS/ADZ decompression. |
| `ADFlib/` | **ADFlib** Amiga Disk File library. ADF image reading/writing, filesystem access. |

---

## `data/` -- Static Resources

| File | Description |
|------|-------------|
| `static/SourceCodePro-Black.ttf` | Source Code Pro monospace font (Black weight). |
| `static/SourceCodePro-BlackItalic.ttf` | Source Code Pro (Black Italic). |
| `static/SourceCodePro-Bold.ttf` | Source Code Pro (Bold). |
| `static/SourceCodePro-BoldItalic.ttf` | Source Code Pro (Bold Italic). |
| `static/SourceCodePro-ExtraBold.ttf` | Source Code Pro (ExtraBold). |
| `static/SourceCodePro-ExtraBoldItalic.ttf` | Source Code Pro (ExtraBold Italic). |
| `static/SourceCodePro-ExtraLight.ttf` | Source Code Pro (ExtraLight). |
| `static/SourceCodePro-ExtraLightItalic.ttf` | Source Code Pro (ExtraLight Italic). |
| `static/SourceCodePro-Italic.ttf` | Source Code Pro (Italic). |
| `static/SourceCodePro-Light.ttf` | Source Code Pro (Light). |
| `static/SourceCodePro-LightItalic.ttf` | Source Code Pro (Light Italic). |
| `static/SourceCodePro-Medium.ttf` | Source Code Pro (Medium). |
| `static/SourceCodePro-MediumItalic.ttf` | Source Code Pro (Medium Italic). |
| `static/SourceCodePro-Regular.ttf` | Source Code Pro (Regular) -- primary UI font. |
| `static/SourceCodePro-SemiBold.ttf` | Source Code Pro (SemiBold). |
| `static/SourceCodePro-SemiBoldItalic.ttf` | Source Code Pro (SemiBold Italic). |
| `OFL.txt` | SIL Open Font License for Source Code Pro. |
| `README.txt` | Font attribution notice. |

---

## `bin/` -- Binary Tools

| File | Description |
|------|-------------|
| `quaesar.png` | Application icon. |
| `install/default_layout.ini` | Default window layout for initial installation. |
| `win/cmake/` | Bundled CMake 3.28 for Windows builds (cmake.exe + modules + templates). |
| `win/clang-format.exe` | Bundled clang-format for Windows code formatting. |

---

## `resources/` -- Application Resources

| File | Description |
|------|-------------|
| `default_layout.ini` | Default ImGui layout configuration. Docked window positions and sizes for first launch. |

---

## `scripts/` -- Build Scripts

| File | Description |
|------|-------------|
| `cmake/bin2c.cmake` | CMake function to embed binary files as C byte arrays (for resource compilation). |
| `cmake/compile_options.cmake` | Compiler flag configuration: warning levels, optimization, C++20 enforcement. |
| `cmake/format_sources.cmake` | Source file listing for clang-format targets. |
| `open_vs_solution.bat` | Windows helper to open the generated Visual Studio solution. |

---

## `.github/workflows/` -- CI/CD

| File | Description |
|------|-------------|
| `ci.yml` | GitHub Actions CI pipeline. Builds on multiple platforms (Windows, macOS, Linux). Runs compilation checks. |
| `format_check.yml` | CI job to verify code formatting compliance using clang-format. |

---

## Configuration Files

| File | Description |
|------|-------------|
| `src/.clang-format` | Clang-format style configuration for the Quaesar source tree. |
| `src/.editorconfig` | EditorConfig for consistent coding style across editors. |
| `.vscode/launch.json` | VS Code debug launch configurations. |
| `.vscode/settings.json` | VS Code workspace settings (formatter, include paths). |

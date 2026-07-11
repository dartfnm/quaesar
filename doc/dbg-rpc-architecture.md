# Debugger ↔ Emulator RPC Architecture

This document describes the in-process communication ("RPC") between the Quaesar debugger and the emulator backends (UAE, vAmiga).

There is **no network or socket-based RPC**. All communication is in-process, cross-thread, using direct shared-memory access and a mutex-protected operation queue.


## Thread Model

```
Main Thread                              Emulator Thread
──────────────                           ───────────────
DebuggerApp (ImGui UI)                   UaeServerThread / VAmServerThread
Debugger (engine)                        │
 │                                       ├── UAE real_main() / vAmiga loop
 │   holds ref_ptr<IVmDbgServiceBridge> ─┤   (CPU, chipset, audio, disk I/O)
 │         │                             │
 │   getClientVm() ──► IVm::VM* ────────►├── UaeVmImp / VAmVmImp
 │                                       │
 └── pushOperationMsg() ──mutex queue──► m_pClientOpsStack[]
                                         │
                          onHandleEvents() drains queue each frame:
                            vm->applyOperationMsgProc(op)
```

Each emulator runs in a dedicated SDL thread. The debugger runs on the main thread with ImGui. They share a `ref_ptr<IVm::VM>` through the bridge interface.


## Three Communication Channels

### Channel 1: Direct Shared-Memory VM Access (no serialization)

The debugger obtains a `IVm::VM*` pointer via `IVmDbgServiceBridge::getClientVm()`. This is the **same object** the emulator thread uses — no copies, no serialization.

```cpp
// dbgConnection.h
class IVmDbgServiceBridge : public qd::RefCounted {
    virtual ref_ptr<IVm::VM> getClientVm() = 0;  // VM for debugger reads
    virtual ref_ptr<IVm::VM> getServerVm() = 0;  // VM for emulator writes
};
```

Both backends return the same VM pointer for client and server:

```cpp
// UAE: src/quasar_app/uae_imp/uae_server_app_part.cpp
class UaeSharedConnectionImpl : public amD::IVmDbgServiceBridge {
    ref_ptr<IVm::VM> m_pUaeVm;
    virtual ref_ptr<IVm::VM> getClientVm() override { return m_pUaeVm; }
    virtual ref_ptr<IVm::VM> getServerVm() override { return m_pUaeVm; }
};
```

**How the debugger uses it** (`debugger.cpp`):

```cpp
void Debugger::setDbgServiceBridge(ref_ptr<IVmDbgServiceBridge> pCon) {
    m_pConnection = pCon;
    m_pVm = m_pConnection->getClientVm();  // direct pointer, shared with emulator thread
}
```

**What goes through direct access:**

| API | Description | Thread Safety |
|-----|-------------|---------------|
| `vm->cpu->getRegA(i)` | Read address register | Unsafe (relies on emulator paused) |
| `vm->cpu->getRegD(i)` | Read data register | Unsafe (relies on emulator paused) |
| `vm->cpu->getPC()` | Read program counter | Unsafe (relies on emulator paused) |
| `vm->mem->getRealAddr(addr)` | Get host pointer to Amiga memory | Unsafe (relies on emulator paused) |
| `vm->mem->getU16(addr)` | Read 16-bit value | Unsafe (relies on emulator paused) |
| `vm->mem->setU16(addr, val)` | Write 16-bit value | Unsafe (relies on emulator paused) |
| `vm->mem->getU32(addr)` | Read 32-bit value | Unsafe (relies on emulator paused) |
| `vm->setVmDebugMode(...)` | Set debug/break/live mode | Unsafe (relies on emulator paused) |
| `vm->fetchStateFromEmu()` | Snapshot current emulator state | Unsafe (relies on emulator paused) |
| `vm->custom->getRegVal(reg)` | Read custom chipset register | Unsafe (relies on emulator paused) |
| `vm->copper->getCopperAddr(n)` | Read copper pointer | Unsafe (relies on emulator paused) |
| `vm->blitter->isBlitterActive()` | Check blitter state | Unsafe (relies on emulator paused) |

All direct access is **not thread-safe by itself**. It works because the debugger only performs these reads when the emulator is paused at a breakpoint or in step mode (`EVmDebugMode::Break`).

The underlying implementation maps directly to UAE's native internals:

```cpp
// uae_vm_imp.cpp -- direct calls into UAE's global state
uint16_t UaeVmImp::Memory::getU16(AddrRef addr) {
    return do_get_mem_word(get_real_address(addr));  // UAE: direct memory access
}
uint32_t UaeVmImp::Cpu::getRegA(int i) const {
    return m68k_areg(regs, i);  // UAE: global regs struct
}
```


### Channel 2: Queued Operation Messages (thread-safe)

For operations that **must execute on the emulator thread** (changing emulator state, stepping, console commands), the debugger uses a mutex-protected message queue.

**The message type** (`uiOperation.h`):

```cpp
namespace qd::operation {
class BaseOpArgs {
    TS_REFLECT_CLASS(BaseOpArgs, void);
public:
    virtual BaseOpArgs* clone();
    template<class T> T* cast_();  // type-safe downcast via TypeInfo
};
}
```

**Sending a message** (main thread → emulator thread):

```cpp
// IVmClientPlayer interface (qsr_app_interfaces.h)
class IVmClientPlayer {
    virtual void pushOperationMsg(qtd::unique_ptr<qd::operation::BaseOpArgs> args) = 0;
};
```

```cpp
// UaeServerThread::pushOperationMsg() -- called from main thread
void UaeServerThread::pushOperationMsg(qtd::unique_ptr<qd::operation::BaseOpArgs> args) {
    qd::MutexLock ml(m_eventMutex);         // ← mutex lock
    m_pClientOpsStack.push_back(qtd::move(args));  // enqueue
}
```

**Draining the queue** (emulator thread, called from UAE/vAmiga event loop):

```cpp
// UaeServerThread::onUaeHandleEvents() -- called from emulator thread
bool UaeServerThread::onUaeHandleEvents() {
    qd::MutexLock ml(m_eventMutex);
    // ... drain SDL events ...

    while (!m_pClientOpsStack.empty()) {
        qd::operation::BaseOpArgs* pCurOpMsg = m_pClientOpsStack.front().get();
        m_pVm->applyOperationMsgProc(pCurOpMsg);  // dispatch to VM
        m_pClientOpsStack.pop_front();
    }
    return false;
}
```

**Dispatch within VM** — each backend's `UaeVmImp`/`VAmVmImp` overrides `applyOperationMsgProcImp()` to handle operation types:

```cpp
// UaeVmImp::applyOperationMsgProcImp() -- runs on emulator thread
qd::EFlow UaeVmImp::applyOperationMsgProcImp(qd::operation::BaseOpArgs* args) {
    // switch on args->getCid() (class ID from type system)
    // handle: VmEmuReset, ExecConsoleCmd, DebugTraceStart,
    //         DisasmTraceStepInto, DebugDmaOption, etc.
}
```

**Thread safety guarantee**: The mutex ensures that:
1. The main thread can enqueue operations at any time without blocking
2. The emulator thread drains all pending operations atomically at a safe point in its event loop


### Channel 3: Console Command Queue (UAE-specific)

UAE has a built-in console/debugger that accepts text commands. A separate queue bridges the Quaesar debugger to UAE's native console.

**UaeConsoleQueue** (`uae_server_thread.cpp`):

```cpp
class UaeConsoleQueue {
    std::queue<qtd::string> m_consoleCmdQueue;
    qd::ThreadEvent* m_pThreadEvent;   // signaling mechanism
    qd::Mutex* m_pMutex;
};
```

**Flow:**

```
Debugger (main thread)              UAE thread
──────────────────                  ──────────
execConsoleCmd("W 0 DFF180 $F00")
  │
  └─► m_pConsoleQueue->addCmdToQueue(cmd)
        mutex lock → push to queue → ThreadEvent::set()
                                     │
                                     └─► uaeWaitConsoleCmdImpl()
                                           ThreadEvent::wait(100)
                                           mutex lock → pop from queue
                                           return cmd text
                                         (UAE executes command natively)
```

Used primarily for:
- `execConsoleCmd("q")` — quit UAE
- Memory writes, register modifications that go through UAE's built-in debugger


## Defined Operations

All operations are in `debuggerOps.h`. Each is a struct inheriting `amD::operation::OperationArgs`:

| Operation | Purpose | Has Shortcut |
|-----------|---------|-------------|
| `VmEmuReset` | Hard reset the Amiga | Yes |
| `ExecConsoleCmd` | Execute UAE console command (carries `qtd::string cmd`) | No |
| `DebugDmaOption` | Change DMA debug display mode | No |
| `DebugTraceStart` | Enter trace/step mode | Yes |
| `DebugTraceContinue` | Continue from breakpoint | Yes |
| `DisasmTraceStepInto` | Step one instruction | Yes |
| `DisasmTraceStepOut` | Step out of current function | Yes |
| `CopperTraceStep` | Step one copper instruction | Yes |
| `DisasmToggleBreakpoint` | Toggle breakpoint at address | Yes |
| `CopperToggleBreakpoint` | Toggle copper breakpoint | Yes |
| `ToggleTurboEmulation` | Toggle turbo/max speed | Yes |
| `DebugWaitScanLines` | Wait N scanlines before next step | Yes |
| `VmPlayerWndAlwaysOnTop` | Toggle emulator window always-on-top | Yes |


## Operation Dispatch Chain

The `IOperationEnvironment` hierarchy forms a chain-of-responsibility pattern:

```
DebuggerApp (top-level environment)
  └─ Debugger (core engine, holds VM bridge)
       └─ IVm::VM (virtual machine interface)
            └─ UaeVmImp / VAmVmImp (backend implementation)
                 └─ UAE globals / vAmiga API (actual emulator state)
```

When an operation is dispatched:

```cpp
// uiOperation.cpp
qd::EFlow IOperationEnvironment::applyOperationMsgProc(BaseOpArgs* args) {
    // 1. Try self
    EFlow f = applyOperationMsgProcImp(args);
    if (f.isDone()) return f;

    // 2. Walk up parent chain
    IOperationEnvironment* parent = getOpEnvParent();
    while (parent) {
        f = parent->applyOperationMsgProcImp(args);
        if (f.isDone()) return f;
        parent = parent->getOpEnvParent();
    }
    return EFlow::NO_RESULT;
}
```

For debugger → VM operations, the debugger directly calls:
```cpp
// debugger.cpp
EFlow Debugger::applyOperationMsgProcImp(BaseOpArgs* args) {
    return m_pVm->applyOperationMsgProcImp(args);  // direct, no parent chain
}
```


## Key Source Files

| File | Role |
|------|------|
| `libs/qd/qui/uiOperation.h` | `IOperationEnvironment`, `BaseOpArgs`, `DECLARE_OPERATION` macros |
| `libs/qd/qui/uiOperation.cpp` | Operation dispatch chain (applyOperationMsgProc) |
| `libs/amDebugger/src/amDebugger/debuggerOps.h` | All debugger operation types |
| `libs/amDebugger/src/amDebugger/debugger.h/cpp` | Debugger engine, holds VM bridge |
| `libs/amDebugger/src/amDebugger/dbgConnection.h` | `IVmDbgServiceBridge` interface |
| `libs/amDebugger/src/amDebugger/vm/vmInterface.h` | `IVm::VM` abstraction (CPU, Memory, etc.) |
| `src/quasar_app/qsr_app_interfaces.h` | `IVmClientPlayer` (pushOperationMsg interface) |
| `src/quasar_app/uae_imp/uae_server_thread.h/cpp` | UAE thread, operation queue, console queue |
| `src/quasar_app/uae_imp/uae_vm_imp.h/cpp` | UAE VM implementation (maps to UAE internals) |
| `src/quasar_app/uae_imp/uae_server_app_part.cpp` | UAE bridge + plugin factory |
| `libs/vAmiga_imp_lib/src/qvAmigaImp/va_server_thread.h/cpp` | vAmiga thread, same queue pattern |
| `libs/vAmiga_imp_lib/src/qvAmigaImp/va_server_app_part.cpp` | vAmiga bridge + plugin factory |


## Summary

| Aspect | Channel 1: Direct VM | Channel 2: Operation Queue | Channel 3: Console Queue |
|--------|---------------------|---------------------------|-------------------------|
| **Thread safety** | None (emulator must be paused) | Mutex-protected | Mutex + ThreadEvent |
| **Direction** | Bidirectional | Main → Emulator | Main → Emulator |
| **Payload** | Method calls on `IVm::VM` | `BaseOpArgs` subtypes | Raw string commands |
| **Latency** | Immediate | Next emulator event loop | Next `wait(100ms)` poll |
| **Use case** | Read registers, memory, state | Step, reset, breakpoints | UAE-native console cmds |
| **Backend** | Both UAE + vAmiga | Both UAE + vAmiga | UAE only |

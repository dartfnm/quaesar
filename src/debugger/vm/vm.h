#pragma once
#include <EASTL/span.h>
#include <debugger/vm/custom_regs.h>
#include <debugger/vm/vm_defs.h>
#include <src/generic/types.h>

namespace qd {

//////////////////////////////////////////////////////////////////////////
//: public vm::imp::UaeEmuVmImp
class VM {
protected:
    int amiga_width = (754 + 7) & ~7;
    int amiga_height = 576;
    bool mInit = false;
    static VM* staticVmInst;
    VM();

public:
    static VM* get() {
        return VM::staticVmInst;
    }
    static VM* setVmInst(VM* vm_inst) {
        VM::staticVmInst = vm_inst;
        return VM::staticVmInst;
    }
    static void destrotVmInst() {
        VM* oldVm = VM::staticVmInst;
        VM::staticVmInst = nullptr;
        delete oldVm;
    }
    virtual ~VM();

    virtual void init() = 0;

    int getScreenSizeX() const {
        return amiga_width;
    }
    int getScreenSizeY() const {
        return amiga_height;
    }

    class Memory { /*: public vm::imp::UaeEmuVmImp::Memory*/
    public:
        eastl::fixed_vector<qd::MemBank, 8, false> banks;

    public:
        const qd::MemBank* getBankByInd(int ind) const {
            if (ind < banks.size())
                return &banks[ind];
            return nullptr;
        }
        eastl::span<const qd::MemBank> getBanks() const {
            return banks;
        }
        virtual uint8_t* getRealAddr(AddrRef ptr) = 0;
        virtual bool getU16(AddrRef addr, uint16_t* out) = 0;
        virtual uint16_t getU16(AddrRef addr) = 0;
        virtual void setU16(AddrRef addr, uint16_t v) = 0;
        virtual uint32_t getU32(AddrRef addr) = 0;
        virtual void setU32(AddrRef addr, uint32_t v) = 0;

    };  // struct Memory
    qd::VM::Memory* mem = nullptr;


    class Cpu {
    public:
        virtual uint32_t getRegA(int i) const = 0;
        virtual uint32_t getRegD(int i) const = 0;
        virtual AddrRef getPC() const = 0;
        virtual bool getFlg(CpuFlg_ f) const = 0;
        virtual int getIntMask() const = 0;
    };  // struct Cpu
    qd::VM::Cpu* cpu = nullptr;

    class CustomRegs {
    public:
        virtual void fetch() = 0;
        virtual void commit() = 0;

        virtual uint16_t getRegVal(CustReg reg) = 0;
        virtual void setRegVal(CustReg reg, uint16_t new_val) = 0;

    };  // class CustomRegs
    qd::VM::CustomRegs* custom = nullptr;


    class Copper {
    public:
        virtual void fetch() = 0;
        virtual AddrRef getCopperAddr(CopperAddr_ copno) = 0;
    };  // class Copper
    qd::VM::Copper* copper = nullptr;


    struct Blitter {
    public:
        virtual bool isBlitterActive() const = 0;
        virtual void* getScreenPixBuf(int mon_id, int* out_size_w, int* out_size_h, int* pitch) = 0;
    };  // Blitter
    qd::VM::Blitter* blitter = nullptr;


};  // class VM
};  // namespace qd

#pragma once
// clang-format off
#include <src/sysconfig.h>
#include <uae_src/include/sysdeps.h>
#include <uae_src/include/options.h>
#include <uae_src/include/memory.h>
#include <uae_src/include/newcpu.h>
// clang-format on
#include <EASTL/array.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/span.h>
#include <EASTL/vector.h>
#include <debugger/vm/custom_regs.h>
#include <debugger/vm/memory.h>
#include <debugger/vm/vm.h>
#include <debugger/vm/vm_defs.h>
#include <src/generic/color.h>
#include <src/generic/types.h>


namespace qd::vm {
namespace imp {

class UaeEmuVmImp final : public VM {
public:
    UaeEmuVmImp();
    virtual void init() override;


    struct Cpu : public VM::Cpu {
        uint32_t getRegA(int i) const {
            return m68k_areg(::regs, i);
        }
        uint32_t getRegD(int i) const {
            return m68k_dreg(regs, i);
        }
        AddrRef getPC() const {
            return m68k_getpc();
        }

        virtual bool getFlg(CpuFlg_ f) const override {
            switch (f) {
                case qd::CpuFlg_Z:
                    return GET_ZFLG();
                case qd::CpuFlg_C:
                    return GET_CFLG();
                case qd::CpuFlg_V:
                    return GET_VFLG();
                case qd::CpuFlg_N:
                    return GET_NFLG();
                case qd::CpuFlg_X:
                    return GET_XFLG();
                default:
                    return false;
            }
        }
        virtual int getIntMask() const override {
            return regs.intmask;
        }
    };  // struct Cpu
    Cpu instCpu;

    ///
    struct Memory final : public VM::Memory {
    public:
        virtual uint8_t* getRealAddr(AddrRef ptr) override {
            return (uint8_t*)::memory_get_real_address(ptr);
        }
        virtual bool getU16(AddrRef addr, uint16_t* out) override {
            *out = (uint16_t)::memory_get_word(addr);
            return true;
        }
        virtual uint16_t getU16(AddrRef addr) override {
            return (uint16_t)::memory_get_word(addr);
        }
        virtual void setU16(AddrRef addr, uint16_t v) override {
            ::memory_put_word(addr, v);
        }
        virtual uint32_t getU32(AddrRef addr) override {
            return (uint32_t)::memory_get_long(addr);
        }
        virtual void setU32(AddrRef addr, uint32_t v) override {
            ::memory_put_long(addr, v);
        }
    };  // struct Memory
    Memory instMemory;

    //
    struct Blitter final : public VM::Blitter {
    public:
        virtual bool isBlitterActive() const override;
        virtual void* getScreenPixBuf(int mon_id, int* out_size_w, int* out_size_h, int* pitch) override;
    } instBlitter;


    //
    class CustomRegs final : public VM::CustomRegs {
        static constexpr size_t data_offset = 2;
        eastl::array<uint16_t, CustReg::_COUNT_ + data_offset> regsData;

    public:
        void fetch() override;
        void commit() override;

        uint16_t getRegVal(CustReg reg) override {
            return regsData[(size_t)reg + data_offset];
        }
        void setRegVal(CustReg reg, uint16_t new_val) override {
            regsData[(size_t)reg + data_offset] = new_val;
        }
    };  // class CustomRegs
    CustomRegs instCustomRegs;


    class Copper final : public VM::Copper {
    public:
        virtual void fetch() override;
        virtual AddrRef getCopperAddr(CopperAddr_ copno) override;
    };  // class Copper
    Copper instCopper;


};  // class UaeEmuVM
//////////////////////////////////////////////////////////////////////////


};  // namespace imp

};  // namespace qd::vm

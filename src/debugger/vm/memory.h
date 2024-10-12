#pragma once
#include <EASTL/string.h>
#include <stdint.h>

namespace qd {

typedef uint32_t AddrRef;

class MemBank {
public:
    enum {
        KICK = 0,
        CHIP = 1,
    };

public:
    int id = -1;
    AddrRef startAddr = 0;
    uint32_t size = 0;
    uint32_t mask = 0;
    eastl::string name;
    eastl::string label;
    uint8_t* realAddr;

public:
};  // class MemBank
//////////////////////////////////////////////////////////////////////////


struct EReg {
    enum Type : uint8_t {
        Dx = 0,
        Ax = 8,
        PC = 16,
        USP = 17,
        MSP = 18,
        ISP = 19,
        VBR = 20,
        SR = 21,
        CCR = 22,
        CACR = 23,
        CAAR = 24,
        SFC = 25,
        DFC = 26,
        TC = 27,
        ITT0 = 28,
        ITT1 = 29,
        DTT0 = 30,
        DTT1 = 31,
        BUSC = 32,
        PCR = 33,
        FPIAR = 34,
        FPCR = 35,
        FPSR = 36,
        MAX_COUNT = 37,
    };  // enum Type

    Type mV = MAX_COUNT;

    EReg() = default;
    EReg(Type v) : mV(v) {
    }
};  // struct EReg


// CPU Status Registers with Condition Code Register (CCR)
struct ECpuFlg {
    enum Type : uint8_t {
        C = 0,  // Carry
        V = 1,  // oVerflow
        Z = 2,  // Zero
        N = 3,  // Negative
        X = 4,  // eXtended

        I0 = 8,   // Interrupt priority mask bit 1
        I1 = 9,   // Interrupt priority mask bit 2
        I2 = 10,  // Interrupt priority mask bit 3
        M = 12,   // Master/Interrupt switch. Determines which stack mode to use if S is set
        S = 13,   // Supervisor Mode flag. If clear, SP refers to UserStack(USP) or SystemStack (SSP)
        T0 = 14,  // Trace bit 1. If set, trace on change of program flow
        T1 = 15,  // Trace bit 2. If set, trace is allowed on any instruction
        STOPPED,

        MAX_COUNT,
    };

    Type mV = MAX_COUNT;

    ECpuFlg() = default;
    ECpuFlg(Type v) : mV(v) {
    }
};  // ECpuFlg


};  // namespace qd

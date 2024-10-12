#include "custom_regs.h"
#include <EASTL/array.h>
#include "custom_regs_list.h"

namespace qd {

eastl::array<const qd::CustomFlagsDesc*, CustReg::_COUNT_> cust_flags_desc = {};


static constexpr int CD_WO = 1;
static constexpr int CD_ECS_AGNUS = 2;
static constexpr int CD_ECS_DENISE = 4;
static constexpr int CD_AGA = 8;
static constexpr int CD_NONE = 16;
static constexpr int CD_DMA_PTR = 32;
static constexpr int CD_COLOR = 64;

#define __ ,
CustReg::Data CustReg::cust_reg_data[_COUNT_] = {
#define REG_TO_LIST(id, addr, specials, mask, desc) {#id, addr, specials, mask, desc},
    QDB_CUSTOM_REGS_LIST(REG_TO_LIST)
#undef REG_TO_LIST
};
#undef __


const qd::CustomFlagsDesc* CustReg::getFlagDesc() const {
    return cust_flags_desc[mV];
}

struct CustomRegsStaticInitializer {
    CustomRegsStaticInitializer() {
        cust_flags_desc[CustReg::DMACON] = &DMAC::getFlagDesc();
        cust_flags_desc[CustReg::DMACONR] = &DMAC::getFlagDesc();
    }
};

static CustomRegsStaticInitializer initCustRegs;


qd::CustReg CustReg::getRegByAddr(AddrRef addr) {
    CustReg::Data val;
    val.addr = addr;
    auto it = eastl::binary_search_i(&CustReg::cust_reg_data[0], &CustReg::cust_reg_data[_COUNT_], val,
                                     [](const auto& a, const auto& b) { return a.addr < b.addr; });
    return it - &CustReg::cust_reg_data[0];
}


const qd::CustomFlagsDesc& DMAC::getFlagDesc() {
    static CustomFlagsDesc desc(CustReg::DMACON);
    static bool isInit = false;
    if (!isInit) {
        isInit = true;
        desc.addBit("AUD0EN", 1, 0);
        desc.addBit("AUD1EN", 1, 1);
        desc.addBit("AUD2EN", 1, 2);
        desc.addBit("AUD3EN", 1, 3);

        desc.addBit("DSKEN", 1, 4);
        desc.addBit("SPREN", 1, 5);
        desc.addBit("BLTEN", 1, 6);
        desc.addBit("COPEN", 1, 7);

        desc.addBit("BPLEN", 1, 8);
        desc.addBit("DMAEN", 1, 9);
        desc.addBit("BLTPRI", 1, 10);
        desc.addBit("BZERO", 1, 13, "Blitter logic zero status bit");
        desc.addBit("BBUSY", 1, 14, "Blitter busy status bit");
        desc.addBit("SETCLR", 1, 15);
        desc.addBit("AUD2EN", 1, 0);
        desc.addBit("AUD3EN", 1, 0);
    }
    return desc;
}


qd::CustomFlagsDesc& CustomFlagsDesc::addBit(const char* p_name, uint8_t bits_count, uint8_t shift_l,
                                             const char* p_desc /*= nullptr*/) {
    int nextNo = 0;
    if (!bits.empty()) {
        nextNo = bits.back().noEnd;
    }
    Bits& cb = bits.push_back();
    cb.name = p_name;
    cb.shiftL = shift_l;
    cb.mask = (1u << bits_count) - 1u;
    cb.noBeg = nextNo;
    cb.noEnd = nextNo + bits_count;
    cb.description = p_desc;
    return *this;
}


};  // namespace qd

#include "vm_uae_imp.h"
// clang-format off
#include <src/sysconfig.h>
#include <uae_src/include/sysdeps.h>
#include <uae_src/include/options.h>
#include <uae_src/include/keyboard.h>
#include <uae_src/include/inputdevice.h>
#include <uae_src/include/inputrecord.h>
#include <uae_src/include/keybuf.h>
#include <uae_src/include/custom.h>
#include <uae_src/include/blitter.h>
#include <uae_src/include/xwin.h>
#include <uae_src/include/drawing.h>
#include <uae_src/include/savestate.h>
#include <uae_src/include/debug.h>
// clang-format on
#include <SDL_log.h>
#include <debugger/vm/vm.h>


extern bool get_custom_color_reg(int colreg, uae_u8* r, uae_u8* g, uae_u8* b);
extern uaecptr bplpt[MAX_PLANES], bplptx[MAX_PLANES];


namespace qd::vm {
namespace imp {

UaeEmuVmImp::UaeEmuVmImp() {
    cpu = &instCpu;
    mem = &instMemory;
    custom = &instCustomRegs;
    copper = &instCopper;
    blitter = &instBlitter;
}


void UaeEmuVmImp::init() {
    if (mInit)
        return;
    mInit = true;
    qd::VM* s = (qd::VM*)(this);
    uint32_t hiAddr = 0;
    while (hiAddr < MEMORY_BANKS) {
        addrbank* uaeBank = mem_banks[hiAddr];
        if (!uaeBank->allocated_size) {
            ++hiAddr;
            continue;
        }

        bool combined = false;
        for (MemBank& existBank : s->mem->banks) {
            if (existBank.realAddr == uaeBank->baseaddr) {
                combined = true;
                hiAddr += uaeBank->allocated_size >> 16;
                break;
            }
        }
        if (combined)
            continue;

        MemBank& memBank = s->mem->banks.emplace_back();
        memBank.id = (int)s->mem->banks.size() - 1;
        memBank.name = uaeBank->name;
        memBank.label = uaeBank->label;
        memBank.startAddr = uaeBank->start;
        memBank.realAddr = uaeBank->baseaddr;
        memBank.mask = uaeBank->mask;
        memBank.size = uaeBank->allocated_size;
        hiAddr += memBank.size >> 16;
    }
}


void* UaeEmuVmImp::Blitter::getScreenPixBuf(int mon_id, int* out_size_w, int* out_size_h, int* pitch) {
    vidbuf_description* vidinfo = &adisplays[mon_id].gfxvidinfo;
    vidbuffer* vb = &vidinfo->drawbuffer;
    if (!vb || !vb->bufmem)
        return nullptr;
    *out_size_w = vb->outwidth;
    *out_size_h = vb->outheight;
    *pitch = vb->rowbytes;
    return vb->bufmem;
}


bool UaeEmuVmImp::Blitter::isBlitterActive() const {
    return blt_info.blit_main || blt_info.blit_finald || blt_info.blit_queued;
}


void UaeEmuVmImp::CustomRegs::fetch() {
    size_t dump_len;
    ::save_custom(&dump_len, (uae_u8*)regsData.data(), 1);
    for (size_t i = 0; i < regsData.size(); ++i)
        qd::swapBytes_<2>(&regsData[i]);
}


void UaeEmuVmImp::CustomRegs::commit() {
    eastl::fixed_vector<uint16_t, CustReg::_COUNT_ + data_offset, false> dst = {regsData.begin(), regsData.end()};
    uint8_t* beg = (uint8_t*)dst.begin();
    dst.erase((uint16_t*)(beg + 0x120), (uint16_t*)(beg + 0x180));
    dst.erase((uint16_t*)(beg + 0x0A0), (uint16_t*)(beg + 0x0E0));

    for (size_t i = 0; i < dst.size(); ++i)
        qd::swapBytes_<2>(&dst[i]);
    ::restore_custom((uae_u8*)dst.data());
}


qd::AddrRef UaeEmuVmImp::Copper::getCopperAddr(CopperAddr_ copno) {
    return ::get_copper_address(copno);
}


void UaeEmuVmImp::Copper::fetch() {
}


};  // namespace imp
//////////////////////////////////////////////////////////////////////////

};  // namespace qd::vm

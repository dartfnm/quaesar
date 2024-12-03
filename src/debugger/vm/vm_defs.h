#pragma once

namespace qd {

static constexpr int breakpoint_reg_end = 37;  // instead of #define BREAKPOINT_REG_END

enum CopperAddr_ {
    CopperAddr_null = 0,
    CopperAddr_cop1lc = 1,
    CopperAddr_cop2lc = 2,
    CopperAddr_vblankip = 3,
    CopperAddr_ip = -1,
};

enum CpuFlg_ {
    CpuFlg_Z,
    CpuFlg_C,
    CpuFlg_V,
    CpuFlg_N,
    CpuFlg_X,
};


enum CopperStates {
    COP_stop,
    COP_waitforever,
    COP_read1,
    COP_read2,
    COP_bltwait,
    COP_bltwait2,
    COP_wait_in2,
    COP_skip_in2,
    COP_wait1,
    COP_wait,
    COP_skip,
    COP_skip1,
    COP_strobe_delay1,
    COP_strobe_delay2,
    COP_strobe_delay3,
    COP_strobe_delay4,
    COP_strobe_delay5,
    COP_strobe_delay1x,
    COP_strobe_delay2x,
    COP_strobe_extra,  // just to skip current cycle when CPU wrote to COPJMP
    COP_start_delay
};

};  // namespace qd

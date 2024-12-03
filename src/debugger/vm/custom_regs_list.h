#pragma once
#include <cstdint>

template <typename T>
inline constexpr uint16_t du16(T a) noexcept {
    return static_cast<uint16_t>(a);
}

#define __ ,
#define QDB_CUSTOM_REGS_LIST(RG)                                                                                       \
    RG(BLTDDAT, 0xdff000, CD_NONE, {}, "Blitter dest. early read (dummy address)")                                     \
    RG(DMACONR, 0xdff002, 0, {}, "Dma control (and blitter status) read")                                              \
    RG(VPOSR, 0xdff004, 0, {}, "Read vert most sig. bits (and frame flop")                                             \
    RG(VHPOSR, 0xdff006, 0, {}, "Read vert and horiz position of beam")                                                \
    RG(DSKDATR, 0xdff008, CD_NONE, {}, "Disk data early read (dummy address)")                                         \
    RG(JOY0DAT, 0xdff00A, 0, {}, "Joystick-mouse 0 data (vert,horiz)")                                                 \
    RG(JOY1DAT, 0xdff00C, 0, {}, "Joystick-mouse 1 data (vert,horiz)")                                                 \
    RG(CLXDAT, 0xdff00E, 0, {}, "Collision data reg. (read and clear)")                                                \
    RG(ADKCONR, 0xdff010, 0, {}, "Audio,disk control register read")                                                   \
    RG(POT0DAT, 0xdff012, 0, {}, "Pot counter pair 0 data (vert,horiz)")                                               \
    RG(POT1DAT, 0xdff014, 0, {}, "Pot counter pair 1 data (vert,horiz)")                                               \
    RG(POTGOR, 0xdff016, 0, {}, "Pot pin data read")                                                                   \
    RG(SERDATR, 0xdff018, 0, {}, "Serial port data and status read")                                                   \
    RG(DSKBYTR, 0xdff01A, 0, {}, "Disk data byte and status read")                                                     \
    RG(INTENAR, 0xdff01C, 0, {}, "Interrupt enable bits read")                                                         \
    RG(INTREQR, 0xdff01E, 0, {}, "Interrupt request bits read")                                                        \
    RG(DSKPTH, 0xdff020, CD_WO | CD_DMA_PTR, {}, "Disk pointer (high 5 bits)")                                         \
    RG(DSKPTL, 0xdff022, CD_WO | CD_DMA_PTR, {}, "Disk pointer (low 15 bits)")                                         \
    RG(DSKLEN, 0xdff024, CD_WO, {}, "Disk lentgh")                                                                     \
    RG(DSKDAT, 0xdff026, CD_NONE, {}, "Disk DMA data write")                                                           \
    RG(REFPTR, 0xdff028, CD_NONE, {}, "Refresh pointer")                                                               \
    RG(VPOSW, 0xdff02A, CD_WO, {}, "Write vert most sig. bits(and frame flop)")                                        \
    RG(VHPOSW, 0xdff02C, CD_WO, {}, "Write vert and horiz pos of beam")                                                \
    RG(COPCON, 0xdff02e, CD_WO, {}, "Coprocessor control reg (CDANG)")                                                 \
    RG(SERDAT, 0xdff030, CD_WO, {}, "Serial port data and stop bits write")                                            \
    RG(SERPER, 0xdff032, CD_WO, {}, "Serial port period and control")                                                  \
    RG(POTGO, 0xdff034, CD_WO, {}, "Pot count start,pot pin drive enable data")                                        \
    RG(JOYTEST, 0xdff036, CD_WO, {}, "Write to all 4 joystick-mouse counters at once")                                 \
    RG(STREQU, 0xdff038, CD_WO, {}, "Strobe for horiz sync with VB and EQU")                                           \
    RG(STRVBL, 0xdff03A, CD_WO, {}, "Strobe for horiz sync with VB (vert blank)")                                      \
    RG(STRHOR, 0xdff03C, CD_WO, {}, "Strobe for horiz sync")                                                           \
    RG(STRLONG, 0xdff03E, CD_WO, {}, "Strobe for identification of long horiz line")                                   \
    RG(BLTCON0, 0xdff040, CD_WO, {}, "Blitter control reg 0")                                                          \
    RG(BLTCON1, 0xdff042, CD_WO, {}, "Blitter control reg 1")                                                          \
    RG(BLTAFWM, 0xdff044, CD_WO, {}, "Blitter first word mask for source A")                                           \
    RG(BLTALWM, 0xdff046, CD_WO, {}, "Blitter last word mask for source A")                                            \
    RG(BLTCPTH, 0xdff048, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source C (high 5 bits)")                         \
    RG(BLTCPTL, 0xdff04A, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source C (low 15 bits)")                         \
    RG(BLTBPTH, 0xdff04C, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source B (high 5 bits)")                         \
    RG(BLTBPTL, 0xdff04E, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source B (low 15 bits)")                         \
    RG(BLTAPTH, 0xdff050, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source A (high 5 bits)")                         \
    RG(BLTAPTL, 0xdff052, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to source A (low 15 bits)")                         \
    RG(BLTDPTH, 0xdff054, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to destn  D (high 5 bits)")                         \
    RG(BLTDPTL, 0xdff056, CD_WO | CD_DMA_PTR, {}, "Blitter pointer to destn  D (low 15 bits)")                         \
    RG(BLTSIZE, 0xdff058, CD_WO, {}, "Blitter start and size (win/width,height)")                                      \
    RG(BLTCON0L, 0xdff05A, CD_WO | CD_ECS_AGNUS, {}, "Blitter control 0 lower 8 bits (minterms)")                      \
    RG(BLTSIZV, 0xdff05C, CD_WO | CD_ECS_AGNUS, {0 __ 0x07FF __ 0x07FF}, "Blitter V size (for 15 bit vert size)")      \
    RG(BLTSIZH, 0xdff05E, CD_WO | CD_ECS_AGNUS, {0 __ 0x7FFF __ 0x7FFF}, "Blitter H size & start (for 11 bit H size)") \
    RG(BLTCMOD, 0xdff060, CD_WO, {}, "Blitter modulo for source C")                                                    \
    RG(BLTBMOD, 0xdff062, CD_WO, {}, "Blitter modulo for source B")                                                    \
    RG(BLTAMOD, 0xdff064, CD_WO, {}, "Blitter modulo for source A")                                                    \
    RG(BLTDMOD, 0xdff066, CD_WO, {}, "Blitter modulo for destn  D")                                                    \
    RG(xDFF068, 0xdff068, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF06A, 0xdff06a, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF06C, 0xdff06c, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF06E, 0xdff06e, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(BLTCDAT, 0xdff070, CD_WO, {}, "Blitter source C data reg")                                                      \
    RG(BLTBDAT, 0xdff072, CD_WO, {}, "Blitter source B data reg")                                                      \
    RG(BLTADAT, 0xdff074, CD_WO, {}, "Blitter source A data reg")                                                      \
    RG(xDFF076, 0xdff076, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF078, 0xdff078, CD_NONE, {}, "Ext logic UHRES sprite pointer and data identifier")                           \
    RG(xDFF07A, 0xdff07A, CD_NONE, {}, "Ext logic UHRES bit plane identifier")                                         \
    RG(LISAID, 0xdff07C, CD_ECS_DENISE, {}, "Chip revision level for Denise/Lisa")                                     \
    RG(DSKSYNC, 0xdff07E, CD_WO, {}, "Disk sync pattern reg for disk read")                                            \
    RG(COP1LCH, 0xdff080, CD_WO | CD_DMA_PTR, {}, "Coprocessor first location reg (high 5 bits)")                      \
    RG(COP1LCL, 0xdff082, CD_WO | CD_DMA_PTR, {}, "Coprocessor first location reg (low 15 bits)")                      \
    RG(COP2LCH, 0xdff084, CD_WO | CD_DMA_PTR, {}, "Coprocessor second reg (high 5 bits)")                              \
    RG(COP2LCL, 0xdff086, CD_WO | CD_DMA_PTR, {}, "Coprocessor second reg (low 15 bits)")                              \
    RG(COPJMP1, 0xdff088, CD_WO, {}, "Coprocessor restart at first location")                                          \
    RG(COPJMP2, 0xdff08A, CD_WO, {}, "Coprocessor restart at second location")                                         \
    RG(COPINS, 0xdff08C, CD_NONE, {}, "Coprocessor inst fetch identify")                                               \
    RG(DIWSTRT, 0xdff08E, CD_WO, {}, "Display window start (upper left vert-hor pos)")                                 \
    RG(DIWSTOP, 0xdff090, CD_WO, {}, "Display window stop (lower right vert-hor pos)")                                 \
    RG(DDFSTRT, 0xdff092, CD_WO, {0x00FC __ 0x00FE __ 0x00FE}, "Display bit plane data fetch start.hor pos")           \
    RG(DDFSTOP, 0xdff094, CD_WO, {0x00FC __ 0x00FE __ 0x00FE}, "Display bit plane data fetch stop.hor pos")            \
    RG(DMACON, 0xdff096, CD_WO, {du16(0x87FF) __ du16(0x87FF) __ du16(0x87FF)}, "DMA control write (clear or set)")    \
    RG(CLXCON, 0xdff098, CD_WO, {}, "Collision control")                                                               \
    RG(INTENA, 0xdff09A, CD_WO, {}, "Interrupt enable bits (clear or set bits)")                                       \
    RG(INTREQ, 0xdff09C, CD_WO, {}, "Interrupt request bits (clear or set bits)")                                      \
    RG(ADKCON, 0xdff09E, CD_WO, {}, "Audio,disk,UART,control")                                                         \
    RG(AUD0LCH, 0xdff0A0, CD_WO | CD_DMA_PTR, {}, "Audio channel 0 location (high 5 bits)")                            \
    RG(AUD0LCL, 0xdff0A2, CD_WO | CD_DMA_PTR, {}, "Audio channel 0 location (low 15 bits)")                            \
    RG(AUD0LEN, 0xdff0A4, CD_WO, {}, "Audio channel 0 lentgh")                                                         \
    RG(AUD0PER, 0xdff0A6, CD_WO, {}, "Audio channel 0 period")                                                         \
    RG(AUD0VOL, 0xdff0A8, CD_WO, {}, "Audio channel 0 volume")                                                         \
    RG(AUD0DAT, 0xdff0AA, CD_WO, {}, "Audio channel 0 data")                                                           \
    RG(xDFF0AC, 0xdff0AC, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF0AE, 0xdff0AE, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(AUD1LCH, 0xdff0B0, CD_WO | CD_DMA_PTR, {}, "Audio channel 1 location (high 5 bits)")                            \
    RG(AUD1LCL, 0xdff0B2, CD_WO | CD_DMA_PTR, {}, "Audio channel 1 location (low 15 bits)")                            \
    RG(AUD1LEN, 0xdff0B4, CD_WO, {}, "Audio channel 1 lentgh")                                                         \
    RG(AUD1PER, 0xdff0B6, CD_WO, {}, "Audio channel 1 period")                                                         \
    RG(AUD1VOL, 0xdff0B8, CD_WO, {}, "Audio channel 1 volume")                                                         \
    RG(AUD1DAT, 0xdff0BA, CD_WO, {}, "Audio channel 1 data")                                                           \
    RG(xDFF0BC, 0xdff0BC, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF0BE, 0xdff0BE, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(AUD2LCH, 0xdff0C0, CD_WO | CD_DMA_PTR, {}, "Audio channel 2 location (high 5 bits)")                            \
    RG(AUD2LCL, 0xdff0C2, CD_WO | CD_DMA_PTR, {}, "Audio channel 2 location (low 15 bits)")                            \
    RG(AUD2LEN, 0xdff0C4, CD_WO, {}, "Audio channel 2 lentgh")                                                         \
    RG(AUD2PER, 0xdff0C6, CD_WO, {}, "Audio channel 2 period")                                                         \
    RG(AUD2VOL, 0xdff0C8, CD_WO, {}, "Audio channel 2 volume")                                                         \
    RG(AUD2DAT, 0xdff0CA, CD_WO, {}, "Audio channel 2 data")                                                           \
    RG(xDFF0CC, 0xdff0CC, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF0CE, 0xdff0CE, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(AUD3LCH, 0xdff0D0, CD_WO | CD_DMA_PTR, {}, "Audio channel 3 location (high 5 bits)")                            \
    RG(AUD3LCL, 0xdff0D2, CD_WO | CD_DMA_PTR, {}, "Audio channel 3 location (low 15 bits)")                            \
    RG(AUD3LEN, 0xdff0D4, CD_WO, {}, "Audio channel 3 lentgh")                                                         \
    RG(AUD3PER, 0xdff0D6, CD_WO, {}, "Audio channel 3 period")                                                         \
    RG(AUD3VOL, 0xdff0D8, CD_WO, {}, "Audio channel 3 volume")                                                         \
    RG(AUD3DAT, 0xdff0DA, CD_WO, {}, "Audio channel 3 data")                                                           \
    RG(xDFF0DC, 0xdff0DC, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(xDFF0DE, 0xdff0DE, CD_NONE, {}, "Unknown or Unused")                                                            \
    RG(BPL1PTH, 0xdff0E0, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 1 (high 5 bits)")                                 \
    RG(BPL1PTL, 0xdff0E2, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 1 (low 15 bits)")                                 \
    RG(BPL2PTH, 0xdff0E4, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 2 (high 5 bits)")                                 \
    RG(BPL2PTL, 0xdff0E6, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 2 (low 15 bits)")                                 \
    RG(BPL3PTH, 0xdff0E8, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 3 (high 5 bits)")                                 \
    RG(BPL3PTL, 0xdff0EA, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 3 (low 15 bits)")                                 \
    RG(BPL4PTH, 0xdff0EC, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 4 (high 5 bits)")                                 \
    RG(BPL4PTL, 0xdff0EE, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 4 (low 15 bits)")                                 \
    RG(BPL5PTH, 0xdff0F0, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 5 (high 5 bits)")                                 \
    RG(BPL5PTL, 0xdff0F2, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 5 (low 15 bits)")                                 \
    RG(BPL6PTH, 0xdff0F4, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 6 (high 5 bits)")                                 \
    RG(BPL6PTL, 0xdff0F6, CD_WO | CD_DMA_PTR, {}, "Bit plane pointer 6 (low 15 bits)")                                 \
    RG(BPL7PTH, 0xdff0F8, CD_WO | CD_AGA | CD_DMA_PTR, {}, "Bit plane pointer 7 (high 5 bits)")                        \
    RG(BPL7PTL, 0xdff0FA, CD_WO | CD_AGA | CD_DMA_PTR, {}, "Bit plane pointer 7 (low 15 bits)")                        \
    RG(BPL8PTH, 0xdff0FC, CD_WO | CD_AGA | CD_DMA_PTR, {}, "Bit plane pointer 8 (high 5 bits)")                        \
    RG(BPL8PTL, 0xdff0FE, CD_WO | CD_AGA | CD_DMA_PTR, {}, "Bit plane pointer 8 (low 15 bits)")                        \
    RG(BPLCON0, 0xdff100, CD_WO, {du16(~0x00F1) __ du16(~0x00B0) __ du16(~0x0080)},                                    \
       "Bit plane control reg (misc control bits)")                                                                    \
    RG(BPLCON1, 0xdff102, CD_WO, {}, "Bit plane control reg (scroll val PF1,PF2)")                                     \
    RG(BPLCON2, 0xdff104, CD_WO, {du16(0x007F) __ du16(0x01FF) __ du16(0x7FFF)},                                       \
       "Bit plane control reg (priority control)")                                                                     \
    RG(BPLCON3, 0xdff106, CD_WO | CD_ECS_DENISE, {0x003F __ 0x003F __ 0xFFFF},                                         \
       "Bit plane control reg (enhanced features)")                                                                    \
    RG(BPL1MOD, 0xdff108, CD_WO, {},                                                                                   \
       "Bit plane modulo (odd planes,or active- fetch lines if bitplane scan-doubling is enabled")                     \
    RG(BPL2MOD, 0xdff10A, CD_WO, {},                                                                                   \
       "Bit plane modulo (even planes or inactive- fetch lines if bitplane scan-doubling is enabled")                  \
    RG(BPLCON4, 0xdff10C, CD_WO | CD_AGA, {}, "Bit plane control reg (bitplane and sprite masks)")                     \
    RG(CLXCON2, 0xdff10e, CD_WO | CD_AGA, {}, "Extended collision control reg")                                        \
    RG(BPL1DAT, 0xdff110, CD_WO, {}, "Bit plane 1 data (parallel to serial con- vert)")                                \
    RG(BPL2DAT, 0xdff112, CD_WO, {}, "Bit plane 2 data (parallel to serial con- vert)")                                \
    RG(BPL3DAT, 0xdff114, CD_WO, {}, "Bit plane 3 data (parallel to serial con- vert)")                                \
    RG(BPL4DAT, 0xdff116, CD_WO, {}, "Bit plane 4 data (parallel to serial con- vert)")                                \
    RG(BPL5DAT, 0xdff118, CD_WO, {}, "Bit plane 5 data (parallel to serial con- vert)")                                \
    RG(BPL6DAT, 0xdff11a, CD_WO, {}, "Bit plane 6 data (parallel to serial con- vert)")                                \
    RG(BPL7DAT, 0xdff11c, CD_WO | CD_AGA, {}, "Bit plane 7 data (parallel to serial con- vert)")                       \
    RG(BPL8DAT, 0xdff11e, CD_WO | CD_AGA, {}, "Bit plane 8 data (parallel to serial con- vert)")                       \
    RG(SPR0PTH, 0xdff120, CD_WO | CD_DMA_PTR, {}, "Sprite 0 pointer (high 5 bits)")                                    \
    RG(SPR0PTL, 0xdff122, CD_WO | CD_DMA_PTR, {}, "Sprite 0 pointer (low 15 bits)")                                    \
    RG(SPR1PTH, 0xdff124, CD_WO | CD_DMA_PTR, {}, "Sprite 1 pointer (high 5 bits)")                                    \
    RG(SPR1PTL, 0xdff126, CD_WO | CD_DMA_PTR, {}, "Sprite 1 pointer (low 15 bits)")                                    \
    RG(SPR2PTH, 0xdff128, CD_WO | CD_DMA_PTR, {}, "Sprite 2 pointer (high 5 bits)")                                    \
    RG(SPR2PTL, 0xdff12A, CD_WO | CD_DMA_PTR, {}, "Sprite 2 pointer (low 15 bits)")                                    \
    RG(SPR3PTH, 0xdff12C, CD_WO | CD_DMA_PTR, {}, "Sprite 3 pointer (high 5 bits)")                                    \
    RG(SPR3PTL, 0xdff12E, CD_WO | CD_DMA_PTR, {}, "Sprite 3 pointer (low 15 bits)")                                    \
    RG(SPR4PTH, 0xdff130, CD_WO | CD_DMA_PTR, {}, "Sprite 4 pointer (high 5 bits)")                                    \
    RG(SPR4PTL, 0xdff132, CD_WO | CD_DMA_PTR, {}, "Sprite 4 pointer (low 15 bits)")                                    \
    RG(SPR5PTH, 0xdff134, CD_WO | CD_DMA_PTR, {}, "Sprite 5 pointer (high 5 bits)")                                    \
    RG(SPR5PTL, 0xdff136, CD_WO | CD_DMA_PTR, {}, "Sprite 5 pointer (low 15 bits)")                                    \
    RG(SPR6PTH, 0xdff138, CD_WO | CD_DMA_PTR, {}, "Sprite 6 pointer (high 5 bits)")                                    \
    RG(SPR6PTL, 0xdff13A, CD_WO | CD_DMA_PTR, {}, "Sprite 6 pointer (low 15 bits)")                                    \
    RG(SPR7PTH, 0xdff13C, CD_WO | CD_DMA_PTR, {}, "Sprite 7 pointer (high 5 bits)")                                    \
    RG(SPR7PTL, 0xdff13E, CD_WO | CD_DMA_PTR, {}, "Sprite 7 pointer (low 15 bits)")                                    \
    RG(SPR0POS, 0xdff140, CD_WO, {}, "Sprite 0 vert-horiz start pos data")                                             \
    RG(SPR0CTL, 0xdff142, CD_WO, {}, "Sprite 0 position and control data")                                             \
    RG(SPR0DATA, 0xdff144, CD_WO, {}, "Sprite 0 image data register A")                                                \
    RG(SPR0DATB, 0xdff146, CD_WO, {}, "Sprite 0 image data register B")                                                \
    RG(SPR1POS, 0xdff148, CD_WO, {}, "Sprite 1 vert-horiz start pos data")                                             \
    RG(SPR1CTL, 0xdff14A, CD_WO, {}, "Sprite 1 position and control data")                                             \
    RG(SPR1DATA, 0xdff14C, CD_WO, {}, "Sprite 1 image data register A")                                                \
    RG(SPR1DATB, 0xdff14E, CD_WO, {}, "Sprite 1 image data register B")                                                \
    RG(SPR2POS, 0xdff150, CD_WO, {}, "Sprite 2 vert-horiz start pos data")                                             \
    RG(SPR2CTL, 0xdff152, CD_WO, {}, "Sprite 2 position and control data")                                             \
    RG(SPR2DATA, 0xdff154, CD_WO, {}, "Sprite 2 image data register A")                                                \
    RG(SPR2DATB, 0xdff156, CD_WO, {}, "Sprite 2 image data register B")                                                \
    RG(SPR3POS, 0xdff158, CD_WO, {}, "Sprite 3 vert-horiz start pos data")                                             \
    RG(SPR3CTL, 0xdff15A, CD_WO, {}, "Sprite 3 position and control data")                                             \
    RG(SPR3DATA, 0xdff15C, CD_WO, {}, "Sprite 3 image data register A")                                                \
    RG(SPR3DATB, 0xdff15E, CD_WO, {}, "Sprite 3 image data register B")                                                \
    RG(SPR4POS, 0xdff160, CD_WO, {}, "Sprite 4 vert-horiz start pos data")                                             \
    RG(SPR4CTL, 0xdff162, CD_WO, {}, "Sprite 4 position and control data")                                             \
    RG(SPR4DATA, 0xdff164, CD_WO, {}, "Sprite 4 image data register A")                                                \
    RG(SPR4DATB, 0xdff166, CD_WO, {}, "Sprite 4 image data register B")                                                \
    RG(SPR5POS, 0xdff168, CD_WO, {}, "Sprite 5 vert-horiz start pos data")                                             \
    RG(SPR5CTL, 0xdff16A, CD_WO, {}, "Sprite 5 position and control data")                                             \
    RG(SPR5DATA, 0xdff16C, CD_WO, {}, "Sprite 5 image data register A")                                                \
    RG(SPR5DATB, 0xdff16E, CD_WO, {}, "Sprite 5 image data register B")                                                \
    RG(SPR6POS, 0xdff170, CD_WO, {}, "Sprite 6 vert-horiz start pos data")                                             \
    RG(SPR6CTL, 0xdff172, CD_WO, {}, "Sprite 6 position and control data")                                             \
    RG(SPR6DATA, 0xdff174, CD_WO, {}, "Sprite 6 image data register A")                                                \
    RG(SPR6DATB, 0xdff176, CD_WO, {}, "Sprite 6 image data register B")                                                \
    RG(SPR7POS, 0xdff178, CD_WO, {}, "Sprite 7 vert-horiz start pos data")                                             \
    RG(SPR7CTL, 0xdff17A, CD_WO, {}, "Sprite 7 position and control data")                                             \
    RG(SPR7DATA, 0xdff17C, CD_WO, {}, "Sprite 7 image data register A")                                                \
    RG(SPR7DATB, 0xdff17E, CD_WO, {}, "Sprite 7 image data register B")                                                \
    RG(COLOR00, 0xdff180, CD_WO | CD_COLOR, {}, "Color table 00")                                                      \
    RG(COLOR01, 0xdff182, CD_WO | CD_COLOR, {}, "Color table 01")                                                      \
    RG(COLOR02, 0xdff184, CD_WO | CD_COLOR, {}, "Color table 02")                                                      \
    RG(COLOR03, 0xdff186, CD_WO | CD_COLOR, {}, "Color table 03")                                                      \
    RG(COLOR04, 0xdff188, CD_WO | CD_COLOR, {}, "Color table 04")                                                      \
    RG(COLOR05, 0xdff18A, CD_WO | CD_COLOR, {}, "Color table 05")                                                      \
    RG(COLOR06, 0xdff18C, CD_WO | CD_COLOR, {}, "Color table 06")                                                      \
    RG(COLOR07, 0xdff18E, CD_WO | CD_COLOR, {}, "Color table 07")                                                      \
    RG(COLOR08, 0xdff190, CD_WO | CD_COLOR, {}, "Color table 08")                                                      \
    RG(COLOR09, 0xdff192, CD_WO | CD_COLOR, {}, "Color table 09")                                                      \
    RG(COLOR10, 0xdff194, CD_WO | CD_COLOR, {}, "Color table 10")                                                      \
    RG(COLOR11, 0xdff196, CD_WO | CD_COLOR, {}, "Color table 11")                                                      \
    RG(COLOR12, 0xdff198, CD_WO | CD_COLOR, {}, "Color table 12")                                                      \
    RG(COLOR13, 0xdff19A, CD_WO | CD_COLOR, {}, "Color table 13")                                                      \
    RG(COLOR14, 0xdff19C, CD_WO | CD_COLOR, {}, "Color table 14")                                                      \
    RG(COLOR15, 0xdff19E, CD_WO | CD_COLOR, {}, "Color table 15")                                                      \
    RG(COLOR16, 0xdff1A0, CD_WO | CD_COLOR, {}, "Color table 16")                                                      \
    RG(COLOR17, 0xdff1A2, CD_WO | CD_COLOR, {}, "Color table 17")                                                      \
    RG(COLOR18, 0xdff1A4, CD_WO | CD_COLOR, {}, "Color table 18")                                                      \
    RG(COLOR19, 0xdff1A6, CD_WO | CD_COLOR, {}, "Color table 19")                                                      \
    RG(COLOR20, 0xdff1A8, CD_WO | CD_COLOR, {}, "Color table 20")                                                      \
    RG(COLOR21, 0xdff1AA, CD_WO | CD_COLOR, {}, "Color table 21")                                                      \
    RG(COLOR22, 0xdff1AC, CD_WO | CD_COLOR, {}, "Color table 22")                                                      \
    RG(COLOR23, 0xdff1AE, CD_WO | CD_COLOR, {}, "Color table 23")                                                      \
    RG(COLOR24, 0xdff1B0, CD_WO | CD_COLOR, {}, "Color table 24")                                                      \
    RG(COLOR25, 0xdff1B2, CD_WO | CD_COLOR, {}, "Color table 25")                                                      \
    RG(COLOR26, 0xdff1B4, CD_WO | CD_COLOR, {}, "Color table 26")                                                      \
    RG(COLOR27, 0xdff1B6, CD_WO | CD_COLOR, {}, "Color table 27")                                                      \
    RG(COLOR28, 0xdff1B8, CD_WO | CD_COLOR, {}, "Color table 28")                                                      \
    RG(COLOR29, 0xdff1BA, CD_WO | CD_COLOR, {}, "Color table 29")                                                      \
    RG(COLOR30, 0xdff1BC, CD_WO | CD_COLOR, {}, "Color table 30")                                                      \
    RG(COLOR31, 0xdff1BE, CD_WO | CD_COLOR, {}, "Color table 31")                                                      \
    RG(HTOTAL, 0xdff1C0, CD_WO | CD_ECS_AGNUS, {}, "Highest number count in horiz line (VARBEAMEN = 1)")               \
    RG(HSSTOP, 0xdff1C2, CD_WO | CD_ECS_DENISE, {}, "Horiz line pos for HSYNC stop")                                   \
    RG(HBSTRT, 0xdff1C4, CD_WO | CD_ECS_DENISE, {}, "Horiz line pos for HBLANK start")                                 \
    RG(HBSTOP, 0xdff1C6, CD_WO | CD_ECS_DENISE, {}, "Horiz line pos for HBLANK stop")                                  \
    RG(VTOTAL, 0xdff1C8, CD_WO | CD_ECS_AGNUS, {}, "Highest numbered vertical line (VARBEAMEN = 1)")                   \
    RG(VSSTOP, 0xdff1CA, CD_WO | CD_ECS_AGNUS, {}, "Vert line for VBLANK start")                                       \
    RG(VBSTRT, 0xdff1CC, CD_WO | CD_ECS_AGNUS, {}, "Vert line for VBLANK start")                                       \
    RG(VBSTOP, 0xdff1CE, CD_WO | CD_ECS_AGNUS, {}, "Vert line for VBLANK stop")                                        \
    RG(SPRHSTRT, 0xdff1D0, CD_WO | CD_ECS_AGNUS, {}, "UHRES sprite vertical start")                                    \
    RG(SPRHSTOP, 0xdff1D2, CD_WO | CD_ECS_AGNUS, {}, "UHRES sprite vertical stop")                                     \
    RG(BPLHSTRT, 0xdff1D4, CD_WO | CD_ECS_AGNUS, {}, "UHRES bit plane vertical stop")                                  \
    RG(BPLHSTOP, 0xdff1D6, CD_WO | CD_ECS_AGNUS, {}, "UHRES bit plane vertical stop")                                  \
    RG(HHPOSW, 0xdff1D8, CD_WO | CD_ECS_AGNUS, {}, "DUAL mode hires H beam counter write")                             \
    RG(HHPOSR, 0xdff1DA, 0 | CD_ECS_AGNUS, {}, "DUAL mode hires H beam counter read")                                  \
    RG(BEAMCON0, 0xdff1DC, CD_WO | CD_ECS_AGNUS, {}, "Beam counter control register (SHRES,UHRES,PAL)")                \
    RG(HSSTRT, 0xdff1DE, CD_WO | CD_ECS_DENISE, {}, "Horizontal sync start (VARHSY)")                                  \
    RG(VSSTRT, 0xdff1E0, CD_WO | CD_ECS_DENISE, {}, "Vertical sync start (VARVSY)")                                    \
    RG(HCENTER, 0xdff1E2, CD_WO | CD_ECS_DENISE, {}, "Horizontal pos for vsync on interlace")                          \
    RG(DIWHIGH, 0xdff1E4, CD_WO | CD_ECS_AGNUS | CD_ECS_DENISE, {}, "Display window upper bits for start/stop")        \
    RG(xDFF1E6, 0xdff1E6, CD_NONE, {}, "UHRES bit plane modulo")                                                       \
    RG(xDFF1E8, 0xdff1E8, CD_NONE, {}, "UHRES sprite pointer (high 5 bits)")                                           \
    RG(xDFF1EA, 0xdff1EA, CD_NONE, {}, "UHRES sprite pointer (low 15 bits)")                                           \
    RG(xDFF1EC, 0xdff1EC, CD_NONE, {}, "VRam (UHRES) bitplane pointer (hi 5 bits)")                                    \
    RG(xDFF1EE, 0xdff1EE, CD_NONE, {}, "VRam (UHRES) bitplane pointer (lo 15 bits)")                                   \
    RG(xDFF1F0, 0xdff1F0, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(xDFF1F2, 0xdff1F2, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(xDFF1F4, 0xdff1F4, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(xDFF1F6, 0xdff1F6, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(xDFF1F8, 0xdff1F8, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(xDFF1FA, 0xdff1Fa, CD_NONE, {}, "Reserved (forever i guess!)")                                                  \
    RG(FMODE, 0xdff1FC, CD_WO | CD_AGA, {}, "Fetch mode register")                                                     \
    RG(xNULL, 0xdff1FE, CD_WO, {},                                                                                     \
       "Can also indicate last 2 or 3 refresh cycles or the restart of the COPPER after lockup.")

/*
 */
#undef __

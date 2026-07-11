# Compiler Warning Elimination TODO

**Original Warnings:** 5,620  
**Current Warnings:** 1,660  
**Eliminated:** 3,960 (70.5% reduction)  
**Files Remaining:** ~95  
**Last Updated:** 2025-05-17  

**Batches Completed:** 15 (pushed to `uae-warnings-only` branch)

---

## Completed Batches

| Batch | Warnings Fixed | Description |
|-------|---------------|-------------|
| 1-4 | ~1,000 | Initial cleanup (uae_src/src files) |
| 5 | ~180 | fdi2raw.cpp, traps.cpp, gfxutil.cpp, linetoscr.cpp |
| 6 | 29 | Final remaining warnings from initial scan |
| 7-10 | 220 | 'dummy' set-but-not-used in cpuemu files (3 attempts) |
| 11 | 138 | Unused functions with `__attribute__((unused))` |
| 12 | 1 | newcpu.cpp unused parameter |
| 13 | 249 | inputevents.def: Fix DEFEVENT/DEFEVENT2 macros (missing data2 field) |
| 14 | 250 | identify.cpp: Fix customData mask arrays + unused variable |
| 15 | 1,137 | Unused parameters across 12 files (drawing, sndboard, custom, memory, etc.) |

**Total Fixed:** 3,960 warnings (70.5% reduction)

---

## Warning Categories (by count) - REMAINING

| Category | Count | Strategy |
|----------|-------|----------|
| missing-field-initializers | ~1,100 | Add missing fields to struct initializers |
| missing-braces | ~560 | Add braces around sub-object initialization |
| unused-parameter | ~450 | Add `(void)param;` to function bodies |
| unused-but-set-variable | ~370 | Comment out or add `(void)` casts |
| sign-compare | ~200 | Cast to matching types |
| unused-variable | ~180 | Comment out or use |
| other | ~50 | Case-by-case fixes |

---

## Phase 1: Quick Wins (~2,500 warnings)

### 1. rommgr.cpp - 1,043 warnings

**Path:** `uae_src/rommgr.cpp`

| Type | Count |
|------|-------|
| missing-braces | 560 |
| missing field 'configname' | 342 |
| missing field 'sortpriority' | 127 |
| sign-compare | 7 |
| unused-parameter | 2 |
| unused-variable | 1 |
| other | 4 |

**Strategy:** Fix romdata struct initialization pattern - add `, NULL, NULL, 0` to initializers

---

### 2. expansion.cpp - 383 warnings

**Path:** `uae_src/expansion.cpp`

| Type | Count |
|------|-------|
| missing field 'type' | 75 |
| missing field 'configname' | 68 |
| missing field 'memory_mid' | 49 |
| missing field 'autoconfig' | 32 |
| missing field 'sub_banks' | 18 |
| missing field 'invert' | 16 |
| missing field 'e8' | 11 |
| missing field 'z3extra' | 10 |
| missing field 'memory_after' | 9 |
| unused-function | 48 |
| unused-variable | 20 |
| unused-parameter | 10 |
| sign-compare | 5 |
| other | 1 |

**Strategy:** Fix struct initialization, comment out unused functions/vars

---

### 3. identify.cpp - 216 warnings

**Path:** `uae_src/identify.cpp`

| Type | Count |
|------|-------|
| missing field 'mask' | 247 |
| missing field 'special' | 1 |
| missing field 'adr' | 1 |
| unused-variable | 1 |

**Strategy:** Fix ide_data_type array initialization

---

### 4. inputevents.def - 244 warnings

**Path:** `uae_src/inputevents.def`

| Type | Count |
|------|-------|
| missing field 'data2' | 249 |

**Strategy:** Fix macro or struct definition

---

### 5. cpuemu_*.cpp files - ~320 warnings

**Files:** cpuemu_0.cpp, cpuemu_11.cpp, cpuemu_13.cpp, cpuemu_20.cpp, cpuemu_21.cpp, cpuemu_22.cpp, cpuemu_23.cpp, cpuemu_24.cpp, cpuemu_31.cpp, cpuemu_32.cpp, cpuemu_33.cpp, cpuemu_34.cpp, cpuemu_35.cpp, cpuemu_40.cpp, cpuemu_50.cpp

| Variable | Total Count |
|----------|-------------|
| tmp_newv | 93 |
| pcadjust | 32 |
| src | 14 |
| oldsr | 8 |
| newsr | 2 |

**Strategy:** LAST ITEM - Fix code generator (gencpu.cpp, build68k.cpp) to not produce these patterns

---

## Phase 2: Systematic Fixes (~1,200 warnings)

### 6. Unused Parameters - 456 warnings

**Top files:**
- cfgfile.cpp: ~15
- custom.cpp: ~32
- debug.cpp: ~16
- drawing.cpp: ~17
- expansion.cpp: ~10
- filesys.cpp: ~27
- gayle.cpp: ~7
- hardfile.cpp: ~9
- idecontrollers.cpp: ~16
- inputdevice.cpp: ~29
- memory.cpp: ~30
- newcpu.cpp: ~21
- rommgr.cpp: ~2
- scsi.cpp: ~33
- sndboard.cpp: ~50
- traps.cpp: ~1

**Top patterns:**
- 22x `ncr`
- 22x `ctx`
- 20x `addr`
- 17x `hpos`
- 17x `b`
- 16x `v`
- 16x `pcibs`
- 16x `board`

**Strategy:** Add `(void)param;` at start of function bodies

---

### 7. Sign-Compare Issues - 207 warnings

| Pattern | Count |
|---------|-------|
| 'uae_u32' vs 'int' | 47 |
| 'int' vs 'uae_u32' | 44 |
| 'int' vs 'unsigned long' | 22 |
| 'uint64_t' vs 'int' | 13 |
| 'uaecptr' vs 'int' | 12 |
| 'size_t' vs 'int' | 8 |
| 'unsigned int' vs 'int' | 10 |
| 'int' vs 'unsigned int' | 10 |
| 'uint64_t' vs 'int64_t' | 7 |
| Other | 34 |

**Strategy:** Cast to matching unsigned/signed types

---

## Phase 3: Cleanup (~300 warnings)

### 8. Unused Variables - 187 warnings

Scattered across 40+ files. Comment out or use appropriately.

**Top files:**
- cfgfile.cpp: ~5
- custom.cpp: ~6
- drawing.cpp: ~7
- newcpu.cpp: ~6

---

### 9. Unused Functions - 48 warnings

**Top files:**
- expansion.cpp: 30
- linetoscr.cpp: 16
- drawing.cpp: 13
- custom.cpp: 11
- gayle.cpp: 8
- cfgfile.cpp: 8
- memory.cpp: 7

**Strategy:** Add `__attribute__((unused))` to static functions

---

### 10. Other Warnings - 17 warnings

| Type | Count |
|------|-------|
| format-mismatch | ~8 |
| other (precedence, comma, etc.) | 8 |
| null-conversion | 1 |

---

## Complete File List (Top 35 files)

| Warnings | File | CPUemu? |
|----------|------|---------|
| 1,043 | uae_src/rommgr.cpp | No |
| 383 | uae_src/expansion.cpp | No |
| 244 | uae_src/../uae_src/inputevents.def | No |
| 216 | uae_src/identify.cpp | No |
| 115 | uae_src/cpuemu_40.cpp | **Yes** |
| 105 | uae_src/filesys.cpp | No |
| 86 | uae_src/sndboard.cpp | No |
| 77 | uae_src/custom.cpp | No |
| 68 | uae_src/memory.cpp | No |
| 61 | uae_src/drawing.cpp | No |
| 60 | uae_src/inputdevice.cpp | No |
| 54 | uae_src/debug.cpp | No |
| 52 | uae_src/keybuf.cpp | No |
| 52 | uae_src/scsi.cpp | No |
| 51 | uae_src/cfgfile.cpp | No |
| 51 | uae_src/cpuemu_0.cpp | **Yes** |
| 51 | uae_src/cpuemu_50.cpp | **Yes** |
| 45 | uae_src/debugmem.cpp | No |
| 35 | uae_src/newcpu.cpp | No |
| 34 | uae_src/gayle.cpp | No |
| 32 | uae_src/idecontrollers.cpp | No |
| 27 | uae_src/hardfile.cpp | No |
| 24 | uae_src/isofs.cpp | No |
| 21 | uae_src/zfile.cpp | No |
| 20 | uae_src/disk.cpp | No |
| 17 | uae_src/fdi2raw.cpp | No |
| 16 | uae_src/linetoscr.cpp | No |
| 16 | uae_src/savestate.cpp | No |
| 16 | uae_src/scsiemul.cpp | No |
| 15 | uae_src/blkdev.cpp | No |
| 15 | uae_src/traps.cpp | No |
| 15 | uae_src/zfile_archive.cpp | No |
| 15 | uae_src/gfxutil.cpp | No |
| 13 | uae_src/newcpu_common.cpp | No |
| ... | (~120 files total) | |

---

## Fix Strategy

1. **missing-field-initializers** → Add missing fields (`, NULL, NULL, 0` or appropriate defaults)
2. **missing-braces** → Use `{{}}` instead of `{0}` for nested structs
3. **unused-parameter** → Add `(void)param;` at function start
4. **unused-but-set-variable** → Comment out or add `(void)var;` if kept for debugging
5. **sign-compare** → Add explicit casts to matching types
6. **unused-variable** → Remove or comment out
7. **unused-function** → Add `__attribute__((unused))` to static functions
8. **format-mismatch** → Fix printf format specifiers
9. **null-conversion** → Use `0` for integers, `nullptr` for pointers
10. **cpuemu files** → **LAST** - Fix code generator (gencpu.cpp, build68k.cpp)

**Rule:** NO pragma suppressions - fix warnings properly with code changes!

---

## Progress Tracking

| Phase | Target | Warnings | Status |
|-------|--------|----------|--------|
| Phase 1.1 | rommgr.cpp | 1,043 | ⏳ Pending |
| Phase 1.2 | expansion.cpp | 383 | ⏳ Pending |
| Phase 1.3 | identify.cpp | 216 | ⏳ Pending |
| Phase 1.4 | inputevents.def | 244 | ⏳ Pending |
| Phase 1.5 | filesys.cpp + others | ~500 | ⏳ Pending |
| Phase 2.1 | Unused parameters | 456 | ⏳ Pending |
| Phase 2.2 | Sign-compare | 207 | ⏳ Pending |
| Phase 3 | Cleanup | ~300 | ⏳ Pending |
| **LAST** | cpuemu generator | 320 | ⏳ Pending (fix generator) |

**Expected Final Result:** 0 warnings (after fixing generator)

---

## Completed Batches

| Batch | Warnings Fixed | Description |
|-------|---------------|-------------|
| 1-4 | ~1,000 | Initial cleanup (uae_src/src files) |
| 5 | ~180 | fdi2raw.cpp, traps.cpp, gfxutil.cpp, linetoscr.cpp |
| 6 | 29 | Final remaining warnings from initial scan |
| 7-10 | 220 | 'dummy' set-but-not-used in cpuemu files (3 attempts) |
| 11 | 138 | Unused functions with `__attribute__((unused))` |
| 12 | 1 | newcpu.cpp unused parameter |

**Total Fixed:** 2,325 warnings (41.4% reduction)

---

## How to Rebuild and Analyze

```bash
cd build
cmake --build --target clean
cmake --build -- -j$(sysctl -n hw.ncpu) 2>&1 | tee /tmp/warning_analysis.log
grep -c "warning:" /tmp/warning_analysis.log
```

## How to Regenerate This File

```bash
cd /Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng
python3 scripts/generate_todo.py  # Create this script from analysis above
```

---

## Next Steps

1. ✅ Fix all non-cpuemu warnings systematically (Phases 1-3)
2. ✅ Verify zero warnings in non-cpuemu files
3. **LAST:** Fix gencpu.cpp and build68k.cpp generators
4. Regenerate cpuemu files with warning-free code
5. Final verification: ZERO warnings across entire codebase

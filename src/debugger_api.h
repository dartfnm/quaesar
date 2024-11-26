#pragma once

#include <stdint.h>

#define DEBUGGER_API_ENABLE 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DebuggerAPI {
    void* priv_data;
    void* (*create)(void* user_data);
    void (*check_exception)(void* s);
    void (*debug_copper)(void* s, uint32_t addr, uint32_t nextaddr, uint16_t word1, uint16_t word2, int hpos, int vpos);
    void (*update)(void* s);
    void (*live_update)(void* s);
    void (*destroy)(void* s);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DebuggerAPI_has_debugger();
void DebuggerAPI_register(DebuggerAPI* api, void* user_data);
void DebuggerAPI_check_exception();
void DebuggerAPI_debug_copper(uint32_t addr, uint32_t nextaddr, uint16_t word1, uint16_t word2, int hpos, int vpos);
void DebuggerAPI_update();
void DebuggerAPI_live_update();

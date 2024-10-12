#pragma once
#include <src/generic/base.h>
#include <src/generic/thread.h>

FORWARD_DECLARATION_3(qd, thread, Event);
FORWARD_DECLARATION_2(qd, Debugger);
struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;


namespace qd {
extern qd::thread::Event onUaeInitialized;

class App {
public:
    qd::Debugger* mDebugger = nullptr;
    bool mQuitRequestPosted = false;
    SDL_Window* s_window = nullptr;
    SDL_Texture* s_texture = nullptr;
    SDL_Renderer* s_renderer = nullptr;

public:
    void requestToQuit();
    bool hasQuitRequest() const;
    void init();
    void mainLoop();
    void destroy();

    qd::Debugger* getDbg() const {
        return mDebugger;
    }

    static App* get() {
        static App inst;
        return &inst;
    }

};  // class QuaesarApp
//////////////////////////////////////////////////////////////////////////


namespace uae {
extern void do_console_cmd_immediate(const char* cmd);
};

};  // namespace qd

extern qd::App* app;

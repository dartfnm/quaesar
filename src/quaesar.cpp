#define SDL_MAIN_NEEDED
#include "quaesar.h"
#include <SDL.h>
#include <SDL_main.h>
#include <debugger/debugger.h>
#include <src/generic/thread.h>
#include <stdarg.h>
#include <stdio.h>

// clang-format off
#include "sysconfig.h"
#include "sysdeps.h"
#include "uae/time.h"
#include "external/cli11/CLI11.hpp"
#include "parse_options.h"
#include "options.h"
#include "adf.h"
#include "uae.h"
// clang-format on

// WTF SDL!
// #undef main

qd::App* app = nullptr;

extern void real_main(int argc, TCHAR** argv);
extern void keyboard_settrans();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this somewhere else

bool ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return false;
    }

    // Compare the end of the string with the suffix
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}


int uae_thread_main_func(void* args = nullptr) {
    real_main(0, nullptr);
    return 0;
}


// Quaesar main
int SDL_main(int argc, char* argv[]) {
    syncbase = 1000000;
    app = qd::App::get();

    Options options;
    CLI::App cliApp{"Quaesar"};

    cliApp.add_option("input", options.input, "Executable or image file (adf, dms)")->check(CLI::ExistingFile);
    cliApp.add_option("-k,--kickstart", options.kickstart, "Path to the kickstart ROM")->check(CLI::ExistingFile);
    cliApp.add_option("--serial_port", options.serial_port, "Serial port path");
    CLI11_PARSE(cliApp, argc, argv);

    keyboard_settrans();
    default_prefs(&currprefs, true, 0);
    fixup_prefs(&currprefs, true);

    if (!options.input.empty()) {
        // TODO: cleanup
        if (ends_with(options.input.c_str(), ".exe") || !ends_with(options.input.c_str(), ".adf")) {
            Adf::create_for_exefile(options.input.c_str());
            strcpy(currprefs.floppyslots[0].df, "dummy.adf");
        } else {
            strcpy(currprefs.floppyslots[0].df, options.input.c_str());
        }
    }

    if (!options.serial_port.empty()) {
        currprefs.use_serial = 1;
        strcpy(currprefs.sername, options.serial_port.c_str());
    }

    // Most compatible mode
    currprefs.cpu_cycle_exact = 1;
    currprefs.cpu_memory_cycle_exact = 1;
    currprefs.blitter_cycle_exact = 1;
    currprefs.floppy_speed = 100;
    //    currprefs.turbo_emulation = 1; // it disables sound
    currprefs.sound_stereo_separation = 0;
    currprefs.uaeboard = 1;

    strcpy(currprefs.romfile, options.kickstart.c_str());

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    atexit(&SDL_Quit);

#ifdef WIN32  // shows Windows ASSERT dialog with the 'Ignore' button, which allows you to continue executing the
              // program.
    _set_error_mode(_OUT_TO_MSGBOX);
#endif

    // start UAE in separate thread
    SDL_Thread* uae_thread_handler;
    uae_thread_handler = SDL_CreateThreadWithStackSize(&uae_thread_main_func, "UAE emulator", 2 << 10, nullptr);

    // wait UAE
    qd::onUaeInitialized.wait();

    ::app->init();
    ::app->mainLoop();

    SDL_WaitThread(uae_thread_handler, nullptr);

    ::app->destroy();

    return 0;
}


//////////////////////////////////////////////////////////////////////////
namespace qd {

qd::thread::Event onUaeInitialized(true);

void App::requestToQuit() {
    mQuitRequestPosted = true;
}


bool App::hasQuitRequest() const {
    return mQuitRequestPosted;
}


void App::init() {
    mDebugger = qd::Debugger_create();
    qd::Debugger_toggle(mDebugger, qd::DebuggerMode_Live);
}


void App::mainLoop() {
    SDL_Event event;
    while (true) {
        if (mQuitRequestPosted) {
            ::quit_program = UAE_QUIT;
            ::currprefs.cpu_cycle_exact = 0;
            break;
        }
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    requestToQuit();
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        // requestToQuit();
                        break;
                    } else if (event.key.keysym.sym == SDLK_F12) {
                        // activate_debugger();
                        // qd::Debugger_toggle(mDebugger, qd::DebuggerMode_Live);
                    }
                    break;
                case SDL_WINDOWEVENT: {
                    Uint8 wndEvent = event.window.event;
                    if (wndEvent == SDL_WINDOWEVENT_CLOSE) {
                        requestToQuit();
                        break;
                    }
                    break;
                }
                default:
                    break;
            }
            qd::Debugger_update_event(&event);
        }
        if (qd::Debugger_is_window_visible(mDebugger))
            mDebugger->update();
    }

    SDL_Log("Waiting UAE thread over ...");
    mDebugger->execConsoleCmd("q");
}


void App::destroy() {
    if (mDebugger) {
        mDebugger->destroy();
        SDL_DestroyRenderer(mDebugger->renderer);
        SDL_DestroyWindow(mDebugger->window);
    }
    mDebugger = nullptr;
}


};  // namespace qd

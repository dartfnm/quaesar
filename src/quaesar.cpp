#include "quaesar.h"
#include <SDL.h>
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

#ifdef WIN32
#define SDL_MAIN_NEEDED
#include <SDL_main.h>
#else
// WTF SDL2MAIN_LIBRARY - I can't to build it on MacOs
#define SDL_main main
#endif  // WIN32

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


int uae_thread_main_func(void* /*args*/ = nullptr) {
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

    // start UAE in separate thread
    SDL_Thread* uae_thread_handler;
    uae_thread_handler = SDL_CreateThreadWithStackSize(&uae_thread_main_func, "UAE emulator", 2 << 10, nullptr);

    // wait UAE initialization
    qd::onUaeInitialized.wait();

    // quaesar main loop
    ::app->init();
    ::app->mainLoop();

    // wait UAE done
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
    createUaeWindow();
    mDebugger = Debugger::get();
    mDebugger->init();
    mDebugger->toggleWndVisible(qd::DebuggerMode_Live);
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
            mDebugger->sdlEventProc(&event);
        }

        if (mDebugger->isVisible()) {
            mDebugger->update();
            mDebugger->render();
        }
        renderUaeWindow();
    }

    // quit
    SDL_Log("Waiting UAE thread over ...");
    mDebugger->execConsoleCmd("q");
}


void App::createUaeWindow() {
    const int amiga_width = 754;
    const int amiga_height = 576;
    const int depth = 32;
    int wndWidth = amiga_width;
    int wndHeight = amiga_height;

    SDL_AtomicSet(&scrFrameNo, 0);

    // Create a window
    app->mUaeWindow = SDL_CreateWindow("Quaesar", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wndWidth, wndHeight,
                                       SDL_WINDOW_RESIZABLE);

    if (!app->mUaeWindow) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    app->mUaeRenderer = SDL_CreateRenderer(app->mUaeWindow, -1, SDL_RENDERER_ACCELERATED);

    if (!app->mUaeRenderer) {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(app->mUaeWindow);
        SDL_Quit();
        return;
    }

    mUaeScrTexture = SDL_CreateTexture(mUaeRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, amiga_width,
                                       amiga_height);

    if (!mUaeScrTexture) {
        SDL_Log("Could not create texture: %s", SDL_GetError());
        SDL_DestroyRenderer(mUaeRenderer);
        SDL_DestroyWindow(mUaeWindow);
        SDL_Quit();
        return;
    }
}


void App::renderUaeWindow() {
    // render UAE texture screen
    int curFrame = SDL_AtomicGet(&scrFrameNo);
    if (curFrame == renderedFrameNo) {
        return;
    }
    renderedFrameNo = curFrame;

    int amiga_width = 754;
    int amiga_height = 576;
    int new_width = 0;
    int new_height = 0;
    int window_width, window_height;
    SDL_GetWindowSize(mUaeWindow, &window_width, &window_height);

    // Maintain aspect ratio
    float image_aspect = (float)amiga_width / (float)amiga_height;
    float window_aspect = (float)window_width / (float)window_height;

    if (window_aspect < image_aspect) {
        new_width = window_width;
        new_height = (int)(window_width / image_aspect);
    } else {
        new_height = window_height;
        new_width = (int)(window_height * image_aspect);
    }
    SDL_Rect rect = {(window_width - new_width) / 2, (window_height - new_height) / 2, new_width, new_height};
    SDL_RenderClear(mUaeRenderer);
    if (mUaeScrTextureMutex.tryLock()) {
        SDL_RenderCopy(mUaeRenderer, mUaeScrTexture, NULL, &rect);
        SDL_RenderPresent(mUaeRenderer);
        mUaeScrTextureMutex.unlock();
    }
}


uint32_t* App::lockUaeScreenTexBuf() {
    mUaeScrTextureMutex.lock();
    uint32_t* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(mUaeScrTexture, NULL, (void**)&pixels, &pitch) == 0) {
        return pixels;
    }
    mUaeScrTextureMutex.unlock();
    return nullptr;
}


void App::unlockUaeScreenTexBuf() {
    SDL_UnlockTexture(mUaeScrTexture);
    mUaeScrTextureMutex.unlock();
    SDL_AtomicIncRef(&scrFrameNo);
}


void App::destroy() {
    if (mDebugger)
        mDebugger->destroy();
    mDebugger = nullptr;
}


};  // namespace qd

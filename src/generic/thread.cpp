#include "thread.h"


namespace qd::thread {

Event::Event(bool auto_reset_event) : mbState(false), mbAutoReset(auto_reset_event) {
    mpCondition = SDL_CreateCond();
    if (!mpCondition) {
        SDL_Log("Thread::Event::create() : FAILED");
        return;
    }
    mpMutex = SDL_CreateMutex();
    if (!mpMutex) {
        SDL_Log("Thread::Event::create() (mutex)");
        return;
    }
}


void Event::set() {
    if (SDL_LockMutex(mpMutex)) {
        SDL_Log("cannot signal event (lock)");
        return;
    }
    mbState = true;
    if (SDL_CondBroadcast(mpCondition) != 0) {
        SDL_UnlockMutex(mpMutex);
        SDL_Log("cannot signal event");
        return;
    }
    SDL_UnlockMutex(mpMutex);
    SDL_CondSignal(mpCondition);
}


void Event::wait() {
    if (SDL_LockMutex(mpMutex) != 0) {
        SDL_Log("wait for event failed (lock)");
        return;
    }
    while (!mbState) {
        if (SDL_CondWait(mpCondition, mpMutex) != 0) {
            SDL_UnlockMutex(mpMutex);
            SDL_Log("wait for event failed");
            return;
        }
    }
    if (mbAutoReset)
        mbState = false;
    SDL_UnlockMutex(mpMutex);
}


bool Event::wait(uint32_t time_out_ms) {
    if (SDL_LockMutex(mpMutex) != 0) {
        SDL_Log("wait for event failed (lock)");
        return false;
    }

    int rc = 0;
    while (!mbState) {
        rc = SDL_CondWaitTimeout(mpCondition, mpMutex, time_out_ms);
        if (rc != 0) {
            if (rc == SDL_MUTEX_TIMEDOUT)
                break;
            SDL_UnlockMutex(mpMutex);
            SDL_Log("cannot wait for event");
            return false;
        }
    }
    if (rc == 0 && mbAutoReset)
        mbState = false;
    SDL_UnlockMutex(mpMutex);
    return rc == 0;
}


void Event::reset() {
    if (SDL_LockMutex(mpMutex) != 0) {
        SDL_Log("Cannot reset event");
        return;
    }
    mbState = false;
    SDL_UnlockMutex(mpMutex);
}


};  // namespace qd::thread

#include "log.h"
#include <src/generic/base.h>


namespace qd {


Log::~Log() {
    done();
}


void Log::registerWriter(LogWriter_ptr p_writer) {
    mpLogWriters.push_back(eastl::move(p_writer));
}


void Log::destroyWriter(ILogWriter* p_ptr) {
    if (!p_ptr)
        return;
    for (auto it = mpLogWriters.begin(); it != mpLogWriters.end(); ++it) {
        ILogWriter* pCurWriter = *it;
        if (pCurWriter == p_ptr) {
            mpLogWriters.erase(it);
            SAFE_DESTROY_AND_DELETE(pCurWriter);
            return;
        }
    }
}


void Log::done() {
    while (!mpLogWriters.empty()) {
        ILogWriter* pCurWriter = mpLogWriters.back();
        mpLogWriters.pop_back();
        SAFE_DESTROY_AND_DELETE(pCurWriter);
    }
}


void Log::logV(LogEntry::ELevel level, const char* message, va_list arguments) {
    LogEntry rec;
    rec.level = level;
    rec.message.append_sprintf_va_list(message, arguments);
    rec.timeStamp = std::time(nullptr);

    for (LogWriter_ptr& curWriter : mpLogWriters) {
        curWriter->addLogEntry(rec);
    }
}


qd::Log& logConsole() {
    static Log instance;
    return instance;
}


};  // namespace qd

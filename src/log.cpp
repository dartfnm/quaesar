#include "log.h"


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
        ILogWriter* curWriter = *it;
        if (curWriter == p_ptr) {
            curWriter->onDestroy();
            mpLogWriters.erase(it);
            return;
        }
    }
}


void Log::done() {
    while (!mpLogWriters.empty()) {
        ILogWriter* pCurWriter = mpLogWriters.back();
        mpLogWriters.pop_back();
        pCurWriter->onDestroy();
        delete pCurWriter;
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

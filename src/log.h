#pragma once
#include <EASTL/fixed_vector.h>
#include <EASTL/string.h>
#include <cstdarg>
#include <ctime>


namespace qd {


struct LogEntry {
    enum ELevel {
        E_DEBUG = 0x01,
        E_INFO = 0x02,
        E_WARNING = 0x04,
        E_ERROR = 0x08,
    };

    std::time_t timeStamp;
    ELevel level;
    eastl::string message;
};  // struct LogEntry
//////////////////////////////////////////////////////////////////////////


class ILogWriter {
public:
    virtual void addLogEntry(const LogEntry& entry) = 0;
    virtual void destroy() {
    }
    virtual ~ILogWriter() = default;
};  // class ILogWriter


//////////////////////////////////////////////////////////////////////////
class Log {
    using LogWriter_ptr = ILogWriter*;
    eastl::fixed_vector<LogWriter_ptr, 4, false> mpLogWriters;

public:
    Log() = default;
    ~Log();

    void registerWriter(LogWriter_ptr p_writer);
    void destroyWriter(ILogWriter* p_ptr);
    void done();

    template <class TWriter>
    TWriter* createWriter_() {
        TWriter* pInst = new TWriter();
        registerWriter(eastl::move(pInst));
        return pInst;
    }

public:
    void logV(LogEntry::ELevel level, const char* message, va_list arguments);

    void debug(const char* message, ...) {
        va_list args;
        va_start(args, message);
        logV(LogEntry::E_DEBUG, message, args);
        va_end(args);
    }

    void info(const char* message, ...) {
        va_list args;
        va_start(args, message);
        logV(LogEntry::E_INFO, message, args);
        va_end(args);
    }

    void warn(const char* message, ...) {
        va_list args;
        va_start(args, message);
        logV(LogEntry::E_WARNING, message, args);
        va_end(args);
    }

    void error(const char* message, ...) {
        va_list args;
        va_start(args, message);
        logV(LogEntry::E_ERROR, message, args);
        va_end(args);
    }

};  // class Log
//////////////////////////////////////////////////////////////////////////


extern qd::Log& logConsole();

};  // namespace qd

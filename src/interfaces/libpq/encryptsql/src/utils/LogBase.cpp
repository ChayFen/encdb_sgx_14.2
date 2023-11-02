#include "LogBase.h"
#include "config.h"
#include <iostream>

namespace util
{
    constexpr size_t LOG_BUFF_SIZE = 1024 * 10;
    __thread char buffer[LOG_BUFF_SIZE];

    LogBase *LogBase::Inst()
    {
        static LogBase instance;
        return &instance;
    }

    LogBase::LogBase()
    {
        m_enabled[log::verbose] = false;
        m_enabled[log::info] = true;
        m_enabled[log::warning] = true;
        m_enabled[log::error] = true;
        m_enabled[log::timer] = false;
        int pid = getpid();
        char logPath[64] = {0};
        sprintf(logPath,"/etc/encryptsql/encryptsql.%d.log",pid );
        this->appender = new log4cpp::FileAppender("fileAppender", logPath, false);

        auto pLayout = new log4cpp::PatternLayout();
        pLayout->setConversionPattern("%d: %p %c %x: %m%n");
        this->appender->setLayout(pLayout);

        root.setPriority(log4cpp::Priority::INFO);
        root.addAppender(this->appender);
    }

    LogBase::~LogBase() {}

    void LogBase::Log(const log::Fmt &msg, log::Severity s)
    {
        if (IsEnabled(s) && !IsEnabled(log::timer))
        {
            switch (s)
            {
            case log::info:
                root.info(msg.str());
                break;
            case log::error:
                root.error(msg.str());
                break;
            case log::warning:
                root.warn(msg.str());
                break;
            }
        }
    }

    bool LogBase::Enable(log::Severity s, bool enable)
    {
        bool prev = m_enabled[s];
        m_enabled[s] = enable;

        return prev;
    }

    void LogBase::DisableAll(bool b)
    {
        m_enabled[log::verbose] = b;
        m_enabled[log::info] = b;
        m_enabled[log::warning] = b;
        m_enabled[log::error] = b;
        m_enabled[log::timer] = b;
    }

    bool LogBase::IsEnabled(log::Severity s) const
    {
        return m_enabled[s];
    }

    void Log(const string &str, log::Severity s)
    {
        LogBase::Inst()->Log(log::Fmt(str), s);
    }

    void DisableAllLogs(bool b)
    {
        LogBase::Inst()->DisableAll(b);
    }

}

using namespace util;

void LogInfo(const char *fmt, ...)
{
    // char buffer[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, LOG_BUFF_SIZE, fmt, ap);
    va_end(ap);
    LogBase::Inst()->Log(log::Fmt(buffer), log::info);
}

void LogWarning(const char *fmt, ...)
{
    // char buffer[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, LOG_BUFF_SIZE, fmt, ap);
    va_end(ap);
    LogBase::Inst()->Log(log::Fmt(buffer), log::warning);
}

void LogError(const char *fmt, ...)
{
    // char buffer[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, LOG_BUFF_SIZE, fmt, ap);
    va_end(ap);
    LogBase::Inst()->Log(log::Fmt(buffer), log::error);
}

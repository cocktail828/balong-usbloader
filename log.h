#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>

#include <time.h>

namespace LOGGER
{
enum class Severity
{
    ERROR,
    WARNNING,
    DEBUG,
    VERBOSE,
    INFO,
};

enum class FileMode
{
    OVERWRITE,
    APPEND,
};

static const std::string SeverityStr[] = {"error", "warn", "debug", "verbose", "info"};
// out stream object
struct Logger
{
    static std::ofstream fout;
    static Severity level_basic;
    static FileMode mode;
    std::ostringstream stream;
    Severity level;
    std::string filename;
    std::string function;
    int line;

public:
    ~Logger() { fout.close(); }
    Logger() : level(Severity::INFO) {}
    Logger(Severity l) : level(l) {}

    static const std::string &get_time()
    {
        static std::string timenow;
        time_t rawtime;
        struct tm *info;
        static char buffer[80];

        time(&rawtime);
        info = localtime(&rawtime);

        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
        timenow = buffer;
        return timenow;
    }

    static void Init(const std::string &lf)
    {
        if (mode == FileMode::APPEND)
            fout.open(lf, std::ios::app);

        if (mode == FileMode::OVERWRITE)
            fout.open(lf, std::ios::trunc);
        fout << "\r\n\r\nnew log start from here, at " << get_time() << std::endl;
    }

    static void Init(const std::string &lf, FileMode m)
    {
        mode = m;
        Init(lf);
    }

    static void Init(Severity l, const std::string &lf, FileMode m)
    {
        level_basic = l;
        Init(lf, m);
    }

    Logger &operator()(Severity l, const std::string &fl, const std::string &func, int ln)
    {
        level = l;
        filename = fl;
        function = func;
        line = ln;
        return *this;
    }

    Logger &operator()(Severity l)
    {
        level = l;
        return *this;
    }

    Logger &operator<<(const char *value)
    {
        stream << value;
        return *this;
    }

    template <typename T>
    Logger &operator<<(const T &value)
    {
        stream << value;
        return *this;
    }

    Logger &operator<<(std::ostream &(*os)(std::ostream &))
    {
        if (static_cast<int>(level_basic) >= static_cast<int>(level))
        {
            if (filename.empty())
                std::cerr << "[" << get_time() << "] " << SeverityStr[static_cast<int>(level)] << ": " << stream.str() << os;
            else
                std::cerr << "[" << get_time() << " " << filename << ":" << line << " " << function << "] "
                          << SeverityStr[static_cast<int>(level)] << ": " << stream.str() << os;
        }

        if (filename.empty())
            fout << "[" << get_time() << "] " << SeverityStr[static_cast<int>(level)] << ": " << stream.str() << os;
        else
            fout << "[" << get_time() << " " << filename << ":" << line << " " << function << "] "
                 << SeverityStr[static_cast<int>(level)] << ": " << stream.str() << os;

        level = Severity::INFO;
        stream.str("");
        filename.clear();
        function.clear();
        line = 0;
        return *this;
    }
};

extern Logger LOG;
#ifdef DEBUG_MODE
#define LOGI LOG(Severity::INFO, __FILE__, __func__, __LINE__)
#define LOGV LOG(Severity::VERBOSE, __FILE__, __func__, __LINE__)
#define LOGD LOG(Severity::DEBUG, __FILE__, __func__, __LINE__)
#define LOGW LOG(Severity::WARNNING, __FILE__, __func__, __LINE__)
#define LOGE LOG(Severity::ERROR, __FILE__, __func__, __LINE__)
#else
#define LOGI LOG(Severity::INFO)
#define LOGV LOG(Severity::VERBOSE)
#define LOGD LOG(Severity::DEBUG)
#define LOGW LOG(Severity::WARNNING)
#define LOGE LOG(Severity::ERROR)
#endif
} // namespace LOGGER

#endif //__LOGGER_H__

/*
* Author   : Harry Yu
* Email    : harryyunull@gmail.com
* Link     : https://github.com/harrynull
*/

#ifndef CCALIGNER_LOGGER_H
#define CCALIGNER_LOGGER_H
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <array>

// Define possible exit codes that will be passed on to the fatal function exit codes.
#define DO_NOT_EXIT                             -1
#define EXIT_OK                                 0
#define EXIT_FILE_NOT_FOUND                     2
#define EXIT_INVALID_PARAMETERS                 3
#define EXIT_INCOMPATIBLE_PARAMETERS            4
#define EXIT_INCOMPLETE_PARAMETERS              5
#define EXIT_INVALID_FILE                       6
#define EXIT_WITH_HELP                          10
#define EXIT_UNKNOWN                            13

namespace Colors {
    using ColorFunc = std::ostream&(std::ostream&);
#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#define WINDOWS_COLOR(colorName, colorCode)\
    inline std::ostream& colorName(std::ostream& s)\
    {SetConsoleTextAttribute(hStdout, colorCode);return s;}

    static HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    WINDOWS_COLOR(red,      FOREGROUND_RED);
    WINDOWS_COLOR(yellow,   FOREGROUND_RED | FOREGROUND_GREEN);
    WINDOWS_COLOR(lred,     FOREGROUND_RED | FOREGROUND_INTENSITY);
    WINDOWS_COLOR(white,    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    WINDOWS_COLOR(lyellow,  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    WINDOWS_COLOR(lwhite,   FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#undef WINDOWS_COLOR

#else
#define LINUX_COLOR(colorName, colorCode) inline std::ostream& colorName(std::ostream &s) {return s << colorCode;}

    LINUX_COLOR(red,        "\033[21;31m");
    LINUX_COLOR(yellow,     "\033[21;33m");
    LINUX_COLOR(lred,       "\033[1;31m");
    LINUX_COLOR(white,      "\033[21;37m");
    LINUX_COLOR(lyellow,    "\033[1;33m");
    LINUX_COLOR(lwhite,     "\033[1;37m");

#undef LINUX_COLOR
#endif
}

class Logger {
public:

    enum Level {
        verbose, debug, info, warning, error, fatal, nolog
    };

    static std::array<const char*, 6> levelTags;
    static std::array<Colors::ColorFunc*, 6> colors;

    // A sink -- where the log will go to. It can be stdout, a file, or any custom ostream (like network).
    class Sink {
    public:
        Sink(std::ostream& os, bool useColor) :_os(os), _useColor(useColor) {}
        
        // Write stringstream to ostream if the level is higher than the minimum output level.
        void output(std::stringstream& ss, Level level) const {
            if (level < _minimumOutputLevel) return;
            if (_useColor) _os << *colors[level];
            _os << ss.rdbuf();
            if (level == error || level == fatal) ss.flush(); // flush immediately to prevent data loss.
        }

        // Set thte minimum output level.
        void setMinimumOutputLevel(Level level) noexcept {
            _minimumOutputLevel = level;
        }

    private:
        std::ostream& _os;
        Level _minimumOutputLevel = Level::verbose;
        bool _useColor;
    };

    class Log {
    public:
        Log(const Logger& logger, const char* fileName, const char* funcName, int lineNumber, Level level, const char* levelName, int exitCode = DO_NOT_EXIT)
            :_logger(logger), _level(level), _exitCode(exitCode) {
            _ss << "[" << getCurrentTime() << "]" << levelName << fileName << " : " << funcName << " | ";
        }
        
        ~Log() {
            _ss << std::endl;
            _logger.log(_ss, _level);
            if (_exitCode != DO_NOT_EXIT) std::exit(_exitCode);
        }

        template <typename T>
        Log& operator<<(const T& rhs) {
            _ss << rhs;
            return *this;
        }

    private:
        static std::string getCurrentTime() {
            std::string currentTime(15, '\0');
            const auto now = std::time(nullptr);
            std::strftime(&currentTime.front(), currentTime.size(), "%m-%d %H:%M:%S", std::localtime(&now));
            return currentTime;
        }

        const Logger& _logger;
        std::stringstream _ss;
        Level _level;
        int _exitCode;
    };

    // It will apply the level to all **existing** sinks. Set level to nolog if you don't want any log.
    void setMinimumOutputLevel(Level level) noexcept {
        for (auto& sink : _sinks) sink.setMinimumOutputLevel(level);
    }

    void log(std::stringstream& ss, Level level) const {
        for (auto& sink : _sinks) sink.output(ss, level);
    }

    friend Logger& getLogger();

private:
    Logger() : _sinks{ {std::cout, true} } {}
    std::vector<Sink> _sinks;
};

inline Logger& getLogger() {
    static Logger logger;
    return logger;
}

#define loggerstream(level) (Logger::Log(getLogger(), __FILE__, __FUNCTION__, __LINE__, Logger::Level::level, Logger::levelTags[Logger::Level::level]))
// Information for tracing
#define verbosestream loggerstream(verbose)
// Information for developers
#define debugstream loggerstream(debug)
// Information for general users
#define infostream loggerstream(info)
// Problems that may affect facility, performance or stability but may not lead the program to crash immediately
#define warningstream loggerstream(warning)
// Something unexpected happened but can be recovered
#define errorstream loggerstream(error)
// Unrecoverable error and program termination is required
#define fatalstream(code) (Logger::Log(getLogger(), __FILE__, __FUNCTION__, __LINE__, Logger::Level::fatal, Logger::levelTags[Logger::Level::fatal], code))

#endif

#include "log_analyzer/LogLevel.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace log_analyzer {

    namespace {

        std::string ToLower(std::string_view input) {
            std::string result;
            result.reserve(input.size());
            for (char c : input) {
                result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            return result;
        }

    }  // namespace

    [[nodiscard]] LogLevel ParseLogLevel(std::string_view raw_level) noexcept {
        static const std::unordered_map<std::string_view, LogLevel> kLevelMap = {
            {"trace", LogLevel::TRACE},
            {"debug", LogLevel::DEBUG},
            {"dbg", LogLevel::DEBUG},
            {"info", LogLevel::INFO},
            {"information", LogLevel::INFO},
            {"notice", LogLevel::INFO},
            {"warn", LogLevel::WARN},
            {"warning", LogLevel::WARN},
            {"error", LogLevel::ERROR},
            {"err", LogLevel::ERROR},
            {"critical", LogLevel::CRITICAL},
            {"crit", LogLevel::CRITICAL},
            {"fatal", LogLevel::FATAL},
        };

        std::string lower = ToLower(raw_level);
        auto it = kLevelMap.find(lower);
        if (it != kLevelMap.end()) {
            return it->second;
        }
        return LogLevel::UNKNOWN;
    }

    [[nodiscard]] std::string LogLevelToString(LogLevel level) noexcept {
        switch (level) {
            case LogLevel::TRACE:
                return "TRACE";
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARN:
                return "WARN";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::CRITICAL:
                return "CRITICAL";
            case LogLevel::FATAL:
                return "FATAL";
            case LogLevel::UNKNOWN:
            default:
                return "UNKNOWN";
        }
    }

    [[nodiscard]] bool IsKnownLogLevel(LogLevel level) noexcept {
        return level != LogLevel::UNKNOWN;
    }

}  // namespace log_analyzer

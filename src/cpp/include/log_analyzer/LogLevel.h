#ifndef LOG_ANALYZER_LOG_LEVEL_H
#define LOG_ANALYZER_LOG_LEVEL_H

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace log_analyzer {

    enum class LogLevel : uint8_t {
        UNKNOWN = 0,
        TRACE = 1,
        DEBUG = 2,
        INFO = 3,
        WARN = 4,
        ERROR = 5,
        CRITICAL = 6,
        FATAL = 7,
    };

    // Operator overloads for comparison-based filtering
    constexpr bool operator<(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) < static_cast<uint8_t>(rhs);
    }

    constexpr bool operator<=(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) <= static_cast<uint8_t>(rhs);
    }

    constexpr bool operator>(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) > static_cast<uint8_t>(rhs);
    }

    constexpr bool operator>=(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) >= static_cast<uint8_t>(rhs);
    }

    constexpr bool operator==(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) == static_cast<uint8_t>(rhs);
    }

    constexpr bool operator!=(LogLevel lhs, LogLevel rhs) noexcept {
        return static_cast<uint8_t>(lhs) != static_cast<uint8_t>(rhs);
    }

    [[nodiscard]] LogLevel ParseLogLevel(std::string_view raw_level) noexcept;

    [[nodiscard]] std::string LogLevelToString(LogLevel level) noexcept;

    [[nodiscard]] bool IsKnownLogLevel(LogLevel level) noexcept;

}  // namespace log_analyzer

#endif  // LOG_ANALYZER_LOG_LEVEL_H

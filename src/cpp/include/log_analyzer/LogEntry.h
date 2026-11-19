#ifndef LOG_ANALYZER_LOG_ENTRY_H
#define LOG_ANALYZER_LOG_ENTRY_H

#include <cstdint>
#include <string>
#include <string_view>

#include "log_analyzer/LogLevel.h"

namespace log_analyzer {

class LogEntry {
public:
    // Factory method for creating from raw parsed values
    [[nodiscard]] static LogEntry CreateFromRaw(
        uint64_t timestamp_ns,
        std::string_view raw_level,
        std::string_view source_file,
        std::string_view service_name,
        std::string_view hostname,
        std::string_view message) noexcept;

    // Getters (const only — immutability guarantee)
    [[nodiscard]] uint64_t GetTimestampNs() const noexcept;
    [[nodiscard]] LogLevel GetLogLevel() const noexcept;
    [[nodiscard]] const std::string& GetSourceFile() const noexcept;
    [[nodiscard]] const std::string& GetServiceName() const noexcept;
    [[nodiscard]] const std::string& GetHostname() const noexcept;
    [[nodiscard]] const std::string& GetMessage() const noexcept;
    [[nodiscard]] const std::string& GetRawLogLevel() const noexcept;

    // Conversion helpers
    [[nodiscard]] std::string ToIso8601() const;
    [[nodiscard]] std::string ToLogLevelString() const;

    // Predicate
    [[nodiscard]] bool IsValid() const noexcept;

private:
    // Private constructor — use CreateFromRaw factory
    LogEntry(uint64_t timestamp_ns,
             LogLevel log_level,
             std::string source_file,
             std::string service_name,
             std::string hostname,
             std::string message,
             std::string raw_log_level) noexcept;

    uint64_t m_timestamp_ns;
    LogLevel m_log_level;
    std::string m_source_file;
    std::string m_service_name;
    std::string m_hostname;
    std::string m_message;
    std::string m_raw_log_level;
};

}  // namespace log_analyzer

#endif  // LOG_ANALYZER_LOG_ENTRY_H

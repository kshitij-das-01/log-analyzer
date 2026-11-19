#include "log_analyzer/LogEntry.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace log_analyzer {

LogEntry::LogEntry(uint64_t timestamp_ns,
                   LogLevel log_level,
                   std::string source_file,
                   std::string service_name,
                   std::string hostname,
                   std::string message,
                   std::string raw_log_level) noexcept
    : m_timestamp_ns(timestamp_ns),
      m_log_level(log_level),
      m_source_file(std::move(source_file)),
      m_service_name(std::move(service_name)),
      m_hostname(std::move(hostname)),
      m_message(std::move(message)),
      m_raw_log_level(std::move(raw_log_level)) {}

[[nodiscard]] LogEntry LogEntry::CreateFromRaw(
    uint64_t timestamp_ns,
    std::string_view raw_level,
    std::string_view source_file,
    std::string_view service_name,
    std::string_view hostname,
    std::string_view message) noexcept {
    LogLevel level = ParseLogLevel(raw_level);
    std::string raw_level_str(raw_level);

    return LogEntry(
        timestamp_ns,
        level,
        std::string(source_file),
        std::string(service_name),
        std::string(hostname),
        std::string(message),
        std::move(raw_level_str));
}

[[nodiscard]] uint64_t LogEntry::GetTimestampNs() const noexcept {
    return m_timestamp_ns;
}

[[nodiscard]] LogLevel LogEntry::GetLogLevel() const noexcept {
    return m_log_level;
}

[[nodiscard]] const std::string& LogEntry::GetSourceFile() const noexcept {
    return m_source_file;
}

[[nodiscard]] const std::string& LogEntry::GetServiceName() const noexcept {
    return m_service_name;
}

[[nodiscard]] const std::string& LogEntry::GetHostname() const noexcept {
    return m_hostname;
}

[[nodiscard]] const std::string& LogEntry::GetMessage() const noexcept {
    return m_message;
}

[[nodiscard]] const std::string& LogEntry::GetRawLogLevel() const noexcept {
    return m_raw_log_level;
}

[[nodiscard]] std::string LogEntry::ToIso8601() const {
    // Convert nanoseconds to seconds and sub-second part
    const time_t seconds = static_cast<time_t>(m_timestamp_ns / 1'000'000'000ULL);
    const uint64_t nanos = m_timestamp_ns % 1'000'000'000ULL;

    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&seconds), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setw(9) << std::setfill('0') << nanos << 'Z';
    return oss.str();
}

[[nodiscard]] std::string LogEntry::ToLogLevelString() const {
    return LogLevelToString(m_log_level);
}

[[nodiscard]] bool LogEntry::IsValid() const noexcept {
    return !m_message.empty() && !m_source_file.empty();
}

}  // namespace log_analyzer

#ifndef LOG_ANALYZER_IN_MEMORY_INDEX_H
#define LOG_ANALYZER_IN_MEMORY_INDEX_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "log_analyzer/LogEntry.h"
#include "log_analyzer/LogLevel.h"
#include "log_analyzer/QueryParams.h"

namespace log_analyzer {

class InMemoryIndex {
public:
    InMemoryIndex() = default;

    // Writer — called by ingestion thread
    void Insert(const LogEntry& entry);

    // Readers — called by query threads (thread-safe)
    [[nodiscard]] std::vector<const LogEntry*> Query(const QueryParams& params) const;

    // Statistics
    [[nodiscard]] size_t GetSize() const noexcept;
    [[nodiscard]] uint64_t GetOldestTimestampNs() const noexcept;
    [[nodiscard]] uint64_t GetNewestTimestampNs() const noexcept;

private:
    // Primary storage — time-ordered contiguous vector
    std::vector<LogEntry> m_entries;

    // Secondary indexes — store indices into m_entries
    // Index 0 = UNKNOWN, then TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL, FATAL
    std::array<std::vector<size_t>, 8> m_level_buckets;

    // Service name → indices
    std::unordered_map<std::string, std::vector<size_t>> m_service_index;

    mutable std::shared_mutex m_mutex;

    // Internal helpers
    [[nodiscard]] std::pair<size_t, size_t> BinarySearchTimeRangeInternal(
        uint64_t start_ns, uint64_t end_ns) const;

    void MergeFilterResultsInternal(
        std::vector<const LogEntry*>& results,
        const QueryParams& params) const;

    void RebuildSecondaryIndexes();
};

}  // namespace log_analyzer

#endif  // LOG_ANALYZER_IN_MEMORY_INDEX_H

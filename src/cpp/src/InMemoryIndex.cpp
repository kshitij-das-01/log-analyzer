#include "log_analyzer/InMemoryIndex.h"

#include <algorithm>
#include <cstring>

namespace log_analyzer {

void InMemoryIndex::Insert(const LogEntry& entry) {
    std::unique_lock<std::shared_mutex> write_lock(m_mutex);

    const size_t index = m_entries.size();

    // Insert in time-sorted position using binary search
    auto insert_pos = std::lower_bound(
        m_entries.begin(), m_entries.end(), entry.GetTimestampNs(),
        [](const LogEntry& e, uint64_t val) {
            return e.GetTimestampNs() < val;
        });

    m_entries.insert(insert_pos, entry);

    // All indices after insert_pos shifted by 1 — need to rebuild secondary indexes
    // For correctness, rebuild all secondary indexes after any insert
    RebuildSecondaryIndexes();
}

[[nodiscard]] std::vector<const LogEntry*> InMemoryIndex::Query(
    const QueryParams& params) const {
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);

    if (m_entries.empty()) {
        return {};
    }

    // 1. Binary search for time range → [start_idx, end_idx)
    auto [start_idx, end_idx] = BinarySearchTimeRangeInternal(
        params.start_time_ns.value_or(0),
        params.end_time_ns.value_or(UINT64_MAX));

    if (start_idx >= end_idx) {
        return {};
    }

    // 2. Collect candidates from secondary indexes within time range
    std::vector<const LogEntry*> results;
    results.reserve(end_idx - start_idx);

    if (params.log_level.has_value() && params.service_name.has_value()) {
        // Both level and service filters — intersect both secondary indexes
        MergeFilterResultsInternal(results, params);
    } else if (params.log_level.has_value()) {
        // Level filter only
        LogLevel level = *params.log_level;
        if (static_cast<size_t>(level) < m_level_buckets.size()) {
            for (size_t idx : m_level_buckets[static_cast<size_t>(level)]) {
                if (idx >= start_idx && idx < end_idx) {
                    results.push_back(&m_entries[idx]);
                    if (results.size() >= params.limit) {
                        break;
                    }
                }
            }
        }
    } else if (params.service_name.has_value()) {
        // Service filter only
        auto it = m_service_index.find(*params.service_name);
        if (it != m_service_index.end()) {
            for (size_t idx : it->second) {
                if (idx >= start_idx && idx < end_idx) {
                    results.push_back(&m_entries[idx]);
                    if (results.size() >= params.limit) {
                        break;
                    }
                }
            }
        }
    } else {
        // No filters — return all entries in time range
        for (size_t idx = start_idx; idx < end_idx; ++idx) {
            results.push_back(&m_entries[idx]);
            if (results.size() >= params.limit) {
                break;
            }
        }
    }

    // 3. Apply text query filter if specified
    if (params.HasTextQuery()) {
        std::vector<const LogEntry*> filtered;
        filtered.reserve(results.size());
        std::string_view needle = *params.text_query;
        for (const LogEntry* entry : results) {
            if (entry->GetMessage().find(needle) != std::string::npos) {
                filtered.push_back(entry);
            }
        }
        results = std::move(filtered);
    }

    return results;
}

[[nodiscard]] size_t InMemoryIndex::GetSize() const noexcept {
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    return m_entries.size();
}

[[nodiscard]] uint64_t InMemoryIndex::GetOldestTimestampNs() const noexcept {
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if (m_entries.empty()) {
        return 0;
    }
    return m_entries.front().GetTimestampNs();
}

[[nodiscard]] uint64_t InMemoryIndex::GetNewestTimestampNs() const noexcept {
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);
    if (m_entries.empty()) {
        return 0;
    }
    return m_entries.back().GetTimestampNs();
}

[[nodiscard]] std::pair<size_t, size_t> InMemoryIndex::BinarySearchTimeRangeInternal(
    uint64_t start_ns, uint64_t end_ns) const {
    // Find first entry with timestamp >= start_ns
    size_t start_idx = 0;
    {
        auto it = std::lower_bound(
            m_entries.begin(), m_entries.end(), start_ns,
            [](const LogEntry& entry, uint64_t val) {
                return entry.GetTimestampNs() < val;
            });
        start_idx = static_cast<size_t>(std::distance(m_entries.begin(), it));
    }

    // Find first entry with timestamp > end_ns (exclusive upper bound)
    size_t end_idx = m_entries.size();
    {
        auto it = std::upper_bound(
            m_entries.begin(), m_entries.end(), end_ns,
            [](uint64_t val, const LogEntry& entry) {
                return val < entry.GetTimestampNs();
            });
        end_idx = static_cast<size_t>(std::distance(m_entries.begin(), it));
    }

    return {start_idx, end_idx};
}

void InMemoryIndex::MergeFilterResultsInternal(
    std::vector<const LogEntry*>& results,
    const QueryParams& params) const {
    if (!params.log_level.has_value() || !params.service_name.has_value()) {
        return;
    }

    LogLevel level = *params.log_level;
    const std::string& service = *params.service_name;

    if (static_cast<size_t>(level) >= m_level_buckets.size()) {
        return;
    }

    const std::vector<size_t>& level_indices = m_level_buckets[static_cast<size_t>(level)];
    auto service_it = m_service_index.find(service);
    if (service_it == m_service_index.end()) {
        return;
    }
    const std::vector<size_t>& service_indices = service_it->second;

    // Two-pointer intersection of sorted index vectors, restricted to time range
    auto [time_start, time_end] = BinarySearchTimeRangeInternal(
        params.start_time_ns.value_or(0),
        params.end_time_ns.value_or(UINT64_MAX));

    size_t i = 0, j = 0;
    while (i < level_indices.size() && j < service_indices.size()) {
        if (level_indices[i] == service_indices[j]) {
            size_t idx = level_indices[i];
            if (idx >= time_start && idx < time_end) {
                results.push_back(&m_entries[idx]);
                if (results.size() >= params.limit) {
                    break;
                }
            }
            ++i;
            ++j;
        } else if (level_indices[i] < service_indices[j]) {
            ++i;
        } else {
            ++j;
        }
    }
}

void InMemoryIndex::RebuildSecondaryIndexes() {
    m_level_buckets.fill({});
    m_service_index.clear();

    for (size_t i = 0; i < m_entries.size(); ++i) {
        const LogEntry& entry = m_entries[i];

        // Level bucket
        LogLevel level = entry.GetLogLevel();
        if (static_cast<size_t>(level) < m_level_buckets.size()) {
            m_level_buckets[static_cast<size_t>(level)].push_back(i);
        }

        // Service index
        m_service_index[entry.GetServiceName()].push_back(i);
    }
}

}  // namespace log_analyzer

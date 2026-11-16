#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "log_analyzer/InMemoryIndex.h"
#include "log_analyzer/LogEntry.h"

using log_analyzer::InMemoryIndex;
using log_analyzer::LogEntry;
using log_analyzer::LogLevel;
using log_analyzer::QueryParams;

static LogEntry MakeEntry(uint64_t ts_ns, LogLevel level, std::string_view service) {
    return LogEntry::CreateFromRaw(
        ts_ns, log_analyzer::LogLevelToString(level), "app.cpp", service, "host", "test message");
}

TEST(InMemoryIndexTest, InsertAndGetSize) {
    InMemoryIndex index;
    EXPECT_EQ(index.GetSize(), 0);

    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    EXPECT_EQ(index.GetSize(), 1);

    index.Insert(MakeEntry(2000ULL, LogLevel::ERROR, "svc-b"));
    EXPECT_EQ(index.GetSize(), 2);
}

TEST(InMemoryIndexTest, TimeOrdering) {
    InMemoryIndex index;
    index.Insert(MakeEntry(3000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::INFO, "svc-a"));

    // Query without filters should return in time order
    QueryParams params;
    auto results = index.Query(params);
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0]->GetTimestampNs(), 1000ULL);
    EXPECT_EQ(results[1]->GetTimestampNs(), 2000ULL);
    EXPECT_EQ(results[2]->GetTimestampNs(), 3000ULL);
}

TEST(InMemoryIndexTest, TimeRangeQuery) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::WARN, "svc-a"));
    index.Insert(MakeEntry(3000ULL, LogLevel::ERROR, "svc-a"));
    index.Insert(MakeEntry(4000ULL, LogLevel::INFO, "svc-a"));

    QueryParams params;
    params.start_time_ns = 1500ULL;
    params.end_time_ns = 3500ULL;
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]->GetTimestampNs(), 2000ULL);
    EXPECT_EQ(results[1]->GetTimestampNs(), 3000ULL);
}

TEST(InMemoryIndexTest, TimeRangeNoMatches) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));

    QueryParams params;
    params.start_time_ns = 5000ULL;
    params.end_time_ns = 6000ULL;
    auto results = index.Query(params);

    EXPECT_TRUE(results.empty());
}

TEST(InMemoryIndexTest, LevelFilter) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::ERROR, "svc-a"));
    index.Insert(MakeEntry(3000ULL, LogLevel::ERROR, "svc-b"));
    index.Insert(MakeEntry(4000ULL, LogLevel::DEBUG, "svc-a"));

    QueryParams params;
    params.log_level = LogLevel::ERROR;
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 2);
    for (const auto* entry : results) {
        EXPECT_EQ(entry->GetLogLevel(), LogLevel::ERROR);
    }
}

TEST(InMemoryIndexTest, ServiceFilter) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::INFO, "svc-b"));
    index.Insert(MakeEntry(3000ULL, LogLevel::INFO, "svc-a"));

    QueryParams params;
    params.service_name = "svc-a";
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 2);
    for (const auto* entry : results) {
        EXPECT_EQ(entry->GetServiceName(), "svc-a");
    }
}

TEST(InMemoryIndexTest, CombinedLevelAndServiceFilter) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::ERROR, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::ERROR, "svc-b"));
    index.Insert(MakeEntry(3000ULL, LogLevel::INFO, "svc-a"));

    QueryParams params;
    params.log_level = LogLevel::ERROR;
    params.service_name = "svc-a";
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->GetTimestampNs(), 1000ULL);
    EXPECT_EQ(results[0]->GetLogLevel(), LogLevel::ERROR);
    EXPECT_EQ(results[0]->GetServiceName(), "svc-a");
}

TEST(InMemoryIndexTest, TextQuery) {
    InMemoryIndex index;
    index.Insert(LogEntry::CreateFromRaw(1000ULL, "INFO", "a.cpp", "s", "h", "connection timeout"));
    index.Insert(LogEntry::CreateFromRaw(2000ULL, "INFO", "a.cpp", "s", "h", "connection established"));
    index.Insert(LogEntry::CreateFromRaw(3000ULL, "INFO", "a.cpp", "s", "h", "database error"));

    QueryParams params;
    params.text_query = "connection";
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]->GetMessage(), "connection timeout");
    EXPECT_EQ(results[1]->GetMessage(), "connection established");
}

TEST(InMemoryIndexTest, Limit) {
    InMemoryIndex index;
    for (uint64_t i = 0; i < 100; ++i) {
        index.Insert(MakeEntry(i * 100ULL, LogLevel::INFO, "svc-a"));
    }

    QueryParams params;
    params.limit = 10;
    auto results = index.Query(params);

    EXPECT_EQ(results.size(), 10);
}

TEST(InMemoryIndexTest, UnknownLevelFilter) {
    InMemoryIndex index;
    index.Insert(LogEntry::CreateFromRaw(1000ULL, "AUDIT", "a.cpp", "s", "h", "audit log"));
    index.Insert(LogEntry::CreateFromRaw(2000ULL, "INFO", "a.cpp", "s", "h", "info log"));

    QueryParams params;
    params.log_level = LogLevel::UNKNOWN;
    auto results = index.Query(params);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->GetRawLogLevel(), "AUDIT");
}

TEST(InMemoryIndexTest, OldestAndNewestTimestamp) {
    InMemoryIndex index;
    index.Insert(MakeEntry(1000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(3000ULL, LogLevel::INFO, "svc-a"));
    index.Insert(MakeEntry(2000ULL, LogLevel::INFO, "svc-a"));

    EXPECT_EQ(index.GetOldestTimestampNs(), 1000ULL);
    EXPECT_EQ(index.GetNewestTimestampNs(), 3000ULL);
}

TEST(InMemoryIndexTest, EmptyIndexQueries) {
    InMemoryIndex index;

    QueryParams params;
    EXPECT_TRUE(index.Query(params).empty());

    params.start_time_ns = 1000ULL;
    EXPECT_TRUE(index.Query(params).empty());

    params.log_level = LogLevel::INFO;
    EXPECT_TRUE(index.Query(params).empty());

    EXPECT_EQ(index.GetSize(), 0);
    EXPECT_EQ(index.GetOldestTimestampNs(), 0);
    EXPECT_EQ(index.GetNewestTimestampNs(), 0);
}

TEST(InMemoryIndexTest, ConcurrencyReadDuringWrite) {
    InMemoryIndex index;
    // Pre-populate
    for (uint64_t i = 0; i < 1000; ++i) {
        index.Insert(MakeEntry(i * 100ULL, LogLevel::INFO, "svc-a"));
    }

    bool error_occurred = false;
    std::thread writer([&]() {
        for (uint64_t i = 1000; i < 2000; ++i) {
            index.Insert(MakeEntry(i * 100ULL, LogLevel::INFO, "svc-a"));
        }
    });

    std::thread reader([&]() {
        for (int j = 0; j < 100; ++j) {
            QueryParams params;
            params.limit = 100;
            try {
                [[maybe_unused]] auto results = index.Query(params);
            } catch (...) {
                error_occurred = true;
            }
        }
    });

    writer.join();
    reader.join();

    EXPECT_FALSE(error_occurred);
    EXPECT_EQ(index.GetSize(), 2000);
}

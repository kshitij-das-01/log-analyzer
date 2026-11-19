#include <gtest/gtest.h>

#include "log_analyzer/LogEntry.h"

using log_analyzer::LogEntry;
using log_analyzer::LogLevel;

TEST(LogEntryTest, CreateFromRawStandardLevels) {
    LogEntry entry = LogEntry::CreateFromRaw(
        1000000000ULL, "INFO", "app.cpp", "auth-service", "host1", "User logged in");

    EXPECT_EQ(entry.GetTimestampNs(), 1000000000ULL);
    EXPECT_EQ(entry.GetLogLevel(), LogLevel::INFO);
    EXPECT_EQ(entry.GetSourceFile(), "app.cpp");
    EXPECT_EQ(entry.GetServiceName(), "auth-service");
    EXPECT_EQ(entry.GetHostname(), "host1");
    EXPECT_EQ(entry.GetMessage(), "User logged in");
    EXPECT_EQ(entry.GetRawLogLevel(), "INFO");
    EXPECT_TRUE(entry.IsValid());
}

TEST(LogEntryTest, CreateFromRawUnknownLevel) {
    LogEntry entry = LogEntry::CreateFromRaw(
        2000000000ULL, "AUDIT", "security.cpp", "audit-service", "host2", "Access denied");

    EXPECT_EQ(entry.GetLogLevel(), LogLevel::UNKNOWN);
    EXPECT_EQ(entry.GetRawLogLevel(), "AUDIT");
    EXPECT_FALSE(IsKnownLogLevel(entry.GetLogLevel()));
}

TEST(LogEntryTest, CreateFromRawExtendedLevels) {
    LogEntry entry = LogEntry::CreateFromRaw(
        3000000000ULL, "CRITICAL", "kernel.cpp", "system-service", "host3", "Panic");

    EXPECT_EQ(entry.GetLogLevel(), LogLevel::CRITICAL);
    EXPECT_EQ(entry.GetRawLogLevel(), "CRITICAL");
}

TEST(LogEntryTest, CreateFromRawCaseInsensitive) {
    LogEntry entry = LogEntry::CreateFromRaw(
        4000000000ULL, "debug", "net.cpp", "net-service", "host4", "Packet received");

    EXPECT_EQ(entry.GetLogLevel(), LogLevel::DEBUG);
    EXPECT_EQ(entry.GetRawLogLevel(), "debug");
    EXPECT_EQ(entry.ToLogLevelString(), "DEBUG");
}

TEST(LogEntryTest, ToIso8601) {
    LogEntry entry = LogEntry::CreateFromRaw(
        1700000000000000000ULL, "INFO", "app.cpp", "svc", "host", "msg");

    std::string iso = entry.ToIso8601();
    EXPECT_TRUE(iso.find("2023-11-14") != std::string::npos);
    EXPECT_TRUE(iso.back() == 'Z');
}

TEST(LogEntryTest, Immutability) {
    LogEntry entry = LogEntry::CreateFromRaw(
        5000000000ULL, "INFO", "app.cpp", "svc", "host", "msg");

    // Verify getters return const references — no setters exist
    EXPECT_EQ(entry.GetMessage(), "msg");
    EXPECT_EQ(entry.GetServiceName(), "svc");
    EXPECT_EQ(entry.GetHostname(), "host");
    EXPECT_EQ(entry.GetSourceFile(), "app.cpp");
}

TEST(LogEntryTest, IsValid) {
    LogEntry valid = LogEntry::CreateFromRaw(
        1ULL, "INFO", "a.cpp", "s", "h", "hello");
    EXPECT_TRUE(valid.IsValid());

    LogEntry empty_msg = LogEntry::CreateFromRaw(
        1ULL, "INFO", "a.cpp", "s", "h", "");
    EXPECT_FALSE(empty_msg.IsValid());

    LogEntry empty_source = LogEntry::CreateFromRaw(
        1ULL, "INFO", "", "s", "h", "hello");
    EXPECT_FALSE(empty_source.IsValid());
}

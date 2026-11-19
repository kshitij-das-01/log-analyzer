#include <gtest/gtest.h>

#include "log_analyzer/LogLevel.h"

using log_analyzer::LogLevel;
using log_analyzer::ParseLogLevel;
using log_analyzer::LogLevelToString;
using log_analyzer::IsKnownLogLevel;

TEST(LogLevelTest, ParseStandardLevels) {
    EXPECT_EQ(ParseLogLevel("DEBUG"), LogLevel::DEBUG);
    EXPECT_EQ(ParseLogLevel("INFO"), LogLevel::INFO);
    EXPECT_EQ(ParseLogLevel("WARN"), LogLevel::WARN);
    EXPECT_EQ(ParseLogLevel("ERROR"), LogLevel::ERROR);
}

TEST(LogLevelTest, ParseCaseInsensitive) {
    EXPECT_EQ(ParseLogLevel("debug"), LogLevel::DEBUG);
    EXPECT_EQ(ParseLogLevel("Debug"), LogLevel::DEBUG);
    EXPECT_EQ(ParseLogLevel("INFO"), LogLevel::INFO);
    EXPECT_EQ(ParseLogLevel("Info"), LogLevel::INFO);
    EXPECT_EQ(ParseLogLevel("error"), LogLevel::ERROR);
}

TEST(LogLevelTest, ParseVariants) {
    EXPECT_EQ(ParseLogLevel("dbg"), LogLevel::DEBUG);
    EXPECT_EQ(ParseLogLevel("warning"), LogLevel::WARN);
    EXPECT_EQ(ParseLogLevel("err"), LogLevel::ERROR);
    EXPECT_EQ(ParseLogLevel("crit"), LogLevel::CRITICAL);
}

TEST(LogLevelTest, ParseUnknownReturnsUnknown) {
    EXPECT_EQ(ParseLogLevel("AUDIT"), LogLevel::UNKNOWN);
    EXPECT_EQ(ParseLogLevel("SECURITY"), LogLevel::UNKNOWN);
    EXPECT_EQ(ParseLogLevel(""), LogLevel::UNKNOWN);
    EXPECT_EQ(ParseLogLevel("random"), LogLevel::UNKNOWN);
}

TEST(LogLevelTest, IsKnownLogLevel) {
    EXPECT_TRUE(IsKnownLogLevel(LogLevel::DEBUG));
    EXPECT_TRUE(IsKnownLogLevel(LogLevel::INFO));
    EXPECT_TRUE(IsKnownLogLevel(LogLevel::ERROR));
    EXPECT_FALSE(IsKnownLogLevel(LogLevel::UNKNOWN));
}

TEST(LogLevelTest, LogLevelToString) {
    EXPECT_EQ(LogLevelToString(LogLevel::DEBUG), "DEBUG");
    EXPECT_EQ(LogLevelToString(LogLevel::INFO), "INFO");
    EXPECT_EQ(LogLevelToString(LogLevel::WARN), "WARN");
    EXPECT_EQ(LogLevelToString(LogLevel::ERROR), "ERROR");
    EXPECT_EQ(LogLevelToString(LogLevel::CRITICAL), "CRITICAL");
    EXPECT_EQ(LogLevelToString(LogLevel::FATAL), "FATAL");
    EXPECT_EQ(LogLevelToString(LogLevel::TRACE), "TRACE");
    EXPECT_EQ(LogLevelToString(LogLevel::UNKNOWN), "UNKNOWN");
}

TEST(LogLevelTest, ComparisonOperators) {
    EXPECT_TRUE(LogLevel::DEBUG < LogLevel::INFO);
    EXPECT_TRUE(LogLevel::INFO <= LogLevel::INFO);
    EXPECT_TRUE(LogLevel::ERROR > LogLevel::WARN);
    EXPECT_TRUE(LogLevel::ERROR >= LogLevel::ERROR);
    EXPECT_TRUE(LogLevel::DEBUG != LogLevel::ERROR);
    EXPECT_TRUE(LogLevel::INFO == LogLevel::INFO);
}

TEST(LogLevelTest, UnderlyingValues) {
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::UNKNOWN), 0);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::TRACE), 1);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::DEBUG), 2);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::INFO), 3);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::WARN), 4);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::ERROR), 5);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::CRITICAL), 6);
    EXPECT_EQ(static_cast<uint8_t>(LogLevel::FATAL), 7);
}

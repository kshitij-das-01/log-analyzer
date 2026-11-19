#ifndef LOG_ANALYZER_QUERY_PARAMS_H
#define LOG_ANALYZER_QUERY_PARAMS_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "log_analyzer/LogLevel.h"

namespace log_analyzer {

    struct QueryParams {
        std::optional<uint64_t> start_time_ns;
        std::optional<uint64_t> end_time_ns;
        std::optional<LogLevel> log_level;
        std::optional<std::string> service_name;
        std::optional<std::string> text_query;
        size_t limit = 1000;

        [[nodiscard]] bool HasTimeRange() const noexcept {
            return start_time_ns.has_value() || end_time_ns.has_value();
        }

        [[nodiscard]] bool HasLevelFilter() const noexcept {
            return log_level.has_value();
        }

        [[nodiscard]] bool HasServiceFilter() const noexcept {
            return service_name.has_value();
        }

        [[nodiscard]] bool HasTextQuery() const noexcept {
            return text_query.has_value() && !text_query->empty();
        }

        [[nodiscard]] bool IsEmpty() const noexcept {
            return !HasTimeRange() && !HasLevelFilter() && !HasServiceFilter() && !HasTextQuery();
        }
    };

}  // namespace log_analyzer

#endif  // LOG_ANALYZER_QUERY_PARAMS_H

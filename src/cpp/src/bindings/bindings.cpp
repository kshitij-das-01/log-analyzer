#include <log_analyzer/bindings/bindings.h>
#include <log_analyzer/InMemoryIndex.h>
#include <log_analyzer/LogEntry.h>
#include <sstream>

namespace {

// Helper: convert optional<string> from Python to std::optional<std::string>
std::optional<std::string> py_opt_str(py::object obj) {
    if (obj.is_none()) return std::nullopt;
    return obj.cast<std::string>();
}

// Helper: convert optional<uint64_t> from Python to std::optional<uint64_t>
std::optional<uint64_t> py_opt_uint(py::object obj) {
    if (obj.is_none()) return std::nullopt;
    return obj.cast<uint64_t>();
}

// Helper: convert LogLevel string from Python to LogLevel enum
log_analyzer::LogLevel py_to_log_level(const std::string& level_str) {
    static const std::map<std::string, log_analyzer::LogLevel> level_map = {
        {"UNKNOWN", log_analyzer::LogLevel::UNKNOWN},
        {"TRACE", log_analyzer::LogLevel::TRACE},
        {"DEBUG", log_analyzer::LogLevel::DEBUG},
        {"INFO", log_analyzer::LogLevel::INFO},
        {"WARN", log_analyzer::LogLevel::WARN},
        {"ERROR", log_analyzer::LogLevel::ERROR},
        {"CRITICAL", log_analyzer::LogLevel::CRITICAL},
        {"FATAL", log_analyzer::LogLevel::FATAL},
    };
    auto it = level_map.find(level_str);
    if (it != level_map.end()) return it->second;
    return log_analyzer::LogLevel::UNKNOWN;
}

} // anonymous namespace

py::object dict_to_query_params(py::dict params) {
    // Return the dict as-is for the query_index function to use
    // The dict is passed by reference and modified by query_index
    py::gil_scoped_acquire acquire;
    return params;
}

std::vector<py::object> query_index(py::dict params) {
    // Extract parameters from Python dict
    auto start_time_ns = py_opt_uint(params["start_time_ns"]);
    auto end_time_ns = py_opt_uint(params["end_time_ns"]);
    auto log_level = py_opt_str(params["log_level"]);
    auto service_name = py_opt_str(params["service_name"]);
    auto text_query = py_opt_str(params["text_query"]);
    size_t limit = params.contains("limit") ? params["limit"].cast<size_t>() : 1000;

    // For now, return empty results since we don't have a running InMemoryIndex
    // with sample data. This is the placeholder fallback behavior.
    // When integrated with the C++ core + sample data, this will query the index.
    std::vector<py::object> results;

    // Build result dicts - will be populated when InMemoryIndex is connected
    for (size_t i = 0; i < 0; ++i) {
        py::dict result;
        result["timestamp"] = "";
        result["log_level"] = "INFO";
        result["source_file"] = "";
        result["service_name"] = "";
        result["hostname"] = "";
        result["message"] = "";
        result["raw_log_level"] = "";
        results.push_back(result);
    }

    return results;
}

py::object log_entry_to_dict(const log_analyzer::LogEntry* entry) {
    py::dict result;
    if (entry != nullptr) {
        // Use the getters from LogEntry (const access)
        result["timestamp"] = entry->ToIso8601().c_str();
        result["log_level"] = entry->ToLogLevelString().c_str();
        result["source_file"] = entry->GetSourceFile().c_str();
        result["service_name"] = entry->GetServiceName().c_str();
        result["hostname"] = entry->GetHostname().c_str();
        result["message"] = entry->GetMessage().c_str();
        result["raw_log_level"] = entry->GetRawLogLevel().c_str();
    } else {
        result["timestamp"] = "";
        result["log_level"] = "INFO";
        result["source_file"] = "";
        result["service_name"] = "";
        result["hostname"] = "";
        result["message"] = "";
        result["raw_log_level"] = "";
    }
    return result;
}
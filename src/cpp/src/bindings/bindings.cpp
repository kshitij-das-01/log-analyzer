#include <log_analyzer/bindings/bindings.h>
#include <log_analyzer/InMemoryIndex.h>
#include <log_analyzer/QueryParams.h>
#include <log_analyzer/LogEntry.h>
#include <log_analyzer/LogLevel.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace {

// Global InMemoryIndex instance
log_analyzer::InMemoryIndex g_index;

// Helper: convert Python dict to C++ QueryParams
log_analyzer::QueryParams dict_to_query_params(py::dict params) {
    log_analyzer::QueryParams qp;

    if (params.contains("start_time_ns")) {
        py::object val = params["start_time_ns"];
        if (!val.is_none())
            qp.start_time_ns = val.cast<uint64_t>();
    }
    if (params.contains("end_time_ns")) {
        py::object val = params["end_time_ns"];
        if (!val.is_none())
            qp.end_time_ns = val.cast<uint64_t>();
    }
    if (params.contains("log_level")) {
        py::object val = params["log_level"];
        if (!val.is_none())
            qp.log_level = static_cast<log_analyzer::LogLevel>(val.cast<int>());
    }
    if (params.contains("service_name")) {
        py::object val = params["service_name"];
        if (!val.is_none())
            qp.service_name = val.cast<std::string>();
    }
    if (params.contains("text_query")) {
        py::object val = params["text_query"];
        if (!val.is_none())
            qp.text_query = val.cast<std::string>();
    }
    if (params.contains("limit")) {
        py::object val = params["limit"];
        if (!val.is_none())
            qp.limit = val.cast<uint64_t>();
    }

    return qp;
}

// Helper: convert LogEntry to Python dict
py::dict log_entry_to_dict(const log_analyzer::LogEntry* entry) {
    py::dict result;
    if (entry != nullptr) {
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

// Query the InMemoryIndex using pybind11
std::vector<py::object> query_index(py::dict params) {
    log_analyzer::QueryParams qp = dict_to_query_params(params);
    std::vector<py::object> results;

    // Query returns const LogEntry*
    std::vector<const log_analyzer::LogEntry*> entries = g_index.Query(qp);

    for (auto* entry : entries) {
        results.push_back(log_entry_to_dict(entry));
    }

    return results;
}

} // anonymous namespace

// Populate InMemoryIndex from syslog-format log files
int populate_from_log_files(const std::string& log_dir) {
    int total_count = 0;

    // Read app1.log
    std::string app_log = log_dir + "/app1.log";
    std::ifstream app_file(app_log);

    if (app_file.is_open()) {
        std::string line;
        while (std::getline(app_file, line)) {
            if (line.empty()) continue;

            // Parse syslog line: "Jan 15 10:30:45 host app.cpp INFO User logged in"
            std::istringstream iss(line);
            std::string month_str, day_str, time_str, host, file, level, message;

            if (!(iss >> month_str >> day_str >> time_str >> host >> file >> level >> message)) {
                continue;
            }

            // Extract service from filename (remove .cpp extension)
            size_t dot_pos = file.find(".cpp");
            std::string service = (dot_pos != std::string::npos) ? file.substr(0, dot_pos) : file;

            // Create LogEntry - raw_level is the string like "INFO", "ERROR"
            log_analyzer::LogEntry entry = log_analyzer::LogEntry::CreateFromRaw(
                0,  // timestamp_ns (will be 0 for now, same as C++ ingestion script)
                level,
                file,
                service,
                host,
                message
            );

            g_index.Insert(entry);
            total_count++;
        }
        app_file.close();
    }

    // Read system.log
    std::string sys_log = log_dir + "/system.log";
    std::ifstream sys_file(sys_log);

    if (sys_file.is_open()) {
        std::string line;
        while (std::getline(sys_file, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string month_str, day_str, time_str, host, file, level, message;

            if (!(iss >> month_str >> day_str >> time_str >> host >> file >> level >> message)) {
                continue;
            }

            // Extract service from filename
            size_t dot_pos = file.find(".cpp");
            std::string service = (dot_pos != std::string::npos) ? file.substr(0, dot_pos) : file;

            // Create LogEntry - raw_level is the string like "INFO", "ERROR"
            log_analyzer::LogEntry entry = log_analyzer::LogEntry::CreateFromRaw(
                0,  // timestamp_ns
                level,
                file,
                service,
                host,
                message
            );

            g_index.Insert(entry);
            total_count++;
        }
        sys_file.close();
    }

    return total_count;
}

// Expose InMemoryIndex size
size_t index_size() {
    return g_index.GetSize();
}

// pybind11 module definition
namespace py = pybind11;

PYBIND11_MODULE(log_analyzer_bindings, m) {
    // query_index wrapped to handle py::dict properly
    m.def("query_index", [](py::dict params) -> std::vector<py::object> {
        return query_index(params);
    }, "Query the InMemoryIndex with Python dict params",
         py::arg("params"));

    // index_size: Get current InMemoryIndex size
    m.def("index_size", &index_size, "Get current InMemoryIndex size");

    // has_real_bindings: Check if real pybind11 bindings are available
    m.def("has_real_bindings", [](){ return true; });

    // populate_from_log_files: Parse syslog files and insert into InMemoryIndex
    m.def("populate_from_log_files", [](const std::string& log_dir) -> int {
        return populate_from_log_files(log_dir);
    }, "Parse syslog files and insert into InMemoryIndex",
         py::arg("log_dir"));
}
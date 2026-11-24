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
        std::cerr << "DEBUG: Opened app1.log" << std::endl;
        std::string line;
        int line_num = 0;
        while (std::getline(app_file, line)) {
            line_num++;
            if (line.empty()) continue;

            // Parse syslog line: "Jan 15 10:30:45 host app.cpp INFO User logged in"
            std::istringstream iss(line);
            std::string month_str, day_str, time_str, host, file, level, message;

            if (!(iss >> month_str >> day_str >> time_str >> host >> file >> level >> message)) {
                std::cerr << "DEBUG: Line " << line_num << " failed to parse" << std::endl;
                continue;
            }
            std::cerr << "DEBUG: Line " << line_num << " parsed: level=" << level << " file=" << file << " message=" << message << std::endl;

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

            std::cerr << "DEBUG: Created entry, about to insert. entry.GetRawLogLevel()=" << entry.GetRawLogLevel() << std::endl;
            g_index.Insert(entry);
            total_count++;
            std::cerr << "DEBUG: Inserted entry, total_count=" << total_count << std::endl;
        }
        app_file.close();
        std::cerr << "DEBUG: Finished app1.log, total_count=" << total_count << std::endl;
    } else {
        std::cerr << "DEBUG: Failed to open app1.log" << std::endl;
    }

    // Read system.log
    std::string sys_log = log_dir + "/system.log";
    std::ifstream sys_file(sys_log);

    if (sys_file.is_open()) {
        std::cerr << "DEBUG: Opened system.log" << std::endl;
        std::string line;
        int line_num = 0;
        while (std::getline(sys_file, line)) {
            line_num++;
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string month_str, day_str, time_str, host, file, level, message;

            if (!(iss >> month_str >> day_str >> time_str >> host >> file >> level >> message)) {
                std::cerr << "DEBUG: system.log line " << line_num << " failed to parse" << std::endl;
                continue;
            }
            std::cerr << "DEBUG: system.log line " << line_num << " parsed: level=" << level << " file=" << file << " message=" << message << std::endl;

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
        std::cerr << "DEBUG: Finished system.log, total_count=" << total_count << std::endl;
    } else {
        std::cerr << "DEBUG: Failed to open system.log" << std::endl;
    }

    std::cerr << "DEBUG: populate_from_log_files returning total_count=" << total_count << std::endl;
    return total_count;
}

// pybind11 module definition
namespace py = pybind11;

PYBIND11_MODULE(log_analyzer_bindings, m) {
    // query_index wrapped to handle py::dict properly
    m.def("query_index", [](py::dict params) -> std::vector<py::object> {
        return query_index(params);
    }, "Query the InMemoryIndex with Python dict params");

    // index_size wrapped
    m.def("index_size", [](py::object = py::none()) -> size_t {
        return g_index.GetSize();
    }, "Get current InMemoryIndex size");

    m.def("has_real_bindings", [](){ return true; });

    // populate_from_log_files wrapped
    m.def("populate_from_log_files", [](const std::string& log_dir) -> int {
        return populate_from_log_files(log_dir);
    }, "Parse syslog files and insert into InMemoryIndex");
}
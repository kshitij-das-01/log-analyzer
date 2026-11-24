#include "log_analyzer/InMemoryIndex.h"
#include "log_analyzer/LogEntry.h"
#include "log_analyzer/LogLevel.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>

// Syslog month lookup
static const std::map<std::string, int> month_lookup = {
    {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
    {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
    {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}
};

// Parse a syslog line: "Jan 15 10:30:45 host app.cpp INFO User logged in"
bool parse_syslog_line(const std::string& line, 
                       std::string& out_service,
                       std::string& out_message,
                       std::string& out_level,
                       std::string& out_host,
                       std::string& out_file) {
    std::istringstream iss(line);
    std::string month_str, day_str, time_str, host, file, level, message;
    
    if (!(iss >> month_str >> day_str >> time_str >> host >> file >> level >> message)) {
        return false;
    }
    
    // Extract service from filename (remove .cpp extension)
    size_t dot_pos = file.find(".cpp");
    std::string service = (dot_pos != std::string::npos) ? file.substr(0, dot_pos) : file;
    
    auto month_it = month_lookup.find(month_str);
    if (month_it == month_lookup.end()) return false;
    
    out_service = service;
    out_message = message;
    out_level = level;
    out_host = host;
    out_file = file;
    
    return true;
}

int main(int argc, char* argv[]) {
    std::string log_dir = "sample-data/logs";
    if (argc > 1) {
        log_dir = argv[1];
    }
    
    log_analyzer::InMemoryIndex index;
    int loaded_count = 0;
    
    // Parse app1.log
    std::string app_log = log_dir + "/app1.log";
    std::ifstream app_file(app_log);
    if (app_file.is_open()) {
        std::string line;
        while (std::getline(app_file, line)) {
            if (line.empty()) continue;
            
            std::string service, message, level, host, file;
            
            if (parse_syslog_line(line, service, message, level, host, file)) {
                log_analyzer::LogEntry entry = log_analyzer::LogEntry::CreateFromRaw(
                    /*timestamp_ns=*/0,
                    /*raw_level=*/level,
                    /*source_file=*/file,
                    /*service_name=*/service,
                    /*hostname=*/host,
                    /*message=*/message
                );
                
                index.Insert(entry);
                loaded_count++;
            }
        }
        app_file.close();
    }
    
    // Parse system.log
    std::string sys_log = log_dir + "/system.log";
    std::ifstream sys_file(sys_log);
    if (sys_file.is_open()) {
        std::string line;
        while (std::getline(sys_file, line)) {
            if (line.empty()) continue;
            
            std::string service, message, level, host, file;
            
            if (parse_syslog_line(line, service, message, level, host, file)) {
                log_analyzer::LogEntry entry = log_analyzer::LogEntry::CreateFromRaw(
                    /*timestamp_ns=*/0,
                    /*raw_level=*/level,
                    /*source_file=*/file,
                    /*service_name=*/service,
                    /*hostname=*/host,
                    /*message=*/message
                );
                
                index.Insert(entry);
                loaded_count++;
            }
        }
        sys_file.close();
    }
    
    // Print summary
    std::cout << "=== Ingestion Summary ===" << std::endl;
    std::cout << "Entries loaded: " << loaded_count << std::endl;
    std::cout << "Index size: " << index.GetSize() << std::endl;
    std::cout << "Oldest timestamp: " << index.GetOldestTimestampNs() << std::endl;
    std::cout << "Newest timestamp: " << index.GetNewestTimestampNs() << std::endl;
    
    // Print sample entries
    std::vector<const log_analyzer::LogEntry*> results;
    index.Query(log_analyzer::QueryParams());
    std::cout << "\nSample entries:" << std::endl;
    for (size_t i = 0; i < std::min<size_t>(5, results.size()); i++) {
        std::cout << "  " << results[i]->GetMessage() << " ("
                  << log_analyzer::LogLevelToString(results[i]->GetLogLevel()) << ")" << std::endl;
    }
    
    std::cout << "=== Done ===" << std::endl;
    return 0;
}

#pragma once

#include <pybind11/pybind11.h>
#include <log_analyzer/InMemoryIndex.h>
#include <log_analyzer/QueryParams.h>
#include <vector>
#include <string>
#include <map>

namespace py = pybind11;

// Convert a Python dict to log_analyzer::QueryParams
py::object dict_to_query_params(py::dict params);

// Build a single LogEntry dict from a LogEntry*
py::object log_entry_to_dict(const log_analyzer::LogEntry* entry);
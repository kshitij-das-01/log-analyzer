"""pybind11 bindings loader for C++ InMemoryIndex.

Try to load the real pybind11 shared library first.
If build fails or module not found, fall back to placeholder functions.
"""

import sys
import os
import importlib
from typing import Optional, Any, Dict, List


# --- Fallback placeholder if pybind11 not available ---


def _placeholder_query_index(params: Dict[str, Any]) -> List[Dict[str, Any]]:
    """Placeholder query when pybind11 bindings not built.

    Returns empty results so API doesn't crash.
    """
    return []


# --- Try to load real pybind11 bindings ---


_cpp_bindings_module: Optional[Any] = None
_has_real_bindings = False


def _try_load_real_bindings() -> None:
    """Attempt to import the pybind11 bindings module.

    Searches for log_analyzer_bindings in the Python path or
    in the C++ build directory adjacent to the source.
    """
    global _cpp_bindings_module, _has_real_bindings

    # List of paths to search for the shared library
    search_paths = []

    # 1. Check if already imported
    if "log_analyzer_bindings" in sys.modules:
        _cpp_bindings_module = sys.modules["log_analyzer_bindings"]
        _has_real_bindings = True
        return

    # 2. Check standard Python path entries
    for path in sys.path:
        if path and os.path.isdir(path):
            search_paths.append(os.path.join(path, "log_analyzer_bindings.cpython-*"))

    # 3. Check C++ build directory (common patterns)
    project_root = os.getenv("LOG_ANALYZER_PROJECT_ROOT")
    if project_root:
        build_dirs = [
            os.path.join(project_root, "src", "cpp", "build"),
            os.path.join(project_root, "src", "cpp", "cmake-build-debug"),
        ]
        for bd in build_dirs:
            if os.path.isdir(bd):
                search_paths.append(os.path.join(bd, "lib_analyzer_bindings.cpython-*"))

    # 4. Check current directory and relative paths
    rel_paths = [
        "src/cpp/build",
        "src/cpp/cmake-build-debug",
    ]
    for rp in rel_paths:
        abs_rp = os.path.abspath(rp)
        if os.path.isdir(abs_rp):
            search_paths.append(os.path.join(abs_rp, "liblog_analyzer_bindings.cpython-*"))

    # Try to import from each search path
    for pattern in search_paths:
        try:
            # Use importlib to find the module
            spec = importlib.util.find_spec("log_analyzer_bindings")
            if spec is not None and spec.origin:
                _cpp_bindings_module = importlib.import_module("log_analyzer_bindings")
                _has_real_bindings = True
                # print(f"[bindings] Loaded real pybind11 module from: {spec.origin}")
                return
        except Exception:
            continue

    # If we get here, real bindings not found - will use placeholder
    _cpp_bindings_module = None
    _has_real_bindings = False


# Attempt initial load
_try_load_real_bindings()


def query_index(params: Dict[str, Any]) -> List[Dict[str, Any]]:
    """Query the C++ In-Memory Index using pybind11 bindings.

    Args:
        params: Dictionary with query parameters:
            - start_time_ns: int or None (nanoseconds since epoch)
            - end_time_ns: int or None (nanoseconds since epoch)
            - log_level: str or None (e.g. "INFO", "ERROR")
            - service_name: str or None
            - text_query: str or None (full-text search)
            - limit: int, default 1000

    Returns:
        List of log entry dictionaries. Empty list if bindings not available.
    """
    if _has_real_bindings and _cpp_bindings_module is not None:
        try:
            # Call the C++ query function
            func = getattr(_cpp_bindings_module, "query_index", None)
            if func is not None and callable(func):
                # Pass params as dict
                result = func(params)
                if isinstance(result, list):
                    # Ensure all items are dicts
                    return [item if isinstance(item, dict) else dict(item) for item in result]
                elif isinstance(result, dict):
                    return [result]
            # If func returns None or fails, fall through to placeholder
        except Exception as e:
            # Binding error - fall through to placeholder
            pass

    # Fallback to placeholder
    return _placeholder_query_index(params)


def has_real_bindings() -> bool:
    """Check if real pybind11 bindings are loaded."""
    return _has_real_bindings


def reload_bindings() -> None:
    """Reload the bindings module (useful after rebuild)."""
    global _cpp_bindings_module, _has_real_bindings
    _cpp_bindings_module = None
    _has_real_bindings = False
    _try_load_real_bindings()
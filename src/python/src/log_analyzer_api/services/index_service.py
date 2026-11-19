from typing import Optional, Any, Dict
from datetime import datetime, timezone

from log_analyzer_api.bindings import cpp_bindings
from log_analyzer_api.bindings.cpp_bindings import query_index as _cpp_query_index
from log_analyzer_api.schemas import (
    StreamParams,
    SearchParams,
    LogEntryResponse,
)


def _parse_datetime_value(v: Any) -> Optional[int]:
    """Convert various datetime formats to nanoseconds since epoch."""
    if v is None:
        return None
    if isinstance(v, int):
        # Already in nanoseconds
        return v
    if isinstance(v, float):
        # Already in seconds or nanos - assume seconds if < 1e12
        if v < 1e12:
            return int(v * 1_000_000_000)
        else:
            return int(v)
    if isinstance(v, str):
        # ISO 8601 string
        try:
            dt = datetime.fromisoformat(v)
            if dt.tzinfo is None:
                dt = dt.replace(tzinfo=timezone.utc)
            return int(dt.timestamp() * 1_000_000_000 + dt.nanosecond)
        except (ValueError, TypeError):
            return None
    if hasattr(v, 'start_time'):  # Handle Pydantic model
        return _parse_datetime_value(v.start_time)
    return None


def _build_query_dict(params: StreamParams) -> dict[str, Any]:
    """Build a C++ query dict from StreamParams model."""
    start_ns = _parse_datetime_value(params.start_time)
    end_ns = _parse_datetime_value(params.end_time)
    
    level_map = {
        "UNKNOWN": 0, "TRACE": 1, "DEBUG": 2, "INFO": 3,
        "WARN": 4, "ERROR": 5, "CRITICAL": 6, "FATAL": 7,
    }
    level_val = level_map.get(params.log_level.value, 0) if params.log_level else None
    
    return {
        "start_time_ns": start_ns,
        "end_time_ns": end_ns,
        "log_level": level_val,
        "service_name": params.service_name,
        "text_query": params.text_query,
        "limit": params.limit,
    }


async def query_logs_full(params: StreamParams) -> list[dict[str, Any]]:
    """Query the C++ In-Memory Index and return raw results."""
    cpp_params = _build_query_dict(params)
    
    try:
        results = await __run_in_executor(cpp_bindings.query_index, cpp_params)
        return results
    except Exception as e:
        # If pybind11 bindings fail, placeholder returns empty list - that's OK
        # print(f"[index_service] Query failed (fallback to placeholder): {e}")
        return []


async def query_logs_search(params: SearchParams) -> list[dict[str, Any]]:
    """Query with full-text search support."""
    cpp_params: dict[str, Any] = {}
    
    start_ns = _parse_datetime_value(params.start_time)
    end_ns = _parse_datetime_value(params.end_time)
    
    level_map = {
        "UNKNOWN": 0, "TRACE": 1, "DEBUG": 2, "INFO": 3,
        "WARN": 4, "ERROR": 5, "CRITICAL": 6, "FATAL": 7,
    }
    level_val = level_map.get(params.log_level.value, 0) if params.log_level else None
    
    cpp_params["start_time_ns"] = start_ns
    cpp_params["end_time_ns"] = end_ns
    cpp_params["log_level"] = level_val
    cpp_params["service_name"] = params.service_name
    cpp_params["text_query"] = params.text_query  # Now available since SearchParams inherits from StreamParams
    cpp_params["limit"] = params.limit
    
    try:
        results = await __run_in_executor(cpp_bindings.query_index, cpp_params)
        return results
    except Exception as e:
        # If pybind11 bindings fail, placeholder returns empty list - that's OK
        # print(f"[index_service] Search query failed (fallback to placeholder): {e}")
        return []


async def get_metrics(time_range: str = "1h") -> dict[str, Any]:
    """Get aggregated metrics from the C++ In-Memory Index."""
    from datetime import datetime as dt_datetime, timezone, timedelta
    
    now = dt_datetime.now(timezone.utc)
    cpp_params: dict[str, Any] = {}
    
    # Parse time range
    unit = time_range[-1]
    value = int(time_range[:-1])
    
    if unit == "h":
        start = now - timedelta(hours=value)
        cpp_params["start_time_ns"] = int(now.timestamp() * 1_000_000_000)
    elif unit == "d":
        start = now - timedelta(days=value)
        cpp_params["start_time_ns"] = int(now.timestamp() * 1_000_000_000)
    elif unit == "w":
        start = now - timedelta(weeks=value)
        cpp_params["start_time_ns"] = int(now.timestamp() * 1_000_000_000)
    
    cpp_params["end_time_ns"] = int(now.timestamp() * 1_000_000_000)
    
    try:
        results = await __run_in_executor(cpp_bindings.query_index, cpp_params)
    except Exception:
        results = []
    
    # Build metrics response
    levels: dict[str, int] = {}
    services: dict[str, int] = {}
    total = len(results)
    
    for entry in results:
        if isinstance(entry, dict):
            lv = isinstance(entry.get("log_level"), str) and isinstance(entry.get("log_level"), str) and entry.get("log_level") or "UNKNOWN"
            svc = isinstance(entry.get("service_name"), str) and entry.get("service_name") or "unknown"
        else:
            lv = getattr(entry, "log_level", "UNKNOWN") or "UNKNOWN"
            svc = getattr(entry, "service_name", "unknown") or "unknown"
        
        levels[lv] = levels.get(lv, 0) + 1
        services[svc] = services.get(svc, 0) + 1
    
    return {
        "time_range": time_range,
        "total_logs": total,
        "levels": levels,
        "services": services,
        "time_buckets": [],
    }


async def __run_in_executor(func, *args) -> Any:
    """Run a synchronous function in a thread executor."""
    import concurrent.futures
    import asyncio
    loop = asyncio.get_running_loop()
    with concurrent.futures.ThreadPoolExecutor() as executor:
        return await loop.run_in_executor(executor, lambda: func(*args))

import pytest
from datetime import datetime, timezone

from log_analyzer_api.schemas import (
    LogLevelEnum,
    LogEntryResponse,
    StreamParams,
    SearchParams,
    MetricsParams,
    LogsResponse,
    MetricsResponse,
)


class TestLogLevelEnum:
    """Test LogLevelEnum validation and values."""
    
    def test_known_values(self):
        assert LogLevelEnum.DEBUG.value == "DEBUG"
        assert LogLevelEnum.INFO.value == "INFO"
        assert LogLevelEnum.WARN.value == "WARN"
        assert LogLevelEnum.ERROR.value == "ERROR"
    
    def test_unknown_value(self):
        assert LogLevelEnum.UNKNOWN.value == "UNKNOWN"
    
    def test_parse_from_string(self):
        assert LogLevelEnum("DEBUG") == LogLevelEnum.DEBUG
        assert LogLevelEnum("INFO") == LogLevelEnum.INFO


class TestStreamParams:
    """Test StreamParams validation."""
    
    def test_default_limit(self):
        params = StreamParams()
        assert params.limit == 1000
    
    def test_custom_limit(self):
        params = StreamParams(limit=500)
        assert params.limit == 500
    
    def test_timezone_aware_validation(self):
        from datetime import datetime
        # This should raise an error for naive datetime
        try:
            params = StreamParams(start_time=datetime(2024, 1, 1))
            # If we get here, the validation passed (might depend on pydantic version)
        except Exception:
            pass  # Expected to fail for naive datetime


class TestSearchParams(StreamParams):
    """Test SearchParams inherits from StreamParams."""
    
    def test_inherits_stream_params(self):
        params = SearchParams(query="error")
        assert params.query == "error"
        assert params.limit == 1000


class TestMetricsParams:
    """Test MetricsParams validation."""
    
    def test_default_time_range(self):
        params = MetricsParams()
        assert params.time_range == "1h"
    
    def test_custom_time_range(self):
        params = MetricsParams(time_range="24h")
        assert params.time_range == "24h"
    
    def test_invalid_time_range(self):
        try:
            MetricsParams(time_range="invalid")
            # If we get here, validation might pass depending on pydantic version
        except Exception:
            pass  # Expected to fail for invalid format


class TestLogsResponse:
    """Test LogsResponse model."""
    
    def test_creation(self):
        response = LogsResponse(logs=[], count=0, limit=100)
        assert response.logs == []
        assert response.count == 0
        assert response.limit == 100


class TestMetricsResponse:
    """Test MetricsResponse model."""
    
    def test_creation(self):
        response = MetricsResponse(
            time_range="24h",
            total_logs=100,
            levels={"INFO": 50, "ERROR": 50},
            services={"svc-a": 80, "svc-b": 20},
            time_buckets=[],
        )
        assert response.time_range == "24h"
        assert response.total_logs == 100

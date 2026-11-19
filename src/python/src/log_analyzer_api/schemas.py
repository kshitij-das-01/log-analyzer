from typing import Optional

from pydantic import BaseModel, Field


class LogLevel(str):
    """Log level string values."""
    UNKNOWN = "UNKNOWN"
    TRACE = "TRACE"
    DEBUG = "DEBUG"
    INFO = "INFO"
    WARN = "WARN"
    ERROR = "ERROR"
    CRITICAL = "CRITICAL"
    FATAL = "FATAL"


class LogEntryResponse(BaseModel):
    """Response model for a single log entry."""
    timestamp: str = Field(..., description="ISO 8601 timestamp with nanoseconds")
    log_level: LogLevel = Field(..., description="Normalized log level")
    source_file: str = Field(..., description="Source file path")
    service_name: str = Field(..., description="Service/application name")
    hostname: str = Field(..., description="Hostname where log originated")
    message: str = Field(..., description="Full log message")
    raw_log_level: str = Field(..., description="Original log level string from source")

    model_config = {"arbitrary_types_allowed": True}


class StreamParams(BaseModel):
    """Query parameters for /api/logs/stream endpoint."""
    start_time: Optional[str] = Field(None, description="Start time ISO 8601")
    end_time: Optional[str] = Field(None, description="End time ISO 8601")
    log_level: Optional[LogLevel] = Field(None, description="Filter by log level")
    service_name: Optional[str] = Field(None, description="Filter by service name")
    limit: int = Field(1000, ge=1, le=10000, description="Maximum results to return")
    text_query: Optional[str] = Field(None, description="Full-text search query")

    model_config = {"arbitrary_types_allowed": True}


class SearchParams(StreamParams):
    """Query parameters for /api/logs/search endpoint (extends StreamParams)."""
    pass


class MetricsParams(BaseModel):
    """Query parameters for /api/dashboard/metrics endpoint."""
    time_range: str = Field("1h", pattern=r"^\d+[hdw]$", description="Time range: e.g., '1h', '24h', '7d', '1w'")

    model_config = {"arbitrary_types_allowed": True}


class LogsResponse(BaseModel):
    """Response wrapper for logs endpoints."""
    logs: list[LogEntryResponse]
    count: int
    limit: int

    model_config = {"arbitrary_types_allowed": True}


class MetricsResponse(BaseModel):
    """Response wrapper for metrics endpoint."""
    time_range: str
    total_logs: int
    levels: dict[str, int]
    services: dict[str, int]
    time_buckets: list[dict]

    model_config = {"arbitrary_types_allowed": True}

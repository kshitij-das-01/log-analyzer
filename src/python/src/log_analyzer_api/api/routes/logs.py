from fastapi import APIRouter, Query
from typing import Optional

from log_analyzer_api.schemas import StreamParams, SearchParams

from log_analyzer_api.services.index_service import query_logs_full, query_logs_search


router = APIRouter(prefix="/api/logs", tags=["logs"])


@router.get("/stream", response_model=None)
async def stream_logs(
    start_time: Optional[str] = Query(None, description="Start time ISO 8601"),
    end_time: Optional[str] = Query(None, description="End time ISO 8601"),
    log_level: Optional[str] = Query(None, description="Filter by log level"),
    service_name: Optional[str] = Query(None, description="Filter by service name"),
    limit: int = Query(1000, ge=1, le=10000, description="Max results"),
    text_query: Optional[str] = Query(None, description="Full-text search query"),
):
    """Stream log entries within a time range with optional filters."""
    try:
        from datetime import datetime, timezone
        
        start_dt = datetime.fromisoformat(start_time) if start_time else None
        end_dt = datetime.fromisoformat(end_time) if end_time else None
        
        if start_dt and start_dt.tzinfo is None:
            start_dt = start_dt.replace(tzinfo=timezone.utc)
        if end_dt and end_dt.tzinfo is None:
            end_dt = end_dt.replace(tzinfo=timezone.utc)
        
        params = StreamParams(
            start_time=start_time,
            end_time=end_time,
            log_level=log_level,
            service_name=service_name,
            limit=limit,
            text_query=text_query,
        )
        
        results = await query_logs_full(params)
        
        # Build response manually since response_model is None
        entries = []
        for r in results:
            entries.append({
                "timestamp": r.get("timestamp", ""),
                "log_level": r.get("log_level", "INFO"),
                "source_file": r.get("source_file", ""),
                "service_name": r.get("service_name", ""),
                "hostname": r.get("hostname", ""),
                "message": r.get("message", ""),
                "raw_log_level": r.get("raw_log_level", ""),
            })
        
        return {"logs": entries, "count": len(entries), "limit": limit}
        
    except Exception as e:
        return {"error": str(e)}


@router.get("/search", response_model=None)
async def search_logs(
    query: Optional[str] = Query(None, description="Full-text search query"),
    start_time: Optional[str] = Query(None, description="Start time ISO 8601"),
    end_time: Optional[str] = Query(None, description="End time ISO 8601"),
    log_level: Optional[str] = Query(None, description="Filter by log level"),
    service_name: Optional[str] = Query(None, description="Filter by service name"),
    limit: int = Query(1000, ge=1, le=10000, description="Max results"),
):
    """Search logs with full-text and field-based filtering."""
    try:
        from datetime import datetime, timezone
        
        start_dt = datetime.fromisoformat(start_time) if start_time else None
        end_dt = datetime.fromisoformat(end_time) if end_time else None
        
        if start_dt and start_dt.tzinfo is None:
            start_dt = start_dt.replace(tzinfo=timezone.utc)
        if end_dt and end_dt.tzinfo is None:
            end_dt = end_dt.replace(tzinfo=timezone.utc)
        
        params = SearchParams(
            start_time=start_time,
            end_time=end_time,
            log_level=log_level,
            service_name=service_name,
            query=query,
            limit=limit,
        )
        
        results = await query_logs_search(params)
        
        entries = []
        for r in results:
            entries.append({
                "timestamp": r.get("timestamp", ""),
                "log_level": r.get("log_level", "INFO"),
                "source_file": r.get("source_file", ""),
                "service_name": r.get("service_name", ""),
                "hostname": r.get("hostname", ""),
                "message": r.get("message", ""),
                "raw_log_level": r.get("raw_log_level", ""),
            })
        
        return {"logs": entries, "count": len(entries), "limit": limit}
        
    except Exception as e:
        return {"error": str(e)}

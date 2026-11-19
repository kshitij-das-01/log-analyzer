from fastapi import APIRouter, Query
from typing import Optional

from log_analyzer_api.services.index_service import get_metrics


router = APIRouter(prefix="/api/dashboard", tags=["dashboard"])


@router.get("/metrics")
async def dashboard_metrics(
    time_range: str = Query("1h", pattern=r"^\d+[hdw]$", description="Time range: 1h, 24h, 7d, 1w"),
):
    """Get aggregated dashboard metrics for the given time range."""
    try:
        metrics = await get_metrics(time_range=time_range)
        return metrics
    except Exception as e:
        return {"error": str(e)}

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from log_analyzer_api.api.routes.logs import router as logs_router
from log_analyzer_api.api.routes.dashboard import router as dashboard_router

app = FastAPI(
    title="Log Analyzer API",
    description="Python FastAPI orchestration layer for the Log Analyzer C++ core",
    version="0.1.0",
)

# Add CORS middleware for frontend access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Include route registries
app.include_router(logs_router)
app.include_router(dashboard_router)


@app.get("/", tags=["root"])
async def root():
    """Root endpoint returning API status."""
    return {
        "message": "Log Analyzer API is running",
        "version": "0.1.0",
        "docs": "/docs",
        "api_prefix": "/api",
    }


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

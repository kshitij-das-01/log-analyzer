"""Integration tests for Log Analyzer API - mirrors C++ GoogleTest patterns.

Tests end-to-end flow: C++ InMemoryIndex → pybind11 → FastAPI → HTTP response.
Follows the same pattern as src/cpp/tests/ GoogleTest suite.
"""

from fastapi.testclient import TestClient
import pytest

from log_analyzer_api.main import app

client = TestClient(app)


class TestLogStream:
    """Stream endpoint tests - mirrors GoogleTest TEST_CASE pattern."""

    def test_stream_returns_200(self):
        """Stream endpoint returns 200 OK."""
        response = client.get("/api/logs/stream?limit=10")
        assert response.status_code == 200

    def test_stream_returns_logs_with_sample_data(self):
        """Stream endpoint returns actual log entries (not empty) when sample data is indexed."""
        response = client.get("/api/logs/stream?limit=100")
        assert response.status_code == 200
        data = response.json()
        # With sample data loaded, should have entries
        assert data["count"] > 0, f"Expected logs > 0, got count={data['count']}"
        assert "logs" in data
        assert len(data["logs"]) > 0


class TestLogSearch:
    """Search endpoint tests - mirrors GoogleTest TEST_CASE pattern."""

    def test_search_returns_200(self):
        """Search endpoint returns 200 OK."""
        response = client.get("/api/logs/search?query=error")
        assert response.status_code == 200

    def test_search_returns_filtered_results(self):
        """Search with query filter returns results matching the query."""
        response = client.get("/api/logs/search?query=ERROR")
        assert response.status_code == 200
        data = response.json()
        assert data["count"] > 0, f"Expected logs with 'ERROR', got count={data['count']}"
        # Verify all returned logs contain the query term
        for entry in data["logs"]:
            message = entry.get("message", "").upper()
            assert "ERROR" in message or "ERROR" in entry.get("raw_log_level", ""), \
                f"Expected 'ERROR' in entry, got: {entry}"


class TestDashboardMetrics:
    """Metrics endpoint tests - mirrors GoogleTest TEST_CASE pattern."""

    def test_metrics_returns_200(self):
        """Metrics endpoint returns 200 OK."""
        response = client.get("/api/dashboard/metrics?time_range=1h")
        assert response.status_code == 200

    def test_metrics_returns_aggregated_data(self):
        """Metrics returns total_logs > 0 and populated levels/services with sample data."""
        response = client.get("/api/dashboard/metrics?time_range=1h")
        assert response.status_code == 200
        data = response.json()
        assert data["total_logs"] > 0, f"Expected total_logs > 0, got {data['total_logs']}"
        assert "levels" in data, "Expected 'levels' in metrics response"
        assert "services" in data, "Expected 'services' in metrics response"
        assert data["time_range"] == "1h"


class TestIntegrationPatterns:
    """Tests that mirror exact C++ GoogleTest patterns."""

    def test_googletest_style_require(self):
        """Mirror: REQUIRE(result.has_value()) pattern from GoogleTest."""
        response = client.get("/api/logs/search?query=login")
        assert response.status_code == 200
        data = response.json()
        # LIKE GoogleTest REQUIRE - will fail if condition false
        assert data["count"] > 0

    def test_googletest_style_sections(self):
        """Mirror: SECTION blocks for related tests."""
        # Stream test section
        s_resp = client.get("/api/logs/stream?limit=5")
        assert s_resp.status_code == 200
        # Search test section
        search_resp = client.get("/api/logs/search?query=ERROR")
        assert search_resp.status_code == 200
        # Metrics test section
        m_resp = client.get("/api/dashboard/metrics?time_range=1h")
        assert m_resp.status_code == 200
        # All three passed = like SECTIONS all passing
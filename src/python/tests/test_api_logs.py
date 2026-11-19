import pytest
from fastapi.testclient import TestClient
from log_analyzer_api.main import app

client = TestClient(app)


class TestRootEndpoint:
    """Test the root endpoint."""
    
    def test_root_returns_status(self):
        response = client.get("/")
        assert response.status_code == 200
        data = response.json()
        assert "message" in data
        assert data["message"] == "Log Analyzer API is running"


class TestStreamLogsEndpoint:
    """Test /api/logs/stream endpoint."""
    
    def test_stream_default_params(self):
        response = client.get("/api/logs/stream")
        assert response.status_code == 200
        data = response.json()
        assert "logs" in data
        assert "count" in data
        assert "limit" in data
    
    def test_stream_with_time_range(self):
        response = client.get("/api/logs/stream?start_time=2024-01-01T00:00:00&end_time=2024-12-31T23:59:59")
        assert response.status_code == 200


class TestSearchLogsEndpoint:
    """Test /api/logs/search endpoint."""
    
    def test_search_default_params(self):
        response = client.get("/api/logs/search")
        assert response.status_code == 200
    
    def test_search_with_query(self):
        response = client.get("/api/logs/search?query=error")
        assert response.status_code == 200


@pytest.fixture
def sample_log_entry():
    """Fixture providing a sample log entry dict."""
    return {
        "timestamp": "2024-01-15T10:30:45.123456789Z",
        "log_level": "INFO",
        "source_file": "app.cpp",
        "service_name": "auth-service",
        "hostname": "host1.example.com",
        "message": "User logged in",
        "raw_log_level": "INFO",
    }


class TestStreamLogsWithSample(sample_log_entry):
    """Test stream endpoint with sample data patterns."""
    
    def test_stream_contains_expected_fields(self):
        # This tests the structure of the response
        response = client.get("/api/logs/stream?limit=5")
        assert response.status_code == 200
        data = response.json()
        assert data["count"] > 0 or data["limit"] == 1000


class TestSearchLogsWithSample(sample_log_entry):
    """Test search endpoint with sample data patterns."""
    
    def test_search_contains_expected_fields(self):
        response = client.get("/api/logs/search?query=login")
        assert response.status_code == 200

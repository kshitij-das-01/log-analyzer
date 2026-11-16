# System Architecture Document (SAD): Log Analyzer v1.0

## 1. Purpose and Scope
This document outlines the conceptual architecture and data flow for the Log Analyzer system. Its purpose is to define the boundaries, interactions, and state transitions between the major components, ensuring a high degree of modularity and resilience.

## 2. Architectural Principles
*   **Asynchronous First:** All major data transitions (Ingestion $\rightarrow$ Processing) must happen asynchronously via Kafka to maintain high throughput and decouple failure domains.
*   **Layered Separation:** Clear boundaries must exist between the high-speed computational core (C++) and the request-handling/business logic layer (Python).
*   **Single Source of Truth:** The `LogEntry` data schema (defined in Section 4) serves as the canonical data structure throughout the system.

## 3. System Component View (Boundary Definition)
The system is logically divided into four primary, interacting components:

### 3.1 Log Source / Edge Layer (External)
*   **Inputs:** Raw log files, text streams, network taps.
*   **Role:** Provides the initial, raw, unstructured data.
*   **Mechanism:** Handled by the C++ Ingestor Service.

### 3.2 Ingestion Pipeline (C++)
*   **Inputs:** Raw Log Sources (from Section 3.1).
*   **Processes:**
    1.  **Parsing:** Reads source data, applies initial format detection.
    2.  **Normalization:** Transforms various raw formats into the standardized `LogEntry` structure.
    3.  **Publishing:** Serializes the structured data and pushes it to the Kafka `raw_logs_topic`.
*   **Output:** Message payloads on the Kafka `raw_logs_topic`.

### 3.3 Processing Core (C++)
*   **Inputs:** Message payloads from Kafka `raw_logs_topic`.
*   **Processes:**
    1.  **Consumption:** Reads and consumes raw logs from Kafka.
    2.  **Enrichment & Indexing:** Executes complex correlation rules (e.g., geo-IP lookup, user ID resolution, pattern matching) and writes the final, optimized record to the In-Memory Index.
*   **Output:** Updates to the **In-Memory Index** (The primary, fast-read data store).

### 3.4 API & Presentation Layer (Python/React)
*   **Inputs:** Direct query requests from the Frontend (via REST API).
*   **Processes:**
    1.  **Request Translation:** Validates user parameters (time range, filters) and translates them into an optimized query protocol for the C++ Core.
    2.  **Query Execution:** Invokes the C++ binding layer to execute the search/aggregation on the In-Memory Index.
    3.  **Response Formatting:** Receives the raw results and formats them into a clean, paginated JSON object for the Frontend.
*   **Output:** JSON responses consumed by the React/TS Frontend.

## 4. Data Flow Diagram (State Transitions)

The flow illustrates the movement of the log data through its lifecycle states:

**[Raw Data]** $\xrightarrow{\text{Ingestion Module}}$ **[Kafka Topic (Transient State)]** $\xrightarrow{\text{Processing Core}}$ **[In-Memory Index (Persistent State)]** $\xrightarrow{\text{API Backend Query}}$ **[JSON Result Payload]** $\xrightarrow{\text{Display}}$ **[Frontend UI]**

### 4.1 State Definition: LogEntry
The system relies on the consistent `LogEntry` state. This state must be maintained across all components and is defined as:

| Field | Type | Mandatory | Description |
| :--- | :--- | :--- | :--- |
| `timestamp` | ISO 8601 String | YES | The event time. |
| `log_level` | String | YES | e.g., INFO, WARN, ERROR. |
| `source_file` | String | YES | Originating file path. |
| `service_name` | String | YES | Application generating log. |
| `hostname` | String | YES | Machine identifier. |
| `message` | String | YES | Full log content. |

## 5. Interaction Details: The Query Path (Deep Dive)

This path is the most critical interaction point between the API and the C++ Core.

1.  **Trigger:** User requests `GET /api/logs/search?query=failure&time_range=1h`.
2.  **Python (Orchestrator):** Receives parameters. Packages them into a structured query object (e.g., a tuple/dataclass: `(time_start, time_end, text_query)`).
3.  **Python $\rightarrow$ C++:** Calls the exposed C++ binding method: `tools.query_index(query_object)`.
4.  **C++ (Core):** Executes the query:
    *   Identifies the required time boundaries within the `In-Memory Index` structure.
    *   Performs an optimized index lookup (e.g., using B-trees or specialized hashing) to fetch raw `LogEntry` records matching the query.
    *   Returns a contiguous block of memory pointers/data representing the found results.
5.  **C++ $\rightarrow$ Python:** Returns the raw, filtered result set.
6.  **Python (Orchestrator):** Serializes the result set into a list of JSON objects and returns the final payload.

## 6. Assumptions & Limitations
*   **Kafka:** Assumes a stable, highly available Kafka cluster is available.
*   **C++ Bindings:** Assumes a reliable, performant binding mechanism (e.g., `pybind11`) is in place for C++ $\leftrightarrow$ Python communication.
*   **Scalability Limit:** The In-Memory Index size is limited by available RAM; disk persistence and recovery must be addressed in a future version.

# Log Analyzer Project: System Design Specification

## 1. Problem Statement
> I want to build **General-Purpose Log Analyzer Web Application** so that **developers and system administrators** can **efficiently ingest, analyze, correlate, and visualize logs from any source** for rapid debugging and system monitoring.

## 2. Constraints & Goals
| Question | Answer |
| :--- | :--- |
| **Who uses it?** | Developers, DevOps Engineers, System Administrators, Security Analysts. |
| **What problem does it solve?** | Provides a unified interface to analyze massive volumes of disparate log data, enabling fast root cause analysis, anomaly detection, and system health monitoring. |
| **Where does it run?** | Cloud-native or on-premise deployment (flexible, but requires robust containerization). |
| **Hard constraints** | Must handle high-volume data streams; core processing must be extremely fast; must be highly decoupled. |
| **Success criteria (MVP)** | Successful ingestion of a standard log format; successful query and display of time-series log data; ability to filter by log level and service. |
| **Out of scope** | Advanced machine learning-based anomaly detection (for V1), full-fledged database migrations, or advanced security features. |

## 3. Scope Tiers

| Tier | Features | Description |
| :--- | :--- | :--- |
| **MVP** | Log Ingestion, In-Memory Indexing, REST API (Stream View, Filter Search), Basic Dashboard Metrics. | Focus on proving the end-to-end pipeline works end-to-end with the defined data model. |
| **v1** | Multi-format ingestion support, Basic Alerting mechanism, Dashboard visualizations (time-series graphs). | Expanding data source support and introducing visual analytics. |
| **Future** | Advanced Anomaly Detection (ML), Automated Log Normalization, Full Security Auditing. | Incorporating advanced AI and automated operational features. |

## 4. Domain Model (The Data Schema)

This section defines the canonical structure for every log entry, which is used for both ingestion and querying.

### 4.1 Entities: `LogEntry`

The structure for all log entries, which will be stored in the C++ In-Memory Index.

| Entity | Attributes | Description | Purpose |
| :--- | :--- | :--- | :--- |
| `timestamp` | ISO 8601 String | The exact time the event occurred. | Essential for all time-series queries. |
| `log_level` | String (Enum) | e.g., `INFO`, `WARN`, `ERROR`, `DEBUG`. | Primary filter for categorization. |
| `source_file` | String | The file where the log originated. | Pinpointing the source of the error. |
| `service_name` | String | The application or service generating the log. | Grouping logs by application. |
| `hostname` | String | The machine/server that generated the log. | System context. |
| `message` | String | The full, raw log message. | The primary content for searching and display. |

### 4.2 Business Rules / Invariants
*   **Immutability:** Once a `LogEntry` is indexed, it must never be modified.
*   **Time Ordering:** All records must be strictly ordered by `timestamp` within any given category/service group.
*   **Completeness:** All records must contain all mandatory fields listed above to ensure queryability.

## 5. User Stories / Use Cases
1.  **As a Developer, I can upload a large log file, so that the system can ingest and index it efficiently.** (Focus on Ingestion)
2.  **As an Analyst, I can filter logs by time range and log level, so that I can quickly isolate critical errors.** (Focus on Querying)
3.  **As an Administrator, I can view aggregated metrics, so that I can quickly assess system health over time.** (Focus on Dashboarding)
4.  **As a User, I can search the full text of any log message, so that I can pinpoint specific error contexts.** (Focus on Full-Text Search)

## 6. Architecture Diagram (Conceptual Flow)

```mermaid
graph TD
    A[Log Sources (Files/Text)] --> B(C++ Ingestor Service);
    B --> C(Kafka: Raw Logs Topic);
    C --> D(C++ Stream Processor);
    D --> E(In-Memory Index - C++);
    E -- Read/Query --> F(Python API Backend);
    F -- API Call --> E;
    F -- API Call --> G(Web Frontend - React/TS);
```

## 7. Module Decomposition

| Module | Responsibility | Depends On |
| :--- | :--- | :--- |
| **Ingestion Module (C++)** | Reads files, parses, normalizes, and publishes to Kafka. | File System, Kafka Client |
| **Stream Processor (C++)** | Consumes Kafka, applies layered indexing logic, writes to In-Memory Index. | Kafka Consumer, In-Memory Index |
| **Data Storage (C++)** | Manages the core, high-speed, layered in-memory index structure. | N/A |
| **API Backend (Python)** | Manages API requests, orchestrates queries, interfaces with C++ Core. | Kafka (for read access, if needed), C++ Bindings |
| **Frontend (React/TS)** | User interaction, visualization, data display. | Python API |

## 8. Interface Signatures (API Contract)

The Python API will expose the following endpoints:

| Endpoint | Method | Description | Request Parameters | Response Structure |
| :--- | :--- | :--- | :--- | :--- |
| `/api/logs/stream` | `GET` | Fetches raw log entries within a time range. | `start_time`, `end_time`, `log_level`, `service_name` (optional) | `Array<LogEntry>` |
| `/api/logs/search` | `GET` | Performs complex full-text and field-based filtering. | `query` (full-text search), `filters` (e.g., `log_level:ERROR`) | `Array<LogEntry>` |
| `/api/dashboard/metrics` | `GET` | Fetches aggregated statistics for a given time period. | `time_range` (e.g., "1h", "24h") | JSON Object of aggregated metrics. |

## 9. Core Data Flow (Example Use Case: Searching for Errors)

1.  **User Action:** User interacts with the React frontend and requests a filtered view (e.g., "Show all ERRORs from service 'auth' in the last 1 hour").
2.  **API Request:** The frontend sends this request to the Python API: `GET /api/logs/search?query=auth&filters=log_level:ERROR&time_range:1h`.
3.  **Orchestration:** The Python backend receives the request and translates it into a highly optimized query for the C++ In-Memory Index.
4.  **Execution:** The C++ engine executes the query against its layered index, retrieving only the relevant, time-sorted `LogEntry` objects directly from memory.
5.  **Response:** The C++ engine returns the raw results to the Python API, which formats them into the final JSON structure and sends them back to the React frontend for display.

## 10. Tech Stack Summary

| Concern | Selection | Rationale |
| :--- | :--- | :--- |
| **Core Engine** | **C++** | Maximum performance for parsing and indexing (CPU/Memory bound). |
| **Orchestration** | **Python** | Flexibility, fast API development, and excellent bridging capabilities to C++. |
| **Data Transport** | **Apache Kafka** | Decoupling high-velocity ingestion from processing; ensures data durability. |
| **In-Memory Store** | **Custom C++ Structures** | Provides the required speed by avoiding disk I/O for hot data. |
| **Frontend** | **React / TypeScript** | Modern, type-safe, and highly interactive for complex data visualization. |

## 11. Build / Test / Deploy Pipeline
1.  **CI:** Use a system (e.g., GitHub Actions) to trigger builds for C++, Python, and the Frontend.
2.  **Testing:**
    *   **C++:** Extensive unit testing for the log parsing and indexing logic.
    *   **Python:** API endpoint testing and integration testing against mock C++ services.
    *   **Frontend:** Unit and integration tests for UI components.
3.  **CD:** Deploy the services in containers (Docker) orchestrated by Kubernetes (for production scalability).

## 12. Implementation Backlog
| Order | Step | Deliverable | Priority |
| :--- | :--- | :--- | :--- |
| 1 | Repo init + build system | Initialize repositories, set up Dockerfiles, and establish the C++/Python build system (e.g., using `pybind11`). | High |
| 2 | Domain Entities | Finalize the C++ `LogEntry` structure and the In-Memory Index data structures. | High |
| 3 | Ingestion Pipeline | Implement the C++ Ingestor to read files, parse, and publish to Kafka. | High |
| 4 | Stream Processing | Implement the C++ Stream Processor to consume Kafka and populate the In-Memory Index. | High |
| 5 | Python API | Build the Python FastAPI service with the defined endpoints, implementing the query logic to talk to the C++ layer. | Medium |
| 6 | Frontend | Build the core React interface to consume the Python API results and visualize the data. | Medium |
| 7 | Deploy + Smoke Test | Set up the CI/CD pipeline and perform initial end-to-end testing. | Low |
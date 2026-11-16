# Log Analyzer Project: Software Engineering Specification

This document details the structural breakdown, dependencies, and module responsibilities for the Log Analyzer project, focusing on a Software Development perspective.

## 1. Overview & Philosophy

This project is designed around a **Polyglot Microservices** architecture, separating high-performance computational tasks from flexible orchestration tasks. The core philosophy is: **Performance where it matters (C++)** and **Flexibility where it matters (Python/React)**.

## 2. System Components & Dependencies

The system is decomposed into four primary services, communicating asynchronously and synchronously:

| Component | Primary Language | Responsibility | Key Dependencies |
| :--- | :--- | :--- | :--- |
| **Ingestion Service** | C++ | File I/O, raw parsing, data normalization, and queuing. | File System, Kafka Client |
| **Stream Processor** | C++ | Consumes Kafka, applies layered indexing logic, writes to the In-Memory Index. | Kafka Consumer, C++ Data Store Interface |
| **API Backend** | Python (FastAPI) | Handles external requests, user authentication, and orchestrates queries. | C++ Interface (via bindings), Kafka (optional reading) |
| **Frontend** | TypeScript/React | User interface, state management, and data visualization. | Python API |

## 3. Module Boundaries & Responsibilities

### 3.1 Core Engine (C++) - The Data Layer
*   **Modules:** `IngestionModule`, `StreamProcessor`, `InMemoryIndex`, `IndexSerializer`.
*   **Responsibility:** All log reading, parsing, and lightning-fast indexing logic.
*   **Dependency:** Interacts directly with the Kafka stream and manages the memory structure.

### 3.2 Orchestration Layer (Python) - The Application Layer
*   **Modules:** `APIServer`, `QueryHandler`, `KafkaManager`.
*   **Responsibility:** Managing API routes, authenticating requests, and translating high-level user requests into low-level, optimized commands for the C++ core.
*   **Dependency:** Requires a robust interface layer (bindings) to the C++ core.

### 3.3 Data Backbone (Kafka)
*   **Role:** Acts as the immutable, durable stream buffer between the high-speed ingestion and the processing layer.
*   **Role:** Decouples producers (C++) from consumers (C++).

## 4. Interface Definitions (The Contract Layer)

This layer explicitly defines the language used for cross-service communication.

### 4.1 C++ $\leftrightarrow$ Python Interface (The Bridge)
*   **Protocol:** Use **gRPC** (or `pybind11` wrappers for simpler calls) to establish a strongly-typed interface.
*   **Focus:** Defining the exact structure of the data (e.g., `LogEntry`) and the methods for querying the In-Memory Index.

### 4.2 Data Contract (Kafka Message Format)
*   **Format:** **JSON** or **Protobuf**.
*   **Schema:** The schema for all messages on the Kafka topic must strictly adhere to the structure defined in the *Kafka Message Schema Definition* (see separate documentation).

## 5. Implementation Backlog (Refined for Development)

This sequence prioritizes building the core performance layer first.

| Order | Step | Deliverable | Focus Area |
| :--- | :--- | :--- | :--- |
| 1 | **Setup & Binding** | Initialize C++/Python build system; establish the `pybind11` interface structure. | **Infrastructure** |
| 2 | **C++ Data Model** | Finalize the C++ `LogEntry` struct and the In-Memory Index data structures. | **Core Logic** |
| 3 | **Ingestion Pipeline** | Implement the C++ Ingestor to read files, parse, and publish to Kafka. | **Performance** |
| 4 | **Stream Processing** | Implement the C++ Stream Processor to consume Kafka and populate the In-Memory Index. | **Performance** |
| 5 | **Python API** | Build the Python FastAPI service with the defined endpoints, implementing the query logic to talk to the C++ layer. | **Orchestration** |
| 6 | **Frontend** | Build the core React interface to consume the Python API results and visualize the data. | **Presentation** |
| 7 | **Deployment** | Implement the CI/CD pipeline and deploy the services in containers (Docker/K8s). | **DevOps** |
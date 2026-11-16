# Log Analyzer Project — Documentation Hub

This directory contains the authoritative specifications, architectural standards, and operational guidelines for the **Log Analyzer** project — a high-performance, polyglot log analysis web application built with C++ (core engine), Python (orchestration/API), Apache Kafka (data transport), and React/TypeScript (frontend).

---

## Documentation Directory

| Document | Scope & Key Contents | Primary Audience |
| :--- | :--- | :--- |
| [`PROJECT_DESIGN.md`](./PROJECT_DESIGN.md) | **Product & Domain Specification**<br>Problem statement, constraints & goals, scope tiers (MVP/v1/Future), domain model (`LogEntry` schema), business rules/invariants, user stories, tech stack summary, implementation backlog. | All Agents & Engineers |
| [`PROJECT_ENGINEERING_SPEC.md`](./PROJECT_ENGINEERING_SPEC.md) | **Module & Interface Specification**<br>System components & dependencies, module boundaries & responsibilities, interface definitions (C++ ↔ Python bridge, Kafka message format), refined implementation backlog ordered by development priority. | All Agents & Engineers |
| [`PROJECT_ARCHITECTURE_SAD.md`](./PROJECT_ARCHITECTURE_SAD.md) | **System Architecture Document**<br>Component boundary definitions, state transition data flow (Raw → Kafka → In-Memory Index → JSON → UI), C++ ↔ Python query path deep dive, assumptions & limitations. | AI Agents, Lead Architect |
| [`subagent-team.md`](./subagent-team.md) | **Specialized AI Subagent Team**<br>Invocation names, roles, tool grants, core responsibilities, and delegation protocols for `log-architect`, `log-engineer`, `stream-processor`, and `adversarial-reviewer`. | AI Agents |
| [`code-complexity-and-quality.md`](./code-complexity-and-quality.md) | **Code Quality & Decomposition Standards**<br>Cyclomatic complexity ceiling (≤6 for C++/Python), early-return pattern, decomposition rules for ingestion pipelines, stream processors, and API route handlers. | All Engineers |
| [`GITFLOW.md`](./GITFLOW.md) | **Branching & Version Control**<br>Atlassian Gitflow workflow adapted for this repo: `main`, `develop`, `feature/*`, `release/*`, `hotfix/*`, commit conventions. | All Contributors |
| [`WORKFLOW.md`](./WORKFLOW.md) | **Development Workflow**<br>Plan → Implement → Commit → Review cycle, branch lifecycle, definition of done, when to invoke which subagent. | All Agents & Engineers |

---

## Architectural Principles

1. **Performance Where It Matters:** CPU- and memory-bound log parsing/indexing must live in C++. No performance-sensitive logic may leak into Python or TypeScript.
2. **Asynchronous First:** All data transitions between Ingestion (C++) and Processing (C++) must flow through Apache Kafka to decouple failure domains and sustain high throughput.
3. **Decoupled Boundaries:** The C++ core exposes a strictly typed interface to Python (via `pybind11` or gRPC). Python may not directly manipulate C++ memory structures.
4. **Immutable LogEntry:** Once indexed, a `LogEntry` record is never modified. All transformations produce new records.
5. **Time-Ordered First:** All log data within a service/host category must be strictly ordered by `timestamp`.

---

## Quick Reference: Canonical Data Schema (`LogEntry`)

| Field | Type | Mandatory | Purpose |
| :--- | :--- | :--- | :--- |
| `timestamp` | ISO 8601 String | YES | Time-series queries |
| `log_level` | String (Enum) | YES | INFO / WARN / ERROR / DEBUG filtering |
| `source_file` | String | YES | Pinpoint log origin |
| `service_name` | String | YES | Group logs by application |
| `hostname` | String | YES | System context |
| `message` | String | YES | Full-text search & display |

---

## API Contract Quick Reference

| Endpoint | Method | Purpose |
| :--- | :--- | :--- |
| `/api/logs/stream` | `GET` | Fetch log entries within a time range with optional level/service filters |
| `/api/logs/search` | `GET` | Complex full-text + field-based filtering |
| `/api/dashboard/metrics` | `GET` | Aggregated statistics grouped by time bucket |

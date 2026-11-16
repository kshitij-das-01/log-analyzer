# AGENTS.md — Log Analyzer Project

This file is the quick-start guide for any AI agent working in this repository. Read this first, then dive into `docs/` for detailed specifications.

---

## 1. Project Summary

**Log Analyzer** is a high-performance, polyglot log analysis web application. The architecture uses:

- **C++** — Core engine: log parsing, normalization, In-Memory Index, Kafka ingestion/consumption
- **Python (FastAPI)** — Orchestration layer: REST API, C++ binding orchestration, query translation
- **Apache Kafka** — Data transport backbone between ingestion and processing
- **React / TypeScript** — Frontend: log viewing, search, dashboard metrics

**Goal:** Ingest massive log volumes, index them in memory for speed, and expose them via a REST API to a modern frontend.

---

## 2. Tech Stack

| Layer | Technology | Location |
| :--- | :--- | :--- |
| Core Engine | C++ (C++17 or later) | `src/cpp/` |
| Orchestration / API | Python (FastAPI) | `src/python/` |
| Data Transport | Apache Kafka | — |
| Frontend | React / TypeScript | `web_app/` |
| Build System | CMake (C++) + Poetry/Pip (Python) | Root / `src/` |

---

## 3. Key Commands (to be implemented)

> These are placeholders until the implementation backlog begins.

```bash
# C++ core build
cd src/cpp && mkdir -p build && cd build && cmake .. && make

# Python API
cd src/python && poetry install && poetry run uvicorn main:app --reload

# Frontend
cd web_app && npm install && npm run dev

# Lint / type checks
# (TBD per language once scaffolding exists)
```

---

## 4. Documentation Map

| File | Purpose | Read When |
| :--- | :--- | :--- |
| `docs/README.md` | Documentation hub — indexes all specs with scope & audience | First, to understand the doc layout |
| `docs/PROJECT_DESIGN.md` | Product & domain spec — problem statement, `LogEntry` schema, API endpoints | When you need to know what to build |
| `docs/PROJECT_ENGINEERING_SPEC.md` | Module & interface spec — polyglot boundaries, refined backlog | When you need to know where code lives |
| `docs/PROJECT_ARCHITECTURE_SAD.md` | System architecture — component boundaries, data flow state transitions, C++ ↔ Python query path | When you need to know how data moves |
| `docs/subagent-team.md` | Specialized AI subagent team — invocation names, roles, handoff protocol | When delegating to specialized subagents |
| `docs/code-complexity-and-quality.md` | Code quality standards — complexity ≤ 6, decomposition patterns, memory safety | Before writing or reviewing code |
| `docs/GITFLOW.md` | Branching & version control — Atlassian Gitflow, commit conventions | Before creating branches or commits |
| `docs/WORKFLOW.md` | Development workflow — Plan → Implement → Commit → Review cycle, branch lifecycle, definition of done | Before starting any feature work |

---

## 5. Coding Standards Quick Reference

### Universal
- **Cyclomatic complexity ≤ 6** per function (C++, Python, TypeScript)
- **No complexity bypasses:** no `#pragma`, `eslint-disable`, `@ts-ignore`, `# noqa`, broad `except:`
- **Early return pattern:** guard clauses, no deep indentation
- **Immutability:** `LogEntry` records are never mutated after indexing
- **Time ordering:** all indexed records within a service/host group are sorted by `timestamp`

### C++ Specific
- **RAII everywhere:** `std::unique_ptr`, `std::string`, `std::vector` — no raw `new`/`delete`
- **No raw `char*` buffers** in hot paths
- **GIL management** when calling C++ from Python via `pybind11`
- **No dangling references** returned across the C++ ↔ Python boundary

### Python Specific
- **Pydantic models** for all request/response schemas — no `dict[str, Any]` passthrough
- **Strict enum types** for `log_level` (`INFO`, `WARN`, `ERROR`, `DEBUG`)
- **Timezone-aware datetimes** only — naive datetimes rejected at API boundary
- **Specific exception handling** — map C++ binding errors to appropriate HTTP status codes

### TypeScript/React Specific
- **Declarative page components** — data fetching, state, and presentation separated
- **Lookup maps** instead of long `switch`/`if-else` chains (e.g., log level badge colors)

---

## 6. Subagent Invocation Guide

Use the specialized subagents defined in `docs/subagent-team.md` based on task domain:

| Subagent | Invocation Name | Best For |
| :--- | :--- | :--- |
| Log Architect | `log-architect` | C++ core, In-Memory Index, binding contracts, Kafka schema, system architecture |
| Stream Processing Engineer | `stream-processor` | C++ ingestion, log parsing, Kafka publishing/consuming, performance optimization |
| API & Integration Engineer | `log-engineer` | Python FastAPI, REST endpoints, C++ binding orchestration, response serialization |
| Adversarial Reviewer | `adversarial-reviewer` | Security audit, performance invariant checks, PR review (read-only) |

**Example invocation:**
```typescript
invoke_subagent({
  TypeName: 'log-architect',
  Role: 'System & C++ Core Architect',
  Prompt: 'Design the C++ LogEntry struct and layered In-Memory Index structure.'
});
```

---

## 7. Getting Started

1. Read `docs/README.md` to understand the documentation layout.
2. Read `docs/PROJECT_DESIGN.md` for the canonical `LogEntry` schema and API contract.
3. Read `docs/WORKFLOW.md` to understand the Plan → Implement → Commit → Review cycle.
4. Check `docs/subagent-team.md` if you plan to delegate work to specialized subagents.
5. Create a feature branch off `develop` per `docs/GITFLOW.md`.
6. Implement, then verify against `docs/code-complexity-and-quality.md` before marking done.

---

## 8. When in Doubt

- **What to build?** → `docs/PROJECT_DESIGN.md`
- **Where does code live?** → `docs/PROJECT_ENGINEERING_SPEC.md`
- **How does data flow?** → `docs/PROJECT_ARCHITECTURE_SAD.md`
- **Is this code quality acceptable?** → `docs/code-complexity-and-quality.md`
- **Which agent should I invoke?** → `docs/subagent-team.md`
- **Which branch should I use?** → `docs/GITFLOW.md`

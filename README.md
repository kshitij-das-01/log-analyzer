# Log Analyzer

A high-performance, polyglot log analysis web application.

## Architecture

- **C++ Core Engine** — Log parsing, normalization, In-Memory Index, Kafka ingestion/consumption
- **Python (FastAPI)** — Orchestration layer: REST API, C++ binding orchestration, query translation
- **Apache Kafka** — Data transport backbone between ingestion and processing
- **React / TypeScript** — Frontend: log viewing, search, dashboard metrics

## Tech Stack

| Layer | Technology | Location |
| :--- | :--- | :--- |
| Core Engine | C++17 + CMake | `src/cpp/` |
| Orchestration | Python 3.10 + FastAPI + Poetry | `src/python/` |
| Data Transport | Apache Kafka | `docker/` |
| Frontend | React + TypeScript + Vite | `web_app/` |

## Documentation

See [`docs/`](docs/) for complete project specifications:

| Document | Purpose |
| :--- | :--- |
| `docs/README.md` | Documentation hub |
| `docs/PROJECT_DESIGN.md` | Product & domain spec |
| `docs/PROJECT_ENGINEERING_SPEC.md` | Module & interface spec |
| `docs/PROJECT_ARCHITECTURE_SAD.md` | System architecture |
| `docs/WORKFLOW.md` | Development workflow (Plan → Implement → Commit → Push) |
| `docs/code-complexity-and-quality.md` | Code quality standards |
| `docs/GITFLOW.md` | Branching & version control |
| `docs/pull-request-guidelines.md` | PR writing guidelines |
| `docs/database-and-security.md` | Security & mutation rules |
| `docs/subagent-team.md` | AI subagent team |

See [`AGENTS.md`](AGENTS.md) for AI agent quick-start guide.

## Getting Started

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler
- Python 3.10+ with Poetry
- Node.js 18+ with npm
- Apache Kafka (for development)

### C++ Core

```bash
mkdir -p build && cd build
cmake ..
make
```

### Python API

```bash
cd src/python
poetry install
poetry run uvicorn main:app --reload
```

### Frontend

```bash
cd web_app
npm install
npm run dev
```

## Development Workflow

We follow a **Plan → Implement → Commit → Push** cycle:

1. **PLAN** — Discuss scope, design, and data contracts
2. **IMPLEMENT** — Write code per standards (complexity ≤ 6)
3. **COMMIT** — Atomic, descriptive commits (requires commit message review)
4. **PUSH** — Push to remote (requires explicit user approval)

No `git push` is executed without your explicit approval.

See [`docs/WORKFLOW.md`](docs/WORKFLOW.md) for details.
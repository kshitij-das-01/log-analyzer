# Code Complexity, Architecture & Quality Standards — Log Analyzer

> **Applies to:** All C++ source code in `src/cpp/`, all Python source code in `src/python/`, and all TypeScript/React source code in `web_app/`.
> **Hard Ceiling:** Cyclomatic Complexity $\le 6$ per function (C++ and Python).
> **Related Documents:**
> - [`PROJECT_DESIGN.md`](./PROJECT_DESIGN.md) — Product & Domain Specification
> - [`PROJECT_ENGINEERING_SPEC.md`](./PROJECT_ENGINEERING_SPEC.md) — Module & Interface Specification
> - [`PROJECT_ARCHITECTURE_SAD.md`](./PROJECT_ARCHITECTURE_SAD.md) — System Architecture Document

---

## 1. Cyclomatic Complexity $\le 6$ Hard Ceiling

Every function, method, hook, route handler, and React component has an absolute cyclomatic complexity limit of **6**.

- **No Complexity Bypasses:**
  - Never add `#pragma` directives, `__attribute__((always_inline))` hacks, or preprocessor abuse to bypass complexity rules or compiler warnings in C++.
  - Never add `eslint-disable`, `eslint-disable-next-line`, `@ts-ignore`, or `@ts-expect-error` to bypass type checking or linter warnings in TypeScript.
  - Never use `# noqa`, `# type: ignore`, or broad `except:` clauses to bypass complexity rules in Python.
  - Never widen existing disables or suppressions.
  - If a function exceeds complexity 6, refactor and decompose it immediately.

---

## 2. Refactoring & Decomposition Rules

When a unit of code approaches complexity 5 or 6, decompose it according to its architectural role:

### 2.1 C++ Ingestion Pipeline

**Do not combine** file I/O, parsing, normalization, Kafka serialization, and network publishing into one monolithic function.

Extract responsibilities into focused functions:

```cpp
// ❌ Anti-pattern: High complexity (nested ifs, try-catch cascades, multiple branches)
LogEntry parseRawLine(const std::string& rawLine) {
  // 30 lines of nested if-else detecting format, extracting fields, validating...
}

// ✅ Correct Pattern: Composed pipeline with complexity <= 3 per function
LogEntry parseRawLine(const std::string& rawLine) {
  auto format = detectFormat(rawLine);        // complexity 1
  auto fields = extractFields(rawLine, format); // complexity 2
  return normalizeLogEntry(fields);            // complexity 1
}
```

**Decomposition targets:**
- `detectFormat(rawLine)` — returns format enum (syslog, json, nginx, apache, plain)
- `extractFields(rawLine, format)` — returns raw field map
- `normalizeLogEntry(fields)` — maps raw fields to `LogEntry` struct with defaults

### 2.2 C++ Stream Processor

**Do not combine** Kafka consumption, enrichment, indexing, and error handling into one processing loop.

- Extract Kafka message consumption into `consumeBatch(consumer, batchSize)`.
- Extract enrichment logic into `enrichLogEntry(entry)` (e.g., hostname resolution, service tagging).
- Extract index write into `indexWriter.write(entry)` — a pure write operation.
- Extract error handling into `handleParseError(entry, error)` — log-and-skip, never crash.

### 2.3 C++ In-Memory Index

- Index query functions must be pure read operations — no side effects, no mutation of indexed records.
- Keep index traversal logic small; extract comparison predicates into named functors or lambdas.
- Use lookup maps/records instead of long `switch` statements or chained `if / else if` for log level or service routing:

```cpp
// ✅ Map lookup instead of switch/if-chain (Complexity: 1)
static const std::unordered_map<LogLevel, IndexBucket*> LEVEL_TO_BUCKET = {
  {LogLevel::INFO,   &infoBucket},
  {LogLevel::WARN,   &warnBucket},
  {LogLevel::ERROR,  &errorBucket},
  {LogLevel::DEBUG,  &debugBucket},
};
```

### 2.4 Python API Route Handlers

**Do not combine** request parsing, validation, C++ binding invocation, and response serialization into one route handler.

- Extract parameter validation into dedicated Pydantic models or validation functions.
- Extract C++ binding invocation into a service layer (e.g., `log_index_service.query(...)`).
- Extract response serialization into dedicated schema classes.

```python
# ✅ Correct Pattern: Composed pipeline with complexity <= 3 per function
@router.get("/api/logs/search")
async def search_logs(params: SearchParams = Depends()):
    validated = validate_search_params(params)   # complexity 1
    results = log_index_service.query(validated) # complexity 1
    return serialize_results(results)            # complexity 1
```

### 2.5 React/TypeScript Frontend Components

- Keep page components purely declarative — data fetching, state management, and presentation are separate concerns.
- A page component should:
  1. Fetch initial data via API calls (or React Query/SWR).
  2. Pass data into specialized presentation components.
  3. Delegate user interactions to handler functions, not inline logic.
- Move conditional sub-layouts into small, focused subcomponents rather than inline ternary soup.
- Use lookup maps/records instead of long `switch` statements for log level badge colors or status styling.

---

## 3. Early Return Pattern

Flatten control flow by returning early on failure or edge cases. Avoid deep indentation.

```cpp
// ❌ Nested if blocks increase cyclomatic complexity rapidly
std::optional<LogEntry> processLine(const std::string& line) {
  if (!line.empty()) {
    if (isValidFormat(line)) {
      if (hasRequiredFields(line)) {
        return parseLine(line);
      }
    }
  }
  return std::nullopt;
}

// ✅ Guard clauses keep cyclomatic complexity minimal
std::optional<LogEntry> processLine(const std::string& line) {
  if (line.empty() || !isValidFormat(line) || !hasRequiredFields(line)) {
    return std::nullopt;
  }
  return parseLine(line);
}
```

---

## 4. Memory Safety & Resource Management (C++ Specific)

- **RAII everywhere:** Use `std::unique_ptr`, `std::shared_ptr`, `std::string`, `std::vector` instead of raw `new`/`delete` or raw `char*` buffers.
- **No raw pointer arithmetic** in hot paths unless wrapped in a safe span/iterator abstraction.
- **GIL management:** When calling C++ from Python via `pybind11`, explicitly manage the GIL — release it during long-running C++ index queries, re-acquire before returning to Python.
- **No dangling references:** C++ binding functions must not return references to stack-allocated or temporary objects that Python will dereference after the call returns.

---

## 5. Type Safety & Validation (Python Specific)

- **Pydantic models** for all request/response schemas — no `dict[str, Any]` passthrough without validation.
- **Strict enum types** for `log_level` (`INFO`, `WARN`, `ERROR`, `DEBUG`) — no raw string comparisons.
- **Datetime parsing:** All timestamps parsed as timezone-aware `datetime` objects; naive datetimes are rejected at the API boundary.
- **No broad `except:`** — catch specific exception types from the C++ binding layer and map them to appropriate HTTP errors.

---

## 6. Checklist Before Marking a Task Done

In addition to PRD Section 18:

- [ ] Cyclomatic complexity $\le 6$ for every modified function (C++, Python, TypeScript)
- [ ] No complexity bypasses (`#pragma`, `eslint-disable`, `@ts-ignore`, `# noqa`, etc.)
- [ ] Early return pattern applied; no deep indentation
- [ ] C++ code uses RAII; no raw `new`/`delete` or raw `char*` buffers
- [ ] Python route handlers delegate validation/binding/serialization to separate functions
- [ ] TypeScript components are declarative; logic extracted to handlers/hooks
- [ ] `LogEntry` immutability invariant verified — no code mutates indexed records
- [ ] Time-ordering invariant verified — index queries return `timestamp`-sorted results
- [ ] C++ ↔ Python binding memory safety verified (GIL, no dangling references)
- [ ] PR evidence provided (execution logs, benchmark data, or test output)

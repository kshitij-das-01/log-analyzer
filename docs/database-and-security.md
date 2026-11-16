# Security, Authorization & Mutation Rules — Log Analyzer

> **Applies to:** All C++ core operations, Python API route handlers, Kafka topic interactions, and data access layers.
> **Related Documents:**
> - [`PROJECT_ARCHITECTURE_SAD.md`](./PROJECT_ARCHITECTURE_SAD.md) — System architecture
> - [`code-complexity-and-quality.md`](./code-complexity-and-quality.md) — Code quality standards
> - [`pull-request-guidelines.md`](./pull-request-guidelines.md) — PR writing guidelines

---

## 1. Golden Rule for Data Mutations

**Every mutation in the C++ In-Memory Index MUST validate input before writing.** No query or write operation may trust client-provided parameters, timestamps, or log content implicitly.

The C++ core is the trust boundary. Python acts as the orchestration layer that validates and translates before forwarding to C++.

---

## 2. Mandatory Verification Order (Python API → C++ Core)

Every API request that results in a write or read against the C++ In-Memory Index must strictly follow this sequential execution pipeline:

```
[Incoming HTTP Request]
                │
                ▼
    1. Parameter Validation
       - Validate start_time, end_time, log_level, service_name, query
       - Reject malformed or out-of-range parameters with 400 / 422
                │
                ▼
    2. C++ Binding Invocation
       - Translate validated params into C++ query object
       - Call pybind11 / gRPC binding method
                │
                ▼
    3. C++ Index Query / Write
       - Execute against In-Memory Index (read-only for queries)
       - For writes: validate LogEntry struct invariants before insert
                │
                ▼
    4. Result Serialization
       - Serialize C++ result set into JSON
       - Never leak internal C++ memory pointers or raw struct layouts
                │
                ▼
    5. HTTP Response
       - Return formatted JSON to frontend
       - Map C++ error codes to appropriate HTTP status codes
```

---

## 3. Server-Side Secret & Credential Isolation

- **Absolute Prohibition on Client-Side Secrets:**
  - Never expose Kafka connection strings, API keys, or service credentials in frontend code or browser bundles.
  - All third-party providers (Kafka client config, C++ binding initialization) must be initialized strictly inside server-side code (Python or C++).
  - Client-side code receives only public identifiers and API endpoints.

- **Environment Variable Handling:**
  - Kafka bootstrap servers, topic names, and C++ binding paths must be loaded from environment variables or secure config files at startup.
  - Never hardcode connection strings in source code.

---

## 4. Query & Mutation Patterns

### Safe API Route Handler Example (Python)

```python
from fastapi import HTTPException, Query
from pydantic import BaseModel, Field, validator
from datetime import datetime, timezone
from typing import Optional, List
import log_index_binding  # pybind11 or gRPC generated module

class SearchParams(BaseModel):
    start_time: Optional[datetime] = None
    end_time: Optional[datetime] = None
    log_level: Optional[str] = Field(None, pattern=r'^(INFO|WARN|ERROR|DEBUG)$')
    service_name: Optional[str] = None
    query: Optional[str] = None
    limit: int = Field(100, ge=1, le=10000)

    @validator('start_time', 'end_time', pre=True)
    def ensure_timezone_aware(cls, v):
        if v is not None and v.tzinfo is None:
            raise ValueError('Timestamps must be timezone-aware')
        return v

    @validator('end_time')
    def end_after_start(cls, v, values):
        if v is not None and values.get('start_time') is not None:
            if v <= values['start_time']:
                raise ValueError('end_time must be after start_time')
        return v

@router.get("/api/logs/search")
async def search_logs(params: SearchParams = Query()):
    # 1. Parameter validation (Pydantic handles this automatically)
    
    # 2. Translate to C++ query object
    cpp_query = {
        "start_time": params.start_time.isoformat() if params.start_time else None,
        "end_time": params.end_time.isoformat() if params.end_time else None,
        "log_level": params.log_level,
        "service_name": params.service_name,
        "query": params.query,
        "limit": params.limit,
    }
    
    try:
        # 3. Invoke C++ binding
        results = log_index_binding.query_index(cpp_query)
    except log_index_binding.IndexError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except log_index_binding.TimeoutError as e:
        raise HTTPException(status_code=504, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail="Internal index error")
    
    # 4. Serialize results
    return {"logs": results, "count": len(results)}
```

### C++ In-Memory Index Write Validation

```cpp
// src/cpp/index/InMemoryIndex.h
class InMemoryIndex {
public:
    // Write path: validate before insert
    bool write(const LogEntry& entry) {
        if (!validateEntry(entry)) {
            return false;  // Reject invalid entries
        }
        // Time-ordered insertion
        buckets_[entry.log_level].insert(entry.timestamp, entry);
        return true;
    }

private:
    bool validateEntry(const LogEntry& entry) {
        // Immutability invariant: entry must be complete
        if (entry.timestamp.empty() || entry.message.empty()) {
            return false;
        }
        if (entry.log_level != LogLevel::INFO &&
            entry.log_level != LogLevel::WARN &&
            entry.log_level != LogLevel::ERROR &&
            entry.log_level != LogLevel::DEBUG) {
            return false;
        }
        // Time ordering: timestamp must be parseable
        if (!isValidISO8601(entry.timestamp)) {
            return false;
        }
        return true;
    }
};
```

---

## 5. Soft-Delete & Immutability Invariants

- **Immutability:** Once a `LogEntry` is written to the In-Memory Index, it must never be modified. All transformations produce new records.
- **No Soft-Delete in MVP:** Log entries are append-only. Deletion/purge is a future concern (v1+).
- **Time Ordering:** All records within a service/host category must be strictly ordered by `timestamp`. Queries must return `timestamp`-sorted results.

---

## 6. Kafka Topic Security

- **Topic Isolation:** The `raw_logs_topic` is internal infrastructure. Frontend and external clients must never publish directly to Kafka.
- **Producer/Consumer Authentication:** C++ Ingestor and Stream Processor must authenticate to Kafka using SASL/SCRAM or mTLS in production.
- **Message Validation:** The Stream Processor must validate incoming Kafka messages against the `LogEntry` schema before indexing. Malformed messages are logged and skipped, never crash the processor.

---

## 7. Checklist Before Marking a Security-Related Task Done

- [ ] Input validation at Python API boundary before C++ binding invocation
- [ ] C++ `validateEntry()` called before every Index write
- [ ] No secrets leaked to frontend or client bundles
- [ ] Kafka producer/consumer authentication configured (production)
- [ ] Immutability invariant verified — no code mutates indexed `LogEntry`
- [ ] Time-ordering invariant verified — index queries return `timestamp`-sorted results
- [ ] C++ error codes properly mapped to HTTP status codes in Python
- [ ] No raw `char*` buffers or dangling references across binding boundary

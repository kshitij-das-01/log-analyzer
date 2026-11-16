# Development Workflow — Log Analyzer

> **Applies to:** All development work across the polyglot stack (C++ core, Python API, React/TS frontend).
> **Related Documents:**
> - [`GITFLOW.md`](./GITFLOW.md) — Branching & version control
> - [`code-complexity-and-quality.md`](./code-complexity-and-quality.md) — Code quality standards
> - [`pull-request-guidelines.md`](./pull-request-guidelines.md) — PR writing guidelines
> - [`subagent-team.md`](./subagent-team.md) — Specialized AI subagent team

---

## 1. The Plan → Implement → Commit Cycle

We follow a disciplined, iterative development cycle. No code is written without a clear plan, no commit is made without your verification, and no push is executed without your approval.

```
┌─────────┐    ┌──────────────┐    ┌──────────────────┐    ┌─────────────┐    ┌──────────┐    ┌──────────┐
│  PLAN   │───▶│  IMPLEMENT   │───▶│   USER VERIFY    │───▶│   COMMIT    │───▶│  REVIEW  │───▶│  PUSH    │
│ (Discuss│    │ (Code per    │    │ (User confirms   │    │ (Atomic,    │    │ (Quality │    │ (User    │
│  specs) │    │  standards)  │    │  implementation) │    │  descriptive)│    │  gate)   │    │ approve) │
└─────────┘    └──────────────┘    └──────────────────┘    └─────────────┘    └──────────┘    └──────────┘
       ▲                                                                                                  │
       │                                                                                                  ▼
       └──────────────────────────────────────────── NEXT FEATURE ───────────────────────────────────────┘
```

---

## 2. Phase 1: PLAN (Discussion Mode)

### 2.1 When to Plan
- Starting a new feature or module
- Modifying existing architecture or data structures
- Changing API contracts or C++ binding interfaces
- Any work that affects multiple components (C++ ↔ Python ↔ React)

### 2.2 What to Discuss
- **Scope:** What exactly is being built? What is out of scope?
- **Design:** Which components are affected? What are the interface changes?
- **Data:** Does the `LogEntry` schema change? Does the Kafka topic schema change?
- **Risk:** One-way vs two-way door? Blast radius?
- **Subagents:** Which specialized subagents should be invoked? (`log-architect`, `stream-processor`, `log-engineer`, `adversarial-reviewer`)

### 2.3 Plan Output
Before moving to implementation, the following must be clear:
- [ ] Feature scope and acceptance criteria defined
- [ ] Affected files/modules identified
- [ ] Interface contracts defined (if crossing component boundaries)
- [ ] Test strategy identified (unit tests, integration tests, benchmarks)
- [ ] Commit plan: how the work will be split into atomic commits

---

## 3. Phase 2: IMPLEMENT (Coding Mode)

### 3.1 Implementation Rules
- **One feature per branch:** Create a `feature/<short-desc>` branch off `develop`
- **Small, focused changes:** Each commit should represent a single logical unit
- **Standards first:** Follow `code-complexity-and-quality.md` (complexity ≤ 6, RAII, early returns, immutability)
- **No stray work:** Do not implement unrelated fixes in the same branch/commit

### 3.2 Per-Layer Implementation Checklist

| Layer | Checklist |
| :--- | :--- |
| **C++ Core** | Complexity ≤ 6, RAII, no raw pointers, validateEntry() before index write, time-ordered insertion, benchmark data for perf changes |
| **Python API** | Pydantic models, input validation, specific exception handling, HTTP status mapping, no `dict[str, Any]` passthrough |
| **React/TS** | Declarative components, lookup maps instead of switch/if-else, no `eslint-disable`/`@ts-ignore` |
| **Cross-layer** | C++ ↔ Python binding memory safety (GIL, no dangling refs), error code mapping |

### 3.3 When to Stop Implementing
Stop when:
- The feature is complete per the plan's acceptance criteria
- All code quality checks pass (complexity ≤ 6, no bypasses)
- Tests/benchmarks pass
- You're ready for user verification

---

## 4. Phase 4: USER VERIFICATION (User Approval Required)

### 4.1 User Verification Rule

**No commit will be made until you (the user) verify the implementation.** Just like pushes, every commit must be confirmed by you after reviewing the code.

### 4.2 Verification Workflow

| Step | Action | Who |
| :--- | :--- | :--- |
| 1 | Complete implementation on `feature/<desc>` branch | Agent |
| 2 | Present implementation summary, file list, and key changes | Agent |
| 3 | User reviews code, tests, and verifies correctness | **You** |
| 4 | User explicitly approves the implementation for commit | **You** |
| 5 | Agent stages and commits the changes | Agent |

### 4.3 What You Should Verify

- [ ] Code matches the agreed plan and design
- [ ] Cyclomatic complexity ≤ 6 per function
- [ ] No complexity bypasses (`#pragma`, `eslint-disable`, `@ts-ignore`, `# noqa`)
- [ ] C++: RAII, no raw `new`/`delete`, no dangling references
- [ ] Python: Pydantic models, specific exception handling
- [ ] TypeScript: Declarative components, no `@ts-ignore`
- [ ] Tests/benchmarks written and passing
- [ ] `LogEntry` immutability and time-ordering invariants verified
- [ ] C++ ↔ Python binding memory safety verified

### 4.4 What Does NOT Require Approval

- Local exploration, prototyping, or spike code (not yet committed)
- Documentation updates that don't affect code behavior

---

## 5. Phase 5: COMMIT (Version Control)

### 5.1 Commit Standards
Follow the conventions in [`GITFLOW.md`](./GITFLOW.md):

```
<type>(<scope>): <subject>

<body>
<footer>
```

**Types:** `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`
**Scopes:** `cpp`, `python`, `react`, `kafka`, `ci`, `bindings`, `index`, `parser`

### 5.2 Atomic Commit Principles
- **One logical change per commit:** Don't bundle unrelated changes
- **Descriptive subject:** Clear, imperative mood ("Add LogEntry struct", not "Added LogEntry")
- **Body explains why:** Not just what, but why this change was made
- **Footer references:** Issue numbers, related PRs, breaking change notices

### 5.3 Example Commit
```
feat(cpp): implement LogEntry struct and In-Memory Index

Add canonical LogEntry struct with timestamp, log_level, source_file,
service_name, hostname, and message fields. Implement layered in-memory
index with time-ordered insertion and query support.

Includes validation before write to enforce immutability and
time-ordering invariants.

Closes #12
```

---

## 6. Phase 6: REVIEW (Quality Gate)

### 6.1 Self-Review Before Commit
- [ ] Cyclomatic complexity ≤ 6 for every modified function
- [ ] No complexity bypasses (`#pragma`, `eslint-disable`, `@ts-ignore`, `# noqa`)
- [ ] Early return pattern applied
- [ ] C++: RAII, no raw `new`/`delete`, no dangling references across binding
- [ ] Python: Pydantic models, specific exception handling
- [ ] TypeScript: Declarative components, no `@ts-ignore`
- [ ] Tests/benchmarks pass
- [ ] `LogEntry` immutability and time-ordering invariants verified

### 6.2 Adversarial Reviewer (Optional but Recommended)
For high-risk changes (binding interface, Kafka schema, index structure), invoke the `adversarial-reviewer` subagent before committing.

---

## 7. Phase 7: PUSH (User Approval Required)

### 7.1 Push Approval Rule

**No `git push` will be executed without explicit user approval.** Just like commits, every push must be confirmed by you before it is sent to the remote repository.

### 7.2 Push Workflow

| Step | Action | Who |
| :--- | :--- | :--- |
| 1 | Complete commit on `feature/<desc>` branch | Agent |
| 2 | Present commit summary and diff for review | Agent |
| 3 | User reviews and approves the commit | **You** |
| 4 | Agent presents push plan (branch, remote, target) | Agent |
| 5 | User explicitly approves `git push` | **You** |
| 6 | Agent executes `git push` | Agent |

### 7.3 What Requires Approval

- Every `git push` to any remote (origin, upstream, etc.)
- Every branch push (`feature/*`, `develop`, `main`, `release/*`, `hotfix/*`)
- Every tag push

### 7.4 What Does NOT Require Approval

- Local `git commit` (still requires commit message approval per standards)
- Local `git merge` on feature branches
- Local `git rebase` or `git cherry-pick`

---

## 8. Branch Lifecycle (Gitflow)

| Stage | Branch | Action | Approval Required |
| :--- | :--- | :--- | :--- |
| **Start work** | `develop` → `feature/<desc>` | `git checkout develop && git pull && git checkout -b feature/...` | No (local) |
| **During work** | `feature/<desc>` | Implement and present for user verification | **Yes — user verification** |
| **Commit** | `feature/<desc>` | Commit after user verification | **Yes — user verification** |
| **Push feature** | `feature/<desc>` → remote | `git push origin feature/<desc>` | **Yes — user approval** |
| **Release prep** | `develop` → `release/<ver>` | Stabilize, version bump, only bug fixes | No (local) |
| **Release** | `release/<ver>` → `main` + tag | `git tag -a v1.0.0` | No (local) |
| **Push release** | `release/<ver>` → remote | `git push origin release/<ver>` | **Yes — user approval** |
| **Push main** | `main` → remote | `git push origin main --tags` | **Yes — user approval** |
| **Hotfix** | `main` → `hotfix/<ver>` | Urgent production fix | No (local) |
| **Push hotfix** | `hotfix/<ver>` → remote | `git push origin hotfix/<ver>` | **Yes — user approval** |




---

## 9. Definition of Done (DoD)

A task is **done** only when ALL of the following are true:

- [ ] Plan phase completed and documented
- [ ] Implementation complete per acceptance criteria
- [ ] User verified the implementation (Phase 4)
- [ ] Code quality standards met (complexity ≤ 6, no bypasses)
- [ ] Tests/benchmarks written and passing
- [ ] Committed to `develop` via `feature/<desc>` branch
- [ ] Commit message follows conventions
- [ ] PR evidence provided (if applicable): build logs, test output, benchmark data

---

## 10. Quick Reference: When to Do What

| Question | Action | Document |
| :--- | :--- | :--- |
| Starting a new feature? | PLAN → Create `feature/<desc>` branch | `WORKFLOW.md`, `GITFLOW.md` |
| Need architectural decision? | PLAN → Invoke `log-architect` | `subagent-team.md` |
| Writing C++ parsing logic? | IMPLEMENT → Check complexity ≤ 6, RAII | `code-complexity-and-quality.md` |
| Writing Python API endpoint? | IMPLEMENT → Pydantic models, input validation | `database-and-security.md` |
| Implementation complete? | USER VERIFY → Present code for your review | `WORKFLOW.md` Phase 4 |
| Ready to commit? | COMMIT → After user verification | `GITFLOW.md`, `pull-request-guidelines.md` |
| Ready to push to remote? | PUSH → Present plan, wait for user approval | `WORKFLOW.md` Phase 7 |
| Unsure about code quality? | REVIEW → Invoke `adversarial-reviewer` | `subagent-team.md` |
| Merging to production? | `release/<ver>` → `main` + tag | `GITFLOW.md` |

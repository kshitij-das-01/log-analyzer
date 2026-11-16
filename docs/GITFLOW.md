# Atlassian Gitflow Workflow Guide — Log Analyzer

This repository follows **Atlassian's Gitflow Workflow**, an established branching model designed for scheduled and continuous releases across a polyglot C++ / Python / TypeScript codebase.

---

## 🌳 Branch Architecture

```
main       ●───────────────●─────────────────────● [v1.0.0] ─────● [v1.0.1] (Hotfix)
            \             /                     /                /
release      \           /        ●────────────●                /
              \         /        /            /                /
develop        ●───────●────────●────────────●────────────────●
                \     /
 feature          ●───●
```

| Branch | Base Branch | Merges Into | Purpose | Naming Convention |
| :--- | :--- | :--- | :--- | :--- |
| **`main`** | *N/A* | *N/A* | Official production history and releases | `main` |
| **`develop`** | `main` | *N/A* | Integration branch for completed features | `develop` |
| **`feature/*`** | `develop` | `develop` | Development of new features or experiments | `feature/<short-desc>` |
| **`release/*`** | `develop` | `main` & `develop` | Release stabilization, testing, version bumps | `release/<version>` |
| **`hotfix/*`** | `main` | `main` & `develop` | Urgent production bug fixes | `hotfix/<version>` |
| **`bugfix/*`** | `develop` | `develop` | General bug fixes not requiring a hotfix | `bugfix/<issue-id>` |

---

## 🚀 Workflows & Commands

### 1. Feature Branches

Features isolate work in progress and are branched off `develop`. Each feature should target a single module boundary (C++ core, Python API, or React frontend) to minimize merge conflicts.

#### Start a Feature
```bash
git checkout develop
git pull origin develop
git checkout -b feature/cpp-logentry-struct
git checkout -b feature/python-search-endpoint
git checkout -b feature/react-log-table
```

#### Finish a Feature
```bash
git checkout develop
git pull origin develop
git merge --no-ff feature/cpp-logentry-struct
git branch -d feature/cpp-logentry-struct
git push origin develop
```
*(In team environments, open a Pull Request targeting `develop` instead of merging locally).*

---

### 2. Release Branches

When `develop` has accumulated enough features for a release, spin up a `release/*` branch. Only bug fixes, documentation, and release metadata adjustments should be committed here.

#### Start a Release
```bash
git checkout develop
git pull origin develop
git checkout -b release/1.0.0
```

#### Finish a Release
1. Merge into `main` and tag the release:
   ```bash
   git checkout main
   git pull origin main
   git merge --no-ff release/1.0.0
   git tag -a v1.0.0 -m "Release v1.0.0"
   git push origin main --tags
   ```

2. Merge back into `develop` so bug fixes made in the release are preserved:
   ```bash
   git checkout develop
   git pull origin develop
   git merge --no-ff release/1.0.0
   git push origin develop
   ```

3. Clean up the release branch:
   ```bash
   git branch -d release/1.0.0
   ```

---

### 3. Hotfix Branches

Hotfixes address critical bugs directly on production (`main`) — for example, a memory leak in the C++ In-Memory Index or a broken API binding.

#### Start a Hotfix
```bash
git checkout main
git pull origin main
git checkout -b hotfix/1.0.1
```

#### Finish a Hotfix
1. Merge into `main` and tag:
   ```bash
   git checkout main
   git pull origin main
   git merge --no-ff hotfix/1.0.1
   git tag -a v1.0.1 -m "Hotfix v1.0.1"
   git push origin main --tags
   ```

2. Merge back into `develop`:
   ```bash
   git checkout develop
   git pull origin develop
   git merge --no-ff hotfix/1.0.1
   git push origin develop
   ```

3. Delete hotfix branch:
   ```bash
   git branch -d hotfix/1.0.1
   ```

---

## ⚙️ Configuration

This repository is pre-configured with standard Gitflow metadata:

```ini
[gitflow "branch"]
    master = main
    develop = develop
[gitflow "prefix"]
    feature = feature/
    bugfix = bugfix/
    release = release/
    hotfix = hotfix/
    support = support/
    versiontag = v
```

---

## 📝 Commit Message Conventions

All commits must follow this format:

```
<type>(<scope>): <subject>

<body>
<footer>
```

| Type | Scope Examples | Description |
| :--- | :--- | :--- |
| `feat` | `cpp`, `python`, `react`, `kafka`, `ci` | New feature |
| `fix` | `cpp`, `python`, `react` | Bug fix |
| `docs` | `api`, `architecture`, `subagent` | Documentation only |
| `style` | `cpp`, `python`, `react` | Formatting, missing semicolons, etc. |
| `refactor` | `cpp`, `python` | Code change that neither fixes a bug nor adds a feature |
| `perf` | `cpp`, `index` | Performance improvement |
| `test` | `cpp`, `python`, `react` | Adding or correcting tests |
| `chore` | `build`, `ci`, `deps` | Build process, CI, dependencies |

**Example:**
```
feat(cpp): implement LogEntry struct and In-Memory Index

Add canonical LogEntry struct with timestamp, log_level, source_file,
service_name, hostname, and message fields. Implement layered in-memory
index with time-ordered insertion and query support.

Closes #12
```

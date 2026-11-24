# Subagent Usage Documentation

## Overview
This document describes how subagents should be used in the Log Analyzer project. It provides guidelines, invocation patterns, and best practices for effective collaboration.

## Available Subagents

### log-architect
**Role**: System & C++ Core Architect  
**Best For**: 
- C++ core implementation (In-Memory Index, bindings, Kafka schema)
- LogEntry struct and data structure design
- System architecture and component boundaries
- C++ ↔ Python query path design

**Example Invocation**:
```typescript
invoke_subagent({
  TypeName: 'log-architect',
  Role: 'System & C++ Core Architect',
  Prompt: 'Design the C++ LogEntry struct and layered In-Memory Index structure.'
})
```

### stream-processor
**Role**: C++ Ingestion & Performance  
**Best For**:
- Log parsing and normalization
- Kafka publishing/consuming
- Performance optimization
- In-Memory Index implementation

**Example Invocation**:
```typescript
invoke_subagent({
  TypeName: 'stream-processor',
  Role: 'C++ Ingestion Engineer',
  Prompt: 'Implement log ingestion pipeline with Kafka topic partitioning'
})
```

### log-engineer
**Role**: Python FastAPI & Integration  
**Best For**:
- FastAPI REST endpoints
- C++ binding orchestration
- Response serialization
- Pydantic model validation

**Example Invocation**:
```typescript
invoke_subagent({
  TypeName: 'log-engineer',
  Role: 'API & Integration Engineer',
  Prompt: 'Design the Python FastAPI endpoint for log search with query parameters'
})
```

### adversarial-reviewer
**Role**: Security & Performance Audit  
**Best For**:
- Security audit of code
- Performance invariant checks
- PR review (read-only)
- Code quality validation

**Example Invocation**:
```typescript
invoke_subagent({
  TypeName: 'adversarial-reviewer',
  Role: 'Adversarial Reviewer',
  Prompt: 'Review this C++ function for potential memory leaks or performance issues'
})
```

---

## Invocation Patterns

### Pattern 1: Design/Architecture Questions
```typescript
invoke_subagent({
  TypeName: 'log-architect',
  Role: 'System & C++ Core Architect',
  Prompt: 'Design the C++ InMemoryIndex Query() method signature for Python integration'
})
```

### Pattern 2: Code Review / Quality
```typescript
invoke_subagent({
  TypeName: 'adversarial-reviewer',
  Role: 'Adversarial Reviewer',
  Prompt: 'Check this Python FastAPI route handler for security vulnerabilities and code quality issues'
})
```

### Pattern 3: Test Design
```typescript
invoke_subagent({
  TypeName: 'stream-processor',
  Role: 'C++ Ingestion Engineer',
  Prompt: 'Design GoogleTest patterns for InMemoryIndex concurrency testing'
})
```

### Pattern 4: API Design
```typescript
invoke_subagent({
  TypeName: 'log-engineer',
  Role: 'API & Integration Engineer',
  Prompt: 'Design Pydantic models for log search with text_query and filter parameters'
})
```

---

## Subagent Workflow

### Step 1: Define the Task
- Clearly specify what you want the subagent to accomplish
- Include project context: Log Analyzer, C++ core + Python FastAPI integration
- Specify the desired output format

### Step 2: Provide Context
- Reference relevant files or code sections
- Mention the two-phase commit strategy (Commit A/B)
- Note any known issues or constraints

### Step 3: Review Results
- Subagents return in background messages
- Review their findings before deciding next steps
- May invoke additional subagents for follow-up questions

### Step 4: Decide Action
- Implement changes based on subagent findings
- Request follow-up investigations
- Proceed with commit workflow

---

## Guidelines

### Do:
- Provide specific, focused prompts
- Reference file paths when relevant
- Mention the project's two-phase commit strategy
- Request specific output formats (design docs, code reviews, test patterns)

### Don't:
- Give vague or overly broad prompts
- Ask subagents to implement code changes directly (they should design/review, not write production code without approval)
- Skip providing project context
- Mix multiple unrelated tasks in one invocation

---

## Communication Channels

### Within Conversation:
- Direct subagent invocations using `invoke_subagent({...})`
- Review background results when idle
- Follow up with additional invocations as needed

### Between Sessions:
- Subagents run in fresh child sessions
- Each starts with fresh context
- Pass sessionID to continue specific child conversations

### Handoff Protocol:
- When transferring between subagents, include relevant context
- Reference previous findings in new invocations
- Maintain consistency with project patterns and conventions

---

## Quick Reference

| Need | Use Subagent | Invocation Name |
|------|-------------|----------------|
| C++ architecture | log-architect | `log-architect` |
| C++ performance | stream-processor | `stream-processor` |
| Python API | log-engineer | `log-engineer` |
| Code review | adversarial-reviewer | `adversarial-reviewer` |
| General research | general | `general` |

---

## Example Session Flow

```
1. User: "Help me design the pybind11 module interface"
2. AI: invoke_subagent({TypeName: 'log-architect', ...})
3. Subagent: Returns design recommendations
4. AI: Reviews findings, may ask follow-up
5. AI: Provides recommendations to user
6. User: "Proceed with implementation"
7. AI: Implements design, creates files
8. AI: Commits changes per workflow
```

---

## File Locations

- **Project docs**: `docs/` directory
- **Subagent definitions**: Referenced from `docs/subagent-team.md`
- **This document**: `docs/subagent-usage.md`
- **Plan documents**: `docs/plan-implementation/python-api-integration/`

---

*Last updated: Tue Nov 24 2026*
*Project: Log Analyzer - Polyglot log analysis system*
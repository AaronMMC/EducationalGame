# SynType — Parallelism Implementation Guide

**Based on:** *"From Imperative Code to Concurrency: A Framework to Guide Novice Programmers in Parallelization"* — Team 3, CS 325

---

## Overview

This document describes how the **7-Phase Parallelization Framework** developed by Team 3 was applied to the SynType arcade typing game. SynType was chosen as one of the framework's real-world C++ test codebases. The sections below walk through all seven phases as they apply specifically to this project.

---

## Phase 1 — Problem Domain Analysis: Finding the Bottlenecks

Before touching any code, we ask: **"Where does the program wait, and what costs the most time?"**

In SynType, three areas were identified as sequential bottlenecks running purely out of habit:

| Bottleneck | Location | Cost |
|---|---|---|
| CPU opponent simulation | `NetworkSim::tick()` — called every frame on the main thread | Blocks rendering if logic grows complex |
| Glitch character cache refresh | `GameScreen::update()` — regenerates random chars every 0.15s | Unnecessary synchronous work mid-frame |
| LAN matchmaking scan | Would require polling UDP sockets | Cannot block the UI thread at all |

All three run sequentially even though none of them depend on the render pipeline's output. They are **pure candidates for parallelization**.

---

## Phase 2 — Sequential Baseline

The original execution flow (single-threaded) per frame:

```
main loop
  └─ handleEvent()
  └─ update(dt)          ← NetworkSim::tick() lives here (CPU sim)
       └─ refreshGlitchCache()   ← random array rebuild
  └─ draw()              ← all SFML rendering
  └─ window.display()
```

**Data dependencies identified (must be preserved):**
- `gs.targetText.size()` must be stable while `refreshGlitchCache` reads it
- `gs.obfuscatedIndices` is written by `NetworkSim::tick()` and read by `draw()` — a classic **read-after-write** dependency requiring synchronization
- `gs.playerHP` / `gs.oppHP` are modified by `tick()` and read by `drawHUD()` — same issue

These shared variables are the exact race conditions Phase 3 asks us to identify.

---

## Phase 3 — Concept Mapping (4-Step Mental Model)

Following the framework's four questions:

1. **"Can these tasks run independently?"**
   - Glitch cache refresh → YES. It only needs `targetText.size()` (read-only) and writes to a local cache array.
   - `NetworkSim::tick()` → YES, if we protect the shared HP/indices values with a mutex.
   - LAN matchmaking → YES. Entirely separate concern from rendering.

2. **"Can I chop this large dataset in half?"** — *Data Parallelism*
   - The glitch character array (`glitchCharCache`) can be filled in parallel: split the array in half, assign each half to a thread.

3. **"Are two tasks changing the same variable?"** — *Shared Resource Identification*
   - `gs.obfuscatedIndices`, `gs.playerHP`, `gs.oppHP` are written by the sim thread and read by the render thread → **mutex required**
   - `glitchCharCache` is written by worker thread, read by render thread → **mutex required**

4. **"Which pattern fits?"**
   - `NetworkSim` sim thread → **Pipeline pattern** (sim produces state, render consumes it)
   - Glitch cache → **Data parallelism** (independent array segments)
   - Matchmaking → **Task parallelism** (completely independent background task)

---

## Phase 4 — Threading Grammar (BNF + RAII)

The framework maps parallel concepts to formal grammar. For C++17 (this project's standard):

```
<thread-launch>  ::= std::thread( <callable>, <args...> )
<mutex-guard>    ::= std::lock_guard<std::mutex> <name>( <mutex> )
                   | std::unique_lock<std::mutex> <name>( <mutex> )
<atomic-var>     ::= std::atomic< <type> > <name> { <init> }
<condvar-wait>   ::= <condvar>.wait( <unique_lock>, <predicate> )
```

Using `std::lock_guard` (RAII) means the lock **auto-releases** when the guard goes out of scope, even on exception — the low-risk zero-cost abstraction described in the framework's Phase 5 C++ section.

---

## Phase 5 — Tool Selection

| Need | Tool | Risk Level |
|---|---|---|
| Background sim thread | `std::thread` (C++17 stdlib) | Low |
| Shared state protection | `std::mutex` + `std::lock_guard` | Low — RAII auto-release |
| Sim → render signalling | `std::atomic<bool>` | Zero — lock-free |
| Matchmaking thread | `std::thread` + `std::atomic<bool> running` | Low |
| Data-parallel cache fill | `std::thread` × 2 + join | Low |

All tools are from the C++17 standard library — **no external dependencies required**.

---

## Phase 6 — Application Development

See the annotated source files for exact integration points:

- [`src/NetworkSim.h`](src/NetworkSim.h) / [`src/NetworkSim.cpp`](src/NetworkSim.cpp) — sim thread + mutex-protected state swap
- [`src/Screens/GameScreen.cpp`](src/Screens/GameScreen.cpp) — parallel glitch cache fill, safe state read under lock
- [`src/GameState.h`](src/GameState.h) — atomic flags, mutex declarations
- [`src/main.cpp`](src/main.cpp) — thread lifecycle (launch / join on exit)

The key rule from Phase 6: **integrate synchronization at the exact points identified in Phase 3**, not globally.

---

## Phase 7 — Evaluation and Reflection

**Correctness verification:**
- Output must match the sequential baseline: HP values, obfuscated indices, and glitch chars must render identically.
- Run with ThreadSanitizer (`-fsanitize=thread`) to detect races:

```bash
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=thread -g"
make -j$(nproc)
./SynType
```

**Performance expectation:**
- The main thread no longer stalls on `NetworkSim::tick()` or cache rebuild — frame time becomes more consistent.
- Matchmaking never blocks the UI — the player sees a live "Searching…" animation with zero jank.

**When parallelism does NOT help (overhead > speedup):**
- If `NetworkSim::tick()` is trivially fast (it currently is), the mutex overhead may exceed the savings. In that case, keep it sequential and only parallelize matchmaking + cache fill.
- Always measure with `std::chrono::high_resolution_clock` before and after to confirm gains.

-

---

## Quick Build Reference

```bash
# Standard build (C++17 required)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Build with ThreadSanitizer (Phase 7 race detection)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=thread -g"
make -j$(nproc)
./SynType   # TSan will print race reports to stderr
```

---

## Conceptual Summary

```
BEFORE (sequential):
  main thread: [event]──[update+sim]──[draw]──[display] ──▶ repeat

AFTER (parallel):
  main thread:  [event]──[read sim state]──[draw]──[display] ──▶ repeat
  sim thread:            [tick()]──[lock]──[swap state]──[unlock] ──▶ repeat
  cache thread:          [fill half A]
  cache thread 2:                  [fill half B]
  matchmaking thread: [broadcast/listen UDP] ──▶ signals main on match found
```

This maps directly to the **Pipeline** and **Data Parallelism** patterns identified in Phase 3 of the framework.
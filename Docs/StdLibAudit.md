# Standard Library Usage Audit

This document lists the usage of the Rust Standard Library (`std`) in the codebase, categorized by the project's strict rules.

## 1. Errors (Forbidden Features)
These features are strictly forbidden and must be replaced by `Cap` layer implementations.

### `std::collections` (Containers)
**Rule:** Use `Cap.Containers` instead of `std::collections`.
**Status:** **VIOLATION DETECTED**

- **`std::collections::HashMap`**
    - `Sim/Scene/API/lib.rs`: Used for Scene/Entity management.
    - `Foundation/Sys/Scripting/API/lib.rs`: Used for internal maps.
    - `Foundation/Sys/RenderGraph/API/types.rs`: Used for resource tracking.
    - `Engine/App/src/main.rs`: Used in application logic.
    - `Foundation/Sys/IR/API/lib.rs`: Used in IR structures.

### `std::vec::Vec`
**Rule:** Use `Cap.Containers::Vector` where possible.
**Status:** Implicitly used. No direct explicit imports found in grep snippet, but likely used via prelude.
- **Action:** Review all `Vec` usages and migrate to `Cap.Containers::Vector`.

### `std::string::String`
**Rule:** Use `Cap.Containers::CapString` or minimal string handling.
**Status:** Implicitly used via prelude.

## 2. Warnings (Discouraged Features)
These features are allowed but discouraged or should be wrapped by `Prm` layer.

### `std::sync` (Synchronization)
**Rule:** Use `Prm.Sync` or `Prm.Threading`.
**Status:** **DETECTED**
- `Engine/App/src/main.rs`: Uses `Arc`, `Mutex`, `AtomicBool`.
- `Foundation/Sys/Scripting/API/lib.rs`: Uses `Arc`.
- `Foundation/Cap/Memory/Impl/Generic/Ops.rs`: Uses `AtomicPtr`, `AtomicUsize` (Low-level impl, likely acceptable).

### `std::time`
**Rule:** Use `Prm.Time` or `Engine.get_gametime()`.
**Status:** **DETECTED**
- `test_matmul.rs`: `Instant::now()` (Test code, acceptable).
- `Foundation/Cap/Containers/Sample/Bench/main.rs`: `Instant` (Benchmark, acceptable).

### `std::fs` (File System)
**Rule:** Use `Prm.File` and `Sys.VFS`.
**Status:** Mostly avoided in core code.
- `Foundation/Sys/Scripting/README.md`: Mentions redirecting `std::fs` to `Sys.VFS`.

## 3. Allowed (Low-Level/Interop)
These features are necessary for implementing the `Prm` and `Cap` layers.

- **`std::ffi`**: `c_void`, `CString` (Used in `Prm.WSI`, `Sys.RHI`).
- **`std::ptr`**: `null`, `null_mut`, `copy_nonoverlapping` (Used in `Cap.Memory`, `Cap.Containers`).
- **`std::mem`**: `transmute`, `size_of` (Used in `Prm.WSI`, `Sys.RHI`).
- **`std::alloc`**: Used in `Cap.Memory` backend (Acceptable).
- **`std::slice`**: `from_raw_parts` (Used in `Prm.WSI`).

## 4. Action Plan
1. **Enforce Lints**: `clippy.toml` has been configured to warn/deny usage of `std::collections` and `std::vec`.
2. **Migrate Containers**:
   - Replace `std::collections::HashMap` with `cap_containers::HashMap` in `Sim`, `Sys.Scripting`, `Sys.RenderGraph`.
   - Replace `std::vec::Vec` with `cap_containers::Vector` in `Sim.Schema`.
3. **Wrap Primitives**:
   - Ensure `Engine.App` uses `Prm.Sync` instead of raw `std::sync`.


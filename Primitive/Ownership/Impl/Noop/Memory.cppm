module;
module Prm.Ownership;
import :Memory;

using namespace Prm;

Expect<void*> VirtualMemory::Reserve(USize) noexcept { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Status VirtualMemory::Commit(void*, USize, PageProtection) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Status VirtualMemory::Decommit(void*, USize) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Status VirtualMemory::Release(void*) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
USize VirtualMemory::PageSize() noexcept { return 4096; }
USize VirtualMemory::AllocationGranularity() noexcept { return 65536; }
Expect<USize> VirtualMemory::LargePageSize() noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Status VirtualMemory::Protect(void*, USize, PageProtection) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Expect<void*> VirtualMemory::ReserveEx(USize, UInt32, bool) noexcept { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

Expect<HeapHandle> Heap::Create() noexcept { return Expect<HeapHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Status Heap::Destroy(HeapHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
HeapHandle Heap::GetProcessDefault() noexcept { return HeapHandle{nullptr}; }
Expect<void*> Heap::AllocRaw(HeapHandle, USize) noexcept { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Status Heap::FreeRaw(HeapHandle, void*) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Expect<void*> Heap::Alloc(HeapHandle, USize, USize) noexcept { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Status Heap::Free(HeapHandle, void*) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
USize Heap::MaximumAlignment() noexcept { return 64; }

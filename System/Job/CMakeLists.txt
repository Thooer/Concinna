API(Sys ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Lang;Memory;Concurrency;Threading;Sync;Ownership;ConcurrentContainers;Time;SystemMemory)
Impl(Sys.Core ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Lang;Memory;Platform;System;Threading;Sync;Ownership;ConcurrentContainers;Tools.Tracy;Concurrency;Time;SystemMemory)
Build(Sys TYPE STATIC BACKEND Sys.Core LINK Lang;Memory;Threading;Sync;Ownership;ConcurrentContainers;Concurrency;Time;SystemMemory)

API(Sys.Job ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Lang;Memory;Concurrency;Threading;Sync;Ownership;ConcurrentContainers;Time;SystemMemory)
Impl(Sys.Job.Core ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Lang;Memory;Platform;System;Threading;Sync;Ownership;ConcurrentContainers;Tools.Tracy;Concurrency;Time;SystemMemory)
Build(Job TYPE STATIC BACKEND Sys.Job.Core LINK Lang;Memory;Threading;Sync;Ownership;ConcurrentContainers;Concurrency;Time;SystemMemory)
add_dependencies(Job Threading)
target_link_libraries(Job PUBLIC Threading)


 
